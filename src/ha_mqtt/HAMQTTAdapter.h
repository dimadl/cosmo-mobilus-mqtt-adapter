#ifndef HA_MQTT_MANAGER_H
#define HA_MQTT_MANAGER_H

#include <Arduino.h>

class HAMQTTAdapter
{
public:
    HAMQTTAdapter(char *mqtt_broker, int mqtt_port, char *mqtt_username, char *mqtt_password);
    void begin();
    void register_switch(int pin, char *name);

private:
    char *mqtt_broker;
    char *topic_ha_status = "homeassistant/status";
    char *mqtt_username;
    char *mqtt_password;
    int mqtt_port = 1883;
};

#endif