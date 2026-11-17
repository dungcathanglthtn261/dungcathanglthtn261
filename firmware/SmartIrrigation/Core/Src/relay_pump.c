#include "relay_pump.h"
#include "main.h"

/* Relay PA4 kich muc THAP (LOW = bom ON). MX_GPIO_Init() da dat chan len High (bom tat). */

static bool pump_on = false;

void relay_pump_init(void)
{
    relay_pump_set(false);
}

void relay_pump_set(bool on)
{
    pump_on = on;
    HAL_GPIO_WritePin(RELAY_GPIO_Port, RELAY_Pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

bool relay_pump_is_on(void)
{
    return pump_on;
}
