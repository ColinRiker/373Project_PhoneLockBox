
#include "Screen_Driver.h"
#include "stm32l4xx_hal.h"
#define HSPI_INSTANCE &hspi1

extern SPI_HandleTypeDef hspi1;

int LCD_HEIGHT = LCD_HEIGHT_1;
int LCD_WIDTH = LCD_WIDTH_1;


void ILI9341_SPI_Send(unsigned char SPI_Data)
{
	HAL_SPI_Transmit(HSPI_INSTANCE, &SPI_Data, 1, 1);
}

void ILI9341_Write_Command(uint8_t Command)
{
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	ILI9341_SPI_Send(Command);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void ILI9341_Write_Data(uint8_t Data)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	ILI9341_SPI_Send(Data);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void ILI9341_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
	ILI9341_Write_Command(0x2A);
	ILI9341_Write_Data(X1>>8);
	ILI9341_Write_Data(X1);
	ILI9341_Write_Data(X2>>8);
	ILI9341_Write_Data(X2);

	ILI9341_Write_Command(0x2B);
	ILI9341_Write_Data(Y1>>8);
	ILI9341_Write_Data(Y1);
	ILI9341_Write_Data(Y2>>8);
	ILI9341_Write_Data(Y2);

	ILI9341_Write_Command(0x2C);
}

/*HARDWARE RESET*/
void ILI9341_Reset(void)
{
	HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}

/*Ser rotation of the screen - changes x0 and y0*/

void ILI9341_Set_Rotation(uint8_t rotation)
{
	ILI9341_Write_Command(0x36);
	HAL_Delay(1);

	switch(rotation)
	{
	case SCREEN_VERTICAL_1:
		ILI9341_Write_Data(0x40|0x08);
		LCD_WIDTH = 240;
		LCD_HEIGHT = 320;
		break;
	case SCREEN_HORIZONTAL_1:
		ILI9341_Write_Data(0x20|0x08);
		LCD_WIDTH  = 320;
		LCD_HEIGHT = 240;
		break;
	case SCREEN_VERTICAL_2:
		ILI9341_Write_Data(0x80|0x08);
		LCD_WIDTH  = 240;
		LCD_HEIGHT = 320;
		break;
	case SCREEN_HORIZONTAL_2:
		ILI9341_Write_Data(0x40|0x80|0x20|0x08);
		LCD_WIDTH  = 320;
		LCD_HEIGHT = 240;
		break;
	default:
		break;
	}
}


void ILI9341_Draw_Colour_Burst(uint16_t Colour, uint32_t Size)
{
	//SENDS COLOUR
	uint32_t Buffer_Size = 0;
	if((Size*2) < BURST_MAX_SIZE)
	{
		Buffer_Size = Size;
	}
	else
	{
		Buffer_Size = BURST_MAX_SIZE;
	}

	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

	unsigned char chifted = 	Colour>>8;;
	unsigned char burst_buffer[Buffer_Size];
	for(uint32_t j = 0; j < Buffer_Size; j+=2)
		{
			burst_buffer[j] = 	chifted;
			burst_buffer[j+1] = Colour;
		}

	uint32_t Sending_Size = Size*2;
	uint32_t Sending_in_Block = Sending_Size/Buffer_Size;
	uint32_t Remainder_from_block = Sending_Size%Buffer_Size;

	if(Sending_in_Block != 0)
	{
		for(uint32_t j = 0; j < (Sending_in_Block); j++)
			{
			HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char *)burst_buffer, Buffer_Size, 10);
			}
	}

	//REMAINDER!
	HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char *)burst_buffer, Remainder_from_block, 10);

	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}


/*Enable LCD display*/
void ILI9341_Enable(void)
{
	HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
}


