#include <Arduino.h>
#include "deskmover.h"

// The buttons on the desk controller
#define PIN_UP 33
#define PIN_DOWN 32

// Created in the setup function.
DeskMover::DeskMover()
    : requestedHeight(0),
      moveTickCycle(0),
      prevHeight(0)
{
    pinMode(PIN_UP, OUTPUT);
    pinMode(PIN_DOWN, OUTPUT);
    digitalWrite(PIN_UP, LOW);
    digitalWrite(PIN_DOWN, LOW);
}

unsigned long lastTickFlip = 0;
unsigned long onTime = 100;
unsigned long offTime = 500;

bool DeskMover::handleManualMovement(bool manualUp, bool manualDown)
{
    if (manualUp)
    {
        requestedHeight = 0;
        return moveDeskUp();
    }

    if (manualDown)
    {
        requestedHeight = 0;
        return moveDeskDown();
    }

    return haltMovement();
}

// Sets a specific target height to achieve.
bool DeskMover::requestHeight(uint16_t currHeight, uint16_t reqHeight)
{
    reqHeight = constrain(reqHeight, 720, 1200);
    if (currHeight == reqHeight)
    {
        haltMovement();
        return true;
    }

    // Numbers above 1000 are less precise so just clamp them to the nearest 10.
    if (reqHeight >= 1000)
        reqHeight -= (reqHeight % 10);
    requestedHeight = reqHeight;

    // if currHeight and reqHeight are within 10mm of each other, we slow down.
    Serial.printf("Requested height: %d, current height: %d\n", reqHeight, currHeight);
    if (abs(currHeight - reqHeight) < 10)
    {
        moveDesk(true, currHeight > reqHeight ? PIN_DOWN : PIN_UP);
        return false;
    }
    else
    {
        moveDesk(false, currHeight > reqHeight ? PIN_DOWN : PIN_UP);
        return false;
    }
}

// Immediately halts any movement by setting both pins to LOW.
bool DeskMover::haltMovement()
{
    digitalWrite(PIN_UP, LOW);
    digitalWrite(PIN_DOWN, LOW);
    return false;
}

void DeskMover::moveDesk(bool nearingTarget, int pin)
{
    if (nearingTarget)
    {
        if (millis() - lastTickFlip > onTime)
        {
            moveTickCycle = !moveTickCycle;
            lastTickFlip = millis();
        }
        if (moveTickCycle)
            digitalWrite(pin, LOW);
        else
            digitalWrite(pin, HIGH);
        return;
    }
    digitalWrite(pin, HIGH);
}

bool DeskMover::moveDeskUp()
{
    digitalWrite(PIN_UP, HIGH);
    return true;
}

bool DeskMover::moveDeskDown()
{
    digitalWrite(PIN_DOWN, HIGH);
    return true;
}

// Just forces the desk to go down. In situations where the esp32 has started up
// after the desk was powered up, the screen might be blank, so we can't read a
// sensible value from it. You call this repeatedly until you get a height
// value, at which point you stop calling it and halt movement. Ugly, but it
// works.
void DeskMover::wakeDesk()
{
    digitalWrite(PIN_DOWN, HIGH);
}