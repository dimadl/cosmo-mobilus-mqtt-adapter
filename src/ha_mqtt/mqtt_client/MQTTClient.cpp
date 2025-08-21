#include "MQTTClient.h"

MQTTClient::MQTTClient(const char *ssid, const char *password, const char *mqtt_broker, uint16_t mqtt_port, const char *mqtt_username, const char *mqtt_password) : wifiClient(), _client(wifiClient)
{
    this->mqtt_username = mqtt_username;
    this->mqtt_password = mqtt_password;
    this->mqtt_broker = mqtt_broker;
    this->mqtt_port = mqtt_port;
    this->ssid = ssid;
    this->password = password;
}

boolean MQTTClient::connect()
{
    uint8_t tries = 0;
    Serial.println("Checking if WiFi is connected");
    while (!WiFi.isConnected())
    {
        tries++;
        if (tries > MQTT_CLIENT_MAX_RETRIES)
        {
            tries = 0;
            Serial.println("Max attempts exceeded while waiting for WiFi connection");
            WiFi.reconnect();
        }

        delay(500);
        Serial.println("Reconnecting to WiFi..");
    }

    Serial.println("Checking if PubSub is connected");
    while (!this->_client.connected())
    {
        String client_id = "esp32-client-1";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (_client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            Serial.println("Public EMQX MQTT broker connected");
        }
        else
        {
            Serial.print("failed with state ");
            Serial.print(_client.state());
            delay(2000);
        }
    }

    Serial.println("Connection is OK. Proceed.");
    return true;
}

boolean MQTTClient::subscribe(const char *topic)
{
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

    return false;
}

void MQTTClient::begin()
{
    // Connecting to a WiFi network
    Serial.printf("Connecting to Wifi Network %s\n", this->ssid);
    WiFi.begin(this->ssid, this->password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("Connecting to WiFi...");
    }
    Serial.println("WiFi connected");
    this->_client.setServer(mqtt_broker, mqtt_port).setBufferSize(MQTT_MESSAGE_BUFFER);
}

boolean MQTTClient::publish(const char *topic, const char *payload, boolean retained)
{
    uint8_t tries = 0;

    this->connect();
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

    return true;
}

void MQTTClient::setCallback(MQTT_CALLBACK_SIGNATURE)
{
    this->_client.setCallback(callback);
}

boolean MQTTClient::loop()
{
    return this->_client.loop();
}
