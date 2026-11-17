/*
 * button.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "app_types.h"

void button_init(void);

/* Goi dinh ky moi 20 ms de quet va chong doi. */
void button_process(void);

/* Lay va xoa su kien nhan cua mot nut.
   Moi lan nhan on dinh chi tra ve true mot lan. */
bool button_take_press(button_id_t id);

#endif /* INC_BUTTON_H_ */
