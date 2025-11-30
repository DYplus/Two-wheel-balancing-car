#include "sr04.h"
#include "main.h" // 包含 GPIO 定义 (例如 GPIOA, GPIO_PIN_x)

// 必须在任何 FreeRTOS 组件头文件之前包含
#include "FreeRTOS.h" 
#include "task.h" 
#include "sr04_task.h" // 引入 SR04Handle

// 假设您的 SR04 接口定义 (请根据您的 main.h 或 CubeMX 配置确认)
#define TRIG_PORT   GPIOA
#define TRIG_PIN    GPIO_PIN_3
#define ECHO_PORT   GPIOA
#define ECHO_PIN    GPIO_PIN_2 

// 外部驱动变量定义
volatile uint32_t uwTickStart = 0; 
volatile uint32_t uwTickEnd = 0;   
volatile uint8_t  flag_data_ready = 0; 


// -----------------------------------------------------------------
// DWT 初始化函数
// -----------------------------------------------------------------
static void DWT_Enable(void)
{
    // 1. 启用 TRCENA (Trace enable)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; 
    
    // 2. 清除周期计数器
    DWT->CYCCNT = 0U; 
    
    // 3. 启用 CYCCNT (Cycle Count Enable)
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; 
}


// -----------------------------------------------------------------
// 驱动模块初始化 (在 main.c 中调度器启动前调用)
// -----------------------------------------------------------------
void SR04_Init(void)
{
    DWT_Enable();
    // 可以在这里执行其他 SR04 相关的 GPIO 初始化（如果 CubeMX 没有完全处理）
}

// -----------------------------------------------------------------
// DWT 延时函数
// -----------------------------------------------------------------
void DWT_Delay_us(uint32_t udelay)
{
    uint32_t start_cycles = DWT_GetCycles();
    uint32_t delay_cycles = udelay * CYCLES_PER_US;
    
    while (DWT_GetCycles() - start_cycles < delay_cycles);
}


// -----------------------------------------------------------------
// SR04 触发函数
// -----------------------------------------------------------------
void SR04_Trigger(void)
{
    // 发送 12us 高电平脉冲到 TRIG (PA3)
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
    DWT_Delay_us(12); // 使用 DWT 进行精确延时
    HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
}


// -----------------------------------------------------------------
// EXTI 中断回调函数 (在 stm32f1xx_it.c 中被 HAL_GPIO_EXTI_IRQHandler 调用)
// -----------------------------------------------------------------
extern TaskHandle_t SR04Handle; // 从 sr04_task.h 引入

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // 确保只处理 SR04 的 ECHO 引脚中断 (PA2)
    if(GPIO_Pin == ECHO_PIN) 
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // 1. 上升沿：开始计时
        if(HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET)
        {
            uwTickStart = DWT_GetCycles(); 
        }
        // 2. 下降沿：结束计时
        else 
        {
            uwTickEnd = DWT_GetCycles(); 
            flag_data_ready = 1;         
            
            // 使用任务句柄唤醒 SR04 任务
            if (SR04Handle != NULL)
            {
                vTaskNotifyGiveFromISR(SR04Handle, &xHigherPriorityTaskWoken);
            }
        }
        
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

