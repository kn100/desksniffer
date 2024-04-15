#include <Arduino.h>
#include "aip650decoder.h"
// Function that takes a byte and returns what digit a 7 segment display is showing.
// IE: It takes the 8 bit data portion of the i2c frame and returns the character
// it resembles. This is an imperfect process since some characters are ambiguous,
// but we assume numeric output is the most likely scenario.
char AIP650Decoder::getDigit(byte inputByte)
{
    // We set the MSB of inputByte to 0. It is the indicator of a period, which
    // we don't care about here.
    switch (inputByte & 0x7F)
    {
    case B00111111:
        return '0';
    case B00000110:
        return '1';
    case B01011011:
        return '2';
    case B01001111:
        return '3';
    case B01100110:
        return '4';
    case B01101101:
        return '5'; // Could also be a S, but impossible to know w/o context.
    case B01111101:
        return '6';
    case B00000111:
        return '7';
    case B01111111:
        return '8';
    case B01101111:
        return '9';
    case B00000000:
        return ' ';
    case B01110110:
        return 'H';
    case B01111001:
        return 'E';
    case B01010000:
        return 'R';
    default:
        return '?';
    }
}

// The AiP650e0 expects that data for each character data packet is sent to an
// i2c device address that represents that segment, that receives exactly one
// byte of data. This function takes a byte and returns which segment it is.
// If it returns -1, the byte is not from a segment we know about.
int AIP650Decoder::getSegment(byte inputByte)
{
    switch (inputByte)
    {
    case B00110101:
        return 0;
    case B00110110:
        return 1;
    case B00110111:
        return 2;
    case B00100100: // Irrelevant i2c devices TODO not appropriate here.
    case B00110100:
        return -1;
    default:
        // TODO: Some sort of debug flag that enables or disables this.
        Serial.print("Unknown i2c device:");
        printBits(inputByte);
        return -1;
    }
}

// For a given data byte, this function returns whether or not the period 
// appears after the digit.
bool AIP650Decoder::hasPeriod(byte inputByte)
{
    return (inputByte & B10000000) != 0;
}

// Helper function to print a byte in binary
void AIP650Decoder::printBits(byte b)
{
    for (int i = 7; i >= 0; i--)
        Serial.print(bitRead(b, i));
    Serial.println();
}