/*Initialize LCD display*/
void ILI9341_Init(void)
{
	ILI9341_Enable();
	ILI9341_Reset();

	//SOFTWARE RESET
	ILI9341_Write_Command(0x01);
	HAL_Delay(1000);

	//POWER CONTROL A
	ILI9341_Write_Command(0xCB);
	ILI9341_Write_Data(0x39);
	ILI9341_Write_Data(0x2C);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x34);
	ILI9341_Write_Data(0x02);

	//POWER CONTROL B
	ILI9341_Write_Command(0xCF);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0xC1);
	ILI9341_Write_Data(0x30);

	//DRIVER TIMING CONTROL A
	ILI9341_Write_Command(0xE8);
	ILI9341_Write_Data(0x85);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x78);

	//DRIVER TIMING CONTROL B
	ILI9341_Write_Command(0xEA);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x00);

	//POWER ON SEQUENCE CONTROL
	ILI9341_Write_Command(0xED);
	ILI9341_Write_Data(0x64);
	ILI9341_Write_Data(0x03);
	ILI9341_Write_Data(0x12);
	ILI9341_Write_Data(0x81);

	//PUMP RATIO CONTROL
	ILI9341_Write_Command(0xF7);
	ILI9341_Write_Data(0x20);

	//POWER CONTROL,VRH[5:0]
	ILI9341_Write_Command(0xC0);
	ILI9341_Write_Data(0x23);

	//POWER CONTROL,SAP[2:0];BT[3:0]
	ILI9341_Write_Command(0xC1);
	ILI9341_Write_Data(0x10);

	//VCM CONTROL
	ILI9341_Write_Command(0xC5);
	ILI9341_Write_Data(0x3E);
	ILI9341_Write_Data(0x28);

	//VCM CONTROL 2
	ILI9341_Write_Command(0xC7);
	ILI9341_Write_Data(0x86);

	//MEMORY ACCESS CONTROL
	ILI9341_Write_Command(0x36);
	ILI9341_Write_Data(0x48);

	//PIXEL FORMAT
	ILI9341_Write_Command(0x3A);
	ILI9341_Write_Data(0x55);

	//FRAME RATIO CONTROL, STANDARD RGB COLOR
	ILI9341_Write_Command(0xB1);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x10);

	//DISPLAY FUNCTION CONTROL
	ILI9341_Write_Command(0xB6);
	ILI9341_Write_Data(0x08);
	ILI9341_Write_Data(0x82);
	ILI9341_Write_Data(0x27);

	//3GAMMA FUNCTION DISABLE
	ILI9341_Write_Command(0xF2);
	ILI9341_Write_Data(0x00);

	//GAMMA CURVE SELECTED
	ILI9341_Write_Command(0x26);
	ILI9341_Write_Data(0x01);

	//POSITIVE GAMMA CORRECTION
	ILI9341_Write_Command(0xE0);
	ILI9341_Write_Data(0x0F);
	ILI9341_Write_Data(0x31);
	ILI9341_Write_Data(0x2B);
	ILI9341_Write_Data(0x0C);
	ILI9341_Write_Data(0x0E);
	ILI9341_Write_Data(0x08);
	ILI9341_Write_Data(0x4E);
	ILI9341_Write_Data(0xF1);
	ILI9341_Write_Data(0x37);
	ILI9341_Write_Data(0x07);
	ILI9341_Write_Data(0x10);
	ILI9341_Write_Data(0x03);
	ILI9341_Write_Data(0x0E);
	ILI9341_Write_Data(0x09);
	ILI9341_Write_Data(0x00);

	//NEGATIVE GAMMA CORRECTION
	ILI9341_Write_Command(0xE1);
	ILI9341_Write_Data(0x00);
	ILI9341_Write_Data(0x0E);
	ILI9341_Write_Data(0x14);
	ILI9341_Write_Data(0x03);
	ILI9341_Write_Data(0x11);
	ILI9341_Write_Data(0x07);
	ILI9341_Write_Data(0x31);
	ILI9341_Write_Data(0xC1);
	ILI9341_Write_Data(0x48);
	ILI9341_Write_Data(0x08);
	ILI9341_Write_Data(0x0F);
	ILI9341_Write_Data(0x0C);
	ILI9341_Write_Data(0x31);
	ILI9341_Write_Data(0x36);
	ILI9341_Write_Data(0x0F);

	//EXIT SLEEP
	ILI9341_Write_Command(0x11);
	HAL_Delay(120);

	//TURN ON DISPLAY
	ILI9341_Write_Command(0x29);

	//STARTING ROTATION
