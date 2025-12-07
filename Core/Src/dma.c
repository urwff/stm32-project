/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma.c
  * @brief   This file provides code for the configuration
  *          of all the requested memory to memory DMA transfers.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "dma.h"
#include "stm32f103xe.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure DMA                                                              */
/*----------------------------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

/**
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma1_channel1
  */
void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma1_channel1 on DMA1_Channel1 */
  hdma_memtomem_dma1_channel1.Instance = DMA1_Channel1;
  hdma_memtomem_dma1_channel1.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel1.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel1.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma1_channel1.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_memtomem_dma1_channel1.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel1.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel1) != HAL_OK)
  {
    Error_Handler();
  }

}

/* USER CODE BEGIN 2 */

/**
 * @brief  根据 DMA_Config_t 中的参数初始化指定的 DMA 通道。
 * @param  dma_channel: DMA 通道的基地址 (如 DMA1_Channel1 等)
 * @param  cfg: 指向包含配置信息的 DMA_Config_t 结构体的指针
 * @retval 0: 成功, -1: 失败
 */
int DMA_Init(DMA_Channel_TypeDef* dma_channel, DMA_Config_t* cfg) {
    uint32_t tmpreg = 0;

    /* 参数检查 */
    if (dma_channel == 0 || cfg == 0) {
        return -1;
    }

    /* -------------------------------------------------------------------------
     * 步骤 1: 禁用 DMA 通道
     * ------------------------------------------------------------------------- */
    /* 在配置通道之前，必须将其禁用以确保没有正在进行的传输。 */
    
    // TODO: 清除 CCR 寄存器中的 EN 位 (bit 0) 以禁用通道。
    // 寄存器: dma_channel->CCR
    // 操作: dma_channel->CCR &= ~DMA_CCR_EN
    dma_channel->CCR &= ~DMA_CCR_EN;
    /* -------------------------------------------------------------------------
     * 步骤 2: 配置外设和存储器地址
     * ------------------------------------------------------------------------- */
    /* 设置外设寄存器地址和存储器缓冲区地址。 */
    
    // TODO: 将外设基地址写入 CPAR 寄存器。
    // 寄存器: dma_channel->CPAR
    // 操作: dma_channel->CPAR = cfg->PeriphBaseAddr
    dma_channel->CPAR = cfg->PeriphBaseAddr;
    // TODO: 将存储器基地址写入 CMAR 寄存器。
    // 寄存器: dma_channel->CMAR
    // 操作: dma_channel->CMAR = cfg->MemBaseAddr
    dma_channel->CMAR = cfg->MemBaseAddr;
    
    /* -------------------------------------------------------------------------
     * 步骤 3: 配置传输数据量
     * ------------------------------------------------------------------------- */
    /* 设置要传输的数据项数量。有效范围: 0 到 65535。 */
    
    // TODO: 将缓冲区大小写入 CNDTR 寄存器。
    // 寄存器: dma_channel->CNDTR
    // 操作: dma_channel->CNDTR = cfg->BufferSize
    dma_channel->CNDTR = cfg->BufferSize;
    /* -------------------------------------------------------------------------
     * 步骤 4: 配置通道控制寄存器 (CCR)
     * ------------------------------------------------------------------------- */
    /* 准备 CCR 寄存器的配置逻辑。
       我们将首先在临时变量 'tmpreg' 中构建该值。 */

    /* 4.1 配置数据传输方向 (DIR 位) */
    // TODO: 根据 cfg->Direction 设置 DIR 位
    // Bit 4 (DIR): 0 = 从外设读取, 1 = 从存储器读取
    if (cfg->Direction == DMA_DIR_PeripheralDST_Mem2Per) {
        tmpreg |= DMA_CCR_DIR;
    }

    /* 4.2 配置循环模式 (CIRC 位) */
    // TODO: 根据 cfg->Mode 设置 CIRC 位
    // Bit 5 (CIRC): 0 = 普通模式, 1 = 循环模式
    if (cfg->Mode == DMA_Mode_Circular) {
        tmpreg |= DMA_CCR_CIRC;
    }

    /* 4.3 配置外设地址增量 (PINC 位) */
    // TODO: 根据 cfg->PeriphInc 设置 PINC 位
    // Bit 6 (PINC): 0 = 禁用, 1 = 使能
    if (cfg->PeriphInc == DMA_Inc_Enable) {
        tmpreg |= DMA_CCR_PINC;
    }

    /* 4.4 配置存储器地址增量 (MINC 位) */
    // TODO: 根据 cfg->MemInc 设置 MINC 位
    // Bit 7 (MINC): 0 = 禁用, 1 = 使能
    if (cfg->MemInc == DMA_Inc_Enable) {
        tmpreg |= DMA_CCR_MINC;
    }

    /* 4.5 配置外设数据宽度 (PSIZE 位) */
    // TODO: 根据 cfg->PeriphDataSize 设置 PSIZE 位 [9:8]
    // 00=8位, 01=16位, 10=32位
    tmpreg |= (cfg->PeriphDataSize << 8);

    /* 4.6 配置存储器数据宽度 (MSIZE 位) */
    // TODO: 根据 cfg->MemDataSize 设置 MSIZE 位 [11:10]
    // 00=8位, 01=16位, 10=32位
    tmpreg |= (cfg->MemDataSize << 10);

    /* 4.7 配置通道优先级 (PL 位) */
    // TODO: 根据 cfg->Priority 设置 PL 位 [13:12]
    // 00=低, 01=中, 10=高, 11=非常高
    tmpreg |= (cfg->Priority << 12);

    /* 4.8 配置存储器到存储器模式 (MEM2MEM 位) */
    // TODO: 根据 cfg->M2M 设置 MEM2MEM 位 (bit 14)
    // 0=禁用, 1=使能
    if (cfg->M2M) {
        tmpreg |= DMA_CCR_MEM2MEM;
    }

    /* 将配置写入寄存器 */
    // TODO: 将 tmpreg 写入 CCR 寄存器 (注意: 确保 EN 为 0，我们在步骤 1 中已做此操作)
    // 寄存器: dma_channel->CCR
    // 操作: dma_channel->CCR = tmpreg
    dma_channel->CCR = tmpreg;

    return 0;
}

