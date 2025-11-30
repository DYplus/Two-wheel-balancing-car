#ifndef __SR04_TASK_H
#define __SR04_TASK_H

// 必须包含 FreeRTOS.h
#include "FreeRTOS.h" 
#include "task.h" 
#include "queue.h"


// 任务函数声明
void SR04_Task(void *pvParameters);

// 外部队列句柄声明
extern QueueHandle_t xSr04QueueHandle;

#endif /* __SR04_TASK_H */
