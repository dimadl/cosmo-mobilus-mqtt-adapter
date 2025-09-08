#include <Arduino.h>
#include "./HAMQTTShutter.h"
#include <PubSubClient.h>

HAMQTTShutter::HAMQTTShutter(const char *name, const char *unique_id, uint8_t fullTimeToClose, MQTTClient &client) : _client(client)
{
    this->name = name;
    this->unique_id = unique_id;
    this->movingTimePerProcent = double(fullTimeToClose) / 100.0;

    char buffer[128];
    sprintf(buffer, this->commandTopicTemplate, this->unique_id);
    this->commandTopic = strdup(buffer);

    sprintf(buffer, this->stateTopicTemplate, this->unique_id);
    this->stateTopic = strdup(buffer);

    sprintf(buffer, this->positionTopicTemplate, this->unique_id);
    this->positionTopic = strdup(buffer);

    sprintf(buffer, this->setPositionTopicTemplate, this->unique_id);
    this->setPositionTopic = strdup(buffer);

    sprintf(buffer, this->availabilityTopicTemplate, this->unique_id);
    this->availabilityTopic = strdup(buffer);
}

const char *HAMQTTShutter::getCommandTopic() const
{
    return this->commandTopic;
}

const char *HAMQTTShutter::getSetPositionTopic()
{
    return setPositionTopic;
}

double HAMQTTShutter::getTimePerProcent()
{
    return this->movingTimePerProcent;
}

bool HAMQTTShutter::reportAvailable()
{
    return _client.publish(availabilityTopic, "online", true);
}

bool HAMQTTShutter::reportUnavailable()
{
    return _client.publish(availabilityTopic, "offline", true);
}

bool HAMQTTShutter::reportOpening()
{
    return _client.publish(stateTopic, "opening", true);
}

bool HAMQTTShutter::reportOpened()
{
    return _client.publish(stateTopic, "opened", true);
}

bool HAMQTTShutter::reportClosing()
{
    return _client.publish(stateTopic, "closing", true);
}

bool HAMQTTShutter::reportClosed()
{
    return _client.publish(stateTopic, "closed", true);
}

bool HAMQTTShutter::reportStopped()
{
    return _client.publish(stateTopic, "stopped", true);
}

bool HAMQTTShutter::reportPosition(uint8_t position)
{
    char buffer[10];
    itoa(position, buffer, 10);
    return _client.publish(positionTopic, buffer, true);
}

void HAMQTTShutter::begin()
{
    if (discover())
    {
        // Publish initial state
        reportAvailable();
        reportStopped();
    }
    else
    {

#if defined(HA_MQTT_SHUTTER_DEBUG)
        Serial.println("Discovery failed. Couldn't send a state");
#endif
    }
}

bool HAMQTTShutter::discover()
{
    // Build the topic first
    char discoveryTopic[50];
    sprintf(discoveryTopic, this->discoveryTopicTemplate, unique_id);

#if defined(HA_MQTT_SHUTTER_DEBUG)
    Serial.println(discoveryTopic);
#endif

    // Construct discovery payload
    char mqttDiscoveryPayload[1024];
    sprintf(mqttDiscoveryPayload, this->discoveryPayloadTemplate, this->name, this->commandTopic, this->stateTopic, this->unique_id, this->positionTopic, this->setPositionTopic, this->availabilityTopic);

#if defined(HA_MQTT_SHUTTER_DEBUG)
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