#include "led.h"

/**
  * @brief  初始化 LED（引脚已在 MX_GPIO_Init 中配置，这里保证初始为熄灭）
  */
void LED_Init(void)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, LED_OFF_LEVEL);
}

/**
  * @brief  点亮 LED
  */
void LED_On(void)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, LED_ON_LEVEL);
}

/**
  * @brief  熄灭 LED
  */
void LED_Off(void)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, LED_OFF_LEVEL);
}

/**
  * @brief  翻转 LED 状态
  */
void LED_Toggle(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
}
