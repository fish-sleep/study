#ifndef __KEY_H
#define __KEY_H

#include "main.h"

/* 四个按键引脚（已在 MX_GPIO_Init 中配置为上拉输入，按下为低电平） */
#define KEY1_GPIO_PORT   GPIOB
#define KEY1_GPIO_PIN    GPIO_PIN_10
#define KEY2_GPIO_PORT   GPIOB
#define KEY2_GPIO_PIN    GPIO_PIN_11
#define KEY3_GPIO_PORT   GPIOB
#define KEY3_GPIO_PIN    GPIO_PIN_3
#define KEY4_GPIO_PORT   GPIOA
#define KEY4_GPIO_PIN    GPIO_PIN_8

/* 按下时的电平（上拉输入，按下为低） */
#define KEY_PRESSED_LEVEL   GPIO_PIN_RESET

/* 按键编号 */
#define KEY1    1
#define KEY2    2
#define KEY3    3
#define KEY4    4

void Key_Init(void);              /* 初始化：记录初始状态 */
uint8_t Key_Read(uint8_t key);    /* 读取指定按键当前电平：0=按下，1=松开 */
uint8_t Key_IsPressed(uint8_t key); /* 检测指定按键是否刚被按下（下降沿），是返回 1 */
uint8_t Key_Scan(void);           /* 扫描：返回刚被按下的按键编号 1~4，无按键返回 0 */

#endif
