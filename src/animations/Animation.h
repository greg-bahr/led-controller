#ifndef Animation_h
#define Animation_h

#include <Arduino.h>
#include <FastLED.h>

enum AnimationType {
    Meteor, FillSolid, ColorWipe, ColorFade
};

class Animation {
    public:
        Animation(CRGB* leds, const int LED_COUNT, int delayTime) 
            : leds(leds), LED_COUNT(LED_COUNT), delayTime(delayTime), lastTimeRan(0), isSetup(false) { }
        virtual ~Animation(){}
        virtual void setup() = 0;
        void setDelayTime(int delayTime);
        void run();
    protected:
        CRGB* leds;
        const int LED_COUNT;
        int delayTime;
    private:
        virtual void stepFrame() = 0;
        long lastTimeRan;
        bool isSetup;
};

#endif