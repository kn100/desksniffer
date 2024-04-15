#ifndef DESKSNIFFER
#define DESKSNIFFER
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
void connectToWiFi();
String currentHeight();
String requestHeight(int height);
void notFound(AsyncWebServerRequest *request);
void setup();
void loop();
#endif