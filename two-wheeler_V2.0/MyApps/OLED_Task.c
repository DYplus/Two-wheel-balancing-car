#include "FreeRTOS.h" // 替换 cmsis_os.h
#include "task.h"     // 包含 vTaskDelay
#include "queue.h"    // 包含 QueueHandle_t, xQueueReceive
#include "main.h"
#include "oled.h"
#include "mpu6050_task.h"
#include "struct.h" 
#include <stdio.h>
#include <math.h>
#include <stdlib.h>


#define CMD_DISPLAY_HOLD_TIME_MS 500 // 指令显示保持时间：500 毫秒
static TickType_t xCmdReceivedTick = 0;   // 记录上次收到有效指令的时间戳
static char cLastValidCmd = 0;            // 存储上次接收到的有效指令字符

// 外部声明消息队列句柄
extern QueueHandle_t xStatusQueueHandle;

static CarOLED_t oled_t = {0};



// 强定义覆盖弱定义
void OLED_Task(void *pvParameters) // **修改：使用 FreeRTOS 原生签名**
{
    // 消除未使用参数的警告 (Good Practice)
    (void)pvParameters;

    BaseType_t status;
    
    // 1. 初始化
    OLED_Init(); 
    OLED_Clear();
    
    // 预先显示静态文本
    OLED_ShowString(0, 0, (uint8_t*)"ENco", 16);
	  OLED_ShowString(40, 0, (uint8_t*)"L:", 16);
	  OLED_ShowString(88, 0, (uint8_t*)"R:", 16);
	
	  OLED_ShowString(0, 2, (uint8_t*)"Roll:", 16);
	
    OLED_ShowString(0, 4, (uint8_t*)"dist:", 16);
	  OLED_ShowString(80, 4, (uint8_t*)"cm", 16);

    for(;;)
    {  

        status = xQueueReceive(xStatusQueueHandle, &oled_t, pdMS_TO_TICKS(5));		
			
        if (status == pdPASS)
        {
					 // 数据提取
            float roll_angle = oled_t.roll;	
            float distance_cm = oled_t.distance_cm;
            int enco_left  = oled_t.Enco_Left;
            int enco_right = oled_t.Enco_Right;
					  char current_cmd = oled_t.Cmd;
					
           //数据转换
            int roll_int = (int)fabs(roll_angle); // 整数部分 (取绝对值)
            // 小数部分：乘以 10 取整，再对 10 取模，得到第一位小数
            int roll_frac = (int)(fabs(roll_angle) * 10) % 10;
					
					 //符号显示
					if (roll_angle < 0) {
                OLED_ShowChar(40, 2, (uint8_t)'-', 16);
            } else {
                OLED_ShowChar(40, 2, (uint8_t)'+', 16);
            }
						// 2.3 数值显示
            OLED_ShowNum(48, 2, roll_int, 2, 16); // 2位整数部分
            OLED_ShowChar(64, 2, (uint8_t)'.', 16); // 小数点
            OLED_ShowNum(72, 2, roll_frac, 1, 16); // 1位小数部分
						
						// B. Distance
            int dist_int = (int)distance_cm; 
            int dist_frac = (int)(distance_cm * 10) % 10;
            
            // 整数部分 (3位)
            OLED_ShowNum(40, 4, dist_int, 3, 16); 
            // 小数点
            OLED_ShowChar(64, 4, (uint8_t)'.', 16);
            // 小数部分 (1位)
            OLED_ShowNum(72, 4, dist_frac, 1, 16);
						
						// 2.1 符号显示
            if (enco_left < 0) {
                OLED_ShowChar(56, 0, (uint8_t)'-', 16);
            } else {
                OLED_ShowChar(56, 0, (uint8_t)'+', 16);
            }
            // 2.2 数值显示
            OLED_ShowNum(64, 0, abs(enco_left), 2, 16); // 2位整数部分

            if (enco_right < 0) {
                OLED_ShowChar(104, 0, (uint8_t)'-', 16);
            } else {
                OLED_ShowChar(104, 0, (uint8_t)'+', 16);
            }
            // 2.2 数值显示
            OLED_ShowNum(104+8, 0, abs(enco_right), 2, 16); // 2位整数部分
						
						// ? 核心保持逻辑 A：更新有效指令和时间戳
        if (current_cmd != 0)
        {
            // 如果接收到非零指令，更新保持变量和时间戳
            cLastValidCmd = current_cmd;
            xCmdReceivedTick = xTaskGetTickCount();
        }

           // 2. ? 核心保持逻辑 B：判断最终显示内容
        char char_to_display = 0;
        
        // 检查上次的有效指令是否已超时
        if (cLastValidCmd != 0 && (xTaskGetTickCount() - xCmdReceivedTick < CMD_DISPLAY_HOLD_TIME_MS))
        {
            // 在保持时间内，继续显示上次的有效指令
            char_to_display = cLastValidCmd;
        }
        else
        {
            // 超过保持时间，或 cLastValidCmd 已经是 0
            char_to_display = ' '; // 显示空格
            cLastValidCmd = 0;     // 清除保持变量，等待下一次有效指令
        }

        // 3. 执行最终的显示
        // (注意：将这部分代码替换你原有的 OLED_ShowChar(0,6,...) 逻辑)
        if (char_to_display != ' ') {
            // 如果是非空格，则显示指令
            OLED_ShowChar(0, 6, (uint8_t)char_to_display, 16);
        } else {
            // 否则显示空格，实现清除效果
            OLED_ShowChar(0, 6, (uint8_t)' ', 16); 
        }

						

        }

        vTaskDelay(100); 
    }
}
		
