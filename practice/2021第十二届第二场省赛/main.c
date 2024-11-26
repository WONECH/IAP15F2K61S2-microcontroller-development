/************************************************************************************************************
													题目要求
										见第十二届单片机设计与开发项目省赛(2021)(第二次)
本地文件地址："D:\1竞赛\蓝桥杯\题目\蓝桥杯试题-省赛\第12届省赛单片机设计与开发（第一部分2）.pdf"
************************************************************************************************************/
/************************************************************************************************************
													感受
	用到了NE555，比较简单，还用了之前没用过的定时器2，也并不难。
************************************************************************************************************/

#include <STC15F2K60S2.H>
#include "iic.h"

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned int count_f = 0;//记录接收到的NE555波数
unsigned int f = 0;//记录NE555频率
unsigned char count_t	= 0;//记录定时时间，直到满1s
unsigned char view = 1;//显示界面
unsigned int rd1 = 0 , rb2 = 0;//rd1、rb2电压值
unsigned char ain = 1;//采集电压通道数
unsigned int flash_rb2 = 0 , flash_f = 0;//缓存
unsigned char LEDswitch = 1;//LED开关
unsigned char LEDstate = 0xff;//记录LED状态
unsigned char count_t2 = 0; //T2计数

void select_573(channel)
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
	select_573(6);
	P0 = 0x01 << pos;
	select_573(7);
	P0 = value;
	delay_nixie(500);
	select_573(7);
	P0 = 0xff;
}

void init_t0()
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x04;
	TL0 = 0xFF;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	EA = 1;
	ET0 = 1;
}

void t0_routine() interrupt 1
{
	count_f++;
}

void init_t1() //50ms
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
	if(++count_t == 20)
	{
		count_t = 0;
		f = count_f;
		count_f = 0;
	}
}

void display_nixie()
{
	if(view == 1)
	{
		display_bit_nixie(0 , ~0x71);
		display_bit_nixie(7 , ~duanma_nixie[f%10]);
		if(f>999999)
			display_bit_nixie(1 , ~duanma_nixie[f/1000000]);
		else
			display_bit_nixie(1 , 0xff);
		if(f>99999)
			display_bit_nixie(2 , ~duanma_nixie[f%1000000/100000]);
		else
			display_bit_nixie(2 , 0xff);
		if(f>9999)
			display_bit_nixie(3 , ~duanma_nixie[f%100000/10000]);
		else
			display_bit_nixie(3 , 0xff);
		if(f>999)
			display_bit_nixie(4 , ~duanma_nixie[f%10000/1000]);
		else
			display_bit_nixie(4 , 0xff);
		if(f>99)
			display_bit_nixie(5 , ~duanma_nixie[f%1000/100]);
		else
			display_bit_nixie(5 , 0xff);
		if(f>9)
			display_bit_nixie(6 , ~duanma_nixie[f%100/10]);
		else
			display_bit_nixie(6 , 0xff);
		select_573(6);
		P0 = 0xff;
		select_573(7);
		P2 = 0xff;
	}
	else if(view == 2)
	{
		display_bit_nixie(0 , 0xc8);
		display_bit_nixie(7 , ~duanma_nixie[(1000000/f)%10]);
		if((1000000/f)>999999)
			display_bit_nixie(1 , ~duanma_nixie[(1000000/f)/1000000]);
		else
			display_bit_nixie(1 , 0xff);
		if((1000000/f)>99999)
			display_bit_nixie(2 , ~duanma_nixie[(1000000/f)%1000000/100000]);
		else
			display_bit_nixie(2 , 0xff);
		if((1000000/f)>9999)
			display_bit_nixie(3 , ~duanma_nixie[(1000000/f)%100000/10000]);
		else
			display_bit_nixie(3 , 0xff);
		if((1000000/f)>999)
			display_bit_nixie(4 , ~duanma_nixie[(1000000/f)%10000/1000]);
		else
			display_bit_nixie(4 , 0xff);
		if((1000000/f)>99)
			display_bit_nixie(5 , ~duanma_nixie[(1000000/f)%1000/100]);
		else
			display_bit_nixie(5 , 0xff);
		if((1000000/f)>9)
			display_bit_nixie(6 , ~duanma_nixie[(1000000/f)%100/10]);
		else
			display_bit_nixie(6 , 0xff);
		select_573(6);
		P0 = 0xff;
		select_573(7);
		P2 = 0xff;
	}
	else
	{
		display_bit_nixie(0 , ~0x3E);
		display_bit_nixie(1 , 0xbf);
		display_bit_nixie(2 , ~duanma_nixie[ain]);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		if(ain == 1)
		{
			display_bit_nixie(5 , ~duanma_nixie_dot[rd1 / 100]);
			display_bit_nixie(6 , ~duanma_nixie[rd1 % 100 / 10]);
			display_bit_nixie(7 , ~duanma_nixie[rd1 % 10]);
		}
		else
		{
			display_bit_nixie(5 , ~duanma_nixie_dot[rb2 / 100]);
			display_bit_nixie(6 , ~duanma_nixie[rb2 % 100 / 10]);
			display_bit_nixie(7 , ~duanma_nixie[rb2 % 10]);
		}
		select_573(6);
		P0 = 0xff;
		select_573(7);
		P2 = 0xff;
	}
}

