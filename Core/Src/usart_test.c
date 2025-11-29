#include "usart_test.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "usart.h"

extern uint8_t g_usart_rx_buffer[100];
extern volatile uint8_t g_usart_rx_len;
extern volatile uint8_t g_usart_message_ready;

#define LOOPBACK_MESSAGE "Hello, Interrupt Loopback Test!"
#define USART_TEST_TIMEOUT 500000U

/**
 * @brief  通过USART1发送测试日志消息
 * @param  msg: 要发送的消息字符串指针
 * @retval 无
 * 
 * @description
 * 内部辅助函数，用于在测试过程中输出日志信息
 * 使用自定义的USART1驱动函数发送字符串
 */
static void usart_test_log(const char *msg) {
  Driver_USART1_SendString((uint8_t *)msg, strlen(msg));
}

/**
 * @brief  重置USART接收状态
 * @param  无
 * @retval 无
 * 
 * @description
 * 清空接收缓冲区并重置相关状态变量，为新的测试做准备
 * - 清空接收缓冲区内容
 * - 重置接收长度计数器
 * - 清除消息接收完成标志
 */
static void reset_usart_rx_state(void) {
  memset(g_usart_rx_buffer, 0, sizeof(g_usart_rx_buffer));
  g_usart_rx_len = 0;
  g_usart_message_ready = 0;
}

/**
 * @brief  测试失败处理函数
 * @param  reason: 失败原因描述字符串
 * @retval TEST_STATUS_FAIL: 测试失败状态
 * 
 * @description
 * 当测试失败时调用此函数，输出失败原因并返回失败状态
 * 用于统一处理测试失败的情况
 */
static TestStatus fail_with_reason(const char *reason) {
  usart_test_log(reason);
  return TEST_STATUS_FAIL;
}

/**
 * @brief  执行中断模式环回测试
 * @param  无
 * @retval TestStatus: 测试结果状态
 * 
 * @description
 * 执行USART1中断模式的环回测试，验证中断接收和发送功能
 * 
 * 测试流程：
 * 1. 初始化USART1驱动
 * 2. 重置接收状态
 * 3. 发送测试消息
 * 4. 等待中断接收完成
 * 5. 验证接收数据的长度和内容
 * 
 * @note
 * 需要硬件环回连接（TX和RX短接）才能正常工作
 * 使用中断方式接收数据，依赖全局变量g_usart_message_ready标志
 */
static TestStatus run_interrupt_loopback(void) {
  Driver_USART1_Init();  // 初始化USART1驱动

  reset_usart_rx_state();  // 重置接收状态
  usart_test_log("\r\n[USART] Interrupt loopback test start\r\n");

  const size_t loopback_len = strlen(LOOPBACK_MESSAGE);
  Driver_USART1_SendString((uint8_t *)LOOPBACK_MESSAGE, loopback_len);  // 发送测试消息

  // 等待中断接收完成，带超时保护
  uint32_t timeout = USART_TEST_TIMEOUT;
  while (!g_usart_message_ready && timeout--) {
    // 空循环等待，由中断服务程序设置g_usart_message_ready标志
  }

  // 检查是否超时
  if (!g_usart_message_ready) {
    return fail_with_reason("[USART][ERR] Loopback timeout\r\n");
  }

  // 验证接收数据长度
  const uint8_t received_len = g_usart_rx_len;
  if (received_len != loopback_len) {
    char msg[80];
    sprintf(msg, "[USART][ERR] Loopback length mismatch exp=%lu act=%u\r\n",
            (unsigned long)loopback_len, received_len);
    usart_test_log(msg);
    return TEST_STATUS_FAIL;
  }

  // 验证接收数据内容
  if (memcmp(g_usart_rx_buffer, LOOPBACK_MESSAGE, received_len) != 0) {
    return fail_with_reason("[USART][ERR] Loopback payload mismatch\r\n");
  }

  usart_test_log("[USART] Interrupt loopback test PASS\r\n");
  return TEST_STATUS_PASS;
}

/**
 * @brief  USART环回测试接口函数
 * @param  无
 * @retval TestStatus: 测试结果状态
 * 
 * @description
 * 对外提供的USART环回测试接口，内部调用中断模式环回测试
 * 
 * @note
 * 这是main.c中调用的公共接口函数
 */
TestStatus usart_loopback_test(void) {
  return run_interrupt_loopback();
}

/**
 * @brief  USART阻塞模式收发测试
 * @param  无
 * @retval TestStatus: 测试结果状态
 * 
 * @description
 * 执行USART1阻塞模式的收发测试，验证阻塞式API功能
 * 
 * 测试流程：
 * 1. 检查缓冲区大小是否足够
 * 2. 发送测试字符串
 * 3. 阻塞接收数据
 * 4. 验证接收数据的长度和内容
 * 
 * @note
 * - 需要硬件环回连接（TX和RX短接）才能正常工作
 * - 使用阻塞式API，函数会等待接收完成后返回
 * - 接收缓冲区大小限制为32字节
 */
TestStatus usart_blocking_tx_rx_test(void) {
  const char *test_str = "Blocking TX/RX Test";  // 测试字符串
  const size_t test_len = strlen(test_str);
  uint8_t rx_buffer[32] = {0};  // 接收缓冲区
  uint8_t rx_len = 0;           // 实际接收长度

  // 检查缓冲区大小
  if (test_len >= sizeof(rx_buffer)) {
    return fail_with_reason("[USART][ERR] Blocking test buffer too small\r\n");
  }

  usart_test_log("\r\n[USART] Blocking TX/RX test start\r\n");
  Driver_USART1_SendString((uint8_t *)test_str, test_len);    // 发送测试数据
  Driver_USART1_ReceiveString(rx_buffer, &rx_len);           // 阻塞接收数据

  // 验证接收数据长度
  if (rx_len != test_len) {
    char msg[80];
    sprintf(msg, "[USART][ERR] Blocking length mismatch exp=%lu act=%u\r\n",
            (unsigned long)test_len, rx_len);
    usart_test_log(msg);
    return TEST_STATUS_FAIL;
  }

  // 验证接收数据内容
  if (memcmp(rx_buffer, test_str, rx_len) != 0) {
    return fail_with_reason("[USART][ERR] Blocking payload mismatch\r\n");
  }

  usart_test_log("[USART] Blocking TX/RX test PASS\r\n");
  return TEST_STATUS_PASS;
}
