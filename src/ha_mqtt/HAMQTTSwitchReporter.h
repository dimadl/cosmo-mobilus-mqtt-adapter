#ifndef HA_MQTT_SWITCH_REPORTER_H
#define HA_MQTT_SWITCH_REPORTER_H

#include <Arduino.h>
#include <PubSubClient.h>

class HAMQTTSwitchReporter
{

public:
    HAMQTTSwitchReporter(int pin, char *name);
    void begin();
    bool should_report(String topic);
    void report_state(bool state);

private:
    int pin;
    char *name;
    PubSubClient _client;

    void _discovery();
    void _report_stateto_mqtt();

protected:
    const char *discoveryTopicFormat = "homeassistant/switch/%s/config";
    const char *stateTopicFormat = "esp32/%s/state";
    const char *commandTopicFormat = "esp32/%s/set";
};

#endif