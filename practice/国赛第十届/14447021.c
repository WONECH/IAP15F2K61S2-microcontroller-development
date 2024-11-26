#include <STC15F2K60S2.H>
#include "iic.h"
#include "onewire.h"
#include "intrins.h"
#include "stdio.h"
void DAC_output();
unsigned char duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned int temp = 0 , distance = 0 , count_chg = 0;
unsigned char view = 1 , view1 = 1 , view2 = 1 , t_p = 30 , d_p = 35 , count_L = 0 , count_H = 0 , TFlag = 0 , DAC = 1 , count = 0 , count_t = 0 ;
char t_p_w = 0 , d_p_w = 0;
unsigned char datnum[8]	, nixie_i = 0 , count_nixie = 0, count_sonic = 0;

void delay(unsigned char t)
{
	while(t--);
}

void select573(unsigned char channel)
{
	switch(channel)
	{
		case 0: P2 = (P2 & 0x1f) | 0x00;break;
		case 4: P2 = (P2 & 0x1f) | 0x80;break;
		case 5: P2 = (P2 & 0x1f) | 0xa0;break;
		case 6: P2 = (P2 & 0x1f) | 0xc0;break;
		case 7: P2 = (P2 & 0x1f) | 0xe0;break;
	}
}

void display_bit_nixie(unsigned char pos , unsigned char value)
{
	select573(7);
	P0 = 0xff;
	select573(6);
	P0 = 0x01 << pos;
	select573(7);
	P0 = value;
	select573(0);
}

void rd18b20()
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
	temp = MSB;
	temp = (temp<<8) | LSB;
	temp = (temp>>4) * 100;
	temp += (LSB & 0x0f) * 6.25;
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

void Timer0Init()		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x18;		//设置定时初值
	TH0 = 0xFC;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		
	EA = 1;
	ET0 = 1;
}

void t0_routine() interrupt 1
{
	display_bit_nixie(nixie_i , datnum[nixie_i]);
	if(++nixie_i >= 8) nixie_i = 0;
	count_sonic++;
}

void sonic()
{
	CMOD = 0x00;
	CL = 0x00;		//设置定时初值
	CH = 0x00;		//设置定时初值
	sendwave();
	CR = 1;
	while((TF0 == 0) && (P11 == 1));
	CR = 0;
	if(CF == 1)
	{
		distance = 999;
		CF = 0;
	}
	else
	{
		distance = (CH<<8) | CL;
		distance = distance*0.017;
	}
}

void display_nixie()
{
	if(view == 1)
	{
		if(view1 == 1)
		{
			datnum[0] = ~duanma_nixie[12];
			datnum[1] = 0xff;
			datnum[2] = 0xff;
			datnum[3] = 0xff;
			datnum[4] = ~duanma_nixie[temp/1000];
			datnum[5] = ~duanma_nixie_dot[temp%1000/100];
			datnum[6] = ~duanma_nixie[temp%100/10];
			datnum[7] = ~duanma_nixie[temp%10];
		}
		else if(view1 == 2)
		{
			datnum[0] = ~0x38;
			datnum[1] = 0xff;
			datnum[2] = 0xff;
			datnum[3] = 0xff;
			datnum[4] = 0xff;
			datnum[5] = 0xff;
			datnum[6] = ~duanma_nixie[distance/10];
			datnum[7] = ~duanma_nixie[distance%10];
		}
		else
		{
			datnum[0] = ~0x67;
			datnum[1] = 0xff;
			datnum[2] = 0xff;
			if(count_chg>9999)
				datnum[3] = ~duanma_nixie[count_chg/10000];
			else
				datnum[3] = 0xff;
			if(count_chg>999)
				datnum[4] = ~duanma_nixie[count_chg%10000/1000];
			else
				datnum[4] = 0xff;
			if(count_chg>99)
				datnum[5] = ~duanma_nixie[count_chg%1000/100];
			else
				datnum[5] = 0xff;
			if(count_chg>9)
				datnum[6] = ~duanma_nixie[count_chg%100/10];
			else
				datnum[6] = 0xff;
			datnum[7] = ~duanma_nixie[count_chg%10];
		}
	}
	else
	{
		if(view2 == 1)
		{
			datnum[0] = ~0x73;
			datnum[1] = 0xff;
			datnum[2] = 0xff;
			datnum[3] = ~duanma_nixie[1];
			datnum[4] = 0xff;
			datnum[5] = 0xff;
			datnum[6] = ~duanma_nixie[(t_p+t_p_w)/10];
			datnum[7] = ~duanma_nixie[(t_p+t_p_w)%10];
		}
		else
		{
			datnum[0] = ~0x73;
			datnum[1] = 0xff;
			datnum[2] = 0xff;
			datnum[3] = ~duanma_nixie[2];
			datnum[4] = 0xff;
			datnum[5] = 0xff;
			datnum[6] = ~duanma_nixie[(d_p+d_p_w)/10];
			datnum[7] = ~duanma_nixie[(d_p+d_p_w)%10];
		}
	}
}

