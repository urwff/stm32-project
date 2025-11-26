/**
 * @file    w24c02.c
 * @brief   W24C02 EEPROM驱动程序实现
 * @author  开发者
 * @date    2025-11-26
 * @version 1.0
 *
 * @description
 * 本文件实现了W24C02 EEPROM存储器的驱动程序，基于STM32 HAL库的I2C接口
 *
 * W24C02特性：
 * - 容量: 2Kbit (256字节)
 * - 接口: I2C总线
 * - 地址: 0xA0 (写) / 0xA1 (读)
 * - 页大小: 8字节
 * - 写周期时间: 5ms (典型值)
 *
 * 支持功能：
 * - 单字节读写
 * - 多字节页写入
 * - 多字节连续读取
 */

#include "w24c02.h"
#include "i2c.h"
#include "regster_i2c.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"

/**
 * @brief  W24C02初始化函数
 * @param  无
 * @retval 无
 *
 * @description
 * 初始化W24C02 EEPROM，主要是初始化I2C2接口
 */
void Hal_W24C02_Init(void) {
  MX_I2C2_Init(); // 初始化I2C2接口
}

/**
 * @brief  使用寄存器方式初始化W24C02
 * @param  无
 * @retval 无
 *
 * @description
 * 使用寄存器直接操作方式初始化W24C02 EEPROM的I2C2接口
 * 
 * @details
 * 该函数直接操作STM32的寄存器来配置I2C2外设，包括：
 * 1. 时钟使能：使能I2C2和GPIOB的时钟
 * 2. GPIO配置：配置PB6(SCL)和PB7(SDA)为I2C复用功能
 * 3. I2C参数配置：设置I2C时钟频率为100kHz标准模式
 * 
 * @note
 * - SCL引脚：PB6 (I2C2_SCL)
 * - SDA引脚：PB7 (I2C2_SDA)
 * - I2C频率：100kHz (标准模式)
 * - 系统时钟：36MHz (APB1)
 */
void regisrter_W24C02_Init(void) {
  // 使能I2C2和GPIO时钟
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // 使能I2C2时钟，APB1外设时钟使能寄存器
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // 使能GPIOB时钟，APB2外设时钟使能寄存器

  // 配置GPIOB的引脚6(SCL)和引脚7(SDA)为复用开漏输出
  GPIOB->CRL &= ~(GPIO_CRL_MODE6 | GPIO_CRL_MODE7); // 清除PB6和PB7的MODE位，准备重新配置
  GPIOB->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_CNF7);   // 清除PB6和PB7的CNF位，准备重新配置

  GPIOB->CRL |= GPIO_CRL_MODE6_1 |
                GPIO_CRL_MODE7_1; // 设置SCL和SDA为输出模式，最大速度10MHz (MODE = 10)
  GPIOB->CRL |= GPIO_CRL_CNF6_1 | GPIO_CRL_CNF7_1; // 设置SCL和SDA为复用开漏输出 (CNF = 10)

  // 配置I2C2参数
  I2C2->CR1 &= ~I2C_CR1_PE; // 关闭I2C2以允许配置，必须先关闭PE位才能修改配置寄存器
  
  // 设置I2C时钟频率
  I2C2->CR2 |= 36;          // 设置APB1时钟频率为36MHz，用于计算I2C时钟分频
                          // CR2[5:0] = FREQ = PCLK1频率(MHz) = 36MHz
                          
  I2C2->CCR = 180;          // 设置时钟控制寄存器，标准模式100kHz
                          // 计算公式：CCR = PCLK1/(2*I2C_CLK) = 36MHz/(2*100kHz) = 180
                          // CCR[11:0] = 180，标准模式下的时钟分频系数
                          
  I2C2->TRISE = 37;         // 设置上升时间寄存器
                          // 计算公式：TRISE = [300ns * PCLK1] + 1 = [300ns * 36MHz] + 1 = 11 + 1 = 37
                          // 用于I2C信号上升沿的滤波时间

  I2C2->CR1 |= I2C_CR1_PE; // 重新使能I2C2，开始工作
}

