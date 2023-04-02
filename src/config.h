#include <Arduino.h>

#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#ifndef config_hpp
#define config_hpp

typedef struct
{
    char server[100] = "";
    unsigned int port = 1883;
    char username[100] = "";
    char password[100] = "";
    unsigned int sensors = 2;
} cfg_struct;

bool readConfig(cfg_struct &cfg);
bool saveConfig(cfg_struct cfg);
bool deleteConfig();
#endif