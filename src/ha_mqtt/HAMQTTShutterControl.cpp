#include <Arduino.h>
#include <PubSubClient.h>
#include "HAMQTTShutter.h"
#include "HAMQTTShutterControl.h"

HAMQTTShutterControl::HAMQTTShutterControl(PubSubClient &client)
    : _client(client)
{
}

void HAMQTTShutterControl::registerShutter(uint8_t index, HAMQTTShutter *shutter)
{
    // Check if the index is in bound
    this->shutters[index] = shutter;
}

void HAMQTTShutterControl::begin()
{
    for (uint8_t i = 0; i < 5; i++)
    {
        if (shutters[i])
        {
            shutters[i]->begin();
        }
    }
}