/**
 * @brief  向W24C02写入单个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @param  byte: 要写入的数据字节
 * @retval 无
 *
 * @description
 * 向指定地址写入一个字节的数据，使用STM32 HAL库函数
 * 
 * @details
 * 该函数使用HAL_I2C_Mem_Write函数进行写入操作：
 * - 设备地址：ADDR (0xA0，写模式)
 * - 内部地址：8位地址模式
 * - 数据长度：1字节
 * - 超时时间：2000ms
 * 
 * @note
 * - 写入后必须等待5ms让EEPROM完成内部写周期
 * - W24C02的写周期典型值为5ms，最大可达10ms
 * - 在写周期期间，EEPROM不会响应I2C通信
 */
void Hal_W24C02_WriteByte(uint8_t innerAddr, uint8_t byte) {
  HAL_I2C_Mem_Write(&hi2c2, ADDR, innerAddr, I2C_MEMADD_SIZE_8BIT, &byte, 1,
                    2000); // 使用HAL库写入数据，超时时间2000ms

  HAL_Delay(5); // 等待EEPROM完成写周期 (典型值5ms)
}

/**
 * @brief  从W24C02读取单个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @retval 读取到的数据字节
 *
 * @description
 * 从指定地址读取一个字节的数据，使用STM32 HAL库函数
 * 
 * @details
 * 该函数使用HAL_I2C_Mem_Read函数进行读取操作：
 * - 设备地址：ADDR + 1 (0xA1，读模式)
 * - 内部地址：8位地址模式
 * - 数据长度：1字节
 * - 超时时间：2000ms
 * 
 * @note
 * - 读操作使用地址0xA1（写地址0xA0 + 1）
 * - 读取操作不需要等待写周期，可以立即进行
 * - I2C协议规定读地址的最低位为1
 */
uint8_t Hal_W24C02_ReadByte(uint8_t innerAddr) {
  uint8_t byte;
  HAL_I2C_Mem_Read(&hi2c2, ADDR + 1, innerAddr, I2C_MEMADD_SIZE_8BIT, &byte, 1,
                   2000); // 使用HAL库读取数据，超时时间2000ms
  return byte;
}

/**
 * @brief  向W24C02写入多个字节 (HAL库版本 - 页写入)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向要写入数据的指针
 * @param  len: 要写入的字节数
 * @retval 无
 *
 * @description
 * 页写入功能，一次写入多个字节的数据，使用STM32 HAL库函数
 * 
 * @details
 * 该函数使用HAL_I2C_Mem_Write函数进行页写入操作：
 * - 设备地址：ADDR (0xA0，写模式)
 * - 内部地址：8位地址模式
 * - 数据长度：可变长度
 * - 超时时间：2000ms
 * 
 * @note
 * - W24C02的页大小为8字节，超过8字节的写入会回卷到页首
 * - 跨页写入时，地址会自动回卷，可能导致数据覆盖
 * - 建议单次写入不超过8字节，或进行分页处理
 * - 写入后必须等待5ms让EEPROM完成内部写周期
 * 
 * @warning
 * 如果起始地址加上写入长度跨越页边界，超出部分会回卷到当前页的起始位置
 * 可能导致意外数据覆盖，使用时需要特别注意
 */
void Hal_W24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len) {
  HAL_I2C_Mem_Write(&hi2c2, ADDR, innerAddr, I2C_MEMADD_SIZE_8BIT, bytes, len,
                    2000); // 使用HAL库写入多字节数据，超时时间2000ms
  HAL_Delay(5); // 等待EEPROM完成写周期 (典型值5ms)
}

/**
 * @brief  从W24C02读取多个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向存储读取数据的缓冲区
 * @param  len: 要读取的字节数
 * @retval 无
 *
 * @description
 * 连续读取功能，一次性读取多个字节的数据，使用STM32 HAL库函数
 * 
 * @details
 * 该函数使用HAL_I2C_Mem_Read函数进行连续读取操作：
 * - 设备地址：ADDR + 1 (0xA1，读模式)
 * - 内部地址：8位地址模式
 * - 数据长度：可变长度
 * - 超时时间：2000ms
 * 
 * @note
 * - 可以跨页连续读取，没有页边界限制
 * - 读取操作不需要等待写周期，可以立即进行
 * - EEPROM内部地址会在读取完成后自动递增
 * - 支持读取整个256字节的存储空间
 */
