#include "app_tasks.h"
#include "main.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

osThreadId_t sensorTaskHandle;
osThreadId_t motorTaskHandle;
osThreadId_t uartTaskHandle;
osThreadId_t statusLedTaskHandle;

osMessageQueueId_t sensorQueueHandle;
osMutexId_t uartMutexHandle;

static AppContext_t g_app;
static volatile uint8_t g_servo_angle = 0;

static void process_command(char *cmd);
static void uart_send_string(const char *s);
static uint8_t button_pressed(void);

void AppTasks_Init(AppContext_t *ctx)
{
    g_app = *ctx;

    uartMutexHandle = osMutexNew(NULL);
    sensorQueueHandle = osMessageQueueNew(8, sizeof(SensorEvent_t), NULL);

    const osThreadAttr_t sensorTaskAttr = {
        .name = "SensorTask",
        .priority = osPriorityHigh,
        .stack_size = 512
    };

    const osThreadAttr_t motorTaskAttr = {
        .name = "MotorTask",
        .priority = osPriorityHigh,
        .stack_size = 512
    };

    const osThreadAttr_t uartTaskAttr = {
        .name = "UartTask",
        .priority = osPriorityNormal,
        .stack_size = 1024
    };

    const osThreadAttr_t statusLedTaskAttr = {
        .name = "StatusLedTask",
        .priority = osPriorityLow,
        .stack_size = 512
    };

    sensorTaskHandle = osThreadNew(SensorTask, NULL, &sensorTaskAttr);
    motorTaskHandle = osThreadNew(MotorTask, NULL, &motorTaskAttr);
    uartTaskHandle = osThreadNew(UartTask, NULL, &uartTaskAttr);
    statusLedTaskHandle = osThreadNew(StatusLedTask, NULL, &statusLedTaskAttr);

    App_Print("\r\n=== FreeRTOS Scheduler Demo ===\r\n");
    App_Print("Commands: help, status, led off, led on, led kill, motor open, motor close\r\n");
}

void SensorTask(void *argument)
{
    (void)argument;

    uint8_t prev = 0;

    for (;;)
    {
        uint8_t now = button_pressed();

        if (now && !prev)
        {
            SensorEvent_t evt = { .triggered = 1 };
            osMessageQueuePut(sensorQueueHandle, &evt, 0, 0);
            App_Print("[SensorTask] Button press detected -> queued motor event\r\n");
        }

        prev = now;
        osDelay(30);
    }
}

void MotorTask(void *argument)
{
    (void)argument;

    SensorEvent_t evt;

    Servo_SetAngle(0);

    for (;;)
    {
        if (osMessageQueueGet(sensorQueueHandle, &evt, NULL, osWaitForever) == osOK)
        {
            if (evt.triggered)
            {
                App_Print("[MotorTask] Executing motor action\r\n");
                Servo_SetAngle(90);
                osDelay(700);
                Servo_SetAngle(0);
                App_Print("[MotorTask] Motor action complete\r\n");
            }
        }
    }
}

void UartTask(void *argument)
{
    (void)argument;

    char rx_line[64];
    uint32_t idx = 0;
    uint8_t ch;

    memset(rx_line, 0, sizeof(rx_line));

    for (;;)
    {
        if (HAL_UART_Receive(g_app.huart, &ch, 1, 10) == HAL_OK)
        {
            HAL_UART_Transmit(g_app.huart, &ch, 1, HAL_MAX_DELAY);

            if (ch == '\r' || ch == '\n')
            {
                HAL_UART_Transmit(g_app.huart, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);

                if (idx > 0)
                {
                    rx_line[idx] = '\0';
                    process_command(rx_line);
                    idx = 0;
                    memset(rx_line, 0, sizeof(rx_line));
                }
            }
            else if (idx < sizeof(rx_line) - 1)
            {
                rx_line[idx++] = (char)ch;
            }
        }

        osDelay(10);
    }
}

void StatusLedTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        osDelay(500);
    }
}

void App_Print(const char *fmt, ...)
{
    char buffer[256];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (uartMutexHandle != NULL)
    {
        osMutexAcquire(uartMutexHandle, osWaitForever);
        uart_send_string(buffer);
        osMutexRelease(uartMutexHandle);
    }
    else
    {
        uart_send_string(buffer);
    }
}

void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180)
    {
        angle = 180;
    }

    g_servo_angle = angle;

    uint32_t pulse_us = 500 + ((uint32_t)angle * 2000U) / 180U;
    __HAL_TIM_SET_COMPARE(g_app.htim_pwm, g_app.pwm_channel, pulse_us);
}

static void process_command(char *cmd)
{
    if (strcmp(cmd, "help") == 0)
    {
        App_Print("Commands:\r\n");
        App_Print("  help\r\n");
        App_Print("  status\r\n");
        App_Print("  led off\r\n");
        App_Print("  led on\r\n");
        App_Print("  led kill\r\n");
        App_Print("  motor open\r\n");
        App_Print("  motor close\r\n");
    }
    else if (strcmp(cmd, "status") == 0)
    {
        App_Print("System status:\r\n");
        App_Print("  Servo angle: %u\r\n", g_servo_angle);
        App_Print("  LED task alive: %s\r\n", statusLedTaskHandle ? "YES" : "NO");
    }
    else if (strcmp(cmd, "led off") == 0)
    {
        if (statusLedTaskHandle != NULL)
        {
            osThreadSuspend(statusLedTaskHandle);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            App_Print("[UartTask] Status LED task suspended\r\n");
        }
    }
    else if (strcmp(cmd, "led on") == 0)
    {
        if (statusLedTaskHandle != NULL)
        {
            osThreadResume(statusLedTaskHandle);
            App_Print("[UartTask] Status LED task resumed\r\n");
        }
    }
    else if (strcmp(cmd, "led kill") == 0)
    {
        if (statusLedTaskHandle != NULL)
        {
            osThreadTerminate(statusLedTaskHandle);
            statusLedTaskHandle = NULL;
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            App_Print("[UartTask] Status LED task terminated\r\n");
        }
    }
    else if (strcmp(cmd, "motor open") == 0)
    {
        Servo_SetAngle(90);
        App_Print("[UartTask] Servo moved to open position\r\n");
    }
    else if (strcmp(cmd, "motor close") == 0)
    {
        Servo_SetAngle(0);
        App_Print("[UartTask] Servo moved to closed position\r\n");
    }
    else
    {
        App_Print("Unknown command: %s\r\n", cmd);
    }
}

static void uart_send_string(const char *s)
{
    HAL_UART_Transmit(g_app.huart, (uint8_t *)s, strlen(s), HAL_MAX_DELAY);
}

static uint8_t button_pressed(void)
{
    return (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) ? 1U : 0U;
}
