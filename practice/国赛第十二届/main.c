//没写完，有事儿，不写了
#include <STC15F2K60S2.H>
#include "iic.h"
#include "ds1302.h"
#include "intrins.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned char rd_addr[] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
unsigned char wt_addr[] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
unsigned char t = 0 , m = 0 , s = 0;
unsigned char view1 = 1 , view11 = 1 , view12 = 1 , view112 = 1 , view113 = 1;
unsigned int dist = 0 , dist_max = 0 , dist_min = 0 , dist_m = 0 , dist_count = 0 , dist_all = 0;
unsigned char change = 0;
unsigned char t_p = 2 , dist_p = 20 , dist_p_s = 0 , t_p_s = 0;
unsigned char rb1 = 0 , flag = 0;

void delay(unsigned char t)
{
	while(t--);
}

void select573(unsigned char y)
{
	switch(y)
	{
		case 0:P2 = (P2 & 0x1f) | 0x00;break;
		case 4:P2 = (P2 & 0x1f) | 0x80;break;
		case 5:P2 = (P2 & 0x1f) | 0xa0;break;
		case 6:P2 = (P2 & 0x1f) | 0xc0;break;
		case 7:P2 = (P2 & 0x1f) | 0xe0;break;
	}
}

void display_bit_nixie(unsigned char pos , unsigned char value)
{
	select573(6);
	P0 = 0x01 << pos;
	select573(7);
	P0 = value;
	delay(500);
	select573(7);
	P0 = 0xff;
	select573(0);
}

void init_1302()
{	
	unsigned char j = 0;
	Write_Ds1302_Byte(0x8e,0x80);
	for(;j<7;j++)
	{
		Write_Ds1302_Byte(wt_addr[j],0x00);
	}
	Write_Ds1302_Byte(0x8e,0x00);
}

void read1302()
{
	t = Read_Ds1302_Byte ( 0x85 );
	m = Read_Ds1302_Byte ( 0x83 );
	s = Read_Ds1302_Byte ( 0x81 );
	t = ((t/16) * 10) + (t%16);
	m = ((m/16) * 10) + (m%16);
	s = ((s/16) * 10) + (s%16);
}

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

void sendwave()
{
	unsigned char i = 0;
	for(;i<8;i++)
	{
		P10 = 1;
		Delay12us();
		P10 = 0;
		Delay12us();
	}
}

void Timer0Init()		//100微秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x9C;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		//定时器0暂停计时
	EA = 1;
	ET0 = 0;
}

void sonic()
{
	sendwave();
	TR0 = 1;
	while((TF0 == 0) || (P11 == 1));
	TR0 = 0;
	if(TF0 == 1)
	{
		TF0 = 0;
		dist = 999;
	}
	else
	{
		dist = TH0;
		dist = (dist << 8) | TL0;
		dist = 17 * dist / 1000;
	}
}

void sonicworking()
{
	if(view112 == 1)
		if(change == 1)
		{
			change = 0;
			sonic();
			dist_count++;
			dist_all += dist;
			dist_m = dist_all * 10 / dist_count;
			if(dist_max <= dist)
				dist_max = dist;
			if(dist_min >= dist)
				dist_min = dist;
		}
	else
		if((s % t_p) == 0)
		{
			sonic();
			dist_count++;
			dist_all += dist;
			dist_m = dist_all * 10 / dist_count;
			if(dist_max <= dist)
				dist_max = dist;
			if(dist_min >= dist)
				dist_min = dist;
		}
}

void read8591()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x01);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	rb1 = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	if(view112 == 1)
	{
		if(rb1 < 70)
		{
			if(flag == 1)
			{
				change = 1;
				flag = 0;
			}
		}
		else
		{
			flag = 1;
		}
	}
}

void write8591()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x41);
	I2CWaitAck();
	I2CSendByte(((4*dist+50)*51)/70);
	I2CWaitAck();
	I2CStop();
}

