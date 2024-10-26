#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>  // Required for using unsigned long
#include "buttonevtbuffer.h"
enum ButtonState {
    IDLE,
    SINGLE_PRESS,
    DOUBLE_PRESS,
    HELD,
};

class Button {
private:
    String name;
    int pin;
    bool unstableState;
    bool stableState;
    unsigned long lastDebounceTime;
    unsigned long lastStateChangeTime;
    ButtonState state;
    const unsigned long debounceDelay = 1; // 10ms debounce time
    const unsigned long HOLD_THRESHOLD = 300;
    const unsigned long RELEASE_THRESHOLD = 100;  
    const unsigned long SINGLE_PRESS_THRESHOLD = 50;
    const unsigned long DOUBLE_PRESS_THRESHOLD = 300;

    void debounceButton();
    void detectAction();
    void recordPresses();
    bool isSinglePress(int i, int j) const;
    bool isValidDoublePress() const;
    ButtonEvtBuffer evtBuffer;

public:
    Button(String name, int pin);
    void recv();
    String getName() const;
    ButtonState getState() const;
    void setState(ButtonState newState, unsigned long time);

    String string() const;

    unsigned long getLastPressTime() const;
    unsigned long getLastReleaseTime() const;
    unsigned long getLastStateChangeTime() const;
    bool getIsPressed() const;
    void setIsPressed(bool pressed);
};

#endif