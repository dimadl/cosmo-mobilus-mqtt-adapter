#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_MESSAGE_BUFFER 1024
#define MQTT_CLIENT_MAX_RETRIES 30

#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_STATE_CALLBACK_SIGNATURE std::function<void()> reconnectedCallback
#endif

class MQTTClientFeedback
{
public:
    MQTTClientFeedback();

    // init
    void begin();

    // WiFi related feedback;
    void wifiConnectionStarted();
    void wifiConnectionIterationCompleted();
    void wifiConnectionSuccess();
    void wifiStatusReset();

    // PubSub related feedback;
    void pubSubCOnnectionStarted();
    void pubSubConnectionIterationCompleted();
    void pubSubConnectionSuccess();
    void pubSubTransferStarted();
    void pubSubTransferCompleted();
    void pubSubStatusReset();
};

class MQTTClient
{
public:
    MQTTClient();
    ~MQTTClient();

    void setServer(const std::string &mqtt_broker, uint16_t mqtt_port, const std::string &mqtt_username, const std::string &mqtt_password);
    void begin();
    boolean subscribe(const char *topic);
    boolean publish(const char *topic, const char *payload, boolean retained);
    void setCallback(MQTT_CALLBACK_SIGNATURE);
    void setReconnectionCallback(MQTT_STATE_CALLBACK_SIGNATURE);
    boolean loop();

private:
    WiFiClient wifiClient;
    PubSubClient _client;
    MQTTClientFeedback feedback;

    std::string ssid;
    std::string password;
    std::string mqtt_broker;
    uint16_t mqtt_port;
    std::string mqtt_username;
    std::string mqtt_password;

    boolean connect();
    MQTT_STATE_CALLBACK_SIGNATURE;
};

#endif // MQTT_CLIENT_H