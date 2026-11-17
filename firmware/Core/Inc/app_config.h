/*
 * app_config.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_APP_CONFIG_H_
#define INC_APP_CONFIG_H_

#define SENSOR_PERIOD_MS       1000U
#define BUTTON_PERIOD_MS         20U
#define CONTROL_PERIOD_MS       100U
#define DISPLAY_PERIOD_MS       200U
#define UART_PERIOD_MS         1000U

#define DEFAULT_SETPOINT         60U
#define BUTTON_DEBOUNCE_SAMPLES    3U
#define SOIL_AVERAGE_SAMPLES       8U

/* Gia tri mo phong; can hieu chuan lai tren mach that. */
#define SOIL_ADC_DRY            3164U
#define SOIL_ADC_WET            1576U

#endif /* INC_APP_CONFIG_H_ */
