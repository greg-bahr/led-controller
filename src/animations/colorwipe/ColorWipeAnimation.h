#ifndef ColorWipeAnimation_h
#define ColorWipeAnimation_h

#include <Arduino.h>
#include <FastLED.h>
#include "../Animation.h"

class ColorWipeAnimation : public Animation {
    public:
        ~ColorWipeAnimation(){}
        ColorWipeAnimation(CRGB* leds, int LED_COUNT, int size, const struct CHSV& color, int delayTime) 
            : Animation(leds, LED_COUNT, delayTime), headIndex(0), size(size), color(color) { }
        void setup(){};
    private:
        const int size;
        const struct CHSV& color;
        int headIndex;
        void stepFrame();
};

#endif