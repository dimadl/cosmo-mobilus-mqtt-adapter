#ifndef COSMO_MOBILUS_HARDWARE_ADAPTER
#define COSMO_MOBILUS_HARDWARE_ADAPTER

#include <Arduino.h>

#define BUTTON_DELAY 10

struct ShutterControlPinsAssignment
{
    u_int8_t pinLeft;
    u_int8_t pinRight;
    u_int8_t pinUp;
    u_int8_t pinDown;
    u_int8_t pinStop;
};

/// @brief Cosmo Mobilus Hardware adapter
class CosmoMobilusHardwareAdapter
{
public:
    CosmoMobilusHardwareAdapter(ShutterControlPinsAssignment &pins);
    void begin();
    bool pressLeft();
    bool pressRight();
    bool pressUp();
    bool pressDown();
    bool pressStop();

private:
    ShutterControlPinsAssignment &_pins;
    bool activatePin(uint8_t pin);
};

#endif // HARDWARE_ADAPTER