#include "key.h"

/* 上一次的按键电平（0=按下，1=松开） */
static uint8_t key_last_state[4] = {GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET};

/**
  * @brief  读取指定按键的当前电平
  * @param  key: 按键编号 KEY1~KEY4
  * @retval 0=按下，1=松开
  */
uint8_t Key_Read(uint8_t key)
{
    uint8_t state = GPIO_PIN_SET;

    switch (key)
    {
        case KEY1:
            state = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_GPIO_PIN);
            break;
        case KEY2:
            state = HAL_GPIO_ReadPin(KEY2_GPIO_PORT, KEY2_GPIO_PIN);
            break;
        case KEY3:
            state = HAL_GPIO_ReadPin(KEY3_GPIO_PORT, KEY3_GPIO_PIN);
            break;
        case KEY4:
            state = HAL_GPIO_ReadPin(KEY4_GPIO_PORT, KEY4_GPIO_PIN);
            break;
        default:
            break;
    }

    return state;
}

/**
  * @brief  初始化按键，记录初始状态
  */
void Key_Init(void)
{
    key_last_state[0] = Key_Read(KEY1);
    key_last_state[1] = Key_Read(KEY2);
    key_last_state[2] = Key_Read(KEY3);
    key_last_state[3] = Key_Read(KEY4);
}

/**
  * @brief  检测指定按键是否刚被按下（“上一次松开、本次按下”的下降沿）
  * @param  key: 按键编号 KEY1~KEY4
  * @retval 1=刚被按下，0=未按下
  */
uint8_t Key_IsPressed(uint8_t key)
{
    uint8_t current;
    uint8_t idx;

    if ((key < KEY1) || (key > KEY4))
    {
        return 0;
    }

    idx = key - 1;
    current = Key_Read(key);

    /* 上一次为松开(高)、本次为按下(低)：检测到一次按下 */
    if ((key_last_state[idx] == GPIO_PIN_SET) && (current == KEY_PRESSED_LEVEL))
    {
        key_last_state[idx] = current;   /* 更新上一次状态 */
        return 1;
    }

    key_last_state[idx] = current;       /* 更新上一次状态 */
    return 0;
}

/**
  * @brief  扫描按键：检测“上一次松开、本次按下”的下降沿
  * @retval 刚被按下的按键编号 1~4，无按键按下返回 0
  */
uint8_t Key_Scan(void)
{
    for (uint8_t i = KEY1; i <= KEY4; i++)
    {
        if (Key_IsPressed(i))
        {
            return i;
        }
    }

    return 0;
}
