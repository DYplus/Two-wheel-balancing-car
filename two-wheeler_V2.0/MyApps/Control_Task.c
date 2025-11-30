#include "control_task.h"
#include "struct.h"      // 结构体定义
#include "tools.h"       // PID 算法和参数
#include "stm32f1xx_hal.h"    
#include "tim.h"

// 引入 FreeRTOS 原生头文件
#include "FreeRTOS.h" 
#include "task.h"      // 包含 vTaskDelayUntil
#include "queue.h"     // 包含 QueueHandle_t, xQueueReceive, xQueueOverwrite
#include <stdlib.h>    // 包含 abs()

// 外部声明消息队列句柄
extern QueueHandle_t xImuQueueHandle;
extern QueueHandle_t xEncoderQueueHandle;
extern QueueHandle_t xRemoteQueueHandle;
extern QueueHandle_t xStatusQueueHandle; // 假设用于 OLED
extern QueueHandle_t xSr04QueueHandle;

// 局部静态变量，存储从队列接收的最新数据
static ImuData_t g_imu_data = {0};
static EncoderData_t g_encoder_data;
static RemoteCmd_t g_remote_cmd = {0};
static CarOLED_t g_caroled_t = {0};
static Sr04Data_t g_sr04_date = {0};
static Control_t g_control_state = {0}; //控制参数


extern void Load(int moto1, int moto2);
extern void Limit(int *motoA, int *motoB);
extern void Stop(float *Med_Jiaodu, float *Jiaodu); 

// 定义任务周期：10ms
#define CONTROL_TASK_FREQUENCY_MS 10
static TickType_t xFrequency = pdMS_TO_TICKS(CONTROL_TASK_FREQUENCY_MS);

TickType_t xLastWakeTime;

void Control_Task(void *pvParameters) // **修改：使用 FreeRTOS 原生签名**
{
	
    // 消除未使用参数的警告 (Good Practice)
    (void)pvParameters;
	
	  // 初始化
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
	
	  Control_Init_Parameters(&g_control_state);
	
	  xLastWakeTime = xTaskGetTickCount(); // 初始化起始时间

    for (;;)
    {
		  	// 周期同步点：确保精确的 10ms 周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // 数据接收 (非阻塞)
        xQueueReceive(xImuQueueHandle, &g_imu_data, 0);       
        xQueueReceive(xEncoderQueueHandle, &g_encoder_data, 0);    
        xQueueReceive(xSr04QueueHandle, &g_sr04_date, 0);       
        xQueueReceive(xRemoteQueueHandle, &g_remote_cmd, 0);
			
		
//			  Load(2000,2000);


        
        // 3. ? 遥控指令到目标值映射和限幅 ?
        
        // 3.1. 前后速度目标 (Target_Speed)
        if((g_remote_cmd.Cmd_Forward == 0) && (g_remote_cmd.Cmd_Backward == 0)) {
            g_control_state.Target_Speed = 0; // 未收到指令 -> 速度清零
        }
        
        if(g_remote_cmd.Cmd_Forward == 1)
        {
            // 障碍物避障逻辑移植
            if(g_sr04_date.distance_cm < 50.0f) {
                g_control_state.Target_Speed++; // 靠近障碍物，减速 (速度目标值增大，但目标速度为负，实际上是减小负速度)
            } else {
                g_control_state.Target_Speed--; // 远离障碍物，增速
            }
        }
        if(g_remote_cmd.Cmd_Backward == 1){
            g_control_state.Target_Speed++; // 后退指令，原逻辑
        }
        
        // Target_Speed 限幅 (使用原 SPEED_Y 宏定义)
        #define SPEED_Y 12 
        g_control_state.Target_Speed = g_control_state.Target_Speed > SPEED_Y ? SPEED_Y : 
                                       (g_control_state.Target_Speed < (-SPEED_Y) ? (-SPEED_Y) : g_control_state.Target_Speed);
        
        
        // 3.2. 转向目标 (Target_turn)
        #define SPEED_Z 150 // 偏航最大设定速度
        
        if((g_remote_cmd.Cmd_Left == 0) && (g_remote_cmd.Cmd_Right == 0)) {
            g_control_state.Target_turn = 0;
            g_control_state.Turn_Kd = 0.6f; // 开启转向约束
        } else {
             g_control_state.Turn_Kd = 0.0f; // 收到转向指令，关闭转向约束
        }
        
        if(g_remote_cmd.Cmd_Left == 1) g_control_state.Target_turn += 30;	// 左转
        if(g_remote_cmd.Cmd_Right == 1) g_control_state.Target_turn -= 30;	// 右转
        
        // Target_turn 限幅
        g_control_state.Target_turn = g_control_state.Target_turn > SPEED_Z ? SPEED_Z : 
                                      (g_control_state.Target_turn < (-SPEED_Z) ? (-SPEED_Z) : g_control_state.Target_turn);
        
        
        // 5. 算法调用和结果计算 
        // 5.1. 直立环 (PD)
        // 期望角度 = 速度环输出 + 机械中值
        g_control_state.Vertical_out = Vertical(&g_control_state,
                                                g_imu_data.roll, 
                                                g_imu_data.gyrox); // 假设 gyrox 对应 Y 轴角速度
//        
//        // 5.2. 速度环 (PI)
//        g_control_state.Velocity_out = Velocity(&g_control_state, 
//                                                g_encoder_data.Encoder_Left_Count, 
//                                                g_encoder_data.Encoder_Right_Count);

//        
//        // 5.3. 转向环 (PD)
//        g_control_state.Turn_out = Turn(&g_control_state, g_imu_data.gyroz);
        
        
        // 5. 混合输出、限幅与驱动
        // PWM_out = Vertical_out (原代码的中间变量)
        g_control_state.MOTO1_PWM = g_control_state.Vertical_out - g_control_state.Turn_out;
        g_control_state.MOTO2_PWM = g_control_state.Vertical_out + g_control_state.Turn_out;
        
        // 5.2. 限幅和安全检测
        Limit(&g_control_state.MOTO1_PWM, &g_control_state.MOTO2_PWM);
        Stop(&g_control_state.Med_Angle, &g_imu_data.roll); // 使用 Med_Angle 的地址和 roll 的地址
        
        // 5.3. 电机驱动
        Load(g_control_state.MOTO1_PWM, g_control_state.MOTO2_PWM);
        
        
        // 6. 状态发送给 OLED
        if (xStatusQueueHandle != NULL)
        {
            g_caroled_t.Motor1_PWM_Out = g_control_state.MOTO1_PWM; 
            g_caroled_t.Motor2_PWM_Out = g_control_state.MOTO2_PWM;
					  g_caroled_t.Enco_Left = g_encoder_data.Encoder_Left_Count;
					g_caroled_t.Enco_Right = g_encoder_data.Encoder_Right_Count;
            g_caroled_t.roll = g_imu_data.roll;
            g_caroled_t.distance_cm = g_sr04_date.distance_cm;
            g_caroled_t.Cmd = g_remote_cmd.Cmd;
            
            xQueueOverwrite(xStatusQueueHandle, &g_caroled_t);
        }

    } // end for (;;) 
}

