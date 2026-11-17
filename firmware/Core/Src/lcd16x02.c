#include "lcd16x02.h"
#include "main.h"
#include "cmsis_os.h"

/* HD44780 16x2, che do 4-bit, RW noi GND. Chan da duoc MX_GPIO_Init() cau hinh output. */

static void delay_us(uint32_t us)     /* DWT: HAL_Delay chi co do phan giai 1 ms */
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks) {}
}

static void pulse(void)
{
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);   delay_us(2U);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET); delay_us(50U);
}

static void write4(uint8_t n)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (n & 1U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (n & 2U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (n & 4U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (n & 8U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    pulse();
}

static void write8(uint8_t v, GPIO_PinState rs)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs);
    write4(v >> 4);
    write4(v & 0x0FU);
}

/* Goi trong task (sau khi scheduler chay) nen dung osDelay, khong dung HAL_Delay. */
void lcd16x02_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    osDelay(50);
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    write4(0x3U); osDelay(5);
    write4(0x3U); delay_us(200U);
    write4(0x3U); delay_us(200U);
    write4(0x2U);                       /* chuyen sang 4-bit */
    write8(0x28U, GPIO_PIN_RESET);      /* 4-bit, 2 dong, 5x8 */
    write8(0x0CU, GPIO_PIN_RESET);      /* display ON, khong con tro */
    write8(0x06U, GPIO_PIN_RESET);      /* tang dia chi */
    write8(0x01U, GPIO_PIN_RESET);      /* xoa man hinh */
    osDelay(2);
}

void lcd16x02_write_line(uint8_t row, const char *text)
{
    write8(0x80U | (row ? 0x40U : 0x00U), GPIO_PIN_RESET);
    for (uint32_t i = 0U; i < 16U; i++)
    {
        char c = ((text != NULL) && (*text != '\0')) ? *text++ : ' ';
        write8((uint8_t)c, GPIO_PIN_SET);
    }
}
