#ifndef __TASK_INIT_H
#define __TASK_INIT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* -----------------------------------------------------
 * 任务句柄声明 (Task Handles)
 * ----------------------------------------------------- */

// 注意：这些句柄需要在 main.c 或某个全局可访问的文件中定义
extern TaskHandle_t OLEDHandle;
extern TaskHandle_t MPU6050Handle;
extern TaskHandle_t EncoderHandle;
extern TaskHandle_t ControlHandle;
extern TaskHandle_t BluetoothHandle;


/* -----------------------------------------------------
 * 队列句柄声明 (Queue Handles)
 * ----------------------------------------------------- */

// IMU 数据队列（MPU6050 -> Control）
extern QueueHandle_t xImuQueueHandle;
// 编码器数据队列（Encoder -> Control）
extern QueueHandle_t xEncoderQueueHandle;
// 遥控器指令队列（Bluetooth -> Control）
extern QueueHandle_t xRemoteQueueHandle;
// 状态信息队列（Control -> OLED）
extern QueueHandle_t xStatusQueueHandle;


/* -----------------------------------------------------
 * 函数声明
 * ----------------------------------------------------- */

/**
 * @brief FreeRTOS 任务和资源初始化函数
 */
void Task_Init(void);

#endif /* __TASK_INIT_H */

