#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

void soil_sensor_init(void);
bool soil_sensor_read_percent(uint8_t *percent);

#endif
