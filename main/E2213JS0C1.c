#include "E2213JS0C1.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"
#include "font.h"
#include "driver/spi_master.h"

extern spi_device_handle_t spi;
static void E2213JS0C1_SendReadByte(uint8_t byte);
static void E2213JS0C1_WriteRegIndex(uint8_t cmd);
static void E2213JS0C1_WriteData8(uint8_t data);
//static void E2213JS0C1_WriteData16(uint16_t data);
static void E2213JS0C1_WriteMultipleData(uint8_t *pData, uint32_t Size);
static void E2213JS0C1_WaiteUntilNotBusy(void);

uint8_t E2213JS0C1_FirstFrameBuffer[E2213JS0C1_BUFFER_SIZE];
uint8_t E2213JS0C1_SecondFrameBuffer[E2213JS0C1_BUFFER_SIZE];

void spi_CMD(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t,0,sizeof(t));
    t.length = sizeof(uint8_t)*8;
    t.flags = SPI_TRANS_USE_TXDATA;
    t.tx_data[0] = cmd;
    ret = spi_device_polling_transmit(spi,&t);
    assert(ret == ESP_OK);
}

/**
 * @brief	SPI??/??????
 * @param	byte??????????????
 * @retval	?????????
 */
static void E2213JS0C1_SendReadByte(uint8_t byte)
{
//	/* ???Tx???????????? */
//	while (!LL_SPI_IsActiveFlag_TXE(E2213JS0C1_SPI));
//	/* ???????? */
//	LL_SPI_TransmitData8(E2213JS0C1_SPI, byte);
//	/* ??????????????? */
//	while(LL_SPI_IsActiveFlag_BSY(E2213JS0C1_SPI));
	// while (HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_TX);
	// HAL_SPI_Transmit(&hspi1, &byte, 1, 0xff);
    // SPI.write(byte);
    spi_CMD(byte);
}

/**
 * @brief	?????
 * @param	cmd?????????????
 * @retval	none
 */
static void E2213JS0C1_WriteRegIndex(uint8_t cmd)
{
    E2213JS0C1_CS_ENABLE();
	E2213JS0C1_DC_CMD();
	E2213JS0C1_SendReadByte(cmd);
	E2213JS0C1_DC_DATA();
    E2213JS0C1_CS_DISABLE();
}

/**
 * @brief	???????8bit??
 * @param	data??????????????(8bit)
 * @retval	none
 */
static void E2213JS0C1_WriteData8(uint8_t data)
{
    E2213JS0C1_CS_ENABLE();
	E2213JS0C1_SendReadByte(data);
    E2213JS0C1_CS_DISABLE();
}

///**
// * @brief	???????16bit??
// * @param	data??????????????(16bit)
// * @retval	none
// */
//static void E2213JS0C1_WriteData16(uint16_t data)
//{
//    E2213JS0C1_CS_ENABLE();
//	E2213JS0C1_SendReadByte(data >> 8);
//	E2213JS0C1_SendReadByte(data);
//    E2213JS0C1_CS_DISABLE();
//}

/**
 * @brief	????????BUSY???????
 * @param	none
 * @retval	none
 */
static void E2213JS0C1_WaiteUntilNotBusy(void)
{
    while(E2213JS0C1_BUSY_READ() == 0)
    { 
        delay(100);
    }    
}

/**
 * @brief	????????????????????
 * @param	pData?????????????????
 * @param	Size?????????????????
 * @retval	none
 */
static void E2213JS0C1_WriteMultipleData(uint8_t *pData, uint32_t Size)
{
	/* ???????uint32_t????????????? */
	uint32_t counter = 0;
    
	E2213JS0C1_CS_ENABLE();
    E2213JS0C1_DC_DATA();
	
	/* ??????????1????? */
	if (Size == 1)
	{
		/* ???????????????????????��????????? */
		E2213JS0C1_SendReadByte(*pData);	
	}
	/* ??????????1 */
	else
	{
		/* ??for?????????????????????????????????��?????u8???????��?????u16 */
		for (counter = Size; counter != 0; counter--)
		{
			E2213JS0C1_SendReadByte(*pData);
			E2213JS0C1_SendReadByte(*(pData + 1));
			counter--;
			pData += 2;
		}
	}
    E2213JS0C1_CS_DISABLE();
}

