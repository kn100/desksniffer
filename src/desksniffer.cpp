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

#include <WiFi.h>
#include <AsyncTCP.h>
#include "desksniffer.h"
#include "deskheight.h"
#include "deskmover.h"
#include "manualcontrols.h"

const char *SSID = "SomeSSID";
const char *PWD = "SomePassword";
AsyncWebServer server(80);

// The i2c pins on the AiP650EO
#define PIN_SDA 12
#define PIN_SCL 13

// The buttons on the desk controller
#define PIN_UP 33
#define PIN_DOWN 32

// The buttons attached to the ESP32 for manual control
#define PIN_BUTTON_UP 4
#define PIN_BUTTON_DOWN 15
#define PIN_BUTTON_MIDDLE 5

// State variables
bool deskBooted = false;
bool moveRequested = false;

DeskMover deskMover(PIN_UP, PIN_DOWN);
ManualControls manualControls(PIN_BUTTON_UP, PIN_BUTTON_DOWN);

void connectToWiFi()
{
	Serial.print("Connecting to ");
	Serial.println(SSID);

	WiFi.begin(SSID, PWD);

	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(500);
	}

	Serial.print("Connected. IP: ");
	Serial.println(WiFi.localIP());
}

// HTTP handler that returns the last known height
String currentHeight()
{
	String height = "{\"height\":";
	height += DeskHeight::getLastKnownHeight();
	height += "}";
	return height;
}

// HTTP handler that sets a requested height and returns OK
String requestHeight(int height)
{
	moveRequested = true;
	deskMover.requestHeight(height);
	return "OK";
}

// HTTP handler for 404
void notFound(AsyncWebServerRequest *request)
{
	request->send(404, "text/plain", "Not found");
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Connecting to desk...");
	delay(3000);

	DeskHeight::initialize(PIN_SDA, PIN_SCL);
	deskMover.initialize();

	// For unimplemented button just yet, just setting the pin as an input to prevent floating.
	pinMode(PIN_BUTTON_MIDDLE, INPUT_PULLUP);

	connectToWiFi();

	// HTTP handler that either returns the current height or sets a new height
	server.on("/desk", HTTP_GET, [](AsyncWebServerRequest *request)
			  { 
		if (request->hasParam("height")) {
			request->send(200, "text/plain", requestHeight(request->getParam("height")->value().toInt()));
		} else {
			request->send(200, "text/plain", currentHeight()); } });

	server.onNotFound(notFound);
	server.begin();
	Serial.println("Successfully initialized. Letsa goooo!");
}

void loop()
{
	// This delay truly sucks. It is here to essentially slow down the calls to deskMover.handle
	// since apparently the desk doesn't like it when you rapidly press buttons extremely fast.
	delay(50);
	DeskHeight::recv();

	int manualControlEngaged = manualControls.handleButtons();
	if (manualControlEngaged != 0)
		moveRequested = true;
	
	if (moveRequested) 
		moveRequested = deskMover.handle(manualControlEngaged == 1, manualControlEngaged == 2, DeskHeight::getLastKnownHeight());
	if (manualControlEngaged != 0)
		return;

	// Check if we have a valid height. If we do not, we will continue by pressing a random button
	// until we get a valid height, at which point, we will stop pressing buttons.
	if (DeskHeight::getLastKnownHeight() == 0)
	{
		deskBooted = false;
		deskMover.wakeDesk();
		return;
	}
	if (!deskBooted)
	{
		deskBooted = true;
		deskMover.haltMovement();
	}

	if (WiFi.status() != WL_CONNECTED)
		ESP.restart();
}
