#include "UI.h"
#include "cmsis_os2.h"
#include "OLED.h"   
#include "Motor.h"
#include "Encoder.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define DISPLAY_ROWS 4          // 显示行数
#define DISPLAY_COLS 16         // 每行字符数（假设使用16字符LCD）

void Item1_Fuction(void);
void Item2_Fuction(void);
void Item3_Fuction(void);
void RunningCtrl_Fuction(void);
void Show_Menu(void);

MenuItem_t menuItems[] = {
    {1, "Mode", Item1_Fuction},
    {2, "Speed", Item2_Fuction},
    {3, "Step", Item3_Fuction},
    {4, "Start", RunningCtrl_Fuction},
};

int8_t cursor = 0;  // 用于跟踪当前选中的菜单项
uint8_t counter = 0;

void start_UITask(void){
    while (1) {
        // UI update code here
        uint32_t lastUpdateTime = osKernelGetTickCount();
        // Update the OLED display

        Show_Menu();
        // OLED_ShowString(0, 0, "Mode:");
        cursor += Get_EncoderCount();
        if (cursor > 4) {
            cursor = 0;
        }
        else if (cursor < 0) {
            cursor = 4;
        }

        if (Motor_GetMode() == Motor_QuantitaveMode) 
            OLED_ShowString(56, 0, "StepMode ");
        else if (Motor_GetMode() == Motor_SpeedMode) 
            OLED_ShowString(56, 0, "SpeedMode");

        OLED_printf(56, 17, "% 4.2f", Motor_GetSpeed());
        OLED_printf(56, 33, "% 4.2f", Motor_GetQuantitave());
        #warning You shuld implement the UI update code here

        if (Get_KeyValue() == Key_ShortPress){
            menuItems[cursor - 1].func();
        }

        OLED_update();

        osDelayUntil(lastUpdateTime + 5);
    }
}

void Show_Menu(void){
    for (int i = 1; i <= DISPLAY_ROWS; i++) {
        if (i == cursor) {
            OLED_ShowOppositeString(0, (i-1) * 16, menuItems[i-1].name);
        }
        else {
            OLED_ShowString(0, (i-1) * 16, menuItems[i-1].name);
        }
    }
}

void Item1_Fuction(void){
    OLED_ShowString(0, 0, "Mode");
    while (1) {
        if (Motor_GetMode() == Motor_QuantitaveMode) 
            OLED_ShowOppositeString(56, 0, "StepMode ");
        else if (Motor_GetMode() == Motor_SpeedMode) 
            OLED_ShowOppositeString(56, 0, "SpeedMode");

        if (Get_EncoderCount() != 0) {
            Motor_SetMode(Motor_GetMode() == Motor_QuantitaveMode ? Motor_SpeedMode : Motor_QuantitaveMode);
        }

        if (Get_KeyValue() == Key_ShortPress) {
            if (Motor_GetMode() == Motor_QuantitaveMode) 
                OLED_ShowString(56, 0, "StepMode ");
            else if (Motor_GetMode() == Motor_SpeedMode) 
                OLED_ShowString(56, 0, "SpeedMode");

            OLED_ShowOppositeString(0, 0, "Mode");

            break;
        }

        OLED_update();
        osDelay(5);
    }
}

void Item2_Fuction(void){
    
}

void Item3_Fuction(void){
    
}

void RunningCtrl_Fuction(void){
    Motor_Start();
}

void OLED_Printf_ChooseBitOpposite(uint8_t x, uint8_t y, uint8_t BitOfOpp,char *str, ...){
    char str_buf[16] = {0};
    va_list args;

    va_start(args, str);
    vsprintf(str_buf, str, args);
    va_end(args);

    for (int i = 0; i < strlen(str_buf); i++){
        if (i == BitOfOpp){
            OLED_ShowOppositeChar(x + i * 8, y, str_buf[i]);
        }
        else{
            OLED_ShowChar(x + i * 8, y, str_buf[i]);
        }
    }
}