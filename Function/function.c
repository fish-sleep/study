#include "function.h"
#include <stdio.h>

/**
  * @brief  PB10(KEY1) 按下时翻转 LED 亮灭状态
  */
void Function_LED_Handle(void)
{
    if (Key_IsPressed(KEY1))
    {
        LED_Toggle();
    }
}

/**
  * @brief  PB11(KEY2) 按下时翻转蜂鸣器状态
  */
void Function_Buzzer_Handle(void)
{
    if (Key_IsPressed(KEY2))
    {
        Buzzer_Toggle();
    }
}

/* 74HC595 步进点灯状态 */
static uint8_t hc595_step = 0;   /* 下一次要点亮的位置 0~7 */

/**
  * @brief  PB3(KEY3) 按下步进点亮下一个灯（到最后一个后回到第一个）
  */
void Function_HC595_Handle(void)
{
    if (Key_IsPressed(KEY3))
    {
        HC595_WriteByte(1 << hc595_step);        /* 点亮当前位置 */
        hc595_step = (hc595_step + 1) % 8;       /* 指向下一个，到最后一个后回到第一个 */
    }
}

/**
  * @brief  输入一个角度，让指定舵机转动到对应角度
  * @param  servo 舵机选择：SERVO_X(PB5) 或 SERVO_Y(PB4)
  * @param  angle 目标角度(0~180°)
  */
void Function_Servo_Rotate(uint8_t servo, float angle)
{
    Servo_SetAngle(servo, angle);
}

/*====================== 菜单系统 ======================*/

/* 菜单状态 */
typedef enum
{
    MENU_STATE_MODE_SELECT = 0,   /* 模式选择界面 */
    MENU_STATE_BASIC,             /* 基础外设模式 */
    MENU_STATE_SERVO,             /* 舵机控制模式 */
    MENU_STATE_AHT30,             /* AHT30 温湿度监控模式 */
    MENU_STATE_BMI160,            /* BMI160 六轴监控模式 */
    MENU_STATE_ADC                /* ADC 监控模式 */
} Menu_State_t;

/* 可选模式编号 */
#define MODE_BASIC   0
#define MODE_SERVO   1
#define MODE_AHT30   2
#define MODE_BMI160  3
#define MODE_ADC     4
#define MODE_COUNT   5

/* 舵机控制模式下控制哪个舵机（可改为 SERVO_Y） */
#define MENU_SERVO_CH   SERVO_X

static Menu_State_t menu_state = MENU_STATE_MODE_SELECT;
static uint8_t mode_index = MODE_BASIC;   /* 模式选择界面的光标位置 */
static int16_t servo_angle = 0;           /* 舵机当前角度（与 Servo_Init 初始值一致） */
static uint32_t aht30_tick = 0;           /* AHT30 上一次读取的时刻 */
static uint8_t aht30_init_ok = 0;         /* AHT30 初始化结果，1=成功 */
static uint32_t bmi160_tick = 0;          /* BMI160 上一次读取的时刻 */
static uint32_t bmi160_page_tick = 0;     /* BMI160 上一次切换显示页的时刻 */
static uint8_t bmi160_page = 0;           /* 0=加速度页，1=陀螺仪页 */
static uint8_t bmi160_init_ok = 0;        /* BMI160 初始化结果，1=成功 */
static uint32_t adc_tick = 0;             /* ADC 上一次刷新的时刻 */
static uint8_t display_dirty = 1;         /* 1=需要刷新 OLED */

/* 模式名称（与 MODE_xxx 编号顺序一致） */
static const char *const mode_names[MODE_COUNT] = {
    "Basic IO",
    "Servo Ctrl",
    "AHT30",
    "BMI160",
    "ADC Monitor"
};

/**
  * @brief  显示“模式选择”界面（当前项高亮在第 2 行，列表滚动）
  */
static void Menu_ModeSelect_Show(void)
{
    char line[16];

    OLED_Clear();
    OLED_ShowString(1, 1, "Mode Select");

    sprintf(line, ">%s", mode_names[mode_index]);
    OLED_ShowString(2, 1, line);

    sprintf(line, " %s", mode_names[(mode_index + 1) % MODE_COUNT]);
    OLED_ShowString(3, 1, line);

    sprintf(line, " %s", mode_names[(mode_index + 2) % MODE_COUNT]);
    OLED_ShowString(4, 1, line);
}

