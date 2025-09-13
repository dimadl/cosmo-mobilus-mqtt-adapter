#include "MQTTClient.h"

MQTTClient *instancePtr = nullptr;

void log_message(char *topic, byte *payload, unsigned int length);

MQTTClient::MQTTClient() : wifiClient(), _client(wifiClient)

{
    this->feedback = MQTTClientFeedback();
    this->_messageQueue = xQueueCreate(10, sizeof(MqttMessage));

    instancePtr = this;
}

MQTTClient::~MQTTClient()
{
}

void MQTTClient::setServer(const std::string &mqtt_broker, uint16_t mqtt_port, const std::string &mqtt_username, const std::string &mqtt_password)
{
    this->mqtt_username = mqtt_username;
    this->mqtt_password = mqtt_password;
    this->mqtt_broker = mqtt_broker;
    this->mqtt_port = mqtt_port;
}

boolean MQTTClient::connect()
{
    this->feedback.pubSubStatusReset();
    this->feedback.wifiStatusReset();

    uint8_t tries = 0;
    Serial.println("Checking if WiFi is connected");
    while (!WiFi.isConnected())
    {
        this->feedback.wifiConnectionStarted();
        tries++;
        if (tries > MQTT_CLIENT_MAX_RETRIES)
        {
            tries = 0;
            Serial.println("Max attempts exceeded while waiting for WiFi connection");
            WiFi.reconnect();
        }

        delay(500);
        Serial.println("Reconnecting to WiFi..");
        this->feedback.wifiConnectionIterationCompleted();
    }

    // "Wifi connection" is successful
    this->feedback.wifiConnectionSuccess();

    Serial.println("Checking if PubSub is connected");
    while (!this->_client.connected())
    {
        // "PubSub connection" started
        this->feedback.pubSubCOnnectionStarted();

        // this->_client.setServer(this->mqtt_broker, this->mqtt_port);

        String client_id = "esp32-client-1";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (_client.connect(client_id.c_str(), mqtt_username.c_str(), mqtt_password.c_str()))
        {
            Serial.println("Public EMQX MQTT broker connected");
        }
        else
        {
            Serial.print("failed with state ");
            Serial.print(_client.state());
            delay(2000);
        }
        this->feedback.pubSubConnectionIterationCompleted();
    }

    this->feedback.pubSubConnectionSuccess();

    Serial.println("Connection is OK. Proceed.");
    return true;
}

void MQTTClient::mqttCallbackStatic(char *topic, byte *payload, unsigned int length)
{
    if (instancePtr)
    {
        instancePtr->mqttCallback(topic, payload, length);
    }
}

void MQTTClient::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    log_message(topic, payload, length);

    MqttMessage msg;

    strncpy(msg.topic, topic, sizeof(msg.topic) - 1);
    msg.topic[sizeof(msg.topic) - 1] = '\0';

    length = min(length, sizeof(msg.payload) - 1);
    memcpy(msg.payload, payload, length);
    msg.payload[length] = '\0';

    xQueueSend(_messageQueue, &msg, 0);
}

void MQTTClient::messageProcessorTask(void *param)
{

    MQTTClient *self = static_cast<MQTTClient *>(param);
    MqttMessage msg;

    while (true)
    {
        if (xQueueReceive(self->_messageQueue, &msg, portMAX_DELAY))
        {
            if (self->_userCallback)
            {
                self->_userCallback(msg);
            }
        }
    }
}

boolean MQTTClient::subscribe(const char *topic)
{
    this->feedback.pubSubTransferStarted();

    Serial.printf("Subscribing on topic %s\n", topic);
    uint8_t tries = 0;

    this->connect();
    while (!this->_client.subscribe(topic))
    {
        Serial.printf("Subscribe on topic %s failed. Retry is on\n", topic);
        tries++;
        if (tries > MQTT_CLIENT_MAX_RETRIES)
        {
            Serial.printf("Subscribe on topic %s failed after all retries\n", topic);
            return false;
        }
        this->subscribe(topic);
    }

    this->feedback.pubSubTransferCompleted();

    return false;
}

