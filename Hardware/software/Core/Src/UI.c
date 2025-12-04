#include "UI.h"
#include "OLED.h"   

void start_UITask(void){
    while (1) {
        uint32_t lastUpdateTime = osKernelGetTickCount();

        // UI update code here
        #warning You shuld implement the UI update code here

        // Update the OLED display
        OLED_update();

        osDelayUntil(lastUpdateTime + 5);
    }
}