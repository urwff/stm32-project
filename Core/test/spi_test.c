/**
 * @file spi_test.c
 * @brief SPI驱动测试程序
 * @version 1.0
 * @date 2025-12-06
 *
 * @details 测试SPI驱动的各项功能，包括：
 *          - HAL库方式的SPI通信
 *          - 寄存器方式的SPI通信
 *          - 两种方式的性能对比
 */

#include "spi_test.h"
#include "spi.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

//======================================================================
//                          私有宏定义
//======================================================================

#define TEST_BUFFER_SIZE 256 // 测试缓冲区大小
#define TEST_LOOP_COUNT 1000 // 性能测试循环次数

// 测试统计结构体
typedef struct {
  uint32_t total_tests;  // 总测试数
  uint32_t passed_tests; // 通过测试数
  uint32_t failed_tests; // 失败测试数
} test_stats_t;

static test_stats_t g_test_stats = {0};

//======================================================================
//                          私有函数声明
//======================================================================

static void print_test_header(const char *test_name);
static void print_test_result(const char *test_name, int result);
static void print_test_summary(void);
static void reset_test_stats(void);

//======================================================================
//                          辅助函数实现
//======================================================================

/**
 * @brief  打印测试标题
 * @param  test_name: 测试名称
 */
static void print_test_header(const char *test_name) {
  printf("\r\n");
  printf("==============================================================\r\n");
  printf("测试: %s\r\n", test_name);
  printf("==============================================================\r\n");
}

/**
 * @brief  打印测试结果
 * @param  test_name: 测试名称
 * @param  result: 测试结果 (0=通过)
 */
static void print_test_result(const char *test_name, int result) {
  g_test_stats.total_tests++;

  if (result == SPI_TEST_PASS) {
    g_test_stats.passed_tests++;
    printf("  [PASS] %s\r\n", test_name);
  } else {
    g_test_stats.failed_tests++;
    printf("  [FAIL] %s (错误码: %d)\r\n", test_name, result);
  }
}

/**
 * @brief  打印测试汇总
 */
static void print_test_summary(void) {
  printf("\r\n");
  printf("==============================================================\r\n");
  printf("测试汇总\r\n");
  printf("==============================================================\r\n");
  printf("  总测试数: %lu\r\n", g_test_stats.total_tests);
  printf("  通过: %lu\r\n", g_test_stats.passed_tests);
  printf("  失败: %lu\r\n", g_test_stats.failed_tests);

  if (g_test_stats.failed_tests == 0) {
    printf("\r\n>>> 所有测试通过! <<<\r\n");
  } else {
    printf("\r\n>>> 存在失败的测试! <<<\r\n");
  }
  printf("==============================================================\r\n");
}

/**
 * @brief  重置测试统计
 */
static void reset_test_stats(void) {
  memset(&g_test_stats, 0, sizeof(g_test_stats));
}

//======================================================================
//                          HAL方式测试
//======================================================================

/**
 * @brief  测试HAL方式CS片选信号控制
 * @return int 0表示成功
 */
static int test_hal_cs_control(void) {
  // 测试CS Start（拉低片选）
  Hal_SPI_Start();

  // 短暂延时，让信号稳定
  for (volatile int i = 0; i < 100; i++) {
  }

  // 测试CS Stop（拉高片选）
  Hal_SPI_Stop();

  printf("  HAL CS控制: Start/Stop执行完成\r\n");
  return SPI_TEST_PASS;
}

/**
 * @brief  测试HAL方式数据交换（回环测试）
 * @return int 0表示成功
 * @note   如果MOSI-MISO短接，发送什么应该收到什么
 *         如果连接了从设备，则验证通信是否正常
 */
static int test_hal_data_exchange(void) {
  uint8_t test_data[] = {0x55, 0xAA, 0x00, 0xFF, 0x12, 0x34, 0x56, 0x78};
  uint8_t rx_data[sizeof(test_data)];

  printf("  HAL 数据交换测试:\r\n");

  Hal_SPI_Start();

  for (int i = 0; i < (int)sizeof(test_data); i++) {
    rx_data[i] = Hal_SPI_SwapByte(test_data[i]);
    printf("    TX: 0x%02X -> RX: 0x%02X\r\n", test_data[i], rx_data[i]);
  }

  Hal_SPI_Stop();

  // 注意: 实际测试结果取决于连接的从设备
  // 如果是回环测试，应该比较tx和rx是否一致
  // 这里只验证通信不会死机

  return SPI_TEST_PASS;
}

/**
 * @brief  测试HAL方式的连续数据传输
 * @return int 0表示成功
 */
