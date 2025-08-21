#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_CLIENT_MAX_RETRIES 30

class MQTTClient
{
public:
    MQTTClient(PubSubClient &client, const char *mqtt_username, const char *mqtt_password);
    boolean subscribe(const char *topic);
    boolean publish(const char *topic, const char *payload, boolean retained);

private:
    PubSubClient &_client;
    const char *mqtt_username;
    const char *mqtt_password;

    boolean connect();
};

#endif // MQTT_CLIENT_H