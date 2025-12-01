#ifndef __AT24C02_TASK_H
#define __AT24C02_TASK_H

#include "FreeRTOS.h"
#include "task.h"

void AT24C64_Task(void *pvParameters);

// 声明任务句柄（可选）
extern TaskHandle_t AT24C64Handle;

#endif
