#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <Preferences.h>

#define HA_MQTT_DEBUG
#include "./ha_mqtt/HAMQTTShutter.h"
#include "./ha_mqtt/HAMQTTShutterControl.h"
#include "./ha_mqtt/CosmoMobilusHardwareAdapter.h"
#include "./ha_mqtt/mqtt_client/MQTTClient.h"

Preferences preferences;

MQTTClient mqttClient;

ShutterControlPinsAssignment pins = {/* left */ 22, /* right */ 21, /* up */ 23, /* down */ 15, /* stop */ 5};
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

  HAMQTTShutter *shutterKitchen = new HAMQTTShutter("Shutter Kitchen", "shutter_kitchen", 16, mqttClient);
  HAMQTTShutter *shutterLivingRoom = new HAMQTTShutter("Shutter Living Room", "shutter_livingroom", 16, mqttClient);
  HAMQTTShutter *shutterLivingRoom2 = new HAMQTTShutter("Shutter Living Room 2", "shutter_livingroom_2", 16, mqttClient);
  HAMQTTShutter *shutterLivingRoom3 = new HAMQTTShutter("Shutter Living Room 3", "shutter_livingroom_3", 16, mqttClient);
  HAMQTTShutter *shutterLivingRoomGarden = new HAMQTTShutter("Shutter Garden", "shutter_garden", 25, mqttClient);
  HAMQTTShutter *shutterOffice = new HAMQTTShutter("Shutter Office", "shutter_office", 16, mqttClient);
  HAMQTTShutter *shutterBedroom = new HAMQTTShutter("Shutter Bedroom", "shutter_bedroom", 16, mqttClient);

  haMqttControl.registerShutter(1, shutterLivingRoom);
  haMqttControl.registerShutter(2, shutterLivingRoom2);
  haMqttControl.registerShutter(3, shutterLivingRoom3);
  haMqttControl.registerShutter(4, shutterLivingRoomGarden);
  haMqttControl.registerShutter(5, shutterOffice);
  haMqttControl.registerShutter(6, shutterBedroom);
  haMqttControl.registerShutter(7, shutterKitchen);
  haMqttControl.begin();
}

void loop()
{
  mqttClient.loop();
}