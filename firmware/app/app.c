#include "app.h"
#include "soil_sensor.h"
#include "lcd16x02.h"
#include "relay_pump.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

typedef enum { MODE_AUTO, MODE_MANUAL } sys_mode_t;

/* Du lieu dung chung giua cac task, bao ve bang mutex. relay_* cung goi duoi mutex. */
static struct { uint8_t soil, setpoint; sys_mode_t mode; } sys = {0, 60, MODE_AUTO};
static SemaphoreHandle_t mtx;

#define LOCK()   xSemaphoreTake(mtx, portMAX_DELAY)
#define UNLOCK() xSemaphoreGive(mtx)

/* Task_SensorRead - 1000 ms */
static void task_sensor(void *arg)
{
    TickType_t t = xTaskGetTickCount();
    for (;;) {
        uint8_t s = soil_read_percent();     /* doc ADC ngoai mutex cho ngan */
        LOCK(); sys.soil = s; UNLOCK();
        vTaskDelayUntil(&t, pdMS_TO_TICKS(1000));
    }
}

/* Task_Control - 100 ms */
static void task_control(void *arg)
{
    TickType_t t = xTaskGetTickCount();
    for (;;) {
        LOCK();
        if (sys.mode == MODE_AUTO) relay_set(sys.soil < sys.setpoint);
        UNLOCK();
        vTaskDelayUntil(&t, pdMS_TO_TICKS(100));
    }
}

/* Task_Display - 200 ms */
static void task_display(void *arg)
{
    TickType_t t = xTaskGetTickCount();
    char l1[17], l2[17];
    for (;;) {
        LOCK();
        snprintf(l1, sizeof l1, "SOIL%3u%% SET%3u%%", sys.soil, sys.setpoint);
        snprintf(l2, sizeof l2, "%-4s  PUMP:%-3s", sys.mode == MODE_AUTO ? "AUTO" : "MAN",
                 relay_is_on() ? "ON" : "OFF");
        UNLOCK();                                           /* ghi LCD cham, khong giu mutex */
        lcd_print_line(0, l1);
        lcd_print_line(1, l2);
        vTaskDelayUntil(&t, pdMS_TO_TICKS(200));
    }
}

void app_init(void)
{
    relay_init();                 /* dau tien: dam bao bom tat */
    soil_init();
    lcd_init();
    lcd_print_line(0, "Smart Irrigation");
    HAL_Delay(800);               /* truoc scheduler nen HAL_Delay van chay */
    sys.soil = soil_read_percent();

    mtx = xSemaphoreCreateMutex();
    /* uu tien: dieu khien cao hon hien thi. Stack tinh theo word. */
    xTaskCreate(task_control, "ctl",  192, NULL, 3, NULL);
    xTaskCreate(task_sensor,  "adc",  192, NULL, 2, NULL);
    xTaskCreate(task_display, "lcd",  256, NULL, 1, NULL);
}

void app_start(void) { vTaskStartScheduler(); }
