#include "mqtt.h"

extern PubSubClient mqtt_client;
mqtt_topic_struct topics[MAX_SENSORS];
char mqtt_id[20];
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    // message received
}

void mqtt_begin(app_state_struct & as)
{
    // Add the data to the json document
    for (unsigned int n = 0; n < as.sensor_count; ++n)
    {
        sprintf(topics[n].temperature, "esp/%08X/sensor%d/temperature", ESP.getChipId(), n);
        sprintf(topics[n].humidity, "esp/%08X/sensor%d/humidity", ESP.getChipId(), n);
    }

    mqtt_client.setServer(as.app_cfg.server, as.app_cfg.port);
    mqtt_client.setCallback(mqttCallback);
    sprintf(mqtt_id, "ESP_%08X", ESP.getChipId());
    mqtt_connect(as);
}

bool mqtt_connect(app_state_struct & as)
{
    // check if we're already connected
    if (mqtt_client.connected())
    {
        return true;
    }

    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    bool res = mqtt_client.connect(mqtt_id, as.app_cfg.username, as.app_cfg.password);
    as.mqtt_state = mqtt_client.state();
    if (res)
    {
        Serial.println("MQTT connected");
        return true;
    }

    Serial.print("MQTT failed, rc=");
    Serial.println(as.mqtt_state);
    return false;
}

void mqtt_publish(app_state_struct & as)
{
    if (!mqtt_connect(as))
    {
        return;
    }

    for (unsigned int n = 0; n < as.sensor_count; ++n)
    {
        Serial.print("sending \"");
        Serial.print(as.sensors[n].temp);
        Serial.print("\" to ");
        Serial.println( topics[n].temperature);
        mqtt_client.publish(topics[n].temperature, String(as.sensors[n].temp).c_str(), true);
        mqtt_client.publish(topics[n].humidity, String(as.sensors[n].hum).c_str(), true);
    }
    mqtt_client.disconnect();
}
