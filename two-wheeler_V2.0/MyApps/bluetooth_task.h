#ifndef __BLUETOOTH_TASK_H
#define __BLUETOOTH_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* -----------------------------------------------------
 * 函数声明
 * ----------------------------------------------------- */

/**
 * @brief 蓝牙通信任务
 * @param pvParameters 任务创建时传入的参数 (通常为 NULL)
 */
void Bluetooth_Task(void *pvParameters);


/* -----------------------------------------------------
 * 外部队列句柄声明
 * ----------------------------------------------------- */

// 声明外部的遥控器指令队列，用于向其他任务发送接收到的指令
// 假设此句柄在 task_init.c 中定义
extern QueueHandle_t xRemoteQueueHandle;

// 如果需要，可以在这里声明其他的蓝牙相关变量或函数

#endif /* __BLUETOOTH_TASK_H */

