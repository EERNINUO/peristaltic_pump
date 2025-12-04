#include "OLED.h"
#include "OLED_font.h"
#include <string.h>

extern SPI_HandleTypeDef hspi1;

uint8_t GraphicMemory[8][128] = {0};

void OLED_send_data(uint8_t dat){ 
    OLED_DC(1);
    OLED_CS(0);
    HAL_SPI_Transmit(&hspi1, &dat, 1, 1000);
    OLED_CS(1);
}

void OLED_send_cmd(uint8_t cmd){
    OLED_DC(0);
    OLED_CS(0);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 1000);
    OLED_CS(1);
}

/**
 * @brief 更新OLED显示屏内容
 * @note 该函数通过DMA方式逐页更新OLED显示屏，每次更新一页内容
 *       页地址从0到7循环更新，实现整个屏幕的刷新
 */
void OLED_update(){
    static uint8_t page = 0;  

    OLED_send_cmd(0xb0+ page); 
    OLED_send_cmd(0x00+ 2); 
    OLED_send_cmd(0x10); 

    OLED_DC(1); 
    OLED_CS(0);
    HAL_SPI_Transmit_DMA(&hspi1, GraphicMemory[page], 128); 
    page = (page + 1) % 8;
}

void OLED_Init(){
    HAL_Delay(100);
    OLED_RES(1);
    HAL_Delay(100);

    OLED_send_cmd(0xAE);   //关闭显示

    OLED_send_cmd(0xd5);   //设置时钟频率分频因子
    OLED_send_cmd(0x80);   //设置时钟频率驱动率
    OLED_send_cmd(0xa8);   //设置驱动路数
    OLED_send_cmd(0x3F);   //1/64
    OLED_send_cmd(0xD3);   //设置显示偏移
    OLED_send_cmd(0x00);   //设置显示偏移
    OLED_send_cmd(0x40);   //设置显示开始行
    OLED_send_cmd(0xA1);   //设置内存地址模式
    OLED_send_cmd(0xC8);   //设置COM输出扫描方向
    OLED_send_cmd(0xDA);   //设置COM硬件引脚配置
    OLED_send_cmd(0x12);   //
    OLED_send_cmd(0x81);   //对比度设置
    OLED_send_cmd(0xCF);   //设置电荷泵
    OLED_send_cmd(0xD9);   //设置预充电周期
    OLED_send_cmd(0xF1);   //设置VCOMH取消选择级别
    OLED_send_cmd(0xDB);   //设置VCOMH取消选择级别
    OLED_send_cmd(0x30);   //
    OLED_send_cmd(0xA4);   //全局显示开启
    OLED_send_cmd(0xA6);   //设置显示方式

    OLED_send_cmd(0x8D);   //电荷泵设置
    OLED_send_cmd(0x14);   //设置电荷泵
    OLED_send_cmd(0xAF);   //开启显示

    HAL_Delay(100);

    // 清空OLED屏幕
    uint8_t i,j;
    for(i=0;i<8;i++){
        OLED_send_cmd(0xb0+i); //  发送命令，设置页地址
        OLED_send_cmd(0x00 + 2); //  发送命令，设置列地址低4位
        OLED_send_cmd(0x10); //  发送命令，设置列地址高4位

        for (j=0;j<128;j++){ //  循环128次，对应OLED屏幕的128列
            OLED_send_data(0x00); //  发送数据，清空屏幕
        }
    }
}

void OLED_Clear(void){
    memset(GraphicMemory, 0, sizeof(GraphicMemory));
}

void OLED_set_point(uint8_t x, uint8_t y){
    OLED_send_cmd(0xb0 + y);
    OLED_send_cmd(0x02 + (x&0x0f));
    OLED_send_cmd(0x10 + (x>>4));
}