void rd24c02()
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(0x01);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	count_L = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(0x02);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	count_chg = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	count_chg = (count_chg << 8) | count_L;
}

void wt24c02()
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(0x01);
	I2CWaitAck();
	I2CSendByte(count_L);
	I2CWaitAck();
	I2CStop();
	
	rd18b20();
	display_nixie();
	sonic();
	DAC_output();
	
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(0x02);
	I2CWaitAck();
	I2CSendByte(count_H);
	I2CWaitAck();
	I2CStop();
}

void Timer1Init()		//50毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xB0;		//设置定时初值
	TH1 = 0x3C;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 0;
	EA = 1;
	ET1 = 1;
}

void t1_routine() interrupt 3
{
	if(++count >= 20)
	{
		count = 0;
		TFlag = 1;
	}
}

void dac(unsigned char output)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x40);
	I2CWaitAck();
	I2CSendByte(output);
	I2CWaitAck();
	I2CStop();
}

void DAC_output()
{
	if(DAC == 1)
	{
		if(distance > d_p)
			dac(4*51);
		else
			dac(2*51);
	}
	else
	{
		dac(20);//0.39V
	}
}

void scankey()
{
	P35=0;
	P32=P33=P34=1;
	if(P32 == 0) //13
	{
		delay(100);
		if(P32 == 0)
		{
			TL1 = 0xB0;		//设置定时初值
			TH1 = 0x3C;		//设置定时初值
			count = 0;
			TR1 = 1;
			while((P32 == 0)&&(TFlag == 0))
			{
				rd18b20();
				display_nixie();
				sonic();
				DAC_output();
			}
			TR1 = 0;
			if(TFlag == 0)
			{
				if(view == 1)
				{
					view = 2;
					view1 = 1;
				}
				else
				{
					view = 1;
					view2 = 1;
					if((d_p_w != 0) || (t_p_w !=0))
					{
						d_p+=d_p_w;
						t_p+=t_p_w;
						d_p_w = 0;
						t_p_w = 0;
						count_chg++;
						count_H = (count_chg>>8);
						count_L = (count_chg & 0x00ff);
						wt24c02();
					}
				}
			}
			else
			{
				TFlag = 0;
				if(DAC == 0)
					DAC = 1;
				else
					DAC = 0;
			}
		}
	}
	if(P33 == 0) //12
	{
		delay(100);
		if(P33 == 0)
		{
			TL1 = 0xB0;		//设置定时初值
			TH1 = 0x3C;		//设置定时初值
			count = 0;
			TR1 = 1;
			while((P33 == 0)&&(TFlag == 0))
			{
				rd18b20();
				display_nixie();
				sonic();
				DAC_output();
			}
			TR1 = 0;
			if(TFlag == 0)
			{
				if(view == 1)
				{
					if(view1 == 1)
					{
						view1 = 2;
					}
					else if(view1 == 2)
					{
						view1 = 3;
					}
					else
					{
						view1 = 1;
					}
				}
				else
				{
					if(view2 == 1)
					{
						view2 = 2;
					}
					else
					{
						view2 = 1;
					}
				}
			}
			else
			{
				TFlag = 0;
				count_chg = 0;
				wt24c02();
			}
		}
	}
	P34=0;
	P32=P33=P35=1;
	if(P32 == 0) //17
	{
		delay(100);
		if(P32 == 0)
		{
			if(view == 2)
			{
				if(view2 == 1)
				{
					if((t_p+t_p_w+2) <= 99)
					{
						t_p_w += 2;
					}
				}
				else
				{
					if((d_p+d_p_w+5) <= 99)
					{
						d_p_w += 5;
					}
				}
			}
			while(P32 == 0)
			{
				rd18b20();
				display_nixie();
				sonic();
				DAC_output();
			}
		}
	}
	if(P33 == 0) //16
	{
		delay(100);
		if(P33 == 0)
		{
			if(view == 2)
			{
				if(view2 == 1)
				{
					if((t_p+t_p_w-2) >= 0)
					{
						t_p_w -= 2;
					}
				}
				else
				{
					if((d_p+d_p_w-5) >= 0)
					{
						d_p_w -= 5;
					}
				}
			}
			while(P33 == 0)
			{
				rd18b20();
				display_nixie();
				sonic();
				DAC_output();
			}
		}
	}
}

void UartInit()		//4800bps@12.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0x8F;		//设定定时初值
	T2H = 0xFD;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
	EA = 1;
	ES = 1;
}

void init_sys()
{
	select573(7);
	P0 = 0xff;
	select573(4);
	P0 = 0xff;
	select573(5);
	P0 = 0x00;
	Timer0Init();
	Timer1Init();
	rd24c02();
}

void main()
{
	init_sys();
	while(1)
	{
		rd18b20();
		display_nixie();
		if(count_sonic >= 200)
		{
			sonic();
			count_sonic = 0;
		}
		DAC_output();
		scankey();
	}
}