/*
 * @Author: wushengran
 * @Date: 2024-05-24 15:39:25
 * @Description: LED Driver Implementation
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "led.h"

/**
 * @brief LED初始化
 *        LED1 -> PA0
 *        LED2 -> PA1
 *        LED3 -> PA8
 */
void LED_Init(void) {
#if LED_USE_HAL
  // HAL库初始化
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  if (LED_GPIO_PORT == GPIOA)
    __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (LED_GPIO_PORT == GPIOB)
    __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (LED_GPIO_PORT == GPIOC)
    __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure GPIO pin Output Level */
  // 默认输出高电平(假设低电平点亮，或者根据之前逻辑)
  // 根据原名为 LED_Off，通常意味着熄灭。
  HAL_GPIO_WritePin(LED_GPIO_PORT, LED1_PIN | LED2_PIN | LED3_PIN,
                    GPIO_PIN_SET);

  /* Configure GPIO pins */
  GPIO_InitStruct.Pin = LED1_PIN | LED2_PIN | LED3_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

#else
  // 寄存器初始化 (STM32F103)
  // 1. 开启端口时钟 (GPIOA)
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

  // 2. 配置模式
  // PA0, PA1 -> CRL
  // 清除原来的设置 (每4位)
  LED_GPIO_PORT->CRL &= ~(0xF << 0); // PA0
  LED_GPIO_PORT->CRL &= ~(0xF << 4); // PA1
  // 设置为推挽输出 2MHz (Mode=10, CNF=00 -> 0010 = 0x2) 或者 50MHz (0011=0x3)
  // 这里选用通用推挽输出 50MHz (0x3)
  LED_GPIO_PORT->CRL |= (0x3 << 0);
  LED_GPIO_PORT->CRL |= (0x3 << 4);

  // PA8 -> CRH
  LED_GPIO_PORT->CRH &= ~(0xF << 0); // PA8
  LED_GPIO_PORT->CRH |= (0x3 << 0);

  // 3. 初始状态 (全灭)
  // 假设高电平灭(ODR=1)
  LED_GPIO_PORT->ODR |= (LED1 | LED2 | LED3);
#endif
}

void LED_On(uint16_t led) {
#if LED_USE_HAL
  // HAL库模式: Reset = Low = 点亮 (假设低有效)
  HAL_GPIO_WritePin(LED_GPIO_PORT, led, GPIO_PIN_RESET);
#else
  // 寄存器模式: BRR (Bit Reset Register) 用于清除位 (输出低)
  LED_GPIO_PORT->BRR = led;
#endif
}

void LED_Off(uint16_t led) {
#if LED_USE_HAL
  // HAL库模式: Set = High = 熄灭
  HAL_GPIO_WritePin(LED_GPIO_PORT, led, GPIO_PIN_SET);
#else
  // 寄存器模式: BSRR (Bit Set Reset Register) 低16位用于设置位 (输出高)
  LED_GPIO_PORT->BSRR = led;
#endif
}

void LED_Toggle(uint16_t led) {
#if LED_USE_HAL
  HAL_GPIO_TogglePin(LED_GPIO_PORT, led);
#else
  // 寄存器模式: ODR 异或
  LED_GPIO_PORT->ODR ^= led;
#endif
}

void LED_OnAll(uint16_t leds[], uint8_t size) {
  for (int i = 0; i < size; i++) {
    LED_On(leds[i]);
  }
}

void LED_OffAll(uint16_t leds[], uint8_t size) {
  for (int i = 0; i < size; i++) {
    LED_Off(leds[i]);
  }
}