//	ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
	ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);

}

//INTERNAL FUNCTION OF LIBRARY, USAGE NOT RECOMENDED, USE Draw_Pixel INSTEAD
/*Sends single pixel colour information to LCD*/
void ILI9341_Draw_Colour(uint16_t Colour)
{
//SENDS COLOUR
	unsigned char TempBuffer[2] = {Colour>>8, Colour};
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(HSPI_INSTANCE, TempBuffer, 2, 1);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
}

void ILI9341_Fill_Screen(uint16_t Colour)
{
	ILI9341_Set_Address(0,0,LCD_WIDTH,LCD_HEIGHT);
	ILI9341_Draw_Colour_Burst(Colour, LCD_WIDTH*LCD_HEIGHT);
}

void ILI9341_Draw_Pixel(uint16_t X,uint16_t Y,uint16_t Colour)
{
	if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;	//OUT OF BOUNDS!

	//ADDRESS
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	ILI9341_SPI_Send(0x2A);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	//XDATA
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	unsigned char Temp_Buffer[4] = {X>>8,X, (X+1)>>8, (X+1)};
	HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer, 4, 1);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	//ADDRESS
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	ILI9341_SPI_Send(0x2B);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	//YDATA
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	unsigned char Temp_Buffer1[4] = {Y>>8,Y, (Y+1)>>8, (Y+1)};
	HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer1, 4, 1);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	//ADDRESS
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	ILI9341_SPI_Send(0x2C);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

	//COLOUR
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
	unsigned char Temp_Buffer2[2] = {Colour>>8, Colour};
	HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer2, 2, 1);
	HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);

}


void ILI9341_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour)
{
	if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
	if((X+Width-1)>=LCD_WIDTH)
		{
			Width=LCD_WIDTH-X;
		}
	if((Y+Height-1)>=LCD_HEIGHT)
		{
			Height=LCD_HEIGHT-Y;
		}
	ILI9341_Set_Address(X, Y, X+Width-1, Y+Height-1);
	ILI9341_Draw_Colour_Burst(Colour, Height*Width);
}


//DRAW LINE FROM X,Y LOCATION to X+Width,Y LOCATION
void ILI9341_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour)
{
	if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
	if((X+Width-1)>=LCD_WIDTH)
		{
			Width=LCD_WIDTH-X;
		}
	ILI9341_Set_Address(X, Y, X+Width-1, Y);
	ILI9341_Draw_Colour_Burst(Colour, Width);
}


//DRAW LINE FROM X,Y LOCATION to X,Y+Height LOCATION
void ILI9341_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Colour)
{
	if((X >=LCD_WIDTH) || (Y >=LCD_HEIGHT)) return;
	if((Y+Height-1)>=LCD_HEIGHT)
		{
			Height=LCD_HEIGHT-Y;
		}
	ILI9341_Set_Address(X, Y, X, Y+Height-1);
	ILI9341_Draw_Colour_Burst(Colour, Height);
}




//INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
//FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom 16bit)
/*Sets address (entire screen) and Sends Height*Width ammount of colour information to LCD*/

void ILI9341_Draw_Char(char ch, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor)
{
	if ((ch < 31) || (ch > 127)) return;

	uint8_t fOffset, fWidth, fHeight, fBPL;
	uint8_t *tempChar;

	fOffset = font[0];
	fWidth = font[1];
	fHeight = font[2];
	fBPL = font[3];

	tempChar = (uint8_t*)&font[((ch - 0x20) * fOffset) + 4]; /* Current Character = Meta + (Character Index * Offset) */

	/* Clear background first */
	ILI9341_Draw_Rectangle(X, Y, fWidth, fHeight, bgcolor);

	for (int j=0; j < fHeight; j++)
	{
		for (int i=0; i < fWidth; i++)
		{
			uint8_t z =  tempChar[fBPL * i + ((j & 0xF8) >> 3) + 1]; /* (j & 0xF8) >> 3, increase one by 8-bits */
			uint8_t b = 1 << (j & 0x07);
			if (( z & b ) != 0x00)
			{
				ILI9341_Draw_Pixel(X+i, Y+j, color);
			}
		}
	}
}