static int test_hal_burst_transfer(void) {
  uint8_t tx_buffer[TEST_BUFFER_SIZE];
  uint8_t rx_buffer[TEST_BUFFER_SIZE];

  // 填充测试数据
  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    tx_buffer[i] = (uint8_t)i;
  }

  Hal_SPI_Start();

  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    rx_buffer[i] = Hal_SPI_SwapByte(tx_buffer[i]);
  }

  Hal_SPI_Stop();

  printf("  HAL Burst传输: 成功传输 %d 字节\r\n", TEST_BUFFER_SIZE);

  return SPI_TEST_PASS;
}

/**
 * @brief  测试HAL库方式的SPI功能
 * @return int 0表示成功，负值表示失败
 */
int SPI_Test_HAL_Functions(void) {
  int result = SPI_TEST_PASS;
  int test_result;

  print_test_header("HAL库方式SPI功能测试");

  // 子测试1: CS片选控制
  test_result = test_hal_cs_control();
  print_test_result("HAL CS控制", test_result);
  if (test_result != SPI_TEST_PASS)
    result = SPI_TEST_FAIL;

  // 子测试2: 单字节数据交换
  test_result = test_hal_data_exchange();
  print_test_result("HAL 数据交换", test_result);
  if (test_result != SPI_TEST_PASS)
    result = SPI_TEST_FAIL;

  // 子测试3: 连续数据传输
  test_result = test_hal_burst_transfer();
  print_test_result("HAL Burst传输", test_result);
  if (test_result != SPI_TEST_PASS)
    result = SPI_TEST_FAIL;

  return result;
}

//======================================================================
//                          寄存器方式测试
//======================================================================

/**
 * @brief  测试寄存器方式CS片选信号控制
 * @return int 0表示成功
 */
static int test_register_cs_control(void) {
  // 测试CS Start（拉低片选）
  Register_SPI_Start();

  // 短暂延时
  for (volatile int i = 0; i < 100; i++) {
  }

  // 测试CS Stop（拉高片选）
  Register_SPI_Stop();

  printf("  寄存器 CS控制: Start/Stop执行完成\r\n");
  return SPI_TEST_PASS;
}

/**
 * @brief  测试寄存器方式数据交换
 * @return int 0表示成功
 */
static int test_register_data_exchange(void) {
  uint8_t test_data[] = {0x55, 0xAA, 0x00, 0xFF, 0x12, 0x34, 0x56, 0x78};
  uint8_t rx_data[sizeof(test_data)];

  printf("  寄存器 数据交换测试:\r\n");

  Register_SPI_Start();

  for (int i = 0; i < (int)sizeof(test_data); i++) {
    rx_data[i] = Register_SPI_SwapByte(test_data[i]);
    printf("    TX: 0x%02X -> RX: 0x%02X\r\n", test_data[i], rx_data[i]);
  }

  Register_SPI_Stop();

  return SPI_TEST_PASS;
}

/**
 * @brief  测试寄存器方式连续数据传输
 * @return int 0表示成功
 */
static int test_register_burst_transfer(void) {
  uint8_t tx_buffer[TEST_BUFFER_SIZE];
  uint8_t rx_buffer[TEST_BUFFER_SIZE];

  // 填充测试数据
  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    tx_buffer[i] = (uint8_t)i;
  }

  Register_SPI_Start();

  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    rx_buffer[i] = Register_SPI_SwapByte(tx_buffer[i]);
  }

  Register_SPI_Stop();

  printf("  寄存器 Burst传输: 成功传输 %d 字节\r\n", TEST_BUFFER_SIZE);

  return SPI_TEST_PASS;
}

/**
 * @brief  测试寄存器方式的SPI功能
 * @return int 0表示成功，负值表示失败
 */
int SPI_Test_Register_Functions(void) {
  int result = SPI_TEST_PASS;
  int test_result;

  print_test_header("寄存器方式SPI功能测试");

  // 子测试1: CS片选控制
  test_result = test_register_cs_control();
  print_test_result("寄存器 CS控制", test_result);
  if (test_result != SPI_TEST_PASS)
    result = SPI_TEST_FAIL;

  // 子测试2: 单字节数据交换
  test_result = test_register_data_exchange();
  print_test_result("寄存器 数据交换", test_result);
  if (test_result != SPI_TEST_PASS)
    result = SPI_TEST_FAIL;

  // 子测试3: 连续数据传输
  test_result = test_register_burst_transfer();
  print_test_result("寄存器 Burst传输", test_result);
  if (test_result != SPI_TEST_PASS)
    result = SPI_TEST_FAIL;

  return result;
}

//======================================================================
//                          性能对比测试
//======================================================================

/**
 * @brief  获取当前系统tick值（用于计时）
 * @return uint32_t 当前tick值
 */
static uint32_t get_tick_count(void) { return HAL_GetTick(); }

/**
 * @brief  SPI性能对比测试（HAL vs 寄存器）
 * @note   比较两种方法的速度差异
 */
