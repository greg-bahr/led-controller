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

CRGB leds[LED_COUNT];

CHSV *currentColor = new CHSV(240, 51, 43);
int currentBrightness = 255;
int currentDelayTime = 50;
AnimationType currentAnimation;

Preferences preferences;

MeteorAnimation meteorAnimation(leds, LED_COUNT, *currentColor, LED_COUNT, currentDelayTime);
FillSolidAnimation fillSolidAnimation(leds, LED_COUNT, *currentColor);
ColorWipeAnimation colorWipeAnimation(leds, LED_COUNT, 10, *currentColor, currentDelayTime);
ColorFadeAnimation colorFadeAnimation(leds, LED_COUNT, *currentColor, currentDelayTime);

void setup()
{
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
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection failed! Restarting...");
    delay(2000);
    ESP.restart();
  }

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

  preferences.begin("led-controller", true);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(currentBrightness);

  BLEDevice::init("LED Receiver");
  BLEServer *pServer = BLEDevice::createServer();

  ledService = pServer->createService(BLE_SERVICE_ID);

  animationCharacteristic = ledService->createCharacteristic(BLE_ANIMATION_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  int animation = preferences.getUInt("animation", AnimationType::FillSolid);
  animationCharacteristic->setValue(animation);

  colorCharacteristic = ledService->createCharacteristic(BLE_COLOR_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  int color = preferences.getInt("color", 0xFFFFFF);
  colorCharacteristic->setValue(color);

  brightnessCharacteristic = ledService->createCharacteristic(BLE_BRIGHTNESS_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  int brightness = preferences.getUInt("brightness", 255);
  brightnessCharacteristic->setValue(brightness);

  delayTimeCharacteristic = ledService->createCharacteristic(BLE_DELAYTIME_CHARACTERISTIC_ID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  int delayTime = preferences.getUInt("delayTime", 50);
  delayTimeCharacteristic->setValue(delayTime);

  preferences.clear();
  preferences.putUInt("animation", animation);
  preferences.putUInt("delayTime", delayTime);
  preferences.putUInt("brightness", brightness);
  preferences.putInt("color", color);

  preferences.end();

  ledService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_ID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

int getColorAsInt()
{
  std::string color = colorCharacteristic->getValue();

  return (color.data()[2] << 16) | (color.data()[1] << 8) | color.data()[0];
}

void loop()
{
  ArduinoOTA.handle();

  int animationNum = animationCharacteristic->getValue().data()[0];
  if (animationNum != currentAnimation)
  {
    currentAnimation = static_cast<AnimationType>(animationNum);

    preferences.begin("led-controller", false);
    preferences.putUInt("animation", currentAnimation);
    preferences.end();
  }

  int brightness = brightnessCharacteristic->getValue().data()[0];
  if (brightness != currentBrightness)
  {
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

  if (h != currentColor->h || s != currentColor->s || v != currentColor->v)
  {
    preferences.begin("led-controller", false);
    preferences.putInt("color", getColorAsInt());
    preferences.end();

    currentColor->h = color.data()[0];
    currentColor->s = color.data()[1];
    currentColor->v = color.data()[2];
  }

  int delayTime = delayTimeCharacteristic->getValue().data()[0];
  if (delayTime != currentDelayTime)
  {
    preferences.begin("led-controller", false);
    preferences.putUInt("delayTime", delayTime);
    preferences.end();

    meteorAnimation.setDelayTime(delayTime);
    colorWipeAnimation.setDelayTime(delayTime);
    colorFadeAnimation.setDelayTime(delayTime);
    currentDelayTime = delayTime;
  }

  switch (currentAnimation)
  {
  case Meteor:
  {
    meteorAnimation.run();
    break;
  }
  case FillSolid:
  {
    fillSolidAnimation.run();
    break;
  }
  case ColorWipe:
  {
    colorWipeAnimation.run();
    break;
  }
  case ColorFade:
  {
    colorFadeAnimation.run();
    break;
  }
  }
}