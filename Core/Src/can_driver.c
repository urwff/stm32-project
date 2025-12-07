/**
 * @file    can_driver.c
 * @brief   STM32F103 bxCAN 驱动实现
 * @author  Generated based on interface design document
 * @date    2025-12-05
 *
 * @note    本文件包含带有 TODO 注释的代码框架，寄存器操作处预留填写位置
 *          请根据 TODO 提示完成寄存器操作代码
 */

/* Includes ------------------------------------------------------------------*/
#include "can_driver.h"

/* Private macro definitions -------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void CAN_GPIO_Init(void);
static int CAN_CalculateBTR(uint32_t baudrate, uint32_t *btr_value);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  配置 CAN 相关 GPIO 引脚
 * @note   默认使用 PA11(RX) / PA12(TX)，如需重映射请修改此函数
 */
static void CAN_GPIO_Init(void) {
  /* 步骤1：使能 GPIOB 和 AFIO 时钟 */
  // TODO: 设置 RCC->APB2ENR 寄存器
  // 寄存器：RCC->APB2ENR
  // 操作：置位 IOPBEN (bit 3) 使能 GPIOB 时钟
  //       置位 AFIOEN (bit 0) 使能 AFIO 时钟
  // -------------------------------------------------------------------------
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
  // -------------------------------------------------------------------------

  /* 步骤2：配置 CAN 引脚重映射到 PB8/PB9 (⚠️ 关键:保护调试接口配置) */
  // TODO: 设置 AFIO->MAPR 寄存器重定向 CAN 到 PB8(RX)/PB9(TX)
  // 寄存器：AFIO->MAPR
  // 位域：CAN_REMAP[1:0] = bit 14-13
  //       值 10 = CAN 重映射到 PB8/PB9
  //
  // ⚠️⚠️ 极端重要：必须使用读-改-写模式,避免影响其他位,特别是:
  //    SWJ_CFG[2:0] (bit 26-24) - 控制 JTAG/SWD 调试接口使能状态
  //    如果误操作这些位,会导致无法再次通过 ST-LINK 烧录！
  // -------------------------------------------------------------------------
  uint32_t mapr_temp = AFIO->MAPR;
  mapr_temp &= ~AFIO_MAPR_CAN_REMAP;       // 清除 CAN_REMAP 字段 (bit 14-13)
  mapr_temp |= AFIO_MAPR_CAN_REMAP_REMAP2; // 设置为 10 = PB8/PB9 映射
  AFIO->MAPR = mapr_temp;
  // -------------------------------------------------------------------------

  /* 步骤3：配置 GPIO 引脚模式 */
  // TODO: 设置 GPIOB->CRH 寄存器配置 PB8 PB9
  //       PB9(CAN_Tx)：复用推挽输出 mode=11 cnf=10
  //       PB8(CAN_Rx): 浮空输入 mode=00 cnf=01
  // 寄存器：GPIOB->CRH
  // 位域：CRH_MODE8 (bit 0-1)、CRH_CNF8 (bit 2-3)
  //       CRH_MODE9 (bit 4-5)、CRH_CNF9 (bit 6-7)
  // -------------------------------------------------------------------------
  // 配置 PB9 (CAN_Tx) 为复用推挽输出，速度 50MHz
  GPIOB->CRH |=
      GPIO_CRH_MODE9; // 设置 PB9 为输出模式，最大速度 50MHz (MODE9[1:0] = 11)
  GPIOB->CRH |= GPIO_CRH_CNF9_1; // 设置 PB9 为复用功能推挽输出 (CNF9[1:0] = 10)
  GPIOB->CRH &= ~GPIO_CRH_CNF9_0;

  // 配置 PB8 (CAN_Rx) 为浮空输入
  GPIOB->CRH &= ~GPIO_CRH_MODE8;  // 设置 PB8 为输入模式 (MODE8[1:0] = 00)
  GPIOB->CRH &= ~GPIO_CRH_CNF8_1; // 设置 PB8 为浮空输入 (CNF8[1:0] = 01)
  GPIOB->CRH |= GPIO_CRH_CNF8_0;
  // -------------------------------------------------------------------------
}

/**
 * @brief  根据波特率计算 BTR 寄存器值
 * @param  baudrate: 目标波特率 (如 500000)
 * @param  btr_value: [out] 计算得到的 BTR 寄存器值
 * @retval 0: 成功, -1: 波特率无法实现
 *
 * @note   波特率计算公式:
 *         BaudRate = APB1_CLK / ((BRP + 1) × (1 + (TS1 + 1) + (TS2 + 1)))
 *
 *         采样点推荐在 75%~87.5% 范围内
 *         典型配置：TS1=6, TS2=1 -> 采样点 = (1+7)/(1+7+2) = 80%
 */
