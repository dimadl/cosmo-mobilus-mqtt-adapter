#ifndef HA_MQTT_SHUTTER_CONTROL_H
#define HA_MQTT_SHUTTER_CONTROL_H

#include "HAMQTTShutter.h"
#include "CosmoMobilusHardwareAdapter.h"

#define FULLY_OPENED 100
#define FULLY_CLOSED 0

#define EEPROM_SIZE 8
#define ADDRESS_POSITION 8

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

    HAMQTTShutter *shutters[8] = {nullptr};
    uint8_t latestShutterPositions[8];

    // internal state of the control
    uint8_t currentPosition;

    // Methods
    void setCurrentPosition(uint8_t currentPosition);
    uint8_t getCurrentPosition();

    void
    moveToShutterIndex(uint8_t shutterIndex);

    void openAndDelay(HAMQTTShutter *shutter, uint16_t time);
    void closeAndDelay(HAMQTTShutter *shutter, uint16_t time);
    void moveControlForward(uint8_t diff);
    void moveControlBackward(uint8_t diff);
    void close(HAMQTTShutter *shutter);
};

#endif // HA_MQTT_SHUTTER_CONTROL_H