void Hal_W24C02_ReadBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len) {
  HAL_I2C_Mem_Read(&hi2c2, ADDR + 1, innerAddr, I2C_MEMADD_SIZE_8BIT, bytes,
                   len, 2000); // 使用HAL库读取多字节数据，超时时间2000ms
}

/**
 * @brief  使用寄存器方式初始化W24C02 (简化版)
 * @param  无
 * @retval 无
 *
 * @description
 * 简化的初始化函数，直接调用HAL库的I2C2初始化
 * 
 * @details
 * 该函数是regisrter_W24C02_Init()的简化版本，直接使用HAL库提供的
 * MX_I2C2_Init()函数来初始化I2C2接口，避免了手动配置寄存器的复杂性
 * 
 * @note
 * - 适用于不需要详细了解底层寄存器配置的场景
 * - 使用STM32CubeMX生成的标准配置
 * - 推荐在大多数应用中使用此函数进行初始化
 */
void register_W24C02_Init(void) {
    MX_I2C2_Init(); // 调用HAL库初始化I2C2
}

/**
 * @brief  使用寄存器方式向W24C02写入单个字节
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @param  byte: 要写入的数据字节
 * @retval 无
 *
 * @description
 * 使用自定义I2C驱动函数向W24C02写入单个字节
 * 
 * @details
 * 完整的I2C写时序流程：
 * 1. START：发送起始条件，开始I2C通信
 * 2. 设备地址(写)：发送W24C02的写地址ADDR (0xA0)
 * 3. 内部地址：发送要写入的EEPROM内部地址
 * 4. 数据：发送要写入的数据字节
 * 5. STOP：发送停止条件，结束I2C通信
 * 
 * @note
 * - 使用自定义的Driver_I2C2_*系列函数进行底层I2C操作
 * - 每个步骤都会等待相应的应答信号
 * - 写入后必须等待5ms让EEPROM完成内部写周期
 * - 在写周期期间，EEPROM不会响应任何I2C通信
 * 
 * @warning
 * 确保在调用此函数前已正确初始化I2C2接口
 */
void register_W24C02_WriteByte(uint8_t innerAddr, uint8_t byte) {

  Driver_I2C2_Start(); // 发送起始条件，开始I2C通信

  Driver_I2C_SendAddr(ADDR); // 发送设备地址（写模式0xA0）并等待EEPROM应答

  Driver_I2C_SendByte(innerAddr); // 发送EEPROM内部存储地址
  Driver_I2C2_ACK();              // 等待EEPROM对内部地址的应答

  Driver_I2C_SendByte(byte); // 发送要写入的数据字节
  Driver_I2C2_ACK();         // 等待EEPROM对数据的应答

  Driver_I2C2_Stop(); // 发送停止条件，结束本次I2C通信

  HAL_Delay(5); // 等待EEPROM完成内部写周期 (典型值5ms，最大可达10ms)
}

/**
 * @brief  使用寄存器方式从W24C02读取单个字节
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @retval 读取到的数据字节，超时返回0
 *
 * @description
 * 使用自定义I2C驱动函数从W24C02读取单个字节
 * 
 * @details
 * 完整的I2C读时序流程：
 * 1. START：发送起始条件，开始I2C通信
 * 2. 设备地址(写)：发送W24C02的写地址ADDR (0xA0)，设置内部地址指针
 * 3. 内部地址：发送要读取的EEPROM内部地址
 * 4. REPEATED START：发送重复起始条件，切换到读模式
 * 5. 设备地址(读)：发送W24C02的读地址ADDR+1 (0xA1)
 * 6. 数据：读取指定地址的数据字节
 * 7. STOP：发送停止条件，结束I2C通信
 * 
 * @note
 * - 使用自定义的Driver_I2C2_*系列函数进行底层I2C操作
 * - 包含超时检测机制，防止程序死等
 * - 超时时间设置为0xFFFF个循环计数
 * - 读取完成后发送NACK表示不再接收数据
 * 
 * @return
 * - 成功：返回读取到的数据字节
 * - 失败：返回0（超时情况）
 * 
 * @warning
 * 确保在调用此函数前已正确初始化I2C2接口
 */
