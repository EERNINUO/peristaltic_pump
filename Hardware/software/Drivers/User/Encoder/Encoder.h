#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "stm32f1xx_hal.h"
#include "main.h"

#define KeyPress() HAL_GPIO_ReadPin(Encoder_Key_GPIO_Port, Encoder_Key_Pin) == GPIO_PIN_SET

typedef enum {
    Key_NoPress = 0,
    Key_ShortPress,
    Key_LongPress
} Key_State;

void Start_EncoderTask(void *argument);
int16_t Get_EncoderCount(void);
Key_State Get_KeyValue(void);

#endif //__ENCODER_H__ 