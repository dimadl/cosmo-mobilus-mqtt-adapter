#include <Arduino.h>
#include <PubSubClient.h>
#include "CosmoMobilusHardwareAdapter.h"
#include "HAMQTTShutter.h"
#include "HAMQTTShutterControl.h"

#define FULLY_OPENED 100

void log_message(char *topic, byte *payload, unsigned int length);

HAMQTTShutterControl::HAMQTTShutterControl(PubSubClient &client, CosmoMobilusHardwareAdapter &hardware)
    : _client(client), _hardware(hardware)
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
    // hardware
    _hardware.begin();

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
        else if (shutters[shutterIndex] && strcmp(shutters[shutterIndex]->getSetPositionTopic(), incomingTopic.c_str()) == 0)
        {
            moveToShutterIndex(shutterIndex);

            // adding delay between commands
            delay(500);

            Serial.println("Requested set position");
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

            HAMQTTShutter *selectedShutter = shutters[shutterIndex];

            // calculate required delay based on the shutter's speed
            int8_t diff = latestShutterPositions[shutterIndex] - requestedPosition;
            Serial.printf("Required shift: %d\n", diff);

            double delay = abs(diff) * selectedShutter->getTimePerProcent();
            Serial.printf("Calculated delay: %.4f\n", delay);
            if (diff < 0)
            {
                // open
                Serial.printf("Openning shutter: %d\n", shutterIndex);
                openAndDelay(shutterIndex, delay);
            }
            else if (diff > 0)
            {
                // close
                Serial.printf("Clossing shutter: %d\n", shutterIndex);
                closeAndDelay(shutterIndex, delay);
            }
            else
            {
                // do nothing, stay on the same possition
            }

            Serial.printf("Report position: %d\n", requestedPosition);
            selectedShutter->reportPosition(requestedPosition);
            latestShutterPositions[shutterIndex] = requestedPosition;
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
    _hardware.pressUp();
    shutters[shutterIndex]->reportOpening();

    // main delay
    delay(time * 1000);

    // stop
    shutters[shutterIndex]->reportStopped();
    _hardware.pressStop();
}

void HAMQTTShutterControl::closeAndDelay(uint8_t shutterIndex, uint16_t time)
{
    // request to close
    _hardware.pressDown();
    shutters[shutterIndex]->reportOpening();

    // wait for required time
    delay(time * 1000);

    // stop
    shutters[shutterIndex]->reportStopped();
    _hardware.pressStop();
}

void HAMQTTShutterControl::moveControlForward(uint8_t p_diff)
{
    for (uint8_t step = 0; step < p_diff; step++)
    {
        _hardware.pressRight();
    }
}

/// @brief
/// @param p_diff - always positive difference
void HAMQTTShutterControl::moveControlBackward(uint8_t p_diff)
{
    for (uint8_t step = 0; step < p_diff; step++)
    {
        _hardware.pressLeft();
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