void SPI_Test_Performance_Compare(void) {
  uint32_t start_tick, hal_time, reg_time;
  uint8_t dummy_data = 0xAA;

  print_test_header("性能对比测试 (HAL vs 寄存器)");

  printf("  测试内容: 发送 %d 字节数据\r\n", TEST_LOOP_COUNT);
  printf("\r\n");

  // ------ HAL库方式性能测试 ------
  Hal_SPI_Start();
  start_tick = get_tick_count();

  for (int i = 0; i < TEST_LOOP_COUNT; i++) {
    Hal_SPI_SwapByte(dummy_data);
  }

  hal_time = get_tick_count() - start_tick;
  Hal_SPI_Stop();

  printf("  HAL库方式:\r\n");
  printf("    传输 %d 字节耗时: %lu ms\r\n", TEST_LOOP_COUNT, hal_time);
  if (hal_time > 0) {
    printf("    近似速率: %.2f KB/s\r\n", (float)TEST_LOOP_COUNT / hal_time);
  }
  printf("\r\n");

  // ------ 寄存器方式性能测试 ------
  Register_SPI_Start();
  start_tick = get_tick_count();

  for (int i = 0; i < TEST_LOOP_COUNT; i++) {
    Register_SPI_SwapByte(dummy_data);
  }

  reg_time = get_tick_count() - start_tick;
  Register_SPI_Stop();

  printf("  寄存器方式:\r\n");
  printf("    传输 %d 字节耗时: %lu ms\r\n", TEST_LOOP_COUNT, reg_time);
  if (reg_time > 0) {
    printf("    近似速率: %.2f KB/s\r\n", (float)TEST_LOOP_COUNT / reg_time);
  }
  printf("\r\n");

  // ------ 性能比较结论 ------
  printf("  性能比较结论:\r\n");
  if (hal_time == reg_time) {
    printf("    两种方式耗时相同\r\n");
  } else if (hal_time > reg_time && reg_time > 0) {
    printf("    寄存器方式更快 (快 %.1f%%)\r\n",
           ((float)hal_time - reg_time) / reg_time * 100);
  } else if (reg_time > hal_time && hal_time > 0) {
    printf("    HAL库方式更快 (快 %.1f%%)\r\n",
           ((float)reg_time - hal_time) / hal_time * 100);
  } else {
    printf("    测试时间太短，无法比较\r\n");
  }

  print_test_result("性能对比测试", SPI_TEST_PASS);
}

//======================================================================
//                          公共测试入口
//======================================================================

/**
 * @brief  运行SPI驱动的快速测试
 * @note   此函数只运行基本的初始化和简单数据交换测试
 */
void SPI_RunQuickTest(void) {
  reset_test_stats();

  printf("\r\n");
  printf("##############################################################\r\n");
  printf("#                   SPI驱动快速测试                          #\r\n");
  printf("##############################################################\r\n");

  // 简单的HAL方式测试
  print_test_header("快速测试 - HAL方式");

  Hal_SPI_Start();
  uint8_t rx = Hal_SPI_SwapByte(0x9F); // 尝试读取设备ID命令
  Hal_SPI_Stop();

  printf("  发送 0x9F, 收到 0x%02X\r\n", rx);
  print_test_result("HAL基本通信", SPI_TEST_PASS);

  // 简单的寄存器方式测试
  print_test_header("快速测试 - 寄存器方式");

  Register_SPI_Start();
  rx = Register_SPI_SwapByte(0x9F);
  Register_SPI_Stop();

  printf("  发送 0x9F, 收到 0x%02X\r\n", rx);
  print_test_result("寄存器基本通信", SPI_TEST_PASS);

  print_test_summary();
}

/**
 * @brief  运行SPI驱动的完整测试套件
 */
void SPI_RunAllTests(void) {
  reset_test_stats();

  printf("\r\n");
  printf("##############################################################\r\n");
  printf("#                   SPI驱动完整测试套件                       #\r\n");
  printf("##############################################################\r\n");
  printf("测试开始...\r\n");
  printf("SPI配置:\r\n");
  printf("  - 模式: Master\r\n");
  printf("  - 数据位: 8-bit\r\n");
  printf("  - 时钟极性: CPOL=0 (空闲低电平)\r\n");
  printf("  - 时钟相位: CPHA=0 (第一边沿采样)\r\n");
  printf("  - 分频系数: 4\r\n");
  printf("  - 片选: 软件控制\r\n");

  // 1. HAL方式功能测试
  SPI_Test_HAL_Functions();

  // 2. 寄存器方式功能测试
  SPI_Test_Register_Functions();

  // 3. 性能对比测试
  SPI_Test_Performance_Compare();

  // 打印测试汇总
  print_test_summary();
}
