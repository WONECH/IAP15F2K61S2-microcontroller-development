/********************************************************************************************************************
                                             2020第十一届第二场省赛真题

题目本地地址："D:\1竞赛\蓝桥杯\题目\蓝桥杯试题-省赛\第11届省赛单片机设计与开发（第二部分1）-温度采集与输出.pdf"
********************************************************************************************************************/
/********************************************************************************************************************
                                                   心得体会
    属于比较简单的题，记得如果数码管最后一位额外的亮的话，可以在数码管显示函数的最后把数码管全部关闭。
********************************************************************************************************************/

#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned int t = 0;
unsigned char view = 1;//页面，1为数据页面，2为参数页面
unsigned char param = 1;//选择要调整的参数，1为tmin，2为tmax
unsigned int tmax = 30;
unsigned int tmin = 20;
unsigned int flag1 = 0 , flag2 = 0;//参数加减标记，实现退出参数页面后调整的参数才生效的效果,flag1服务于tmin，flag2服务于tmax
unsigned char error1 = 0 , error2 = 0;//操作错误标志符，0为正常操作，1为错误操作。error1代表参数越界，error2代表参数逻辑错误，即tmax<tmin

void select573(unsigned char y)
{
	switch(y)
	{
		case 4 : P2 = (P2 & 0x1f) | 0x80 ; break ;
		case 5 : P2 = (P2 & 0x1f) | 0xa0 ; break ;
		case 6 : P2 = (P2 & 0x1f) | 0xc0 ; break ;
		case 7 : P2 = (P2 & 0x1f) | 0xe0 ; break ;
	}
}

void initsys()
{
	select573(4);
	P0 = 0xff;
	select573(5);
	P0 = 0x00;
	select573(7);
	P0 = 0xff;
}

void delay_nixie(unsigned int t)
{
	while(t--);
}

void display_bit_nixie(unsigned char pos , unsigned char value)
{
	select573(7);
	P0 = 0xff;
	select573(6);
	P0 = 0x01 << pos;
	select573(7);
	P0 = value;
	delay_nixie(1500);
}

void display_nixie()
{
	if(view == 1)
	{
		display_bit_nixie(0 , ~0x39);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , 0xff);
		display_bit_nixie(6 , ~duanma_nixie[(t%100)/10]);
		display_bit_nixie(7 , ~duanma_nixie[t%10]);
		select573(7);
		P0 = 0xff;	
	}
	else
	{
		display_bit_nixie(0 , ~0x73);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , ~duanma_nixie[(tmax + flag2)/10]);
		display_bit_nixie(4 , ~duanma_nixie[(tmax + flag2)%10]);
		display_bit_nixie(5 , 0xff);
		display_bit_nixie(6 , ~duanma_nixie[(tmin + flag1)/10]);
		display_bit_nixie(7 , ~duanma_nixie[(tmin + flag1)%10]);
		select573(7);
		P0 = 0xff;
	}
}

void read_18b20()
{
	unsigned char MSB , LSB;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB = Read_DS18B20();
	MSB = Read_DS18B20();
	init_ds18b20();
	
	t = MSB;
	t = (t<<8) | LSB;
	
	t = t>>4;
}

void scan_key()
{
	if(P33 == 0)                //s4
	{
		delay_nixie(100);
		if(P33 == 0)
		{
			if(view == 1)
				view = 2;
			else
			{
				view = 1;
				if((tmin+ flag1)<=(tmax+flag2))
				{
					tmin = tmin + flag1;
					tmax = tmax + flag2;
				}
				flag1 = 0;
				flag2 = 0;
				error1 = 0;
				error2 = 0;
			}
			while(P33 == 0)
			{
				display_nixie();
			}
		}
	}
	
	if(P32 == 0)               //s5
	{
		delay_nixie(100);
		if(P32 == 0)
		{
			if(param == 1)
				param = 2;
			else
				param = 1;
			while(P32 == 0)
			{
				display_nixie();
			}
		}
	}
	
	if(P31 == 0)               //s6
	{
		delay_nixie(100);
		if(P31 == 0)
		{
			if(param == 1)
			{
				if((tmin + flag1) == 99)
				{
					error1 = 1;
				}
				else
				{
					error1 = 0;
					flag1++;
				}
			}
			else
			{
				if((tmax + flag2) == 99)
				{
					error1 = 1;
				}
				else
				{
					error1 = 0;
					flag2++;
				}
			}
			if((tmin + flag1)>(tmax + flag2))
				error2 = 1;
			else
				error2 = 0;
			while(P31 == 0)
			{
				display_nixie();
			}
		}
	}

		if(P30 == 0)               //s7
	{
		delay_nixie(100);
		if(P30 == 0)
		{
			if(param == 1)
			{
				if((tmin + flag1) == 0)
				{
					error1 = 1;
				}
				else
				{
					error1 = 0;
					flag1--;
				}
			}
			else
			{
				if((tmax + flag2) == 0)
				{
					error1 = 1;
				}
				else
				{
					error1 = 0;
					flag2--;
				}
			}
			if((tmin + flag1)>(tmax + flag2))
				error2 = 1;
			else
				error2 = 0;
			while(P30 == 0)
			{
				display_nixie();
			}
		}
	}
}

void LEDworking()
{
	if(t>tmax)
	{
		select573(4);
		P0 &= 0xfe;
	}
	else
	{
		select573(4);
		P0 |= 0x01;
	}
	if(t<=tmax && t>=tmin)
	{
		select573(4);
		P0 &= 0xfd;
	}
	else
	{
		select573(4);
		P0 |= 0x02;
	}
	if(t<tmin)
	{
		select573(4);
		P0 &= 0xfb;
	}
	else
	{
		select573(4);
		P0 |= 0x04;
	}
	if(error1 == 1 || error2 == 1)
	{
		select573(4);
		P0 &= 0xf7;
	}
	else
	{
		select573(4);
		P0 |= 0x08;
	}
}

void write8591()
{
	unsigned int dat = 0;
	
	IIC_Start(); 
	IIC_SendByte(0x90);
	IIC_WaitAck(); 
	IIC_SendByte(0x40);
	IIC_WaitAck(); 
	if(t>tmax)
		dat = 4 * 51;
	else if(t < tmin)
		dat = 2 * 51;
	else
		dat = 3 * 51;
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

void main()
{
	initsys();
	while(1)
	{
		write8591();
		read_18b20();
		display_nixie();
		scan_key();
		LEDworking();
	}
}