uint8_t register_W24C02_ReadByte(uint8_t innerAddr) {
  uint8_t byte;

  Driver_I2C2_Start(); // 发送起始条件，开始I2C通信

  Driver_I2C_SendAddr(ADDR); // 发送设备地址（写模式0xA0），准备设置内部地址指针
  Driver_I2C2_ACK();         // 等待EEPROM对设备地址的应答

  Driver_I2C_SendByte(innerAddr); // 发送要读取的EEPROM内部地址
  Driver_I2C2_ACK();              // 等待EEPROM对内部地址的应答

  Driver_I2C2_Start(); // 发送重复起始条件，切换到读模式
  Driver_I2C2_ACK();    // 等待重复起始条件的应答

  Driver_I2C_SendAddr(ADDR + 1); // 发送设备地址（读模式0xA1）
  Driver_I2C2_ACK();             // 等待EEPROM对读地址的应答

  /* 读取数据字节 */
  uint8_t timeout = 0xFFFF; // 设置超时计数器，防止无限等待

  /* 等待数据接收完成 */
  while (!(I2C2->SR1 & I2C_SR1_RXNE) && timeout) {
    timeout--; // 超时计数递减，直到RXNE标志位置位或超时
  }

  /* 检查是否超时 */
  if (timeout == 0) {
    Driver_I2C2_NACK(); // 发送非应答，表示不再接收数据
    Driver_I2C2_Stop(); // 发送停止条件，结束I2C通信
    return 0;           // 超时，返回默认值0
  }

  byte = Driver_I2C_ReadByte(); // 读取I2C数据寄存器中的数据字节

  Driver_I2C2_NACK(); // 发送非应答，表示这是最后一个字节
  Driver_I2C2_Stop(); // 发送停止条件，结束本次I2C通信

  return byte;
}

/**
 * @brief  使用寄存器方式向W24C02写入多个字节 (页写入)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向要写入数据的指针
 * @param  len: 要写入的字节数
 * @retval 无
 *
 * @description
 * 使用自定义I2C驱动函数向W24C02写入多个字节数据
 * 
 * @details
 * 完整的I2C页写入时序流程：
 * 1. START：发送起始条件，开始I2C通信
 * 2. 设备地址(写)：发送W24C02的写地址ADDR (0xA0)
 * 3. 内部地址：发送EEPROM起始地址
 * 4. 数据：连续发送多个数据字节
 * 5. STOP：发送停止条件，结束I2C通信
 * 
 * @note
 * - 使用自定义的Driver_I2C2_*系列函数进行底层I2C操作
 * - W24C02的页大小为8字节，超过8字节会回卷到页首
 * - 每个数据字节都会等待EEPROM的应答信号
 * - 写入后必须等待5ms让EEPROM完成内部写周期
 * - 在写周期期间，EEPROM不会响应任何I2C通信
 * 
 * @warning
 * - 跨页写入时，地址会自动回卷，可能导致数据覆盖
 * - 如果起始地址加上写入长度跨越页边界，超出部分会回卷到当前页的起始位置
 * - 建议单次写入不超过8字节，或进行分页处理
 * 
 * @see
 * register_W24C02_WriteByte() - 单字节写入函数
 */
void register_W24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes,
                                uint8_t len) {
  Driver_I2C2_Start(); // 发送起始条件，开始I2C通信

  Driver_I2C_SendAddr(ADDR); // 发送设备地址（写模式0xA0）
  Driver_I2C2_ACK();         // 等待EEPROM对设备地址的应答

  Driver_I2C_SendByte(innerAddr); // 发送EEPROM起始地址
  Driver_I2C2_ACK();              // 等待EEPROM对内部地址的应答

  // 循环发送多个数据字节
  for (uint8_t i = 0; i < len; i++) {
    Driver_I2C_SendByte(bytes[i]); // 发送第i个数据字节
    Driver_I2C2_ACK();             // 等待EEPROM对数据的应答
  }

  Driver_I2C2_Stop(); // 发送停止条件，结束本次I2C通信

  HAL_Delay(5); // 等待EEPROM完成写周期 (典型值5ms，最大可达10ms)
}

