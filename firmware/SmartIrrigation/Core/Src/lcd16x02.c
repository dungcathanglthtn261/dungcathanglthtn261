#include "lcd16x02.h"
#include "main.h"

#include <stddef.h>

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((uint32_t)(DWT->CYCCNT - start) < ticks) { }
}

static void pulse_enable(void)
{
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    delay_us(2U);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    delay_us(50U);
}

static void write_nibble(uint8_t value)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin,
                      (value & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin,
                      (value & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin,
                      (value & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin,
                      (value & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    pulse_enable();
}

static void write_byte(uint8_t value, GPIO_PinState rs)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs);
    write_nibble((uint8_t)(value >> 4));
    write_nibble((uint8_t)(value & 0x0FU));
}

void lcd16x02_init(void)
{
    /* CubeMX already configured the LCD GPIOs; only enable the Cortex DWT timer. */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    HAL_Delay(50U);
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    write_nibble(0x03U);
    HAL_Delay(5U);
    write_nibble(0x03U);
    delay_us(200U);
    write_nibble(0x03U);
    delay_us(200U);
    write_nibble(0x02U);
    write_byte(0x28U, GPIO_PIN_RESET); /* 4-bit, two lines, 5x8 font */
    write_byte(0x0CU, GPIO_PIN_RESET); /* display on, cursor off */
    write_byte(0x06U, GPIO_PIN_RESET); /* increment address */
    write_byte(0x01U, GPIO_PIN_RESET);
    HAL_Delay(2U);
}

void lcd16x02_write_line(uint8_t row, const char *text)
{
    if (row > 1U)
    {
        return;
    }

    write_byte((uint8_t)(0x80U | ((row == 0U) ? 0x00U : 0x40U)), GPIO_PIN_RESET);
    for (uint8_t column = 0U; column < 16U; column++)
    {
        char ch = (text != NULL) ? text[column] : '\0';
        write_byte((uint8_t)((ch != '\0') ? ch : ' '), GPIO_PIN_SET);
        if (ch == '\0' && text != NULL)
        {
            text = NULL; /* pad the rest without reading past the terminator */
        }
    }
}
