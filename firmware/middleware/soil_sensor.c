#include "soil_sensor.h"

#include "adc.h"
#include "app_config.h"

#define SOIL_ADC_TIMEOUT_MS 10U

void soil_sensor_init(void)
{
    /* hadc1/channel 0 and PA0 are initialized once by CubeMX in main(). */
}

bool soil_sensor_read_percent(uint8_t *percent)
{
    if ((percent == NULL) || (hadc1.Instance != ADC1) ||
        (SOIL_ADC_DRY == SOIL_ADC_WET))
    {
        return false;
    }

    uint32_t sum = 0U;
    for (uint32_t sample = 0U; sample < SOIL_AVERAGE_SAMPLES; sample++)
    {
        if (HAL_ADC_Start(&hadc1) != HAL_OK)
        {
            (void)HAL_ADC_Stop(&hadc1);
            return false;
        }

        if (HAL_ADC_PollForConversion(&hadc1, SOIL_ADC_TIMEOUT_MS) != HAL_OK)
        {
            (void)HAL_ADC_Stop(&hadc1);
            return false;
        }

        sum += HAL_ADC_GetValue(&hadc1);
        if (HAL_ADC_Stop(&hadc1) != HAL_OK)
        {
            return false;
        }
    }

    uint32_t raw = sum / SOIL_AVERAGE_SAMPLES;
    int32_t numerator = ((int32_t)raw - (int32_t)SOIL_ADC_DRY) * 100;
    int32_t denominator = (int32_t)SOIL_ADC_WET - (int32_t)SOIL_ADC_DRY;
    int32_t mapped = numerator / denominator;

    if (mapped < 0)
    {
        mapped = 0;
    }
    else if (mapped > 100)
    {
        mapped = 100;
    }

    *percent = (uint8_t)mapped;
    return true;
}
