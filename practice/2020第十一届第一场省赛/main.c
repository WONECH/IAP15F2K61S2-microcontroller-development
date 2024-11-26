/********************************************************************************************************************
                                             2020第十一届第一场省赛真题

题目本地地址："D:\1竞赛\蓝桥杯\题目\蓝桥杯试题-省赛\第11届省赛单片机设计与开发（第二部分2）.pdf"
********************************************************************************************************************/
/********************************************************************************************************************
                                                   心得体会
    属于比较简单的题。用到了AT24c02。
********************************************************************************************************************/

#include <STC15F2K60S2.H>
#include "iic.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
float vain3 = 0;
int vain3_int1 = 0;
unsigned char view = 1;
float vp = 0;
int vp_int1 = 0;
unsigned int count = 0;
unsigned char count_state = 0;//用于作为count是否需要加1的判据
unsigned char LED_state = 0xff;
unsigned char count_t = 0;//定时计数
unsigned char count_f = 0;//无效按键操作计数

void select_573(unsigned char y)
{
	switch(y)
	{
		case 4:P2 = (P2 & 0x1f) | 0x80;break;
		case 5:P2 = (P2 & 0x1f) | 0xa0;break;
		case 6:P2 = (P2 & 0x1f) | 0xc0;break;
		case 7:P2 = (P2 & 0x1f) | 0xe0;break;
	}
}

void read_24c02()
{
	IIC_Start(); 
	IIC_SendByte(0xa0); 
	IIC_WaitAck(); 
	IIC_SendByte(0x00); 
	IIC_WaitAck(); 
	IIC_Stop(); 
	
	IIC_Start(); 
	IIC_SendByte(0xa1); 
	IIC_WaitAck(); 
	vp = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
}

void init_sys()
{
	select_573(4);
	P0 = 0xff;
	select_573(7);
	P0 = 0xff;
	select_573(5);
	P0 = 0x00;
	read_24c02();
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

void read_8592()
{
	IIC_Start(); 
	IIC_SendByte(0x90); 
	IIC_WaitAck(); 
	IIC_SendByte(0x03); 
	IIC_WaitAck(); 
	IIC_Stop(); 
	
	IIC_Start(); 
	IIC_SendByte(0x91); 
	IIC_WaitAck(); 
	vain3 = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	vain3 = vain3 / 51;
}

void display_nixie()
{
	if(view == 1)
	{
		display_bit_nixie(0 , ~0x3E);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		vain3_int1 = vain3 * 100;
		display_bit_nixie(5 , ~duanma_nixie_dot[vain3_int1 / 100]);
		display_bit_nixie(6 , ~duanma_nixie[(vain3_int1 % 100) / 10]);
		display_bit_nixie(7 , ~duanma_nixie[vain3_int1 % 10]);
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
		vp_int1 = vp * 10;
		display_bit_nixie(5 , ~duanma_nixie_dot[vp_int1 / 100]);
		display_bit_nixie(6 , ~duanma_nixie[(vp_int1 % 100) / 10]);
		display_bit_nixie(7 , ~duanma_nixie[vp_int1 % 10]);
		select_573(7);
		P0 = 0xff;
	}
	else
	{
		display_bit_nixie(0 , ~0x37);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		if(count > 99)
		{
			display_bit_nixie(5 , ~duanma_nixie_dot[count / 100]);
			display_bit_nixie(6 , ~duanma_nixie[(count % 100) / 10]);
			display_bit_nixie(7 , ~duanma_nixie[count % 10]);
		}
		else if(count > 9)
		{
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , ~duanma_nixie[count / 10]);
			display_bit_nixie(7 , ~duanma_nixie[count % 10]);
		}
		else
		{
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , 0xff);
			display_bit_nixie(7 , ~duanma_nixie[count]);
		}
		select_573(7);
		P0 = 0xff;
	}
}

void change_count()
{
	if((vain3 * 10) > vp)
	{
		count_state = 1;
		TR0 = 1;
	}
	else
	{
		if(count_state)
			count++;
		count_state = 0;
		TR0 = 0;
	}
}

void Timer0Init()		//50毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xB0;		//设置定时初值
	TH0 = 0x3C;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0开始计时
	EA = 1;
	ET0 = 1;
}

void t0_routine() interrupt 1
{
	if(++count_t == 100)
	{
		LED_state = LED_state & 0xfe;
		select_573(4);
		P0 = LED_state;
		count_t = 0;
		TR0 = 0;
	}
}

void LEDworking()
{
	if(!count_state)
	{
		LED_state = LED_state | 0x01;
		select_573(4);
		P0 = LED_state;
	}
	if(count % 2 == 0)
	{
		LED_state = LED_state | 0x02;
		select_573(4);
		P0 = LED_state;
	}
	else
	{
		LED_state = LED_state & 0xfd;
		select_573(4);
		P0 = LED_state;
	}
	if(count_f >= 3)
	{
		LED_state = LED_state & 0xfb;
		select_573(4);
		P0 = LED_state;
	}
	else
	{
		LED_state = LED_state | 0x04;
		select_573(4);
		P0 = LED_state;
	}
}

void write_24c02()
{
	IIC_Start(); 
	IIC_SendByte(0xa0); 
	IIC_WaitAck(); 
	IIC_SendByte(0x00); 
	IIC_WaitAck(); 
	IIC_SendByte(vp); 
	IIC_WaitAck(); 
	IIC_Stop(); 
}

void scankey()
{
	P32=P33=P34=1;
	P35 = 0;
	if(P33 == 0)					//S12
	{
		delay_nixie(100);
		if(P33 == 0)
		{
			if(view == 1)
			{
				count_f = 0;
				view = 2;
			}
			else if(view == 2)
			{
				write_24c02();
				view = 3;
				count_f = 0;
			}
			else
			{
				count_f = 0;
				view = 1;
			}
			while(P33 == 0)
			{
				read_8592();
				display_nixie();
				LEDworking();
				change_count();
			}
		}
	}
	if(P32 == 0)					//S13
	{
		delay_nixie(100);
		if(P32 == 0)
		{
			if(view == 3)
			{
				count_f = 0;
				count = 0;
			}
			else
				count_f++;
			while(P32 == 0)
			{
				read_8592();
				display_nixie();
				LEDworking();
				change_count();
			}
		}
	}
	
	P32=P33=P35=1;
	P34 = 0;
	if(P33 == 0)					//S16
	{
		delay_nixie(100);
		if(P33 == 0)
		{
			if(view == 2)
			{
				vp += 5;
				count_f = 0;
				if(vp > 50)
				{
					vp = 0;
				}
			}
			else
				count_f++;
			while(P33 == 0)
			{
				read_8592();
				display_nixie();
				LEDworking();
				change_count();
			}
		}
	}
	if(P32 == 0)					//S17
	{
		delay_nixie(100);
		if(P32 == 0)
		{
			if(view == 2)
			{
				count_f = 0;
				vp -= 5;
				if(vp < 0)
				{
					vp =50;
				}
			}
			else
				count_f++;
			while(P32 == 0)
			{
				read_8592();
				display_nixie();
				LEDworking();
				change_count();
			}
		}
	}
}

void main()
{
	init_sys();
	Timer0Init();
	while(1)
	{
		read_8592();
		display_nixie();
		LEDworking();
		change_count();
		scankey();
	}
}