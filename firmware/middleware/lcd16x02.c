#include "lcd16x02.h"
#include "stm32f1xx_hal.h"

#define RS GPIO_PIN_0
#define EN GPIO_PIN_1
#define D4 GPIO_PIN_10
#define D5 GPIO_PIN_11
#define D6 GPIO_PIN_12
#define D7 GPIO_PIN_13

static void delay_us(uint32_t us)     /* DWT: HAL_Delay chi co do phan giai 1 ms */
{
    uint32_t start = DWT->CYCCNT, ticks = us * (SystemCoreClock / 1000000U);
    while (DWT->CYCCNT - start < ticks) {}
}

static void pulse(void)
{
    HAL_GPIO_WritePin(GPIOB, EN, GPIO_PIN_SET);   delay_us(2);
    HAL_GPIO_WritePin(GPIOB, EN, GPIO_PIN_RESET); delay_us(50);
}

static void write4(uint8_t n)
{
    HAL_GPIO_WritePin(GPIOB, D4, (n & 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, D5, (n & 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, D6, (n & 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, D7, (n & 8) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    pulse();
}

static void write8(uint8_t v, int rs)
{
    HAL_GPIO_WritePin(GPIOB, RS, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
    write4(v >> 4);
    write4(v & 0x0F);
}

void lcd_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef g = {.Pin = RS | EN | D4 | D5 | D6 | D7,
                          .Mode = GPIO_MODE_OUTPUT_PP, .Speed = GPIO_SPEED_FREQ_LOW};
    HAL_GPIO_Init(GPIOB, &g);

    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOB, RS, GPIO_PIN_RESET);
    write4(0x3); HAL_Delay(5);
    write4(0x3); delay_us(200);
    write4(0x3); delay_us(200);
    write4(0x2);                     /* chuyen sang 4-bit */
    write8(0x28, 0);                 /* 4-bit, 2 dong, 5x8 */
    write8(0x0C, 0);                 /* display ON, khong con tro */
    write8(0x06, 0);                 /* tang dia chi */
    lcd_clear();
}

void lcd_clear(void) { write8(0x01, 0); HAL_Delay(2); }

void lcd_print_line(unsigned row, const char *s)
{
    write8(0x80 | (row ? 0x40 : 0x00), 0);
    for (int i = 0; i < 16; i++)
        write8(*s ? (uint8_t)*s++ : ' ', 1);
}