void MQTTClient::begin()
{
    this->feedback.begin();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("Connecting to WiFi...");
    }
    Serial.println("WiFi connected");
    this->_client.setServer(mqtt_broker.c_str(), mqtt_port).setBufferSize(MQTT_MESSAGE_BUFFER);
    this->_client.setCallback(mqttCallbackStatic);

    // Start the message processing task
    xTaskCreatePinnedToCore(
        messageProcessorTask,
        "MQTTProcessor",
        4096,
        this,
        1,
        NULL,
        1 // Core 1
    );
}

boolean MQTTClient::publish(const char *topic, const char *payload, boolean retained)
{
    this->feedback.pubSubTransferStarted();

    uint8_t tries = 0;

    this->connect();
    Serial.printf("Publishing to %s\n", this->mqtt_broker.c_str());
    while (!this->_client.publish(topic, payload, retained))
    {
        Serial.printf("Publish on topic %s failed. Retry is on\n", topic);
        tries++;
        if (tries > MQTT_CLIENT_MAX_RETRIES)
        {
            Serial.printf("Publish on topic %s failed after all retries\n", topic);
            return false;
        }
        this->publish(topic, payload, retained);
    }

    this->feedback.pubSubTransferCompleted();

    return true;
}

void MQTTClient::setMessageCallback(MessageCallback callback)
{
    _userCallback = callback;
}

void MQTTClient::setReconnectionCallback(MQTT_STATE_CALLBACK_SIGNATURE)
{
    this->reconnectedCallback = reconnectedCallback;
}

boolean MQTTClient::loop()
{
    if (!_client.connected())
    {
        this->connect();
    }
    return this->_client.loop();
}

#define MQTT_FEEDBACK_PIN_LOADING 33        // Loading pin for both Wifi and PubSub
#define MQTT_FEEDBACK_PIN_WIFI_SUCCESS 25   // WiFiSuccess
#define MQTT_FEEDBACK_PIN_PUBSUB_SUCCESS 26 // PubSub Success

MQTTClientFeedback::MQTTClientFeedback()
{
}

void MQTTClientFeedback::begin()
{
    pinMode(MQTT_FEEDBACK_PIN_LOADING, OUTPUT);
    pinMode(MQTT_FEEDBACK_PIN_WIFI_SUCCESS, OUTPUT);
    pinMode(MQTT_FEEDBACK_PIN_PUBSUB_SUCCESS, OUTPUT);
}

void MQTTClientFeedback::wifiConnectionStarted()
{
    digitalWrite(MQTT_FEEDBACK_PIN_LOADING, HIGH);
}

void MQTTClientFeedback::wifiConnectionIterationCompleted()
{
    digitalWrite(MQTT_FEEDBACK_PIN_LOADING, LOW);
}

void MQTTClientFeedback::wifiConnectionSuccess()
{
    digitalWrite(MQTT_FEEDBACK_PIN_WIFI_SUCCESS, HIGH);
}

void MQTTClientFeedback::wifiStatusReset()
{
    digitalWrite(MQTT_FEEDBACK_PIN_WIFI_SUCCESS, LOW);
}

void MQTTClientFeedback::pubSubCOnnectionStarted()
{
    digitalWrite(MQTT_FEEDBACK_PIN_LOADING, HIGH);
}

void MQTTClientFeedback::pubSubConnectionIterationCompleted()
{
    digitalWrite(MQTT_FEEDBACK_PIN_LOADING, LOW);
}

void MQTTClientFeedback::pubSubConnectionSuccess()
{
    digitalWrite(MQTT_FEEDBACK_PIN_PUBSUB_SUCCESS, HIGH);
}

void MQTTClientFeedback::pubSubTransferStarted()
{
    digitalWrite(MQTT_FEEDBACK_PIN_LOADING, HIGH);
}

void MQTTClientFeedback::pubSubTransferCompleted()
{
    digitalWrite(MQTT_FEEDBACK_PIN_LOADING, LOW);
}

void MQTTClientFeedback::pubSubStatusReset()
{
    digitalWrite(MQTT_FEEDBACK_PIN_PUBSUB_SUCCESS, LOW);
}

void log_message(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}
