#include <Arduino.h>

#include "deskheight.h"
#include "aip650decoder.h"

int DeskHeight::sdaPin;
int DeskHeight::sclPin;
volatile byte DeskHeight::i2cStatus;
volatile byte DeskHeight::dataBuffer[4096];
volatile uint16_t DeskHeight::bufferPoiW;
volatile uint16_t DeskHeight::bufferPoiR;
uint16_t DeskHeight::lastKnownHeight;
struct DeskHeight::segment DeskHeight::segs[3];

void DeskHeight::initialize(int sdaPin, int sclPin)
{
    DeskHeight::sdaPin = sdaPin;
    DeskHeight::sclPin = sclPin;
    i2cStatus = I2C_IDLE;
    bufferPoiW = 0;
    bufferPoiR = 0;
    lastKnownHeight = 0;
    for (int i = 0; i < 3; i++)
    {
        segs[i] = {' ', false};
    }
    pinMode(sdaPin, INPUT_PULLUP);
    pinMode(sclPin, INPUT_PULLUP);
    attachInterrupt(sclPin, i2cTriggerOnRaisingSCL, RISING);
    attachInterrupt(sdaPin, i2cTriggerOnChangeSDA, CHANGE);
    Serial.println("DeskHeight:: Interrupts attached");
}

void DeskHeight::stop()
{
    detachInterrupt(sclPin);
    detachInterrupt(sdaPin);
    Serial.println("DeskHeight:: Interrupts detached");
}

// recv triggers actions which need to happen repeatedly in order to keep the
// state of the DeskHeight object up to date. not calling it repeatedly will
// cause the buffer to overflow. recv must be called repeatedly in the control
// loop. It is recommended to call it before any call to getLastKnownHeight.
void DeskHeight::recv()
{
    processDataBuffer();
}

// getLastKnownHeight returns the last known height of the desk. if it returns
// 0, we never received a valid height from the desk controller. It will
// return a height value between 720 and 1200. Heights above 1000 are less
// precise since the desk controller only sends heights in increments of 10
// above 1000.
uint16_t DeskHeight::getLastKnownHeight()
{
    // Time to convert the segment display to a height value, if possible:
    bool hasPeriod = false;

    uint16_t heightToSet = 0;
    for (int i = 0; i < 3; i++)
    {
        if (segs[i].segVal == ' ' || segs[i].segVal == 'H' || segs[i].segVal == 'E' || segs[i].segVal == 'R')
        {
            break;
        }
        if (segs[i].periodAfter)
        {
            hasPeriod = true;
        }
        uint8_t digit = segs[i].segVal - '0';

        heightToSet += digit * pow(10, 2 - i);
    }
    if (!hasPeriod && heightToSet != 0)
    {
        heightToSet *= 10;
    }
    if (heightToSet != 0 && heightToSet != lastKnownHeight)
    {
        // If the heightToSet is not between 720 and 1200, then it is invalid.
        // If the heightToSet is more than 10 away from the last known height,
        // then it is also likely invalid (i2c noise).
        // TODO: Implement some sort of smoothing algorithm to prevent this.
        if (heightToSet < 720 || heightToSet > 1200)
        {
            Serial.println("NOISE, wildly wrong value");
            return lastKnownHeight;
        }
        if (abs(heightToSet - lastKnownHeight) > 10 && lastKnownHeight != 0)
        {
            Serial.println("NOISE, too far from last known height");
            return lastKnownHeight;
        }
        lastKnownHeight = heightToSet;
    }
    return lastKnownHeight;
}

void DeskHeight::processDataBuffer()
{
    if (bufferPoiW == bufferPoiR)
        return;

    uint16_t pw = bufferPoiW;
    uint8_t bytesReadThisSession = 0;
    byte addressByte = 0;
    byte dataByte = 0;

    for (int i = bufferPoiR; i < pw; i++)
    {
        byte data = dataBuffer[i];
        if (data == 'S')
            continue;

        if (data == 's')
        {
            // if we read 19 bytes, then we probably have a valid i2c frame:
            // ADDR (7) | R/W (1) | N/ACK (1) | DATA BYTE (8) | N/ACK (1) | Trash (1)
            if (bytesReadThisSession == 19)
            {
                addressByte = 0;
                for (int j = 0; j < 7; j++)
                {
                    addressByte |= dataBuffer[i - 19 + j] << (6 - j);
                }

                dataByte = 0;
                for (int j = 0; j < 8; j++)
                {
                    dataByte |= dataBuffer[i - 19 + 9 + j] << (7 - j);
                }

                uint8_t seg = AIP650Decoder::getSegment(addressByte);
                if (seg != -1)
                {
                    segs[seg] = {AIP650Decoder::getDigit(dataByte), AIP650Decoder::hasPeriod(dataByte)};
                }
            }
            bytesReadThisSession = 0;
        }
        else
            bytesReadThisSession++;

        bufferPoiR++;
    }

    // if there is no I2C action in progress and there wasn't during the
    // Serial.print then buffer was printed out completely and can be reset.
    if (i2cStatus == I2C_IDLE && pw == bufferPoiW)
        bufferPoiW = bufferPoiR = 0;
};

void IRAM_ATTR DeskHeight::i2cTriggerOnRaisingSCL()
{
    if (i2cStatus == I2C_TRX)
        dataBuffer[bufferPoiW++] = digitalRead(sdaPin);
};

// This interrupt handles recording start and stop conditions.
void IRAM_ATTR DeskHeight::i2cTriggerOnChangeSDA()
{
    if (!digitalRead(sclPin))
        return;

    bool sda = digitalRead(sdaPin);

    if (i2cStatus == I2C_IDLE && !sda) // Clock was high, SDA changed low
    {
        i2cStatus = I2C_TRX;
        dataBuffer[bufferPoiW++] = 'S';
    }
    else if (i2cStatus == I2C_TRX && sda) // Clock was high, SDA changed high
    {
        i2cStatus = I2C_IDLE;
        dataBuffer[bufferPoiW++] = 's';
    }
};