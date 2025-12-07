/**
 * @file    test_w24c02.h
 * @brief   W24C02 EEPROM 驱动测试头文件
 * @date    2025-12-06
 */

#ifndef __TEST_W24C02_H
#define __TEST_W24C02_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ========================== 测试配置 ========================== */

/**
 * @brief 选择测试的 API 版本
 *        0 = HAL 库版本
 *        1 = 寄存器版本
 */
#define W24C02_TEST_USE_REGISTER_API 0

/* ========================== 测试函数声明 ========================== */

/**
 * @brief   运行 W24C02 完整测试套件
 * @retval  0: 所有测试通过, -1: 有测试失败
 */
int w24c02_run_tests(void);

/**
 * @brief   W24C02 快速自检
 * @note    写入并读回少量数据验证基本功能
 * @retval  0: 自检通过, -1: 自检失败
 */
int w24c02_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_W24C02_H */
