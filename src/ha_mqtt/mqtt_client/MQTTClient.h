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

struct MqttMessage
{
    char topic[128];
    char payload[256];
};

using MessageCallback = std::function<void(const MqttMessage &)>;

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

    void setMessageCallback(MessageCallback callback);
    void setReconnectionCallback(MQTT_STATE_CALLBACK_SIGNATURE);
    boolean loop();

private:
    WiFiClient wifiClient;
    PubSubClient _client;
    MQTTClientFeedback feedback;

    std::string mqtt_broker;
    uint16_t mqtt_port;
    std::string mqtt_username;
    std::string mqtt_password;

    QueueHandle_t _messageQueue;
    MessageCallback _userCallback = nullptr;

    boolean connect();
    static void mqttCallbackStatic(char *topic, byte *payload, unsigned int length);
    void mqttCallback(char *topic, byte *payload, unsigned int length);

    static void messageProcessorTask(void *param);

    MQTT_STATE_CALLBACK_SIGNATURE;
};

#endif // MQTT_CLIENT_H