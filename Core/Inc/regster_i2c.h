#ifndef __DRIVER_I2C2_H
#define __DRIVER_I2C2_H


#include "stm32f1xx.h"
#include "usart.h"

#define ACK 0
#define NACK 1

#define OK 1
#define FAIL 0

void Driver_I2C2_Init(void);

uint8_t Driver_I2C2_Start(void);

void Driver_I2C2_Stop(void);

void Driver_I2C2_ACK(void);

void Driver_I2C2_NACK(void);

uint8_t Driver_I2C_SendAddr(uint8_t addr);

uint8_t Driver_I2C_SendByte(uint8_t byte);

uint8_t Driver_I2C_ReadByte(void);

#endif