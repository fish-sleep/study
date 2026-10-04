#include "servo.h"

/* 50Hz 舵机 PWM 参数（TIM3: Prescaler=71 => 1MHz，Period=19999 => 20ms 周期）：
   脉宽 0.5ms(500) 对应 0°，脉宽 2.5ms(2500) 对应 180° */
#define SERVO_PULSE_MIN   500    /* 0°   脉宽 */
#define SERVO_PULSE_MAX   2500   /* 180° 脉宽 */

/**
  * @brief  初始化舵机：启动 TIM3 两个 PWM 通道，并把舵机转到中间位置(90°)
  */
void Servo_Init(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    Servo_SetAngle(SERVO_X, 0.0f);
    Servo_SetAngle(SERVO_Y, 90.0f);
}

/**
  * @brief  把角度(0~180°)转换成 PWM 脉宽并输出到对应舵机
  * @param  servo 舵机选择：SERVO_X(PB5) 或 SERVO_Y(PB4)
  * @param  angle 目标角度，范围 0~180°，超出范围自动截断
  */
void Servo_SetAngle(uint8_t servo, float angle)
{
    uint32_t pulse;

    /* 角度限幅 */
    if (angle < SERVO_ANGLE_MIN)
    {
        angle = SERVO_ANGLE_MIN;
    }
    if (angle > SERVO_ANGLE_MAX)
    {
        angle = SERVO_ANGLE_MAX;
    }

    /* 线性映射：0°->500, 180°->2500 */
    pulse = SERVO_PULSE_MIN
            + (uint32_t)((angle / SERVO_ANGLE_MAX) * (SERVO_PULSE_MAX - SERVO_PULSE_MIN));

    if (servo == SERVO_Y)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
    }
}
