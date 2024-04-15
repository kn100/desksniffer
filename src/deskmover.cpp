#include <Arduino.h>
#include "deskmover.h"

// Created in the setup function.
DeskMover::DeskMover(int upPin, int downPin)
    : upPin(upPin),
      downPin(downPin),
      requestedHeight(0),
      moveTickCycle(0),
      prevHeight(0),
      valueSameForNumberOfCycles(0),
      requestedMove(false)
{
    pinMode(upPin, OUTPUT);
    pinMode(downPin, OUTPUT);
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, LOW);
}

void DeskMover::initialize()
{
    pinMode(upPin, OUTPUT);
    pinMode(downPin, OUTPUT);
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, LOW);
}

// To be called repeatedly in the loop function when a move should be happening
// You'd call it once to start a movement, and if the desk is then determined
// to actually need to move, this will return true until the desk is done moving,
// at which point it will return false. You can then stop calling it.
bool DeskMover::handle(bool manualUp, bool manualDown, uint16_t currHeight)
{
    if (manualUp || manualDown)
        requestedHeight = 0;

    if (manualUp)
        moveDesk(false, upPin);
    else if (manualDown)
        moveDesk(false, downPin);
    else if (requestedHeight == 0 || requestedHeight == currHeight)
        haltMovement();
    else
    {
        uint16_t direction = (requestedHeight > currHeight) ? upPin : downPin;
        uint16_t distanceToTarget = abs(requestedHeight - currHeight);
        bool nearingTarget = (distanceToTarget < 10) ? true : false;
        moveDesk(nearingTarget, direction);
    }
    return deskIsMoving(currHeight);
}

// Sets a specific target height to achieve.
void DeskMover::requestHeight(uint16_t height)
{
    // Valid height range is 720 to 1200.
    height = constrain(height, 720, 1200);

    // Numbers above 1000 are less precise so just clamp them to the nearest 10.
    if (height >= 1000)
        height -= (height % 10);
    Serial.println("Requested height: " + String(height) + "mm");
    requestedHeight = height;
}

// Immediately halts any movement by setting both pins to LOW.
void DeskMover::haltMovement()
{
    digitalWrite(upPin, LOW);
    digitalWrite(downPin, LOW);
    requestedMove = false;
}

// Decides whether the desk is moving. It is determined to be moving if:
// 1. We are trying to move it
// 2. The height is changing We will continue to assume it is moving until the
//    currentHeight has not changed for a number of cycles. This is to account
//    for overshoot/undershoot.
bool DeskMover::deskIsMoving(uint16_t currHeight)
{
    // If we're trying to move, it's safest to assume we are moving.
    if (requestedMove)
    {
        valueSameForNumberOfCycles = 0;
        return true;
    }

    // If we see a difference between the previously observed height and the
    // current height, we're moving, but are overshooting. We should wait until
    // we're not moving anymore.
    if (prevHeight != currHeight)
    {
        prevHeight = currHeight;
        valueSameForNumberOfCycles = 0;
        Serial.print("o");
        return true;
    }

    // If we see the same height for a number of cycles, we're done moving.
    valueSameForNumberOfCycles++;
    // 25 implies 1.5 seconds of no movement. Should be long enough to assume
    // we're done.
    bool stillMoving = (valueSameForNumberOfCycles > 25) ? false : true;
    if (stillMoving)
    {
        Serial.print(".");
    }
    else
    {
        Serial.println("DONE");
    }
    return stillMoving;
}

// Triggers desk movement. nearingTarget informs the function whether the desk
// is getting close to the target, so that we can slow down the movement. pin is
// which pin to set high (ie, desk go up or down)
void DeskMover::moveDesk(bool nearingTarget, int pin)
{
    Serial.print(pin);
    // Flip tick cycle. (We just press the button half the time when we go slow,
    // to attempt to avoid overshoot).
    moveTickCycle = !moveTickCycle;
    // If nearingTarget is true, only move the desk every other tick.
    // Otherwise, move the desk every tick.
    if (nearingTarget && moveTickCycle)
        digitalWrite(pin, HIGH);
    else if (!nearingTarget)
        digitalWrite(pin, HIGH);
    else
        digitalWrite(pin, LOW);
    requestedMove = true;
}

// Just forces the desk to go down. In situations where the esp32 has started up
// after the desk was powered up, the screen might be blank, so we can't read a
// sensible value from it. You call this repeatedly until you get a height
// value, at which point you stop calling it and halt movement. Ugly, but it
// works.
void DeskMover::wakeDesk()
{
    digitalWrite(downPin, HIGH);
}