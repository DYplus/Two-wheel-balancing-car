#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "at24c64.h"

/* 外部变量 */
extern SemaphoreHandle_t g_i2c1_mutex;

/* RAM 全局变量，用于 OLED 显示 */
uint8_t g_eeprom_counter = 0x00;  // 初值
uint8_t g_at_counter = 0;

/* EEPROM 地址 */
#define EEPROM_COUNTER_ADDR 0x0000  // 16-bit 地址
#define EEPROM_DEFAULT_VAL  0xA1

/* 写入 EEPROM 并确认写入完成 */
static HAL_StatusTypeDef AT24C64_WriteByteWithReady(uint16_t addr, uint8_t data)
{
    HAL_StatusTypeDef status;
    status = AT24C64_WriteByte(addr, data);
    if (status != HAL_OK) return status;

    // 等待 EEPROM 内部写入完成
    for (int i = 0; i < 10; i++)
    {
        if (HAL_I2C_IsDeviceReady(&hi2c1, AT24C64_I2C_ADDR_WRITE, 1, 10) == HAL_OK)
            return HAL_OK;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return HAL_ERROR;
}

/* EEPROM 任务 */
void AT24C64_Task(void *pvParameters)
{
    (void)pvParameters;
    HAL_StatusTypeDef status;
    uint8_t value = 0;

    // 系统稳定等待
    vTaskDelay(pdMS_TO_TICKS(500));

    /* Step 1: 写入 EEPROM 默认值 */
    if (xSemaphoreTake(g_i2c1_mutex, portMAX_DELAY) == pdTRUE)
    {
        status = AT24C64_WriteByteWithReady(EEPROM_COUNTER_ADDR, EEPROM_DEFAULT_VAL);
        g_eeprom_counter = (status == HAL_OK) ? EEPROM_DEFAULT_VAL : 0xEF;

        // 读取 EEPROM 确认写入
        status = AT24C64_ReadByte(EEPROM_COUNTER_ADDR, &value);
        g_eeprom_counter = (status == HAL_OK) ? value : 0xBB;

        xSemaphoreGive(g_i2c1_mutex);
    }

    /* Step 2: 主循环，只累加 g_at_counter */
    while (1)
    {
        g_at_counter++;  // 验证任务运行
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒累加一次
    }
}
