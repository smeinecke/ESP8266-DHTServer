#include <Arduino.h>

#include <Wire.h>
#include <SPI.h>
#include "Adafruit_SHT31.h"
#include <DHT.h>

#ifndef sensors_hpp
#define sensors_hpp

typedef struct
{
    float temp = 0.0;
    float hum = 0.0;
    bool error = false;
} sensor_data;

#define DHTPIN 14 // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
// #define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
// #define DHTTYPE    DHT21     // DHT 21 (AM2301)

const float max_diff = 1.0;

void sensors_begin();
void readDHT(sensor_data &d);
void readSHT(sensor_data &d);
#endif