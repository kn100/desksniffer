#ifndef DESKSNIFFER
#define DESKSNIFFER
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "controlpanel.h"
void connectToWiFi();
String currentHeight();
String requestHeight(int height);
void notFound(AsyncWebServerRequest *request);
void setup();
void loop();
void performFastOperations();
void performSlowOperations();
uint16_t requestedHeight = 0;
unsigned long lastHeightRequestTime = 0;
bool satisfied = true;
#endif