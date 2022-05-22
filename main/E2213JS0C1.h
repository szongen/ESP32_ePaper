#ifndef __E2213JS0C1_H
#define __E2213JS0C1_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define delay(x) vTaskDelay(x)

#define E2213JS0C1_CS 5
#define E2213JS0C1_DC 26
#define E2213JS0C1_RST 25
#define E2213JS0C1_BUSY 27


#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
/*引脚定义*************************************************************************/
// /* SPI号 */
// #define E2213JS0C1_SPI					(SPI1)
/* CS */
#define E2213JS0C1_CS_ENABLE() gpio_set_level(E2213JS0C1_CS,GPIO_PIN_RESET)
	// HAL_GPIO_WritePin(DO_SCREEN_CS_GPIO_Port, DO_SCREEN_CS_Pin ,GPIO_PIN_RESET)
#define E2213JS0C1_CS_DISABLE() gpio_set_level(E2213JS0C1_CS,GPIO_PIN_SET)
    // HAL_GPIO_WritePin(DO_SCREEN_CS_GPIO_Port, DO_SCREEN_CS_Pin ,GPIO_PIN_SET)
/* DC */
#define E2213JS0C1_DC_DATA() gpio_set_level(E2213JS0C1_DC,GPIO_PIN_SET);
    // HAL_GPIO_WritePin(DO_SCREEN_DC_GPIO_Port, DO_SCREEN_DC_Pin,GPIO_PIN_SET)
#define E2213JS0C1_DC_CMD()  gpio_set_level(E2213JS0C1_DC,GPIO_PIN_RESET)
    // HAL_GPIO_WritePin(DO_SCREEN_DC_GPIO_Port, DO_SCREEN_DC_Pin,GPIO_PIN_RESET)
/* RST */
#define E2213JS0C1_RST_ENABLE() gpio_set_level(E2213JS0C1_RST,GPIO_PIN_SET)
    // HAL_GPIO_WritePin(DO_SCREEN_RST_GPIO_Port, DO_SCREEN_RST_Pin,GPIO_PIN_SET)
#define E2213JS0C1_RST_DISABLE() gpio_set_level(E2213JS0C1_RST,GPIO_PIN_RESET)
    // HAL_GPIO_WritePin(DO_SCREEN_RST_GPIO_Port, DO_SCREEN_RST_Pin,GPIO_PIN_RESET)
/* BUSY */
#define E2213JS0C1_BUSY_READ() gpio_get_level(E2213JS0C1_BUSY)
    // HAL_GPIO_ReadPin(DI_SCREEN_BUSY_GPIO_Port,DI_SCREEN_BUSY_Pin)
