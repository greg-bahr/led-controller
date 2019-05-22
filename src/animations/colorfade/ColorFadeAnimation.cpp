#include <Arduino.h>
#include <FastLED.h>
#include "ColorFadeAnimation.h";

void ColorFadeAnimation::stepFrame() {
    currentColor.hue++;

    for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = currentColor;
    }

    FastLED.show();
}

void ColorFadeAnimation::setup() {
    currentColor = startColor;
}