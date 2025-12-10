#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "stm32f1xx_hal.h"
#include "main.h"

#define OneStep 0.017453292519943295769236907684886f // 1 step = 1.8 degree

#define Motor_EN(x) \
    do{                                                  \
        HAL_GPIO_WritePin(EN_GPIO_Port, EN_Pin, (GPIO_PinState)x);     \
    }while(0)

#define Motor_DIR(x) \
    do{                                                  \
        HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, (GPIO_PinState)x);    \
    }while(0)

#ifndef __HAL_TIM_SetRepetitionCounter
#define __HAL_TIM_SetRepetitionCounter(__HANDLE__, __REPCOUNTER__) \
    do{                                                    \
        (__HANDLE__)->Instance->RCR = (__REPCOUNTER__);  \
        (__HANDLE__)->Init.RepetitionCounter = (__REPCOUNTER__);    \
    } while(0)
#endif

typedef enum{
    Motor_SpeedMode, 
    Motor_QuantitaveMode
} Motor_Mode_t;

typedef enum {
    MICROSTEP_1 = 1,
    MICROSTEP_2 = 2,
    MICROSTEP_4 = 4,
    MICROSTEP_8 = 8,
    MICROSTEP_16 = 16,
    MICROSTEP_32 = 32
} MicrostepMode_t;

typedef struct{ // 定速模式配置
    float rad_speed; // 速度(rad/min)
    uint16_t period_speed; // 速度(定时器周期(us))
} SpeedMode_Config_t;

typedef struct{ // 定量模式配置
    float speed; // 速度(rad/min)
    uint16_t period_speed; // 速度(定时器周期(us))
    int64_t current_step; // 当前步数
    int64_t target_step; // 目标步数
    uint32_t RCR_UpdateBatch; // RC重装载剩余次数
    uint8_t RCR_LastUpdateValue; // 最后一次RC重装载值
} QuantitaveMode_Config_t;

typedef struct Motor{
    uint8_t microStep_mode; // 1, 2, 4, 8, 16, 32
    Motor_Mode_t mode; // 模式
    SpeedMode_Config_t speedMode_config; // 定速模式配置
    QuantitaveMode_Config_t QuantitaveMode_config; // 定步模式配置
} Motor_t;

void Motor_Init(void);
void RCRBatch_Update(void);
void Motor_SetSpeed(float speed);   
float Motor_GetSpeed();
void Motor_SetQuantitave_ByStep(int64_t step);
void Motor_SetQuantitave_ByRad(float rad);
float Motor_GetQuantitave(void);
void Motor_SetMicrostep(MicrostepMode_t microstep);
void Motor_SetMode(Motor_Mode_t mode);
Motor_Mode_t Motor_GetMode(void);
void Motor_Start(void);
void Motor_Stop(void);

#endif