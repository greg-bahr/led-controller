#include <Arduino.h>
#include "Animation.h"

void Animation::run() {
    if (!isSetup) {
        this->setup();
        isSetup = true;
    }

    long currentTime = millis();
    if (currentTime - lastTimeRan >= delayTime) {
        this->stepFrame();
        lastTimeRan = currentTime;
    }
}

void Animation::setDelayTime(int delayTime) {
    this->delayTime = delayTime;
}