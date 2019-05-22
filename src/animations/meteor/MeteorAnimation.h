#ifndef MeteorAnimation_h
#define MeteorAnimation_h

#include <Arduino.h>
#include <FastLED.h>
#include "../Animation.h"

class MeteorAnimation : public Animation {
    public:
        ~MeteorAnimation(){}
        MeteorAnimation(CRGB* leds, int LED_COUNT, CHSV& color, int size, int delayTime) 
            : Animation(leds, LED_COUNT, delayTime), color(color), size(size), currentHead(0) { }
        void setup(){}
    private:
        void stepFrame();
        CHSV& color;
        int size;
        int currentHead;
};

#endif