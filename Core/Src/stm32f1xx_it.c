/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @brief   STM32F1xx中断服务程序 - 处理系统和外设中断
 * @author  STMicroelectronics & 开发者
 * @date    2025-11-29
 * @version 1.0
 * 
 * @description
 * 本文件包含了STM32F1xx系列微控制器的中断服务程序，主要功能：
 * - Cortex-M3内核异常处理程序
 * - 外设中断服务程序
 * - USART1中断处理（包含自定义接收逻辑）
 * 
 * 中断处理功能：
 * - 系统异常：NMI、HardFault、MemManage、BusFault、UsageFault等
 * - 系统服务：SVC、PendSV、SysTick等
 * - 外设中断：USART1全局中断
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
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief  不可屏蔽中断处理程序
  * @param  无
  * @retval 无
  * 
  * @description
  * 处理不可屏蔽中断(NMI)，通常由外部NMI引脚或内部时钟安全系统触发
  * 这是最高优先级的中断，无法被屏蔽
  * 
  * @note
  * 发生NMI通常表示系统出现严重问题，需要立即处理
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1) {  // 进入无限循环，等待系统复位或调试
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief  硬件错误中断处理程序
  * @param  无
  * @retval 无
  * 
  * @description
  * 处理硬件错误中断，当处理器遇到无法处理的错误时触发
  * 常见原因包括：
  * - 访问无效内存地址
  * - 执行无效指令
  * - 堆栈溢出
  * - 总线错误等
  * 
  * @note
  * 发生HardFault通常表示程序存在严重错误，需要调试分析
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)  // 进入无限循环，保持系统状态便于调试
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief  USART1全局中断处理程序
  * @param  无
  * @retval 无
  * 
  * @description
  * 处理USART1的所有中断事件，包括：
  * - 接收数据中断 (RXNE)
  * - 发送数据中断 (TXE)
  * - 传输完成中断 (TC)
  * - 空闲线路检测中断 (IDLE)
  * - 错误中断等
  * 
  * 自定义功能：
  * - 实现字符串接收缓冲
  * - 空闲线路检测用于判断消息结束
  * - 缓冲区溢出保护
  * 
  * @note
  * 先调用HAL库标准处理程序，再执行自定义逻辑
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);  // 调用HAL库标准中断处理程序
  /* USER CODE BEGIN USART1_IRQn 1 */
  
  // 检查是否接收到数据 (RXNE标志位)
  if ((USART1->SR & USART_SR_RXNE) != 0) {
    if (g_usart_rx_len < sizeof(g_usart_rx_buffer)) {
      // 缓冲区未满，存储接收到的字节
      g_usart_rx_buffer[g_usart_rx_len++] = (uint8_t)USART1->DR;
    } else {
      // 缓冲区已满，丢弃数据但仍需读取DR寄存器清除RXNE标志
      volatile uint32_t discard = USART1->DR;
      (void)discard;  // 避免编译器警告
    }
  }

  // 检查线路是否空闲 (IDLE标志位)，表示一次传输结束
  if ((USART1->SR & USART_SR_IDLE) != 0) {
    // 清除IDLE标志：先读SR寄存器，再读DR寄存器
    volatile uint32_t temp_val = USART1->SR; // 读取状态寄存器
    temp_val = USART1->DR;                   // 读取数据寄存器完成清除序列
    (void)temp_val;                          // 避免编译器警告
    g_usart_message_ready = 1;               // 设置消息接收完成标志
  }
  /* USER CODE END USART1_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
