#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "cmsis_os2.h"

extern osThreadId_t statusLedTaskHandle;

void AppTasks_Init(void);
void StatusLedTask(void *argument);

#endif /* APP_TASKS_H */
