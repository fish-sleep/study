#include "aht30.h"

/* AHT30 器件地址：7 位地址 0x38，HAL 库要求左移 1 位 */
#define AHT30_I2C_ADDR      (0x38 << 1)

/* 初始化（校准）命令：0xE1 0x08 0x00（AHT30） */
#define AHT30_CMD_INIT_0    0xE1
#define AHT30_CMD_INIT_1    0x08
#define AHT30_CMD_INIT_2    0x00

/* 兼容 AHT20 芯片的初始化命令：0xBE 0x08 0x00 */
#define AHT30_CMD_ALT_INIT_0    0xBE
#define AHT30_CMD_ALT_INIT_1    0x08
#define AHT30_CMD_ALT_INIT_2    0x00

/* 触发测量命令：0xAC 0x33 0x00 */
#define AHT30_CMD_MEAS_0    0xAC
#define AHT30_CMD_MEAS_1    0x33
#define AHT30_CMD_MEAS_2    0x00

/* 状态字节标志位 */
#define AHT30_STATUS_BUSY   0x80   /* 忙标志，1=测量中 */
#define AHT30_STATUS_CAL    0x08   /* 校准使能标志，1=已校准 */

/* 分辨率 2^20 = 1048576 */
#define AHT30_RESOLUTION    1048576.0f

/* 软复位命令 */
#define AHT30_CMD_RESET     0xBA

/**
  * @brief  向 AHT30 写 3 字节命令
  * @retval 0=成功，1=失败
  */
static uint8_t AHT30_WriteCmd(uint8_t c0, uint8_t c1, uint8_t c2)
{
    uint8_t cmd[3] = {c0, c1, c2};
    return (HAL_I2C_Master_Transmit(&hi2c1, AHT30_I2C_ADDR, cmd, 3, 100) == HAL_OK) ? 0 : 1;
}

/**
  * @brief  发送 AHT30 软复位命令
  * @retval 0=成功，1=失败
  */
static uint8_t AHT30_SoftReset(void)
{
    uint8_t cmd = AHT30_CMD_RESET;
    return (HAL_I2C_Master_Transmit(&hi2c1, AHT30_I2C_ADDR, &cmd, 1, 100) == HAL_OK) ? 0 : 1;
}

/**
  * @brief  AHT30 初始化：上电稳定 → 软复位 → 发送校准命令 → 实测验证
  * @note   部分芯片（AHT20 兼容）对 0xE1 不应答，失败后尝试 0xBE；
  *         最终以“能否成功读取一次数据”作为初始化是否成功的标准。
  * @retval 0=成功，1=失败
  */
uint8_t AHT30_Init(void)
{
    AHT30_Data_t tmp;

    HAL_Delay(100);   /* 上电稳定 */

    AHT30_SoftReset();              /* 软复位（结果忽略） */
    HAL_Delay(20);

    /* 发送初始化/校准命令：AHT30 用 0xE1，AHT20 兼容芯片用 0xBE */
    if (AHT30_WriteCmd(AHT30_CMD_INIT_0, AHT30_CMD_INIT_1, AHT30_CMD_INIT_2) != 0)
    {
        AHT30_WriteCmd(AHT30_CMD_ALT_INIT_0, AHT30_CMD_ALT_INIT_1, AHT30_CMD_ALT_INIT_2);
    }
    HAL_Delay(100);

    /* 通过实际读取一次验证传感器是否可用 */
    return AHT30_ReadData(&tmp);
}

/**
  * @brief  读取 AHT30 温湿度
  * @param  data: 温湿度结构体指针
  * @retval 0=成功，1=失败
  */
uint8_t AHT30_ReadData(AHT30_Data_t *data)
{
    uint8_t buf[6] = {0};
    uint32_t raw_h;
    uint32_t raw_t;
    uint8_t retry;

    if (data == NULL)
    {
        return 1;
    }

    /* 触发一次测量 */
    if (AHT30_WriteCmd(AHT30_CMD_MEAS_0, AHT30_CMD_MEAS_1, AHT30_CMD_MEAS_2) != 0)
    {
        return 1;
    }

    /* 等待测量完成并读取，最多重试 10 次（每次约 80ms） */
    for (retry = 0; retry < 10; retry++)
    {
        HAL_Delay(80);

        if (HAL_I2C_Master_Receive(&hi2c1, AHT30_I2C_ADDR, buf, 6, 100) != HAL_OK)
        {
            continue;   /* 读失败，重试 */
        }

        if (!(buf[0] & AHT30_STATUS_BUSY))
        {
            break;      /* 测量完成 */
        }
    }

    /* 忙标志未清零，数据无效 */
    if (buf[0] & AHT30_STATUS_BUSY)
    {
        return 1;
    }

    /* 湿度原始值：[1]<<12 | [2]<<4 | [3]>>4 */
    raw_h = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | ((uint32_t)buf[3] >> 4);

    /* 温度原始值：([3]&0x0F)<<16 | [4]<<8 | [5] */
    raw_t = (((uint32_t)buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | (uint32_t)buf[5];

    /* RH% = raw_h / 2^20 * 100 */
    data->humidity = (float)raw_h * 100.0f / AHT30_RESOLUTION;

    /* T℃ = raw_t / 2^20 * 200 - 50 */
    data->temperature = (float)raw_t * 200.0f / AHT30_RESOLUTION - 50.0f;

    return 0;
}
