#ifndef __AHT30_H
#define __AHT30_H

#include "main.h"
#include "i2c.h"

/* 温湿度数据结构 */
typedef struct
{
    float temperature;   /* 温度，单位 ℃ */
    float humidity;      /* 相对湿度，单位 %RH */
} AHT30_Data_t;

uint8_t AHT30_Init(void);                    /* 初始化（发送校准命令），成功返回 0 */
uint8_t AHT30_ReadData(AHT30_Data_t *data);  /* 读取温湿度，成功返回 0 */

#endif