void ILI9341_Draw_Text(const char* str, const uint8_t font[], uint16_t X, uint16_t Y, uint16_t color, uint16_t bgcolor)
{
	uint8_t charWidth;			/* Width of character */
	uint8_t fOffset = font[0];	/* Offset of character */
	uint8_t fWidth = font[1];	/* Width of font */

	while (*str)
	{
		ILI9341_Draw_Char(*str, font, X, Y, color, bgcolor);

		/* Check character width and calculate proper position */
		uint8_t *tempChar = (uint8_t*)&font[((*str - 0x20) * fOffset) + 4];
		charWidth = tempChar[0];

		if(charWidth + 2 < fWidth)
		{
			/* If character width is smaller than font width */
			X += (charWidth + 2);
		}
		else
		{
			X += fWidth;
		}

		str++;
	}
}
/*Draws a full screen picture from flash. Image converted from RGB .jpeg/other to C array using online converter*/
//USING CONVERTER: http://www.digole.com/tools/PicturetoC_Hex_converter.php
//65K colour (2Bytes / Pixel)
void ILI9341_Draw_Image(const char* Image_Array, uint8_t Orientation)
{
	if(Orientation == SCREEN_HORIZONTAL_1)
	{
		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
		ILI9341_Set_Address(0,0,LCD_WIDTH,LCD_HEIGHT);

		HAL_GPIO_WritePin(LCD_DC_PORT,LCD_DC_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);
		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < LCD_WIDTH*LCD_HEIGHT*2/BURST_MAX_SIZE; i++)
		{
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];
				}
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);
				counter += BURST_MAX_SIZE;
		}
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	}
	else if(Orientation == SCREEN_HORIZONTAL_2)
	{
		ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);
		ILI9341_Set_Address(0,0,LCD_WIDTH,LCD_HEIGHT);

		HAL_GPIO_WritePin(LCD_DC_PORT,LCD_DC_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < LCD_WIDTH*LCD_HEIGHT*2/BURST_MAX_SIZE; i++)
		{
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];
				}
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);
				counter += BURST_MAX_SIZE;
		}
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	}
	else if(Orientation == SCREEN_VERTICAL_2)
	{
		ILI9341_Set_Rotation(SCREEN_VERTICAL_2);
		ILI9341_Set_Address(0,0,LCD_HEIGHT,LCD_WIDTH);

		HAL_GPIO_WritePin(LCD_DC_PORT,LCD_DC_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < LCD_WIDTH*LCD_HEIGHT*2/BURST_MAX_SIZE; i++)
		{
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];
				}
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);
				counter += BURST_MAX_SIZE;
		}
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	}
	else if(Orientation == SCREEN_VERTICAL_1)
	{
		ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
		ILI9341_Set_Address(0,0,LCD_HEIGHT,LCD_WIDTH);

		HAL_GPIO_WritePin(LCD_DC_PORT,LCD_DC_PIN, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_RESET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for(uint32_t i = 0; i < LCD_WIDTH*LCD_HEIGHT*2/BURST_MAX_SIZE; i++)
		{
				for(uint32_t k = 0; k< BURST_MAX_SIZE; k++)
				{
					Temp_small_buffer[k]	= Image_Array[counter+k];
				}
				HAL_SPI_Transmit(&hspi1, (unsigned char*)Temp_small_buffer, BURST_MAX_SIZE, 10);
				counter += BURST_MAX_SIZE;
		}
		HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
	}
}
void ILI9341_Draw_HollowCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color)
{
	int x = radius-1;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (radius << 1);

	while (x >= y)
	{
		ILI9341_Draw_Pixel(X + x, Y + y, color);
		ILI9341_Draw_Pixel(X + y, Y + x, color);
		ILI9341_Draw_Pixel(X - y, Y + x, color);
		ILI9341_Draw_Pixel(X - x, Y + y, color);
		ILI9341_Draw_Pixel(X - x, Y - y, color);
		ILI9341_Draw_Pixel(X - y, Y - x, color);
		ILI9341_Draw_Pixel(X + y, Y - x, color);
		ILI9341_Draw_Pixel(X + x, Y - y, color);

		if (err <= 0)
		{
			y++;
			err += dy;
			dy += 2;
		}

		if (err > 0)
		{
			x--;
			dx += 2;
			err += (-radius << 1) + dx;
		}
	}
}

