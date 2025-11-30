#ifndef __SR04_H
#define __SR04_H

// 包含 HAL 库和 Cortex 内核寄存器定义
#include "stm32f1xx_hal.h" 
#include "core_cm3.h" // STM32F1xx 是 Cortex-M3

// --- DWT 周期计数器宏定义 ---
// DWT 计数器以 CPU 主频运行
#define DWT_GetCycles() DWT->CYCCNT
// 假设 SystemCoreClock 是 72MHz (或您的实际主频)
#define CYCLES_PER_US (SystemCoreClock / 1000000U) 

// --- 驱动函数声明 ---

// 初始化 DWT 计数器和 SR04 驱动
void SR04_Init(void); 

// 使用 DWT 实现微秒级延时
void DWT_Delay_us(uint32_t udelay);

// 触发 SR04 测距
void SR04_Trigger(void);

// 外部计时变量声明 (供任务和中断使用)
extern volatile uint32_t uwTickStart; 
extern volatile uint32_t uwTickEnd;   
extern volatile uint8_t  flag_data_ready; 

#endif /* __SR04_H */

