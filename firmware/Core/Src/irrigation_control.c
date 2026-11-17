#include "irrigation_control.h"
#include "app_config.h"
#include "relay_pump.h"

#define SETPOINT_STEP  5U
#define SETPOINT_MIN   10U
#define SETPOINT_MAX   90U

/* Tat ca ham duoi day goi khi dang giu systemDataMutex. */

void irrigation_control_init(system_data_t *data)
{
    relay_pump_init();
    data->pump_on = false;
}

void irrigation_control_handle_button(system_data_t *data, button_id_t button)
{
    switch (button)
    {
    case BUTTON_MODE:
        data->mode = (data->mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
        break;
    case BUTTON_UP:
        if (data->setpoint_percent + SETPOINT_STEP <= SETPOINT_MAX) { data->setpoint_percent += SETPOINT_STEP; }
        break;
    case BUTTON_DOWN:
        if (data->setpoint_percent >= SETPOINT_MIN + SETPOINT_STEP) { data->setpoint_percent -= SETPOINT_STEP; }
        break;
    case BUTTON_PUMP:
        if (data->mode == MODE_MANUAL) { data->pump_on = !data->pump_on; }   /* AUTO: bo qua */
        break;
    default:
        break;
    }
}

void irrigation_control_update(system_data_t *data)
{
    if (data->mode == MODE_AUTO)
    {
        /* Chua co du lieu cam bien hop le thi khong tuoi. */
        data->pump_on = data->sensor_valid && (data->soil_percent < data->setpoint_percent);
    }
    relay_pump_set(data->pump_on);
}
