/*
 * app_types.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_APP_TYPES_H_
#define INC_APP_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    MODE_AUTO = 0,
    MODE_MANUAL
} system_mode_t;

typedef enum
{
    BUTTON_MODE = 0,
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_PUMP,
    BUTTON_COUNT
} button_id_t;

typedef struct
{
    uint8_t soil_percent;
    uint8_t setpoint_percent;
    system_mode_t mode;
    bool pump_on;
    bool sensor_valid;
} system_data_t;


#endif /* INC_APP_TYPES_H_ */
