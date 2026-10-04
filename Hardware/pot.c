#include "pot.h"

/**
  * @brief  启动 ADC1 连续转换（PA0 滑动变阻器）
  */
void Pot_Init(void)
{
    HAL_ADC_Start(POT_ADC_HANDLE);
}

/**
  * @brief  读取 ADC 原始值（0~4095）
  */
uint16_t Pot_GetValue(void)
{
    return (uint16_t)HAL_ADC_GetValue(POT_ADC_HANDLE);
}