/**
 * @brief  使用寄存器方式从W24C02读取多个字节 (连续读取)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向存储读取数据的缓冲区
 * @param  len: 要读取的字节数
 * @retval 无
 *
 * @description
 * 使用自定义I2C驱动函数从W24C02连续读取多个字节数据
 * 
 * @details
 * 完整的I2C连续读取时序流程：
 * 1. START：发送起始条件，开始I2C通信
 * 2. 设备地址(写)：发送W24C02的写地址ADDR (0xA0)，设置内部地址指针
 * 3. 内部地址：发送要读取的EEPROM起始地址
 * 4. REPEATED START：发送重复起始条件，切换到读模式
 * 5. 设备地址(读)：发送W24C02的读地址ADDR+1 (0xA1)
 * 6. 数据：连续读取多个数据字节
 * 7. STOP：发送停止条件，结束I2C通信
 * 
 * @note
 * - 使用自定义的Driver_I2C2_*系列函数进行底层I2C操作
 * - 可以跨页连续读取，没有页边界限制
 * - EEPROM内部地址会在读取完成后自动递增
 * - 支持读取整个256字节的存储空间
 * - 包含超时检测机制，防止程序死等
 * - 每个数据字节都有独立的超时检测
 * 
 * @warning
 * - 确保bytes缓冲区有足够空间存储len个字节
 * - 超时时函数会直接返回，已读取的数据可能不完整
 * - 确保在调用此函数前已正确初始化I2C2接口
 * 
 * @see
 * register_W24C02_ReadByte() - 单字节读取函数
 */
void register_W24C02_ReadBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len) {
  Driver_I2C2_Start(); // 发送起始条件，开始I2C通信

  Driver_I2C_SendAddr(ADDR); // 发送设备地址（写模式0xA0），准备设置内部地址指针
  Driver_I2C2_ACK();         // 等待EEPROM对设备地址的应答

  Driver_I2C_SendByte(innerAddr); // 发送要读取的EEPROM起始地址
  Driver_I2C2_ACK();              // 等待EEPROM对内部地址的应答

  Driver_I2C2_Start(); // 发送重复起始条件，切换到读模式
  Driver_I2C2_ACK();    // 等待重复起始条件的应答

  Driver_I2C_SendAddr(ADDR + 1); // 发送设备地址（读模式0xA1）
  Driver_I2C2_ACK();             // 等待EEPROM对读地址的应答

  // 循环读取多个数据字节
  for (uint8_t i = 0; i < len; i++) {
    /* 读取第i个数据字节 */
    uint8_t timeout = 0xFFFF; // 为每个字节设置独立的超时计数器

    /* 等待数据接收完成 */
    while (!(I2C2->SR1 & I2C_SR1_RXNE) && timeout) {
      timeout--; // 超时计数递减，直到RXNE标志位置位或超时
    }

    /* 检查是否超时 */
    if (timeout == 0) {
      Driver_I2C2_NACK(); // 发送非应答，表示不再接收数据
      Driver_I2C2_Stop(); // 发送停止条件，结束I2C通信
      return ;             // 超时，直接返回（已读取数据可能不完整）
    }

    bytes[i] = Driver_I2C_ReadByte(); // 读取I2C数据寄存器中的第i个数据字节

    // 根据位置决定发送ACK还是NACK
    if (i < len - 1) {
      Driver_I2C2_ACK();  // 发送应答，表示继续接收下一个字节
    } else {
      Driver_I2C2_NACK(); // 发送非应答，表示这是最后一个字节
    }
  }

  Driver_I2C2_Stop(); // 发送停止条件，结束本次I2C通信
}
