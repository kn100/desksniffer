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

ButtonState Button::getState() const {
    return state;
}

String Button::string() const {
    String stateString;
    switch (state) {
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

void Button::setState(ButtonState newState, unsigned long time) {
    if (time != lastStateChangeTime) {
        lastStateChangeTime = time;
        state = newState;
        Serial.printf("Button %d: %s\n", pin, string().c_str());
    }
}

unsigned long Button::getLastPressTime() const {
    return evtBuffer.mostRecentStateTS(true);
}

unsigned long Button::getLastReleaseTime() const {
    return evtBuffer.mostRecentStateTS(false);
}

unsigned long Button::getLastStateChangeTime() const {
    return lastStateChangeTime;
}


bool Button::getIsPressed() const {
    return evtBuffer.get(0).state;
}

void Button::setIsPressed(bool pressed) {
    ButtonStateChangeEvent evt = {pressed, millis()};
    if (evtBuffer.add(evt)) {
        Serial.printf("Button %d: %s\n", pin, evtBuffer.string().c_str());
    }
}

void Button::recv() {
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

void Button::recordPresses() {
	if (stableState == true && getIsPressed() == false) {
		setIsPressed(true);
	} else if (stableState == false && getIsPressed() == true) {
		setIsPressed(false);
	}
}

void Button::detectAction() {
    switch (evtBuffer.get(0).state) {
        case true:
            if (millis() - evtBuffer.mostRecentStateTS(true) > HOLD_THRESHOLD) {
                setState(HELD, evtBuffer.mostRecentStateTS(true));
            }
            break;
        case false:
            if (millis() - evtBuffer.mostRecentStateTS(false) > RELEASE_THRESHOLD && getState() == HELD) {
                setState(IDLE, evtBuffer.mostRecentStateTS(false));

                return;
            } 
            if (isValidDoublePress()) {
                setState(DOUBLE_PRESS, evtBuffer.mostRecentStateTS(false));
                return;
            }
            if (isSinglePress(0, 1)) {
                // Don't register a single press if the most recent event was less than DOUBLE_PRESS_THRESHOLD ago
                if (!(millis() - evtBuffer.get(0).time < HOLD_THRESHOLD)) {
                    setState(SINGLE_PRESS, evtBuffer.mostRecentStateTS(false));
                }
            }
            break;

    }
}

bool Button::isSinglePress(int i, int j) const {
    if (evtBuffer.get(i).state == false && evtBuffer.get(j).state == true)
    {
        if (evtBuffer.get(i).time - evtBuffer.get(j).time < HOLD_THRESHOLD) {
            return true;
        }
    }
    return false;
}

bool Button::isValidDoublePress() const {
    return isSinglePress(0, 1) && isSinglePress(2, 3) && evtBuffer.get(0).time - evtBuffer.get(3).time < HOLD_THRESHOLD;
}

String Button::getName() const {
    return name;
}