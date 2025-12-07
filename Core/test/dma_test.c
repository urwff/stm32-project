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
static int test_custom_dma_init(void);

/* 公开函数实现 --------------------------------------------------------------*/

/**
 * @brief  运行所有 DMA 测试
 */
void DMA_RunAllTests(void) {
  printf("\r\n");
  printf("========================================\r\n");
  printf("      DMA Driver Test Suite Start       \r\n");
  printf("========================================\r\n");

  int result1 = test_dma_mem2mem_transfer();
  int result2 = test_custom_dma_init();

  if (result1 == TEST_PASS && result2 == TEST_PASS) {
    printf("\r\nTest Result: PASSED\r\n");
  } else {
    printf("\r\nTest Result: FAILED\r\n");
  }
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
    /* 如果超时，可能是因为传输已经完成了，或者真的出问题了 */
    /* 检查一下 TC 标志 */
    if (__HAL_DMA_GET_FLAG(&hdma_memtomem_dma1_channel1, DMA_FLAG_TC1)) {
      /* 传输其实完成了 */
      __HAL_DMA_CLEAR_FLAG(&hdma_memtomem_dma1_channel1, DMA_FLAG_TC1);
    } else {
      printf("  [FAIL] HAL_DMA_PollForTransfer failed! Status: %d\r\n", status);
      return TEST_FAIL;
    }
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

/**
 * @brief  测试自定义 DMA 寄存器初始化函数
 * @note   不执行实际传输，只验证寄存器配置是否正确
 * @retval TEST_PASS / TEST_FAIL
 */
static int test_custom_dma_init(void) {
  DMA_Config_t dma_cfg;
  /* 使用 DMA1_Channel2 进行测试，以免干扰 Main 中使用的 Channel1 */
  DMA_Channel_TypeDef *test_channel = DMA1_Channel2;
  uint32_t dummy_periph = 0x40001000;
  uint32_t dummy_mem = 0x20001000;

  printf("[TEST] Custom DMA Register Init\r\n");

  /* 1. 准备配置参数 */
  dma_cfg.PeriphBaseAddr = dummy_periph;
  dma_cfg.MemBaseAddr = dummy_mem;
  dma_cfg.Direction = DMA_DIR_PeripheralDST_Mem2Per; // M->P (DIR=1)
  dma_cfg.BufferSize = 128;
  dma_cfg.PeriphInc = DMA_Inc_Enable;          // PINC=1
  dma_cfg.MemInc = DMA_Inc_Enable;             // MINC=1
  dma_cfg.PeriphDataSize = DMA_DataSize_Word;  // PSIZE=10 (32bit)
  dma_cfg.MemDataSize = DMA_DataSize_HalfWord; // MSIZE=01 (16bit)
  dma_cfg.Mode = DMA_Mode_Circular;            // CIRC=1
  dma_cfg.Priority = DMA_Priority_High;        // PL=10
  dma_cfg.M2M = false;                         // MEM2MEM=0

  /* 先确保时钟开启 (虽然 Channel1 开启了 DMA1 时钟，这里再确保一下) */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* 2. 调用初始化函数 */
  if (DMA_Init(test_channel, &dma_cfg) != 0) {
    printf("  [FAIL] DMA_Init returned error\r\n");
    return TEST_FAIL;
  }

  /* 3. 验证寄存器值 */
  uint32_t ccr_val = test_channel->CCR;

  int fail = 0;

  /* 检查 CPAR 和 CMAR */
  if (test_channel->CPAR != dummy_periph) {
    printf("  [FAIL] CPAR mismatch: 0x%08lX (expected 0x%08lX)\r\n",
           test_channel->CPAR, dummy_periph);
    fail++;
  }
  if (test_channel->CMAR != dummy_mem) {
    printf("  [FAIL] CMAR mismatch: 0x%08lX (expected 0x%08lX)\r\n",
           test_channel->CMAR, dummy_mem);
    fail++;
  }
  if (test_channel->CNDTR != 128) {
    printf("  [FAIL] CNDTR mismatch: %lu (expected 128)\r\n",
           test_channel->CNDTR);
    fail++;
  }

  /* 检查 CCR各 位 */
  /* DIR (Bit 4) 应为 1 */
  if ((ccr_val & DMA_CCR_DIR) == 0) {
    printf("  [FAIL] CCR.DIR not set\r\n");
    fail++;
  }
  /* CIRC (Bit 5) 应为 1 */
  if ((ccr_val & DMA_CCR_CIRC) == 0) {
    printf("  [FAIL] CCR.CIRC not set\r\n");
    fail++;
  }
  /* PINC (Bit 6) 应为 1 */
  if ((ccr_val & DMA_CCR_PINC) == 0) {
    printf("  [FAIL] CCR.PINC not set\r\n");
    fail++;
  }
  /* MINC (Bit 7) 应为 1 */
  if ((ccr_val & DMA_CCR_MINC) == 0) {
    printf("  [FAIL] CCR.MINC not set\r\n");
    fail++;
  }
  /* PSIZE (Bit 9:8) 应为 10 (32bit) */
  if (((ccr_val >> 8) & 0x3) != 0x2) {
    printf("  [FAIL] CCR.PSIZE incorrect\r\n");
    fail++;
  }
  /* MSIZE (Bit 11:10) 应为 01 (16bit) */
  if (((ccr_val >> 10) & 0x3) != 0x1) {
    printf("  [FAIL] CCR.MSIZE incorrect\r\n");
    fail++;
  }
  /* PL (Bit 13:12) 应为 10 (High) */
  if (((ccr_val >> 12) & 0x3) != 0x2) {
    printf("  [FAIL] CCR.PL incorrect\r\n");
    fail++;
  }
  /* MEM2MEM (Bit 14) 应为 0 */
  if ((ccr_val & DMA_CCR_MEM2MEM) != 0) {
    printf("  [FAIL] CCR.MEM2MEM incorrectly set\r\n");
    fail++;
  }

  /* 4. 测试 DMA_Cmd */
  DMA_Cmd(test_channel, true);
  if ((test_channel->CCR & DMA_CCR_EN) == 0) {
    printf("  [FAIL] DMA_Cmd(true) did not set EN bit\r\n");
    fail++;
  }

  DMA_Cmd(test_channel, false);
  if ((test_channel->CCR & DMA_CCR_EN) != 0) {
    printf("  [FAIL] DMA_Cmd(false) did not clear EN bit\r\n");
    fail++;
  }

  if (fail == 0) {
    printf("  [PASS] Custom DMA Init & Cmd verified\r\n");
    return TEST_PASS;
  } else {
    return TEST_FAIL;
  }
}
