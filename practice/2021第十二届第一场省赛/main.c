/************************************************************************************************************
																											题目要求
												                   见第十二届单片机设计与开发项目省赛(2021)
本地文件地址："D:\1竞赛\蓝桥杯\题目\蓝桥杯试题-省赛\第12届省赛单片机设计与开发（第二部分1）.pdf"
************************************************************************************************************/
/************************************************************************************************************
																										    感受
第五套真题，感觉这套巨简单。。。但DAC输出电压没有检验是否正确，没万用表。
************************************************************************************************************/
#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned int temp = 0;//温度
unsigned char view = 1;//页面
unsigned char param = 25;//温度参数
unsigned char vad = 0;//DAC输出的电压值
unsigned char mode = 1;//模式
int add = 0;//累加计数，用于实现“等待退出参数页面参数设置才生效”的效果
unsigned char LED_state = 0xff;//记录LED状态

void select_573(unsigned char channel)
{
	switch(channel)
	{
		case 4: P2 = (P2 & 0x1f) | 0x80;break;
		case 5: P2 = (P2 & 0x1f) | 0xa0;break;
		case 6: P2 = (P2 & 0x1f) | 0xc0;break;
		case 7: P2 = (P2 & 0x1f) | 0xe0;break;
	}
}

void init_sys()
{
	select_573(4);
	P0 = 0xff;
	select_573(5);
	P0 = 0x00;
	select_573(7);
	P0 = 0xff;
}

void delay_nixie(unsigned char t)
{
	while(t--);
}

void display_bit_nixie(unsigned char pos , unsigned char value)
{
	select_573(7);
	P0 = 0xff;
	select_573(6);
	P0 = 0x01 << pos;
	select_573(7);
	P0 = value;
	delay_nixie(500);
}

void read_18b20()
{
	unsigned char LSB = 0 , MSB = 0;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB = Read_DS18B20();
	MSB = Read_DS18B20();
	init_ds18b20();
	
	temp = MSB;
	temp = (temp << 8) | LSB;
	
	temp = ((temp >> 4) * 100) + ((LSB & 0x0f) * 6.25);
}

void display_nixie()
{
	if(view == 1)
	{
		display_bit_nixie(0 , ~0x39);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , ~duanma_nixie[temp / 1000]);
		display_bit_nixie(5 , ~duanma_nixie_dot[(temp % 1000) / 100]);
		display_bit_nixie(6 , ~duanma_nixie[(temp % 100) / 10]);
		display_bit_nixie(7 , ~duanma_nixie[temp % 10]);
		select_573(7);
		P0 = 0xff;
	}
	else if(view == 2)
	{
		display_bit_nixie(0 , ~0x73);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , 0xff);
		display_bit_nixie(6 , ~duanma_nixie[(param + add) / 10]);
		display_bit_nixie(7 , ~duanma_nixie[(param + add) % 10]);
		select_573(7);
		P0 = 0xff;
	}
	else
	{
		display_bit_nixie(0 , ~0x77);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , ~duanma_nixie_dot[vad / 100]);
		display_bit_nixie(6 , ~duanma_nixie[(vad % 100) / 10]);
		display_bit_nixie(7 , ~duanma_nixie[vad % 10]);
		select_573(7);
		P0 = 0xff;
	}
}

void dac()
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x40);
	IIC_WaitAck();
	if(mode == 1)
	{
		if(temp < param)
			IIC_SendByte(0);
		else
			IIC_SendByte(255);
	}
	else
	{
		if(temp <= 2000)
			IIC_SendByte(51);
		else if(temp > 4000)
			IIC_SendByte(4 * 51);
		else
			IIC_SendByte((((3 * 51) / 2000) * temp) - (2 * 51));
	}
	IIC_WaitAck();
	IIC_Stop();
}

void LEDworking()
{
	if(mode == 1)
	{
		select_573(4);
		P0 = LED_state;
		P00 = 0;
		LED_state = P0;
	}
	else
	{
		select_573(4);
		P0 = LED_state;
		P00 = 1;
		LED_state = P0;
	}
	if(view == 1)
	{
		select_573(4);
		P0 = LED_state;
		P01 = 0;
		LED_state = P0;
	}
	else
	{
		select_573(4);
		P0 = LED_state;
		P01 = 1;
		LED_state = P0;
	}
	if(view == 2)
	{
		select_573(4);
		P0 = LED_state;
		P02 = 0;
		LED_state = P0;
	}
	else
	{
		select_573(4);
		P0 = LED_state;
		P02 = 1;
		LED_state = P0;
	}
	if(view == 3)
	{
		select_573(4);
		P0 = LED_state;
		P03 = 0;
		LED_state = P0;
	}
	else
	{
		select_573(4);
		P0 = LED_state;
		P03 = 1;
		LED_state = P0;
	}
}

void scan_key()
{
	P44 = 0;
	P42 = P33 = P32 = 1;
	if(P33 == 0)              //S4
	{
		delay_nixie(100);
		if(P33 == 0)
		{
			if(view == 1)
				view = 2;
			else if(view == 2)
			{
				view = 3;
				param += add;
			}
			else
				view = 1;
			while(P33 == 0)
			{
				read_18b20();
				display_nixie();
				dac();
				LEDworking();
			}
		}
	}
	if(P32 == 0)              //S5
	{
		delay_nixie(100);
		if(P32 == 0)
		{
			if(mode == 1)
				mode = 2;
			else
				mode = 1;
			while(P32 == 0)
			{
				read_18b20();
				display_nixie();
				dac();
				LEDworking();
			}
		}
	}
	
	P42 = 0;
	P44 = P33 = P32 = 1;
	if(P33 == 0)              //S8
	{
		delay_nixie(100);
		if(P33 == 0)
		{
			if(view == 2)
			{
				add--;
			}
			while(P33 == 0)
			{
				read_18b20();
				display_nixie();
				dac();
				LEDworking();
			}
		}
	}
	if(P32 == 0)              //S9
	{
		delay_nixie(100);
		if(P32 == 0)
		{
			if(view == 2)
			{
				add++;
			}
			while(P32 == 0)
			{
				read_18b20();
				display_nixie();
				dac();
				LEDworking();
			}
		}
	}
}

void main()
{
	init_sys();
	while(1)
	{
		read_18b20();
		display_nixie();
		scan_key();
		dac();
		LEDworking();
	}
}