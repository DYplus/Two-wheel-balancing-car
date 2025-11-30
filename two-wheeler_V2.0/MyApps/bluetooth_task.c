// ==========================================================
// File: bluetooth_task.c
// Description: FreeRTOS 蓝牙指令处理任务
// ==========================================================
#include "bluetooth_task.h"
#include "struct.h"


// --- 全局变量定义 ---
TaskHandle_t xBluetoothTaskHandle = NULL; // 蓝牙任务句柄定义

extern UART_HandleTypeDef huart3;
extern uint8_t rx_buf[1];
extern uint8_t Bluetooth_data;
extern QueueHandle_t xRemoteQueueHandle;



void Bluetooth_Task(void *pvParameters)
{
    // 1. 在任务开始时获取并保存自己的句柄
    xBluetoothTaskHandle = xTaskGetCurrentTaskHandle();
    
    // 2. 启动第一次串口中断接收
    // 注意：需要确保 huart3 在调用前已被正确初始化
    HAL_UART_Receive_IT(&huart3, rx_buf, 1);

    for (;;)
    {
        // 3. 等待任务通知
        // ulTaskNotifyTake(pdTRUE, ...) 的第一个参数 pdTRUE 表示在接收后清零通知
        // portMAX_DELAY 表示永久等待，直到收到通知
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			
			  RemoteCmd_t remote_cmd = {0};

        uint8_t received_byte = Bluetooth_data; // 读取全局接收到的数据

        if (received_byte == 0x47) {     // 前
            remote_cmd.Cmd_Forward = 1;
				    remote_cmd.Cmd = 'F';
				}
        else if (received_byte == 0x4B) {// 后
            remote_cmd.Cmd_Backward = 1;
						remote_cmd.Cmd = 'B';
			}
        else if (received_byte == 0x4A){ // 右
            remote_cmd.Cmd_Right = 1;
						remote_cmd.Cmd = 'R';
			}
        else if (received_byte == 0x48){ // 左
            remote_cmd.Cmd_Left = 1;
				    remote_cmd.Cmd = 'L';
			}

        // --- 5. 将解析后的指令通过消息队列发送给 control 任务 ---
        if (xRemoteQueueHandle != NULL)
        {
            // 立即发送 (等待时间设为 0)，如果队列满则丢弃，保证实时性
            if (xQueueSend(xRemoteQueueHandle, &remote_cmd, 0) != pdPASS)
            {
                // 队列发送失败 (通常意味着 control 任务处理太慢或队列太短)
            }
        }
    }
}

