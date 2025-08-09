#include <Arduino.h>
#include <PubSubClient.h>
#include "HAMQTTShutter.h"
#include "HAMQTTShutterControl.h"

#define BUTTON_DELAY 10

void log_message(char *topic, byte *payload, unsigned int length);

HAMQTTShutterControl::HAMQTTShutterControl(PubSubClient &client, ShutterControlPinsAssignment &pins)
    : _client(client), _pins(pins)
{
}

void HAMQTTShutterControl::registerShutter(uint8_t index, HAMQTTShutter *shutter)
{
    // Check if the index is in bound
    this->shutters[index] = shutter;
}

void HAMQTTShutterControl::begin()
{
    pinMode(_pins.pinLeft, OUTPUT);
    pinMode(_pins.pinRight, OUTPUT);
    pinMode(_pins.pinUp, OUTPUT);
    pinMode(_pins.pinDown, OUTPUT);
    pinMode(_pins.pinStop, OUTPUT);

    for (uint8_t i = 0; i < 5; i++)
    {
        if (shutters[i])
        {
            shutters[i]->begin();
            _client.subscribe(shutters[i]->getCommandTopic());
            _client.subscribe(shutters[i]->getSetPositionTopic());
        }
    }
}

void HAMQTTShutterControl::handleCommand(char *topic, byte *payload, unsigned int length)
{
    log_message(topic, payload, length);
    String incomingTopic = String(topic);
    String message;

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    message.trim();

    for (uint8_t shutterIndex = 0; shutterIndex < 5; shutterIndex++)
    {
        if (shutters[shutterIndex] && shutters[shutterIndex]->getCommandTopic() == incomingTopic.c_str())
        {
            // we know index, but we don't now the current position of the control,
            // so first we should calculate the position and move the control to this position
            u_int8_t diff = shutterIndex - currentPosition;
            if (diff < 0)
            {
                // then the control should be moved forfward
                moveControlForward(diff);
            }
            else if (diff > 0)
            {
                // then the control should be moved backward
                moveControlBackward(diff);
            }
            else
            {
                // stay on the the same position of the control
            }

            if (message == "CLOSE")
            {
                close(shutterIndex);
            }
            else if (message == "OPEN")
            {
                // open();
            }
            else if (message == "STOP")
            {
                // stop();
            }
        }
    }
}

void HAMQTTShutterControl::moveControlForward(u_int8_t diff)
{
    for (uint8_t step = 0; step < abs(diff); step++)
    {
        // Activate pin represeting the "Right" button
        // Deactivate pin represeting the "Right" button
        // The delay should be very low and adjusted when the real controller will received
        digitalWrite(_pins.pinRight, HIGH);
        delay(BUTTON_DELAY);
        digitalWrite(_pins.pinRight, LOW);
    }
}

void HAMQTTShutterControl::moveControlBackward(u_int8_t diff)
{
    for (uint8_t step = 0; step < abs(diff); step++)
    {
        // Activate pin represeting the "Left" button
        // Deactivate pin represeting the "Left" button
        // The delay should be very low and adjusted when the real controller will received
        digitalWrite(_pins.pinLeft, HIGH);
        delay(BUTTON_DELAY);
        digitalWrite(_pins.pinLeft, LOW);
    }
}

void HAMQTTShutterControl::close(u_int8_t shutterIndex)
{
    Serial.println("Start shutter closing");
    // This requires calculation of the position based on passed time
    // Calculation is following: passedTime / reuiredTimeToFullyClose * 100%
    // When close started: start time tracking
    // Track passedTime / reuiredTimeToFullyClose * 100%
    //    1. If Stop was not pressed and passedTime / reuiredTimeToFullyClose * 100% > reuiredTimeToFullyClose consider fully closed and publish closed stated
    //    2. If Stop was pressed, calculate the current position passedTime / reuiredTimeToFullyClose * 100% and publish open state and the position
}

void log_message(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}