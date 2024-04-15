#ifndef DESKHEIGHT_H
#define DESKHEIGHT_H

#include <Arduino.h>
#include "aip650decoder.h"

#define I2C_IDLE 0
#define I2C_TRX 2

/* DeskHeight is a class that is entirely static. This code is for the ESP32,
 * and is part of a project to connect a VIVO Electric Dual Motor Standing Desk
 * Frame (V122EB) to the internet. It reads data that is being sent to a
 * Aip650EO LCD Display Driver from Wuxi I-core Elec. The format is kinda-I2C.
 * It is a 3 segment display, where each segment is its own i2c device. Each
 * segment receives exactly one byte of data, and the MSB of that byte indicates
 * whether a period should be displayed after the digit. It maintains a
 * dataBuffer that is continually emptied by you calling recv() in your control
 * loop. You can then request the last known height at any time. */
class DeskHeight
{
    static int sdaPin;
    static int sclPin;
    static volatile byte i2cStatus;
    static volatile byte dataBuffer[4096];
    static volatile uint16_t bufferPoiW;
    static volatile uint16_t bufferPoiR;
    static uint16_t lastKnownHeight;
    static uint16_t lastKnownHeights[10];

    static struct segment
    {
        char segVal;
        bool periodAfter;
    } segs[3];

    static void IRAM_ATTR i2cTriggerOnRaisingSCL();
    static void IRAM_ATTR i2cTriggerOnChangeSDA();
    static void processDataBuffer();
    public: 
        static void initialize(int sdaPin, int sclPin);
        static void stop();
        static void recv();
        static uint16_t getLastKnownHeight();
};
#endif