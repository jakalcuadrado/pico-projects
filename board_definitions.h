#ifndef BOARD_DEF_H
#define BOARD_DEF_H

#include "pico/stdlib.h"

#define TEMPERATURE_EXPECTED_SENSORS 10

    typedef struct mesuared_data mesuared_data;
struct mesuared_data
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint16_t level;
    uint8_t temperature_sensor_id[TEMPERATURE_EXPECTED_SENSORS];
    int16_t temperature_array[TEMPERATURE_EXPECTED_SENSORS];
    uint8_t signal_strength;
    uint16_t credit;
};

#endif