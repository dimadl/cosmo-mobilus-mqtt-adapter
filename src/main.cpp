#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#define HA_MQTT_DEBUG
#include "./ha_mqtt/shutter/HAMQTTShutter.h"
#include "./ha_mqtt/HAMQTTShutterControl.h"
#include "./ha_mqtt/CosmoMobilusHardwareAdapter.h"
#include "./ha_mqtt/mqtt_client/MQTTClient.h"

const char *shuttersConfig = R"rawliteral(
{
  "shutters": [
    {"id": "shutter_kitchen", "name": "Shutter Kitchen", "fullTimeToClose": 16, "index": "7"},
    {"id": "shutter_livingroom", "name": "Shutter Living Room", "fullTimeToClose": 16, "index": "1"},
    {"id": "shutter_livingroom_2", "name": "Shutter Living Room 2", "fullTimeToClose": 16, "index": "2"}, 
    {"id": "shutter_livingroom_3", "name": "Shutter Living Room 3", "fullTimeToClose": 16, "index": "3"}, 
    {"id": "shutter_garden", "name": "Shutter Garden", "fullTimeToClose": 25, "index": "4"}, 
    {"id": "shutter_office", "name": "Shutter Office", "fullTimeToClose": 16, "index": "5"},
    {"id": "shutter_bedroom", "name": "Shutter Bedroom", "fullTimeToClose": 16, "index": "6"}
  ]
}
)rawliteral";

Preferences preferences;

MQTTClient mqttClient;

ShutterControlPinsAssignment pins = {/* left */ 22, /* right */ 21, /* up */ 23, /* down */ 15, /* stop */ 4};
CosmoMobilusHardwareAdapter hardwareAdapter = CosmoMobilusHardwareAdapter(pins);
HAMQTTShutterControl haMqttControl(mqttClient, hardwareAdapter);

void setup()
{
  // Set software serial baud to 115200;
  Serial.begin(115200);

  EEPROM.begin(10);

  // Load saved parameters from flash
  preferences.begin("config", true);
  String mqtt_broker = preferences.getString("mqtt_broker", "");
  int mqtt_port = preferences.getInt("mqtt_port", 1883);
  String mqtt_username = preferences.getString("mqtt_username", "");
  String mqtt_password = preferences.getString("mqtt_password", "");
  preferences.end();

  // Let Wifi Manager do connection
  WiFiManager wm;

  WiFiManagerParameter custom_mqtt_broker("mqtt_broker", "MQTT Broker", mqtt_broker.c_str(), 40);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", String(mqtt_port).c_str(), 6);
  WiFiManagerParameter custom_mqtt_username("mqtt_username", "MQTT Username", mqtt_username.c_str(), 40);
  WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", mqtt_password.c_str(), 40, "type='password'");

  wm.addParameter(&custom_mqtt_broker);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_username);
  wm.addParameter(&custom_mqtt_password);

  if (mqtt_broker.length() != 0 && mqtt_port != 0 && mqtt_username.length() != 0 && mqtt_password.length() != 0)
  {
    Serial.println("MQTT Paramters exist. Auto connect");
    if (!wm.autoConnect("Cosmo_Mobilus_PD_Adapter", "pd_adapter"))
    {
      Serial.println("Failed to connect and hot timeout. Restarting..");
      ESP.restart();
    }
  }
  else
  {
    Serial.println("MQTT Are not present. Starting the portal...");
    wm.startConfigPortal("Cosmo_Mobilus_PD_Adapter", "pd_adapter");

    if (wm.getWiFiIsSaved())
    {
      // Settings were saved via portal
      Serial.println("WiFi connected.");
      Serial.println("IP address: " + WiFi.localIP().toString());

      mqtt_broker = custom_mqtt_broker.getValue();
      mqtt_port = String(custom_mqtt_port.getValue()).toInt();
      mqtt_username = custom_mqtt_username.getValue();
      mqtt_password = custom_mqtt_password.getValue();

      preferences.begin("config", false);
      preferences.putString("mqtt_broker", mqtt_broker);
      preferences.putInt("mqtt_port", mqtt_port);
      preferences.putString("mqtt_username", mqtt_username);
      preferences.putString("mqtt_password", mqtt_password);
      preferences.end();
    }
  }

  mqttClient.setServer(mqtt_broker.c_str(), mqtt_port, mqtt_username.c_str(), mqtt_password.c_str());

  DynamicJsonDocument shutterConfigResult(1024);
  DeserializationError error = deserializeJson(shutterConfigResult, shuttersConfig);

  if (error)
  {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray fields = shutterConfigResult["shutters"];
  for (JsonObject field : fields)
  {
    const char *id = field["id"];
    const char *placeholder = field["name"];
    Serial.printf("Field: ID=%s, Placeholder=%s\n", id, placeholder);

    HAMQTTShutter *shutter = new HAMQTTShutter(field["name"], field["id"], field["fullTimeToClose"], mqttClient);
    haMqttControl.registerShutter(field["index"], shutter);
  }

  haMqttControl.begin();
}

void loop()
{
  mqttClient.loop();
}