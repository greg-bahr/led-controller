#ifndef ColorFadeAnimation_h
#define ColorFadeAnimation_h

#include <Arduino.h>
#include <FastLED.h>
#include "../Animation.h"

class ColorFadeAnimation : public Animation {
    public:
        ~ColorFadeAnimation(){}
        ColorFadeAnimation(CRGB* leds, int LED_COUNT, CHSV& startColor, int delayTime) 
            : Animation(leds, LED_COUNT, delayTime), startColor(startColor) { }
        void setup() override;
    private:
        void stepFrame() override;
        CHSV& startColor;
        CHSV currentColor;
};

#endif