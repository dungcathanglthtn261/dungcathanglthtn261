/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_types.h"
#include "app_config.h"
#include "soil_sensor.h"
#include "button.h"
#include "relay_pump.h"
#include "lcd16x02.h"
#include "uart_comm.h"
#include "irrigation_control.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
volatile uint32_t g_button_press_count[BUTTON_COUNT] = {0U};
/* Lenh UART = nut ao; chi ISR UART (uart_comm.c) ghi, tach rieng de khong tranh ghi voi task Button. */
volatile uint32_t g_uart_press_count[BUTTON_COUNT] = {0U};
system_data_t g_system_data =
{
    .soil_percent = 0U,
    .setpoint_percent = DEFAULT_SETPOINT,
    .mode = MODE_AUTO,
    .pump_on = false,
    .sensor_valid = false
};
/* USER CODE END Variables */
osThreadId buttonTaskHandle;
osThreadId sensorTaskHandle;
osThreadId controlTaskHandle;
osThreadId displayTaskHandle;
osThreadId uartTaskHandle;
osMutexId systemDataMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartbuttonTask(void const * argument);
void StartsensorTask(void const * argument);
void StartcontrolTask(void const * argument);
void StartdisplayTask(void const * argument);
void StartuartTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of systemDataMutex */
  osMutexDef(systemDataMutex);
  systemDataMutexHandle = osMutexCreate(osMutex(systemDataMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of buttonTask */
  osThreadDef(buttonTask, StartbuttonTask, osPriorityNormal, 0, 128);
  buttonTaskHandle = osThreadCreate(osThread(buttonTask), NULL);

  /* definition and creation of sensorTask */
  osThreadDef(sensorTask, StartsensorTask, osPriorityNormal, 0, 128);
  sensorTaskHandle = osThreadCreate(osThread(sensorTask), NULL);

  /* definition and creation of controlTask */
  osThreadDef(controlTask, StartcontrolTask, osPriorityAboveNormal, 0, 128);
  controlTaskHandle = osThreadCreate(osThread(controlTask), NULL);

  /* definition and creation of displayTask */
  osThreadDef(displayTask, StartdisplayTask, osPriorityBelowNormal, 0, 384);
  displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

  /* definition and creation of uartTask */
  osThreadDef(uartTask, StartuartTask, osPriorityBelowNormal, 0, 384);
  uartTaskHandle = osThreadCreate(osThread(uartTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartbuttonTask */
/**
  * @brief  Function implementing the buttonTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartbuttonTask */
void StartbuttonTask(void const * argument)
{
  /* USER CODE BEGIN StartbuttonTask */
	button_init();
  /* Infinite loop */
  for(;;)
  {
    button_process();

    for (uint32_t i = 0U; i < BUTTON_COUNT; i++)
    {
      if (button_take_press((button_id_t)i))
      {
        g_button_press_count[i]++;
      }
    }

    osDelay(BUTTON_PERIOD_MS);
  }
  /* USER CODE END StartbuttonTask */
}

/* USER CODE BEGIN Header_StartsensorTask */
/**
* @brief Function implementing the sensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartsensorTask */
void StartsensorTask(void const * argument)
{
  /* USER CODE BEGIN StartsensorTask */
  soil_sensor_init();
  /* Infinite loop */
  for(;;)
  {
    uint8_t percent = 0U;
    bool ok = soil_sensor_read_percent(&percent);   /* doc ADC ngoai mutex cho ngan */

    if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
    {
      if (ok) { g_system_data.soil_percent = percent; }
      g_system_data.sensor_valid = ok;
      osMutexRelease(systemDataMutexHandle);
    }

    osDelay(SENSOR_PERIOD_MS);
  }
  /* USER CODE END StartsensorTask */
}

/* USER CODE BEGIN Header_StartcontrolTask */
/**
* @brief Function implementing the controlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartcontrolTask */
void StartcontrolTask(void const * argument)
{
  /* USER CODE BEGIN StartcontrolTask */
  uint32_t handled[BUTTON_COUNT] = {0U};
  uint32_t handled_uart[BUTTON_COUNT] = {0U};

  if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
  {
    irrigation_control_init(&g_system_data);
    osMutexRelease(systemDataMutexHandle);
  }
  /* Infinite loop */
  for(;;)
  {
    if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
    {
      /* So lan nhan do task Button dem; xu ly phan chenh lech. */
      for (uint32_t i = 0U; i < BUTTON_COUNT; i++)
      {
        while (handled[i] != g_button_press_count[i])
        {
          handled[i]++;
          irrigation_control_handle_button(&g_system_data, (button_id_t)i);
        }
        while (handled_uart[i] != g_uart_press_count[i])
        {
          handled_uart[i]++;
          irrigation_control_handle_button(&g_system_data, (button_id_t)i);
        }
      }
      irrigation_control_update(&g_system_data);
      osMutexRelease(systemDataMutexHandle);
    }

    osDelay(CONTROL_PERIOD_MS);
  }
  /* USER CODE END StartcontrolTask */
}

/* USER CODE BEGIN Header_StartdisplayTask */
/**
* @brief Function implementing the displayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartdisplayTask */
void StartdisplayTask(void const * argument)
{
  /* USER CODE BEGIN StartdisplayTask */
  static const char *const MODE_STR[] = { "AUTO", "MAN", "IDLE", "ERR" };   /* theo thu tu system_mode_t */
  char line1[17];
  char line2[17];

  lcd16x02_init();
  lcd16x02_write_line(0U, "Smart Irrigation");
  osDelay(800);
  /* Infinite loop */
  for(;;)
  {
    if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
    {
      system_data_t s = g_system_data;
      osMutexRelease(systemDataMutexHandle);           /* ghi LCD cham, khong giu mutex */

      if (s.sensor_valid)
      {
        snprintf(line1, sizeof line1, "SOIL%3u%% SET%3u%%", s.soil_percent, s.setpoint_percent);
      }
      else
      {
        snprintf(line1, sizeof line1, "SOIL ERR SET%3u%%", s.setpoint_percent);
      }
      snprintf(line2, sizeof line2, "%-4s  PUMP:%-3s",
               MODE_STR[s.mode], s.pump_on ? "ON" : "OFF");
      lcd16x02_write_line(0U, line1);
      lcd16x02_write_line(1U, line2);
    }

    osDelay(DISPLAY_PERIOD_MS);
  }
  /* USER CODE END StartdisplayTask */
}

/* USER CODE BEGIN Header_StartuartTask */
/**
* @brief Function implementing the uartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartuartTask */
void StartuartTask(void const * argument)
{
  /* USER CODE BEGIN StartuartTask */
  /* Infinite loop */
	system_data_t snapshot;

	uart_comm_init();

  for (;;)
  {
	if (osMutexWait(systemDataMutexHandle, osWaitForever) == osOK)
	{
	  snapshot = g_system_data;
	  osMutexRelease(systemDataMutexHandle);

	  uart_comm_send_status(&snapshot);
	}

	osDelay(UART_PERIOD_MS);
  }
  /* USER CODE END StartuartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