/**
 * @brief	?????
 * @param	none
 * @retval	none
 */
void E2213JS0C1_Init(void)
{

    /* ????SPI */
//    LL_SPI_Enable(E2213JS0C1_SPI); 
    
    /* ?????�� */
    E2213JS0C1_RST_ENABLE();
    delay(5);
    E2213JS0C1_RST_DISABLE();
    delay(10);
    E2213JS0C1_RST_ENABLE();
    delay(5);
    /* ??????�� */
    E2213JS0C1_WriteRegIndex(SOFT_RESET_CMD);
    E2213JS0C1_WriteData8(SOFT_RESET_DATA);
    delay(5);
    /* ???????????��??25?????? */
    E2213JS0C1_WriteRegIndex(SET_TEMP_CMD);
    E2213JS0C1_WriteData8(SET_TEMP_25_DATA);
    /* Active Temperature */
    E2213JS0C1_WriteRegIndex(ACTIVE_TEMP_CMD);
    E2213JS0C1_WriteData8(ACTIVE_TEMP_25_DATA);
//    /* Panel Settings??????????????????????????????????????? */
//    E2213JS0C1_WriteRegIndex(PANEL_SET_CMD);
//    E2213JS0C1_WriteData8(PANEL_SET_DATA_1);
//    E2213JS0C1_WriteData8(PANEL_SET_DATA_2);   
}

/**
 * @brief	????????????????????????????????
 * @param	none
 * @retval	none
 */
void E2213JS0C1_SendImageData(void)
{
    /* ????????Frame?????? */
    E2213JS0C1_WriteRegIndex(FIRST_FRAME_CMD); 
    E2213JS0C1_WriteMultipleData(E2213JS0C1_FirstFrameBuffer,E2213JS0C1_BUFFER_SIZE);    
    /* ????????Frame?????? */  
    E2213JS0C1_WriteRegIndex(SECOND_FRAME_CMD);
    E2213JS0C1_WriteMultipleData(E2213JS0C1_SecondFrameBuffer,E2213JS0C1_BUFFER_SIZE);
}

/**
 * @brief	???????????
 * @param	none
 * @retval	none
 */
void E2213JS0C1_SendUpdateCmd(void)
{
    /* ???BUSY?????? */
    E2213JS0C1_WaiteUntilNotBusy();
    /* Power on command????DC/DC */
    E2213JS0C1_WriteRegIndex(TURN_ON_DCDC_CMD);
    /* ???BUSY?????? */
    E2213JS0C1_WaiteUntilNotBusy();
    /* ?????? */
    E2213JS0C1_WriteRegIndex(DISPLAY_REFRESH_CMD);
    /* ???BUSY?????? */
    E2213JS0C1_WaiteUntilNotBusy();
}

/**
 * @brief	???DC/DC??????????????????
 * @param	none
 * @retval	none
 */
void E2213JS0C1_TurnOffDCDC(void)
{
    /* ???DC/DC???? */
    E2213JS0C1_WriteRegIndex(TURN_OFF_DCDC_CMD);
    /* ???BUSY?????? */
    E2213JS0C1_WaiteUntilNotBusy();
    /* ????????????????CS??MOSI??CLK????0???��?CVV???��?????????????? */
}


/**
 * @brief	??????????
 * @param	none
 * @retval	none
 */