static int CAN_CalculateBTR(uint32_t baudrate, uint32_t *btr_value) {
  uint32_t brp;
  uint32_t ts1 = 6;  /* TS1 = 7 Tq (寄存器值 = 实际值 - 1) */
  uint32_t ts2 = 1;  /* TS2 = 2 Tq (寄存器值 = 实际值 - 1) */
  uint32_t sjw = 0;  /* SJW = 1 Tq (寄存器值 = 实际值 - 1) */
  uint32_t tq_count; /* 每个位时间的 Tq 数 */

  /* 参数检查 */
  if (baudrate == 0 || btr_value == NULL) {
    return -1;
  }

  /* 每个位时间的 Tq 数 = 1 + (TS1+1) + (TS2+1) = 1 + 7 + 2 = 10 */
  tq_count = 1 + (ts1 + 1) + (ts2 + 1);

  /* 计算分频系数: BRP = APB1_CLK / (baudrate × tq_count) - 1 */
  brp = CAN_APB1_CLK_HZ / (baudrate * tq_count) - 1;

  /* 检查 BRP 范围 (0-1023, 即 10 位) */
  if (brp > 0x3FF) {
    return -1; /* 波特率过低 */
  }

  /* 组装 BTR 寄存器值 */
  // TODO: 根据 BRP, TS1, TS2, SJW 组装 BTR 值
  // 寄存器：CAN->BTR
  // 位域：BRP[9:0] = bit 0-9
  //       TS1[3:0] = bit 16-19
  //       TS2[2:0] = bit 20-22
  //       SJW[1:0] = bit 24-25
  // -------------------------------------------------------------------------
  *btr_value = (brp << CAN_BTR_BRP_Pos) | (ts1 << CAN_BTR_TS1_Pos) |
               (ts2 << CAN_BTR_TS2_Pos) | (sjw << CAN_BTR_SJW_Pos);
  // -------------------------------------------------------------------------

  return 0;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  初始化 CAN 外设
 * @param  baudrate: 波特率（如 500000 表示 500kbps）
 * @param  mode: 工作模式
 * @retval CAN_INIT_OK: 成功
 *         CAN_INIT_ENTER_TIMEOUT: 进入初始化模式超时
 *         CAN_INIT_EXIT_TIMEOUT: 退出初始化模式超时
 */
int CAN_Init(uint32_t baudrate, CAN_Mode_t mode) {
  uint32_t timeout;
  uint32_t btr_value = 0;

  /* 参数检查 */
  if (baudrate == 0) {
    return CAN_INIT_ENTER_TIMEOUT;
  }

  /* ========== 步骤1：使能 CAN 时钟 ========== */
  // TODO: 设置 RCC->APB1ENR 寄存器，使能 CAN1 时钟
  // 寄存器：RCC->APB1ENR
  // 操作：置位 CAN1EN (bit 25)
  // -------------------------------------------------------------------------
  RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
  // -------------------------------------------------------------------------

  /* ========== 步骤2：配置 GPIO 引脚 ========== */
  CAN_GPIO_Init();

  /* ========== 步骤3：请求进入初始化模式 ========== */
  // TODO: 置位 CAN_MCR.INRQ 请求进入初始化模式
  // 寄存器：CAN1->MCR
  // 操作：置位 INRQ (bit 0)
  // -------------------------------------------------------------------------
  CAN1->MCR |= CAN_MCR_INRQ;
  // -------------------------------------------------------------------------

  /* 等待 INAK 置位确认进入初始化模式 */
  timeout = CAN_TIMEOUT_VALUE;
  // TODO: 轮询等待 CAN_MSR.INAK 置位
  // 寄存器：CAN1->MSR
  // 条件：检测 INAK (bit 0) == 1
  // -------------------------------------------------------------------------
  while (!(CAN1->MSR & CAN_MSR_INAK)) {
    if (--timeout == 0) {
      return CAN_INIT_ENTER_TIMEOUT;
    }
  }
  // -------------------------------------------------------------------------

  /* ========== 步骤4：退出睡眠模式 ========== */
  // TODO: 清除 CAN_MCR.SLEEP 退出睡眠模式
  // 寄存器：CAN1->MCR
  // 操作：清除 SLEEP (bit 1)
  // -------------------------------------------------------------------------
  CAN1->MCR &= ~CAN_MCR_SLEEP;
  // -------------------------------------------------------------------------

  /* ========== 步骤5：配置 MCR 控制寄存器 ========== */
  // TODO: 配置 CAN_MCR 的其他功能位
  // 寄存器：CAN1->MCR
  // 建议配置：
  //   - ABOM = 1：使能自动离线恢复（Bus-Off 后自动恢复）
  //   - NART = 0：允许自动重传（发送失败自动重发）
  //   - RFLM = 0：接收 FIFO 覆盖模式（新报文覆盖旧报文）
  //   - TXFP = 0：按 ID 优先级发送
  //   - AWUM = 0：手动唤醒模式
  //   - TTCM = 0：关闭时间触发模式
  // -------------------------------------------------------------------------
  CAN1->MCR |= CAN_MCR_ABOM; /* 使能自动离线恢复 */

  CAN1->MCR |= CAN_MCR_AWUM; /* 手动唤醒模式 */

  // -------------------------------------------------------------------------

  /* ========== 步骤6：配置位时序寄存器 ========== */
  /* 计算 BTR 寄存器值 */
  if (CAN_CalculateBTR(baudrate, &btr_value) != 0) {
    return CAN_INIT_ENTER_TIMEOUT;
  }

  /* 根据工作模式添加 LBKM/SILM 位 */
  switch (mode) {
  case BX_CAN_MODE_LOOPBACK:
    // TODO: 添加环回模式位
    // 位域：LBKM (bit 30) = 1
    // -------------------------------------------------------------------------
    btr_value |= CAN_BTR_LBKM;
    // -------------------------------------------------------------------------
    break;

  case BX_CAN_MODE_SILENT:
    // TODO: 添加静默模式位
    // 位域：SILM (bit 31) = 1
    // -------------------------------------------------------------------------
    btr_value |= CAN_BTR_SILM;
    // -------------------------------------------------------------------------
    break;

  case BX_CAN_MODE_LOOPBACK_SILENT:
    // TODO: 添加环回+静默模式位
    // 位域：LBKM (bit 30) = 1, SILM (bit 31) = 1
    // -------------------------------------------------------------------------
    btr_value |= CAN_BTR_LBKM | CAN_BTR_SILM;
    // -------------------------------------------------------------------------
    break;

  case BX_CAN_MODE_NORMAL:
  default:
    /* 正常模式，不添加额外位 */
    break;
  }

  // TODO: 写入 BTR 寄存器
  // 寄存器：CAN1->BTR
  // 注意：BTR 只能在初始化模式下修改！
  // -------------------------------------------------------------------------
  CAN1->BTR = btr_value;
  // -------------------------------------------------------------------------

  /* ========== 步骤7：配置默认过滤器（接收所有报文） ========== */
  // TODO: 进入过滤器初始化模式
  // 寄存器：CAN1->FMR
  // 操作：置位 FINIT (bit 0)
  // -------------------------------------------------------------------------
  CAN1->FMR |= CAN_FMR_FINIT;
  // -------------------------------------------------------------------------

  // TODO: 配置过滤器 0 为 32位掩码模式，接收所有报文
  // 寄存器：CAN1->FM1R  - 过滤器模式寄存器
  //         CAN1->FS1R  - 过滤器位宽寄存器
  //         CAN1->FFA1R - 过滤器 FIFO 分配寄存器
  //         CAN1->sFilterRegister[0].FR1 - 过滤器 ID
  //         CAN1->sFilterRegister[0].FR2 - 过滤器掩码
  //         CAN1->FA1R  - 过滤器激活寄存器
  // 配置步骤：
  //   1. FM1R bit0 = 0: 过滤器 0 使用掩码模式
  //   2. FS1R bit0 = 1: 过滤器 0 使用 32 位位宽
  //   3. FFA1R bit0 = 0: 过滤器 0 分配给 FIFO0
  //   4. FR1 = 0, FR2 = 0: ID 和掩码都为 0，接收所有报文
  //   5. FA1R bit0 = 1: 激活过滤器 0
  // -------------------------------------------------------------------------
  CAN1->FM1R &= ~CAN_FM1R_FBM0;              /* 掩码模式 */
  CAN1->FS1R |= CAN_FS1R_FSC0;               /* 32 位位宽 */
  CAN1->FFA1R &= ~CAN_FFA1R_FFA0;            /* 分配给 FIFO0 */
  CAN1->sFilterRegister[0].FR1 = 0x00000000; /* ID = 0 */
  CAN1->sFilterRegister[0].FR2 = 0x00000000; /* Mask = 0（接收所有） */
  CAN1->FA1R |= CAN_FA1R_FACT0;              /* 激活过滤器 0 */
  // -------------------------------------------------------------------------

  // TODO: 退出过滤器初始化模式
  // 寄存器：CAN1->FMR
  // 操作：清除 FINIT (bit 0)
  // -------------------------------------------------------------------------
  CAN1->FMR &= ~CAN_FMR_FINIT;
  // -------------------------------------------------------------------------

  /* ========== 步骤8：退出初始化模式，进入正常模式 ========== */
  // TODO: 清除 CAN_MCR.INRQ 请求退出初始化模式
  // 寄存器：CAN1->MCR
  // 操作：清除 INRQ (bit 0)
  // -------------------------------------------------------------------------
  CAN1->MCR &= ~CAN_MCR_INRQ;
  // -------------------------------------------------------------------------

  /* 等待 INAK 清零确认退出初始化模式 */
  timeout = CAN_TIMEOUT_VALUE;
  // TODO: 轮询等待 CAN_MSR.INAK 清零
  // 寄存器：CAN1->MSR
  // 条件：检测 INAK (bit 0) == 0
  // 注意：退出初始化模式需要硬件等待 11 个连续隐性位完成同步
  // -------------------------------------------------------------------------
  while (CAN1->MSR & CAN_MSR_INAK) {
    if (--timeout == 0) {
      return CAN_INIT_EXIT_TIMEOUT;
    }
  }
  // -------------------------------------------------------------------------

  return CAN_INIT_OK;
}

/**
 * @brief  发送 CAN 报文（轮询方式）
 * @param  id: 报文标识符
 * @param  ide: 帧类型 (0=标准帧, 1=扩展帧)
 * @param  rtr: 远程帧标志 (0=数据帧, 1=远程帧)
 * @param  data: 数据指针
 * @param  len: 数据长度 (0-8)
 * @retval 0-2: 使用的邮箱号, CAN_TX_NO_MAILBOX: 无空闲邮箱
 */
int CAN_Transmit(uint32_t id, uint8_t ide, uint8_t rtr, uint8_t *data,
                 uint8_t len) {
  int mailbox = CAN_TX_NO_MAILBOX;
  uint32_t tsr;
  uint32_t tir = 0;

  /* 参数检查 */
  if (len > 8) {
    len = 8;
  }

  /* ========== 步骤1：查询空闲邮箱 ========== */
  // TODO: 读取 CAN_TSR 寄存器，检查 TME0/TME1/TME2 位
  // 寄存器：CAN1->TSR
  // 位域：TME0 (bit 26), TME1 (bit 27), TME2 (bit 28)
  //       CODE[1:0] (bit 24-25) 指示下一个空闲邮箱号
  // 策略：优先使用 CODE 指示的邮箱，或者逐个检查 TMEx
  // -------------------------------------------------------------------------
  tsr = CAN1->TSR;
  /* 使用 CODE 字段获取空闲邮箱号 */
  if (tsr & CAN_TSR_TME0) {
    mailbox = 0;
  } else if (tsr & CAN_TSR_TME1) {
    mailbox = 1;
  } else if (tsr & CAN_TSR_TME2) {
    mailbox = 2;
  } else {
    return CAN_TX_NO_MAILBOX; /* 所有邮箱都忙 */
  }
  // -------------------------------------------------------------------------

  if (mailbox < 0) {
    return CAN_TX_NO_MAILBOX;
  }

  /* ========== 步骤2：设置标识符寄存器 ========== */
  // TODO: 根据 ide 填充 CAN_TIxR 寄存器
  // 寄存器：CAN1->sTxMailBox[mailbox].TIR
  // 标准帧布局：STID[10:0] 位于 bit 21-31
  // 扩展帧布局：EXID[28:0] 位于 bit 3-31 (完整 29 位)，实际 EXID[17:0] 在 bit
  // 3-20 位域：TXRQ (bit 0) = 发送请求
  //       RTR (bit 1) = 远程帧标志
  //       IDE (bit 2) = 扩展帧标志
  //       STID[10:0] (bit 21-31) = 标准标识符
  //       EXID[17:0] (bit 3-20) = 扩展标识符低 18 位
  // -------------------------------------------------------------------------
  if (ide == 0) {
    /* 标准帧：ID 左移 21 位放入 STID 位域 */
    tir = (id << CAN_TI0R_STID_Pos) | (rtr ? CAN_TI0R_RTR : 0);
  } else {
    /* 扩展帧：ID 左移 3 位放入 EXID 位域，置位 IDE */
    tir = (id << CAN_TI0R_EXID_Pos) | CAN_TI0R_IDE | (rtr ? CAN_TI0R_RTR : 0);
  }
  CAN1->sTxMailBox[mailbox].TIR = tir;
  // -------------------------------------------------------------------------

  /* ========== 步骤3：设置数据长度 ========== */
  // TODO: 设置 CAN_TDTxR.DLC 字段
  // 寄存器：CAN1->sTxMailBox[mailbox].TDTR
  // 位域：DLC[3:0] (bit 0-3) = 数据长度码
  // -------------------------------------------------------------------------
  CAN1->sTxMailBox[mailbox].TDTR = (len & 0x0F);
  // -------------------------------------------------------------------------

  /* ========== 步骤4：填充数据（数据帧时） ========== */
  if (rtr == 0 && data != NULL) {
    // TODO: 将 data[0-3] 填入 CAN_TDLxR，data[4-7] 填入 CAN_TDHxR
    // 寄存器：CAN1->sTxMailBox[mailbox].TDLR (低 4 字节)
    //         CAN1->sTxMailBox[mailbox].TDHR (高 4 字节)
    // 布局：DATA0 在 bit 0-7, DATA1 在 bit 8-15, DATA2 在 bit 16-23, DATA3 在
    // bit 24-31
    // ---------------------------------------------------------------------
    // 填充低 4 字节数据 (DATA0-DATA3)
    CAN1->sTxMailBox[mailbox].TDLR = ((uint32_t)data[0] << 0) |  // DATA0
                                     ((uint32_t)data[1] << 8) |  // DATA1
                                     ((uint32_t)data[2] << 16) | // DATA2
                                     ((uint32_t)data[3] << 24);  // DATA3

    // 填充高 4 字节数据 (DATA4-DATA7)
    CAN1->sTxMailBox[mailbox].TDHR = ((uint32_t)data[4] << 0) |  // DATA4
                                     ((uint32_t)data[5] << 8) |  // DATA5
                                     ((uint32_t)data[6] << 16) | // DATA6
                                     ((uint32_t)data[7] << 24);  // DATA7
    // ---------------------------------------------------------------------
  }

  /* ========== 步骤5：请求发送 ========== */
  // TODO: 置位 CAN_TIxR.TXRQ 请求发送
  // 寄存器：CAN1->sTxMailBox[mailbox].TIR
  // 操作：置位 TXRQ (bit 0)
  // -------------------------------------------------------------------------
  CAN1->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;
  // -------------------------------------------------------------------------

  return mailbox;
}

/**
 * @brief  等待发送完成
 * @param  mailbox: 邮箱号 (0-2)
 * @param  timeout: 超时计数值
 * @retval CAN_TX_WAIT_OK/ALST/TERR/TIMEOUT
 */
int CAN_TransmitWait(uint8_t mailbox, uint32_t timeout) {
  uint32_t tsr;
  uint32_t rqcp_mask, txok_mask, alst_mask, terr_mask;

  /* 参数检查 */
  if (mailbox > 2) {
    return CAN_TX_WAIT_TERR;
  }

  /* 根据邮箱号确定状态位掩码 */
  switch (mailbox) {
  case 0:
    // TODO: 设置邮箱 0 的状态位掩码
    // 寄存器：CAN1->TSR
    // 位域：RQCP0 (bit 0), TXOK0 (bit 1), ALST0 (bit 2), TERR0 (bit 3)
    // -------------------------------------------------------------------------
    // rqcp_mask = CAN_TSR_RQCP0;
    // txok_mask = CAN_TSR_TXOK0;
    // alst_mask = CAN_TSR_ALST0;
    // terr_mask = CAN_TSR_TERR0;
    // -------------------------------------------------------------------------
    break;
  case 1:
    // TODO: 设置邮箱 1 的状态位掩码
    // 位域：RQCP1 (bit 8), TXOK1 (bit 9), ALST1 (bit 10), TERR1 (bit 11)
    // -------------------------------------------------------------------------
    // rqcp_mask = CAN_TSR_RQCP1;
    // txok_mask = CAN_TSR_TXOK1;
    // alst_mask = CAN_TSR_ALST1;
    // terr_mask = CAN_TSR_TERR1;
    // -------------------------------------------------------------------------
    break;
  case 2:
    // TODO: 设置邮箱 2 的状态位掩码
    // 位域：RQCP2 (bit 16), TXOK2 (bit 17), ALST2 (bit 18), TERR2 (bit 19)
    // -------------------------------------------------------------------------
    // rqcp_mask = CAN_TSR_RQCP2;
    // txok_mask = CAN_TSR_TXOK2;
    // alst_mask = CAN_TSR_ALST2;
    // terr_mask = CAN_TSR_TERR2;
    // -------------------------------------------------------------------------
    break;
  default:
    return CAN_TX_WAIT_TERR;
  }

  /* ========== 轮询等待 RQCP 置位 ========== */
  // TODO: 轮询 CAN_TSR.RQCPx 直到置位或超时
  // 寄存器：CAN1->TSR
  // 条件：检测 RQCPx == 1（请求完成）
  // -------------------------------------------------------------------------
  // while (timeout > 0) {
  //     tsr = CAN1->TSR;
  //     if (tsr & rqcp_mask) {
  //         break;  /* 请求完成 */
  //     }
  //     timeout--;
  // }
  //
  // if (timeout == 0) {
  //     return CAN_TX_WAIT_TIMEOUT;
  // }
  // -------------------------------------------------------------------------

  /* ========== 检查发送结果 ========== */
  // TODO: 读取 TSR 寄存器判断发送结果
  // 寄存器：CAN1->TSR
  // 判断：TXOK = 1 表示成功，ALST = 1 表示仲裁丢失，TERR = 1 表示错误
  // -------------------------------------------------------------------------
  // tsr = CAN1->TSR;
  //
  // /* 清除 RQCP 标志（写 1 清除） */
  // CAN1->TSR = rqcp_mask;
  //
  // if (tsr & txok_mask) {
  //     return CAN_TX_WAIT_OK;
  // } else if (tsr & alst_mask) {
  //     return CAN_TX_WAIT_ALST;
  // } else {
  //     return CAN_TX_WAIT_TERR;
  // }
  // -------------------------------------------------------------------------

  return CAN_TX_WAIT_OK; /* 临时返回 */
}

/**
 * @brief  接收 CAN 报文（轮询方式）
 * @param  fifo: FIFO 号 (0 或 1)
 * @param  id: [out] 接收到的报文标识符
 * @param  ide: [out] 帧类型
 * @param  rtr: [out] 远程帧标志
 * @param  data: [out] 数据缓冲区
 * @param  len: [out] 数据长度
 * @retval CAN_RX_OK/CAN_RX_EMPTY
 */
int CAN_Receive(uint8_t fifo, uint32_t *id, uint8_t *ide, uint8_t *rtr,
                uint8_t *data, uint8_t *len) {
  uint32_t rir, rdtr, rdlr, rdhr;
  __IO uint32_t *rfr; /* FIFO 状态寄存器指针 */

  /* 参数检查 */
  if (fifo > 1 || id == NULL || ide == NULL || rtr == NULL || data == NULL ||
      len == NULL) {
    return CAN_RX_EMPTY;
  }

  /* ========== 步骤1：检查 FIFO 是否有待处理报文 ========== */
  // TODO: 读取 CAN_RFxR 寄存器，检查 FMP[1:0] 字段
  // 寄存器：CAN1->RF0R (FIFO0) 或 CAN1->RF1R (FIFO1)
  // 位域：FMP[1:0] (bit 0-1) = 待处理消息数量
  //       值 00 = 无消息，01/10/11 = 1/2/3 条消息
  // -------------------------------------------------------------------------
  if (fifo == 0) {
    rfr = &CAN1->RF0R;
    if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0) {
      return CAN_RX_EMPTY;
    }
  } else {
    rfr = &CAN1->RF1R;
    if ((CAN1->RF1R & CAN_RF1R_FMP1) == 0) {
      return CAN_RX_EMPTY;
    }
  }
  // -------------------------------------------------------------------------

  /* ========== 步骤2：读取标识符寄存器 ========== */
  // TODO: 从 CAN_RIxR 读取标识符和帧类型
  // 寄存器：CAN1->sFIFOMailBox[fifo].RIR
  // 位域：IDE (bit 2) = 扩展帧标志
  //       RTR (bit 1) = 远程帧标志
  //       STID[10:0] (bit 21-31) = 标准标识符
  //       EXID[17:0] (bit 3-20) = 扩展标识符
  // -------------------------------------------------------------------------
  rir = CAN1->sFIFOMailBox[fifo].RIR;

  /* 解析帧类型 */
  *ide = (rir & CAN_RI0R_IDE) ? 1 : 0;
  *rtr = (rir & CAN_RI0R_RTR) ? 1 : 0;

  /* 解析标识符 */
  if (*ide == 0) {
    /* 标准帧：右移 21 位获取 STID */
    *id = (rir >> CAN_RI0R_STID_Pos) & 0x7FF;
  } else {
    /* 扩展帧：右移 3 位获取完整 29 位 ID */
    *id = (rir >> CAN_RI0R_EXID_Pos) & 0x1FFFFFFF;
  }
  // -------------------------------------------------------------------------

  /* ========== 步骤3：读取数据长度 ========== */
  // TODO: 从 CAN_RDTxR 读取 DLC 字段
  // 寄存器：CAN1->sFIFOMailBox[fifo].RDTR
  // 位域：DLC[3:0] (bit 0-3) = 数据长度码
  //       FMI[7:0] (bit 8-15) = 过滤器匹配索引（可选读取）
  // -------------------------------------------------------------------------
  rdtr = CAN1->sFIFOMailBox[fifo].RDTR;
  *len = rdtr & 0x0F;
  if (*len > 8) {
    *len = 8;
  }
  // -------------------------------------------------------------------------

  /* ========== 步骤4：读取数据（数据帧时） ========== */
  // TODO: 从 CAN_RDLxR 和 CAN_RDHxR 读取数据
  // 寄存器：CAN1->sFIFOMailBox[fifo].RDLR (低 4 字节)
  //         CAN1->sFIFOMailBox[fifo].RDHR (高 4 字节)
  // 布局：DATA0 在 bit 0-7, DATA1 在 bit 8-15, 以此类推
  // -------------------------------------------------------------------------
  rdlr = CAN1->sFIFOMailBox[fifo].RDLR;
  rdhr = CAN1->sFIFOMailBox[fifo].RDHR;

  data[0] = (uint8_t)(rdlr >> 0);
  data[1] = (uint8_t)(rdlr >> 8);
  data[2] = (uint8_t)(rdlr >> 16);
  data[3] = (uint8_t)(rdlr >> 24);
  data[4] = (uint8_t)(rdhr >> 0);
  data[5] = (uint8_t)(rdhr >> 8);
  data[6] = (uint8_t)(rdhr >> 16);
  data[7] = (uint8_t)(rdhr >> 24);
  // -------------------------------------------------------------------------

  /* ========== 步骤5：释放 FIFO 邮箱 ========== */
  // TODO: 置位 CAN_RFxR.RFOM 释放邮箱
  // 寄存器：CAN1->RF0R 或 CAN1->RF1R
  // 操作：置位 RFOM0 (bit 5) 或 RFOM1 (bit 5)
  // 注意：必须释放邮箱才能读取下一条报文！
  // -------------------------------------------------------------------------
  if (fifo == 0) {
    CAN1->RF0R |= CAN_RF0R_RFOM0;
  } else {
    CAN1->RF1R |= CAN_RF1R_RFOM1;
  }
  // -------------------------------------------------------------------------

  return CAN_RX_OK;
}

