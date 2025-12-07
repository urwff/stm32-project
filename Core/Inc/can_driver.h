/**
 * @file    can_driver.h
 * @brief   STM32F103 bxCAN 驱动头文件
 * @author  Generated based on interface design document
 * @date    2025-12-05
 *
 * @note    适用于 STM32F103 系列芯片，CAN 2.0A/B 协议
 *          最大波特率：1 Mbit/s
 */

#ifndef __CAN_DRIVER_H
#define __CAN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f103xe.h"
#include <stdio.h>
/* Exported types ------------------------------------------------------------*/

/**
 * @brief CAN 工作模式枚举
 */
typedef enum {
  BX_CAN_MODE_NORMAL = 0,         /**< 正常模式：正常收发报文 */
  BX_CAN_MODE_LOOPBACK = 1,       /**< 环回模式：自发自收用于自检 */
  BX_CAN_MODE_SILENT = 2,         /**< 静默模式：只接收不发送（总线分析） */
  BX_CAN_MODE_LOOPBACK_SILENT = 3 /**< 环回+静默模式：热自检，不影响总线 */
} CAN_Mode_t;

/**
 * @brief CAN 过滤器模式枚举
 */
typedef enum {
  CAN_FILTER_MODE_MASK = 0, /**< 掩码模式：ID与掩码匹配 */
  CAN_FILTER_MODE_LIST = 1  /**< 列表模式：精确匹配ID列表 */
} CAN_FilterMode_t;

/**
 * @brief CAN 过滤器位宽枚举
 */
typedef enum {
  CAN_FILTER_SCALE_16BIT = 0, /**< 双16位过滤器 */
  CAN_FILTER_SCALE_32BIT = 1  /**< 单32位过滤器 */
} CAN_FilterScale_t;

/**
 * @brief CAN 错误代码枚举（LEC: Last Error Code）
 */
typedef enum {
  CAN_LEC_NO_ERROR = 0,      /**< 无错误 */
  CAN_LEC_STUFF_ERROR = 1,   /**< 填充错误 */
  CAN_LEC_FORM_ERROR = 2,    /**< 格式错误 */
  CAN_LEC_ACK_ERROR = 3,     /**< 应答错误 */
  CAN_LEC_BIT_RECESSIVE = 4, /**< 隐性位错误 */
  CAN_LEC_BIT_DOMINANT = 5,  /**< 显性位错误 */
  CAN_LEC_CRC_ERROR = 6      /**< CRC 错误 */
} CAN_LastErrorCode_t;

/* Exported constants --------------------------------------------------------*/

/** 最大过滤器数量（STM32F103） */
#define CAN_FILTER_COUNT 14

/** 发送邮箱总数 */
#define CAN_TX_MAILBOX_COUNT 3

/** CAN 初始化返回值定义 */
#define CAN_INIT_OK 0             /**< 初始化成功 */
#define CAN_INIT_ENTER_TIMEOUT -1 /**< 进入初始化模式超时 */
#define CAN_INIT_EXIT_TIMEOUT -2  /**< 退出初始化模式超时 */

/** CAN 发送返回值定义 */
#define CAN_TX_NO_MAILBOX -1 /**< 无空闲发送邮箱 */

/** CAN 等待发送返回值定义 */
#define CAN_TX_WAIT_OK 0       /**< 发送成功 */
#define CAN_TX_WAIT_ALST -1    /**< 仲裁丢失 */
#define CAN_TX_WAIT_TERR -2    /**< 发送错误 */
#define CAN_TX_WAIT_TIMEOUT -3 /**< 超时 */

/** CAN 接收返回值定义 */
#define CAN_RX_OK 0     /**< 接收成功 */
#define CAN_RX_EMPTY -1 /**< FIFO 为空 */

/** CAN 过滤器配置返回值定义 */
#define CAN_FILTER_OK 0           /**< 配置成功 */
#define CAN_FILTER_PARAM_ERROR -1 /**< 参数错误 */

/* TODO: 用户可修改配置
 * --------------------------------------------------------*/

/**
 * @brief 默认超时时间（用于轮询等待）
 * @note  用户可根据实际系统时钟调整此值
 */
#define CAN_TIMEOUT_VALUE 0x0000FFFFUL

/**
 * @brief APB1 时钟频率（用于波特率计算）
 * @note  用户需根据实际系统时钟配置修改此值
 *        典型值：36MHz（72MHz SYSCLK，APB1 2分频）
 */
