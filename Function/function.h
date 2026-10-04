#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "main.h"
#include "led.h"
#include "key.h"
#include "buzzer.h"
#include "hc595.h"
#include "OLED.h"
#include "servo.h"
#include "message.h"
#include "aht30.h"
#include "bmi160.h"
#include "pot.h"

void Function_Menu_Handle(void);     /* 菜单主循环，需在 while(1) 中周期调用 */

void Function_LED_Handle(void);      /* PB10(KEY1) 按下翻转 LED 亮灭 */
void Function_Buzzer_Handle(void);   /* PB11(KEY2) 按下翻转蜂鸣器状态 */
void Function_HC595_Handle(void);    /* PB3(KEY3) 步进点灯 */
void Function_Servo_Rotate(uint8_t servo, float angle); /* 输入角度，舵机转动到对应角度（servo: SERVO_X/SERVO_Y） */

#endif
