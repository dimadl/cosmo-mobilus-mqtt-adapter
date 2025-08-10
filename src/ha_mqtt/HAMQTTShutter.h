#ifndef HA_MQTT_SHUTTER_H
#define HA_MQTT_SHUTTER_H

#include <Arduino.h>
#include <PubSubClient.h>

class HAMQTTShutter
{
public:
    HAMQTTShutter(const char *name, const char *unique_id, uint8_t fullTimeToClose, PubSubClient &client);
    void begin();
    const char *getCommandTopic() const;
    const char *getSetPositionTopic();
    double getTimePerProcent();

    // MQTT reporting
    // availability
    bool reportAvailable();
    bool reportUnavailable();

    // state
    bool reportOpened();
    bool reportOpening();
    bool reportClosed();
    bool reportClosing();
    bool reportStopped();
    bool reportPosition(uint8_t position);

private:
    const char *name;
    const char *unique_id;
    double movingTimePerProcent;

    const char *commandTopic;
    const char *stateTopic;
    const char *positionTopic;
    const char *setPositionTopic;
    const char *availabilityTopic;

    const char *discoveryTopicTemplate = "homeassistant/cover/%s/config";
    const char *commandTopicTemplate = "shutter/%s/set";
    const char *stateTopicTemplate = "shutter/%s/state";
    const char *positionTopicTemplate = "shutter/%s/position";
    const char *setPositionTopicTemplate = "shutter/%s/set-position";
    const char *availabilityTopicTemplate = "shutter/%s/availability";
    const char *discoveryPayloadTemplate = "{"
                                           "\"name\":\"%s\","
                                           "\"command_topic\":\"%s\","
                                           "\"state_topic\":\"%s\","
                                           "\"unique_id\":\"%s\","
                                           "\"device_class\":\"shutter\","
                                           "\"position_topic\":\"%s\","
                                           "\"set_position_topic\": \"%s\","
                                           "\"payload_open\":\"OPEN\","
                                           "\"payload_close\":\"CLOSE\","
                                           "\"payload_stop\":\"STOP\","
                                           "\"state_open\":\"opened\","
                                           "\"state_opening\":\"opening\","
                                           "\"state_closing\":\"closing\","
                                           "\"state_closed\":\"closed\","
                                           "\"state_stopped\":\"stopped\","
                                           "\"availability\":{\"topic\":\"%s\"}"
                                           "}";
    PubSubClient &_client;

    bool discover();
};

#endif // HA_MQTT_SHUTTER_H