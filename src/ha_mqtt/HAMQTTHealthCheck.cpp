#include <Arduino.h>
#include "HAMQTTHealthCheck.h"

void HAMQTTHealthCheck::begin()
{
    // Start the health check task
    xTaskCreatePinnedToCore(
        healthCheckTask,
        "MQTTHealthCheck",
        4096,
        this,
        1,
        NULL,
        1 // Core 1
    );
}

void HAMQTTHealthCheck::ping()
{
    Serial.println("Ping processed");
    this->lastSavedPing = millis();
}

void HAMQTTHealthCheck::healthCheckTask(void *param)
{
    HAMQTTHealthCheck *self = static_cast<HAMQTTHealthCheck *>(param);
    while (true)
    {
        Serial.println("Healthcheck");
        if (millis() - self->lastSavedPing > HA_MQTT_HEALTH_CHECK_THRESHOLD)
        {
            self->onDisconnected();
        }
        delay(HA_MQTT_HEALTH_CHECK_FREQ);
    }
}

void HAMQTTHealthCheck::onDisconnected()
{
    ESP.restart();
}
