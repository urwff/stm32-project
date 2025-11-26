/**
 * @file    my_usart.c
 * @brief   STM32F103 USART1驱动程序实现
 * @author  开发者
 * @date    2025-11-26
 * @version 1.0
 * 
 * @description
 * 本文件实现了STM32F103芯片的USART1串口通信驱动程序，包括：
 * - USART1初始化配置
 * - 单字符发送和接收
 * - 字符串发送和接收
 * 
 * 硬件连接：
 * - PA9:  USART1_TX (发送引脚)
 * - PA10: USART1_RX (接收引脚)
 * 
 * 通信参数：
 * - 波特率: 115200 bps
 * - 数据位: 8位
 * - 停止位: 1位
 * - 校验位: 无
 */

#include "my_usart.h"
#include "stm32f103xe.h"

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
  GPIOA->CRH &= ~GPIO_CRH_CNF9;         // 清除CNF9位
  GPIOA->CRH |= GPIO_CRH_CNF9_1;        // CNF9 = 10 (复用推挽)
  GPIOA->CRH |= GPIO_CRH_MODE9;         // MODE9 = 11 (输出模式，最大速度50MHz)

  // 配置PA10为浮空输入
  GPIOA->CRH &= ~GPIO_CRH_CNF10_1;      // CNF10_1 = 0
  GPIOA->CRH |= GPIO_CRH_CNF10_0;       // CNF10_0 = 1，CNF10 = 01 (浮空输入)
  GPIOA->CRH &= ~GPIO_CRH_MODE10;       // MODE10 = 00 (输入模式)

  /* 3. 配置USART1参数 */
  USART1->BRR = 0x271;                  // 波特率设置为115200bps (假设PCLK2=72MHz)
                                        // BRR = PCLK2/(16*波特率) = 72000000/(16*115200) ≈ 625 = 0x271
  
  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE; // 使能发送器(TE)和接收器(RE)
  USART1->CR1 |= USART_CR1_UE;                // 使能USART1外设
  
  /* 3.3 配置数据字长度为8位 */
  USART1->CR1 &= ~USART_CR1_M;          // M=0，8位数据长度
  
  /* 3.4 配置无校验位 */
  USART1->CR1 &= ~USART_CR1_PCE;        // PCE=0，禁用校验
  
  /* 3.5 配置停止位长度为1位 */
  USART1->CR2 &= ~USART_CR2_STOP;       // STOP=00，1个停止位
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
  while (!(USART1->SR & USART_SR_TXE))  // 等待发送数据寄存器空(TXE=1)
    ;
  USART1->DR = byte;                    // 将数据写入数据寄存器
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
void Driver_USART1_SendString(uint8_t *str, uint16_t len){
  for (int i = 0; i < len; i++) {
    Driver_USART1_SendChar(str[i]);     // 逐个发送字符
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
uint8_t Driver_USART1_ReceiveChar(void){
  while (!(USART1->SR & USART_SR_RXNE)) // 等待接收数据寄存器非空(RXNE=1)
    ;
  return (uint8_t)(USART1->DR);         // 从数据寄存器读取接收到的数据
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
void Driver_USART1_ReceiveString(uint8_t buff[], uint8_t *len){
  uint8_t i = 0;
  uint8_t ch;
  while (1) {
    ch = Driver_USART1_ReceiveChar();   // 接收一个字符
    if (ch == '\r' || ch == '\n') {    // 遇到回车或换行符结束接收
      break;
    }
    buff[i++] = ch;                     // 将字符存入缓冲区
  }
  *len = i;                             // 返回接收到的字符长度
}