uint8_t buffer1Single,buffer1Double,buffer2Single,buffer2Double;
void E2213JS0C1_ClearFullScreen(uint8_t color)
{
    uint16_t i,j;
    uint8_t buffer1,buffer2;

    /* ????????????????????????????buffer?????? */
    switch(color)
    {
        case RED:
            buffer1Single = RED_BUFFER1;
            buffer1Double = RED_BUFFER1;
            buffer2Single = RED_BUFFER2;
            buffer2Double = RED_BUFFER2;
            break;
        case WHITE:
            buffer1Single = WHITE_BUFFER1;
            buffer1Double = WHITE_BUFFER1;
            buffer2Single = WHITE_BUFFER2;
            buffer2Double = WHITE_BUFFER2;
            break;
        case BLACK:
            buffer1Single = BLACK_BUFFER1;
            buffer1Double = BLACK_BUFFER1;
            buffer2Single = BLACK_BUFFER2;
            buffer2Double = BLACK_BUFFER2;
            break;
        case LIGHTRED:
            buffer1Single = LIGHTRED_BUFFER1;
            buffer1Double = LIGHTRED_BUFFER1;
            buffer2Single = LIGHTRED_BUFFER2_SINGLE;
            buffer2Double = LIGHTRED_BUFFER2_DOUBLE;
            break;
        case GREY:
            buffer1Single = GREY_BUFFER1_SINGLE;
            buffer1Double = GREY_BUFFER1_DOUBLE;
            buffer2Single = GREY_BUFFER2;
            buffer2Double = GREY_BUFFER2;
            break;
        case DARKRED:
            buffer1Single = DARKRED_BUFFER1_SINGLE;
            buffer1Double = DARKRED_BUFFER1_DOUBLE;
            buffer2Single = DARKRED_BUFFER2_SINGLE;
            buffer2Double = DARKRED_BUFFER2_DOUBLE;
            break;
    } 
    /* ?????????????????Buffer */
    for (i = 0; i < E2213JS0C1_BUFFER_HEIGHT_SIZE; i++)
    {
        /* ????��???1��,?????????��??????????????????? */
        if((i % 2))
        {
            buffer1 = buffer1Single;
            buffer2 = buffer2Single;
        }
        else
        {
            buffer1 = buffer1Double;
            buffer2 = buffer2Double;
        }
        for(j = 0; j < E2213JS0C1_BUFFER_WIDTH_SIZE; j++)
        {
            E2213JS0C1_FirstFrameBuffer[i * E2213JS0C1_BUFFER_WIDTH_SIZE + j] = buffer1;
            E2213JS0C1_SecondFrameBuffer[i * E2213JS0C1_BUFFER_WIDTH_SIZE + j] = buffer2;
        }
    }
}

/**
 * @brief	????
 * @param	xPos??X??????
 * @param	yPos??Y??????
 * @param	color??????????????
 * @retval	none
 */
void E2213JS0C1_DrawPoint(uint8_t xPos, uint8_t yPos, uint8_t color)
{
    uint16_t i;
    uint8_t n;
    /* ?��?????????? */
    if ((xPos > E2213JS0C1_XPOS_MAX) || (yPos > E2213JS0C1_YPOS_MAX))
    {
        return;
    }
    /* ????????????????i???n��???��??? */
    i = yPos * E2213JS0C1_BUFFER_WIDTH_SIZE + (xPos / 8);
    n = 7- (xPos % 8);
    /* ????��???? */
    switch(color)
    {
        case RED:
            ClearBit(E2213JS0C1_FirstFrameBuffer[i], n);
            SetBit(E2213JS0C1_SecondFrameBuffer[i], n);
            break;
        case WHITE:
            ClearBit(E2213JS0C1_FirstFrameBuffer[i], n);
            ClearBit(E2213JS0C1_SecondFrameBuffer[i], n);
            break;
        case BLACK:
            SetBit(E2213JS0C1_FirstFrameBuffer[i], n);
            ClearBit(E2213JS0C1_SecondFrameBuffer[i], n);
            break;
        default: 
            return;
    }
}

/**
 * @brief	??????????/?????
 * @param	xStart??X?????????
 * @param	yStart??Y?????????
 * @param	length??????
 * @param	orientation??????HORIZONTAL??VERTICAL
 * @param	color??????????????
 * @retval	none
 */
void E2213JS0C1_DrawLine(uint8_t xStart, uint8_t yStart, uint8_t length, 
    enum ENUM_ORIENTATION orientation, uint8_t color)
{
    /* ?��?????????? */
    if ((xStart > E2213JS0C1_XPOS_MAX) || (yStart > E2213JS0C1_YPOS_MAX))
    {
        return;
    }
    else if ((orientation == HORIZONTAL) && (xStart + length - 1 > E2213JS0C1_XPOS_MAX))
    {
        return;
    }
    else if ((orientation == VERTICAL) && (yStart + length - 1 > E2213JS0C1_YPOS_MAX))
    {
        return;
    }
    /* ???? */
    for (uint8_t i = 0; i < length; i++)
    {
        E2213JS0C1_DrawPoint(xStart, yStart, color); 
        switch(orientation)
        {
            case HORIZONTAL:
                xStart++;           
                break;
            case VERTICAL:
                yStart++;
                break;
        }  
    }
}

