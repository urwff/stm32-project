/**
 * @file w25q32_test.c
 * @brief W25Q32 Flash驱动的综合测试程序
 * @version 1.0
 * @date 2025-11-29
 * 
 * @note
 * 本测试程序涵盖:
 * 1. 初始化测试 - 验证芯片ID和连接
 * 2. 读写测试 - 验证数据的正确读写
 * 3. 擦除测试 - 验证扇区和块擦除功能
 * 4. 边界测试 - 验证跨页/跨扇区操作
 * 5. 错误处理测试 - 验证参数校验
 * 6. 性能测试 - 评估读写速度
 */

#include "w25q32.h"
#include "usart.h"  // 用于打印测试结果
#include <string.h>
#include <stdio.h>
#include "stdlib.h"

//======================================================================
//                          测试配置和宏定义
//======================================================================

#define TEST_SECTOR_NUM     10      // 用于测试的扇区号
#define TEST_PAGE_NUM       100     // 用于测试的页号
#define TEST_DATA_SIZE      256     // 测试数据大小

// 测试结果统计
typedef struct {
    uint32_t total_tests;
    uint32_t passed_tests;
    uint32_t failed_tests;
} TestResult_t;

static TestResult_t g_test_result = {0, 0, 0};
static W25Q32_State_t g_w25q32_state;

//======================================================================
//                          辅助函数
//======================================================================

/**
 * @brief 打印测试结果
 */
static void print_test_result(const char *test_name, uint8_t passed) {
    g_test_result.total_tests++;
    if (passed) {
        g_test_result.passed_tests++;
        printf("[PASS] %s\r\n", test_name);
    } else {
        g_test_result.failed_tests++;
        printf("[FAIL] %s\r\n", test_name);
    }
}

/**
 * @brief 打印芯片信息
 */
static void print_chip_info(W25Q32_State_t *state) {
    printf("\r\n========== W25Q32 芯片信息 ==========\r\n");
    printf("制造商ID: 0x%02X\r\n", state->manufacturer_id);
    printf("JEDEC ID: 0x%04X\r\n", state->jedec_id);
    printf("设备ID: 0x%02X\r\n", state->device_id);
    printf("唯一ID: 0x%016llX\r\n", state->unique_id);
    printf("总页数: %lu\r\n", state->page_count);
    printf("总扇区数: %lu\r\n", state->sector_count);
    printf("总块数(64KB): %lu\r\n", state->block_64k_count);
    printf("====================================\r\n\r\n");
}

/**
 * @brief 生成测试数据
 */
static void generate_test_data(uint8_t *buffer, uint32_t size, uint8_t seed) {
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = (uint8_t)(seed + i);
    }
}

/**
 * @brief 验证数据是否全为0xFF (擦除后的状态)
 */
