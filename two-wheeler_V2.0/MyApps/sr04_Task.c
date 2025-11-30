#include "FreeRTOS.h" 
#include "task.h"
#include "queue.h"

#include "sr04_task.h"
#include "sr04.h" 
#include "struct.h" 

// 外部句柄和队列声明/定义 (定义在 Task_Init 所在的文件中)
extern QueueHandle_t xSr04QueueHandle; 
extern TaskHandle_t SR04Handle; 

// 外部变量
extern volatile uint32_t uwTickStart; 
extern volatile uint32_t uwTickEnd;   
extern volatile uint8_t  flag_data_ready; 

// 10Hz 任务周期 (100ms)
#define SR04_TASK_FREQUENCY_MS 100 
const TickType_t xFrequency = pdMS_TO_TICKS(SR04_TASK_FREQUENCY_MS);


void SR04_Task(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    Sr04Data_t sr04_data = {-1.0f}; 
    uint32_t cycle_count;
    
    // DWT 驱动中的 CPU 周期数/us
    const float f_cycles_per_us = (float)CYCLES_PER_US;
    
    // 确保 DWT 计时是 100ms 周期性的
    for(;;)
    {
        // ---------------------------------------------------------
        // 1. 发起测量并等待中断
        // ---------------------------------------------------------
        flag_data_ready = 0;
        SR04_Trigger();

        // 等待 EXTI 中断唤醒，最大等待 100ms (xFrequency)
        if (ulTaskNotifyTake(pdTRUE, xFrequency) == 1) 
        {
            // 接收到通知，数据已就绪
            if (flag_data_ready == 1) 
            {
                // 计算周期差
                cycle_count = uwTickEnd - uwTickStart;
                
                // 周期数转换为 us 时间：Time_us = Cycle_Count / Cycles_per_us
                float uwTickTime_us = (float)cycle_count / f_cycles_per_us; 
                
                // 计算距离：Distance = Time_us * 0.017 (cm/us)
                sr04_data.distance_cm = uwTickTime_us * 0.017f; 
            } else {
                // 如果被唤醒但标志未置位，理论上不应发生，设为错误值
                sr04_data.distance_cm = -2.0f;
            }
        } 
        else 
        {
            // 超时未收到数据 (100ms 已过)
            sr04_data.distance_cm = -10.0f; 
        }

        // ---------------------------------------------------------
        // 2. 数据发送
        // ---------------------------------------------------------
        if (xSr04QueueHandle != NULL)
        {
            // 使用覆盖发送，确保 Control Task/OLED Task 总是拿到最新数据
            xQueueOverwrite(xSr04QueueHandle, &sr04_data);
        }

        // ---------------------------------------------------------
        // 3. 任务延时 (强制保持 100ms 周期)
        // ---------------------------------------------------------
        vTaskDelayUntil(&xLastWakeTime, xFrequency); 
    }
}
