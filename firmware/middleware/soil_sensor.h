#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H
#include <stdint.h>

/* Cam bien do am dat tren PA0 / ADC1_IN0. Dien dung: KHO -> ADC CAO. */
void     soil_init(void);
uint16_t soil_read_raw(void);      /* trung binh 8 mau, 0..4095 */
uint8_t  soil_read_percent(void);  /* 0..100 % */

#endif
