#ifndef FillSolidAnimation_h
#define FillSolidAnimation_h

#include <Arduino.h>
#include <FastLED.h>
#include "../Animation.h"

class FillSolidAnimation : public Animation {
    public:
        ~FillSolidAnimation(){}
        FillSolidAnimation(CRGB* leds, int LED_COUNT, CHSV& color) 
            : Animation(leds, LED_COUNT, 500), color(color) { }
        void setup();
    private:
        CHSV& color;
        void stepFrame();
};

#endif