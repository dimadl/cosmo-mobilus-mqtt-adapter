#ifndef HA_MQTT_HEALTH_CHECK_H
#define HA_MQTT_HEALTH_CHECK_H

/*
    How often the health check job should be triggered
*/
#define HA_MQTT_HEALTH_CHECK_FREQ 60000 // in millis, 1 minute

/*
    Threshold after which the system enters disconnected state and the onDisconnected() method is triggered
*/
#define HA_MQTT_HEALTH_CHECK_THRESHOLD 60000 * 5 // in millis, 5 minutes

/*
    The class defines health check processing function.
    External processes must invoke ping() method to keep the health monitor healthy.

    To start the health check job the begin() method must be invoked. It creates the a task which will be executed
    every HA_MQTT_HEALTH_CHECK_FREQ milli - healthCheckTask()

    When a dicsonnection occurs - onDisconnected() is triggered
*/
class HAMQTTHealthCheck
{
private:
    /* Stores timestamp of the last ping() invocation */
    long lastSavedPing;

    /*
        A task that should be executed with defined HA_MQTT_HEALTH_CHECK_FREQ frequency.
        The goal of the task is to decide whether MQTT can consume messages.
    */
    static void healthCheckTask(void *param);

    /* Disonnection handler. The current implementation relies on the ESP.restart() */
    void onDisconnected();

public:
    HAMQTTHealthCheck() = default;

    /* Initiates health check task*/
    void begin();

    /* Must be triggered when the system receives a ping from external system connected via MQTT */
    void ping();
};

#endif // HA_MQTT_HEALTH_CHECK_H