/**
 * @brief  配置 CAN 过滤器
 * @param  filter_num: 过滤器编号 (0-13)
 * @param  mode: 过滤器模式
 * @param  scale: 过滤器位宽
 * @param  fifo: 分配的 FIFO (0 或 1)
 * @param  id: ID 值
 * @param  mask: 掩码值
 * @retval CAN_FILTER_OK/CAN_FILTER_PARAM_ERROR
 */
int CAN_FilterConfig(uint8_t filter_num, CAN_FilterMode_t mode,
                     CAN_FilterScale_t scale, uint8_t fifo, uint32_t id,
                     uint32_t mask) {
  uint32_t filter_bit;

  /* 参数检查 */
  if (filter_num >= CAN_FILTER_COUNT || fifo > 1) {
    return CAN_FILTER_PARAM_ERROR;
  }

  filter_bit = (1UL << filter_num);

  /* ========== 步骤1：进入过滤器初始化模式 ========== */
  // TODO: 置位 CAN_FMR.FINIT
  // 寄存器：CAN1->FMR
  // 操作：置位 FINIT (bit 0)
  // -------------------------------------------------------------------------
  // CAN1->FMR |= CAN_FMR_FINIT;
  // -------------------------------------------------------------------------

  /* ========== 步骤2：禁用目标过滤器 ========== */
  // TODO: 清除 CAN_FA1R 对应位
  // 寄存器：CAN1->FA1R
  // 操作：清除 filter_num 对应的位
  // -------------------------------------------------------------------------
  // CAN1->FA1R &= ~filter_bit;
  // -------------------------------------------------------------------------

  /* ========== 步骤3：配置过滤器模式 ========== */
  // TODO: 设置 CAN_FM1R 对应位
  // 寄存器：CAN1->FM1R
  // 操作：bit = 0 表示掩码模式，bit = 1 表示列表模式
  // -------------------------------------------------------------------------
  // if (mode == CAN_FILTER_MODE_LIST) {
  //     CAN1->FM1R |= filter_bit;   /* 列表模式 */
  // } else {
  //     CAN1->FM1R &= ~filter_bit;  /* 掩码模式 */
  // }
  // -------------------------------------------------------------------------

  /* ========== 步骤4：配置过滤器位宽 ========== */
  // TODO: 设置 CAN_FS1R 对应位
  // 寄存器：CAN1->FS1R
  // 操作：bit = 0 表示双 16 位，bit = 1 表示单 32 位
  // -------------------------------------------------------------------------
  // if (scale == CAN_FILTER_SCALE_32BIT) {
  //     CAN1->FS1R |= filter_bit;   /* 32 位位宽 */
  // } else {
  //     CAN1->FS1R &= ~filter_bit;  /* 16 位位宽 */
  // }
  // -------------------------------------------------------------------------

  /* ========== 步骤5：配置 FIFO 分配 ========== */
  // TODO: 设置 CAN_FFA1R 对应位
  // 寄存器：CAN1->FFA1R
  // 操作：bit = 0 分配给 FIFO0，bit = 1 分配给 FIFO1
  // -------------------------------------------------------------------------
  // if (fifo == 1) {
  //     CAN1->FFA1R |= filter_bit;   /* 分配给 FIFO1 */
  // } else {
  //     CAN1->FFA1R &= ~filter_bit;  /* 分配给 FIFO0 */
  // }
  // -------------------------------------------------------------------------

  /* ========== 步骤6：设置过滤器值 ========== */
  // TODO: 写入 CAN_FiR1 和 CAN_FiR2
  // 寄存器：CAN1->sFilterRegister[filter_num].FR1
  //         CAN1->sFilterRegister[filter_num].FR2
  // 32位掩码模式：FR1 = ID, FR2 = Mask
  // 32位列表模式：FR1 = ID1, FR2 = ID2
  // 16位模式：每个寄存器包含两个 16 位值
  // -------------------------------------------------------------------------
  // CAN1->sFilterRegister[filter_num].FR1 = id;
  // CAN1->sFilterRegister[filter_num].FR2 = mask;
  // -------------------------------------------------------------------------

  /* ========== 步骤7：激活过滤器 ========== */
  // TODO: 置位 CAN_FA1R 对应位
  // 寄存器：CAN1->FA1R
  // 操作：置位 filter_num 对应的位
  // -------------------------------------------------------------------------
  // CAN1->FA1R |= filter_bit;
  // -------------------------------------------------------------------------

  /* ========== 步骤8：退出过滤器初始化模式 ========== */
  // TODO: 清除 CAN_FMR.FINIT
  // 寄存器：CAN1->FMR
  // 操作：清除 FINIT (bit 0)
  // -------------------------------------------------------------------------
  // CAN1->FMR &= ~CAN_FMR_FINIT;
  // -------------------------------------------------------------------------

  return CAN_FILTER_OK;
}

