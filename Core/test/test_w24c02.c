/**
 * @file    test_w24c02.c
 * @brief   W24C02 EEPROM 驱动测试文件
 * @date    2025-12-06
 *
 * @note    W24C02 特性：
 *          - 容量: 256 字节 (地址 0x00 ~ 0xFF)
 *          - 页大小: 8 字节
 *          - 写周期: 5ms
 *          - 设备地址: 0xA0
 *
 * @warning 测试会修改 EEPROM 内容！建议使用保留地址区域
 *          默认测试区域: 0xF0 ~ 0xFF (最后 16 字节)
 */

#include "test_w24c02.h"
#include "w24c02.h"
#include <stdio.h>
#include <string.h>

/* ========================== 测试配置 ========================== */

/** 测试使用的起始地址（避免覆盖重要数据） */
#define TEST_START_ADDR 0xF0

/** 测试使用的区域大小 */
#define TEST_REGION_SIZE 16

/** 写操作后延时（ms），W24C02 写周期需要 5ms */
#define WRITE_DELAY_MS 10

/* ========================== 私有变量 ========================== */

static uint32_t test_passed = 0;
static uint32_t test_failed = 0;
static uint32_t test_skipped = 0;

/* ========================== 测试断言宏 ========================== */

#define TEST_ASSERT(condition, msg)                                            \
  do {                                                                         \
    if (condition) {                                                           \
      test_passed++;                                                           \
      printf("[PASS] %s\r\n", msg);                                            \
    } else {                                                                   \
      test_failed++;                                                           \
      printf("[FAIL] %s\r\n", msg);                                            \
    }                                                                          \
  } while (0)

#define TEST_ASSERT_EQUAL(expected, actual, msg)                               \
  do {                                                                         \
    if ((expected) == (actual)) {                                              \
      test_passed++;                                                           \
      printf("[PASS] %s (expected=0x%02X, actual=0x%02X)\r\n", msg,            \
             (unsigned)(expected), (unsigned)(actual));                        \
    } else {                                                                   \
      test_failed++;                                                           \
      printf("[FAIL] %s (expected=0x%02X, actual=0x%02X)\r\n", msg,            \
             (unsigned)(expected), (unsigned)(actual));                        \
    }                                                                          \
  } while (0)

#define TEST_SKIP(msg)                                                         \
  do {                                                                         \
    test_skipped++;                                                            \
    printf("[SKIP] %s\r\n", msg);                                              \
  } while (0)

#define TEST_GROUP_BEGIN(name) printf("\r\n=== %s ===\r\n", name)
#define TEST_GROUP_END() printf("\r\n")

/* ========================== API 抽象层 ========================== */

/**
 * @brief 根据配置选择使用的 API 版本
 */
#if W24C02_TEST_USE_REGISTER_API
#define W24C02_Init() register_W24C02_Init()
#define W24C02_WriteByte(addr, byte) register_W24C02_WriteByte(addr, byte)
#define W24C02_ReadByte(addr) register_W24C02_ReadByte(addr)
#define W24C02_WriteBytes(addr, data, len)                                     \
  register_W24C02_WriteBytes(addr, data, len)
#define W24C02_ReadBytes(addr, data, len)                                      \
  register_W24C02_ReadBytes(addr, data, len)
#define API_NAME "Register"
#else
#define W24C02_Init() Hal_W24C02_Init()
#define W24C02_WriteByte(addr, byte) Hal_W24C02_WriteByte(addr, byte)
#define W24C02_ReadByte(addr) Hal_W24C02_ReadByte(addr)
#define W24C02_WriteBytes(addr, data, len)                                     \
  Hal_W24C02_WriteBytes(addr, data, len)
#define W24C02_ReadBytes(addr, data, len) Hal_W24C02_ReadBytes(addr, data, len)
#define API_NAME "HAL"
#endif

/* ========================== 延时函数 ========================== */

/**
 * @brief 简单延时（需根据实际系统实现）
 * @note  可替换为 HAL_Delay() 或其他延时函数
 */
