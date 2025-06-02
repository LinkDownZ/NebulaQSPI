//
// Created by hemeng on 2025/5/7.
//

#include "uart_handler.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "i2c.h"
#include "stm32h7xx_hal.h"
#include "usart.h"
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
PUTCHAR_PROTOTYPE {
    HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, 0xFFFF);
    return ch;
}


#define UART_RX_BUFFER_SIZE 64

uint8_t uartRxIndex;
char uartRxBuffer[UART_RX_BUFFER_SIZE];
uint8_t uartReceiveByte;

uint8_t reset_flag = 0;
uint8_t reboot_flag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        /* 存储字节 */
        if (uartRxIndex < UART_RX_BUFFER_SIZE - 1) {
            uartRxBuffer[uartRxIndex++] = uartReceiveByte;

            /* 遇到换行或回车，视作命令结束 */
            if (uartReceiveByte == '\n' || uartReceiveByte == '\r') {
                uartRxBuffer[uartRxIndex - 1] = '\0';  // 用 '\0' 终结字符串

                /* 命令识别 */
                if (strcmp(uartRxBuffer, "reset") == 0) {
                    reset_flag = 1;
                } else if (strcmp(uartRxBuffer, "reboot") == 0) {
                    reboot_flag = 1;
                } else {
                    printf("Unknown cmd: %s\r\n", uartRxBuffer);
                }

                /* 回显并重置缓冲 */
                printf("CMD> %s\r\n", uartRxBuffer);
                memset(uartRxBuffer, 0, UART_RX_BUFFER_SIZE);
                uartRxIndex = 0;
            }
        } else {
            /* 缓冲区溢出 */
            printf("Error: UART Rx buffer overflow\r\n");
            memset(uartRxBuffer, 0, UART_RX_BUFFER_SIZE);
            uartRxIndex = 0;
        }

        /* 重新启动下一字节中断接收 */
        if (HAL_UART_Receive_IT(huart, &uartReceiveByte, 1) != HAL_OK) {
            printf("Error: HAL_UART_Receive_IT failed\r\n");
        }
    }
}


void uart_init() {
    const HAL_StatusTypeDef result = HAL_UART_Receive_IT(&huart1, &uartReceiveByte, 1);
    if (result != HAL_OK) {
        printf("uart init error\n");
    }
}


void UART_Process() {
    if (reset_flag) {
        printf("Reset...\n");
        eeprom_settings_t settings;
        settings.is_upgrade = 0;
        settings.application_address = QSPI_BASE;
        settings.start_flag = 0;
        EEPROM_Init(&hi2c1);
        const HAL_StatusTypeDef result = eeprom_save_settings(&settings);
        if (result != HAL_OK) {
            printf("保存失败 error code:%d\n", result);
        }
        NVIC_SystemReset();
    }
    if (reboot_flag) {
        printf("Rebooting...\n");
        NVIC_SystemReset();
    }
}
