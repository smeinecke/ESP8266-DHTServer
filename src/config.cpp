#include "config.h"

bool deleteConfig()
{
    Serial.println("mounting FS...");

    if (!LittleFS.begin())
    {
        Serial.println("failed to mount FS");
        return false;
    }

    Serial.println("mounted file system");

    if (!LittleFS.exists("/config.json"))
    {
        Serial.println("no config file found");
        return false;
    }

    if (!LittleFS.remove("/config.json"))
    {
        Serial.println("failed to remove config");
        return false;
    }

    return true;
}

bool readConfig(cfg_struct &cfg)
{
    Serial.println("mounting FS...");

    if (!LittleFS.begin())
    {
        Serial.println("failed to mount FS");
        return false;
    }

    Serial.println("mounted file system");
    if (!LittleFS.exists("/config.json"))
    {
        Serial.println("no config file found");
        return false;
    }
    // file exists, reading and loading
    Serial.println("reading config file");
    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile)
    {
        Serial.println("failed to load json config");
        return false;
    }

    Serial.println("opened config file");
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, buf.get());
    if (error)
    {
        Serial.println("failed to load json config");
        return false;
    }
    Serial.println("parsed json: ");
    serializeJson(doc, Serial);

    if (!doc.containsKey("mqtt"))
    {
        Serial.println("no mqtt key found in config!");
        return false;
    }

    JsonObject mqtt_doc = doc["mqtt"];
    strcpy(cfg.server, mqtt_doc["server"]);
    if (!mqtt_doc.containsKey("port"))
    {
        cfg.port = mqtt_doc["port"];
    }

    strcpy(cfg.username, mqtt_doc["username"]);
    strcpy(cfg.password, mqtt_doc["password"]);
    return true;
}

bool saveConfig(cfg_struct cfg)
{
    Serial.println("saving config");
    DynamicJsonDocument doc(1024);
    JsonObject mqtt_doc = doc.createNestedObject("mqtt");
    mqtt_doc["server"] = cfg.server;
    mqtt_doc["port"] = cfg.port;
    mqtt_doc["username"] = cfg.username;
    mqtt_doc["password"] = cfg.password;

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
        return false;
    }

    serializeJson(doc, Serial);
    if (!serializeJson(doc, configFile))
    {
        Serial.println("failed to store json");
        return false;
    }
    configFile.close();
    return true;
}