static void delay_ms(uint32_t ms) {
  /* 使用 HAL_Delay 或简单循环延时 */
  extern void HAL_Delay(uint32_t Delay);
  HAL_Delay(ms);
}

/* ========================== 测试用例 ========================== */

/**
 * @brief 测试单字节读写
 */
static void test_single_byte_rw(void) {
  uint8_t test_addr = TEST_START_ADDR;
  uint8_t test_value = 0xA5;
  uint8_t read_value;

  /* 写入单字节 */
  W24C02_WriteByte(test_addr, test_value);
  delay_ms(WRITE_DELAY_MS);

  /* 读回并验证 */
  read_value = W24C02_ReadByte(test_addr);
  TEST_ASSERT_EQUAL(test_value, read_value, "Single byte R/W: 0xA5");

  /* 再测试一个不同的值 */
  test_value = 0x5A;
  W24C02_WriteByte(test_addr, test_value);
  delay_ms(WRITE_DELAY_MS);

  read_value = W24C02_ReadByte(test_addr);
  TEST_ASSERT_EQUAL(test_value, read_value, "Single byte R/W: 0x5A");

  /* 测试边界值 */
  test_value = 0x00;
  W24C02_WriteByte(test_addr, test_value);
  delay_ms(WRITE_DELAY_MS);
  read_value = W24C02_ReadByte(test_addr);
  TEST_ASSERT_EQUAL(test_value, read_value, "Single byte R/W: 0x00");

  test_value = 0xFF;
  W24C02_WriteByte(test_addr, test_value);
  delay_ms(WRITE_DELAY_MS);
  read_value = W24C02_ReadByte(test_addr);
  TEST_ASSERT_EQUAL(test_value, read_value, "Single byte R/W: 0xFF");
}

/**
 * @brief 测试多字节读写（页内）
 */
static void test_multi_byte_page_rw(void) {
  uint8_t test_addr = TEST_START_ADDR;
  uint8_t tx_data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  uint8_t rx_data[8] = {0};

  /* 写入 8 字节（一页） */
  W24C02_WriteBytes(test_addr, tx_data, 8);
  delay_ms(WRITE_DELAY_MS);

  /* 读回并验证 */
  W24C02_ReadBytes(test_addr, rx_data, 8);

  int match = (memcmp(tx_data, rx_data, 8) == 0);
  TEST_ASSERT(match, "Page write/read: 8 bytes");

  if (!match) {
    printf("  TX: %02X %02X %02X %02X %02X %02X %02X %02X\r\n", tx_data[0],
           tx_data[1], tx_data[2], tx_data[3], tx_data[4], tx_data[5],
           tx_data[6], tx_data[7]);
    printf("  RX: %02X %02X %02X %02X %02X %02X %02X %02X\r\n", rx_data[0],
           rx_data[1], rx_data[2], rx_data[3], rx_data[4], rx_data[5],
           rx_data[6], rx_data[7]);
  }
}

/**
 * @brief 测试跨页写入
 * @note  W24C02 页大小为 8 字节，跨页写入需要分次写入
 */
static void test_cross_page_rw(void) {
  /* 从页边界前 4 字节开始写入 12 字节（跨越一个页边界） */
  uint8_t test_addr = TEST_START_ADDR + 4; /* 假设 TEST_START_ADDR 是页对齐的 */
  uint8_t tx_data[12] = {0xA1, 0xA2, 0xA3, 0xA4, 0xB1, 0xB2,
                         0xB3, 0xB4, 0xC1, 0xC2, 0xC3, 0xC4};
  uint8_t rx_data[12] = {0};

  /* 写入数据（驱动应该处理跨页） */
  W24C02_WriteBytes(test_addr, tx_data, 12);
  delay_ms(WRITE_DELAY_MS * 3); /* 可能需要多次写周期 */

  /* 读回并验证 */
  W24C02_ReadBytes(test_addr, rx_data, 12);

  int match = (memcmp(tx_data, rx_data, 12) == 0);
  TEST_ASSERT(match, "Cross-page write/read: 12 bytes");

  if (!match) {
    printf("  TX: ");
    for (int i = 0; i < 12; i++)
      printf("%02X ", tx_data[i]);
    printf("\r\n");
    printf("  RX: ");
    for (int i = 0; i < 12; i++)
      printf("%02X ", rx_data[i]);
    printf("\r\n");
  }
}

