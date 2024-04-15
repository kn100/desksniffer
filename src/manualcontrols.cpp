#include <Arduino.h>
#include "manualcontrols.h"

ManualControls::ManualControls(int upPin, int downPin)
    : upPin(upPin),
      downPin(downPin)
{
	pinMode(upPin, INPUT_PULLUP);
	pinMode(downPin, INPUT_PULLUP);
}

int ManualControls::handleButtons()
{
	bool upButtonPressed = digitalRead(upPin) == LOW;
	bool downButtonPressed = digitalRead(downPin) == LOW;

	return upButtonPressed ? 1 : (downButtonPressed ? 2 : 0);
}