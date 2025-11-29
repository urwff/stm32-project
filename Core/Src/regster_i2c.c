/**
 * @file    regster_i2c.c
 * @brief   STM32F103 I2C2驱动程序实现
 * @details 本文件实现了STM32F103微控制器的I2C2外设驱动程序，
 *          提供了I2C通信的基本功能，包括初始化、起始/停止条件、
 *          数据发送/接收等操作。
 * @author  开发者
 * @date    2025-11-26
 * @version 1.0
 */

#include "regster_i2c.h"
#include "stm32f103xe.h"
#include <stdint.h>

/**
 * @brief  初始化I2C2外设
 * @details 配置I2C2外设的时钟、工作模式、时钟频率等参数
 *          注意：此函数仅配置I2C2外设寄存器，GPIO引脚需要单独配置
 * @param  无
 * @retval 无
 * @note   使用前需要确保相关GPIO引脚已正确配置为I2C功能
 *         PB10 - I2C2_SCL (时钟线)
 *         PB11 - I2C2_SDA (数据线)
 */
void Driver_I2C2_Init(void) {
  /* 步骤1: 使能I2C2外设时钟 */
  RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; // 使能APB1总线上的I2C2时钟

  /* 步骤2: 配置I2C2工作模式 */
  I2C2->CR1 &= ~I2C_CR1_SMBUS; // 清除SMBUS位，选择I2C模式（非SMBus模式）

  /* 步骤3: 配置I2C2时钟频率 */
  I2C2->CR2 |= 36; // 设置PCLK1频率为36MHz（用于计算I2C时钟）
                   // CR2[5:0] = FREQ[5:0] = 36 (36MHz)

  /* 步骤4: 配置I2C时钟控制寄存器 */
  I2C2->CCR &= ~I2C_CCR_FS; // 清除FS位，选择标准模式（100kHz）
  I2C2->CCR = 180; // 设置CCR值为180
                   // 在标准模式下：Thigh = Tlow = CCR * TPCLK1
                   // TPCLK1 = 1/36MHz ≈ 27.8ns
                   // Thigh = Tlow = 180 * 27.8ns = 5μs
                   // I2C频率 = 1/(Thigh + Tlow) = 1/10μs = 100kHz

  /* 步骤5: 配置上升时间 */
  I2C2->TRISE = 37; // 设置最大上升时间
                    // 标准模式下最大上升时间为1000ns
                    // TRISE = (最大上升时间/TPCLK1) + 1
                    // TRISE = (1000ns / 27.8ns) + 1 ≈ 37

  /* 步骤6: 使能I2C2外设 */
  I2C2->CR1 |= I2C_CR1_PE; // 设置PE位，使能I2C2外设
}

/**
 * @brief  发送I2C起始条件
 * @details 设置控制寄存器CR1的START位，产生I2C起始条件信号
 *          起始条件：在SCL为高电平时，SDA从高电平变为低电平
 * @param  无
 * @retval OK   - 起始条件发送成功
 * @retval FAIL - 起始条件发送失败（超时）
 * @note   此函数会等待硬件自动清除START位，并检查SB标志位
 *         SB位置位表示起始条件已成功发送
 */
uint8_t Driver_I2C2_Start(void) {
  I2C2->CR1 |= I2C_CR1_START; // 设置START位，硬件自动发送起始条件
  
  uint16_t timeout = 0xFFFF; // 设置超时计数器，防止死循环
  
  /* 等待起始条件发送完成 */
  while (!(I2C2->SR1 & I2C_SR1_SB) && timeout) {
    timeout--; // 超时计数递减
  }
  
  /* 检查是否超时 */
  return timeout ? OK : FAIL; // 超时返回FAIL，否则返回OK
}
/**
 * @brief  发送I2C停止条件
 * @details 设置控制寄存器CR1的STOP位，产生I2C停止条件信号
 *          停止条件：在SCL为高电平时，SDA从低电平变为高电平
 * @param  无
 * @retval 无
 * @note   停止条件由硬件自动生成，STOP位会被硬件自动清除
 *         停止条件标志着一次I2C通信的结束
 */
void Driver_I2C2_Stop(void){
    I2C2->CR1 |= I2C_CR1_STOP; // 设置STOP位，硬件自动发送停止条件
}

/**
 * @brief  使能I2C应答信号
 * @details 设置控制寄存器CR1的ACK位，使能自动应答功能
 *          当接收到数据字节后，硬件会自动发送ACK信号
 * @param  无
 * @retval 无
 * @note   通常在接收数据前调用此函数，表示准备接收更多数据
 *         ACK信号表示数据接收成功，从设备可以继续发送下一个字节
 */
