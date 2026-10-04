#ifndef __BUZZER_H
#define __BUZZER_H

#include "main.h"

/* PA15 蜂鸣器引脚（已在 MX_GPIO_Init 中配置为推挽输出）：高电平响 */
#define BUZZER_GPIO_PORT   GPIOA
#define BUZZER_GPIO_PIN    GPIO_PIN_15

void Buzzer_Init(void);     /* 初始化（关闭蜂鸣器） */
void Buzzer_On(void);       /* 响 */
void Buzzer_Off(void);      /* 停 */
void Buzzer_Toggle(void);   /* 翻转 */

#endif
