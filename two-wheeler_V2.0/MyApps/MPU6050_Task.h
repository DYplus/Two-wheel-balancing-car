#ifndef __MPU6050_TASK_H
#define __MPU6050_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"

/* Exported constants --------------------------------------------------------*/
#define MPU6050_TASK_RATE_MS         10    // MPU6050读取频率(ms)
#define MPU6050_CALIBRATION_SAMPLES  100   // 校准采样次数

/* Exported functions prototypes ---------------------------------------------*/
void MPU6050_Task(void *pvParameters);
uint8_t MPU6050_Sensor_Init(void);
void MPU6050_Calibrate(void);



#ifdef __cplusplus
}
#endif

#endif /* __MPU6050_TASK_H */


