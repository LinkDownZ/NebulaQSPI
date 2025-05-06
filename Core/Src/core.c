//
// Created by hemeng on 2025/5/5.
//

#include "core.h"

#include "iwdg.h"
#include "sdram.h"


typedef void (*pFunction)(void);

/**
 * 必须是O0或者O1编译
 * @param addr 调整地址
 */
__attribute__((optimize("O1")))
void Jump_To_App(const uint32_t addr) {
    const int8_t result = QSPI_W25Qxx_MemoryMappedMode();
    if (result == QSPI_W25Qxx_OK) {
        printf("进入内存映射模式成功\r\n");
    } else {
        printf("内存映射错误,错误代码:%d\r\n", result);
        NVIC_SystemReset();
    }
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    for (uint8_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }
    __set_CONTROL(0);
    __disable_irq();
    __set_PRIMASK(1);
    printf("跳转地址:0x%08" PRIx32 "\n", addr);
    const pFunction JumpToApplication = (pFunction) *(__IO uint32_t *) (addr + 4);
    __set_MSP(*(__IO uint32_t *) addr);
    JumpToApplication();
}


void start_load() {
    if (HAL_OK != EEPROM_Init(&hi2c1)) {
        printf("初始化eeprom失败\n");
        HAL_Delay(60000);
    }
    printf("初始化eeprom成功\n");
    eeprom_settings_t settings;
    HAL_StatusTypeDef result = eeprom_load_settings(&settings);
    if (result != HAL_OK) {
        printf("加载settings失败 error code:%d\n", result);
        HAL_Delay(60000);
    }
    if (QSPI_W25Qxx_OK != QSPI_W25Qxx_Init()) {
        printf("初始化flash失败\n");
        HAL_Delay(60000);
    }
    printf("初始化flash成功\n");
    const uint32_t tickStart = HAL_GetTick();
    // 填充整个32MB SDRAM空间
    memset((void *) SDRAM_BANK_ADDR, 0xcc, SDRAM_Size);
    const uint32_t tickEnd = HAL_GetTick();
    printf("SDRAM填充耗时: %lu ms\n", tickEnd - tickStart);
    if (settings.is_upgrade == 0 && settings.start_flag == 0) {
        settings.start_flag = 1;
        eeprom_save_settings(&settings);
        Jump_To_App(settings.application_address);
    }
    if (settings.is_upgrade == 0 && settings.start_flag != 0) {
        printf("APP启动失败 重新下载APP\n");
        settings.start_flag = 1;
        eeprom_save_settings(&settings);
        Jump_To_App(settings.application_address);
    }
    if (settings.is_upgrade != 0) {
        printf("APP升级\n");
    }
}