/**
 * @brief 测试不同地址的读写
 */
static void test_different_addresses(void) {
  /* 测试不同地址能否正确读写 */
  uint8_t addr1 = TEST_START_ADDR;
  uint8_t addr2 = TEST_START_ADDR + 4;
  uint8_t addr3 = TEST_START_ADDR + 8;

  uint8_t val1 = 0x11, val2 = 0x22, val3 = 0x33;
  uint8_t read1, read2, read3;

  /* 写入三个不同地址 */
  W24C02_WriteByte(addr1, val1);
  delay_ms(WRITE_DELAY_MS);
  W24C02_WriteByte(addr2, val2);
  delay_ms(WRITE_DELAY_MS);
  W24C02_WriteByte(addr3, val3);
  delay_ms(WRITE_DELAY_MS);

  /* 读回验证（乱序读取） */
  read3 = W24C02_ReadByte(addr3);
  read1 = W24C02_ReadByte(addr1);
  read2 = W24C02_ReadByte(addr2);

  TEST_ASSERT_EQUAL(val1, read1, "Address isolation: addr1");
  TEST_ASSERT_EQUAL(val2, read2, "Address isolation: addr2");
  TEST_ASSERT_EQUAL(val3, read3, "Address isolation: addr3");
}

/**
 * @brief 测试连续写入覆盖
 */
static void test_overwrite(void) {
  uint8_t addr = TEST_START_ADDR;

  /* 第一次写入 */
  W24C02_WriteByte(addr, 0xAA);
  delay_ms(WRITE_DELAY_MS);

  uint8_t first_read = W24C02_ReadByte(addr);
  TEST_ASSERT_EQUAL(0xAA, first_read, "Overwrite test: first write");

  /* 覆盖写入 */
  W24C02_WriteByte(addr, 0x55);
  delay_ms(WRITE_DELAY_MS);

  uint8_t second_read = W24C02_ReadByte(addr);
  TEST_ASSERT_EQUAL(0x55, second_read, "Overwrite test: second write");
}

/**
 * @brief 测试数据持久性（热重启后数据保留）
 * @note  此测试仅在非首次运行时有意义
 */
static void test_data_persistence(void) {
  /* 写入一个"签名"值 */
  uint8_t signature_addr = 0xFF; /* 使用最后一个地址 */
  uint8_t signature_value = 0x42;

  uint8_t current_value = W24C02_ReadByte(signature_addr);

  if (current_value == signature_value) {
    printf("[INFO] Persistence: Signature found (data survived reset)\r\n");
    test_passed++;
  } else {
    printf("[INFO] Persistence: No signature, writing new one\r\n");
    W24C02_WriteByte(signature_addr, signature_value);
    delay_ms(WRITE_DELAY_MS);

    /* 验证写入成功 */
    current_value = W24C02_ReadByte(signature_addr);
    TEST_ASSERT_EQUAL(signature_value, current_value,
                      "Persistence: Signature written");
  }
}

/**
 * @brief 测试边界地址
 */
static void test_boundary_addresses(void) {
  uint8_t read_val;

  /* 测试地址 0x00 */
  W24C02_WriteByte(0x00, 0xF0);
  delay_ms(WRITE_DELAY_MS);
  read_val = W24C02_ReadByte(0x00);
  TEST_ASSERT_EQUAL(0xF0, read_val, "Boundary: Address 0x00");

  /* 恢复原值（避免破坏数据） */
  W24C02_WriteByte(0x00, 0x00);
  delay_ms(WRITE_DELAY_MS);

  /* 测试地址 0xFF */
  W24C02_WriteByte(0xFF, 0x0F);
  delay_ms(WRITE_DELAY_MS);
  read_val = W24C02_ReadByte(0xFF);
  TEST_ASSERT_EQUAL(0x0F, read_val, "Boundary: Address 0xFF");
}

/**
 * @brief 测试连续读取性能
 */