/**
 * @brief  使能或禁用指定的 DMA 通道。
 * @param  dma_channel: DMA 通道的基地址。
 * @param  state: DMA 通道的新状态 (ENABLE/true 或 DISABLE/false)。
 * @retval 无
 */
void DMA_Cmd(DMA_Channel_TypeDef* dma_channel, bool state) {
    if (dma_channel == 0) return;

    if (state) {
        // TODO: 设置 EN 位 (bit 0) 以使能通道
        // 寄存器: dma_channel->CCR
        // 操作: dma_channel->CCR |= DMA_CCR_EN
        dma_channel->CCR |= DMA_CCR_EN;
    } else {
        // TODO: 清除 EN 位 (bit 0) 以禁用通道
        // 寄存器: dma_channel->CCR
        // 操作: dma_channel->CCR &= ~DMA_CCR_EN
        dma_channel->CCR &= ~DMA_CCR_EN;
    }
}

/**
 * @brief  返回当前 DMAy Channelx 传输中剩余的数据单元数量。
 * @param  dma_channel: DMA 通道的基地址。
 * @retval 剩余的数据单元数量。
 */
uint16_t DMA_GetCurrDataCounter(DMA_Channel_TypeDef* dma_channel) {
    if (dma_channel == 0) return 0;
    
    // TODO: 读取 CNDTR 寄存器
    // 寄存器: dma_channel->CNDTR
    // 操作: return (uint16_t)(dma_channel->CNDTR)
    
    return 0; // 占位符
}

/**
 * @brief  检查指定的 DMAy Channelx 标志位是否置位。
 * @param  flag: 指定要检查的标志位。
 * @retval 标志位的新状态 (1 表示 SET, 0 表示 RESET)。
 */
uint8_t DMA_GetFlagStatus(uint32_t flag) {
    /* 在 DMA1 ISR 中检查状态 */
    // 注意: 如果使用 DMA2，逻辑需要根据标志/通道检查 DMA2->ISR。
    // 为了简单起见，假设使用 DMA1 或通过标志参数全局处理。
    
    // TODO: 读取 DMA1 中断状态寄存器
    // 寄存器: DMA1->ISR (或 DMA2->ISR)
    // 操作: 检查 if (DMA1->ISR & flag) != 0
    
    // 示例逻辑 (占位符):
    // if ((DMA1->ISR & flag) != 0) {
    //     return 1;
    // }
    
    return 0;
}

/**
 * @brief  清除 DMAy Channelx 的挂起标志位。
 * @param  flag: 指定要清除的标志位。
 * @retval 无
 */
void DMA_ClearFlag(uint32_t flag) {
    /* 在 DMA1 IFCR 中清除状态 */
    // 注意: 关于 DMA2 的说明同上。
    
    // TODO: 写入 DMA1 中断标志清除寄存器
    // 寄存器: DMA1->IFCR (或 DMA2->IFCR)
    // 操作: DMA1->IFCR = flag
}

/* USER CODE END 2 */

