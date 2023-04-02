#include <Arduino.h>

#include "config.h"
#include "app.h"
#include <ArduinoOTA.h>

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include "sensors.h"
#include "mqtt.h"

WiFiManager wifiManager;

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

ESP8266WebServer server(80);
app_state_struct as;

#define TOTAL_SENSORS 2

const int led = 13;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated

// Updates DHT readings every 30 seconds
const long interval = 30000;

// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

String getJsonMessage()
{
  // Create a dynamic json buffer with enough capacity for the JSON message
  DynamicJsonDocument doc(1024);

  doc["mqtt_state"] = as.mqtt_state;

  // Add the data to the json document
  for (unsigned int n = 0; n < as.sensor_count; ++n)
  {
    char object_name[20];
    sprintf(object_name, "sensor%d", n);
    JsonObject sensor = doc.createNestedObject(object_name);
    sensor["temp"] = as.sensors[n].temp;
    sensor["humidity"] = as.sensors[n].hum;
    sensor["error"] = as.sensors[n].error;
  }

  // Convert the json document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);

  return jsonStr;
}

void handleStatusJson()
{
  digitalWrite(led, 1);
  server.send(200, "application/json", getJsonMessage());
  digitalWrite(led, 0);
}

size_t capacityForJson(String json)
{
  const size_t CAPACITY_MULTIPLIER = 2;
  return json.length() * CAPACITY_MULTIPLIER;
}

void handleStoreMQTTconfig()
{
  if (server.method() != HTTP_POST)
  {
    server.send(500, "application/json", "{\"msg\":\"invalid method\"}");
    return;
  }

  String jsonPayload = server.arg("plain");
  size_t capacity = capacityForJson(jsonPayload);
  if (capacity > 1024)
  {
    server.send(500, "application/json", "{\"msg\":\"data too large\"}");
    return;
  }

  DynamicJsonDocument *doc = new DynamicJsonDocument(capacity);
  DeserializationError error = deserializeJson(*doc, jsonPayload.c_str());
  if (error)
  {
    server.send(500, "application/json", "{\"msg\":\"failed to decode json payload\"}");
    delete doc;
    return;
  }

  if ((*doc).containsKey("reset"))
  {
    deleteConfig();
  }
  else
  {
    if (!(*doc).containsKey("server"))
    {
      server.send(500, "application/json", "{\"msg\":\"server parameter missing\"}");
      delete doc;
      return;
    }
    if (!(*doc).containsKey("username"))
    {
      server.send(500, "application/json", "{\"msg\":\"username parameter missing\"}");
      delete doc;
      return;
    }
    if (!(*doc).containsKey("password"))
    {
      server.send(500, "application/json", "{\"msg\":\"password parameter missing\"}");
      delete doc;
      return;
    }

    strcpy(as.app_cfg.server, (*doc)["server"]);
    if (!(*doc).containsKey("port"))
    {
      as.app_cfg.port = (*doc)["port"];
    }

    strcpy(as.app_cfg.username, (*doc)["username"]);
    strcpy(as.app_cfg.password, (*doc)["password"]);

    saveConfig(as.app_cfg);
  }
  delete doc;
  server.send(200, "application/json", "{\"msg\":\"updated.\"}");

  delay(500);
  ESP.restart();
  return;
}

void handleNotFound()
{
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void)
{
  as.sensor_count = TOTAL_SENSORS;

  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  ArduinoOTA.setHostname("keller-temp");
  ArduinoOTA.begin();
  sensors_begin();
  mqtt_begin(as);

  bool config_valid = readConfig(as.app_cfg);

  // wifiManager.resetSettings();

  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", as.app_cfg.server, 100);
  wifiManager.addParameter(&custom_mqtt_server);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt username", as.app_cfg.username, 100);
  wifiManager.addParameter(&custom_mqtt_username);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt password", as.app_cfg.password, 100);
  wifiManager.addParameter(&custom_mqtt_password);

  // set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // wifiManager.setDarkMode(true);
  wifiManager.setConnectRetries(10);

  if (!config_valid)
  {
    wifiManager.resetSettings();
  }
  else
  {
    wifiManager.setConfigPortalTimeout(180);
  }

  bool res = wifiManager.autoConnect();
  if (!res)
  {
    Serial.println("Failed to connect");
    delay(30 * 1000);
    Serial.println("Restart...");
    delay(500);
    ESP.restart();
    return;
  }
  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  strcpy(as.app_cfg.server, custom_mqtt_server.getValue());
  strcpy(as.app_cfg.username, custom_mqtt_username.getValue());
  strcpy(as.app_cfg.password, custom_mqtt_password.getValue());

  // save the custom parameters to FS
  if (shouldSaveConfig)
  {
    saveConfig(as.app_cfg);
  }

  mqtt_begin(as);
  server.on("/json", handleStatusJson);
  server.on("/config", handleStoreMQTTconfig);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
  server.handleClient();
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();
  if (previousMillis == 0 || currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    Serial.println("Checking sensors!");
    readDHT(as.sensors[0]);
    readSHT(as.sensors[1]);
    mqtt_publish(as);
    Serial.println(getJsonMessage());
  }
}
