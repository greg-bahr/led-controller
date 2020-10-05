#include <Arduino.h>
#include <FastLED.h>
#include "FillSolidAnimation.h"

void FillSolidAnimation::setup() {
    fill_solid(leds, LED_COUNT, color);
    FastLED.show();
}

void FillSolidAnimation::stepFrame() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = color;
    }

    FastLED.show();
}