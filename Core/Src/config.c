//
// Created by hemeng on 2025/5/2.
//
#include "config.h"


uint16_t CRC_Calculate(uint32_t mark, char text[], uint32_t len) {
    HAL_CRC_Calculate(&hcrc, &mark, 4);
    HAL_CRC_Accumulate(&hcrc, &len, 2);
    return (uint16_t) (HAL_CRC_Accumulate(&hcrc, (uint32_t *) text, len) & 0xFFFF);
}

HAL_StatusTypeDef eeprom_load_settings0(
    eeprom_settings_t *settings) {
    uint32_t magic_num;
    HAL_StatusTypeDef result = EEPROM_Read(0, (uint8_t *) &magic_num, sizeof(magic_num));
    if (HAL_OK != result) {
        return result;
    }
    if (magic_num != 0xFEFCDDDC) {
        return HAL_ERROR;
    }
    uint16_t len;
    result = EEPROM_Read(sizeof(magic_num), (uint8_t *) &len, sizeof(len));
    if (HAL_OK != result) {
        return result;
    }
    char text[len + 1];
    const uint32_t text_offset = sizeof(magic_num) + sizeof(len);
    result = EEPROM_Read(text_offset, (uint8_t *) text, len);
    if (result != HAL_OK) {
        return result;
    }
    text[len] = '\0';
    // 5. 读取存储的 CRC 校验值（2 字节）
    uint16_t read_crc;
    result = EEPROM_Read(text_offset + len,
                         (uint8_t *) &read_crc,
                         sizeof(read_crc));
    if (result != HAL_OK) {
        return result;
    }
    // 6. 校验 CRC，保证读到的数据完整无误
    if (read_crc != CRC_Calculate(magic_num, text, len)) {
        return HAL_ERROR;
    }
    cJSON *root = cJSON_Parse(text);
    if (!root) {
        return HAL_ERROR;
    }
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(root, "is_upgrade");
    if (cJSON_IsNumber(item)) {
        settings->is_upgrade = (uint8_t) item->valueint;
    }
    item = cJSON_GetObjectItemCaseSensitive(root, "start_flag");
    if (cJSON_IsNumber(item)) {
        settings->start_flag = (uint8_t) item->valueint;
    }
    item = cJSON_GetObjectItemCaseSensitive(root, "application_address");
    if (cJSON_IsString(item) && item->valuestring) {
        char *endptr;
        settings->application_address = strtoull(item->valuestring, &endptr, 10);
    }
    item = cJSON_GetObjectItemCaseSensitive(root, "ntp_domain");
    if (cJSON_IsString(item) && item->valuestring) {
        settings->ntp_domain = malloc(strlen(item->valuestring) + 1);
        strcpy(settings->ntp_domain, item->valuestring);
    } else {
        settings->ntp_domain = NULL;
    }
    item = cJSON_GetObjectItemCaseSensitive(root, "ota_domain");
    if (cJSON_IsString(item) && item->valuestring) {
        settings->ota_domain = malloc(strlen(item->valuestring) + 1);
        strcpy(settings->ota_domain, item->valuestring);
    } else {
        settings->ota_domain = NULL;
    }
    cJSON_Delete(root);
    return HAL_OK;
}

HAL_StatusTypeDef eeprom_load_settings(eeprom_settings_t *settings) {
    const HAL_StatusTypeDef result = eeprom_load_settings0(settings);
    char *default_ntp_domain = "ntp.aliyun.com";
    char *default_ota_domain = "api.hemeng.org";
    if (result != HAL_OK) {
        settings->ntp_domain = default_ntp_domain;
        settings->ota_domain = default_ota_domain;
    } else {
        if (settings->ntp_domain == NULL) {
            settings->ntp_domain = default_ntp_domain;
        }
        if (settings->ota_domain == NULL) {
            settings->ota_domain = default_ota_domain;
        }
    }
    return result;
}


HAL_StatusTypeDef eeprom_save_settings(
    const eeprom_settings_t *settings) {
    uint32_t magic_num = 0xFEFCDDDC;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ntp_domain", settings->ntp_domain);
    cJSON_AddNumberToObject(root, "is_upgrade", settings->is_upgrade);
    cJSON_AddNumberToObject(root, "start_flag", settings->start_flag);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%" PRIu32, settings->application_address);
    cJSON_AddStringToObject(root, "application_address", buffer);
    char *text = cJSON_PrintUnformatted(root);
    uint16_t len = strlen(text);
    uint16_t calculated_crc = CRC_Calculate(magic_num, text, len);
    HAL_StatusTypeDef result = EEPROM_Write(0, (uint8_t *) &magic_num, sizeof(magic_num));
    if (HAL_OK != result) {
        return result;
    }
    result = EEPROM_Write(sizeof(magic_num), (uint8_t *) &len, sizeof(len));
    if (HAL_OK != result) {
        return result;
    }
    result = EEPROM_Write(sizeof(magic_num) + sizeof(len), (uint8_t *) text, len);
    if (HAL_OK != result) {
        return result;
    }
    result = EEPROM_Write(sizeof(magic_num) + sizeof(len) + len, (uint8_t *) &calculated_crc, sizeof(calculated_crc));
    cJSON_free(text);
    cJSON_Delete(root);
    return result;
}
