// ==========================================================
// File: tools.c
// Description: 包含 PID 算法、参数初始化和辅助控制函数
// ?? 注意：所有 PID 算法现在通过 Control_t* 指针操作状态。
// ==========================================================
#include "tools.h"
#include "struct.h"
#include <stdlib.h> 
#include <math.h>

// -------------------------------------------------------------
// PID 核心算法实现 (修改为操作 Control_t 结构体)
// -------------------------------------------------------------

// 假设低通滤波系数 'a' 仍为固定常数
static const float a = 0.7f; 


/**
 * @brief 直立环 PD 控制器
 * @param ctrl: Control_t 结构体指针，用于获取 Kp/Kd 和期望角度。
 * @param Angle: 真实角度 (roll)。
 * @param gyro_Y: 角速度 (gyrox)。
 * @return 直立环输出的 PWM 值。
 */
int Vertical(Control_t *ctrl, float Angle, float gyro_Y)
{
    // 期望角度 = Velocity_out + Med_Angle
    float Med = ctrl->Velocity_out + ctrl->Med_Angle;
    
    int temp;
    // 使用结构体中的 PID 参数
    temp = ctrl->Vertical_Kp * (Angle - Med) + ctrl->Vertical_Kd * gyro_Y;
    return temp;
}


/**
 * @brief 速度环 PI 控制器
 * @param ctrl: Control_t 结构体指针，用于获取/修改内部状态 (Encoder_S, Err_LowOut_last) 和参数。
 * @param encoder_L: 左编码器计数值。
 * @param encoder_R: 右编码器计数值。
 * @return 速度环输出，作为直立环的期望角度。
 */
int Velocity(Control_t *ctrl, int encoder_L, int encoder_R)
{
    int Err, Err_LowOut, temp;
    
    // 动态更新 Ki
    ctrl->Velocity_Ki = ctrl->Velocity_Kp / 200.0f; // 注意使用浮点数除法

    // 1、计算偏差值：总速度偏差 = 实际总速度 - 目标速度
    // ?? 使用结构体中的 Target_Speed
    Err = (encoder_L + encoder_R) - ctrl->Target_Speed; 

    // 2、低通滤波
    // ?? 使用结构体中的 Err_LowOut_last
    Err_LowOut = (1.0f - a) * Err + a * ctrl->Err_LowOut_last;
    ctrl->Err_LowOut_last = Err_LowOut; // 更新结构体中的上次输出

    // 3、积分
    ctrl->Encoder_S += Err_LowOut;

    // 4、积分限幅和清零
    if (ctrl->stop_flag == 1) { // ?? 使用结构体中的 Flag
        ctrl->Encoder_S = 0; 
        ctrl->stop_flag = 0;
    }
    // 积分项限幅 (-20000~20000)
    ctrl->Encoder_S = (ctrl->Encoder_S > 20000) ? 20000 : ((ctrl->Encoder_S < (-20000)) ? (-20000) : ctrl->Encoder_S);

    // 5、速度环计算
    temp = ctrl->Velocity_Kp * Err_LowOut + ctrl->Velocity_Ki * ctrl->Encoder_S;
    
    // 记录输出，用于 Vertical 函数的期望角度输入
    ctrl->Velocity_out = temp; 
    
    return temp;
}


/**
 * @brief 转向环 PD 控制器
 * @param ctrl: Control_t 结构体指针，用于获取参数和 Target_turn。
 * @param gyro_Z: 偏航角速度 (gyroz)。
 * @return 转向环输出的 PWM 偏差值。
 */
int Turn(Control_t *ctrl, float gyro_Z)
{
    int temp;
    // ?? 使用结构体中的 Target_turn 和可变 Turn_Kd
    temp = ctrl->Turn_Kp * ctrl->Target_turn + ctrl->Turn_Kd * gyro_Z;
    
    ctrl->Turn_out = temp; // 记录输出
    return temp;
}




