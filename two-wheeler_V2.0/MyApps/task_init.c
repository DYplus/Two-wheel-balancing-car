#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "struct.h"
// --- 添加或确认以下任务头文件已包含 ---
#include "oled_task.h"
#include "mpu6050_task.h"
#include "encoder_task.h"
#include "control_task.h"
#include "bluetooth_task.h" 
#include "at24c64_task.h" 
#include "sr04_task.h"
// ------------------------------------------

/* 队列句柄 */
QueueHandle_t xImuQueueHandle;
QueueHandle_t xEncoderQueueHandle;
QueueHandle_t xRemoteQueueHandle;
QueueHandle_t xStatusQueueHandle;
QueueHandle_t xSr04QueueHandle;

/* 任务句柄 */
TaskHandle_t OLEDHandle;
TaskHandle_t MPU6050Handle;
TaskHandle_t EncoderHandle;
TaskHandle_t ControlHandle;
TaskHandle_t BluetoothHandle;
TaskHandle_t SR04Handle;
TaskHandle_t AT24C64Handle;  // EEPROM 任务句柄

// I2C总线互斥量的句柄。用于保护 I2C1 资源。
SemaphoreHandle_t g_i2c1_mutex = NULL;

// EEPROM 循环计数器的全局变量

void Task_Init(void)
{
	
	// I2C1 互斥量创建
    g_i2c1_mutex = xSemaphoreCreateMutex();
    configASSERT(g_i2c1_mutex);
	
    /* ============== 创建队列（原生 API） ============== */

    /* IMU 队列 */
    xImuQueueHandle = xQueueCreate(1, sizeof(ImuData_t));

    /* 编码器数据 */
    xEncoderQueueHandle = xQueueCreate(1, sizeof(EncoderData_t));

    /* 遥控器指令 */
    xRemoteQueueHandle = xQueueCreate(1, sizeof(RemoteCmd_t));

    /* 状态信息（OLED） */
    xStatusQueueHandle = xQueueCreate(1, sizeof(CarOLED_t));
	
	  xSr04QueueHandle = xQueueCreate(1, sizeof(Sr04Data_t));
    /* 单槽队列 + 覆盖发送 = 最新状态 */

    /* 队列检查 */
    configASSERT(xImuQueueHandle);
    configASSERT(xEncoderQueueHandle);
    configASSERT(xRemoteQueueHandle);
    configASSERT(xStatusQueueHandle);

    /* ============== 创建任务（原生 API） ============== */


    xTaskCreate(OLED_Task, "OLED", 256, NULL, 4, &OLEDHandle);
    xTaskCreate(MPU6050_Task, "MPU", 512, NULL, 3, &MPU6050Handle);
    xTaskCreate(Encoder_Task, "ENC", 128, NULL, 3, &EncoderHandle);
    xTaskCreate(Control_Task, "CTRL", 512, NULL, 3, &ControlHandle);
    xTaskCreate(Bluetooth_Task, "BT", 128, NULL, 3, &BluetoothHandle);
		xTaskCreate(SR04_Task, "SR04", 256, NULL, 3, &SR04Handle);
		xTaskCreate(AT24C64_Task, "EEPROM", 128, NULL, 3, &AT24C64Handle);//  EEPROM 任务
}
