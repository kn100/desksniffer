#include <Arduino.h>
#include <ArduinoJson.h>
#include "mqttcontrol.h"
#include <WiFi.h>
#include <MQTT.h>

// define a string called x and set it to y
const char* state_topic = "kn100/desksniffer";
const char* command_topic = "kn100/desksniffer/set";

// ALMOST works, data ends up in hass but isn't parsed correctly, and hass only emits values from 0 to 100.
// https://discord.com/channels/330944238910963714/672220541343760384/1233296704070221946
//  mosquitto_pub -h 192.168.2.188 -u mqttuser -P "MzEEwfFeVX&%4r" -t "homeassistant/number/desksniffer/config" -m ""
//  mosquitto_sub -h 192.168.2.188 -u mqttuser -P "MzEEwfFeVX&%4r" -t \# -v
MQTTControl::MQTTControl(WiFiClient wifiClient)
    : wifiClient(wifiClient), lastKnownHeight(0), requestedHeight(0), mqtt(256, 512)
{
}

bool MQTTControl::connect(String addr, String clientId, String username, String password)
{
    // Validate WiFi connection
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected. Con't connect to broker if there aint no WiFis!!");
        return false;
    }

    mqtt.begin(addr.c_str(), wifiClient);
    mqtt.onMessage([this](String &topic, String &payload) {
        this->messageReceived(topic, payload);
    });
    Serial.print("\nconnecting to mqtt broker...");
    while (!mqtt.connect(clientId.c_str(), username.c_str(), password.c_str())) {
        return false;
    }

    DynamicJsonDocument doc(256);
    doc["device"]["identifiers"] = "sd1";
    doc["device"]["manufacturer"] = "kn100";
    doc["device"]["model"] = "desk";
    doc["object_id"] = "kevs-desk";
    doc["unique_id"] = "kevs-unique-desk";
    doc["state_topic"] = state_topic;
    doc["command_topic"] = command_topic;
    doc["value_template"] = "{{ value }}";
    doc["unit_of_measurement"] = "mm";
    String jsonStr;
    serializeJson(doc, jsonStr);
    Serial.println(jsonStr);

    Serial.println("connected to mqtt broker");
    mqtt.subscribe(command_topic);
    Serial.println("Publishing discovery payload");
    mqtt.publish("homeassistant/number/desksniffer/config", "", 0, 0);
    mqtt.publish("homeassistant/number/desksniffer/config", jsonStr, 1, 0);
    return true;
}


void MQTTControl::messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
    if (topic == command_topic) {
        Serial.println("Setting height to: " + payload);
        requestedHeight = payload.toInt();
    }
} 

uint16_t MQTTControl::handle(uint16_t height)
{
    mqtt.loop();
    // Firstly, publish the current height to the broker
    if (lastKnownHeight != height)
    {
        Serial.println("Publishing height: " + String(height));
        mqtt.publish(state_topic, String(height));
        lastKnownHeight = height;
    }

    // Then, check if there is a new height to set the desk to
    if (requestedHeight == lastKnownHeight) {
        // Request has been fulfilled
        requestedHeight = 0;
    } else if (requestedHeight != 0)
    {
        // Pending request to set the desk to a new height, return it
        return requestedHeight;
    }
    return requestedHeight;
}