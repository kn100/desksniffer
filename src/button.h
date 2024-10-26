#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "buttonevtbuffer.h"
enum ButtonState
{
    IDLE,
    SINGLE_PRESS,
    DOUBLE_PRESS,
    HELD,
};

class Button
{
private:
    const unsigned long debounceDelay = 1;
    const unsigned long holdThreshold = 300;
    const unsigned long releaseThreshold = 100;
    String name;
    int pin;
    bool unstableState;
    bool stableState;
    unsigned long lastDebounceTime;
    unsigned long lastStateChangeTime;
    ButtonState state;
    void setState(ButtonState newState, unsigned long time);

    void debounceButton();
    void detectAction();
    void recordPresses();
    bool isSinglePress(int i, int j) const;
    bool isValidDoublePress() const;
    void setIsPressed(bool pressed);
    ButtonEvtBuffer evtBuffer;

public:
    Button(String name, int pin);
    void recv();
    String getName() const;
    ButtonState getState() const;
    String string() const;
    unsigned long getLastStateChangeTS() const;
    bool getIsPressed() const;
};

#endif