/**
	* @brief	清空局部内容
	* @param	x: 起始列
	* @param	y: 起始行
	* @param	Width: 清空宽度
	* @param	Height: 清空高度
	* @retval	无
	*/
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height){
	// 确认参数是否有效
	if (X >= OLED_WIDTH || Y >= OLED_HEIGHT || Width == 0 || Height == 0) return;
	// 判断可写域
	uint8_t effx = ((X + Width) >= OLED_WIDTH) ? OLED_WIDTH : (X + Width);
	uint8_t effy = ((Y + Height) >= OLED_HEIGHT) ? OLED_HEIGHT : (Y + Height);
	
	for (uint8_t j = Y; j < effy; j++){
		for (uint8_t i = X; i < effx; i++){
			GraphicMemory[j / 8][i] &= ~(0x01 << (j % 8));
		}
	}
}

/**
 * @brief 在OLED显示屏上显示图像
 * @param X: 图像显示的起始列坐标(0-127)
 * @param Y: 图像显示的起始行坐标(0-63)
 * @param Width: 图像的宽度(像素)
 * @param Height: 图像的高度(像素)
 * @param Image: 图像数据指针，数据格式为纵向8位打包
 * @note  该函数会自动处理图像边界，超出屏幕的部分将被裁剪
 *        图像数据格式为纵向8位打包，即每8个垂直像素打包为1个字节
 * @retval 无
 */
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, uint8_t* Image){
	// 清空指定区域
	OLED_ClearArea(X, Y, Width, Height);
	// 参数合法性检查
    if (Image == NULL || Width == 0 || Height == 0) return;
    if (X >= OLED_WIDTH || Y >= OLED_HEIGHT) return;
	
	// 计算有效显示区域（裁剪超出屏幕的部分）
    uint8_t maxX = (X + Width > OLED_WIDTH) ? OLED_WIDTH : X + Width;
    uint8_t maxY = (Y + Height > OLED_HEIGHT) ? OLED_HEIGHT : Y + Height;
    uint8_t effWidth  = maxX - X;
    uint8_t effHeight = maxY - Y;

	// 如果有效区域为0，则直接返回
	if (effWidth == 0 || effHeight == 0) return; 

	for (uint8_t j = 0; j < ((effHeight - 1) / 8 + 1); j++){
		for (uint8_t i = 0; i < effWidth; i++){
			// 检查目标页是否超出边界
			if ((Y / 8 + j) >= (OLED_HEIGHT / 8)) break;
			GraphicMemory[Y / 8 + j][X + i] |= Image[Width * j + i] << (Y % 8);
			// 确保不越界访问下一个页
			if (((Y % 8) != 0) && ((Y / 8 + j + 1) < (OLED_HEIGHT / 8))){
				GraphicMemory[Y / 8 + 1 + j][X + i] |= Image[Width * j + i] >> (8 - (Y % 8));
			}
		}
	}
}

void OLED_ShowChar(uint8_t X, uint8_t Y, char Char){
	if (Char < 32 || Char > 127) return; // 只支持 ASCII 32~127
	// 确定字符在数组中的位置
	uint8_t index = Char - ' ';
	
	OLED_ShowImage(X, Y, ENGLISH_WIDTH, ENGLISH_HEIGHT, (uint8_t*)OLED_F8X16[index]);
}

void OLED_ShowString(uint8_t X, uint8_t Y, char *String){
    // 字符串为空或页数超出范围，返回错误
    if (!String || Y >= OLED_HEIGHT) return;

    uint8_t x = X;
    uint8_t y = Y;

    for (uint8_t i = 0; String[i] != '\0'; i++){
        if (X > 128) break; // 屏幕已满

        if (String[i] == '\n') { // 手动换行
            x = X;
            y += ENGLISH_HEIGHT;
            continue;
        }
        // 自动换行：当前行已满
        if (x + ENGLISH_WIDTH > 128){
            x = X;
            y += ENGLISH_HEIGHT;
            if (y >= ENGLISH_HEIGHT) break;
        }
        OLED_ShowChar(x, y, String[i]);
        x += ENGLISH_WIDTH;
    }
}
