#include "CosmoMobilusHardwareAdapter.h"

CosmoMobilusHardwareAdapter::CosmoMobilusHardwareAdapter(ShutterControlPinsAssignment &pins) : _pins(pins)
{
}

void CosmoMobilusHardwareAdapter::begin()
{
    pinMode(_pins.pinLeft, OUTPUT);
    pinMode(_pins.pinRight, OUTPUT);
    pinMode(_pins.pinUp, OUTPUT);
    pinMode(_pins.pinDown, OUTPUT);
    pinMode(_pins.pinStop, OUTPUT);
}

bool CosmoMobilusHardwareAdapter::pressLeft()
{
    return activatePin(_pins.pinLeft);
}

bool CosmoMobilusHardwareAdapter::pressRight()
{
    return activatePin(_pins.pinRight);
}

bool CosmoMobilusHardwareAdapter::pressUp()
{
    return activatePin(_pins.pinUp);
}

bool CosmoMobilusHardwareAdapter::pressDown()
{
    return activatePin(_pins.pinDown);
}

bool CosmoMobilusHardwareAdapter::pressStop()
{
    return activatePin(_pins.pinStop);
}

bool CosmoMobilusHardwareAdapter::activatePin(uint8_t pin)
{
    // Activate pin represeting the "Left" button
    // Deactivate pin represeting the "Left" button
    // The delay should be very low and adjusted when the real controller will received
    digitalWrite(pin, HIGH);
    delay(BUTTON_DELAY);
    digitalWrite(pin, LOW);

    return true;
}
