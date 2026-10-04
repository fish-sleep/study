#ifndef __LED_H
#define __LED_H

#include "main.h"

/* PC13 指示灯引脚（已在 MX_GPIO_Init 中配置为推挽输出） */
#define LED_GPIO_PORT    GPIOC
#define LED_GPIO_PIN     GPIO_PIN_13

/* 点亮/熄灭电平：若板上 LED 为低电平点亮，请把 ON/OFF 两个宏互换 */
#define LED_ON_LEVEL     GPIO_PIN_SET
#define LED_OFF_LEVEL    GPIO_PIN_RESET

void LED_Init(void);     /* 初始化（熄灭 LED） */
void LED_On(void);       /* 点亮 */
void LED_Off(void);      /* 熄灭 */
void LED_Toggle(void);   /* 翻转 */

#endif
