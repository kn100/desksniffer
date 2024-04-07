/**
 * This code is for the ESP32, and is part of a project to connect a VIVO
 *Electric Dual Motor Standing Desk Frame (V122EB) to the internet. It reads
 *data that is being sent to a Aip650EO LCD Display Driver from Wuxi I-core
 *Elec. The format is kinda-I2C. It is a 3 segment display, where each segment
 *is its own i2c device. Each segment receives exactly one byte of data, and the
 *MSB of that byte indicates whether a period should be displayed after the
 *digit. It will output to serial what is being displayed on the LCD. If you got
 *here looking for how to sniff i2c for your own hardware, you might want to
 *look at https://github.com/kn100/I2C-sniffer as that project is far more
 *general. No guarantees it works though :)
 */


#include <Arduino.h>

#define PIN_SDA 12
#define PIN_SCL 13

#define I2C_IDLE 0
#define I2C_TRX 2

// Status of the I2C BUS (Idle or TRX), so we know when to start and stop
// recording data.
static volatile byte i2cStatus = I2C_IDLE;
// Array for storing data we got. Arbitrarily sized. Arrived at this size by
// trial and error. It is a buffer where when we catch up, we start writing to
// the beginning again to avoid running out of space. Kind of like a circular
// buffer, but not really.
static volatile byte dataBuffer[1024];
// points to the first empty position in the dataBuffer for bytes to be written
static volatile uint16_t bufferPoiW = 0;
// points to the position where we've read up until (ie, how far behind we are
// in the buffer)
static uint16_t bufferPoiR = 0;

// Structure and storage for each segment. Each segment has a value and a
// boolean indicating whether a period should be displayed after the digit.
struct segment{
	char segVal;
	bool periodAfter;
} segs[3];

// Helper function to print a byte in binary
void printBits(byte b)
{
  for(int i = 7; i >= 0; i--)
	Serial.print(bitRead(b,i));
  Serial.println();  
}

// Function that takes a byte and returns what digit a 7 segment display is showing.
//  ■■■■■       For reference, take this terrible ASCII drawing that indicates 
// ■  4  ■      which bit illumunates which segment. I couldn't tell you what model
// ■2   6■      number the LCD is specifically, but it is a 3 segment display
// ■  1  ■      connected to a Aip650EO LCD Display Driver from Wuxi I-core Elec. 
//  ■■■■■       This list is not exhaustive, and just covers the characters my
// ■     ■      particular desk seems to display.
// ■3   5■   0
// ■  7  ■  ■■■ 
//  ■■■■■   ■■■
char getDigit(byte inputByte) {
	// Set the MSB of inputByte to 0. If it was 1, it indicates the period AFTER
	// the digit should be illuminated.
	switch (inputByte & 0x7F) {
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

// The AiP650e0 seems to expect data as if each segment is its own i2c device,
// that receives exactly one byte of data. This function takes a byte and
// returns which segment it is.
int getSegment(byte inputByte) {
	switch (inputByte) {
		case B00110101:
			return 0;
		case B00110110:
			return 1;
		case B00110111:
			return 2;
		case B00100100: // Irrelevant i2c devices
		case B00110100: 
			return -1;
		default: 
			Serial.print("Unknown i2c device:");
			printBits(inputByte);
			return -1;
	}
}

// Takes a byte and returns whether the MSB is set to 1. If it is, the segment
// should display a period after the digit.
bool hasPeriod(byte inputByte) {
	return (inputByte & B10000000) != 0;
}


// Handles recording data recvd between start and stop conditions.
void IRAM_ATTR i2cTriggerOnRaisingSCL()
{
	if (i2cStatus == I2C_TRX) 
		dataBuffer[bufferPoiW++] = digitalRead(PIN_SDA);
}

// This interrupt handles recording start and stop conditions.
void IRAM_ATTR i2cTriggerOnChangeSDA()
{
	if (!digitalRead(PIN_SCL))
		return;

	bool sda = digitalRead(PIN_SDA);

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
}

void processDataBuffer()
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

				uint8_t seg = getSegment(addressByte);
				if (seg != -1) {
					segs[seg] = {getDigit(dataByte), hasPeriod(dataByte)};
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
}

void setup()
{
	for (int i = 0; i < 3; i++) {
		segs[i] = {' ', false};
	}

	i2cStatus = I2C_IDLE;
	bufferPoiW = 0;
	bufferPoiR = 0;

	pinMode(PIN_SCL, INPUT_PULLUP);
	pinMode(PIN_SDA, INPUT_PULLUP);
	attachInterrupt(PIN_SCL, i2cTriggerOnRaisingSCL, RISING); 
	attachInterrupt(PIN_SDA, i2cTriggerOnChangeSDA, CHANGE);  
	Serial.begin(115200);
}

void loop()
{
	if (i2cStatus == I2C_IDLE)
	{
		processDataBuffer();

		// Print the data to the serial console in a human readable format.
		for (int i = 0; i < 3; i++) {
			Serial.print(segs[i].segVal);
			if (segs[i].periodAfter) {
				Serial.print(".");
			}
		}
		Serial.println();
		delay(50);
	}
}