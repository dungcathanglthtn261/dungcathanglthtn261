#include "irrigation_control.h"
#include "app_config.h"
#include "relay_pump.h"

#define SETPOINT_STEP  5U
#define SETPOINT_MIN   10U
#define SETPOINT_MAX   90U
#define HYSTERESIS      5U   /* +/-5% quanh setpoint, chong relay dong/ngat lien tuc */

/* Tat ca ham duoi day goi khi dang giu systemDataMutex. */

/* Chuyen trang thai; doi trang thai nao cung tat bom (giong sketch.ino). */
static void enter_state(system_data_t *data, system_mode_t state)
{
    data->mode = state;
    data->pump_on = false;
}

void irrigation_control_init(system_data_t *data)
{
    relay_pump_init();
    enter_state(data, MODE_IDLE);
}

void irrigation_control_handle_button(system_data_t *data, button_id_t button)
{
    switch (button)
    {
    case BUTTON_MODE:
        /* IDLE/AUTO/ERROR -> MANUAL; MANUAL -> AUTO */
        enter_state(data, (data->mode == MODE_MANUAL) ? MODE_AUTO : MODE_MANUAL);
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
    switch (data->mode)
    {
    case MODE_IDLE:
        if (data->sensor_valid) { enter_state(data, MODE_AUTO); }
        break;

    case MODE_AUTO:
        if (!data->sensor_valid)
        {
            enter_state(data, MODE_ERROR);
        }
        else if (data->soil_percent < data->setpoint_percent - HYSTERESIS)   /* setpoint >= 10 nen khong am */
        {
            data->pump_on = true;
        }
        else if (data->soil_percent >= data->setpoint_percent + HYSTERESIS)
        {
            data->pump_on = false;
        }
        /* trong dai [SET-H, SET+H): giu nguyen trang thai cu */
        break;

    case MODE_ERROR:
        if (data->sensor_valid) { enter_state(data, MODE_AUTO); }
        break;

    case MODE_MANUAL:
    default:
        break;                          /* bom do nguoi dung bat/tat */
    }
    relay_pump_set(data->pump_on);
}
