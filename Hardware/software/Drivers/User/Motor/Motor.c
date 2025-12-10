#include "Motor.h"

#define Motor_M0(x) \
    do{                                                  \
        HAL_GPIO_WritePin(M0_GPIO_Port, M0_Pin, x);      \
    }while(0)

#define Motor_M1(x) \
    do{                                                  \
        HAL_GPIO_WritePin(M1_GPIO_Port, M1_Pin, x);      \
    }while(0)

#define Motor_M2(x) \
    do{                                                  \
        HAL_GPIO_WritePin(M2_GPIO_Port, M2_Pin, x);      \
    }while(0)

extern TIM_HandleTypeDef htim1;

Motor_t motor;

void Motor_ApplyMicrostep(void);

void Motor_Init(void){
    motor.microStep_mode = MICROSTEP_32;
    motor.mode = Motor_SpeedMode;

    motor.speedMode_config.period_speed = 0;
    motor.speedMode_config.rad_speed = 0;

    motor.QuantitaveMode_config.speed = 0;
    motor.QuantitaveMode_config.period_speed = 0;
    motor.QuantitaveMode_config.current_step = 0;
    motor.QuantitaveMode_config.target_step = 0;
    motor.QuantitaveMode_config.RCR_LastUpdateValue = 0;
    motor.QuantitaveMode_config.RCR_UpdateBatch = 0;
}

void Motor_SetMode(Motor_Mode_t mode){
    motor.mode = mode;
}

Motor_Mode_t Motor_GetMode(void){
    // if (motor.mode == Motor_SpeedMode){
    //     return "SpeedMode";
    // }
    // else if(motor.mode == Motor_QuantitaveMode){
    //     return "StepMode ";
    // }
    return motor.mode;
}

/**
 * @brief   更新电机RCR值
 * @details 在步进模式下，更新定时器重载寄存器(TIM1->RCR)的值以实现电机的批量更新控制。
 *          在步进模式下，会根据配置的更新批次进行相应的设置。
 *          在定时器更新中断中调用该函数。
 */
void RCRBatch_Update(void){
    // 检查电机是否处于步进模式
    if(motor.mode == Motor_QuantitaveMode){
        // 更新步进模式下的批量运行
        // 如果更新批次大于1，设置定时器重载寄存器为最大值(256-1)
        if (motor.QuantitaveMode_config.RCR_UpdateBatch > 1){
            __HAL_TIM_SetRepetitionCounter(&htim1, 255);
        }
        // 最后一次更新(余数)
        else if (motor.QuantitaveMode_config.RCR_UpdateBatch == 1){
            __HAL_TIM_SetRepetitionCounter(&htim1, motor.QuantitaveMode_config.RCR_LastUpdateValue -1 );
        }
        else{
            Motor_Stop();
            motor.QuantitaveMode_config.current_step += motor.QuantitaveMode_config.RCR_LastUpdateValue;
            return;
        }
        motor.QuantitaveMode_config.current_step += 256;
        motor.QuantitaveMode_config.RCR_UpdateBatch--;
    }
    else{
        return;
    }
}

void Motor_SetMicrostep(MicrostepMode_t microStep){
    motor.microStep_mode = microStep;
}

void Motor_ApplyMicrostep(void){
    switch(motor.microStep_mode){
        case 1: Motor_M0(0); Motor_M1(0); Motor_M2(0); break;
        case 2: Motor_M0(0); Motor_M1(0); Motor_M2(1); break;
        case 4: Motor_M0(0); Motor_M1(1); Motor_M2(0); break;
        case 8: Motor_M0(0); Motor_M1(1); Motor_M2(1); break;
        case 16: Motor_M0(1); Motor_M1(0); Motor_M2(0); break;
        case 32: Motor_M0(1); Motor_M1(0); Motor_M2(1); break;

        default: Motor_M0(1); Motor_M1(0); Motor_M2(1); break;
    }
}

