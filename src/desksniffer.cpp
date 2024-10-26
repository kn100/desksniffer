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
#include "controlpanel.h"
#include "mqttcontrol.h"

// Your WiFi credentials
const char *SSID = "SomeSSID";
const char *PWD = "SomePassword";

// For gating the slow operations
unsigned long lastExecutionTime = 0;
const unsigned long interval = 40; // How frequently in ms we want to run the "slow" operations

AsyncWebServer server(80);
DeskMover deskMover;
ControlPanel controlPanel;

void setup()
{
	Serial.begin(115200);
	Serial.println("Connecting to desk...");

	DeskHeight::initialize();

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
	performFastOperations();

	if (millis() - lastExecutionTime < interval)
		return;
	lastExecutionTime = millis();
	performSlowOperations();
}

void performFastOperations()
{
	controlPanel.recv();
}

void performSlowOperations()
{
	DeskHeight::recv();
	// Serial.println(controlPanel.string());
	if (DeskHeight::getLastKnownHeight() == 0)
	{
		lastHeightRequestTime = 0;
	}

	// If the buttons have not changed, and there has been no web request for 30 seconds, we will not do anything.
	bool validRequestInTimeframe = lastHeightRequestTime != 0 && (millis() - lastHeightRequestTime) < 30000;
	Action newAction = controlPanel.getAction();
	if (lastHeightRequestTime != newAction.time)
	{
		if (newAction.object == DESK)
		{
			switch (newAction.command)
			{
			case UP:
				satisfied = true;
				deskMover.handleManualMovement(true, false);
				break;
			case DOWN:
				satisfied = true;
				deskMover.handleManualMovement(false, true);
				break;
			case HALT:
				satisfied = true;
				deskMover.haltMovement();
				break;
			// These still not working because we are entering these cases every time rather than just once.
			case UPBY:
				satisfied = false;
				requestedHeight = DeskHeight::getLastKnownHeight() + newAction.value;
				Serial.printf("Requested height: %d, request time: %d\n", requestedHeight, newAction.time);
				lastHeightRequestTime = newAction.time;
				break;
			case DOWNBY:
				satisfied = false;
				requestedHeight = DeskHeight::getLastKnownHeight() - newAction.value;
				lastHeightRequestTime = newAction.time;
				break;
			case TOGGLE:
				break;
			}
		}
	}
	if (!satisfied)
		satisfied = deskMover.requestHeight(DeskHeight::getLastKnownHeight(), requestedHeight);

	if (WiFi.status() != WL_CONNECTED)
		ESP.restart();
}

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
	lastHeightRequestTime = millis();
	deskMover.requestHeight(DeskHeight::getLastKnownHeight(), height);
	return "OK";
}

// HTTP handler for 404
void notFound(AsyncWebServerRequest *request)
{
	request->send(404, "text/plain", "Not found");
}