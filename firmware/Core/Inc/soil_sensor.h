/*
 * soil_sensor.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_SOIL_SENSOR_H_
#define INC_SOIL_SENSOR_H_

#include <stdint.h>
#include <stdbool.h>

void soil_sensor_init(void);

/* Tra ve true khi doc thanh cong.
   Ghi do am 0..100 vao bien ma percent tro den. */
bool soil_sensor_read_percent(uint8_t *percent);

#endif /* INC_SOIL_SENSOR_H_ */
