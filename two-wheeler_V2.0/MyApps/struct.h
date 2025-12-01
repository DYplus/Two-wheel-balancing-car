#ifndef __STRUCT_H
#define __STRUCT_H

#include "main.h"
#include "string.h"

//IMU 数据结构体
typedef struct
{
    float pitch;    // 俯仰角
    float roll;     // 侧倾角 (直立环主输入)
    float yaw;      // 航向角
	
    short gyrox;    // 俯仰角速度 (直立环 D 项输入)
    short gyroy;    // 侧倾角速度
    short gyroz;    // 偏航角速度 (转向环 D 项输入)
	
    short aacx;     // X轴加速度
    short aacy;     // Y轴加速度
    short aacz;     // Z轴加速度
	
	  float temp;     ///< 温度值 (摄氏度)
} ImuData_t;

// 编码器数据结构体 

typedef struct
{
    int Encoder_Left_Count;     // 左编码器计数
    int Encoder_Right_Count;    // 右编码器计数
} EncoderData_t;  

// 遥控指令结构体

typedef struct
{
    uint8_t Cmd_Forward;        // 前进指令 (1:有效)
    uint8_t Cmd_Backward;       // 后退指令 (1:有效)
    uint8_t Cmd_Left;           // 左转指令 (1:有效)
    uint8_t Cmd_Right;          // 右转指令 (1:有效)
	
	  char Cmd;
    float Obstacle_Distance;    // 障碍物距离 (用于自动减速/避障)
} RemoteCmd_t;

//控制量输出结构体 ---

typedef struct
{
    int Motor1_PWM_Out;         // 左电机最终 PWM
    int Motor2_PWM_Out;         // 右电机最终 PWM
} MotorPwmOut_t;




// 小车控制结构体
typedef struct
{
    // --- 1. PID 参数 ---
    float Vertical_Kp;
    float Vertical_Kd;
    float Velocity_Kp;
    float Velocity_Ki;
    float Turn_Kp;
    float Turn_Kd;
    float Med_Angle; // 平衡时角度值偏移量

    // --- 2. PID 内部状态 (原 static 变量) ---
    int Encoder_S;         // 速度环积分项累加值
    int Err_LowOut_last;   // 速度误差低通滤波的上次输出值
    uint8_t stop_flag;     // 速度环积分清零标志

    // --- 3. 实时目标和输出 ---
    int Target_Speed;
    int Target_turn;

    // --- 4. 中间输出结果 ---
    int Vertical_out;  //直立环
    int Velocity_out;
    int Turn_out;
    
    // --- 5. 最终电机输出 ---
    int MOTO1_PWM;
    int MOTO2_PWM;

} Control_t;

typedef struct
{
    float roll;
	  float distance_cm;
	
	  int Enco_Right;
	  int Enco_Left;

    int Motor1_PWM_Out;         // 
    int Motor2_PWM_Out;         // 编码器
	
	  char Cmd;  //控制指令

} CarOLED_t; // 我们可以创建一个精简版，或者直接使用 CarState_t

typedef struct
{
    float distance_cm; 

} Sr04Data_t; 

typedef struct
{
    uint8_t eeprom_value;
} EepromData_t;


#endif /* __STRUCT_H */

