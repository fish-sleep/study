#ifndef __BMI160_H
#define __BMI160_H

#include "main.h"
#include "spi.h"

/* 六轴数据结构 */
typedef struct
{
    float acc_x, acc_y, acc_z;   /* 加速度，单位 g */
    float gyr_x, gyr_y, gyr_z;   /* 角速度，单位 °/s (dps) */
} BMI160_Data_t;

uint8_t BMI160_Init(void);                     /* 初始化（软复位+配置），成功返回 0 */
uint8_t BMI160_ReadID(uint8_t *id);            /* 读取芯片 ID，应为 0xD1 */
uint8_t BMI160_ReadData(BMI160_Data_t *data);  /* 读取六轴数据，成功返回 0 */

#endif
