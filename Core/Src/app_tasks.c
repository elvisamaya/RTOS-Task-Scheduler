#include "app_tasks.h"
#include "main.h"
#include <string.h>

osThreadId_t statusLedTaskHandle;
osThreadId_t uartTaskHandle;
osThreadId_t sensorTaskHandle;
osThreadId_t motorTaskHandle;

osMessageQueueId_t sensorQueueHandle;

static UART_HandleTypeDef *g_huart = NULL;
static TIM_HandleTypeDef *g_htim_pwm = NULL;

static void send_text(const char *msg)
{
    HAL_UART_Transmit(g_huart, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void Servo_SetAngle(uint8_t angle)
{
    if (angle > 180)
    {
        angle = 180;
    }

    uint32_t pulse_us = 500 + ((uint32_t)angle * 2000U) / 180U;
    __HAL_TIM_SET_COMPARE(g_htim_pwm, TIM_CHANNEL_1, pulse_us);
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

    for (;;)
    {
        send_text("[UartTask] System alive\r\n");
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
            SensorEvent_t evt = { .triggered = 1 };
            osMessageQueuePut(sensorQueueHandle, &evt, 0, 0);
            send_text("[SensorTask] Button press queued\r\n");
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
                send_text("[MotorTask] Event received\r\n");
                Servo_SetAngle(90);
                osDelay(700);
                Servo_SetAngle(0);
                send_text("[MotorTask] Motion complete\r\n");
            }
        }
    }
}

void AppTasks_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim_pwm)
{
    g_huart = huart;
    g_htim_pwm = htim_pwm;

    sensorQueueHandle = osMessageQueueNew(8, sizeof(SensorEvent_t), NULL);

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

    const osThreadAttr_t motorTaskAttr = {
        .name = "MotorTask",
        .priority = osPriorityHigh,
        .stack_size = 256
    };

    statusLedTaskHandle = osThreadNew(StatusLedTask, NULL, &statusLedTaskAttr);
    uartTaskHandle = osThreadNew(UartTask, NULL, &uartTaskAttr);
    sensorTaskHandle = osThreadNew(SensorTask, NULL, &sensorTaskAttr);
    motorTaskHandle = osThreadNew(MotorTask, NULL, &motorTaskAttr);
}
