#ifndef HA_MQTT_SHUTTER_CONTROL_H
#define HA_MQTT_SHUTTER_CONTROL_H

#include "HAMQTTShutter.h"

struct ShutterControlPinsAssignment
{
    u_int8_t pinLeft;
    u_int8_t pinRight;
    u_int8_t pinUp;
    u_int8_t pinDown;
    u_int8_t pinStop;
};

class HAMQTTShutterControl
{
public:
    HAMQTTShutterControl(PubSubClient &client, ShutterControlPinsAssignment &pins);
    void begin();
    void registerShutter(uint8_t index, HAMQTTShutter *shutter);
    void handleCommand(char *topic, byte *payload, unsigned int length);

private:
    PubSubClient &_client;
    ShutterControlPinsAssignment &_pins;

    HAMQTTShutter *shutters[5] = {nullptr};

    // internal state of the control
    u_int8_t currentPosition = 1;

    // Methods
    void moveControlForward(u_int8_t diff);
    void moveControlBackward(u_int8_t diff);
    void close(u_int8_t shutterIndex);
};

#endif // HA_MQTT_SHUTTER_CONTROL_H