#include "app_tasks.h"
#include "main.h"
#include <string.h>

osThreadId_t statusLedTaskHandle;
osThreadId_t uartTaskHandle;

static UART_HandleTypeDef *g_huart = NULL;

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
        HAL_UART_Transmit(g_huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
        osDelay(1000);
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

    statusLedTaskHandle = osThreadNew(StatusLedTask, NULL, &statusLedTaskAttr);
    uartTaskHandle = osThreadNew(UartTask, NULL, &uartTaskAttr);
}
