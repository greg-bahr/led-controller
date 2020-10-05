#include <Arduino.h>
#include <FastLED.h>
#include "MeteorAnimation.h"

void MeteorAnimation::stepFrame() {
    for (int i = 0; i < LED_COUNT; i++) {
        if (random(10) > 5) {
            leds[i].fadeToBlackBy(64);
        }
    }

    if (currentHead < size) {
        leds[currentHead] = color;
    } else if (currentHead > delayTime) {
        currentHead = 0;
    }
    currentHead++;
    
    FastLED.show();
}