#define CAN_APB1_CLK_HZ 36000000UL

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  初始化 CAN 外设
 * @param  baudrate: 波特率（如 500000 表示 500kbps）
 * @param  mode: 工作模式
 *         - CAN_MODE_NORMAL: 正常模式
 *         - CAN_MODE_LOOPBACK: 环回模式
 *         - CAN_MODE_SILENT: 静默模式
 *         - CAN_MODE_LOOPBACK_SILENT: 环回+静默模式
 * @retval CAN_INIT_OK: 成功
 *         CAN_INIT_ENTER_TIMEOUT: 进入初始化模式超时
 *         CAN_INIT_EXIT_TIMEOUT: 退出初始化模式超时
 */
int CAN_Init(uint32_t baudrate, CAN_Mode_t mode);

/**
 * @brief  发送 CAN 报文（轮询方式）
 * @param  id: 报文标识符
 *         - 标准帧：11位 ID (0x000 ~ 0x7FF)
 *         - 扩展帧：29位 ID (0x00000000 ~ 0x1FFFFFFF)
 * @param  ide: 帧类型
 *         - 0: 标准帧
 *         - 1: 扩展帧
 * @param  rtr: 远程帧标志
 *         - 0: 数据帧
 *         - 1: 远程帧
 * @param  data: 数据指针（数据帧时有效）
 * @param  len: 数据长度（0-8字节）
 * @retval 0-2: 使用的邮箱号
 *         CAN_TX_NO_MAILBOX: 无空闲邮箱
 */
int CAN_Transmit(uint32_t id, uint8_t ide, uint8_t rtr, uint8_t *data,
                 uint8_t len);

/**
 * @brief  等待发送完成
 * @param  mailbox: 邮箱号 (0-2)
 * @param  timeout: 超时计数值
 * @retval CAN_TX_WAIT_OK: 发送成功
 *         CAN_TX_WAIT_ALST: 仲裁丢失
 *         CAN_TX_WAIT_TERR: 发送错误
 *         CAN_TX_WAIT_TIMEOUT: 超时
 */
int CAN_TransmitWait(uint8_t mailbox, uint32_t timeout);

/**
 * @brief  接收 CAN 报文（轮询方式）
 * @param  fifo: FIFO 号 (0 或 1)
 * @param  id: [out] 接收到的报文标识符
 * @param  ide: [out] 帧类型（0=标准帧，1=扩展帧）
 * @param  rtr: [out] 远程帧标志（0=数据帧，1=远程帧）
 * @param  data: [out] 数据缓冲区（至少8字节）
 * @param  len: [out] 数据长度
 * @retval CAN_RX_OK: 接收成功
 *         CAN_RX_EMPTY: FIFO 为空
 */
int CAN_Receive(uint8_t fifo, uint32_t *id, uint8_t *ide, uint8_t *rtr,
                uint8_t *data, uint8_t *len);

/**
 * @brief  配置 CAN 过滤器
 * @param  filter_num: 过滤器编号 (0-13)
 * @param  mode: 过滤器模式
 *         - CAN_FILTER_MODE_MASK: 掩码模式
 *         - CAN_FILTER_MODE_LIST: 列表模式
 * @param  scale: 过滤器位宽
 *         - CAN_FILTER_SCALE_16BIT: 双16位
 *         - CAN_FILTER_SCALE_32BIT: 单32位
 * @param  fifo: 分配的 FIFO (0 或 1)
 * @param  id: ID 值（掩码模式）或 ID1（列表模式）
 * @param  mask: 掩码值（掩码模式）或 ID2（列表模式）
 * @retval CAN_FILTER_OK: 配置成功
 *         CAN_FILTER_PARAM_ERROR: 参数错误
 */
int CAN_FilterConfig(uint8_t filter_num, CAN_FilterMode_t mode,
                     CAN_FilterScale_t scale, uint8_t fifo, uint32_t id,
                     uint32_t mask);

/**
 * @brief  获取 CAN 错误状态
 * @param  tec: [out] 发送错误计数器值（可为 NULL）
 * @param  rec: [out] 接收错误计数器值（可为 NULL）
 * @param  lec: [out] 最后错误码（可为 NULL）
 * @retval 错误标志组合：
 *         - bit0: EWGF (错误警告标志，TEC/REC >= 96)
 *         - bit1: EPVF (错误被动标志，TEC/REC > 127)
 *         - bit2: BOFF (离线标志，TEC > 255)
 */
uint8_t CAN_GetError(uint8_t *tec, uint8_t *rec, uint8_t *lec);

/**
 * @brief  获取 FIFO 中待处理消息数量
 * @param  fifo: FIFO 号 (0 或 1)
 * @retval 待处理消息数量 (0-3)
 */
uint8_t CAN_GetPendingMessages(uint8_t fifo);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_DRIVER_H */
