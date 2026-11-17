#include "soil_sensor.h"
#include "app_config.h"
#include "adc.h"
#include <stddef.h>

/* Cam bien dien dung: KHO -> ADC CAO. hadc1 da duoc MX_ADC1_Init() cau hinh (PA0, ADC1_IN0). */

void soil_sensor_init(void)
{
    (void)HAL_ADCEx_Calibration_Start(&hadc1);
}

bool soil_sensor_read_percent(uint8_t *percent)
{
    if (percent == NULL)
    {
        return false;
    }

    uint32_t sum = 0U;
    for (uint32_t i = 0U; i < SOIL_AVERAGE_SAMPLES; i++)
    {
        if ((HAL_ADC_Start(&hadc1) != HAL_OK) ||
            (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK))
        {
            (void)HAL_ADC_Stop(&hadc1);
            return false;
        }
        sum += HAL_ADC_GetValue(&hadc1);
        (void)HAL_ADC_Stop(&hadc1);
    }

    int32_t raw = (int32_t)(sum / SOIL_AVERAGE_SAMPLES);
    int32_t pct = (raw - (int32_t)SOIL_ADC_DRY) * 100 /
                  ((int32_t)SOIL_ADC_WET - (int32_t)SOIL_ADC_DRY);

    if (pct < 0)   { pct = 0; }
    if (pct > 100) { pct = 100; }

    *percent = (uint8_t)pct;
    return true;
}
