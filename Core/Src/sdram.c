//
// Created by hemeng on 25-4-20.
//

#include "sdram.h"


/******************************************************************************************************
*	函 数 名: SDRAM_Initialization_Sequence
*	入口参数: hsdram - SDRAM_HandleTypeDef定义的变量，即表示定义的sdram
*				 Command	- 控制指令
*	返 回 值: 无
*	函数功能: SDRAM 参数配置
*	说    明: 配置SDRAM相关时序和控制方式
*******************************************************************************************************/

void SDRAM_InitConfig(SDRAM_HandleTypeDef *hsdram) {
    FMC_SDRAM_CommandTypeDef command; // 声明变量
    __IO uint32_t tmpmrd = 0;

    /* Configure a clock configuration enable command */
    command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE; // 开启SDRAM时钟
    command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT); // 发送控制指令
    HAL_Delay(1); // 延时等待

    /* Configure a PALL (precharge all) command */
    command.CommandMode = FMC_SDRAM_CMD_PALL; // 预充电命令
    command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT); // 发送控制指令

    /* Configure a Auto-Refresh command */
    command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE; // 使用自动刷新
    command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    command.AutoRefreshNumber = 8; // 自动刷新次数
    command.ModeRegisterDefinition = 0;

    HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT); // 发送控制指令

    /* Program the external memory mode register */
    tmpmrd = (uint32_t) SDRAM_MODEREG_BURST_LENGTH_2 |
             SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
             SDRAM_MODEREG_CAS_LATENCY_3 |
             SDRAM_MODEREG_OPERATING_MODE_STANDARD |
             SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE; // 加载模式寄存器命令
    command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
    command.AutoRefreshNumber = 1;
    command.ModeRegisterDefinition = tmpmrd;

    HAL_SDRAM_SendCommand(hsdram, &command, SDRAM_TIMEOUT); // 发送控制指令

    HAL_SDRAM_ProgramRefreshRate(hsdram, 918); // 配置刷新率
}
