//
// Created by hemeng on 2025/3/29.
//
#include <at24c32.h>


static I2C_HandleTypeDef *eeprom_hi2c = NULL; // I2C句柄指针

// 初始化I2C句柄，传入空指针返回错误
HAL_StatusTypeDef EEPROM_Init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == NULL) {
        return HAL_ERROR;
    }
    eeprom_hi2c = hi2c;
    return HAL_OK;
}

// 写入数据（自动处理页边界）
// data声明为const，确保数据内容不会被修改
HAL_StatusTypeDef EEPROM_Write(uint16_t memAddr, const uint8_t *data, const uint16_t size) {
    uint16_t bytesWritten = 0;
    while (bytesWritten < size) {
        // 计算当前页内的偏移和剩余空间
        const uint16_t pageOffset = memAddr % EEPROM_PAGE_SIZE;
        const uint16_t spaceInPage = EEPROM_PAGE_SIZE - pageOffset;
        const uint16_t chunkSize = (size - bytesWritten < spaceInPage) ? (size - bytesWritten) : spaceInPage;
        // 写入当前块数据
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(eeprom_hi2c, EEPROM_I2C_ADDR, memAddr,
                                                     I2C_MEMADD_SIZE_16BIT, (uint8_t *) &data[bytesWritten],
                                                     chunkSize, 100);
        if (status != HAL_OK) {
            return status;
        }
        // 等待EEPROM内部写入完成
        HAL_Delay(EEPROM_WRITE_DELAY);
        bytesWritten += chunkSize;
        memAddr += chunkSize;
    }
    return HAL_OK;
}

// 读取数据（无需分页）
HAL_StatusTypeDef EEPROM_Read(const uint16_t memAddr, uint8_t *data, const uint16_t size) {
    return HAL_I2C_Mem_Read(eeprom_hi2c, EEPROM_I2C_ADDR, memAddr,
                            I2C_MEMADD_SIZE_16BIT, data, size, 400);
}
