#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"
#include "intrins.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned int vain3 = 0;
unsigned int temp = 0;
unsigned int count_f = 0 , count_t = 0;
unsigned int f = 0;
unsigned char view1 = 1;
unsigned char view11 = 1;
unsigned int kp_vain3 = 1;
unsigned int kp_f = 1;
unsigned int kp_temp = 1;
unsigned char ledstate = 0xff;
unsigned char f_L = 0 , temp_L = 0; //用于读取24c02的频率的低八位和温度的低八位
unsigned char param = 0;
unsigned char count_t2 = 0;

void ledworking();

void delay(unsigned char t)
{
	while(t--);
}

void select573(unsigned char channel)
{
	switch(channel)
	{
		case 0 : P2 = (P2 & 0x1f) | 0x00;break;
		case 4 : P2 = (P2 & 0x1f) | 0x80;break;
		case 5 : P2 = (P2 & 0x1f) | 0xa0;break;
		case 6 : P2 = (P2 & 0x1f) | 0xc0;break;
		case 7 : P2 = (P2 & 0x1f) | 0xe0;break;
	}
}

void init_sys()
{
	select573(4);
	P0 = 0xff;
	select573(5);
	P0 = 0x00;
	select573(0);
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

void display_nixie()
{
	if(view1 == 1)
	{
		if(view11 == 1)
		{
			display_bit_nixie(0, ~0x3E);
			display_bit_nixie(1, 0xff);
			display_bit_nixie(2, 0xff);
			display_bit_nixie(3, 0xff);
			display_bit_nixie(4, 0xff);
			display_bit_nixie(5, 0xff);
			display_bit_nixie(6, ~duanma_nixie_dot[vain3/10]);
			display_bit_nixie(7, ~duanma_nixie[vain3%10]);
		}
		else if(view11 == 2)
		{
			display_bit_nixie(0, ~0x71);
			display_bit_nixie(1, 0xff);
			display_bit_nixie(2, 0xff);
			if(f > 9999)
			{
				display_bit_nixie(3, ~duanma_nixie[f/10000]);
			}
			else
				display_bit_nixie(3, 0xff);
			if(f > 999)
			{
				display_bit_nixie(4, ~duanma_nixie[(f%10000)/1000]);
			}
			else
				display_bit_nixie(4, 0xff);
			if(f > 99)
			{
				display_bit_nixie(5, ~duanma_nixie[(f%1000)/100]);
			}
			else
				display_bit_nixie(5, 0xff);
			if(f > 9)
			{
				display_bit_nixie(6, ~duanma_nixie[(f%100)/10]);
			}
			else
				display_bit_nixie(6, 0xff);
			display_bit_nixie(7, ~duanma_nixie[f%10]);
		}
		else
		{
			display_bit_nixie(0, ~0x39);
			display_bit_nixie(1, 0xff);
			display_bit_nixie(2, 0xff);
			display_bit_nixie(3, 0xff);
			if(temp > 999)
			{
				display_bit_nixie(4, ~duanma_nixie[temp/1000]);
			}
			else
				display_bit_nixie(4, 0xff);
			display_bit_nixie(5, ~duanma_nixie_dot[temp%1000/100]);
			display_bit_nixie(6, ~duanma_nixie[temp%100/10]);
			display_bit_nixie(7, ~duanma_nixie[temp%10]);
		}
	}

	else if(view1 == 2)
	{
		if(view11 == 1)
		{
			display_bit_nixie(0, ~0x76);
			display_bit_nixie(1, ~0x3E);
			display_bit_nixie(2, 0xff);
			display_bit_nixie(3, 0xff);
			display_bit_nixie(4, 0xff);
			display_bit_nixie(5, 0xff);
			display_bit_nixie(6, ~duanma_nixie_dot[kp_vain3/10]);
			display_bit_nixie(7, ~duanma_nixie[kp_vain3%10]);
		}
		else if(view11 == 2)
		{
			display_bit_nixie(0, ~0x76);
			display_bit_nixie(1, ~0x71);
			display_bit_nixie(2, 0xff);
			if(f > 9999)
			{
				display_bit_nixie(3, ~duanma_nixie[kp_f/10000]);
			}
			else
				display_bit_nixie(3, 0xff);
			if(f > 999)
			{
				display_bit_nixie(4, ~duanma_nixie[(kp_f%10000)/1000]);
			}
			else
				display_bit_nixie(4, 0xff);
			if(f > 99)
			{
				display_bit_nixie(5, ~duanma_nixie[(kp_f%1000)/100]);
			}
			else
				display_bit_nixie(5, 0xff);
			if(f > 9)
			{
				display_bit_nixie(6, ~duanma_nixie[(kp_f%100)/10]);
			}
			else
				display_bit_nixie(6, 0xff);
			display_bit_nixie(7, ~duanma_nixie[kp_f%10]);
		}
		else
		{
			display_bit_nixie(0, ~0x76);
			display_bit_nixie(1, ~0x39);
			display_bit_nixie(2, 0xff);
			display_bit_nixie(3, 0xff);
			if(kp_temp > 999)
			{
				display_bit_nixie(4, ~duanma_nixie[kp_temp/1000]);
			}
			else
				display_bit_nixie(4, 0xff);
			display_bit_nixie(5, ~duanma_nixie_dot[kp_temp%1000/100]);
			display_bit_nixie(6, ~duanma_nixie[kp_temp%100/10]);
			display_bit_nixie(7, ~duanma_nixie[kp_temp%10]);
		}
	}
		else
	{
		display_bit_nixie(0, ~0x73);
		display_bit_nixie(1, 0xff);
		display_bit_nixie(2, 0xff);
		display_bit_nixie(3, 0xff);
		display_bit_nixie(4, 0xff);
		display_bit_nixie(5, 0xff);
		display_bit_nixie(6, ~duanma_nixie_dot[param/10]);
		display_bit_nixie(7, ~duanma_nixie[param%10]);
	}
}

void read_18B20()
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
	temp = (temp << 8) | LSB;
	temp = (( temp >> 4 ) * 100) + ((LSB & 0x0f) * 6.25);
}