void read_rd1()
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
	rd1 = IIC_RecByte();
	rd1 = (rd1 * (5.0 / 255)) * 100;
	IIC_SendAck(1);
	IIC_Stop();
}

void read_rb2()
{
	IIC_Start();
	IIC_SendByte(0x90); 
	IIC_WaitAck();
	IIC_SendByte(0x01); 
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91); 
	IIC_WaitAck();
	rb2 = IIC_RecByte();
	rb2 = (rb2 * (5.0 / 255)) * 100;
	IIC_SendAck(1);
	IIC_Stop();
}

void delay_key(unsigned char t)
{
	while(t--);
}

void LEDworking()
{
	if(!LEDswitch)
	{
		select_573(4);
		P0 = 0xff;
	}
	else
	{
		if(rb2 > flash_rb2)
		{
			LEDstate &= 0xfe;
			select_573(4);
			P0 =LEDstate;
		}
		else
		{
			LEDstate |= 0x01;
			select_573(4);
			P0 =LEDstate;
		}
		if(f > flash_f)
		{
			LEDstate &= 0xfd;
			select_573(4);
			P0 =LEDstate;
		}
		else
		{
			LEDstate |= 0x02;
			select_573(4);
			P0 =LEDstate;
		}
		if(view == 1)
		{
			LEDstate &= 0xfb;
			select_573(4);
			P0 =LEDstate;
		}
		else
		{
			LEDstate |= 0x04;
			select_573(4);
			P0 =LEDstate;
		}
		if(view == 2)
		{
			LEDstate &= 0xf7;
			select_573(4);
			P0 =LEDstate;
		}
		else
		{
			LEDstate |= 0x08;
			select_573(4);
			P0 =LEDstate;
		}
		if(view == 3)
		{
			LEDstate &= 0xef;
			select_573(4);
			P0 =LEDstate;
		}
		else
		{
			LEDstate |= 0x10;
			select_573(4);
			P0 =LEDstate;
		}
	}
}

void Timer2Init()		//50毫秒@12.000MHz
{
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0xB0;		//设置定时初值
	T2H = 0x3C;		//设置定时初值
	AUXR &= 0xef;		//定时器2暂不计时
	IE2 = 0x04;
}

void t2_routine() interrupt 12
{
	if(++count_t2 == 20)
		LEDswitch = !LEDswitch;
}

void scankey()
{
	if(P33 == 0)                 //s4
	{
		delay_key(100);
		if(P33 == 0)
		{
			if(view == 1)
				view = 2;
			else if(view == 2)
				view = 3;
			else
			{
				view = 1;
				ain = 1;
			}
			while(P33 == 0)
			{
				display_nixie();
				read_rd1();
				read_rb2();
				LEDworking();
			}
		}
	}
	if(P32 == 0)                 //s5
	{
		delay_key(100);
		if(P32 == 0)
		{
			if(view == 3)
			{
				if(ain == 1)
					ain = 3;
				else
					ain = 1;
			}
			while(P32 == 0)
			{
				display_nixie();
				read_rd1();
				read_rb2();
				LEDworking();
			}
		}
	}
	if(P31 == 0)                 //s6
	{
		delay_key(100);
		if(P31 == 0)
		{
			flash_rb2 = rb2 ;
			while(P31 == 0)
			{
				display_nixie();
				read_rd1();
				read_rb2();
				LEDworking();
			}
		}
	}
	if(P30 == 0)                 //s7
	{
		delay_key(100);
		if(P30 == 0)
		{
			flash_f = f ;
			while(P30 == 0)
			{
				AUXR |= 0x10;		//定时器2开始计时
				display_nixie();
				read_rd1();
				read_rb2();
				LEDworking();
			}
			AUXR &= 0xef;		//定时器2停止
			count_t2 = 0;
			T2L = 0xB0;		//设置定时初值
			T2H = 0x3C;		//设置定时初值
		}
	}
}

void main()
{
	init_sys();
	init_t0();
	init_t1();
	Timer2Init();
	while(1)
	{
		display_nixie();
		read_rd1();
		read_rb2();
		scankey();
		LEDworking();
	}
}