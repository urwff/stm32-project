/**
 * @file    usart_test.h
 * @brief   USART串口测试模块头文件
 * @author  开发者
 * @date    2025-11-29
 * @version 1.0
 *
 * @description
 * 本头文件定义了USART串口测试功能的接口和数据结构
 * 
 * 测试功能包括：
 * - 中断模式环回测试：验证USART中断接收和发送功能
 * - 阻塞模式收发测试：验证USART阻塞式API功能
 * 
 * @note
 * 测试需要硬件环回连接（TX和RX引脚短接）
 */

#ifndef USART_TEST_H
#define USART_TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================== 数据结构定义 ========================== */

/**
 * @brief  测试状态枚举
 * @description
 * 定义测试执行后的状态结果
 */
typedef enum {
  TEST_STATUS_NOT_RUN = 0,  /**< 测试未运行 */
  TEST_STATUS_PASS,         /**< 测试通过 */
  TEST_STATUS_FAIL          /**< 测试失败 */
} TestStatus;

/* ========================== 函数声明 ========================== */

/**
 * @brief  USART环回测试
 * @param  无
 * @retval TestStatus: 测试结果状态
 * 
 * @description
 * 执行USART1的环回测试，验证中断模式的收发功能
 * 
 * 测试原理：
 * - 通过硬件环回（TX和RX短接）发送测试数据
 * - 使用中断方式接收数据
 * - 比较发送和接收的数据是否一致
 * 
 * @note
 * 需要将USART1的TX(PA9)和RX(PA10)引脚短接
 */
TestStatus usart_loopback_test(void);

/**
 * @brief  USART阻塞模式收发测试
 * @param  无
 * @retval TestStatus: 测试结果状态
 * 
 * @description
 * 执行USART1的阻塞模式收发测试，验证阻塞式API功能
 * 
 * 测试原理：
 * - 通过硬件环回（TX和RX短接）发送测试数据
 * - 使用阻塞方式接收数据
 * - 比较发送和接收的数据是否一致
 * 
 * @note
 * 需要将USART1的TX(PA9)和RX(PA10)引脚短接
 */
TestStatus usart_blocking_tx_rx_test(void);

#ifdef __cplusplus
}
#endif

#endif // USART_TEST_H
