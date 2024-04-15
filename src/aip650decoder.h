#ifndef AIP650DECODE
#define AIP650DECODE
#include <Arduino.h>
// AIP650Decoder is a collection of static methods that help decode the data
// you'll get if you ever sniff the i2c data from it.
//  ■■■■■       For reference, take this terrible ASCII drawing that indicates
// ■  4  ■      which bit illumunates which segment. I couldn't tell you what model
// ■2   6■      number the LCD is specifically, but it is a 3 segment display
// ■  1  ■      connected to a Aip650EO LCD Display Driver from Wuxi I-core Elec.
//  ■■■■■       This list is not exhaustive, and just covers the characters my
// ■     ■      particular desk seems to like to display when I confuse it.
// ■3   5■   0  
// ■  7  ■  ■■■
//  ■■■■■   ■■■
class AIP650Decoder
{
public:
    static char getDigit(byte inputByte);
    static int getSegment(byte inputByte);
    static bool hasPeriod(byte inputByte);
    static void printBits(byte b);
};
#endif