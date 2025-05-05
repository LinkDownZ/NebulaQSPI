//
// Created by hemeng on 2025/5/5.
//

#include "core.h"


typedef void (*pFunction)(void);

pFunction JumpToApplication;

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
    JumpToApplication = (pFunction) *(__IO uint32_t *) (addr + 4);
    __set_MSP(*(__IO uint32_t *) addr);
    JumpToApplication();
}


void start_load() {
    if (HAL_OK != EEPROM_Init(&hi2c1)) {
        printf("初始化eeprom失败\n");
        NVIC_SystemReset();
    }
    printf("初始化eeprom成功\n");
    if (0) {
        eeprom_settings_t settings;
        settings.is_upgrade = 0;
        settings.ntp_domain = "ntp.aliyun.com";
        settings.ota_domain = "api.hemeng.org";
        settings.application_address = QSPI_BASE;
        settings.start_flag = 0;
        const HAL_StatusTypeDef result = eeprom_save_settings(&settings);
        if (result != HAL_OK) {
            printf("保存失败 error code:%d\n", result);
        }
    }
    eeprom_settings_t settings;
    const HAL_StatusTypeDef result = eeprom_load_settings(&settings);
    if (result != HAL_OK) {
        printf("加载settings失败 error code:%d\n", result);
        NVIC_SystemReset();
    }
    if (QSPI_W25Qxx_OK != QSPI_W25Qxx_Init()) {
        printf("初始化flash失败\n");
        NVIC_SystemReset();
    }
    printf("初始化flash成功\n");
    if (settings.is_upgrade == 0 && settings.start_flag == 0) {
        settings.start_flag = 1;
        eeprom_save_settings(&settings);
        Jump_To_App(settings.application_address);
    }
    if (settings.is_upgrade == 0 && settings.start_flag != 0) {
        printf("APP启动失败 重新下载APP\n");
    }
    if (settings.is_upgrade != 0) {
        printf("APP升级\n");
    }
}