void ILI9341_Draw_FilledCircle(uint16_t X, uint16_t Y, uint16_t radius, uint16_t color)
{

	int x = radius;
	int y = 0;
	int xChange = 1 - (radius << 1);
	int yChange = 0;
	int radiusError = 0;

	ILI9341_Draw_Rectangle(X-radius-10,Y-radius-10,(radius*2)+20,(radius*2)+20,WHITE);



	while (x >= y)
	{
		for (int i = X - x; i <= X + x; i++)
		{
			ILI9341_Draw_Pixel(i, Y + y,color);
			ILI9341_Draw_Pixel(i, Y - y,color);
		}

		for (int i = X - y; i <= X + y; i++)
		{
			ILI9341_Draw_Pixel(i, Y + x,color);
			ILI9341_Draw_Pixel(i, Y - x,color);
		}

		y++;
		radiusError += yChange;
		yChange += 2;

		if (((radiusError << 1) + xChange) > 0)
		{
			x--;
			radiusError += xChange;
			xChange += 2;
		}
	}
}

void ILI9341_Draw_HollowRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	float negCalc = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	//DRAW HORIZONTAL!
	if(!negX)
	{
		xLen = X1 - X0;
	}
	else
	{
		xLen = X0 - X1;
	}
	ILI9341_Draw_Horizontal_Line(X0, Y0, xLen, color);
	ILI9341_Draw_Horizontal_Line(X0, Y1, xLen, color);

	//DRAW VERTICAL!
	if(!negY)
	{
		yLen = Y1 - Y0;
	}
	else
	{
		yLen = Y0 - Y1;
	}

	ILI9341_Draw_Vertical_Line(X0, Y0, yLen, color);
	ILI9341_Draw_Vertical_Line(X1, Y0, yLen, color);

	if((xLen > 0)||(yLen > 0))
	{
		ILI9341_Draw_Pixel(X1, Y1, color);
	}
}

void ILI9341_Draw_FilledRectangleCoord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t color)
{
	uint16_t xLen = 0;
	uint16_t yLen = 0;
	uint8_t negX = 0;
	uint8_t negY = 0;
	int32_t negCalc = 0;
	uint16_t X0True = 0;
	uint16_t Y0True = 0;

	negCalc = X1 - X0;
	if(negCalc < 0) negX = 1;
	negCalc = 0;

	negCalc = Y1 - Y0;
	if(negCalc < 0) negY = 1;

	if(!negX)
	{
		xLen = X1 - X0;
		X0True = X0;
	}
	else
	{
		xLen = X0 - X1;
		X0True = X1;
	}

	if(!negY)
	{
		yLen = Y1 - Y0;
		Y0True = Y0;
	}
	else
	{
		yLen = Y0 - Y1;
		Y0True = Y1;
	}

	ILI9341_Draw_Rectangle(X0True, Y0True, xLen, yLen, color);
}
//DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
//Location is dependant on screen orientation. x0 and y0 locations change with orientations.
//Using pixels to draw big simple structures is not recommended as it is really slow
//Try using either rectangles or lines if possible
//

//DRAW RECTANGLE OF SET SIZE AND HEIGTH AT X and Y POSITION WITH CUSTOM COLOUR
//
//Rectangle is hollow. X and Y positions mark the upper left corner of rectangle
//As with all other draw calls x0 and y0 locations dependant on screen orientation
//