/**
 * @brief 初始化 Control_t 结构体中的所有PID参数和内部状态。
 * @param ctrl 指向 Control_t 结构体的指针。
 */
void Control_Init_Parameters(Control_t *ctrl)
{
    // 确保指针有效
    if (ctrl == NULL) {
        return;
    }
    
    // 默认清零所有字段 (确保内部状态如 Encoder_S, Err_LowOut_last 都是 0)
    // 这是一个良好的实践，但也可以省略，因为我们会重新赋值。
    // memset(ctrl, 0, sizeof(Control_t)); 
    
    // --- 1. PID 参数初始化 (基于原 control.c 文件中的值) ---
    ctrl->Vertical_Kp = -120.0f;
    ctrl->Vertical_Kd = -0.75f;
    
    ctrl->Velocity_Kp = -1.1f;
    // Velocity_Ki 是根据 Kp 计算得出的，这里直接计算
    ctrl->Velocity_Ki = ctrl->Velocity_Kp / 200.0f; // 约 -0.0055f
    
    ctrl->Turn_Kp = 10.0f;
    // Turn_Kd 初始值为 0.6f (转向约束开启状态)
    ctrl->Turn_Kd = 0.6f; 
    
    ctrl->Med_Angle = -3.0f; // 平衡时角度偏移量
    
    // --- 2. PID 内部状态初始化 ---
    ctrl->Encoder_S = 0;         // 速度环积分项累加值
    ctrl->Err_LowOut_last = 0;   // 速度误差低通滤波的上次输出值
    ctrl->stop_flag = 0;         // 初始状态：不停止

    // --- 3. 实时目标和输出 (初始值) ---
    ctrl->Target_Speed = 0;
    ctrl->Target_turn = 0;
    
    // 最终输出 (初始化为 0)
    ctrl->MOTO1_PWM = 0;
    ctrl->MOTO2_PWM = 0;
    ctrl->Vertical_out = 0;
    ctrl->Velocity_out = 0;
    ctrl->Turn_out = 0;
}

