#include "buzzer.h"

/**
  * @brief  初始化蜂鸣器（关闭）
  */
void Buzzer_Init(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  蜂鸣器响（输出高电平）
  */
void Buzzer_On(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, GPIO_PIN_SET);
}

/**
  * @brief  蜂鸣器停（输出低电平）
  */
void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  翻转蜂鸣器状态
  */
void Buzzer_Toggle(void)
{
    HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
}