/*枚举*****************************************************************************/
/* 方向：水平/垂直 */
enum ENUM_ORIENTATION
{
    HORIZONTAL,
    VERTICAL
};
/* 填充：实心/空心 */
enum ENUM_FILL
{
    SOLID,
    HOLLOW
};
/*屏幕参数*************************************************************************/
/* X轴坐标0~103；Y轴坐标0~211 */
#define E2213JS0C1_W 104
#define E2213JS0C1_H 212
#define E2213JS0C1_XPOS_MAX 103
#define E2213JS0C1_YPOS_MAX 211
#define E2213JS0C1_BUFFER_SIZE (E2213JS0C1_W * E2213JS0C1_H / 8)
#define E2213JS0C1_BUFFER_WIDTH_SIZE (E2213JS0C1_W / 8)
#define E2213JS0C1_BUFFER_HEIGHT_SIZE (E2213JS0C1_H)
/*颜色*****************************************************************************/
// uint8_t 
// {
//     RED,
//     WHITE,
//     BLACK,
//     GREY,
//     DARKRED,
//     LIGHTRED
// };
#define RED 1
#define WHITE 2
#define BLACK 3
#define GREY 4
#define DARKRED 5
#define LIGHTRED 6
/* 01红，00白，10黑 */
#define RED_BUFFER1             0X00
#define RED_BUFFER2             0XFF
#define WHITE_BUFFER1           0x00
#define WHITE_BUFFER2           0x00
#define BLACK_BUFFER1           0xFF
#define BLACK_BUFFER2           0x00
/* 红白相间=浅红色，红黑相间=深红色，黑白相间=灰色 */
#define GREY_BUFFER1_SINGLE     0xAA
#define GREY_BUFFER1_DOUBLE     0x55
#define GREY_BUFFER2            0x00
#define DARKRED_BUFFER1_SINGLE  0xAA
#define DARKRED_BUFFER1_DOUBLE  0x55
#define DARKRED_BUFFER2_SINGLE  0x55
#define DARKRED_BUFFER2_DOUBLE  0xAA
#define LIGHTRED_BUFFER1        0x00
#define LIGHTRED_BUFFER2_SINGLE 0xAA
#define LIGHTRED_BUFFER2_DOUBLE 0x55
/* RGB565 */
#define RGB565_RED              0xF800
#define RGB565_WHITE            0xFFFF
#define RGB565_BLACK            0x0000
/*字库*****************************************************************************/
#define FONT_1608		    (0)
#define FONT_1608_WIDTH		(8)
#define FONT_1608_HEIGHT	(16)
/*指令*****************************************************************************/
#define SOFT_RESET_CMD          0x00
#define SOFT_RESET_DATA         0x0E
#define SET_TEMP_CMD            0xE5
#define SET_TEMP_25_DATA        0x19
#define ACTIVE_TEMP_CMD         0xE0
#define ACTIVE_TEMP_25_DATA     0x02
//#define PANEL_SET_CMD         0x00
//#define PANEL_SET_DATA_1      0xCF
//#define PANEL_SET_DATA_2      0x89
#define FIRST_FRAME_CMD         0x10
#define SECOND_FRAME_CMD        0x13
#define TURN_ON_DCDC_CMD        0x04
#define TURN_OFF_DCDC_CMD       0x02
#define DISPLAY_REFRESH_CMD     0x12
/*位操作***************************************************************************/
/* 指定的某一位数置1 */
#define SetBit(x, y)   (x |= (1<<y))
/* 指定的某一位数置0 */
#define ClearBit(x, y) (x &= ~(1<<y))
/* 指定的某一位数取反 */
#define ReverseBit(x,y) (x^=(1<<y))
/* 获取的某一位的值 */
#define GetBit(x, y)   ((x>>y) & 1)
/* 指定的某一位数置为指定的0或1 */
#define WriteBit(data, position, flag)   (flag ? SetBit(data, position) : ClearBit(data, position))
/*函数*******************************************************************************/
void E2213JS0C1_Init(void);
void E2213JS0C1_SendImageData(void);
void E2213JS0C1_ClearFullScreen(uint8_t color);
void E2213JS0C1_SendUpdateCmd(void);
void E2213JS0C1_TurnOffDCDC(void);
void E2213JS0C1_DrawPoint(uint8_t xPos, uint8_t yPos, uint8_t color);
void E2213JS0C1_DrawLine(uint8_t xStart, uint8_t yStart, uint8_t length, 
    enum ENUM_ORIENTATION orientation, uint8_t color);
void E2213JS0C1_DrawRectangle(uint8_t xStart, uint8_t yStart, uint8_t width, 
    uint8_t height, enum ENUM_FILL fill, uint8_t borderColor, 
    uint8_t fillColor);
uint8_t E2213JS0C1_ShowChar(uint8_t xStart, uint8_t yStart, uint8_t chr, 
    uint8_t font, uint8_t fontColor, uint8_t backgroundColor);
uint8_t E2213JS0C1_ShowCharStr(uint8_t xStart, uint8_t yStart, char* str, 
    uint8_t font, uint8_t fontColor, uint8_t backgroundColor);
void E2213JS0C1_DrawBmp(uint8_t xStart, uint8_t yStart, uint8_t bmpWidth, 
    uint8_t bmpHeight, uint8_t fontColor, uint8_t backgroundColor, 
    const unsigned char* pic);
void E2213JS0C1_DrawImage(uint8_t xStart, uint8_t yStart, uint8_t imageWidth, 
    uint8_t imageHeight, const unsigned char* pic);
#endif