/**
  * @brief  显示“基础外设”界面
  */
static void Menu_Basic_Show(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "Basic IO");
    OLED_ShowString(2, 1, "K1:LED");
    OLED_ShowString(3, 1, "K2:Buzz K3:595");
    OLED_ShowString(4, 1, "K4:Back");
}

/**
  * @brief  显示“舵机控制”界面
  */
static void Menu_Servo_Show(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "Servo Ctrl");
    OLED_ShowString(2, 1, "Angle:");
    OLED_ShowNum(2, 8, (uint32_t)servo_angle, 3);
    OLED_ShowString(3, 1, "K1:+10 K2:-10");
    OLED_ShowString(4, 1, "K4:Back");
}

/**
  * @brief  模式选择界面：PB10 选下一个，PB11 确定进入
  */
static void Menu_ModeSelect_Handle(void)
{
    if (Key_IsPressed(KEY1))                 /* PB10：选择下一个 */
    {
        mode_index = (mode_index + 1) % MODE_COUNT;
        display_dirty = 1;
    }

    if (Key_IsPressed(KEY2))                 /* PB11：确定 */
    {
        switch (mode_index)
        {
            case MODE_BASIC:
                menu_state = MENU_STATE_BASIC;
                break;
            case MODE_SERVO:
                menu_state = MENU_STATE_SERVO;
                break;
            case MODE_AHT30:
                menu_state = MENU_STATE_AHT30;
                break;
            case MODE_BMI160:
                menu_state = MENU_STATE_BMI160;
                break;
            case MODE_ADC:
                menu_state = MENU_STATE_ADC;
                break;
            default:
                menu_state = MENU_STATE_BASIC;
                break;
        }
        display_dirty = 1;
        return;                              /* 状态已切换，由新状态刷新显示 */
    }

    if (display_dirty)
    {
        Menu_ModeSelect_Show();
        display_dirty = 0;
    }
}

/**
  * @brief  基础外设模式：运行 LED/蜂鸣器/HC595 功能，PA8 返回
  */
static void Menu_Basic_Handle(void)
{
    if (display_dirty)
    {
        Menu_Basic_Show();
        Message_Init();          /* 进入基础外设模式：启动串口收发 */
        display_dirty = 0;
    }

    Function_LED_Handle();       /* PB10 翻转 LED */
    Function_Buzzer_Handle();    /* PB11 翻转蜂鸣器 */
    Function_HC595_Handle();     /* PB3 步进点灯 */

    if (Key_IsPressed(KEY4))                 /* PA8：返回模式选择 */
    {
        Message_Stop();                      /* 退出基础外设模式：停止串口收发 */
        menu_state = MENU_STATE_MODE_SELECT;
        display_dirty = 1;
        return;                              /* 状态已切换，由新状态刷新显示 */
    }
}

/**
  * @brief  舵机控制模式：PB10 角度 +10，PB11 角度 -10，PA8 返回
  */
static void Menu_Servo_Handle(void)
{
    if (Key_IsPressed(KEY1))                 /* PB10：角度 +10 */
    {
        servo_angle += 10;
        if (servo_angle > 180) { servo_angle = 180; }
        Servo_SetAngle(MENU_SERVO_CH, (float)servo_angle);
        display_dirty = 1;
    }

    if (Key_IsPressed(KEY2))                 /* PB11：角度 -10 */
    {
        servo_angle -= 10;
        if (servo_angle < 0) { servo_angle = 0; }
        Servo_SetAngle(MENU_SERVO_CH, (float)servo_angle);
        display_dirty = 1;
    }

    if (Key_IsPressed(KEY4))                 /* PA8：返回模式选择 */
    {
        menu_state = MENU_STATE_MODE_SELECT;
        display_dirty = 1;
        return;                              /* 状态已切换，由新状态刷新显示 */
    }

    if (display_dirty)
    {
        Menu_Servo_Show();
        display_dirty = 0;
    }
}

/**
  * @brief  刷新 AHT30 温湿度显示（保留 1 位小数）
  */
