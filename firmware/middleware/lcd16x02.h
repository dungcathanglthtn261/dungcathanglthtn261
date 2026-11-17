#ifndef LCD16X02_H
#define LCD16X02_H

#include <stdint.h>

/* HD44780 16x2, 4-bit: RS=PB0, E=PB1, D4-D7=PB10-PB13, RW=GND. */
void lcd16x02_init(void);
void lcd16x02_write_line(uint8_t row, const char *text);

#endif
