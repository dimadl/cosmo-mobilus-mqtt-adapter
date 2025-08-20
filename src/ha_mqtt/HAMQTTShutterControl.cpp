#include <Arduino.h>
#include <PubSubClient.h>
#include "CosmoMobilusHardwareAdapter.h"
#include "HAMQTTShutter.h"
#include "HAMQTTShutterControl.h"

#include <EEPROM.h>

void log_message(char *topic, byte *payload, unsigned int length);

HAMQTTShutterControl::HAMQTTShutterControl(PubSubClient &client, CosmoMobilusHardwareAdapter &hardware)
    : _client(client), _hardware(hardware)
{
}

void HAMQTTShutterControl::setCurrentPosition(uint8_t currentPosition)
{
    // save to memmemory to make it available after restart
    EEPROM.write(ADDRESS_POSITION, currentPosition);
    EEPROM.commit();
    Serial.printf("Saving the current position to the memmory %d\n", currentPosition);
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

    // read state from memomory memmory
    uint8_t memShutterPosition = EEPROM.read(index);
    this->latestShutterPositions[index] = memShutterPosition == 255 ? FULLY_OPENED : memShutterPosition;

    Serial.printf("Read the latest position of shutter %d from memmory: %d\n", index, memShutterPosition);
}

void HAMQTTShutterControl::begin()
{
    int8_t memCurrentControlPosition = EEPROM.read(ADDRESS_POSITION);
    this->currentPosition = memCurrentControlPosition == 255 ? 0 : memCurrentControlPosition;
    Serial.printf("Read the current contorl position from memmory: %d\n", memCurrentControlPosition);

    // hardware
    _hardware.begin();

    for (uint8_t i = 0; i < 8; i++)
    {
        if (shutters[i])
        {
            shutters[i]->begin();
            shutters[i]->reportPosition(this->latestShutterPositions[i]);
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

    for (uint8_t shutterIndex = 0; shutterIndex < 8; shutterIndex++)
    {
        if (shutters[shutterIndex] && shutters[shutterIndex]->getCommandTopic() == incomingTopic.c_str())
        {

            if (message == "CLOSE")
            {
                close(shutters[shutterIndex]);
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
                openAndDelay(selectedShutter, delay);
            }
            else if (diff > 0)
            {
                // close
                Serial.printf("Clossing shutter: %d\n", shutterIndex);
                closeAndDelay(selectedShutter, delay);
            }
            else
            {
                // do nothing, stay on the same possition
            }

            Serial.printf("Report position: %d\n", requestedPosition);

            if (requestedPosition == FULLY_CLOSED)
            {
                selectedShutter->reportClosed();
            }

            selectedShutter->reportPosition(requestedPosition);
            latestShutterPositions[shutterIndex] = requestedPosition;
            EEPROM.write(shutterIndex, requestedPosition);
            EEPROM.commit();
            Serial.printf("Saving the latest position of shutter %d to memmory: %d\n", shutterIndex, requestedPosition);
        }
    }
}

void HAMQTTShutterControl::moveToShutterIndex(uint8_t shutterIndex)
{
    // we know index, but we don't now the current position of the control,
    // so first we should calculate the position and move the control to this position
    Serial.printf("Checking if control movement required to reach %d\n", shutterIndex);
    Serial.printf("Current position is %d\n", getCurrentPosition());

    int8_t diff = shutterIndex - getCurrentPosition();

    Serial.printf("Diff for control %d\n", diff);
    if (diff > 0)
    {
        // then the control should be moved forfward
        moveControlForward(abs(diff));
    }
    else if (diff < 0)
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

void HAMQTTShutterControl::openAndDelay(HAMQTTShutter *shutter, uint16_t time)
{
    // request to open
    shutter->reportOpening();
    _hardware.pressUp();

    // main delay
    delay(time * 1000);

    // stop
    _hardware.pressStop();
    shutter->reportStopped();
    shutter->reportOpened();
}

void HAMQTTShutterControl::closeAndDelay(HAMQTTShutter *shutter, uint16_t time)
{
    // request to close
    shutter->reportClosing();
    _hardware.pressDown();

    // wait for required time
    delay(time * 1000);

    // stop
    _hardware.pressStop();
    shutter->reportStopped();
}

void HAMQTTShutterControl::moveControlForward(uint8_t p_diff)
{
    for (uint8_t step = 0; step < p_diff; step++)
    {
        _hardware.pressRight();
        delay(500);
    }
}

/// @brief
/// @param p_diff - always positive difference
void HAMQTTShutterControl::moveControlBackward(uint8_t p_diff)
{
    for (uint8_t step = 0; step < p_diff; step++)
    {
        _hardware.pressLeft();
        delay(500);
    }
}

void HAMQTTShutterControl::close(HAMQTTShutter *shutter)
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