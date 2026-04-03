#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct
{
    uint8_t triggered;
} SensorEvent_t;

extern osThreadId_t statusLedTaskHandle;
extern osThreadId_t uartTaskHandle;
extern osThreadId_t sensorTaskHandle;
extern osThreadId_t motorTaskHandle;

extern osMessageQueueId_t sensorQueueHandle;

void AppTasks_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim_pwm);
void StatusLedTask(void *argument);
void UartTask(void *argument);
void SensorTask(void *argument);
void MotorTask(void *argument);
void Servo_SetAngle(uint8_t angle);

#endif /* APP_TASKS_H */
