#include "config.h"
#include "sensors.h"

#ifndef app_hpp
#define app_hpp

#define MAX_SENSORS 16

typedef struct
{
    sensor_data sensors[MAX_SENSORS];
    unsigned int sensor_count;

    cfg_struct app_cfg;
    int mqtt_state = -1;
} app_state_struct;
#endif