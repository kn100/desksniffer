#ifndef MQTTCONTROL
#define MQTTCONTROL
#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>

class MQTTControl
{
    public: 
        MQTTControl(WiFiClient wifiClient);
        // function to trigger a connection to the mqtt broker
        bool connect(String addr, String clientId, String username, String password);
        // function to inform mqtt broker of current height, and it will return 
        // the new height to set the desk to, or its last known height.
        uint16_t handle(uint16_t height);

    private:
        WiFiClient wifiClient;
        MQTTClient mqtt;
        void messageReceived(String &topic, String &payload);
        uint16_t lastKnownHeight;
        uint16_t requestedHeight;
};

#endif