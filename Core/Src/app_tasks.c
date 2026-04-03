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

static void process_command(char *cmd)
{
    if (strcmp(cmd, "led off") == 0)
    {
        if (statusLedTaskHandle != NULL)
        {
            osThreadSuspend(statusLedTaskHandle);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            send_text("[UartTask] LED task suspended\r\n");
        }
    }
    else if (strcmp(cmd, "led on") == 0)
    {
        if (statusLedTaskHandle != NULL)
        {
            osThreadResume(statusLedTaskHandle);
            send_text("[UartTask] LED task resumed\r\n");
        }
    }
    else if (strcmp(cmd, "led kill") == 0)
    {
        if (statusLedTaskHandle != NULL)
        {
            osThreadTerminate(statusLedTaskHandle);
            statusLedTaskHandle = NULL;
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            send_text("[UartTask] LED task terminated\r\n");
        }
    }
    else if (strcmp(cmd, "motor open") == 0)
    {
        Servo_SetAngle(90);
        send_text("[UartTask] Servo open\r\n");
    }
    else if (strcmp(cmd, "motor close") == 0)
    {
        Servo_SetAngle(0);
        send_text("[UartTask] Servo close\r\n");
    }
    else
    {
        send_text("[UartTask] Unknown command\r\n");
    }
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

    char rx_line[32];
    uint32_t idx = 0;
    uint8_t ch;

    memset(rx_line, 0, sizeof(rx_line));
    send_text("Commands: led off, led on, led kill, motor open, motor close\r\n");

    for (;;)
    {
        if (HAL_UART_Receive(g_huart, &ch, 1, 10) == HAL_OK)
        {
            HAL_UART_Transmit(g_huart, &ch, 1, HAL_MAX_DELAY);

            if (ch == '\r' || ch == '\n')
            {
                HAL_UART_Transmit(g_huart, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);

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
        .stack_size = 768
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
