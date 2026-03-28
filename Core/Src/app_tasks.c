#include "app_tasks.h"
#include "main.h"

osThreadId_t statusLedTaskHandle;

void StatusLedTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        osDelay(500);
    }
}

void AppTasks_Init(void)
{
    const osThreadAttr_t statusLedTaskAttr = {
        .name = "StatusLedTask",
        .priority = osPriorityLow,
        .stack_size = 256
    };

    statusLedTaskHandle = osThreadNew(StatusLedTask, NULL, &statusLedTaskAttr);
}