/**
 * @brief  获取 CAN 错误状态
 * @param  tec: [out] 发送错误计数器值
 * @param  rec: [out] 接收错误计数器值
 * @param  lec: [out] 最后错误码
 * @retval 错误标志组合 (bit0=EWGF, bit1=EPVF, bit2=BOFF)
 */
uint8_t CAN_GetError(uint8_t *tec, uint8_t *rec, uint8_t *lec) {
  uint32_t esr;
  uint8_t error_flags = 0;

  /* ========== 读取 ESR 寄存器 ========== */
  // TODO: 读取 CAN_ESR 寄存器
  // 寄存器：CAN1->ESR
  // 位域：EWGF (bit 0) = 错误警告标志
  //       EPVF (bit 1) = 错误被动标志
  //       BOFF (bit 2) = 离线标志
  //       LEC[2:0] (bit 4-6) = 最后错误码
  //       TEC[7:0] (bit 16-23) = 发送错误计数器
  //       REC[7:0] (bit 24-31) = 接收错误计数器
  // -------------------------------------------------------------------------
  // esr = CAN1->ESR;
  //
  // /* 提取错误标志 */
  // error_flags = esr & 0x07;  /* EWGF | EPVF | BOFF */
  //
  // /* 提取各字段 */
  // if (tec != NULL) {
  //     *tec = (esr >> CAN_ESR_TEC_Pos) & 0xFF;
  // }
  // if (rec != NULL) {
  //     *rec = (esr >> CAN_ESR_REC_Pos) & 0xFF;
  // }
  // if (lec != NULL) {
  //     *lec = (esr >> CAN_ESR_LEC_Pos) & 0x07;
  // }
  // -------------------------------------------------------------------------

  return error_flags;
}

/**
 * @brief  获取 FIFO 中待处理消息数量
 * @param  fifo: FIFO 号 (0 或 1)
 * @retval 待处理消息数量 (0-3)
 */
uint8_t CAN_GetPendingMessages(uint8_t fifo) {
  // TODO: 读取 CAN_RFxR.FMP 字段
  // 寄存器：CAN1->RF0R 或 CAN1->RF1R
  // 位域：FMP[1:0] (bit 0-1) = 待处理消息数量
  // -------------------------------------------------------------------------
  // if (fifo == 0) {
  //     return (CAN1->RF0R & CAN_RF0R_FMP0);
  // } else {
  //     return (CAN1->RF1R & CAN_RF1R_FMP1);
  // }
  // -------------------------------------------------------------------------

  return 0; /* 临时返回 */
}

/************************ END OF FILE *****************************************/
