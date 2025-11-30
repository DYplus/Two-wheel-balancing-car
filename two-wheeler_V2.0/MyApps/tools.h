// ==========================================================
// File: tools.h 
// ==========================================================
#ifndef __TOOLS_H
#define __TOOLS_H

#include "struct.h" // 确保 Control_t 结构体已定义

// 外部全局变量声明 (如果有)
extern float Vert_Kp, Vert_Kd;
extern float Vel_Kp, Vel_Ki;
extern float Turn_Kp, Turn_Kd;
extern float Med_Balance_Angle;
extern uint8_t Flag_Stop;

// -------------------------------------------------------------
// PID 核心函数声明 (统一名称)
// -------------------------------------------------------------

// 直立环 PD 控制器
// 输入：期望角度(Med)、真实角度(Angle)、角速度(gyro_Y)
int Vertical(Control_t *ctrl, float Angle, float gyro_Y);

// 速度环 PI 控制器
// 输入：期望速度(Target)、左编码器(encoder_L)、右编码器(encoder_R)
int Velocity(Control_t *ctrl, int encoder_L, int encoder_R);

// 转向环 PD 控制器
// 输入：偏航角速度(gyro_Z)、目标转向值(Target_turn)
int Turn(Control_t *ctrl, float gyro_Z);

// 输出函数 (Limit, Load, Stop)
extern void Load(int moto1, int moto2);
extern void Limit(int *motoA, int *motoB);
extern void Stop(float *Med_Jiaodu, float *Jiaodu); 

#endif // __TOOLS_H

