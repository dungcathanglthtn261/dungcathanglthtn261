/*
 * irrigation_control.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_IRRIGATION_CONTROL_H_
#define INC_IRRIGATION_CONTROL_H_

#include "app_types.h"

void irrigation_control_init(system_data_t *data);

/* Xu ly su kien nut nhan trong task Control. */
void irrigation_control_handle_button(system_data_t *data,
                                      button_id_t button);

/* Cap nhat dieu khien AUTO/MANUAL.
   Chi cho phep AUTO tuoi khi sensor_valid = true. */
void irrigation_control_update(system_data_t *data);

#endif /* INC_IRRIGATION_CONTROL_H_ */
