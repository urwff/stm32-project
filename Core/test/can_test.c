/**
 * @file    can_test.c
 * @brief   HAL CAN 驱动测试文件
 * @note    基于 HAL 库的 CAN 测试，利用 Silent Loopback 模式进行自测
 */

#include "can_test.h"
#include "can.h"
#include <string.h>

/* 引用 can.c 中定义的句柄 */
extern CAN_HandleTypeDef hcan;

/* 私有宏定义 ----------------------------------------------------------------*/
#define TEST_PASS 1
#define TEST_FAIL 0
#define CAN_TIMEOUT_MS 100

/* 私有函数声明 --------------------------------------------------------------*/
static int test_can_hal_loopback(void);

/* 公开函数实现 --------------------------------------------------------------*/

/**
 * @brief  运行 HAL CAN 测试
 */
void CAN_RunHALTests(void) {
  printf("\r\n");
  printf("========================================\r\n");
  printf("      HAL CAN Driver Test Suite         \r\n");
  printf("========================================\r\n");

  int result = test_can_hal_loopback();

  printf("\r\nTest Result: %s\r\n",
         (result == TEST_PASS) ? "PASSED" : "FAILED");
  printf("========================================\r\n");
}

/* 私有函数实现 --------------------------------------------------------------*/

/**
 * @brief  测试 CAN 环回模式 (Silent Loopback)
 * @retval TEST_PASS / TEST_FAIL
 */
static int test_can_hal_loopback(void) {
  CAN_TxHeaderTypeDef TxHeader;
  CAN_RxHeaderTypeDef RxHeader;
  uint8_t TxData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  uint8_t RxData[8] = {0};
  uint32_t TxMailbox;
  CAN_FilterTypeDef sFilterConfig;

  printf("[TEST] CAN HAL Loopback (Self-Test)\r\n");

  /* 1. 确保 CAN 已初始化 (MX_CAN_Init 应该已经被 main
   * 调用，但我们这里重置一下以确保状态) */
  /* 注意：can.c 中 MX_CAN_Init 使用了
   * CAN_MODE_SILENT_LOOPBACK，这正是我们需要的 */
  if (HAL_CAN_GetState(&hcan) == HAL_CAN_STATE_RESET) {
    MX_CAN_Init();
  }

  /* 2. 配置过滤器：接收所有 ID */
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
    printf("  [FAIL] HAL_CAN_ConfigFilter failed\r\n");
    return TEST_FAIL;
  }

  /* 3. 启动 CAN */
  if (HAL_CAN_Start(&hcan) != HAL_OK) {
    printf("  [FAIL] HAL_CAN_Start failed\r\n");
    return TEST_FAIL;
  }

  /* 4. 准备发送消息 (标准帧 ID: 0x321) */
  TxHeader.StdId = 0x321;
  TxHeader.ExtId = 0x01;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.DLC = 8;
  TxHeader.TransmitGlobalTime = DISABLE;

  /* 5. 发送消息 */
  if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
    printf("  [FAIL] HAL_CAN_AddTxMessage failed\r\n");
    /* 尝试停止 CAN 以便下次测试 */
    HAL_CAN_Stop(&hcan);
    return TEST_FAIL;
  }

  printf("  Message transmitted (Mailbox: %lu)\r\n", TxMailbox);

  /* 6. 等待接收 (轮询 FIFO0) */
  uint32_t tickstart = HAL_GetTick();
  while (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) == 0) {
    if ((HAL_GetTick() - tickstart) > CAN_TIMEOUT_MS) {
      printf("  [FAIL] Receive timeout (No message in FIFO0)\r\n");
      HAL_CAN_Stop(&hcan);
      return TEST_FAIL;
    }
  }

  /* 7. 读取消息 */
  if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) {
    printf("  [FAIL] HAL_CAN_GetRxMessage failed\r\n");
    HAL_CAN_Stop(&hcan);
    return TEST_FAIL;
  }

  /* 8. 停止 CAN (测试结束) */
  HAL_CAN_Stop(&hcan);

  /* 9. 验证数据 */
  int data_match = (memcmp(TxData, RxData, 8) == 0);
  int id_match = (RxHeader.StdId == 0x321);

  if (data_match && id_match) {
    printf("  [PASS] ID and Data match\r\n");
    return TEST_PASS;
  } else {
    printf("  [FAIL] Verification mismatch\r\n");
    if (!id_match)
      printf("    ID Expected: 0x321, Actual: 0x%lX\r\n", RxHeader.StdId);
    if (!data_match)
      printf("    Data mismatch\r\n");
    return TEST_FAIL;
  }
}
