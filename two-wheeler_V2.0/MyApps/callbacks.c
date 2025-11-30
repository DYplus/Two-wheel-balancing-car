#include "freertos.h"
#include "main.h"
#include "callbacks.h"
#include "task_init.h"


extern UART_HandleTypeDef huart3;
uint8_t rx_buf[1];           // 串口接收缓冲区
uint8_t Bluetooth_data;      // 蓝牙接收到的数据字节

extern TaskHandle_t xBluetoothTaskHandle; // 蓝牙任务句柄
extern QueueHandle_t xControlQueueHandle; // 控制任务队列句柄

/**
 * @brief 串口接收完成回调函数
 * @param huart 串口句柄指针
 * @note 此函数会在 USART3 中断中被 HAL 库调用
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 确保是 USART3 触发的回调
    if (huart->Instance == USART3)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // 1. 将接收到的数据保存到全局变量，供任务读取
        Bluetooth_data = rx_buf[0];

        // 2. 发送任务通知给 Bluetooth_Task (非阻塞)
        if (xBluetoothTaskHandle != NULL)
        {
            // 发送任务通知，解除 Bluetooth_Task 的阻塞状态
            vTaskNotifyGiveFromISR(xBluetoothTaskHandle, &xHigherPriorityTaskWoken);
        }

        // 3. 重新启动接收中断，准备接收下一个字节
        HAL_UART_Receive_IT(&huart3, rx_buf, 1);

        // 4. 如果任务通知唤醒了更高优先级的任务，则请求上下文切换
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

