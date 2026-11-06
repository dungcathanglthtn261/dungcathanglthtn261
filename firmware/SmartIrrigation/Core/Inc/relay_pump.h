/*
 * relay_pump.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_RELAY_PUMP_H_
#define INC_RELAY_PUMP_H_

#include <stdbool.h>

/* Khoi tao bom o trang thai OFF. */
void relay_pump_init(void);

/* on = true: PA4 LOW; on = false: PA4 HIGH. */
void relay_pump_set(bool on);

bool relay_pump_is_on(void);

#endif /* INC_RELAY_PUMP_H_ */
