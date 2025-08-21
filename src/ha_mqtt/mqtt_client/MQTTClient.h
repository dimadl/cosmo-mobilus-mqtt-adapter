#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_MESSAGE_BUFFER 1024
#define MQTT_CLIENT_MAX_RETRIES 30

class MQTTClient
{
public:
    MQTTClient(const char *ssid, const char *password, const char *mqtt_broker, uint16_t mqtt_port, const char *mqtt_username, const char *mqtt_password);
    void begin();
    boolean subscribe(const char *topic);
    boolean publish(const char *topic, const char *payload, boolean retained);
    void setCallback(MQTT_CALLBACK_SIGNATURE);
    boolean loop();

private:
    WiFiClient wifiClient;
    PubSubClient _client;

    const char *ssid;
    const char *password;
    const char *mqtt_broker;
    uint16_t mqtt_port;
    const char *mqtt_username;
    const char *mqtt_password;

    boolean connect();
};

#endif // MQTT_CLIENT_H