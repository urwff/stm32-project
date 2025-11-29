/**
 * @file w25q32_test_example.c
 * @brief W25Q32测试的使用示例
 * @version 1.0
 * @date 2025-11-29
 * 
 * 此文件展示如何在main函数中调用W25Q32测试
 */

#include "main.h"
#include "w25q32_test.h"
#include "usart.h"
#include <stdio.h>

/**
 * @brief 使用示例: 在main函数中调用W25Q32测试
 * 
 * 将以下代码添加到您的main.c文件中:
 */
void W25Q32_Test_Example(void) {
    /* 
     * 1. 首先确保已经初始化了所需的外设:
     *    - SPI外设 (用于与W25Q32通信)
     *    - USART外设 (用于输出测试结果)
     * 
     * 例如:
     * MX_SPI1_Init();   // 初始化SPI
     * MX_USART1_UART_Init();  // 初始化串口
     * 
     * 2. 重定向printf到串口 (如果需要)
     *    确保实现了 _write() 或 fputc() 函数
     */
    
    printf("\r\n系统初始化完成\r\n");
    printf("开始W25Q32 Flash测试...\r\n\r\n");
    
    /* 
     * 方式1: 运行完整测试套件
     * 测试时间较长(可能需要几秒到几十秒)
     * 适用于全面验证芯片功能
     */
    W25Q32_RunAllTests();
    
    /* 
     * 方式2: 运行快速测试
     * 只测试基本的初始化和读写功能
     * 适用于快速验证芯片是否正常工作
     */
    // W25Q32_RunQuickTest();
    
    printf("测试完成！\r\n");
}

/**
 * @brief 典型的main函数集成示例
 */
void Main_Function_Example(void) {
    // 以下是伪代码示例，展示如何在实际项目中集成测试
    
    /* 系统时钟配置 */
    // SystemClock_Config();
    
    /* 初始化外设 */
    // MX_GPIO_Init();
    // MX_SPI1_Init();      // 初始化SPI，用于W25Q32通信
    // MX_USART1_UART_Init(); // 初始化串口，用于输出测试结果
    
    /* 可选: 添加启动延时，确保W25Q32上电稳定 */
    // HAL_Delay(100);
    
    /* 运行W25Q32测试 */
    // W25Q32_RunAllTests();  // 或使用 W25Q32_RunQuickTest()
    
    /* 主循环 */
    while(1) {
        // 您的应用代码
        
        /* 可以添加按键触发测试 */
        // if (按键按下) {
        //     W25Q32_RunQuickTest();
        // }
    }
}

/**
 * @brief 如何集成到现有项目
 * 
 * 步骤1: 将以下文件添加到项目中
 *   - Core/Inc/w25q32.h
 *   - Core/Src/w25q32.c
 *   - Core/Inc/w25q32_test.h
 *   - Core/Src/w25q32_test.c
 * 
 * 步骤2: 在main.c中添加头文件
 *   #include "w25q32_test.h"
 * 
 * 步骤3: 确保SPI和USART已正确初始化
 * 
 * 步骤4: 在适当的位置调用测试函数
 *   - 开发阶段: 在main函数开始处调用 W25Q32_RunAllTests()
 *   - 生产测试: 通过按键或命令触发测试
 *   - 日常验证: 使用 W25Q32_RunQuickTest()
 * 
 * 步骤5: 通过串口查看测试结果
 *   - 波特率: 通常为115200
 *   - 使用串口调试助手或终端软件接收数据
 */

/**
 * @brief 串口重定向示例 (如果需要)
 * 
 * 方法1: 重定向到USART1 (使用HAL库)
 */

/**
 * @brief 测试结果解读
 * 
 * 测试输出格式:
 * [PASS] 测试名称    - 测试通过
 * [FAIL] 测试名称    - 测试失败
 * 
 * 常见问题排查:
 * 
 * 1. 如果"芯片初始化和ID验证"失败:
 *    - 检查SPI连接是否正确 (CLK, MISO, MOSI, CS)
 *    - 检查W25Q32供电是否正常 (3.3V)
 *    - 检查SPI时钟速度是否合适 (建议≤20MHz)
 * 
 * 2. 如果读写测试失败:
 *    - 检查SPI通信时序是否正确
 *    - 检查片选信号是否正常工作
 *    - 尝试降低SPI时钟速度
 * 
 * 3. 如果擦除测试失败:
 *    - 检查是否有写保护引脚拉低
 *    - 确认擦除时间足够 (增加超时计数器的值)
 * 
 * 4. 如果部分测试超时:
 *    - 增大 W25Q32_WaitForWriteEnd() 中的超时计数器
 *    - 检查SPI通信是否稳定
 */

/**
 * @brief 性能优化建议
 * 
 * 1. 对于大量数据写入:
 *    - 按页对齐数据 (256字节)
 *    - 批量擦除扇区，然后连续写入
 * 
 * 2. 对于频繁读取:
 *    - 使用快速读取命令 (需要修改驱动)
 *    - 考虑使用Dual/Quad SPI模式
 * 
 * 3. 减少擦除次数:
 *    - 使用文件系统 (如LittleFS, FatFS)
 *    - 实现磨损均衡算法
 */
