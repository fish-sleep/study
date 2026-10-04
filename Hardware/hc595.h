#ifndef __HC595_H
#define __HC595_H

#include "main.h"

/* 74HC595 引脚定义（不级联，单颗芯片） */
#define HC595_DS_PORT      GPIOA   /* DS   串行数据输入  PA7 */
#define HC595_DS_PIN       GPIO_PIN_7
#define HC595_STCP_PORT    GPIOB   /* STCP 锁存时钟     PB0 */
#define HC595_STCP_PIN     GPIO_PIN_0
#define HC595_SHCP_PORT    GPIOB   /* SHCP 移位时钟     PB1 */
#define HC595_SHCP_PIN     GPIO_PIN_1

void HC595_WriteByte(uint8_t data); /* 写入一个字节并锁存输出（MSB 先送） */
uint8_t HC595_GetData(void);        /* 返回当前锁存输出的数据（bit0~bit7 对应 Q0~Q7） */

#endif
