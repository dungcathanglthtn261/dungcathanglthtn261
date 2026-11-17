#include "relay_pump.h"

#include "main.h"

static bool pump_on = false;

void relay_pump_init(void)
{
    /* CubeMX configures PA4 as output and drives it high before the scheduler. */
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, GPIO_PIN_SET);
    pump_on = false;
}

void relay_pump_set(bool on)
{
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin,
                      on ? GPIO_PIN_RESET : GPIO_PIN_SET);
    pump_on = on;
}

bool relay_pump_is_on(void)
{
    return pump_on;
}
