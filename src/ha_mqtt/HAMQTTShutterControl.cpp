#include <Arduino.h>
#include <PubSubClient.h>
#include "HAMQTTShutter.h"
#include "HAMQTTShutterControl.h"

#define BUTTON_DELAY 10

#define FULLY_OPENED 100

void log_message(char *topic, byte *payload, unsigned int length);
bool activatePin(uint8_t pin);

HAMQTTShutterControl::HAMQTTShutterControl(PubSubClient &client, ShutterControlPinsAssignment &pins)
    : _client(client), _pins(pins)
{
}

void HAMQTTShutterControl::setCurrentPosition(uint8_t currentPosition)
{
    // save to memmemory to make it available after restart
    this->currentPosition = currentPosition;
}

uint8_t HAMQTTShutterControl::getCurrentPosition()
{
    // retrieve from memmory
    return this->currentPosition;
}

void HAMQTTShutterControl::registerShutter(uint8_t index, HAMQTTShutter *shutter)
{
    // Check if the index is in bound
    this->shutters[index] = shutter;
    this->latestShutterPositions[index] = FULLY_OPENED;
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
        else if (shutters[shutterIndex] && shutters[shutterIndex]->getSetPositionTopic() == incomingTopic.c_str())
        {
            // set position of the shutter
            uint8_t requestedPosition = message.toInt();

            // current - 100
            // requested - 80
            // current - requested = 20
            // ---> need to close shutter by 20%

            // current - 40
            // requested - 80
            // current - requeszted = -40
            // ----> need to open by 40%
            uint8_t diff = latestShutterPositions[shutterIndex] - requestedPosition;
            HAMQTTShutter *selectedShutter = shutters[shutterIndex];
            if (diff < 0)
            {
                // open
                uint16_t delay = abs(diff) * selectedShutter->getTimePerProcent();
                openAndDelay(shutterIndex, delay);
            }
            else if (diff > 0)
            {
                // close
                uint16_t delay = abs(diff) * selectedShutter->getTimePerProcent();
                closeAndDelay(shutterIndex, delay);
            }
            else
            {
                // do nothing, stay on the same possition
            }
        }
    }
}

void HAMQTTShutterControl::moveToShutterIndex(uint8_t shutterIndex)
{
    // we know index, but we don't now the current position of the control,
    // so first we should calculate the position and move the control to this position
    uint8_t diff = shutterIndex - getCurrentPosition();
    if (diff < 0)
    {
        // then the control should be moved forfward
        moveControlForward(abs(diff));
    }
    else if (diff > 0)
    {
        // then the control should be moved backward
        moveControlBackward(abs(diff));
    }
    else
    {
        // stay on the the same position of the control
    }
    setCurrentPosition(shutterIndex);
}

void HAMQTTShutterControl::openAndDelay(uint8_t shutterIndex, uint16_t time)
{
    // request to open
    activatePin(_pins.pinUp);

    // main delay
    delay(time);

    // stop
    activatePin(_pins.pinStop);
}

void HAMQTTShutterControl::closeAndDelay(uint8_t shutterIndex, uint16_t time)
{
    // request to close
    activatePin(_pins.pinDown);

    // wait for required time
    delay(time);

    // stop
    activatePin(_pins.pinStop);
}

void HAMQTTShutterControl::moveControlForward(uint8_t p_diff)
{
    for (uint8_t step = 0; step < p_diff; step++)
    {
        activatePin(_pins.pinRight);
    }
}

/// @brief
/// @param p_diff - always positive difference
void HAMQTTShutterControl::moveControlBackward(uint8_t p_diff)
{
    for (uint8_t step = 0; step < p_diff; step++)
    {
        activatePin(_pins.pinLeft);
    }
}

void HAMQTTShutterControl::close(uint8_t shutterIndex)
{
    Serial.println("Start shutter closing");
    // This requires calculation of the position based on passed time
    // Calculation is following: passedTime / reuiredTimeToFullyClose * 100%
    // When close started: start time tracking
    // Track passedTime / reuiredTimeToFullyClose * 100%
    //    1. If Stop was not pressed and passedTime / reuiredTimeToFullyClose * 100% > reuiredTimeToFullyClose consider fully closed and publish closed stated
    //    2. If Stop was pressed, calculate the current position passedTime / reuiredTimeToFullyClose * 100% and publish open state and the position
}

bool activatePin(uint8_t pin)
{
    // Activate pin represeting the "Left" button
    // Deactivate pin represeting the "Left" button
    // The delay should be very low and adjusted when the real controller will received
    digitalWrite(pin, HIGH);
    delay(BUTTON_DELAY);
    digitalWrite(pin, LOW);
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