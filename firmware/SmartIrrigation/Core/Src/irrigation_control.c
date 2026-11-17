#include "irrigation_control.h"

#include "relay_pump.h"

#include <stddef.h>

void irrigation_control_init(system_data_t *data)
{
    if (data == NULL)
    {
        return;
    }

    data->pump_on = false;
    if (data->setpoint_percent > 100U)
    {
        data->setpoint_percent = 100U;
    }
    relay_pump_set(false);
}

void irrigation_control_handle_button(system_data_t *data, button_id_t button)
{
    if (data == NULL)
    {
        return;
    }

    switch (button)
    {
        case BUTTON_MODE:
            data->mode = (data->mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
            data->pump_on = false;
            relay_pump_set(false);
            break;

        case BUTTON_UP:
            if (data->setpoint_percent < 100U)
            {
                data->setpoint_percent++;
            }
            break;

        case BUTTON_DOWN:
            if (data->setpoint_percent > 0U)
            {
                data->setpoint_percent--;
            }
            break;

        case BUTTON_PUMP:
            if (data->mode == MODE_MANUAL)
            {
                data->pump_on = !data->pump_on;
                relay_pump_set(data->pump_on);
            }
            break;

        default:
            break;
    }
}

void irrigation_control_update(system_data_t *data)
{
    if ((data == NULL) || (data->mode != MODE_AUTO))
    {
        return;
    }

    bool should_run = data->sensor_valid &&
                      (data->soil_percent < data->setpoint_percent);
    if (data->pump_on != should_run)
    {
        relay_pump_set(should_run);
        data->pump_on = should_run;
    }
}
