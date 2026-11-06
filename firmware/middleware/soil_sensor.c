#include "soil_sensor.h"
#include "stm32f1xx_hal.h"

/* shortcut: gia tri mau, phai do lai theo cam bien that (xem docs) */
#define ADC_DRY 3000
#define ADC_WET 1300
#define N_SAMPLES 8

static ADC_HandleTypeDef hadc;

void soil_init(void)
{
    RCC_PeriphCLKInitTypeDef clk = {0};
    clk.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    clk.AdcClockSelection = RCC_ADCPCLK2_DIV6;      /* 72/6 = 12 MHz <= 14 */
    HAL_RCCEx_PeriphCLKConfig(&clk);

    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef g = {.Pin = GPIO_PIN_0, .Mode = GPIO_MODE_ANALOG};
    HAL_GPIO_Init(GPIOA, &g);

    hadc.Instance = ADC1;
    hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 1;
    HAL_ADC_Init(&hadc);
    HAL_ADCEx_Calibration_Start(&hadc);

    ADC_ChannelConfTypeDef ch = {.Channel = ADC_CHANNEL_0, .Rank = ADC_REGULAR_RANK_1,
                                 .SamplingTime = ADC_SAMPLETIME_55CYCLES_5};
    HAL_ADC_ConfigChannel(&hadc, &ch);
}

uint16_t soil_read_raw(void)
{
    uint32_t sum = 0;
    for (int i = 0; i < N_SAMPLES; i++) {
        HAL_ADC_Start(&hadc);
        HAL_ADC_PollForConversion(&hadc, 10);
        sum += HAL_ADC_GetValue(&hadc);
        HAL_ADC_Stop(&hadc);
    }
    return (uint16_t)(sum / N_SAMPLES);
}

uint8_t soil_read_percent(void)
{
    long pct = ((long)soil_read_raw() - ADC_DRY) * 100 / (ADC_WET - ADC_DRY);
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    return (uint8_t)pct;
}
