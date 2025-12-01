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
#include "semphr.h"


#define CMD_DISPLAY_HOLD_TIME_MS 500 // 指令显示保持时间：500 毫秒
static TickType_t xCmdReceivedTick = 0;   // 记录上次收到有效指令的时间戳
static char cLastValidCmd = 0;            // 存储上次接收到的有效指令字符


// 外部声明消息队列句柄
extern QueueHandle_t xStatusQueueHandle;

static CarOLED_t oled_t = {0};

// 外部声明 I2C 互斥量和全局变量
extern SemaphoreHandle_t g_i2c1_mutex;
extern uint8_t g_eeprom_counter;
extern uint8_t g_at_counter;


void OLED_Task(void *pvParameters) // **修改：使用 FreeRTOS 原生签名**
{
    // 消除未使用参数的警告 (Good Practice)
    (void)pvParameters;

    BaseType_t status;
    
    if (xSemaphoreTake(g_i2c1_mutex, portMAX_DELAY) == pdTRUE)
    {
        OLED_Init(); 
        OLED_Clear();
        xSemaphoreGive(g_i2c1_mutex);
    }
    
    if (xSemaphoreTake(g_i2c1_mutex, portMAX_DELAY) == pdTRUE)
    {
        OLED_ShowString(0, 0, (uint8_t*)"ENco", 16);
        OLED_ShowString(40, 0, (uint8_t*)"L:", 16);
        OLED_ShowString(88, 0, (uint8_t*)"R:", 16);
        
        OLED_ShowString(0, 2, (uint8_t*)"Roll:", 16);
        
        OLED_ShowString(0, 4, (uint8_t*)"dist:", 16);
        OLED_ShowString(80, 4, (uint8_t*)"cm", 16);
        
        // **新增EEPROM显示区域**
        OLED_ShowString(0, 6, (uint8_t*)"EPRM:", 16); // 占用第6行
        
        xSemaphoreGive(g_i2c1_mutex);
    }

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
            int roll_frac = (int)(fabs(roll_angle) * 10) % 10;
					
					  int dist_int = (int)distance_cm; 
            int dist_frac = (int)(distance_cm * 10) % 10;
					
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
 
					
					
				// 动态显示操作 (必须受 I2C 互斥锁保护)
				
				if (xSemaphoreTake(g_i2c1_mutex, pdMS_TO_TICKS(50)) == pdTRUE)
            {
							// 1. Encoder 显示
						if (enco_left < 0) {
                OLED_ShowChar(56, 0, (uint8_t)'-', 16);
            } else {
                OLED_ShowChar(56, 0, (uint8_t)'+', 16);
            }
            OLED_ShowNum(64, 0, abs(enco_left), 2, 16); // 2位整数部分

            if (enco_right < 0) {
                OLED_ShowChar(104, 0, (uint8_t)'-', 16);
            } else {
                OLED_ShowChar(104, 0, (uint8_t)'+', 16);
            }
            OLED_ShowNum(104+8, 0, abs(enco_right), 2, 16); // 2位整数部分
						
						// 2. Roll 角度显示
					if (roll_angle < 0) {
                OLED_ShowChar(40, 2, (uint8_t)'-', 16);
            } else {
                OLED_ShowChar(40, 2, (uint8_t)'+', 16);
            }
            OLED_ShowNum(48, 2, roll_int, 2, 16); // 2位整数部分
            OLED_ShowChar(64, 2, (uint8_t)'.', 16); // 小数点
            OLED_ShowNum(72, 2, roll_frac, 1, 16); // 1位小数部分
						
            // 3. Distance 距离显示
            OLED_ShowNum(40, 4, dist_int, 3, 16); 
            OLED_ShowChar(64, 4, (uint8_t)'.', 16);
            OLED_ShowNum(72, 4, dist_frac, 1, 16);
						
						// EEPROM 计数器显示 (Row 6)**
						uint8_t eeprom_value = g_eeprom_counter;
            char eeprom_str[5]; 
            // 格式化为 0xXX 的十六进制字符串
            snprintf(eeprom_str, sizeof(eeprom_str), "0x%02X", eeprom_value); 
            OLED_ShowString(40, 6, (uint8_t*)eeprom_str, 16);
						OLED_ShowNum(90, 6, g_at_counter, 2, 16);
						
	
						//遥控指令显示 此时要么是有效指令，要么是 ' ' (空格)

            OLED_ShowChar(108, 6, (uint8_t)char_to_display, 16);
						
       if (char_to_display != ' ') {
            // 如果是非空格，则显示指令
            OLED_ShowChar(108, 6, (uint8_t)char_to_display, 16);
        } else {
            // 否则显示空格，实现清除效果
            OLED_ShowChar(108, 6, (uint8_t)' ', 16); 
        }
 
						
						xSemaphoreGive(g_i2c1_mutex);
				
        }

        vTaskDelay(100); 
			}
    }
}
		
