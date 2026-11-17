/*
 * uart_comm.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_UART_COMM_H_
#define INC_UART_COMM_H_

#include "app_types.h"

void uart_comm_init(void);

/* Gui thong so he thong qua USART2. */
void uart_comm_send_status(const system_data_t *data);

#endif /* INC_UART_COMM_H_ */
