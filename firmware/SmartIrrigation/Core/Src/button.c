#include "button.h"
#include "app_config.h"
#include "main.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState stable;
    uint8_t count;
    bool pending_press;
} button_state_t;

static button_state_t buttons[BUTTON_COUNT] =
{
    [BUTTON_MODE] =
    {
        .port = BTN_MODE_GPIO_Port,
        .pin  = BTN_MODE_Pin
    },
    [BUTTON_UP] =
    {
        .port = BTN_UP_GPIO_Port,
        .pin  = BTN_UP_Pin
    },
    [BUTTON_DOWN] =
    {
        .port = BTN_DOWN_GPIO_Port,
        .pin  = BTN_DOWN_Pin
    },
    [BUTTON_PUMP] =
    {
        .port = BTN_PUMP_GPIO_Port,
        .pin  = BTN_PUMP_Pin
    }
};

void button_init(void)
{
    for (uint32_t i = 0U; i < BUTTON_COUNT; i++)
    {
        buttons[i].stable =
            HAL_GPIO_ReadPin(buttons[i].port, buttons[i].pin);

        buttons[i].count = 0U;
        buttons[i].pending_press = false;
    }
}

void button_process(void)
{
    for (uint32_t i = 0U; i < BUTTON_COUNT; i++)
    {
        button_state_t *button = &buttons[i];

        GPIO_PinState current =
            HAL_GPIO_ReadPin(button->port, button->pin);

        if (current == button->stable)
        {
            button->count = 0U;
            continue;
        }

        button->count++;

        if (button->count >= BUTTON_DEBOUNCE_SAMPLES)
        {
            button->stable = current;
            button->count = 0U;

            /* Pull-up: nhan nut tuong ung muc LOW. */
            if (current == GPIO_PIN_RESET)
            {
                button->pending_press = true;
            }
        }
    }
}

bool button_take_press(button_id_t id)
{
    if ((unsigned int)id >= (unsigned int)BUTTON_COUNT)
    {
        return false;
    }

    bool pressed = buttons[id].pending_press;
    buttons[id].pending_press = false;

    return pressed;
}
