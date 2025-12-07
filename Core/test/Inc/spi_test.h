/**
 * @file spi_test.h
 * @brief SPI驱动测试程序头文件
 * @version 1.0
 * @date 2025-12-06
 *
 * @details 本测试模块提供对SPI驱动的测试功能，包括：
 *          - HAL库方式的SPI通信测试
 *          - 寄存器方式的SPI通信测试
 *          - 回环测试（如果硬件支持）
 *          - 性能对比测试
 */

#ifndef __SPI_TEST_H
#define __SPI_TEST_H

#include <stdint.h>

//======================================================================
//                          测试结果定义
//======================================================================

#define SPI_TEST_PASS 0           // 测试通过
#define SPI_TEST_FAIL -1          // 测试失败
#define SPI_TEST_TIMEOUT -2       // 超时错误
#define SPI_TEST_COMMUNICATION -3 // 通信错误

//======================================================================
//                          测试函数声明
//======================================================================

/**
 * @brief  运行SPI驱动的完整测试套件
 * @note   此函数会运行所有测试用例，包括:
 *         - SPI初始化测试
 *         - HAL库方式CS片选测试
 *         - HAL库方式数据交换测试
 *         - 寄存器方式CS片选测试
 *         - 寄存器方式数据交换测试
 *         - HAL与寄存器方式性能对比测试
 * @warning 测试需要实际的SPI从设备连接（如W25Q32 Flash）
 */
void SPI_RunAllTests(void);

/**
 * @brief  运行SPI驱动的快速测试
 * @note   此函数只运行基本的初始化和简单数据交换测试
 */
void SPI_RunQuickTest(void);

/**
 * @brief  测试HAL库方式的SPI功能
 * @return int 0表示成功，负值表示失败
 */
int SPI_Test_HAL_Functions(void);

/**
 * @brief  测试寄存器方式的SPI功能
 * @return int 0表示成功，负值表示失败
 */
int SPI_Test_Register_Functions(void);

/**
 * @brief  SPI性能对比测试（HAL vs 寄存器）
 * @note   比较两种方法的速度差异
 */
void SPI_Test_Performance_Compare(void);

#endif // __SPI_TEST_H
