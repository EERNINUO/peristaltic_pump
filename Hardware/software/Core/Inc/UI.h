#ifndef _UI_H_
#define _UI_H_

#include "stm32f1xx_hal.h"

typedef struct{
    uint8_t index;
    char* name;
    void (*func)(void);
} MenuItem_t;

void start_UITask(void);

#endif /* _UI_H_ */
