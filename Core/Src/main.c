/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : 主程序文件 - STM32F103 W24C02 EEPROM测试程序
 * @description    : 本程序实现了对W24C02 EEPROM存储器的完整测试，包括：
 *                   - 单字节读写测试
 *                   - 多字节页写入测试  
 *                   - 数据持久性验证测试
 *                   - 通过UART串口输出测试结果
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "w24c02.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

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
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @brief  通过UART串口发送字符串消息
 * @param  msg: 要发送的字符串指针
 * @retval None
 * @note   使用UART1外设，阻塞方式发送，等待发送完成
 */
static void UART_Print(const char *msg) {
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/* USER CODE END 0 */

/**
  * @brief  应用程序入口点 - 主函数
  * @retval int 程序返回值（实际不会返回）
  * @note   本函数执行系统初始化和W24C02 EEPROM测试流程
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  
  // W24C02 EEPROM 测试代码
  uint8_t writeData = 0x5A;      // 测试写入的单字节数据
  uint8_t readData = 0;           // 读取的单字节数据
  uint8_t writeBuffer[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};  // 多字节写入测试数据
  uint8_t readBuffer[8] = {0};    // 多字节读取缓冲区
  char msgBuf[64];                // 串口消息缓冲区
  uint8_t testPass = 1;           // 测试结果标志
  
  // 初始化 W24C02 (使用寄存器版本)
  register_W24C02_Init();
  HAL_Delay(100);
  
  // 发送测试开始信息
  sprintf(msgBuf, "\r\n=== W24C02 EEPROM Test Start ===\r\n");
  UART_Print(msgBuf);
  
  // 测试1: 单字节写入和读取
  sprintf(msgBuf, "\r\n[Test 1] Single Byte Write/Read\r\n");
  UART_Print(msgBuf);
  
  register_W24C02_WriteByte(0x00, writeData);  // 在地址0x00写入0x5A
  HAL_Delay(10);
  readData = register_W24C02_ReadByte(0x00);   // 从地址0x00读取
  
  sprintf(msgBuf, "Write: 0x%02X, Read: 0x%02X ", writeData, readData);
  UART_Print(msgBuf);
  
  if (readData == writeData) {
    sprintf(msgBuf, "[PASS]\r\n");
  } else {
    sprintf(msgBuf, "[FAIL]\r\n");
    testPass = 0;
  }
  UART_Print(msgBuf);
  
  // 测试2: 多字节写入和读取 (页写入)
  sprintf(msgBuf, "\r\n[Test 2] Multi-Byte Write/Read (Page)\r\n");
  UART_Print(msgBuf);
  
  register_W24C02_WriteBytes(0x10, writeBuffer, 8);  // 从地址0x10开始写入8字节
  HAL_Delay(10);
  register_W24C02_ReadBytes(0x10, readBuffer, 8);    // 从地址0x10开始读取8字节
  
  sprintf(msgBuf, "Write: ");
  UART_Print(msgBuf);
  for (int i = 0; i < 8; i++) {
    sprintf(msgBuf, "0x%02X ", writeBuffer[i]);
    UART_Print(msgBuf);
  }
  
  sprintf(msgBuf, "\r\nRead:  ");
  UART_Print(msgBuf);
  for (int i = 0; i < 8; i++) {
    sprintf(msgBuf, "0x%02X ", readBuffer[i]);
    UART_Print(msgBuf);
  }
  
  // 验证多字节读写
  uint8_t multiBytePass = 1;
  for (int i = 0; i < 8; i++) {
    if (writeBuffer[i] != readBuffer[i]) {
      multiBytePass = 0;
      testPass = 0;
      break;
    }
  }
  
  if (multiBytePass) {
    sprintf(msgBuf, " [PASS]\r\n");
  } else {
    sprintf(msgBuf, " [FAIL]\r\n");
  }
  UART_Print(msgBuf);
  
  // 测试3: 数据持久性测试 - 写入不同值再读取
  sprintf(msgBuf, "\r\n[Test 3] Data Persistence Test\r\n");
  UART_Print(msgBuf);
  
  register_W24C02_WriteByte(0x20, 0xAB);
  HAL_Delay(10);
  register_W24C02_WriteByte(0x21, 0xCD);
  HAL_Delay(10);
  
  uint8_t val1 = register_W24C02_ReadByte(0x20);
  uint8_t val2 = register_W24C02_ReadByte(0x21);
  
  sprintf(msgBuf, "Addr 0x20: Write 0xAB, Read 0x%02X\r\n", val1);
  UART_Print(msgBuf);
  sprintf(msgBuf, "Addr 0x21: Write 0xCD, Read 0x%02X\r\n", val2);
  UART_Print(msgBuf);
  
  if (val1 == 0xAB && val2 == 0xCD) {
    sprintf(msgBuf, "[PASS]\r\n");
  } else {
    sprintf(msgBuf, "[FAIL]\r\n");
    testPass = 0;
  }
  UART_Print(msgBuf);
  
  // 测试总结
  sprintf(msgBuf, "\r\n=== Test Summary ===\r\n");
  UART_Print(msgBuf);
  
  if (testPass) {
    sprintf(msgBuf, "All tests PASSED!\r\n");
  } else {
    sprintf(msgBuf, "Some tests FAILED!\r\n");
  }
  UART_Print(msgBuf);
  
  sprintf(msgBuf, "=== W24C02 EEPROM Test End ===\r\n\r\n");
  UART_Print(msgBuf);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief  系统时钟配置函数
  * @retval None
  * @note   配置系统时钟为72MHz，使用外部高速晶振(HSE)和PLL
  *         - HSE: 8MHz外部晶振
  *         - PLL倍频: 9倍 (8MHz * 9 = 72MHz)
  *         - AHB分频: 2分频 (72MHz / 2 = 36MHz)
  *         - APB1和APB2: 不分频 (36MHz)
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};  // RCC振荡器初始化结构体
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};  // RCC时钟初始化结构体

  /** 根据RCC_OscInitTypeDef结构体中指定的参数初始化RCC振荡器
  * 配置外部高速晶振(HSE)和锁相环(PLL)
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;  // 使用HSE振荡器
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                    // 启用HSE
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;     // HSE预分频为1
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                    // 启用HSI(内部振荡器)
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;               // 启用PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;       // PLL时钟源为HSE
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;              // PLL倍频系数为9
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)       // 配置振荡器
  {
    Error_Handler();  // 配置失败则进入错误处理
  }

  /** 初始化CPU、AHB和APB总线时钟
  * 配置系统时钟源和各总线的分频系数
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;  // 配置所有时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;              // 系统时钟源为PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;                     // AHB时钟2分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                      // APB1时钟不分频
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;                      // APB2时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) // 配置时钟和Flash延迟
  {
    Error_Handler();  // 配置失败则进入错误处理
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  错误处理函数 - 当发生错误时执行此函数
  * @retval None
  * @note   禁用所有中断并进入无限循环，用于调试错误情况
  *         在实际应用中可以添加错误指示(如LED闪烁)或系统复位
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* 用户可以在此添加自己的错误状态报告实现 */
  __disable_irq();  // 禁用全局中断
  while (1) {       // 进入无限循环，等待调试或看门狗复位
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  断言失败报告函数 - 报告发生断言错误的源文件名和行号
  * @param  file: 指向源文件名的指针
  * @param  line: 断言错误发生的源代码行号
  * @retval None
  * @note   仅在启用USE_FULL_ASSERT宏时编译此函数
  *         用于调试时定位参数错误的具体位置
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* 用户可以在此添加自己的实现来报告文件名和行号
     例如: printf("参数值错误: 文件 %s 第 %d 行\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
