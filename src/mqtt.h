#include <Arduino.h>

#include "config.h"
#include "app.h"
#include <PubSubClient.h>

typedef struct
{
    char humidity[100];
    char temperature[100];
} mqtt_topic_struct;

void mqtt_begin(app_state_struct &as);
bool mqtt_connect(app_state_struct &as);
void mqtt_publish(app_state_struct &as);