void display_nixie()
{
	if(view1 == 1)
	{
		if(view11 == 1)
		{
			display_bit_nixie(0 , ~duanma_nixie[t/10]);
			display_bit_nixie(1 , ~duanma_nixie[t%10]);
			display_bit_nixie(2 , 0xbf);
			display_bit_nixie(3 , ~duanma_nixie[m/10]);
			display_bit_nixie(4 , ~duanma_nixie[m%10]);
			display_bit_nixie(5 , 0xbf);
			display_bit_nixie(6 , ~duanma_nixie[t/10]);
			display_bit_nixie(7 , ~duanma_nixie[t%10]);
		}
		else if(view11 == 2)
		{
			display_bit_nixie(0 , ~0x38);
			if(view112 == 1)
			{
				display_bit_nixie(1 , ~duanma_nixie[12]);
			}
			else
			{
				display_bit_nixie(1 , ~duanma_nixie[15]);
			}
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			if(dist > 99)
			{
				display_bit_nixie(5 , ~duanma_nixie[dist/100]);
			}
			else
			{
				display_bit_nixie(5 , 0xff);
			}
			if(dist > 9)
			{
				display_bit_nixie(6 , ~duanma_nixie[dist%100/10]);
			}
			else
			{
				display_bit_nixie(6 , 0xff);
			}
			display_bit_nixie(7 , ~duanma_nixie[t%10]);
		}
		else
		{
			display_bit_nixie(0 , ~0x76);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			if(view113 == 1)
			{
				display_bit_nixie(1 , 0xfe);
				if(dist_max>999)
					display_bit_nixie(4 , ~duanma_nixie[dist_max/1000]);
				else
					display_bit_nixie(4 , 0xff);
				if(dist_max>99)
					display_bit_nixie(5 , ~duanma_nixie[dist_max%1000/100]);
				else
					display_bit_nixie(5 , 0xff);
				if(dist_max>9)
					display_bit_nixie(6 , ~duanma_nixie[dist_max%100/10]);
				else
					display_bit_nixie(6 , 0xff);
				display_bit_nixie(7 , ~duanma_nixie[dist_max%10]);
			}
			else if(view113 == 2)
			{
				display_bit_nixie(1 , 0xbf);
				if(dist_m>999)
					display_bit_nixie(4 , ~duanma_nixie[dist_m/1000]);
				else
					display_bit_nixie(4 , 0xff);
				if(dist_m>99)
					display_bit_nixie(5 , ~duanma_nixie[dist_m%1000/100]);
				else
					display_bit_nixie(5 , 0xff);
				display_bit_nixie(6 , ~duanma_nixie_dot[dist_m%100/10]);
				display_bit_nixie(7 , ~duanma_nixie[dist_m%10]);
			}
			else
			{
				display_bit_nixie(1 , 0xf7);
				if(dist_min>999)
					display_bit_nixie(4 , ~duanma_nixie[dist_min/1000]);
				else
					display_bit_nixie(4 , 0xff);
				if(dist_min>99)
					display_bit_nixie(5 , ~duanma_nixie[dist_min%1000/100]);
				else
					display_bit_nixie(5 , 0xff);
				if(dist_min>9)
					display_bit_nixie(6 , ~duanma_nixie[dist_min%100/10]);
				else
					display_bit_nixie(6 , 0xff);
				display_bit_nixie(7 , ~duanma_nixie[dist_min%10]);
			}
		}
	}
	else
	{
		display_bit_nixie(0 , ~0x73);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , 0xff);
		if(view12 == 1)
		{
			display_bit_nixie(1 , ~duanma_nixie[1]);
			display_bit_nixie(6 , ~duanma_nixie[t_p_s/10]);
			display_bit_nixie(7 , ~duanma_nixie[t_p_s%10]);
		}
		else
		{
			display_bit_nixie(1 , ~duanma_nixie[2]);
			display_bit_nixie(6 , ~duanma_nixie[dist_p_s/10]);
			display_bit_nixie(7 , ~duanma_nixie[dist_p_s%10]);
		}
	}
}

void init_sys()
{
	select573(4);
	P0 = 0xff;
	select573(7);
	P0 = 0xff;
	select573(5);
	P0 = 0x00;
	init_1302();
	Timer0Init();
}

void scankey()
{
	P44 = 0;
	P42 = P33 = P32 = 1;
	if(P33 == 0)						//s4
	{
		delay(100);
		if(P33 == 0)
		{
			if(view1 == 1)
				view1 = 2;
			else
			{
				view1 = 1;
				t_p = t_p_s;
				dist_p = dist_p_s;
			}
			while(P33 == 0)
			{
				read1302();
				sonicworking();
				read8591();
				write8591();
				display_nixie();
			}
		}
	}
	if(P32 == 0)						//s5
	{
		delay(100);
		if(P32 == 0)
		{
			if(view1 == 1)
			{
				if(view11 == 1)
					view11 = 2;
				else if(view11 == 2)
					view11 = 3;
				else
					view11 = 1;
			}
			else
			{
				if(view12 == 1)
				{
					view12 = 2;
				}
				else
				{
					view12 = 1;
				}
			}
			while(P32 == 0)
			{
				read1302();
				sonicworking();
				read8591();
				write8591();
				display_nixie();
			}
		}
	}
	
	P42 = 0;
	P44 = P33 = P32 = 1;
	if(P33 == 0)						//s8
	{
		delay(100);
		if(P33 == 0)
		{
			if((view1 = 1) && (view11 == 2))
			{
				if(view112 == 1)
					view112 = 2;
				else
					view112 = 1;
			}
			else if((view1 = 1) && (view11 == 3))
			{
				if(view113 == 1)
					view113 = 2;
				else if(view113 == 2)
					view113 = 3;
				else
					view113 = 1;
			}
			while(P33 == 0)
			{
				read1302();
				sonicworking();
				read8591();
				write8591();
				display_nixie();
			}
		}
	}
	if(P32 == 0)						//s9
	{
		delay(100);
		if(P32 == 0)
		{
			if(view1 == 2)
			{
				if(view12 == 1)
				{
					if(t_p_s == 2)
						t_p_s = 3;
					else if(t_p_s == 9)
						t_p_s = 2;
					else
						t_p_s += 2;
				}
				else
				{
					if(dist_p_s == 80)
						dist_p_s = 10;
					else
						dist_p_s += 10;
				}
			}
			while(P32 == 0)
			{
				read1302();
				sonicworking();
				read8591();
				write8591();
				display_nixie();
			}
		}
	}
}

void main()
{
	init_sys();
	while(1)
	{
		read1302();
		sonicworking();
		read8591();
		write8591();
		display_nixie();
		scankey();
	}
}