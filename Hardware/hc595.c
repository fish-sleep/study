#include "hc595.h"

/* 当前锁存输出的数据（bit0~bit7 对应 Q0~Q7），供状态查询 */
static uint8_t hc595_data = 0;

/**
  * @brief  写入一个字节并锁存输出
  * @param  data: 要输出的 8 位数据，bit=1 点亮对应输出
  * @note   MSB 先送，即第一个移入的位最终出现在 Q7 上；
  *         若流水方向反了，可改成 LSB 先送。
  */
void HC595_WriteByte(uint8_t data)
{
    hc595_data = data;   /* 记录当前输出 */

    for (uint8_t i = 0; i < 8; i++)
    {
        /* 先把最高位送到 DS 数据线 */
        HAL_GPIO_WritePin(HC595_DS_PORT, HC595_DS_PIN,
                          (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        /* SHCP 产生一个上升沿，把 DS 移入一位 */
        HAL_GPIO_WritePin(HC595_SHCP_PORT, HC595_SHCP_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(HC595_SHCP_PORT, HC595_SHCP_PIN, GPIO_PIN_RESET);

        data <<= 1;
    }

    /* STCP 产生一个上升沿，把移位寄存器内容锁存到 Q0~Q7 输出 */
    HAL_GPIO_WritePin(HC595_STCP_PORT, HC595_STCP_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HC595_STCP_PORT, HC595_STCP_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  返回当前锁存输出的数据（bit0~bit7 对应 Q0~Q7）
  */
uint8_t HC595_GetData(void)
{
    return hc595_data;
}
