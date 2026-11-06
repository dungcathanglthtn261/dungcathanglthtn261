#ifndef LCD16X02_H
#define LCD16X02_H

/* HD44780 16x2, che do 4-bit: RS=PB0 E=PB1 D4..D7=PB10..PB13, RW noi GND. */
void lcd_init(void);
void lcd_clear(void);
void lcd_print_line(unsigned row, const char *s);   /* ghi du 16 ky tu, tu dong pad khoang trang */

#endif
