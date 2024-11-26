#include <STC15F2K60S2.H>
#include "ds1302.h"
#include "onewire.h"
#include "iic.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned char code wt_addr[] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
unsigned char code init_time[] = {0x50,0x59,0x16,0x06,0x06,0x02,0x23};
unsigned char h = 0 , m = 0 , s = 0 , ledstate = 0xff , led3 = 1 , flag = 1 , count = 0;
unsigned char view = 1 , view1 = 1 , view2 = 1;
unsigned int temp = 0 , rd1 = 0;
char h_p = 17 , t_p = 25 , l_p = 4 , h_p_w = 0 , t_p_w = 0 , l_p_w = 0;

void select573(unsigned char y)
{
	switch(y)
	{
		case 0:P2 = (P2&0x1f) | 0x00;break;
		case 4:P2 = (P2&0x1f) | 0x80;break;
		case 5:P2 = (P2&0x1f) | 0xa0;break;
		case 6:P2 = (P2&0x1f) | 0xc0;break;
		case 7:P2 = (P2&0x1f) | 0xe0;break;
	}
}

void delay(unsigned char t)
{
	while(t--);
}

void display_bit_nixie(unsigned char pos , unsigned char dat)
{
	select573(6);
	P0 = 0x01 << pos;
	select573(7);
	P0 = dat;
	delay(500);
	select573(7);
	P0 = 0xff;
	select573(0);
}

void wt_1302()
{
	unsigned char i=0;
	Write_Ds1302_Byte( 0x8e,0x00 );
	for(;i<7;i++)
	{
		Write_Ds1302_Byte( wt_addr[i],init_time[i] );
	}
	Write_Ds1302_Byte( 0x8e,0x80 );
}

void rd_1302()
{
	h = Read_Ds1302_Byte ( 0x85 );
	m = Read_Ds1302_Byte ( 0x83 );
	s = Read_Ds1302_Byte ( 0x81 );
}

void rd_18b20()
{
	unsigned char LSB , MSB;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	LSB = Read_DS18B20();
	MSB = Read_DS18B20();
	temp = MSB;
	temp = (temp<<8) | LSB;
	temp = (temp >> 4)*10;
	temp += (LSB & 0x0f) * 0.625;
}

void rd_8591()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x01);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	rd1 = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	rd1 = rd1 * 100 / 51;
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
	if(++count >= 60)
	{
		led3 = flag;
		count = 0;
		TR0 = 0;
	}
}

void jgm_rd1()
{
	if(flag == 1)
	{
		if(rd1<=60)
		{
			TL0 = 0xB0;		//设置定时初值
			TH0 = 0x3C;		//设置定时初值
			count = 0;
			flag = 0;
			TR0 = 1;
		}
	}
	else
	{
		if(rd1>60)
		{
			TL0 = 0xB0;		//设置定时初值
			TH0 = 0x3C;		//设置定时初值
			count = 0;
			flag = 1;
			TR0 = 1;
		}
	}
}

void display_nixie()
{
	if(view == 1)
	{
		if(view1 == 1)
		{
			display_bit_nixie(0 , ~duanma_nixie[h/16]);
			display_bit_nixie(1 , ~duanma_nixie[h%16]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , ~duanma_nixie[m/16]);
			display_bit_nixie(4 , ~duanma_nixie[m%16]);
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , ~duanma_nixie[s/16]);
			display_bit_nixie(7 , ~duanma_nixie[s%16]);
		}
		else if(view1 == 2)
		{
			display_bit_nixie(0 , ~duanma_nixie[15]);
			display_bit_nixie(1 , 0xff);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			display_bit_nixie(5 , ~duanma_nixie[temp/100]);
			display_bit_nixie(6 , ~duanma_nixie_dot[temp%100/10]);
			display_bit_nixie(7 , ~duanma_nixie[temp%10]);
		}
		else
		{
			display_bit_nixie(0 , ~0x76);
			display_bit_nixie(1 , 0xff);
			display_bit_nixie(2 , ~duanma_nixie_dot[rd1/100]);
			display_bit_nixie(3 , ~duanma_nixie[rd1%100/10]);
			display_bit_nixie(4 , ~duanma_nixie[rd1%10]);
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , 0xff);
			if(rd1 > 60)
				display_bit_nixie(7 , ~duanma_nixie[0]);
			else
				display_bit_nixie(7 , ~duanma_nixie[1]);
		}
	}
	else
	{
		if(view2 == 1)
		{
			display_bit_nixie(0 , ~duanma_nixie[5]);
			display_bit_nixie(1 , ~duanma_nixie[4]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , ~duanma_nixie[(h_p + h_p_w)/10]);
			display_bit_nixie(7 , ~duanma_nixie[(h_p + h_p_w)%10]);
		}
		else if(view2 == 2)
		{
			display_bit_nixie(0 , ~duanma_nixie[5]);
			display_bit_nixie(1 , ~duanma_nixie[5]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , ~duanma_nixie[(t_p + t_p_w)/10]);
			display_bit_nixie(7 , ~duanma_nixie[(t_p + t_p_w)%10]);
		}
		else
		{
			display_bit_nixie(0 , ~duanma_nixie[5]);
			display_bit_nixie(1 , ~duanma_nixie[6]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , 0xff);
			display_bit_nixie(7 , ~duanma_nixie[l_p + l_p_w]);
		}
	}
}

