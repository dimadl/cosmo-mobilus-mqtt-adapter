#include <Arduino.h>
#include "HAMQTTShutter.h"
#include <PubSubClient.h>

#define HA_MQTT_DEBUG

HAMQTTShutter::HAMQTTShutter(const char *name, const char *unique_id, uint8_t fullTimeToClose, PubSubClient &client) : _client(client)
{
    this->name = name;
    this->unique_id = unique_id;
    this->movingTimePerProcent = fullTimeToClose / 100;

    char buffer[100];
    sprintf(buffer, this->commandTopicTemplate, this->unique_id);
    strncpy(this->commandTopic, buffer, sizeof(this->commandTopic));

    sprintf(buffer, this->stateTopicTemplate, this->unique_id);
    strncpy(this->stateTopic, buffer, sizeof(this->stateTopic));

    sprintf(buffer, this->positionTopicTemplate, this->unique_id);
    strncpy(this->positionTopic, buffer, sizeof(this->positionTopic));

    sprintf(buffer, this->setPositionTopicTemplate, this->unique_id);
    strncpy(this->setPositionTopic, buffer, sizeof(this->setPositionTopic));
}

const char *HAMQTTShutter::getCommandTopic()
{
    return this->commandTopic;
}

const char *HAMQTTShutter::getSetPositionTopic()
{
    return this->setPositionTopic;
}

uint8_t HAMQTTShutter::getTimePerProcent()
{
    return this->movingTimePerProcent;
}

void HAMQTTShutter::begin()
{
    if (discover())
    {
        // Publish initial state
        _client.publish(stateTopic, "stopped", true);
        _client.publish(positionTopic, "100", true);
    }
    else
    {

#if defined(HA_MQTT_DEBUG)
        Serial.println("Discovery failed. Couldn't send a state");
#endif
    }
}

bool HAMQTTShutter::discover()
{
    // Build the topic first
    char discoveryTopic[50];
    sprintf(discoveryTopic, this->discoveryTopicTemplate, unique_id);

#if defined(HA_MQTT_DEBUG)
    Serial.println(discoveryTopic);
#endif

    // Construct discovery payload
    char mqttDiscoveryPayload[500];
    sprintf(mqttDiscoveryPayload, this->discoveryPayloadTemplate, this->name, this->commandTopic, this->stateTopic, this->unique_id, this->positionTopic, this->setPositionTopic);

#if defined(HA_MQTT_DEBUG)
    Serial.println(strlen(mqttDiscoveryPayload));
#endif

    // Send the discovery paload to MQTT Broker
    bool result = _client.publish(discoveryTopic, mqttDiscoveryPayload, true);
    if (result)
    {
        Serial.println("Discovery message successfully published");
    }
    else
    {
        Serial.println("Discovery message failed");
    }

    return result;
}