/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
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
#include "spi.h"
#include "main.h"
#include "stm32f103xe.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
 * @brief  启动HAL库方式的SPI通信（拉低CS片选信号）
 * @param  None
 * @retval None
 */
void Hal_SPI_Start(void){
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  停止HAL库方式的SPI通信（拉高CS片选信号）
 * @param  None
 * @retval None
 */
void Hal_SPI_Stop(void){
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief  使用HAL库进行SPI数据交换
 * @param  byte: 要发送的字节数据
 * @retval receivedByte: 接收到的字节数据
 */
uint8_t Hal_SPI_SwapByte(uint8_t byte) {
  uint8_t receivedByte = 0;
  if (HAL_SPI_TransmitReceive(&hspi1, &byte, &receivedByte, 1, 2000) != HAL_OK) {
    Error_Handler();
  }
  return receivedByte;
}

/**
 * @brief  启动寄存器方式的SPI通信（拉低CS片选信号）
 * @param  None
 * @retval None
 * @note   直接操作GPIOC的ODR寄存器，PC13为CS引脚
 */
void Register_SPI_Start(void){
    GPIOC->ODR &= ~GPIO_ODR_ODR13;  // 清除PC13位，拉低CS引脚
}

/**
 * @brief  停止寄存器方式的SPI通信（拉高CS片选信号）
 * @param  None
 * @retval None
 * @note   直接操作GPIOC的ODR寄存器，PC13为CS引脚
 */
void Register_SPI_Stop(void){
    GPIOC->ODR |= GPIO_ODR_ODR13;   // 置位PC13位，拉高CS引脚
}

/**
 * @brief  使用寄存器方式进行SPI数据交换
 * @param  byte: 要发送的字节数据
 * @retval 接收到的字节数据
 * @note   直接操作SPI1寄存器实现数据收发
 */
uint8_t Register_SPI_SwapByte(uint8_t byte){
  // 等待发送缓冲区为空（TXE位为1表示空闲）
  while ((SPI1->SR & SPI_SR_TXE) == 0) ;
  
  // 发送数据到SPI数据寄存器
  SPI1->DR = byte;
  
  // 等待接收缓冲区非空（RXNE位为1表示有数据）
  while ((SPI1->SR & SPI_SR_RXNE)==0) ;
  
  // 读取接收到的数据并返回
  return (uint8_t)(SPI1->DR& 0xFF);
}
/* USER CODE END 1 */
