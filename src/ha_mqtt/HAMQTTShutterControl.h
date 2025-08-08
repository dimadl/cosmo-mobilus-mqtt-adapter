#ifndef HA_MQTT_SHUTTER_CONTROL_H
#define HA_MQTT_SHUTTER_CONTROL_H

#include "HAMQTTShutter.h"

class HAMQTTShutterControl
{
public:
    HAMQTTShutterControl(PubSubClient &client);
    void begin();
    void registerShutter(uint8_t index, HAMQTTShutter *shutter);

private:
    PubSubClient &_client;
    HAMQTTShutter *shutters[5] = {nullptr};
};

#endif // HA_MQTT_SHUTTER_CONTROL_H