#ifndef HA_MQTT_SHUTTER_H
#define HA_MQTT_SHUTTER_H

#include <Arduino.h>
#include <PubSubClient.h>

class HAMQTTShutter
{
public:
    HAMQTTShutter(const char *name, const char *unique_id, PubSubClient &client);
    void begin();
    void setPosition(uint16_t pos);

private:
    const char *name;
    const char *unique_id;

    char commandTopic[100];
    char stateTopic[100];
    char positionTopic[100];

    const char *discoveryTopicTemplate = "homeassistant/cover/%s/config";
    const char *commandTopicTemplate = "shutter/%s/set";
    const char *stateTopicTemplate = "shutter/%s/state";
    const char *positionTopicTemplate = "shutter/%s/position";
    const char *discoveryPayloadTemplate = "{"
                                           "\"name\":\"%s\","
                                           "\"command_topic\":\"%s\","
                                           "\"state_topic\":\"%s\","
                                           "\"unique_id\":\"%s\","
                                           "\"device_class\":\"shutter\","
                                           "\"position_topic\":\"%s\","
                                           "\"payload_open\":\"OPEN\","
                                           "\"payload_close\":\"CLOSE\","
                                           "\"payload_stop\":\"STOP\","
                                           "\"state_opening\":\"opening\","
                                           "\"state_closed\":\"closed\","
                                           "\"state_stopped\":\"stopped\""
                                           "}";
    PubSubClient &_client;

    bool discover();
};

#endif // HA_MQTT_SHUTTER_H