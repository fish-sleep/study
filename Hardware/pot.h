#ifndef __POT_H
#define __POT_H

#include "main.h"
#include "adc.h"

/* 滑动变阻器：PA0 -> ADC1 通道 0，连续转换模式（已在 CubeMX 中配置） */
#define POT_ADC_HANDLE    (&hadc1)

void Pot_Init(void);           /* 启动 ADC 连续转换 */
uint16_t Pot_GetValue(void);   /* 读取 ADC 原始值 0~4095 */

#endif
