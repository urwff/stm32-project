/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dma.h
  * @brief   This file contains all the function prototypes for
  *          the dma.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* DMA memory to memory transfer handles -------------------------------------*/
extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

/* USER CODE BEGIN Includes */
#include <stdbool.h>

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/** 
 * @brief  DMA 数据传输方向
 */
typedef enum {
    DMA_DIR_PeripheralDST = 0,      /*!< 外设作为目标 (P->M 读取外设? 注意：此处需根据实际寄存器定义确认) 
                                         注意：STM32F1 DMA_CCR DIR 位: 0 = 从外设读取 (P->M), 1 = 从存储器读取 (M->P) */
    DMA_DIR_PeripheralSRC = 0,      /*!< 数据方向：外设到存储器 (CCR.DIR = 0) */
    DMA_DIR_PeripheralDST_Mem2Per = 1 /*!< 数据方向：存储器到外设 (CCR.DIR = 1) */
} DMA_Direction_TypeDef;

/** 
 * @brief  DMA 数据宽度
 */
typedef enum {
    DMA_DataSize_Byte     = 0,      /*!< 8位字节 (00) */
    DMA_DataSize_HalfWord = 1,      /*!< 16位半字 (01) */
    DMA_DataSize_Word     = 2       /*!< 32位字 (10) */
} DMA_DataSize_TypeDef;

/** 
 * @brief  DMA 模式
 */
typedef enum {
    DMA_Mode_Normal   = 0,          /*!< 普通模式 (单次传输) */
    DMA_Mode_Circular = 1           /*!< 循环模式 */
} DMA_Mode_TypeDef;

/** 
 * @brief  DMA 优先级
 */
typedef enum {
    DMA_Priority_Low       = 0,     /*!< 低优先级 */
    DMA_Priority_Medium    = 1,     /*!< 中等优先级 */
    DMA_Priority_High      = 2,     /*!< 高优先级 */
    DMA_Priority_VeryHigh  = 3      /*!< 非常高优先级 */
} DMA_Priority_TypeDef;

/** 
 * @brief  DMA 地址增量模式
 */
typedef enum {
    DMA_Inc_Disable = 0,            /*!< 地址固定 */
    DMA_Inc_Enable  = 1             /*!< 每次传输后地址自动递增 */
} DMA_Inc_TypeDef;

/** 
 * @brief  DMA 配置结构体
 */
typedef struct {
    uint32_t              PeriphBaseAddr; /*!< 外设数据寄存器基地址 */
    uint32_t              MemBaseAddr;    /*!< 存储器缓冲区基地址 */
    DMA_Direction_TypeDef Direction;      /*!< 传输方向 (P->M 或 M->P) */
    uint16_t              BufferSize;     /*!< 待传输的数据量 (0-65535) */
    DMA_Inc_TypeDef       PeriphInc;      /*!< 外设地址增量模式 */
    DMA_Inc_TypeDef       MemInc;         /*!< 存储器地址增量模式 */
    DMA_DataSize_TypeDef  PeriphDataSize; /*!< 外设数据宽度 */
    DMA_DataSize_TypeDef  MemDataSize;    /*!< 存储器数据宽度 */
    DMA_Mode_TypeDef      Mode;           /*!< 普通或循环模式 */
    DMA_Priority_TypeDef  Priority;       /*!< 软件优先级 */
    bool                  M2M;            /*!< 存储器到存储器模式使能 */
} DMA_Config_t;

/* Exported constants --------------------------------------------------------*/
/* DMA 标志位定义，用于 DMA_GetFlagStatus 和 DMA_ClearFlag */
#define DMA1_FLAG_GL1                      ((uint32_t)0x00000001) /*!< DMA1 通道1 全局中断标志 */
#define DMA1_FLAG_TC1                      ((uint32_t)0x00000002) /*!< DMA1 通道1 传输完成标志 */
#define DMA1_FLAG_HT1                      ((uint32_t)0x00000004) /*!< DMA1 通道1 半传输标志 */
#define DMA1_FLAG_TE1                      ((uint32_t)0x00000008) /*!< DMA1 通道1 传输错误标志 */
/* USER CODE END Private defines */

void MX_DMA_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief  根据 DMA_Config_t 中的参数初始化指定的 DMA 通道。
 * @param  dma_channel: DMA 通道的基地址 (如 DMA1_Channel1 等)
 * @param  cfg: 指向包含配置信息的 DMA_Config_t 结构体的指针
 * @retval 0: 成功, -1: 失败 (例如空指针或无效参数)
 */
int DMA_Init(DMA_Channel_TypeDef* dma_channel, DMA_Config_t* cfg);

/**
 * @brief  使能或禁用指定的 DMA 通道。
 * @param  dma_channel: DMA 通道的基地址
 * @param  state: DMA 通道的新状态 (ENABLE/true 或 DISABLE/false)
 * @retval 无
 */
void DMA_Cmd(DMA_Channel_TypeDef* dma_channel, bool state);

/**
 * @brief  返回当前 DMAy Channelx 传输中剩余的数据单元数量。
 * @param  dma_channel: DMA 通道的基地址
 * @retval 剩余的数据单元数量
 */
uint16_t DMA_GetCurrDataCounter(DMA_Channel_TypeDef* dma_channel);

/**
 * @brief  检查指定的 DMAy Channelx 标志位是否置位。
 * @param  flag: 指定要检查的标志位。
 *         该参数可以是 DMA_FLAG_xxx 常量之一。
 * @retval 标志位的新状态 (SET 或 RESET)。
 */
uint8_t DMA_GetFlagStatus(uint32_t flag);

/**
 * @brief  清除 DMAy Channelx 的挂起标志位。
 * @param  flag: 指定要清除的标志位。
 *         该参数可以是 DMA_FLAG_xxx 常量的任意组合。
 * @retval 无
 */
void DMA_ClearFlag(uint32_t flag);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA_H__ */