void read_8591()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x03);
	I2CWaitAck();
	display_nixie();
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	vain3 = I2CReceiveByte();
	vain3 = vain3*10 / 51;
	I2CSendAck(1);
	I2CStop();
}

void Timer0Init()
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x06;		//设置定时器模式
	TL0 = 0xff;		//设置定时初值
	TH0 = 0xff;		//设置定时重载值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	EA = 1;
	ET0 = 1;
}

void t0_routine() interrupt 1
{
	count_f++ ;
}

void Timer1Init()		//50毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xB0;		//设置定时初值
	TH1 = 0x3C;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	EA = 1;
	ET1 = 1;
}

void t1_routine() interrupt 3
{
	if(++count_t >= 20)
	{
		f = count_f;
		count_f = 0;
		count_t = 0;
	}
}

void write_24c02(unsigned char addr , unsigned int dat)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}

int read_24c02(unsigned char addr)
{
	unsigned char dat = 0;
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	display_nixie();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	dat = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return dat;
}

void delay_24c02(unsigned int t)
{
	while(t--)
	{
		read_18B20();
		read_8591();
		display_nixie();
	}
}

void Delay1ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 12;
	j = 169;
	do
	{
		read_18B20();
		read_8591();
		display_nixie();
		ledworking();
		while (--j);
	} while (--i);
}

void scankey()
{
	if(P33 == 0) //S4
	{
		delay(100);
		if(P33 == 0)
		{
			if(view1 == 1 || view1 == 2)
			{
				if(view11 == 1)
				{
					view11 = 2;
				}
				else if(view11 == 2)
				{
					view11 = 3;
				}
				else
				{
					view11 = 1;
				}
				while(P33 == 0)
				{
					read_18B20();
					read_8591();
					display_nixie();
					ledworking();
				}
			}
		}
	}
	if(P32 == 0) //S5
	{
		delay(100);
		if(P32 == 0)
		{
			write_24c02(0x01 , vain3);
			delay_24c02(1);
			write_24c02(0x02 , f);
			delay_24c02(1);
			write_24c02(0x03 , (f>>8));
			delay_24c02(1);
			write_24c02(0x04 , temp);
			delay_24c02(1);
			write_24c02(0x05 , (temp>>8));
			delay_24c02(1);
			kp_vain3 =vain3;
			kp_f =f;
			kp_temp =temp;
			while(P32 == 0)
			{
				read_18B20();
				read_8591();
				display_nixie();
				ledworking();
			}
		}
	}
	if(P31 == 0) //S6
	{
		delay(100);
		if(P31 == 0)
		{
			if(view1 == 3)
			{
				unsigned char i = 0;
				if(param >= 50)
					param = 1;
				else
					param++;
				while(P31 == 0)
				{
					i++;
					Delay1ms();
					if(i > 5)
					{
						if(param >= 50)
							param = 1;
						else
							param++;
					}
				}
			}
			else 
			{
				if(view1 == 1)
				{
					view1 = 2;
				}
				else
				{
					view1 = 1;
				}
				while(P31 == 0)
				{
					read_18B20();
					read_8591();
					display_nixie();
					ledworking();
				}
			}
		}
	}
		if(P30 == 0) //S7
	{
		delay(100);
		if(P30 == 0)
		{
			if(view1 == 3)
			{
				view1 = 1;
			}
			else
			{
				view1 = 3;
			}
			while(P30 == 0)
			{
				read_18B20();
				read_8591();
				display_nixie();
				ledworking();
			}
		}
	}
}

void Timer2Init()		//50毫秒@12.000MHz
{
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0xB0;		//设置定时初值
	T2H = 0x3C;		//设置定时初值
	AUXR &= 0xef;		//定时器2开始计时
	EA = 1;
	IE2 |= 0x04;
}

void t2_routine() interrupt 12
{
	if(++count_t2 >= 16)
	{
		if((ledstate & 0x80) == 0x00)
		{
			ledstate |= 0x80;
		}
		else
		{
			ledstate &= 0x7f;
		}
	}
}

void ledworking()
{
	if((view1 == 1)&&(view11 == 3))
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
	if((view1 == 1)&&(view11 == 2))
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
	if((view1 == 1)&&(view11 == 1))
	{
		ledstate &= 0xfb;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate |= 0x04;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(vain3 > param)
	{
		AUXR |= 0x10;
	}
	else
	{
		AUXR &= 0xef;
		count_t2 = 0;
		ledstate |= 0x80;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
}

void main()
{
	Timer2Init();
	init_sys();
	Timer0Init();
	Timer1Init();
	kp_vain3 = read_24c02(0x01);
	kp_f = read_24c02(0x03);
	f_L = read_24c02(0x02);
	kp_f = (kp_f << 8) | f_L;
	kp_temp = read_24c02(0x05);
	temp_L = read_24c02(0x04);
	kp_temp = (kp_temp << 8) | temp_L;
	while(1)
	{
		read_18B20();
		read_8591();
		display_nixie();
		scankey();
		ledworking();
	}
}