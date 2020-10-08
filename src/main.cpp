#include <Arduino.h>
#include <FastLED.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <SPIFFS.h>
#include "animations/colorwipe/ColorWipeAnimation.h"
#include "animations/colorfade/ColorFadeAnimation.h"
#include "animations/fillsolid/FillSolidAnimation.h"
#include "animations/meteor/MeteorAnimation.h"

#define LED_COUNT 150
#define LED_PIN 23

#define BLE_SERVICE_ID "318e961b-a7ba-4acf-95a3-11d94bf554b1"
#define BLE_ANIMATION_CHARACTERISTIC_ID "a96ac45a-57af-4b56-b92d-e9dfb800b521"
#define BLE_COLOR_CHARACTERISTIC_ID "75e42479-5c7e-494d-b391-1d1311153bf5"
#define BLE_BRIGHTNESS_CHARACTERISTIC_ID "de6e22e4-07a0-4924-bc3c-a054ed998e31"
#define BLE_DELAYTIME_CHARACTERISTIC_ID "778decdc-ef0f-4151-9ab6-000150d7d21a"

BLECharacteristic *brightnessCharacteristic;
BLECharacteristic *colorCharacteristic;
BLECharacteristic *animationCharacteristic;
BLECharacteristic *delayTimeCharacteristic;
BLEService *ledService;

bool connectedToWifi = false;

CRGB leds[LED_COUNT];

CHSV *currentColor = new CHSV(100, 100, 100);
int currentBrightness = 255;
int currentDelayTime = 50;
AnimationType currentAnimation;
Animation* animation;

Preferences preferences;

void switchAnimation() {
  delete animation;

  switch (currentAnimation) {
    case Meteor:
      animation = new MeteorAnimation(leds, LED_COUNT, *currentColor, LED_COUNT, currentDelayTime);
      break;
    case FillSolid:
      animation = new FillSolidAnimation(leds, LED_COUNT, *currentColor);
      break;
    case ColorWipe:
      animation = new ColorWipeAnimation(leds, LED_COUNT, 10, *currentColor, currentDelayTime);
      break;
    case ColorFade:
      animation = new ColorFadeAnimation(leds, LED_COUNT, *currentColor, currentDelayTime);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  SPIFFS.begin();
  File f = SPIFFS.open("/wifi_credentials.txt", "r");
  char ssid[30];
  char password[30];

  int length = f.readBytesUntil('\n', ssid, 30);
  ssid[length - 1] = '\0';
  length = f.readBytesUntil('\n', password, 30);
  password[length] = '\0';

  f.close();
  SPIFFS.end();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  connectedToWifi = WiFi.waitForConnectResult() == WL_CONNECTED;

  if (!connectedToWifi) {
    Serial.println("Connection failed! OTA Updates will be disabled!");
  } else {
    ArduinoOTA.setHostname("led-controller");

    ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

    ArduinoOTA.begin();
  }

  preferences.begin("led-controller", true);

  BLEDevice::init("LED Receiver");
  BLEServer *pServer = BLEDevice::createServer();

  ledService = pServer->createService(BLE_SERVICE_ID);

  colorCharacteristic = ledService->createCharacteristic(BLE_COLOR_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  int color = preferences.getInt("color", 0xFFFFFF);
  colorCharacteristic->setValue(color);

  brightnessCharacteristic = ledService->createCharacteristic(BLE_BRIGHTNESS_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  currentBrightness = preferences.getUInt("brightness", 255);
  brightnessCharacteristic->setValue(currentBrightness);

  delayTimeCharacteristic = ledService->createCharacteristic(BLE_DELAYTIME_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  currentDelayTime = preferences.getUInt("delayTime", 50);
  delayTimeCharacteristic->setValue(currentDelayTime);

  animationCharacteristic = ledService->createCharacteristic(BLE_ANIMATION_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  int animationNum = preferences.getUInt("animation", AnimationType::FillSolid);
  currentAnimation = static_cast<AnimationType>(animationNum);
  animationCharacteristic->setValue(animationNum);
  switchAnimation();

  preferences.end();
  ledService->start();

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(currentBrightness);

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_ID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

int getColorAsInt() {
  std::string color = colorCharacteristic->getValue();

  return (color.data()[2] << 16) | (color.data()[1] << 8) | color.data()[0];
}

void loop() {
  if (connectedToWifi) {
    ArduinoOTA.handle();
  }

  int animationNum = animationCharacteristic->getValue().data()[0];
  if (animationNum != currentAnimation) {
    currentAnimation = static_cast<AnimationType>(animationNum);
    switchAnimation();

    preferences.begin("led-controller", false);
    preferences.putUInt("animation", currentAnimation);
    preferences.end();
  }

  int brightness = brightnessCharacteristic->getValue().data()[0];
  if (brightness != currentBrightness) {
    preferences.begin("led-controller", false);
    preferences.putUInt("brightness", brightness);
    preferences.end();

    currentBrightness = brightness;
    FastLED.setBrightness(currentBrightness);
    FastLED.show();
  }

  std::string color = colorCharacteristic->getValue();
  int h = color.data()[0];
  int s = color.data()[1];
  int v = color.data()[2];

  if (h != currentColor->h || s != currentColor->s || v != currentColor->v) {
    preferences.begin("led-controller", false);
    preferences.putInt("color", getColorAsInt());
    preferences.end();

    currentColor->h = color.data()[0];
    currentColor->s = color.data()[1];
    currentColor->v = color.data()[2];
  }

  int delayTime = delayTimeCharacteristic->getValue().data()[0];
  if (delayTime != currentDelayTime) {
    preferences.begin("led-controller", false);
    preferences.putUInt("delayTime", delayTime);
    preferences.end();

    currentDelayTime = delayTime;
    animation->setDelayTime(delayTime);
  }

  animation->run();
}