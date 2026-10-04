#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "main.h"
#include "usart.h"

void Message_Init(void);       /* 初始化：启动串口接收中断 */
void Message_Stop(void);       /* 停止串口接收（退出基础外设模式时调用） */
void Message_SendStatus(void); /* 发送 LED / Buzzer / HC595 当前状态 */

/* HAL 串口接收完成回调（重定义 HAL 弱函数）：
   收到上位机数据后回复“成功接收”，并重新开启接收 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif
