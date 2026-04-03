#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef struct
{
    uint8_t triggered;
} SensorEvent_t;

typedef struct
{
    UART_HandleTypeDef *huart;
    TIM_HandleTypeDef *htim_pwm;
    uint32_t pwm_channel;
} AppContext_t;

extern osThreadId_t sensorTaskHandle;
extern osThreadId_t motorTaskHandle;
extern osThreadId_t uartTaskHandle;
extern osThreadId_t statusLedTaskHandle;

extern osMessageQueueId_t sensorQueueHandle;
extern osMutexId_t uartMutexHandle;

void AppTasks_Init(AppContext_t *ctx);

void SensorTask(void *argument);
void MotorTask(void *argument);
void UartTask(void *argument);
void StatusLedTask(void *argument);

void App_Print(const char *fmt, ...);
void Servo_SetAngle(uint8_t angle);

#endif /* APP_TASKS_H */
