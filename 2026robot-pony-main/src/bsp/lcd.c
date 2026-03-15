#include "stm32f7xx_hal.h"
#include "lcd.h"
#include "lcdfont.h"

/*
  函数说明：写数据包
  入口数据：dat为指向要发送的数据缓冲区的首地址的指针，lenth为数据长度，单位字节
  返回值：  无
*/
void LCD_WriteDataPacket(uint8_t *dat,uint16_t lenth)
{
	LCD_CS_Clr();
	HAL_SPI_Transmit(&hspi1,dat,lenth,100);
	LCD_CS_Set();
}


/*
  函数说明：写数据
  入口数据：dat为要写入的数据
  返回值：  无
*/
void LCD_WriteData(uint8_t dat)
{
	LCD_CS_Clr();
	HAL_SPI_Transmit(&hspi1,&dat,1,100);
	LCD_CS_Set();
}


/*
  函数说明：写命令
  入口数据：dat为要发送的命令
  返回值：  无
*/
void LCD_WriteCom(uint8_t dat)
{
	LCD_CS_Clr();
	LCD_DC_Clr();
	HAL_SPI_Transmit(&hspi1,&dat,1,100);
	LCD_DC_Set();
	LCD_CS_Set();
}


void LCD_Write16Data(uint16_t dat)
{
	LCD_CS_Clr();
	uint8_t hdat = dat>>8;uint8_t ldat = dat;
	HAL_SPI_Transmit(&hspi1,&hdat,1,100);
	HAL_SPI_Transmit(&hspi1,&ldat,1,100);
	LCD_CS_Set();
}
/*
  函数说明：设置起始和结束地址
  入口数据：x1,x2 设置列的起始和结束地址
            y1,y2 设置行的起始和结束地址
  返回值：  无
*/
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WriteCom(0x2A);
		LCD_Write16Data(x1+2);
		LCD_Write16Data(x2+2);
		LCD_WriteCom(0x2B);
		LCD_Write16Data(y1+1);
		LCD_Write16Data(y2+1);
		LCD_WriteCom(0x2C);
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WriteCom(0x2A);
		LCD_Write16Data(x1+2);
		LCD_Write16Data(x2+2);
		LCD_WriteCom(0x2B);
		LCD_Write16Data(y1+1);
		LCD_Write16Data(y2+1);
		LCD_WriteCom(0x2C);
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WriteCom(0x2A);
		LCD_Write16Data(x1+1);
		LCD_Write16Data(x2+1);
		LCD_WriteCom(0x2B);
		LCD_Write16Data(y1+2);
		LCD_Write16Data(y2+2);
		LCD_WriteCom(0x2C);
	}
	else
	{
		LCD_WriteCom(0x2A);
		LCD_Write16Data(x1+1);
		LCD_Write16Data(x2+1);
		LCD_WriteCom(0x2B);
		LCD_Write16Data(y1+2);
		LCD_Write16Data(y2+2);
		LCD_WriteCom(0x2C);
	}
}


void LCD_Init(void)
{
	LCD_RES_Clr();//复位
	HAL_Delay(100);
	LCD_RES_Set();
	HAL_Delay(100);
	
	LCD_BLK_Set();//打开背光
	
	HAL_Delay(100);
	LCD_WriteCom(0x11);
	HAL_Delay(100);
	
	//------------------------------------ST7735S Frame Rate-----------------------------------------// 
	LCD_WriteCom (0xB1); 
	LCD_WriteData(0x05); 
	LCD_WriteData(0x3C); 
	LCD_WriteData(0x3C); 
	LCD_WriteCom (0xB2); 
	LCD_WriteData(0x05);
	LCD_WriteData(0x3C); 
	LCD_WriteData(0x3C); 
	LCD_WriteCom (0xB3); 
	LCD_WriteData(0x05); 
	LCD_WriteData(0x3C); 
	LCD_WriteData(0x3C); 
	LCD_WriteData(0x05); 
	LCD_WriteData(0x3C); 
	LCD_WriteData(0x3C); 
	//------------------------------------End ST7735S Frame Rate---------------------------------// 
	LCD_WriteCom (0xB4); //Dot inversion 
	LCD_WriteData(0x03);
	//------------------------------------ST7735S Power Sequence---------------------------------// 
	LCD_WriteCom (0xC0); 
	LCD_WriteData(0x28); 
	LCD_WriteData(0x08); 
	LCD_WriteData(0x04); 
	LCD_WriteCom (0xC1); 
	LCD_WriteData(0XC0); 
	LCD_WriteCom (0xC2); 
	LCD_WriteData(0x0D); 
	LCD_WriteData(0x00); 
	LCD_WriteCom (0xC3); 
	LCD_WriteData(0x8D); 
	LCD_WriteData(0x2A); 
	LCD_WriteCom (0xC4); 
	LCD_WriteData(0x8D); 
	LCD_WriteData(0xEE); 
	//---------------------------------End ST7735S Power Sequence-------------------------------------// 
	LCD_WriteCom(0xC5); //VCOM 
	LCD_WriteData(0x1A); 
	LCD_WriteCom(0x36); //MX, MY, RGB mode 
	if(USE_HORIZONTAL==0)LCD_WriteData(0x00);
	else if(USE_HORIZONTAL==1)LCD_WriteData(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WriteData(0x70);
	else LCD_WriteData(0xA0); 
		//------------------------------------ST7735S Gamma Sequence---------------------------------// 
	LCD_WriteCom(0xE0); 
	LCD_WriteData(0x04); 
	LCD_WriteData(0x22); 
	LCD_WriteData(0x07); 
	LCD_WriteData(0x0A); 
	LCD_WriteData(0x2E); 
	LCD_WriteData(0x30); 
	LCD_WriteData(0x25); 
	LCD_WriteData(0x2A); 
	LCD_WriteData(0x28); 
	LCD_WriteData(0x26); 
	LCD_WriteData(0x2E); 
	LCD_WriteData(0x3A); 
	LCD_WriteData(0x00); 
	LCD_WriteData(0x01); 
	LCD_WriteData(0x03); 
	LCD_WriteData(0x13); 
	LCD_WriteCom(0xE1); 
	LCD_WriteData(0x04); 
	LCD_WriteData(0x16); 
	LCD_WriteData(0x06); 
	LCD_WriteData(0x0D); 
	LCD_WriteData(0x2D); 
	LCD_WriteData(0x26); 
	LCD_WriteData(0x23); 
	LCD_WriteData(0x27); 
	LCD_WriteData(0x27); 
	LCD_WriteData(0x25); 
	LCD_WriteData(0x2D); 
	LCD_WriteData(0x3B); 
	LCD_WriteData(0x00); 
	LCD_WriteData(0x01); 
	LCD_WriteData(0x04); 
	LCD_WriteData(0x13); 
	//------------------------------------End ST7735S Gamma Sequence-----------------------------// 
	LCD_WriteCom(0x3A); //65k mode 
	LCD_WriteData(0x05); 
	LCD_WriteCom(0x29); //Display on 
	LCD_Fill(0,0,128,160,WHITE);
} 


/*
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
*/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_Write16Data(color);
}


/*
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
*/
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		     //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_Write16Data(fc);
				else LCD_Write16Data(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}


/*
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
*/
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}
/*
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
*/
void LCD_ShowIntNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;
	uint8_t sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/*
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
*/
void LCD_ShowFloatNum(uint16_t x,uint16_t y,float num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t t,temp,sizex;
	uint16_t num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}


/*
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
				color       要填充的颜色
      返回值：  无
*/
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_Write16Data(color);
		}
	} 					  	    
}

