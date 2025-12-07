#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

/* 根据环境自动选择头文件，支持从 main.h 传入宏 */
#if defined(USE_HAL_DRIVER) || defined(STM32F1xx_HAL_H)
#include "stm32f1xx_hal.h"
#define LED_USE_HAL 1
#else
#include "stm32f10x.h"
#define LED_USE_HAL 0
#endif

// 定义 LED 引脚和端口
// 假设 LED1-PA0, LED2-PA1, LED3-PA8 (根据原有 ODR0/1/8 推断)
// 如果实际电路不同，请修改此处
#define LED_GPIO_PORT GPIOA

#if LED_USE_HAL
#define LED1_PIN GPIO_PIN_0
#define LED2_PIN GPIO_PIN_1
#define LED3_PIN GPIO_PIN_8

// 为了兼容旧接口，定义 LED1/2/3 为 PIN 值
#define LED1 LED1_PIN
#define LED2 LED2_PIN
#define LED3 LED3_PIN
#else
// 寄存器模式下，LED1/2/3 对应 ODR 寄存器的位
// 为了方便位操作，这里定义为位掩码
#define LED1 GPIO_ODR_ODR0
#define LED2 GPIO_ODR_ODR1
#define LED3 GPIO_ODR_ODR8
#endif

// 初始化
void LED_Init(void);

// 控制单个LED (参数为 LED1/LED2/LED3 宏)
void LED_On(uint16_t led);
void LED_Off(uint16_t led);
void LED_Toggle(uint16_t led);

// 批量控制
void LED_OnAll(uint16_t leds[], uint8_t size);
void LED_OffAll(uint16_t leds[], uint8_t size);

#ifdef __cplusplus
}
#endif

#endif
