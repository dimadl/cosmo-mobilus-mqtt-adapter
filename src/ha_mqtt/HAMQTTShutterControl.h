#ifndef HA_MQTT_SHUTTER_CONTROL_H
#define HA_MQTT_SHUTTER_CONTROL_H

#include "HAMQTTShutter.h"
#include "CosmoMobilusHardwareAdapter.h"

class HAMQTTShutterControl
{
public:
    HAMQTTShutterControl(PubSubClient &client, CosmoMobilusHardwareAdapter &hardware);
    void begin();
    void registerShutter(uint8_t index, HAMQTTShutter *shutter);
    void handleCommand(char *topic, byte *payload, unsigned int length);

private:
    PubSubClient &_client;
    CosmoMobilusHardwareAdapter &_hardware;

    HAMQTTShutter *shutters[5] = {nullptr};
    uint8_t latestShutterPositions[5];

    // internal state of the control
    uint8_t currentPosition = 1;

    // Methods
    void setCurrentPosition(uint8_t currentPosition);
    uint8_t getCurrentPosition();

    void
    moveToShutterIndex(uint8_t shutterIndex);

    void openAndDelay(uint8_t shutterIndex, uint16_t time);
    void closeAndDelay(uint8_t shutterIndex, uint16_t time);
    void moveControlForward(uint8_t diff);
    void moveControlBackward(uint8_t diff);
    void close(uint8_t shutterIndex);
};

#endif // HA_MQTT_SHUTTER_CONTROL_H