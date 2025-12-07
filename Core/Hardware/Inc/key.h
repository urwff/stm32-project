#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(USE_HAL_DRIVER) || defined(STM32F1xx_HAL_H)
#include "stm32f1xx_hal.h"
#define KEY_USE_HAL 1
#else
#include "stm32f10x.h"
#define KEY_USE_HAL 0
#endif

#include "led.h"

// 定义 KEY 引脚 (PF10)
#define KEY_GPIO_PORT GPIOF

#if KEY_USE_HAL
#define KEY_PIN GPIO_PIN_10
#else
#define KEY_PIN GPIO_IDR_IDR10
#endif

// 初始化
void Key_Init(void);

// 获取按键状态 (1=按下, 0=释放, 假设低有效则反之，需根据电路确认)
// 原代码: (GPIOF->IDR & GPIO_IDR_IDR10) != 0 可能意味着高电平有效？
// 通常按键接地是低有效。原代码判断 != 0 执行 LED_Toggle，意味着高电平触发？
// 待会在implementation里处理。
uint8_t Key_GetState(void);

#ifdef __cplusplus
}
#endif

#endif
