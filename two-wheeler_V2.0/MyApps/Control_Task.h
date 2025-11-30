#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H

#include "freertos.h" // 
#include "struct.h"


void Control_Task(void *pvParameters);
void Control_Init_Parameters(Control_t *ctrl);

// --- 外部引用 (电机驱动和安全函数) ---
extern void Load(int moto1, int moto2);			
extern void Limit(int *motoA, int *motoB);		
extern void Stop(float *Med_Jiaodu, float *Jiaodu); 

#endif /* __CONTROL_TASK_H */