void Motor_SetSpeed(float speed_rpm){
    uint8_t microStep = motor.microStep_mode;
    if (motor.mode == Motor_SpeedMode){
        motor.speedMode_config.rad_speed = speed_rpm;
        motor.speedMode_config.period_speed = (uint32_t)(1e6 / ((speed_rpm * 200 * microStep) / 60.0f));
    }
    else if(motor.mode == Motor_QuantitaveMode){
        motor.QuantitaveMode_config.speed = speed_rpm;
        motor.QuantitaveMode_config.period_speed = (uint32_t)(1e6 / ((speed_rpm * 200 * microStep) / 60.0f));
    }
}

float Motor_GetSpeed(){
    if (motor.mode == Motor_SpeedMode){
        return motor.speedMode_config.rad_speed;
    }
    else if(motor.mode == Motor_QuantitaveMode){
        return motor.QuantitaveMode_config.speed;
    }
    return 0;
}


void Motor_SetQuantitave_ByStep(int64_t step){
    if (motor.mode == Motor_QuantitaveMode){
        motor.QuantitaveMode_config.target_step = step;
        motor.QuantitaveMode_config.current_step = 0;
        motor.QuantitaveMode_config.RCR_UpdateBatch = (step / 256) + 1;
        motor.QuantitaveMode_config.RCR_LastUpdateValue = step % 256;
    }
}   

void Motor_SetQuantitave_ByRad(float rad){
    Motor_SetQuantitave_ByStep(rad * 200 * motor.microStep_mode);
}

float Motor_GetQuantitave(void){
    int64_t remain_step = motor.QuantitaveMode_config.target_step - motor.QuantitaveMode_config.current_step;
    float remain_rad = (float)remain_step / (float)(200.f * motor.microStep_mode);
    return remain_rad;
}

/**
 * @brief 启动电机函数
 * 根据电机当前模式（速度模式或步进模式）配置电机参数并启动
 */
void Motor_Start(){
    Motor_ApplyMicrostep();
    Motor_EN(0);
    // 判断电机是否为速度模式
    if(motor.mode == Motor_SpeedMode){
        // 根据速度值确定电机转向
        if(motor.speedMode_config.rad_speed >= 0){
            Motor_DIR(1);  // 正转
        }
        else {
            Motor_DIR(0);  // 反转
        }
        // 设置定时器自动重载值和比较值，控制PWM输出
        __HAL_TIM_SetAutoreload(&htim1, motor.speedMode_config.period_speed);
        __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, motor.speedMode_config.period_speed / 2);
    }
    // 判断电机是否为步进模式
    else if(motor.mode == Motor_QuantitaveMode){
        // 根据目标步数确定电机转向
        if(motor.QuantitaveMode_config.target_step >= 0){
            Motor_DIR(1);  // 正转
        }
        else {
            Motor_DIR(0);  // 反转
        }
        // 设置定时器参数，控制步进电机的步进
        __HAL_TIM_SetAutoreload(&htim1, motor.QuantitaveMode_config.period_speed);
        __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, motor.QuantitaveMode_config.period_speed / 2);
        
                // 更新步进模式下的批量运行
        // 如果更新批次大于1，设置定时器重载寄存器为最大值(256-1)
        if (motor.QuantitaveMode_config.RCR_UpdateBatch > 1){
            __HAL_TIM_SetRepetitionCounter(&htim1, 255);
        }
        // 最后一次更新(余数)
        else if (motor.QuantitaveMode_config.RCR_UpdateBatch == 1){
            __HAL_TIM_SetRepetitionCounter(&htim1, motor.QuantitaveMode_config.RCR_LastUpdateValue -1 );
        }
        else{
            Motor_Stop();
            return;
        }
        motor.QuantitaveMode_config.RCR_UpdateBatch--;

    }
    else{
        Motor_EN(1);
        return;
    }
    // 启动PWM输出，使电机开始运转
    if (motor.mode == Motor_SpeedMode){
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    }
    else if(motor.mode == Motor_QuantitaveMode){
        // 启动定时器中断，控制步进电机的步进
        HAL_TIM_PWM_Start_IT(&htim1, TIM_CHANNEL_1);
        __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);

    }
}

void Motor_Stop(void){
    // 停止定时器
    HAL_TIM_PWM_Stop_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop(&htim1);
    __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE);

    Motor_EN(1);
}