/**
 * @brief	??????
 * @param	xStart??X?????????
 * @param	yStart??Y?????????
 * @param	width??????
 * @param	height?????
 * @param	fill?????SOLID/HOLLOW
 * @param	borderColor?????????????????
 * @param	fillColor?????????????????
 * @retval	none
 */
void E2213JS0C1_DrawRectangle(uint8_t xStart, uint8_t yStart, uint8_t width, 
    uint8_t height, enum ENUM_FILL fill, uint8_t borderColor, uint8_t fillColor)
{
    /* ?��?????????? */
    if ((xStart > E2213JS0C1_XPOS_MAX) || (yStart > E2213JS0C1_YPOS_MAX)  ||
        (xStart + width - 1 > E2213JS0C1_XPOS_MAX) || (yStart + height - 1 > E2213JS0C1_YPOS_MAX))
    {
        return;
    }    
    /* ????? */
    E2213JS0C1_DrawLine(xStart, yStart, width, HORIZONTAL,borderColor);    
    E2213JS0C1_DrawLine(xStart, yStart + height - 1, width, HORIZONTAL,borderColor);
    E2213JS0C1_DrawLine(xStart, yStart + 1, height - 2, VERTICAL,borderColor);
    E2213JS0C1_DrawLine(xStart + width - 1, yStart + 1, height - 2, VERTICAL,borderColor);
    /* ??? */
    if (fill == SOLID)
    {
        yStart++; 
        for (uint8_t i = 0; i < (height - 2); i++)
        {
           E2213JS0C1_DrawLine(xStart + 1, yStart, width - 2, HORIZONTAL,fillColor);  
           yStart++; 
        }
    }
}

/**
 * @brief	?????????
 * @param	xStart??X?????????
 * @param	yStart??Y?????????
 * @param	ch?????????????
 * @param	font?????
 * @param	fontColor?????????????????????
 * @param	backgroundColor??????????????????
 * @retval	?????????????????????????????????X??????
 */
uint8_t E2213JS0C1_ShowChar(uint8_t xStart, uint8_t yStart, uint8_t chr, 
    uint8_t font, uint8_t fontColor, uint8_t backgroundColor)
{
    uint8_t xPos, yPos, temp, fontWidth, fontHeight;
    chr = chr - ' ';//?????????
    xPos = xStart;
    yPos = yStart;
    switch(font)
	{
        case FONT_1608:
            fontWidth = FONT_1608_WIDTH;
            fontHeight = FONT_1608_HEIGHT;
            /* ????? */
            for (uint8_t t = 0; t < fontHeight; t++)
            {                   
                temp = (uint8_t)ACSII_1608[chr][t];
                /* ????? */
                for (uint8_t i = 0; i < fontWidth; i++)
                {
                    if (temp & 0x80)
                    {
                        E2213JS0C1_DrawPoint(xPos, yPos, fontColor);
                    }
                    else 
                    {
                        E2213JS0C1_DrawPoint(xPos, yPos, backgroundColor);
                    }
                    temp <<= 1;
                    xPos++;
                }
                xPos = xStart;
                yPos++;
            }                    
        break;
    }
    /* ??????????????? */
	return fontWidth;  
}

/**
 * @brief	????????
 * @param	startX???????x??????
 * @param	startY???????y??????
 * @param	str:?????????????
 * @param	font?????
 * @param	fontColor?????????????????????
 * @param	backgroundColor??????????????????
 * @retval	???????????????X???????
 */
uint8_t E2213JS0C1_ShowCharStr(uint8_t xStart, uint8_t yStart, char* str, 
    uint8_t font, uint8_t fontColor, uint8_t backgroundColor)
{	
	while(1)
	{	
		/* ?��?????????????????ACSII?��??? ' ' ?? '~'?????????��????????????????*/
		if ((*str <= '~') && (*str >= ' '))
		{
			xStart += E2213JS0C1_ShowChar(xStart, yStart, *str, font, fontColor, backgroundColor);
		}		
		/* ?��??????0x00????????? */
		else if (*str == 0x00)
		{
			/* ??????? */
			break;
		}
		/* ???????? */
		str++;
	}	
	return xStart;
}

