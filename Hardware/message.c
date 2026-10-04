#include "message.h"
#include "led.h"
#include "buzzer.h"
#include "hc595.h"
#include <stdio.h>
#include <string.h>

/* 接收缓冲：逐字节累积命令，以 \n 作为一帧结束 */
#define MSG_RX_BUF_SIZE   32
static uint8_t rx_buf[MSG_RX_BUF_SIZE];
static uint8_t rx_index = 0;
static uint8_t rx_data = 0;   /* 单字节接收缓冲 */

/**
  * @brief  初始化串口消息模块：清空缓冲并启动接收中断
  */
void Message_Init(void)
{
    rx_index = 0;              /* 清空接收缓冲 */
    HAL_UART_Receive_IT(&huart2, &rx_data, 1);
}

/**
  * @brief  停止串口接收（退出基础外设模式时调用）
  */
void Message_Stop(void)
{
    HAL_UART_AbortReceive_IT(&huart2);
}

/**
  * @brief  通过串口发送 LED / Buzzer / HC595 的当前状态
  * @note   格式：LED:ON Buzzer:OFF HC595:LED3\r\n
  */
void Message_SendStatus(void)
{
    char buf[64];
    uint8_t led = HAL_GPIO_ReadPin(LED_GPIO_PORT, LED_GPIO_PIN);
    uint8_t buz = HAL_GPIO_ReadPin(BUZZER_GPIO_PORT, BUZZER_GPIO_PIN);
    uint8_t hc  = HC595_GetData();
    int8_t light = -1;   /* -1 表示没有灯亮 */

    /* 找出 HC595 当前点亮的第一个灯（bit0 对应第 1 个灯） */
    for (uint8_t i = 0; i < 8; i++)
    {
        if (hc & (1u << i))
        {
            light = (int8_t)(i + 1);
            break;
        }
    }

    if (light >= 0)
    {
        sprintf(buf, "LED:%s Buzzer:%s HC595:LED%d\r\n",
                (led == LED_ON_LEVEL) ? "ON" : "OFF",
                (buz == GPIO_PIN_SET) ? "ON" : "OFF",
                light);
    }
    else
    {
        sprintf(buf, "LED:%s Buzzer:%s HC595:NONE\r\n",
                (led == LED_ON_LEVEL) ? "ON" : "OFF",
                (buz == GPIO_PIN_SET) ? "ON" : "OFF");
    }

    HAL_UART_Transmit(&huart2, (uint8_t *)buf, (uint16_t)strlen(buf), 100);
}

/**
  * @brief  串口接收完成回调（重定义 HAL 弱函数）
  * @note   收到的字节依次存入接收缓冲，收到换行时判断整条命令：
  *         命令为“State”则返回外设状态，否则回复“读取失败”，
  *         最后重新开启接收中断等待下一帧
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (rx_data == '\n')                     /* 收到换行，一帧命令结束 */
        {
            rx_buf[rx_index] = '\0';             /* 补字符串结束符 */

            if (strcmp((char *)rx_buf, "State") == 0)
            {
                Message_SendStatus();            /* 命令正确：返回外设状态 */
            }
            else
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)"Read Fail\r\n", 11, 100);
            }

            rx_index = 0;                        /* 清空缓冲，准备下一帧 */
        }
        else if (rx_data == '\r')
        {
            /* 忽略回车 */
        }
        else
        {
            /* 存入普通字符，防止越界 */
            if (rx_index < (MSG_RX_BUF_SIZE - 1))
            {
                rx_buf[rx_index++] = rx_data;
            }
        }

        /* 重新开启接收中断 */
        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}
