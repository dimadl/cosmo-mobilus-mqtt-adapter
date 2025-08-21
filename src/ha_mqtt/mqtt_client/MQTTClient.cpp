#include "MQTTClient.h"

MQTTClient::MQTTClient(PubSubClient &client, const char *mqtt_username, const char *mqtt_password) : _client(client)
{
    this->mqtt_username = mqtt_username;
    this->mqtt_password = mqtt_password;
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
    while (!_client.connected())
    {
        String client_id = "esp32-client-";
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
