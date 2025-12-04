#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "gpio.h"

#define SPI_mode

#define OLED_WIDTH 128
#define OLED_HEIGHT 64  

#define OLED_CS(x) do{HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, (GPIO_PinState)x);}while(0)
#define OLED_DC(x) do{HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, (GPIO_PinState)x);}while(0)
#define OLED_RES(x) do{HAL_GPIO_WritePin(RES_GPIO_Port, RES_Pin, (GPIO_PinState)x);}while(0)

void OLED_Init(void);
void OLED_update(void);
void OLED_Clear(void);
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t* Image);
void OLED_ShowChar(uint8_t X, uint8_t Y, char Char);
void OLED_ShowString(uint8_t X, uint8_t Y, char *String);

#endif // __OLED_H