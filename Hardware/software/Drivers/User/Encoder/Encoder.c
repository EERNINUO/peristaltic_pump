#include "Encoder.h"
#include "cmsis_os2.h"

extern TIM_HandleTypeDef htim3;

uint8_t KeyValue = Key_NoPress;

/**
 * @brief 编码器任务函数，用于检测按键短按和长按事件
 * @param argument 任务参数（未使用）
 * @note 该函数在一个无限循环中运行，持续检测按键状态
 */
void Start_EncoderTask(void *argument) {
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    for(;;)
    {
        if (KeyPress()){
            uint32_t StartPress_Time = osKernelGetTickCount();
            osDelay(10);
            if (KeyPress()){
                while (KeyPress()){
                    osDelay(10);
                    uint32_t Now_Time = osKernelGetTickCount();
                    if (Now_Time - StartPress_Time > 1000){
                        break;
                    }
                }
                uint32_t Press_Time = osKernelGetTickCount() - StartPress_Time;
                if (Press_Time < 1000) KeyValue = Key_ShortPress;
                else KeyValue = Key_LongPress;
            }
        }
        osDelay(10);
    }
}

int16_t Get_EncoderCount(void) {
    uint16_t EncoderCount = __HAL_TIM_GET_COUNTER(&htim3);
    if (EncoderCount % 4 == 0) {
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        return (int16_t)EncoderCount / 4;
    }

    return 0;
}

Key_State Get_KeyValue(void) {
    uint8_t KeyValue_Temp = KeyValue;
    KeyValue = Key_NoPress;
    return KeyValue_Temp;
}