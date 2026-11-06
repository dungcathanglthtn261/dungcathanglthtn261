/*
 * lcd16x02.h
 *
 *  Created on: Nov 6, 2026
 *      Author: user
 */

#ifndef INC_LCD16X02_H_
#define INC_LCD16X02_H_

#include <stdint.h>

void lcd16x02_init(void);

/* row = 0 hoac 1.
   Hien thi toi da 16 ky tu, bu khoang trang phan con lai. */
void lcd16x02_write_line(uint8_t row, const char *text);

#endif /* INC_LCD16X02_H_ */
