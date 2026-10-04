#ifndef __SERVO_H
#define __SERVO_H

#include "main.h"
#include "tim.h"

/* 舵机定义（TIM3 已配置为 50Hz PWM）：
   X 轴舵机 -> PB5 -> TIM3_CH2
   Y 轴舵机 -> PB4 -> TIM3_CH1 */
#define SERVO_X   2   /* PB5 */
#define SERVO_Y   1   /* PB4 */

/* 舵机角度范围 */
#define SERVO_ANGLE_MIN   0.0f
#define SERVO_ANGLE_MAX   180.0f

void Servo_Init(void);                            /* 启动 PWM 并把两个舵机转到初始位置 */
void Servo_SetAngle(uint8_t servo, float angle);  /* 把指定舵机(SERVO_X/SERVO_Y)转到对应角度(0~180°) */

#endif
