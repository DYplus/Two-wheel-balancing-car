#include "mpu6050_task.h"
#include "struct.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"

#include "FreeRTOS.h" 
#include "task.h"      
#include "queue.h"     

static ImuData_t mpu_data = {0};

extern QueueHandle_t xImuQueueHandle;

// 定义任务周期：10ms
#define MPU_TASK_FREQUENCY_MS 10
static TickType_t xFrequency = pdMS_TO_TICKS(MPU_TASK_FREQUENCY_MS);

// 任务签名修正为 FreeRTOS 原生 API
void MPU6050_Task(void *pvParameters)
{
    (void)pvParameters; 
    
    TickType_t xLastWakeTime; 

    // 1. 初始化
    MPU_Init();
    mpu_dmp_init();
    
    xLastWakeTime = xTaskGetTickCount(); 

    for(;;)
    {
        // 2. 读取传感器数据
        mpu_dmp_get_data(&mpu_data.pitch, &mpu_data.roll, &mpu_data.yaw);
        MPU_Get_Gyroscope(&mpu_data.gyrox, &mpu_data.gyroy, &mpu_data.gyroz);
        MPU_Get_Accelerometer(&mpu_data.aacx, &mpu_data.aacy, &mpu_data.aacz); 
			
        
        // 3. 通过消息队列发送数据
        if (xImuQueueHandle != NULL)
        {
            xQueueOverwrite(xImuQueueHandle, &mpu_data);  
        }

        // 4. 任务延时 (绝对延时)
        vTaskDelay(10); 
    }
}


