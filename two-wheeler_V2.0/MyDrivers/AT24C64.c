#include "at24c64.h"

/**
 * @brief 向 AT24C64 指定地址写入一个字节
 */
HAL_StatusTypeDef AT24C64_WriteByte(uint16_t MemAddress, uint8_t Data)
{
    return HAL_I2C_Mem_Write(&hi2c1, AT24C64_I2C_ADDR_WRITE, MemAddress,
                             AT24C64_MEM_ADDR_SIZE, &Data, 1, 100);
}

/**
 * @brief 从 AT24C64 指定地址读取一个字节
 */
HAL_StatusTypeDef AT24C64_ReadByte(uint16_t MemAddress, uint8_t *Data)
{
    return HAL_I2C_Mem_Read(&hi2c1, AT24C64_I2C_ADDR_WRITE, MemAddress,
                            AT24C64_MEM_ADDR_SIZE, Data, 1, 100);
}