void ledworking()
{
	if(((h<8))||((((h/16)*10)+(h%16))>=h_p))
	{
		ledstate &= 0xfe;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate |= 0x01;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(temp<(t_p*10))
	{
		ledstate &= 0xfd;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate |= 0x02;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(led3 == 1)
	{
		ledstate |= 0x04;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate &= 0xfb;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(flag == 0)
	{
		if(l_p == 4)
		{
			ledstate = (ledstate | 0xf8) & 0xf7;
			select573(4);
			P0 = ledstate;
			select573(0);
		}
		else if(l_p == 5)
		{
			ledstate = (ledstate | 0xf8) & 0xef;
			select573(4);
			P0 = ledstate;
			select573(0);
		}
		else if(l_p == 6)
		{
			ledstate = (ledstate | 0xf8) & 0xdf;
			select573(4);
			P0 = ledstate;
			select573(0);
		}
		else if(l_p == 7)
		{
			ledstate = (ledstate | 0xf8) & 0xbf;
			select573(4);
			P0 = ledstate;
			select573(0);
		}
		else
		{
			ledstate = (ledstate | 0xf8) & 0x7f;
			select573(4);
			P0 = ledstate;
			select573(0);
		}
	}
	else
	{
		ledstate |= 0xf8;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
}

void scankey()
{
	P44 = 0;
	P32 = P42 = P33 = 1;
	if(P33 == 0)				//S4
	{
		delay(100);
		if(P33 == 0)
		{
			if(view == 1)
			{
				view = 2;
				view1 = 1;
			}
			else
			{
				view = 1;
				h_p += h_p_w;
				t_p += t_p_w;
				l_p += l_p_w;
				h_p_w = 0;
				t_p_w = 0;
				l_p_w = 0;
				view2 = 1;
			}
			while(P33 == 0)
			{
				rd_18b20();
				rd_1302();
				display_nixie();
			}
		}
	}
	if(P32 == 0)				//S5
	{
		delay(100);
		if(P32 == 0)
		{
			if(view == 1)
			{
				if(view1 == 1)
					view1 = 2;
				else if(view1 == 2)
					view1 = 3;
				else
					view1 = 1;
			}
			else
			{
				if(view2 == 1)
					view2 = 2;
				else if(view2 == 2)
					view2 = 3;
				else
					view2 = 1;
			}
			while(P32 == 0)
			{
				rd_18b20();
				rd_1302();
				display_nixie();
			}
		}
	}
	
	P42 = 0;
	P32 = P44 = P33 = 1;
	if(P33 == 0)				//S8
	{
		delay(100);
		if(P33 == 0)
		{
			if(view2 == 1)
			{
				if((h_p + h_p_w) > 0)
					h_p_w--;
			}
			else if(view2 == 2)
			{
				if((t_p + t_p_w) > 0)
					t_p_w--;
			}
			else
			{
				if((l_p + l_p_w) > 4)
					l_p_w--;
			}
			while(P33 == 0)
			{
				rd_18b20();
				rd_1302();
				display_nixie();
			}
		}
	}
	if(P32 == 0)				//S9
	{
		delay(100);
		if(P32 == 0)
		{
			if(view2 == 1)
			{
				if((h_p + h_p_w) < 23)
					h_p_w++;
			}
			else if(view2 == 2)
			{
				if((t_p + t_p_w) < 99)
					t_p_w++;
			}
			else
			{
				if((l_p + l_p_w) < 8)
					l_p_w++;
			}
			while(P32 == 0)
			{
				rd_18b20();
				rd_1302();
				display_nixie();
			}
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
	select573(0);
	wt_1302();
	Timer0Init();
}

void main()
{
	init_sys();
	while(1)
	{
		jgm_rd1();
		rd_18b20();
		rd_1302();
		display_nixie();
		scankey();
		rd_8591();
		ledworking();
	}
}