void Driver_I2C2_ACK(void){
    I2C2->CR1 |= I2C_CR1_ACK; // 设置ACK位，使能自动应答
}

/**
 * @brief  禁止I2C应答信号
 * @details 清除控制寄存器CR1的ACK位，禁止自动应答功能
 *          当接收到数据字节后，硬件会自动发送NACK信号
 * @param  无
 * @retval 无
 * @note   通常在接收最后一个数据字节前调用此函数
 *         NACK信号表示不再接收数据，从设备应停止发送
 */
void Driver_I2C2_NACK(void){
    I2C2->CR1 &= ~I2C_CR1_ACK; // 清除ACK位，禁止自动应答（发送NACK）
}

/**
 * @brief  发送I2C设备地址
 * @details 向I2C总线发送7位设备地址和读写位，并等待从设备应答
 *          地址格式：[A6 A5 A4 A3 A2 A1 A0 R/W]
 *          R/W位：0=写操作，1=读操作
 * @param  addr 要发送的设备地址字节（包含7位地址和1位读写位）
 * @retval OK   - 地址发送成功，从设备已应答
 * @retval FAIL - 地址发送失败（超时或从设备无应答）
 * @note   此函数会自动清除ADDR标志位（通过读取SR1和SR2寄存器）
 *         必须在发送起始条件后调用此函数
 */
uint8_t Driver_I2C_SendAddr(uint8_t addr){
    I2C2->DR = addr; // 将地址字节写入数据寄存器
    
    uint16_t timeout = 0xFFFF; // 设置超时计数器
    
    /* 等待地址发送完成并接收到从设备应答 */
    while (!(I2C2->SR1 & I2C_SR1_ADDR) && timeout) {
        timeout--; // 超时计数递减
    }
    
    /* 检查是否超时 */
    if (timeout == 0) {
        return FAIL; // 超时，从设备无应答
    }
    
    /* 清除ADDR标志位 */
    volatile uint32_t temp = I2C2->SR2; // 读取SR2寄存器以清除ADDR标志
    (void)temp; // 防止编译器优化掉这个读操作
    
    return OK; // 地址发送成功
}

/**
 * @brief  发送I2C数据字节
 * @details 向I2C总线发送一个8位数据字节，并等待发送完成
 *          此函数会等待数据寄存器空闲，然后发送数据
 * @param  byte 要发送的8位数据字节
 * @retval OK   - 数据发送成功
 * @retval FAIL - 数据发送失败（超时）
 * @note   必须在成功发送设备地址后调用此函数
 *         TXE标志位表示数据寄存器为空，可以写入新数据
 */
uint8_t Driver_I2C_SendByte(uint8_t byte){
    uint16_t timeout = 0xFFFF; // 设置超时计数器
    
    /* 等待数据寄存器空闲 */
    while (!(I2C2->SR1 & I2C_SR1_TXE) && timeout) {
        timeout--; // 超时计数递减
    }
    
    /* 检查第一次超时 */
    if (timeout == 0) {
        return FAIL; // 数据寄存器未空闲，发送失败
    }
    
    I2C2->DR = byte; // 将数据字节写入数据寄存器

    /* 重新设置超时计数器，等待数据发送完成 */
    timeout = 0xFFFF;
    while (!(I2C2->SR1 & I2C_SR1_TXE) && timeout) {
        timeout--; // 超时计数递减
    }
    
    /* 检查第二次超时 */
    return timeout ? OK : FAIL; // 超时返回FAIL，否则返回OK
}   

/**
 * @brief  读取I2C数据字节
 * @details 从I2C总线读取一个8位数据字节
 *          此函数会等待接收缓冲区有数据可读
 * @param  无
 * @retval 接收到的8位数据字节，超时时返回0
 * @note   在调用此函数前，应先配置ACK/NACK信号
 *         RXNE标志位表示接收缓冲区非空，有数据可读
 *         对于最后一个字节，应在调用前设置NACK
 */
uint8_t Driver_I2C_ReadByte(void){
    uint16_t timeout = 0xFFFF; // 设置超时计数器
   
    /* 等待接收缓冲区有数据 */
    while (!(I2C2->SR1 & I2C_SR1_RXNE) && timeout) {
        timeout--; // 超时计数递减
    }
    
    /* 检查是否超时 */
    if (timeout == 0) {
        return 0; // 超时返回0（可能需要根据应用需求修改）
    }
    
    return I2C2->DR; // 读取并返回接收到的数据字节
}
