#include "stm32f1xx_hal.h"
#include "app.h"

/* shortcut: FreeRTOSConfig.h + SysTick/PendSV/SVC handler lay tu project Cube (bat FreeRTOS, TIMx lam HAL timebase).
 * HSE 8 MHz -> PLL x9 = 72 MHz (Blue Pill). Bo file nay neu dung project STM32CubeIDE. */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef o = {0};
    o.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    o.HSEState = RCC_HSE_ON;
    o.PLL.PLLState = RCC_PLL_ON;
    o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    o.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&o);

    RCC_ClkInitTypeDef c = {0};
    c.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    c.AHBCLKDivider = RCC_SYSCLK_DIV1;
    c.APB1CLKDivider = RCC_HCLK_DIV2;
    c.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&c, FLASH_LATENCY_2);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    app_init();
    app_start();
    for (;;) {}
}
