/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
uint8_t g_usart_rx_buffer[100] = {0};
volatile uint8_t g_usart_rx_len = 0;
volatile uint8_t g_usart_message_ready = 0;

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  NVIC_SetPriority(USART1_IRQn, 2);
  NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief  USART1初始化函数
 * @param  无
 * @retval 无
 *
 * @description
 * 初始化USART1串口通信，配置GPIO引脚和USART参数
 *
 * 配置步骤：
 * 1. 使能USART1和GPIOA时钟
 * 2. 配置PA9为复用推挽输出(TX)，PA10为浮空输入(RX)
 * 3. 设置波特率为115200bps
 * 4. 配置数据格式：8位数据，无校验，1位停止位
 * 5. 使能发送器和接收器
 */
void Driver_USART1_Init(void) {
  /* 1. 使能时钟 */
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // 使能USART1时钟
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;   // 使能GPIOA时钟

  /* 2. 配置GPIO引脚的工作模式
     PA9=Tx(复用推挽输出 CNF=10 MODE=11)
     PA10=Rx(浮空输入 CNF=01 MODE=00) */

  // 配置PA9为复用推挽输出，最大速度50MHz
  GPIOA->CRH &= ~GPIO_CRH_CNF9;  // 清除CNF9位
  GPIOA->CRH |= GPIO_CRH_CNF9_1; // CNF9 = 10 (复用推挽)
  GPIOA->CRH |= GPIO_CRH_MODE9;  // MODE9 = 11 (输出模式，最大速度50MHz)

  // 配置PA10为浮空输入
  GPIOA->CRH &= ~GPIO_CRH_CNF10_1; // CNF10_1 = 0
  GPIOA->CRH |= GPIO_CRH_CNF10_0;  // CNF10_0 = 1，CNF10 = 01 (浮空输入)
  GPIOA->CRH &= ~GPIO_CRH_MODE10;  // MODE10 = 00 (输入模式)

  /* 3. 配置USART1参数 */
  USART1->BRR =
      0x138; // 波特率设置为115200bps (PCLK2=36MHz)
             // BRR = PCLK2/(16*波特率) = 36000000/(16*115200) ≈ 19.53 = 0x138

  /* 3.3 配置数据字长度为8位 */
  USART1->CR1 &= ~USART_CR1_M; // M=0，8位数据长度

  /* 3.4 配置无校验位 */
  USART1->CR1 &= ~USART_CR1_PCE; // PCE=0，禁用校验

  /* 3.5 配置停止位长度为1位 */
  USART1->CR2 &= ~USART_CR2_STOP; // STOP=00，1个停止位

  /* 3.6 使能接收中断和空闲中断 */
  USART1->CR1 |= USART_CR1_RXNEIE | USART_CR1_IDLEIE;

  /* 3.7 使能发送器(TE)和接收器(RE) */
  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;

  /* 3.8 最后使能USART1外设 (UE必须在所有配置完成后再使能) */
  USART1->CR1 |= USART_CR1_UE;

  /* 4. 配置NVIC中断 */
  NVIC_SetPriorityGrouping(3);      // 设置优先级分组为3
  NVIC_SetPriority(USART1_IRQn, 2); // 设置中断优先级
  NVIC_EnableIRQ(USART1_IRQn);      // 使能USART1中断
}

/**
 * @brief  发送单个字符
 * @param  byte: 要发送的字符
 * @retval 无
 *
 * @description
 * 通过USART1发送一个字符，函数会等待发送缓冲区空闲后再发送数据
 */
void Driver_USART1_SendChar(char byte) {
  while (!(USART1->SR & USART_SR_TXE)) // 等待发送数据寄存器空(TXE=1)
    ;
  USART1->DR = byte; // 将数据写入数据寄存器
}

/**
 * @brief  发送字符串
 * @param  str: 指向要发送的字符串的指针
 * @param  len: 要发送的字符串长度
 * @retval 无
 *
 * @description
 * 通过USART1发送指定长度的字符串，逐个字符发送
 */
void Driver_USART1_SendString(uint8_t *str, uint16_t len) {
  for (int i = 0; i < len; i++) {
    Driver_USART1_SendChar(str[i]); // 逐个发送字符
  }
}

/**
 * @brief  接收单个字符
 * @param  无
 * @retval 接收到的字符
 *
 * @description
 * 从USART1接收一个字符，函数会等待接收缓冲区有数据后再读取
 */
uint8_t Driver_USART1_ReceiveChar(void) {
  while (!(USART1->SR & USART_SR_RXNE)) // 等待接收数据寄存器非空(RXNE=1)
    ;
  return (uint8_t)(USART1->DR); // 从数据寄存器读取接收到的数据
}

/**
 * @brief  接收字符串
 * @param  buff: 存储接收数据的缓冲区
 * @param  len: 指向存储接收长度的变量的指针
 * @retval 无
 *
 * @description
 * 从USART1接收字符串，直到遇到回车符('\r')或换行符('\n')为止
 * 接收到的字符存储在buff中，实际接收长度通过len参数返回
 */
void Driver_USART1_ReceiveString(uint8_t buff[], uint8_t *len) {
  uint8_t i = 0;
  while (1) {
    while ((USART1->SR & USART_SR_RXNE) == 0) {
      if (USART1->SR & USART_SR_IDLE) {
        *len = i;
        return;
      }
    }
    buff[i] = USART1->DR;
    i++;
  }
}
/* USER CODE END 1 */
