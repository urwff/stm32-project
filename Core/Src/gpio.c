/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO引脚配置文件 - 提供所有使用的GPIO引脚的配置代码
  * @author  STMicroelectronics & 开发者
  * @date    2025-11-29
  * @version 1.0
  * 
  * @description
  * 本文件包含了项目中所有GPIO引脚的初始化配置代码，主要功能：
  * - 使能相关GPIO端口的时钟
  * - 配置SPI片选信号CS引脚
  * - 为其他外设预留GPIO配置空间
  * 
  * GPIO引脚分配：
  * - CS引脚：SPI设备片选信号，用于控制W25Q32等SPI设备
  * - 其他引脚：根据项目需求配置
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
 * @brief  GPIO引脚初始化函数
 * @param  无
 * @retval 无
 * 
 * @description
 * 配置项目中使用的所有GPIO引脚，包括：
 * - 模拟输入引脚 (Analog)
 * - 数字输入引脚 (Input) 
 * - 数字输出引脚 (Output)
 * - 事件输出引脚 (EVENT_OUT)
 * - 外部中断引脚 (EXTI)
 * 
 * 当前配置的引脚：
 * - CS引脚：SPI片选信号，推挽输出，高速模式
 * 
 * @note
 * 此函数由STM32CubeMX自动生成，手动修改可能会被覆盖
 */
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};  // GPIO初始化结构体

  /* 使能GPIO端口时钟 */
  __HAL_RCC_GPIOC_CLK_ENABLE();  // 使能GPIOC端口时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();  // 使能GPIOA端口时钟  
  __HAL_RCC_GPIOB_CLK_ENABLE();  // 使能GPIOB端口时钟

  /* 配置GPIO引脚输出电平 */
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);  // 设置CS引脚为高电平（SPI设备未选中状态）

  /* 配置CS引脚 - SPI片选信号 */
  GPIO_InitStruct.Pin = CS_Pin;                    // 引脚号
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      // 推挽输出模式
  GPIO_InitStruct.Pull = GPIO_NOPULL;              // 无上下拉电阻
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;    // 高速输出（50MHz）
  HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);   // 应用配置到指定端口

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
