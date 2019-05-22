#include <Arduino.h>
#include <FastLED.h>
#include "ColorWipeAnimation.h";

void ColorWipeAnimation::stepFrame() {
    for (int i = headIndex - size; i < headIndex; i++) {
        if (i >= 0 && i < LED_COUNT) {
            leds[i] = color;
        }
    }

    FastLED.show();
    FastLED.clear();

    headIndex = (headIndex + 1) % (LED_COUNT+size);
}