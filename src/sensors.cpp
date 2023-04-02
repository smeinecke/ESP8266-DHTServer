#include "sensors.h"

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

bool checkBound(float newValue, float prevValue, float maxDiff)
{
    return !isnan(newValue) &&
           (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void sensors_begin()
{
    dht.begin();
    if (!sht31.begin(0x44))
    { // Set to 0x45 for alternate i2c addr
        Serial.println("Couldn't find SHT31");
    }
}

void readDHT(sensor_data &d)
{
    d.error = false;
    float temp = dht.readTemperature();
    if (isnan(temp))
    {
        Serial.println("Failed to read from DHT sensor!");
        d.error = true;
        return;
    }

    if (checkBound(temp, d.temp, max_diff))
    {
        d.temp = temp;
    }

    // Read Humidity
    temp = dht.readHumidity();
    if (isnan(temp))
    {
        Serial.println("Failed to read from DHT sensor!");
        d.error = true;
        return;
    }

    if (checkBound(temp, d.hum, max_diff))
    {
        d.hum = temp;
    }
}

void readSHT(sensor_data &d)
{
    d.error = false;
    float temp = sht31.readTemperature();
    if (isnan(temp))
    {
        Serial.println("Failed to read from SHT31 sensor!");
        d.error = true;
        return;
    }

    if (checkBound(temp, d.temp, max_diff))
    {
        d.temp = temp;
    }

    // Read Humidity
    temp = sht31.readHumidity();
    if (isnan(temp))
    {
        Serial.println("Failed to read from SHT31 sensor!");
        d.error = true;
        return;
    }

    if (checkBound(temp, d.hum, max_diff))
    {
        d.hum = temp;
    }
}
