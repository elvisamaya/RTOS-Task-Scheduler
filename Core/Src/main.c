#include "main.h"
#include "cmsis_os2.h"
#include "app_tasks.h"

int main(void)
{
    HAL_Init();

    osKernelInitialize();
    AppTasks_Init();
    osKernelStart();

    while (1)
    {
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
