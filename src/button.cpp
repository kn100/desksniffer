#include <Arduino.h>
#include "button.h"

Button::Button(String name, int pin)

    : state(IDLE),
      pin(pin),
      name(name),
      evtBuffer(ButtonEvtBuffer())
{
    pinMode(pin, INPUT_PULLUP);
}

ButtonState Button::getState() const
{
    return state;
}

String Button::string() const
{
    String stateString;
    switch (state)
    {
    case IDLE:
        stateString = "IDLE";
        break;
    case SINGLE_PRESS:
        stateString = "SINGLE_PRESS";
        break;
    case DOUBLE_PRESS:
        stateString = "DOUBLE_PRESS";
        break;
    case HELD:
        stateString = "HELD";
        break;
    }
    return stateString + ":" + String(lastStateChangeTime);
}

void Button::setState(ButtonState newState, unsigned long time)
{
    if (time == lastStateChangeTime)
        return;

    lastStateChangeTime = time;
    state = newState;
    Serial.printf("Button %d: %s\n", pin, string().c_str());
}

unsigned long Button::getLastStateChangeTS() const
{
    return lastStateChangeTime;
}

bool Button::getIsPressed() const
{
    return evtBuffer.get(0).state;
}

void Button::setIsPressed(bool pressed)
{
    ButtonStateChangeEvent evt = {pressed, millis()};
    if (evtBuffer.add(evt))
        Serial.printf("Button %d: %s\n", pin, evtBuffer.string());
}

void Button::recv()
{
    debounceButton();
    recordPresses();
    detectAction();
}

// Helper function to debounce a button
void Button::debounceButton()
{
    bool reading = digitalRead(pin) == LOW;
    if (reading != unstableState)
    {
        lastDebounceTime = millis();
        unstableState = reading;
    }
    long currentTime = millis();
    if ((currentTime - lastDebounceTime) > debounceDelay)
    {
        stableState = reading;
    }
}

void Button::recordPresses()
{
    if (stableState == true && getIsPressed() == false)
        setIsPressed(true);
    else if (stableState == false && getIsPressed() == true)
        setIsPressed(false);
}

void Button::detectAction()
{
    switch (evtBuffer.get(0).state)
    {
    case true:
        if (millis() - evtBuffer.mostRecentStateTS(true) > holdThreshold)
            setState(HELD, evtBuffer.mostRecentStateTS(true));
        break;
    case false:
        if (millis() - evtBuffer.mostRecentStateTS(false) > releaseThreshold && getState() == HELD)
            return setState(IDLE, evtBuffer.mostRecentStateTS(false));
        if (isValidDoublePress())
            return setState(DOUBLE_PRESS, evtBuffer.mostRecentStateTS(false));
        if (isSinglePress(0, 1) && !(millis() - evtBuffer.get(0).time < holdThreshold))
            setState(SINGLE_PRESS, evtBuffer.mostRecentStateTS(false));
        break;
    }
}

bool Button::isSinglePress(int i, int j) const
{
    return (evtBuffer.get(i).state == false &&
            evtBuffer.get(j).state == true &&
            evtBuffer.get(i).time - evtBuffer.get(j).time < holdThreshold);
}

bool Button::isValidDoublePress() const
{
    return isSinglePress(0, 1) && isSinglePress(2, 3) && evtBuffer.get(0).time - evtBuffer.get(3).time < holdThreshold;
}

String Button::getName() const
{
    return name;
}