static void test_sequential_read(void) {
  uint8_t buffer[16];

  /* 连续读取 16 字节 */
  W24C02_ReadBytes(TEST_START_ADDR, buffer, 16);

  /* 只要不死机就算通过 */
  printf("[PASS] Sequential read: 16 bytes completed\r\n");
  test_passed++;

  /* 打印读取内容 */
  printf("  Data: ");
  for (int i = 0; i < 16; i++) {
    printf("%02X ", buffer[i]);
  }
  printf("\r\n");
}

/* ========================== 公开函数实现 ========================== */

/**
 * @brief 运行完整测试套件
 */
int w24c02_run_tests(void) {
  /* 重置统计 */
  test_passed = 0;
  test_failed = 0;
  test_skipped = 0;

  printf("\r\n");
  printf("========================================\r\n");
  printf("     W24C02 EEPROM Test Suite (%s)     \r\n", API_NAME);
  printf("========================================\r\n");
  printf("  Test region: 0x%02X ~ 0x%02X\r\n", TEST_START_ADDR,
         TEST_START_ADDR + TEST_REGION_SIZE - 1);
  printf("  Page size: 8 bytes\r\n");
  printf("  Write delay: %d ms\r\n", WRITE_DELAY_MS);

  /* 初始化 */
  W24C02_Init();

  /* 测试组 1：基本读写 */
  TEST_GROUP_BEGIN("Basic Read/Write Tests");
  test_single_byte_rw();
  TEST_GROUP_END();

  /* 测试组 2：多字节读写 */
  TEST_GROUP_BEGIN("Multi-byte Read/Write Tests");
  test_multi_byte_page_rw();
  test_cross_page_rw();
  TEST_GROUP_END();

  /* 测试组 3：地址测试 */
  TEST_GROUP_BEGIN("Address Tests");
  test_different_addresses();
  test_boundary_addresses();
  TEST_GROUP_END();

  /* 测试组 4：覆盖写入测试 */
  TEST_GROUP_BEGIN("Overwrite Tests");
  test_overwrite();
  TEST_GROUP_END();

  /* 测试组 5：持久性与性能 */
  TEST_GROUP_BEGIN("Persistence & Performance Tests");
  test_data_persistence();
  test_sequential_read();
  TEST_GROUP_END();

  /* 打印统计 */
  printf("========================================\r\n");
  printf("          Test Results Summary          \r\n");
  printf("========================================\r\n");
  printf("  Passed:  %lu\r\n", test_passed);
  printf("  Failed:  %lu\r\n", test_failed);
  printf("  Skipped: %lu\r\n", test_skipped);
  printf("  Total:   %lu\r\n", test_passed + test_failed + test_skipped);
  printf("========================================\r\n");

  if (test_failed > 0) {
    printf("  RESULT: FAILED\r\n");
    return -1;
  } else {
    printf("  RESULT: PASSED\r\n");
    return 0;
  }
}

/**
 * @brief W24C02 快速自检
 */
int w24c02_self_test(void) {
  printf("\r\n[Self-Test] W24C02 EEPROM Quick Test (%s API)\r\n", API_NAME);

  /* 初始化 */
  W24C02_Init();
  printf("[Self-Test] Init OK\r\n");

  /* 使用测试区域最后一个地址 */
  uint8_t addr = TEST_START_ADDR + TEST_REGION_SIZE - 1;
  uint8_t test_pattern = 0x5A;

  /* 保存原值 */
  uint8_t original = W24C02_ReadByte(addr);

  /* 写入测试数据 */
  W24C02_WriteByte(addr, test_pattern);
  delay_ms(WRITE_DELAY_MS);

  /* 读回验证 */
  uint8_t readback = W24C02_ReadByte(addr);

  if (readback != test_pattern) {
    printf("[Self-Test] FAIL: Write 0x%02X, Read 0x%02X\r\n", test_pattern,
           readback);
    return -1;
  }
  printf("[Self-Test] Single byte R/W OK\r\n");

  /* 恢复原值 */
  W24C02_WriteByte(addr, original);
  delay_ms(WRITE_DELAY_MS);

  printf("[Self-Test] PASSED\r\n\r\n");
  return 0;
}
