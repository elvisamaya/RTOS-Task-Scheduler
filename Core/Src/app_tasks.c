#include "app_tasks.h"
#include "main.h"
#include <string.h>

osThreadId_t statusLedTaskHandle;
osThreadId_t uartTaskHandle;
osThreadId_t sensorTaskHandle;

static UART_HandleTypeDef *g_huart = NULL;

static void send_text(const char *msg)
{
    HAL_UART_Transmit(g_huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
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

void UartTask(void *argument)
{
    (void)argument;

    const char *msg = "[UartTask] System alive\r\n";

    for (;;)
    {
        send_text(msg);
        osDelay(2000);
    }
}

void SensorTask(void *argument)
{
    (void)argument;

    uint8_t prev = 0;

    for (;;)
    {
        uint8_t now = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) ? 1U : 0U;

        if (now && !prev)
        {
            send_text("[SensorTask] Button press detected\r\n");
        }

        prev = now;
        osDelay(30);
    }
}

void AppTasks_Init(UART_HandleTypeDef *huart)
{
    g_huart = huart;

    const osThreadAttr_t statusLedTaskAttr = {
        .name = "StatusLedTask",
        .priority = osPriorityLow,
        .stack_size = 256
    };

    const osThreadAttr_t uartTaskAttr = {
        .name = "UartTask",
        .priority = osPriorityNormal,
        .stack_size = 512
    };

    const osThreadAttr_t sensorTaskAttr = {
        .name = "SensorTask",
        .priority = osPriorityHigh,
        .stack_size = 256
    };

    statusLedTaskHandle = osThreadNew(StatusLedTask, NULL, &statusLedTaskAttr);
    uartTaskHandle = osThreadNew(UartTask, NULL, &uartTaskAttr);
    sensorTaskHandle = osThreadNew(SensorTask, NULL, &sensorTaskAttr);
}
