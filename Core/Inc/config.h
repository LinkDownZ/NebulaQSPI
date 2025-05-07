//
// Created by hemeng on 2025/5/2.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>

#include "stm32h7xx_hal.h"
#include "at24c32.h"
#include "cJSON.h"
#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>

#include "crc.h"

typedef struct {
    uint8_t is_upgrade;
    uint8_t start_flag;
    char *ntp_domain;
    char *ota_domain;
    uint32_t application_address;
} eeprom_settings_t;

HAL_StatusTypeDef eeprom_load_settings(
    eeprom_settings_t *settings);

HAL_StatusTypeDef eeprom_save_default_settings(void);

HAL_StatusTypeDef eeprom_save_settings(
    const eeprom_settings_t *settings);
#endif //CONFIG_H