/**
 * @brief	?????bmp??
 * @param	xStart?????????????x??????	    
 * @param	yStart?????????????y??????
 * @param	bmpWidth??x???????????
 * @param	bmpHeight??y??????????
 * @param	fontColor:?????????
 * @param	backgroundColor???????????
 * @param	pic????
 * @retval	none
 */
void E2213JS0C1_DrawBmp(uint8_t xStart, uint8_t yStart, uint8_t bmpWidth, 
    uint8_t bmpHeight, uint8_t fontColor, uint8_t backgroundColor, 
        const unsigned char* pic)
{
    /* ?��?????????? */
    if ((xStart > E2213JS0C1_XPOS_MAX) || (yStart > E2213JS0C1_YPOS_MAX)  ||
        (xStart + bmpWidth - 1 > E2213JS0C1_XPOS_MAX) || (yStart + bmpHeight - 1 > E2213JS0C1_YPOS_MAX))
    {
        return;
    }    
    uint8_t xPos, yPos, temp;
    xPos = xStart;
    yPos = yStart; 
    
    /* ????? */
    for (uint8_t t = 0; t < bmpHeight; t++)
    {   
        /* ????? */          
        for (uint8_t i = 0; i < (bmpWidth / 8); i++)
        {
            temp = *pic; 
            for(uint8_t j = 0; j < 8; j++)
            {
                if (temp & 0x80)
                {
                    E2213JS0C1_DrawPoint(xPos, yPos, fontColor);
                }
                else 
                {
                    E2213JS0C1_DrawPoint(xPos, yPos, backgroundColor);
                }
                temp <<= 1;
                xPos++;
            }
            pic++;
        }
        if ((bmpWidth % 8) != 0)
        {
            for (uint8_t i = 0; i < (bmpWidth % 8); i++)
            {
                temp = *pic; 
                if (temp & 0x80)
                {
                    E2213JS0C1_DrawPoint(xPos, yPos, fontColor);
                }
                else 
                {
                    E2213JS0C1_DrawPoint(xPos, yPos, backgroundColor);
                }
                temp <<= 1;
                xPos++;
            }
            pic++;
        }
        xPos = xStart;
        yPos++;
    }
}

/**
 * @brief	???????????????
 * @param	xStart?????????????x??????	    
 * @param	yStart?????????????y??????
 * @param	bmpWidth??x???????????
 * @param	bmpHeight??y??????????
 * @param	pic????
 * @retval	none
 */
uint8_t color;
void E2213JS0C1_DrawImage(uint8_t xStart, uint8_t yStart, uint8_t imageWidth, 
    uint8_t imageHeight, const unsigned char* pic)
{
    /* ?��?????????? */
    if ((xStart > E2213JS0C1_XPOS_MAX) || (yStart > E2213JS0C1_YPOS_MAX)  ||
        (xStart + imageWidth - 1 > E2213JS0C1_XPOS_MAX) || (yStart + imageHeight - 1 > E2213JS0C1_YPOS_MAX))
    {
        return;
    }
	uint16_t temp;
    uint8_t xPos, yPos;  
    
    xPos = xStart;
    yPos = yStart;
    /* ????? */
    for (uint8_t t = 0; t < imageHeight; t++)
    {            
        /* ????? */
        for (uint8_t i = 0; i < imageWidth; i++)
        {
            temp = *pic;
            temp = temp << 8;
            temp = temp + *++pic;            
            switch(temp)
            {
                /* ??? */
                case RGB565_WHITE:
                    color = WHITE;                
                break;
                /* ??? */
                case RGB565_RED:
                    color = BLACK;                  
                break;
                /* ??? */
                case RGB565_BLACK:
                    color = BLACK;                   
                break;
            }
            E2213JS0C1_DrawPoint(xPos, yPos, color);
            xPos++;
            pic++;
        }
        xPos = xStart;
        yPos++;
    }                
}