static void Menu_AHT30_ShowData(void)
{
    AHT30_Data_t aht;
    char buf[16];

    OLED_Clear();
    OLED_ShowString(1, 1, "AHT30 Monitor");

    if (AHT30_ReadData(&aht) != 0)
    {
        OLED_ShowString(2, 1, "T: --");
        OLED_ShowString(3, 1, "H: --");
    }
    else
    {
        int t_int = (int)aht.temperature;
        int t_dec = (int)((aht.temperature - (float)t_int) * 10.0f);
        int h_int = (int)aht.humidity;
        int h_dec = (int)((aht.humidity - (float)h_int) * 10.0f);
        if (t_dec < 0) { t_dec = -t_dec; }
        if (h_dec < 0) { h_dec = -h_dec; }

        sprintf(buf, "T:%d.%d C", t_int, t_dec);
        OLED_ShowString(2, 1, buf);

        sprintf(buf, "H:%d.%d %%", h_int, h_dec);
        OLED_ShowString(3, 1, buf);
    }

    OLED_ShowString(4, 1, "K4:Back");
}

/**
  * @brief  AHT30 监控模式：进入时检查初始化返回值，每 500ms 刷新，PA8 返回
  */
static void Menu_AHT30_Handle(void)
{
    if (display_dirty)
    {
        aht30_tick = 0;                        /* 进入时立即读取一次 */
        aht30_init_ok = (AHT30_Init() == 0) ? 1 : 0;   /* 检查初始化返回值 */
        display_dirty = 0;
    }

    if (Key_IsPressed(KEY4))                 /* PA8：返回模式选择 */
    {
        menu_state = MENU_STATE_MODE_SELECT;
        display_dirty = 1;
        return;                              /* 状态已切换，由新状态刷新显示 */
    }

    /* 每 500ms 刷新一次温湿度 */
    if ((HAL_GetTick() - aht30_tick) >= 500)
    {
        aht30_tick = HAL_GetTick();

        if (aht30_init_ok)
        {
            Menu_AHT30_ShowData();
        }
        else
        {
            OLED_Clear();
            OLED_ShowString(1, 1, "AHT30 Monitor");
            OLED_ShowString(2, 1, "Init Fail");
            OLED_ShowString(4, 1, "K4:Back");
        }
    }
}

/**
  * @brief  把 float 格式化成 1 位小数字符串（如 "12.3"、"-0.5"）
  */
static void Format1Dec(char *buf, float val)
{
    int i = (int)val;
    int d = (int)((val - (float)i) * 10.0f);
    if (d < 0) { d = -d; }
    sprintf(buf, "%d.%d", i, d);
}

/**
  * @brief  刷新 BMI160 显示：加速度页或陀螺仪页
  */
static void Menu_BMI160_ShowData(void)
{
    BMI160_Data_t bmi;
    char buf[16];
    char t[8];

    OLED_Clear();

    if (BMI160_ReadData(&bmi) != 0)
    {
        OLED_ShowString(1, 1, "BMI160");
        OLED_ShowString(2, 1, "Read Fail");
        OLED_ShowString(4, 1, "K4:Back");
        return;
    }

    if (bmi160_page == 0)
    {
        OLED_ShowString(1, 1, "BMI160 Acc");
        Format1Dec(t, bmi.acc_x); sprintf(buf, "X:%s g", t); OLED_ShowString(2, 1, buf);
        Format1Dec(t, bmi.acc_y); sprintf(buf, "Y:%s g", t); OLED_ShowString(3, 1, buf);
        Format1Dec(t, bmi.acc_z); sprintf(buf, "Z:%s g", t); OLED_ShowString(4, 1, buf);
    }
    else
    {
        OLED_ShowString(1, 1, "BMI160 Gyr");
        Format1Dec(t, bmi.gyr_x); sprintf(buf, "X:%s", t); OLED_ShowString(2, 1, buf);
        Format1Dec(t, bmi.gyr_y); sprintf(buf, "Y:%s", t); OLED_ShowString(3, 1, buf);
        Format1Dec(t, bmi.gyr_z); sprintf(buf, "Z:%s", t); OLED_ShowString(4, 1, buf);
    }
}

