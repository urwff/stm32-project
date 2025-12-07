/*
 * @Author: wushengran
 * @Date: 2024-05-27 14:19:39
 * @Description: KEY Driver Implementation
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "key.h"
#include "led.h"

/**
 * @brief 按键初始化
 *        KEY -> PF10
 *        HAL模式: 配置为中断上升沿模式
 *        寄存器模式: 配置为中断模式
 */
void Key_Init(void) {
#if KEY_USE_HAL
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  // PF 端口时钟
  if (KEY_GPIO_PORT == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (KEY_GPIO_PORT == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (KEY_GPIO_PORT == GPIOC)
    __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (KEY_GPIO_PORT == GPIOD)
    __HAL_RCC_GPIOD_CLK_ENABLE();
  else if (KEY_GPIO_PORT == GPIOE)
    __HAL_RCC_GPIOE_CLK_ENABLE();
  else if (KEY_GPIO_PORT == GPIOF)
    __HAL_RCC_GPIOF_CLK_ENABLE();

  /* Configure GPIO pin */
  GPIO_InitStruct.Pin = KEY_PIN;
  // 假设按下为高电平，配置为上升沿中断 + 下拉
  // 如果实际是按下接地，请改为 GPIO_MODE_IT_FALLING + GPIO_PULLUP
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(KEY_GPIO_PORT, &GPIO_InitStruct);

  /* EXTI interrupt init */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0); // 优先级可根据需要调整
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

#else
  // 寄存器初始化 (STM32F103)
  // 1. 配置时钟 (GPIOF + AFIO)
  RCC->APB2ENR |= RCC_APB2ENR_IOPFEN | RCC_APB2ENR_AFIOEN;

  // 2. GPIO工作模式 configuration PF10 -> Input
  // CRH [11:8] corresponds to Pin 10
  // Input Mode (00) + Floating (01) or Pull-up/down (10)
  // 假设用 Pull-up/down (10) -> Mode=00, CNF=10 -> 0x8
  KEY_GPIO_PORT->CRH &= ~(0xF << 8); // Clear
  KEY_GPIO_PORT->CRH |= (0x8 << 8);  // Input with pull-up/down

  // 下拉 (ODR=0) or 上拉 (ODR=1)
  // 假设下拉
  KEY_GPIO_PORT->ODR &= ~KEY_PIN;

  // 3. AFIO EXTI Configuration
  // EXTI10 maps to PF10. EXTICR[2] controls EXTI8-11.
  // EXTI10 is in EXTICR[2] bits [11:8]
  // Value for Port F is usually 0x05 (A=0, B=1, ..., F=5)
  AFIO->EXTICR[2] &= ~(0xF << 8);
  AFIO->EXTICR[2] |= (0x5 << 8);

  // 4. EXTI Configuration
  EXTI->IMR |= (1 << 10);   // Unmask Line 10
  EXTI->RTSR |= (1 << 10);  // Rising Trigger
  EXTI->FTSR &= ~(1 << 10); // Clear Falling Trigger

  // 5. NVIC Configuration
  NVIC_SetPriority(EXTI15_10_IRQn, 2);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
#endif
}

/**
 * @brief 获取按键状态
 * @return 1:按下, 0:松开 (假设高有效)
 */
uint8_t Key_GetState(void) {
#if KEY_USE_HAL
  return (HAL_GPIO_ReadPin(KEY_GPIO_PORT, KEY_PIN) == GPIO_PIN_SET) ? 1 : 0;
#else
  return ((KEY_GPIO_PORT->IDR & KEY_PIN) != 0) ? 1 : 0; // Mask check
#endif
}

// =================== 中断处理 ===================

/**
 * @brief 按键按下处理函数(核心业务逻辑)
 */
static void Key_OnPress(void) { LED_Toggle(LED1); }

#if KEY_USE_HAL
/**
 * @brief HAL EXTI Callback
 *        使用 weak 定义，允许用户在 main.c 中覆盖
 */
__weak void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == KEY_PIN) {
    Key_OnPress();
  }
}
#else
/**
 * @brief EXTI15_10 中断服务函数 (Register Mode)
 */
void EXTI15_10_IRQHandler(void) {
  // 判断是否是 EXTI10 (PF10)
  if (EXTI->PR & (1 << 10)) {
    // 简单的去抖 (可选，这里仅做状态判断)
    // 再次确认电平状态
    if ((KEY_GPIO_PORT->IDR & KEY_PIN) != 0) {
      Key_OnPress();
    }

    // 清除中断标志
    EXTI->PR |= (1 << 10);
  }
}
#endif
