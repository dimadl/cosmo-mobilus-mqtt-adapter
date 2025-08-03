#include <Arduino.h>
#include "HAMQTTAdapter.h"
#include "HAMQTTSwitchReporter.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char *ON_STATE = "ON";

WiFiClient espClient;
PubSubClient client(espClient);

HAMQTTSwitchReporter *switchReporters[5];

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.println("Callback");
    String incomingTopic = String(topic);
    String message;

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    message.trim();

    for (uint8_t i = 0; i < sizeof(switchReporters); i++)
    {
        /* Search for switch to report state */
        HAMQTTSwitchReporter *switchReporter = switchReporters[0];
        if (switchReporter->should_report(incomingTopic))
        {
            switchReporter->report_state(message == ON_STATE);
        }
    }
}

HAMQTTAdapter::HAMQTTAdapter(char *mqtt_broker, int mqtt_port, char *mqtt_username, char *mqtt_password)
{
    this->mqtt_broker = mqtt_broker;
    this->mqtt_port = mqtt_port;
    this->mqtt_password = mqtt_username;
    this->mqtt_password = mqtt_password;
}

void HAMQTTAdapter::begin()
{
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected())
    {
        String client_id = "esp32-cosmo-mobilus-adapter-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the Home Assistant (HA) MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            Serial.println("HA MQTT Broker connected");
        }
        else
        {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
}