/**
  * @brief  BMI160 监控模式：进入时检查初始化，每 1s 切换加速度/陀螺仪页，
  *         每 200ms 刷新数据，PA8 返回
  */
static void Menu_BMI160_Handle(void)
{
    if (display_dirty)
    {
        bmi160_tick = 0;                      /* 进入时立即刷新一次 */
        bmi160_page_tick = HAL_GetTick();     /* 1 秒后再切换显示页 */
        bmi160_page = 0;
        bmi160_init_ok = (BMI160_Init() == 0) ? 1 : 0;   /* 检查初始化返回值 */

        if (!bmi160_init_ok)
        {
            OLED_Clear();
            OLED_ShowString(1, 1, "BMI160");
            OLED_ShowString(2, 1, "Init Fail");
            OLED_ShowString(4, 1, "K4:Back");
        }

        display_dirty = 0;
    }

    if (Key_IsPressed(KEY4))                 /* PA8：返回模式选择 */
    {
        menu_state = MENU_STATE_MODE_SELECT;
        display_dirty = 1;
        return;                              /* 状态已切换，由新状态刷新显示 */
    }

    if (!bmi160_init_ok)
    {
        return;
    }

    /* 每 1 秒切换一次显示页（加速度 / 陀螺仪） */
    if ((HAL_GetTick() - bmi160_page_tick) >= 1000)
    {
        bmi160_page_tick = HAL_GetTick();
        bmi160_page = bmi160_page ? 0 : 1;
        bmi160_tick = 0;                     /* 切页后立即刷新 */
    }

    /* 每 200ms 刷新一次数据 */
    if ((HAL_GetTick() - bmi160_tick) >= 200)
    {
        bmi160_tick = HAL_GetTick();
        Menu_BMI160_ShowData();
    }
}

/**
  * @brief  ADC 原始值换算成电压（参考 3.3V、12 位分辨率 4095），返回毫伏值
  */
static uint16_t Function_ADC_ToMillivolt(uint16_t adc)
{
    return (uint16_t)(((uint32_t)adc * 3300u) / 4095u);
}

/**
  * @brief  刷新 ADC 监控显示：原始值 + 对应电压
  */
static void Menu_ADC_ShowData(void)
{
    char buf[16];
    uint16_t adc = Pot_GetValue();
    uint16_t mv = Function_ADC_ToMillivolt(adc);

    OLED_Clear();
    OLED_ShowString(1, 1, "ADC Monitor");

    sprintf(buf, "ADC:%d", (int)adc);
    OLED_ShowString(2, 1, buf);

    sprintf(buf, "V:%d.%03dV", (int)(mv / 1000), (int)(mv % 1000));
    OLED_ShowString(3, 1, buf);

    OLED_ShowString(4, 1, "K4:Back");
}

/**
  * @brief  ADC 监控模式：每 100ms 刷新一次，PA8 返回
  */
static void Menu_ADC_Handle(void)
{
    if (display_dirty)
    {
        adc_tick = 0;                        /* 进入时立即刷新一次 */
        display_dirty = 0;
    }

    if (Key_IsPressed(KEY4))                 /* PA8：返回模式选择 */
    {
        menu_state = MENU_STATE_MODE_SELECT;
        display_dirty = 1;
        return;
    }

    if ((HAL_GetTick() - adc_tick) >= 100)   /* 每 100ms 刷新一次 */
    {
        adc_tick = HAL_GetTick();
        Menu_ADC_ShowData();
    }
}

/**
  * @brief  菜单主循环：需在主程序 while(1) 中周期调用
  */
void Function_Menu_Handle(void)
{
    switch (menu_state)
    {
        case MENU_STATE_MODE_SELECT:
            Menu_ModeSelect_Handle();
            break;

        case MENU_STATE_BASIC:
            Menu_Basic_Handle();
            break;

        case MENU_STATE_SERVO:
            Menu_Servo_Handle();
            break;

        case MENU_STATE_AHT30:
            Menu_AHT30_Handle();
            break;

        case MENU_STATE_BMI160:
            Menu_BMI160_Handle();
            break;

        case MENU_STATE_ADC:
            Menu_ADC_Handle();
            break;

        default:
            menu_state = MENU_STATE_MODE_SELECT;
            display_dirty = 1;
            break;
    }
}
