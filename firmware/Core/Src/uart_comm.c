#include "uart_comm.h"
#include "usart.h"

#include <stdio.h>
#include <stddef.h>

#define UART_TX_BUFFER_SIZE 96U

static char tx_buffer[UART_TX_BUFFER_SIZE];
static volatile bool tx_busy = false;

void uart_comm_init(void)
{
    /* USART2 da duoc khoi tao trong main.c.
       Ham nay chi goi mot lan, truoc khi gui du lieu. */
    tx_busy = false;
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
        "SOIL: %u%% | SET: %u%% | MODE: %s | PUMP: %s | VALID: %u\r\n",
        (unsigned int)data->soil_percent,
        (unsigned int)data->setpoint_percent,
        data->mode == MODE_AUTO ? "AUTO" : "MANUAL",
        data->pump_on ? "ON" : "OFF",
        data->sensor_valid ? 1U : 0U
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
    if (huart->Instance == USART2)
    {
        tx_busy = false;
    }
}
