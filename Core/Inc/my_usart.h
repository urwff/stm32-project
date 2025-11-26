/*头文件定义*/
#ifndef __MY_USART_H__
#define __MY_USART_H__
#include "stdio.h"

#include "stm32f1xx.h"
#include <sys/_intsup.h>

void Driver_USART1_Init(void);

void Driver_USART1_SendChar(char byte);

void Driver_USART1_SendString(uint8_t *str, uint16_t len);

uint8_t Driver_USART1_ReceiveChar(void);

void Driver_USART1_ReceiveString(uint8_t buff[], uint8_t *len);

#endif