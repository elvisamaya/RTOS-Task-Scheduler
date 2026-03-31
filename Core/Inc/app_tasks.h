#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

extern osThreadId_t statusLedTaskHandle;
extern osThreadId_t uartTaskHandle;

void AppTasks_Init(UART_HandleTypeDef *huart);
void StatusLedTask(void *argument);
void UartTask(void *argument);

#endif /* APP_TASKS_H */
