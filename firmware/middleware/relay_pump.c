#include "relay_pump.h"
#include "stm32f1xx_hal.h"

static bool s_on;

void relay_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   /* ghi truoc de khong nhay relay */
    GPIO_InitTypeDef g = {.Pin = GPIO_PIN_4, .Mode = GPIO_MODE_OUTPUT_PP, .Speed = GPIO_SPEED_FREQ_LOW};
    HAL_GPIO_Init(GPIOA, &g);
    s_on = false;
}

void relay_set(bool on)
{
    s_on = on;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

bool relay_is_on(void) { return s_on; }
