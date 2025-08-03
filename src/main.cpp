#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "";
const char *password = "";

// MQTT Broker
const char *mqtt_broker = "";
const char *topic_ha_status = "homeassistant/status";
const char *mqtt_username = "mqtt_user";
const char *mqtt_password = "";
const int mqtt_port = 1883;

const char *discoveryTopic = "homeassistant/switch/cosmo_mobilus_up/config";

const char *stateTopic = "esp32/cosmo_mobilus/down/state";
const char *commandTopic = "esp32/cosmo_mobilus/down/set";
const char *discoveryPayload = R"rawliteral({
  "name": "Cosmo Mobilus Button Down",
  "command_topic": "esp32/cosmo_mobilus/down/set",
  "state_topic": "esp32/cosmo_mobilus/down/state",
  "unique_id": "cosmo_mobilus_down"
})rawliteral";

WiFiClient espClient;
PubSubClient client(espClient);

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

void callback(char *topic, byte *payload, unsigned int length)
{
  log_message(topic, payload, length);
  String incomingTopic = String(topic);
  String message;

  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  message.trim();

  if (incomingTopic == commandTopic)
  {
    if (message == "ON")
    {
      digitalWrite(23, HIGH);
      client.publish(stateTopic, "ON");
    }
    else if (message == "OFF")
    {
      digitalWrite(23, LOW);
      client.publish(stateTopic, "OFF");
    }
  }
}

void setup()
{
  // Set software serial baud to 115200;
  Serial.begin(115200);

  pinMode(23, OUTPUT);

  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the Wi-Fi network");
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected())
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public EMQX MQTT broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe(topic_ha_status);
  client.subscribe(commandTopic);
  client.publish(discoveryTopic, discoveryPayload, true);
  client.publish(stateTopic, "OFF");
}

void loop()
{
  client.loop();
}