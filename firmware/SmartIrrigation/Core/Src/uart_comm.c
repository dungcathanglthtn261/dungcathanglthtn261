#include "uart_comm.h"
#include "usart.h"

#include <stdio.h>
#include <stddef.h>

#define UART_TX_BUFFER_SIZE 96U

static const char *const MODE_NAME[] = { "AUTO", "MANUAL", "IDLE", "ERROR" };   /* theo thu tu system_mode_t */

static char tx_buffer[UART_TX_BUFFER_SIZE];
static volatile bool tx_busy = false;
static uint8_t rx_byte;

/* Dinh nghia trong freertos.c; task Control xu ly nhu so lan nhan nut. */
extern volatile uint32_t g_uart_press_count[BUTTON_COUNT];

/* Lenh 1 ky tu: m = MODE, p = PUMP, + = UP, - = DOWN. */
static void uart_comm_start_rx(void)
{
    (void)HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
}

void uart_comm_init(void)
{
    /* USART2 da duoc khoi tao trong main.c.
       Ham nay chi goi mot lan, truoc khi gui du lieu. */
    tx_busy = false;
    uart_comm_start_rx();
}

void uart_comm_send_status(const system_data_t *data)
{
    if ((data == NULL) || tx_busy)
    {
        return;
    }

    int length = snprintf(
        tx_buffer,
        sizeof(tx_buffer),
        "SOIL: %u%% | SET: %u%% | MODE: %s | PUMP: %s\r\n",
        (unsigned int)data->soil_percent,
        (unsigned int)data->setpoint_percent,
        MODE_NAME[data->mode],
        data->pump_on ? "ON" : "OFF"
    );

    if ((length <= 0) || ((size_t)length >= sizeof(tx_buffer)))
    {
        return;
    }

    tx_busy = true;

    HAL_StatusTypeDef result = HAL_UART_Transmit_IT(
        &huart2,
        (uint8_t *)tx_buffer,
        (uint16_t)length
    );

    if (result != HAL_OK)
    {
        tx_busy = false;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        tx_busy = false;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        /* Release our static TX buffer after a HAL transfer error. */
        tx_busy = false;
        /* HAL dung nhan khi loi (vd overrun): nhan lai. */
        uart_comm_start_rx();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->Instance != USART2))
    {
        return;
    }

    switch (rx_byte)
    {
        case 'm': case 'M': g_uart_press_count[BUTTON_MODE]++; break;
        case 'p': case 'P': g_uart_press_count[BUTTON_PUMP]++; break;
        case '+':           g_uart_press_count[BUTTON_UP]++;   break;
        case '-':           g_uart_press_count[BUTTON_DOWN]++; break;
        default:                                               break;
    }
    uart_comm_start_rx();
}
