/**
 * @file    w24c02.h
 * @brief   W24C02 EEPROM驱动程序头文件
 * @author  开发者
 * @date    2025-11-26
 * @version 1.0
 *
 * @description
 * 本头文件定义了W24C02 EEPROM存储器的驱动程序接口
 *
 * W24C02特性：
 * - 容量: 2Kbit (256字节)
 * - 接口: I2C总线
 * - 设备地址: 0xA0 (写) / 0xA1 (读)
 * - 页大小: 8字节
 * - 工作电压: 2.5V - 5.5V
 * - 写周期时间: 5ms (典型值)
 *
 * 提供两套API：
 * 1. HAL库版本 (Hal_W24C02_xxx) - 基于STM32 HAL库
 * 2. 寄存器版本 (register_W24C02_xxx) - 基于寄存器直接操作
 */

#ifndef __INF_W24C02_H
#define __INF_W24C02_H

#include "regster_i2c.h"
#include "string.h"

/* ========================== 宏定义 ========================== */

/**
 * @brief W24C02设备地址定义
 * @note  I2C 7位地址格式：1010 A2 A1 A0
 *        A2, A1, A0为地址选择引脚，通常接地
 *        写操作地址：0xA0 (1010 0000)
 *        读操作地址：0xA1 (1010 0001)
 */
#define ADDR 0xA0

/* ========================== HAL库版本函数声明 ========================== */

/**
 * @brief  W24C02初始化函数 (HAL库版本)
 * @param  无
 * @retval 无
 */
void Hal_W24C02_Init(void);

/**
 * @brief  向W24C02写入单个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @param  byte: 要写入的数据字节
 * @retval 无
 */
void Hal_W24C02_WriteByte(uint8_t innerAddr, uint8_t byte);

/**
 * @brief  从W24C02读取单个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @retval 读取到的数据字节
 */
uint8_t Hal_W24C02_ReadByte(uint8_t innerAddr);

/**
 * @brief  向W24C02写入多个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向要写入数据的指针
 * @param  len: 要写入的字节数
 * @retval 无
 * @note   注意页边界限制，W24C02页大小为8字节
 */
void Hal_W24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len);

/**
 * @brief  从W24C02读取多个字节 (HAL库版本)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向存储读取数据的缓冲区
 * @param  len: 要读取的字节数
 * @retval 无
 */
void Hal_W24C02_ReadBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len);

/* ========================== 寄存器版本函数声明 ========================== */

/**
 * @brief  W24C02初始化函数 (寄存器版本)
 * @param  无
 * @retval 无
 */
void register_W24C02_Init(void);

/**
 * @brief  向W24C02写入单个字节 (寄存器版本)
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @param  byte: 要写入的数据字节
 * @retval 无
 */
void register_W24C02_WriteByte(uint8_t innerAddr, uint8_t byte);

/**
 * @brief  从W24C02读取单个字节 (寄存器版本)
 * @param  innerAddr: EEPROM内部地址 (0-255)
 * @retval 读取到的数据字节
 */
uint8_t register_W24C02_ReadByte(uint8_t innerAddr);

/**
 * @brief  向W24C02写入多个字节 (寄存器版本)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向要写入数据的指针
 * @param  len: 要写入的字节数
 * @retval 无
 * @note   注意页边界限制，W24C02页大小为8字节
 */
void register_W24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len);

/**
 * @brief  从W24C02读取多个字节 (寄存器版本)
 * @param  innerAddr: EEPROM内部起始地址 (0-255)
 * @param  bytes: 指向存储读取数据的缓冲区
 * @param  len: 要读取的字节数
 * @retval 无
 */
void register_W24C02_ReadBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len);

#endif /* __INF_W24C02_H */