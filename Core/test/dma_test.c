/**
 * @file    dma_test.c
 * @brief   DMA 驱动测试文件
 * @note    测试 dma.c 中配置的内存到内存 (Mem2Mem) 传输功能
 */

#include "dma_test.h"
#include "dma.h"
#include <string.h>

/* 引用 dma.c 中定义的句柄 */
extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

/* 私有宏定义 ----------------------------------------------------------------*/
#define TEST_BUFFER_SIZE 32
#define TEST_PASS 1
#define TEST_FAIL 0

/* 私有函数声明 --------------------------------------------------------------*/
static int test_dma_mem2mem_transfer(void);

/* 公开函数实现 --------------------------------------------------------------*/

/**
 * @brief  运行所有 DMA 测试
 */
void DMA_RunAllTests(void) {
  printf("\r\n");
  printf("========================================\r\n");
  printf("      DMA Driver Test Suite Start       \r\n");
  printf("========================================\r\n");

  int result = test_dma_mem2mem_transfer();

  printf("\r\nTest Result: %s\r\n",
         (result == TEST_PASS) ? "PASSED" : "FAILED");
  printf("========================================\r\n");
}

/* 私有函数实现 --------------------------------------------------------------*/

/**
 * @brief  测试内存到内存的 DMA 传输
 * @retval TEST_PASS / TEST_FAIL
 */
static int test_dma_mem2mem_transfer(void) {
  uint8_t src_buffer[TEST_BUFFER_SIZE];
  uint8_t dst_buffer[TEST_BUFFER_SIZE];
  HAL_StatusTypeDef status;

  printf("[TEST] DMA Mem2Mem Transfer\r\n");

  /* 1. 准备测试数据 */
  for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
    src_buffer[i] = i + 0x10;
    dst_buffer[i] = 0x00;
  }

  /* 2. 启动 DMA 传输 */
  /* 注意：根据 MX_DMA_Init 配置，Direction 是 MEMORY_TO_MEMORY */
  status = HAL_DMA_Start(&hdma_memtomem_dma1_channel1, (uint32_t)src_buffer,
                         (uint32_t)dst_buffer, TEST_BUFFER_SIZE);

  if (status != HAL_OK) {
    printf("  [FAIL] HAL_DMA_Start failed! Status: %d\r\n", status);
    return TEST_FAIL;
  }

  /* 3. 等待传输完成 (轮询方式) */
  /* 内存到内存传输通常很快，但也需要 Poll */
  status = HAL_DMA_PollForTransfer(&hdma_memtomem_dma1_channel1,
                                   HAL_DMA_FULL_TRANSFER, 100);

  if (status != HAL_OK) {
    printf("  [FAIL] HAL_DMA_PollForTransfer failed! Status: %d\r\n", status);
    return TEST_FAIL;
  }

  /* 4. 校验数据 */
  if (memcmp(src_buffer, dst_buffer, TEST_BUFFER_SIZE) == 0) {
    printf("  [PASS] Data verification successful (%d bytes)\r\n",
           TEST_BUFFER_SIZE);
    return TEST_PASS;
  } else {
    printf("  [FAIL] Data mismatch!\r\n");
    printf("    Expected: %02X %02X ...\r\n", src_buffer[0], src_buffer[1]);
    printf("    Actual:   %02X %02X ...\r\n", dst_buffer[0], dst_buffer[1]);
    return TEST_FAIL;
  }
}
