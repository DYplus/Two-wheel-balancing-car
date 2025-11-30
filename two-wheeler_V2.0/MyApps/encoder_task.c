#include "encoder_task.h"
#include "struct.h"      
#include "encoder.h"     

// 引入 FreeRTOS 原生头文件
#include "FreeRTOS.h" 
#include "task.h"      // 包含 vTaskDelayUntil
#include "queue.h"     // 包含 QueueHandle_t, xQueueOverwrite
#include "tim.h"       // 假设 TIM_HandleTypeDef 定义在这里 (用于 HAL_TIM_Encoder_Start)


// 外部声明定时器句柄 (假设这些是在某个初始化文件中定义的)
extern TIM_HandleTypeDef htim2; 
extern TIM_HandleTypeDef htim4; 

// 外部声明消息队列句柄 (需要在创建队列的文件中定义)
extern QueueHandle_t xEncoderQueueHandle;

// 局部/静态变量，用于存储最新数据
static EncoderData_t encoder_data;

// 定义任务周期：10ms
#define ENCODER_TASK_FREQUENCY_MS 10
static TickType_t xFrequency = pdMS_TO_TICKS(ENCODER_TASK_FREQUENCY_MS);


/**
 * @brief 编码器数据采集任务
 * @param pvParameters 任务创建时传入的参数 (FreeRTOS 原生签名)
 */
void Encoder_Task(void *pvParameters) // **修改：使用 FreeRTOS 原生签名**
{
    // 消除未使用参数的警告 (Good Practice)
    (void)pvParameters;

    // 用于 vTaskDelayUntil 的变量，必须在循环前初始化
    TickType_t xLastWakeTime; 
    
    // 1. 初始化
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
    
    // 必须在进入循环前获取当前系统时间
    xLastWakeTime = xTaskGetTickCount(); 
    
    for(;;)
    {
        // 1. 读取传感器数据
        // 左轮
        encoder_data.Encoder_Left_Count = Read_Speed(&htim2); 
        // 右轮 (取负保持与原控制逻辑一致)
        encoder_data.Encoder_Right_Count = -Read_Speed(&htim4); 
        
        // 2. 通过消息队列发送数据
        if (xEncoderQueueHandle != NULL)
        {
            // 使用 xQueueOverwrite 覆盖发送，确保 Control_Task 拿到最新数据
            xQueueOverwrite(xEncoderQueueHandle, &encoder_data); 
        }

        // 3. 任务延时 (绝对延时)
        // 使用 vTaskDelayUntil 确保精确的 10ms 周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency); 
    }
}