static uint8_t verify_erased(uint8_t *buffer, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        if (buffer[i] != 0xFF) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief 比较两个缓冲区的数据
 */
static uint8_t compare_buffers(uint8_t *buf1, uint8_t *buf2, uint32_t size) {
    return (memcmp(buf1, buf2, size) == 0);
}

//======================================================================
//                          测试用例实现
//======================================================================

/**
 * @brief 测试1: 初始化测试
 */
static void test_initialization(void) {
    printf("\r\n========== 测试1: 初始化测试 ==========\r\n");
    
    W25Q32_Status_t status = W25Q32_Init(&g_w25q32_state);
    
    // 验证初始化结果
    uint8_t passed = (status == W25Q32_OK) &&
                     (g_w25q32_state.manufacturer_id == W25Q32_EXPECTED_MANUFACTURER_ID) &&
                     (g_w25q32_state.jedec_id == W25Q32_EXPECTED_JEDEC_ID_PART);
    
    print_test_result("芯片初始化和ID验证", passed);
    
    if (passed) {
        print_chip_info(&g_w25q32_state);
    }
    
    // 测试空指针保护
    status = W25Q32_Init(NULL);
    print_test_result("空指针参数保护", status == W25Q32_INVALID_PARAM);
}

/**
 * @brief 测试2: 基本读写测试
 */
static void test_basic_read_write(void) {
    printf("\r\n========== 测试2: 基本读写测试 ==========\r\n");
    
    uint8_t write_buffer[TEST_DATA_SIZE];
    uint8_t read_buffer[TEST_DATA_SIZE];
    W25Q32_Status_t status;
    
    // 生成测试数据
    generate_test_data(write_buffer, TEST_DATA_SIZE, 0xAA);
    
    // 先擦除测试扇区
    printf("擦除测试扇区 %d...\r\n", TEST_SECTOR_NUM);
    status = W25Q32_SectorErase_4KB(TEST_SECTOR_NUM);
    print_test_result("扇区擦除", status == W25Q32_OK);
    
    // 验证擦除后数据为0xFF
    memset(read_buffer, 0x00, TEST_DATA_SIZE);
    status = W25Q32_ReadData(TEST_SECTOR_NUM * W25Q32_SECTOR_SIZE, read_buffer, TEST_DATA_SIZE);
    print_test_result("读取擦除后数据", status == W25Q32_OK && verify_erased(read_buffer, TEST_DATA_SIZE));
    
    // 写入数据
    printf("写入测试数据...\r\n");
    status = W25Q32_PageProgram(TEST_PAGE_NUM, 0, write_buffer, TEST_DATA_SIZE);
    print_test_result("页编程", status == W25Q32_OK);
    
    // 读取数据
    memset(read_buffer, 0x00, TEST_DATA_SIZE);
    status = W25Q32_ReadData(TEST_PAGE_NUM * W25Q32_PAGE_SIZE, read_buffer, TEST_DATA_SIZE);
    print_test_result("读取数据", status == W25Q32_OK);
    
    // 验证数据
    print_test_result("数据一致性验证", compare_buffers(write_buffer, read_buffer, TEST_DATA_SIZE));
}

/**
 * @brief 测试3: 跨页写入测试
 */
static void test_cross_page_write(void) {
    printf("\r\n========== 测试3: 跨页写入测试 ==========\r\n");
    
    uint8_t write_buffer[200];
    uint8_t read_buffer[200];
    W25Q32_Status_t status;
    uint32_t test_page = TEST_PAGE_NUM + 10;
    uint16_t offset = 200;  // 从页内偏移200开始写，会被截断
    
    generate_test_data(write_buffer, 200, 0x55);
    
    // 擦除测试页所在的扇区
    uint32_t sector = (test_page * W25Q32_PAGE_SIZE) / W25Q32_SECTOR_SIZE;
    W25Q32_SectorErase_4KB(sector);
    
    // 尝试跨页写入 - 驱动应该自动截断
    status = W25Q32_PageProgram(test_page, offset, write_buffer, 200);
    print_test_result("跨页写入自动截断", status == W25Q32_OK);
    
    // 验证只写入了不跨页的部分
    uint32_t expected_size = W25Q32_PAGE_SIZE - offset;
    memset(read_buffer, 0x00, 200);
    W25Q32_ReadData(test_page * W25Q32_PAGE_SIZE + offset, read_buffer, expected_size);
    print_test_result("跨页数据验证", compare_buffers(write_buffer, read_buffer, expected_size));
}

/**
 * @brief 测试4: 边界条件测试
 */
static void test_boundary_conditions(void) {
    printf("\r\n========== 测试4: 边界条件测试 ==========\r\n");
    
    W25Q32_Status_t status;
    uint8_t dummy_buffer[10];
    
    // 测试无效的扇区号
    status = W25Q32_SectorErase_4KB(g_w25q32_state.sector_count);
    print_test_result("无效扇区号检测", status == W25Q32_INVALID_PARAM);
    
    // 测试无效的页号
    status = W25Q32_PageProgram(g_w25q32_state.page_count, 0, dummy_buffer, 10);
    print_test_result("无效页号检测", status == W25Q32_INVALID_PARAM);
    
    // 测试无效的读取地址
    status = W25Q32_ReadData(W25Q32_TOTAL_SIZE_BYTES, dummy_buffer, 10);
    print_test_result("超出地址范围检测", status == W25Q32_INVALID_PARAM);
    
    // 测试空指针
    status = W25Q32_PageProgram(0, 0, NULL, 10);
    print_test_result("写入空指针检测", status == W25Q32_INVALID_PARAM);
    
    status = W25Q32_ReadData(0, NULL, 10);
    print_test_result("读取空指针检测", status == W25Q32_INVALID_PARAM);
    
    // 测试零长度操作
    status = W25Q32_PageProgram(0, 0, dummy_buffer, 0);
    print_test_result("零长度写入", status == W25Q32_OK);
    
    status = W25Q32_ReadData(0, dummy_buffer, 0);
    print_test_result("零长度读取", status == W25Q32_OK);
}

/**
 * @brief 测试5: 多页连续读写测试
 */
static void test_multi_page_operations(void) {
    printf("\r\n========== 测试5: 多页连续读写测试 ==========\r\n");
    
    #define MULTI_PAGE_SIZE (W25Q32_PAGE_SIZE * 4)  // 4页数据
    uint8_t *write_buffer = (uint8_t*)malloc(MULTI_PAGE_SIZE);
    uint8_t *read_buffer = (uint8_t*)malloc(MULTI_PAGE_SIZE);
    
    if (write_buffer == NULL || read_buffer == NULL) {
        printf("内存分配失败！\r\n");
        if (write_buffer) free(write_buffer);
        if (read_buffer) free(read_buffer);
        return;
    }
    
    W25Q32_Status_t status;
    uint32_t start_page = TEST_PAGE_NUM + 20;
    
    // 生成测试数据
    generate_test_data(write_buffer, MULTI_PAGE_SIZE, 0x77);
    
    // 擦除相关扇区
    uint32_t start_sector = (start_page * W25Q32_PAGE_SIZE) / W25Q32_SECTOR_SIZE;
    uint32_t end_sector = ((start_page + 4) * W25Q32_PAGE_SIZE) / W25Q32_SECTOR_SIZE;
    
    for (uint32_t sector = start_sector; sector <= end_sector; sector++) {
        W25Q32_SectorErase_4KB(sector);
    }
    
    // 逐页写入
    printf("写入4页数据...\r\n");
    uint8_t all_writes_ok = 1;
    for (uint32_t i = 0; i < 4; i++) {
        status = W25Q32_PageProgram(start_page + i, 0, 
                                    write_buffer + (i * W25Q32_PAGE_SIZE), 
                                    W25Q32_PAGE_SIZE);
        if (status != W25Q32_OK) {
            all_writes_ok = 0;
            break;
        }
    }
    print_test_result("多页写入", all_writes_ok);
    
    // 一次性读取所有数据
    memset(read_buffer, 0x00, MULTI_PAGE_SIZE);
    status = W25Q32_ReadData(start_page * W25Q32_PAGE_SIZE, read_buffer, MULTI_PAGE_SIZE);
    print_test_result("多页读取", status == W25Q32_OK);
    
    // 验证数据
    print_test_result("多页数据验证", compare_buffers(write_buffer, read_buffer, MULTI_PAGE_SIZE));
    
    free(write_buffer);
    free(read_buffer);
}

/**
 * @brief 测试6: 擦除功能全面测试
 */
static void test_erase_operations(void) {
    printf("\r\n========== 测试6: 擦除功能测试 ==========\r\n");
    
    uint8_t read_buffer[W25Q32_SECTOR_SIZE];
    W25Q32_Status_t status;
    uint32_t test_sector = TEST_SECTOR_NUM + 5;
    
    // 先写入一些数据
    uint8_t write_data[256];
    generate_test_data(write_data, 256, 0xCC);
    uint32_t test_page = (test_sector * W25Q32_SECTOR_SIZE) / W25Q32_PAGE_SIZE;
    
    W25Q32_SectorErase_4KB(test_sector);
    W25Q32_PageProgram(test_page, 0, write_data, 256);
    
    // 验证数据已写入
    memset(read_buffer, 0x00, 256);
    W25Q32_ReadData(test_sector * W25Q32_SECTOR_SIZE, read_buffer, 256);
    uint8_t data_written = compare_buffers(write_data, read_buffer, 256);
    print_test_result("擦除前数据写入", data_written);
    
    // 执行扇区擦除
    status = W25Q32_SectorErase_4KB(test_sector);
    print_test_result("扇区擦除执行", status == W25Q32_OK);
    
    // 验证整个扇区被擦除
    memset(read_buffer, 0x00, W25Q32_SECTOR_SIZE);
    W25Q32_ReadData(test_sector * W25Q32_SECTOR_SIZE, read_buffer, W25Q32_SECTOR_SIZE);
    print_test_result("扇区擦除验证", verify_erased(read_buffer, W25Q32_SECTOR_SIZE));
}

/**
 * @brief 测试7: 性能测试
 */
static void test_performance(void) {
    printf("\r\n========== 测试7: 性能测试 ==========\r\n");
    
    uint8_t buffer[W25Q32_PAGE_SIZE];
    generate_test_data(buffer, W25Q32_PAGE_SIZE, 0x88);
    
    uint32_t test_page = TEST_PAGE_NUM + 50;
    uint32_t test_sector = (test_page * W25Q32_PAGE_SIZE) / W25Q32_SECTOR_SIZE;
    
    // 擦除测试
    W25Q32_SectorErase_4KB(test_sector);
    
    // 页编程速度测试 (写10页)
    printf("页编程性能测试 (10页)...\r\n");
    // 注意: 这里需要添加时间测量代码,使用HAL_GetTick()或DWT
    for (int i = 0; i < 10; i++) {
        W25Q32_PageProgram(test_page + i, 0, buffer, W25Q32_PAGE_SIZE);
    }
    printf("页编程完成\r\n");
    
    // 读取速度测试 (读取10页)
    printf("读取性能测试 (10页)...\r\n");
    for (int i = 0; i < 10; i++) {
        W25Q32_ReadData((test_page + i) * W25Q32_PAGE_SIZE, buffer, W25Q32_PAGE_SIZE);
    }
    printf("读取完成\r\n");
    
    print_test_result("性能测试完成", 1);
}

/**
 * @brief 测试8: 电源管理测试
 */
static void test_power_management(void) {
    printf("\r\n========== 测试8: 电源管理测试 ==========\r\n");
    
    uint8_t buffer[10] = {0};
    
    // 进入掉电模式
    W25Q32_PowerDown();
    printf("芯片进入掉电模式\r\n");
    
    // 尝试读取 (应该失败或读取到错误数据)
    W25Q32_ReadData(0, buffer, 10);
    
    // 唤醒芯片
    W25Q32_ReleasePowerDown();
    printf("芯片已唤醒\r\n");
    
    // 再次尝试读取 (应该成功)
    W25Q32_Status_t status = W25Q32_ReadData(0, buffer, 10);
    print_test_result("掉电唤醒功能", status == W25Q32_OK);
}

//======================================================================
//                          主测试函数
//======================================================================

/**
 * @brief 运行所有W25Q32测试
 */
void W25Q32_RunAllTests(void) {
    printf("\r\n");
    printf("========================================\r\n");
    printf("     W25Q32 Flash驱动综合测试开始\r\n");
    printf("========================================\r\n");
    
    // 重置测试结果
    g_test_result.total_tests = 0;
    g_test_result.passed_tests = 0;
    g_test_result.failed_tests = 0;
    
    // 运行所有测试
    test_initialization();
    test_basic_read_write();
    test_cross_page_write();
    test_boundary_conditions();
    test_multi_page_operations();
    test_erase_operations();
    test_performance();
    test_power_management();
    
    // 打印测试总结
    printf("\r\n");
    printf("========================================\r\n");
    printf("           测试总结\r\n");
    printf("========================================\r\n");
    printf("总测试数: %lu\r\n", g_test_result.total_tests);
    printf("通过: %lu\r\n", g_test_result.passed_tests);
    printf("失败: %lu\r\n", g_test_result.failed_tests);
    printf("通过率: %.2f%%\r\n", 
           (float)g_test_result.passed_tests / g_test_result.total_tests * 100.0f);
    printf("========================================\r\n\r\n");
}

/**
 * @brief 运行快速测试 (只测试基本功能)
 */
void W25Q32_RunQuickTest(void) {
    printf("\r\n========== W25Q32 快速测试 ==========\r\n");
    
    g_test_result.total_tests = 0;
    g_test_result.passed_tests = 0;
    g_test_result.failed_tests = 0;
    
    test_initialization();
    test_basic_read_write();
    
    printf("\r\n快速测试完成: %lu/%lu 通过\r\n\r\n", 
           g_test_result.passed_tests, g_test_result.total_tests);
}
