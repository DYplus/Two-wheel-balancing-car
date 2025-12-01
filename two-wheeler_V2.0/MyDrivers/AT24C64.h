#ifndef __AT24C64_H
#define __AT24C64_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// AT24C64 I2C 地址
#define AT24C64_I2C_ADDR_7BIT   0x50
#define AT24C64_I2C_ADDR_WRITE   (AT24C64_I2C_ADDR_7BIT << 1)
#define AT24C64_MEM_ADDR_SIZE    I2C_MEMADD_SIZE_16BIT  // 16-bit 内存地址

// EEPROM 写入延时（ms）
#define AT24C64_WRITE_TIMEOUT    5

extern I2C_HandleTypeDef hi2c1;

// 函数声明
HAL_StatusTypeDef AT24C64_WriteByte(uint16_t MemAddress, uint8_t Data);
HAL_StatusTypeDef AT24C64_ReadByte(uint16_t MemAddress, uint8_t *Data);

#endif /* __AT24C64_H */
