#include "bmi160.h"

/* 软件片选引脚：PB12（SPI2 硬件 NSS 引脚，当前空闲） */
#define BMI160_CS_PORT   GPIOB
#define BMI160_CS_PIN    GPIO_PIN_12

/* 寄存器地址 */
#define BMI160_REG_CHIP_ID   0x00
#define BMI160_REG_CMD       0x7E
#define BMI160_REG_ACC_RANGE 0x40
#define BMI160_REG_ACC_CONF  0x41
#define BMI160_REG_GYR_RANGE 0x42
#define BMI160_REG_GYR_CONF  0x43
#define BMI160_REG_DATA      0x0C   /* GYR_X_LSB 起连续 12 字节 */

/* 命令 */
#define BMI160_CMD_SOFT_RESET  0xB6
#define BMI160_CMD_ACC_NORMAL  0x11
#define BMI160_CMD_GYR_NORMAL  0x15

/* 量程与输出数据率配置：加速度 ±8g、陀螺仪 ±1000dps、100Hz */
#define BMI160_ACC_RANGE_8G    0x08
#define BMI160_ACC_CONF_100HZ  0x28
#define BMI160_GYR_RANGE_1000  0x01
#define BMI160_GYR_CONF_100HZ  0x28

#define BMI160_CS_LOW()   HAL_GPIO_WritePin(BMI160_CS_PORT, BMI160_CS_PIN, GPIO_PIN_RESET)
#define BMI160_CS_HIGH()  HAL_GPIO_WritePin(BMI160_CS_PORT, BMI160_CS_PIN, GPIO_PIN_SET)

/**
  * @brief  初始化软件片选引脚
  */
static void BMI160_CS_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpio.Pin = BMI160_CS_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BMI160_CS_PORT, &gpio);
}

/**
  * @brief  写单个寄存器
  * @note   BMI160 SPI 协议：第一个字节 bit7=0 表示写，bit6~bit0 为寄存器地址
  */
static void BMI160_WriteReg(uint8_t reg, uint8_t val)
{
    uint8_t tx[2];

    tx[0] = reg & 0x7F;          /* bit7=0 写 */
    tx[1] = val;

    BMI160_CS_LOW();
    HAL_SPI_Transmit(&hspi2, tx, 2, 100);
    BMI160_CS_HIGH();
}

/**
  * @brief  从指定寄存器连续读取 len 字节（地址自动递增）
  * @note   BMI160 SPI 协议：第一个字节 bit7=1 表示读，bit6~bit0 为寄存器地址
  */
static void BMI160_ReadRegs(uint8_t reg, uint8_t *rx, uint8_t len)
{
    uint8_t tx[16] = {0};
    uint8_t tmp[16] = {0};
    uint8_t i;

    tx[0] = reg | 0x80;          /* bit7=1 读 */

    BMI160_CS_LOW();
    HAL_SPI_TransmitReceive(&hspi2, tx, tmp, (uint16_t)(len + 1), 100);
    BMI160_CS_HIGH();

    /* tmp[0] 是地址阶段的无用字节，tmp[1..len] 才是数据 */
    for (i = 0; i < len; i++)
    {
        rx[i] = tmp[i + 1];
    }
}

/**
  * @brief  BMI160 初始化：软复位 → 校验 ID → 配置 PMU/量程/数据率
  * @retval 0=成功，1=失败
  */
uint8_t BMI160_Init(void)
{
    uint8_t id = 0;

    BMI160_CS_Init();
    BMI160_CS_HIGH();

    /* 软复位 */
    BMI160_WriteReg(BMI160_REG_CMD, BMI160_CMD_SOFT_RESET);
    HAL_Delay(100);

    /* 校验芯片 ID，应为 0xD1 */
    BMI160_ReadRegs(BMI160_REG_CHIP_ID, &id, 1);
    if (id != 0xD1)
    {
        return 1;
    }

    /* 加速度计、陀螺仪进入 normal 模式 */
    BMI160_WriteReg(BMI160_REG_CMD, BMI160_CMD_ACC_NORMAL);
    HAL_Delay(10);
    BMI160_WriteReg(BMI160_REG_CMD, BMI160_CMD_GYR_NORMAL);
    HAL_Delay(10);

    /* 配置量程和输出数据率 */
    BMI160_WriteReg(BMI160_REG_ACC_RANGE, BMI160_ACC_RANGE_8G);
    BMI160_WriteReg(BMI160_REG_ACC_CONF,  BMI160_ACC_CONF_100HZ);
    BMI160_WriteReg(BMI160_REG_GYR_RANGE, BMI160_GYR_RANGE_1000);
    BMI160_WriteReg(BMI160_REG_GYR_CONF,  BMI160_GYR_CONF_100HZ);

    return 0;
}

/**
  * @brief  读取芯片 ID（正常应为 0xD1）
  */
uint8_t BMI160_ReadID(uint8_t *id)
{
    if (id == NULL)
    {
        return 1;
    }

    BMI160_ReadRegs(BMI160_REG_CHIP_ID, id, 1);
    return 0;
}

/**
  * @brief  读取六轴数据
  * @retval 0=成功，1=参数错误
  */
uint8_t BMI160_ReadData(BMI160_Data_t *data)
{
    uint8_t buf[12];
    int16_t ax, ay, az, gx, gy, gz;

    if (data == NULL)
    {
        return 1;
    }

    /* 从 0x0C 连续读 12 字节：陀螺仪 6 + 加速度 6（小端） */
    BMI160_ReadRegs(BMI160_REG_DATA, buf, 12);

    gx = (int16_t)((buf[1] << 8) | buf[0]);
    gy = (int16_t)((buf[3] << 8) | buf[2]);
    gz = (int16_t)((buf[5] << 8) | buf[4]);
    ax = (int16_t)((buf[7] << 8) | buf[6]);
    ay = (int16_t)((buf[9] << 8) | buf[8]);
    az = (int16_t)((buf[11] << 8) | buf[10]);

    /* 加速度 ±8g：raw / 32768 * 16 */
    data->acc_x = (float)ax * 16.0f / 32768.0f;
    data->acc_y = (float)ay * 16.0f / 32768.0f;
    data->acc_z = (float)az * 16.0f / 32768.0f;

    /* 陀螺仪 ±1000dps：raw / 32768 * 2000 */
    data->gyr_x = (float)gx * 2000.0f / 32768.0f;
    data->gyr_y = (float)gy * 2000.0f / 32768.0f;
    data->gyr_z = (float)gz * 2000.0f / 32768.0f;

    return 0;
}
