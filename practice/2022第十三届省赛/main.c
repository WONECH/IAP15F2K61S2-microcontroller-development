/************************************************************************************************************
																											题目要求
												                   见第十三届单片机设计与开发项目省赛(2022)
本地文件地址："D:\1竞赛\蓝桥杯\题目\蓝桥杯试题-省赛\第13届省赛单片机设计与开发（第二部分）.pdf"
************************************************************************************************************/
/************************************************************************************************************
																										    注意
做题时间太零散，最后不想继续做下去了，蜂鸣器部分没做完，L3亮灭0.1s也有问题。
************************************************************************************************************/

#include <STC15F2K60S2.H>
#include <ds1302.H>
#include <onewire.H>

unsigned char code duanma_Nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_Nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned char time[] = {0x55,0x59,0x23,0x12,0x03,0x07,0x23};//ds1302初始化时间
unsigned char rd_addr[] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};//ds1302写地址
unsigned char wt_addr[] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};//ds1302读地址
unsigned int temp = 0; //实时温度
unsigned char tparam = 23; //温度参数
unsigned char view = 1; //界面
unsigned char mode = 1; //模式，1为温度控制模式，2为时间控制模式
unsigned char t_h = 0 , t_m = 0 , t_s = 0;
unsigned char LED_state = 0xff; //记录LED的P0
unsigned char relay_state = 0x00; //记录继电器的P0
unsigned char count0 = 0 , count1 = 0; //定时器计数
unsigned char relay_switch = 1;  //记录继电器的开关状态

void select_573(unsigned char y)
{
	switch(y)
	{
		case 4: P2 = (P2 & 0x1f) | 0x80	;break;
		case 5: P2 = (P2 & 0x1f) | 0xa0	;break;
		case 6: P2 = (P2 & 0x1f) | 0xc0	;break;
		case 7: P2 = (P2 & 0x1f) | 0xe0	;break;
	}
}

void init_sys()
{
	select_573(7);
	P0 = 0xff;
	select_573(4);
	P0 = 0xff;
	select_573(5);
	P0 = 0x00;
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

void rd_18b20()
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
	temp = (temp << 8) | LSB ;
	temp = ( temp >> 4 ) * 10;
	temp += (LSB & 0x0f) * 0.625;
}

void init_1302()
{
	unsigned char i = 0;
	Write_Ds1302_Byte( 0x8e , 0x00 );
	for(;i<7;i++)
	{
		Write_Ds1302_Byte( wt_addr[i] , time[i] );
	}
	Write_Ds1302_Byte( 0x8e , 0x80 );
}

void rd_1302()
{
	t_s = Read_Ds1302_Byte( 0x81 );
	t_m = Read_Ds1302_Byte( 0x83 );
	t_h = Read_Ds1302_Byte( 0x85 );
}

void display_nixie()
{
	if(view == 1)
	{
		display_bit_nixie(0 , ~0x3E);
		display_bit_nixie(1 , ~duanma_Nixie[view]);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , ~duanma_Nixie[temp/100]);
		display_bit_nixie(6 , ~duanma_Nixie_dot[(temp%100)/10]);
		display_bit_nixie(7 , ~duanma_Nixie[temp%10]);
		select_573(7);
		P0 = 0xff;
	}
	else if(view == 2)
	{
		display_bit_nixie(0 , ~0x3E);
		display_bit_nixie(1 , ~duanma_Nixie[view]);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , ~duanma_Nixie[t_h/16]);
		display_bit_nixie(4 , ~duanma_Nixie[t_h%16]);
		display_bit_nixie(5 , 0xbf);
		display_bit_nixie(6 , ~duanma_Nixie[t_m/16]);
		display_bit_nixie(7 , ~duanma_Nixie[t_m%16]);
		select_573(7);
		P0 = 0xff;
	}
	else
	{
		display_bit_nixie(0 , ~0x3E);
		display_bit_nixie(1 , ~duanma_Nixie[view]);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , 0xff);
		display_bit_nixie(6 , ~duanma_Nixie[tparam/10]);
		display_bit_nixie(7 , ~duanma_Nixie[tparam%10]);
		select_573(7);
		P0 = 0xff;
	}
}

void delay_key(unsigned char t)
{
	while(t--);
}

void Timer0Init(void)		//50毫秒@12.000MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xB0;		//设置定时初值
	TH0 = 0x3C;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 0;		
	EA = 1;
	ET0 = 1;
}

void T0_routine() interrupt 1
{
	if(++count0 == 100)
	{
		TR0 = 0;
		select_573(4);
		P0 = LED_state | 0x01;
		LED_state = P0;
//		select_573(5);
//		P0 = relay_state & 0xef;
//		relay_state = P0;
//		relay_switch = 0;
		count0 = 0;
	}
}

void Timer1Init()		//10毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xF0;		//设置定时初值
	TH1 = 0xD8;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 0;		
	EA = 1;
	ET1 = 1;
}

void T1_routine() interrupt 3
{
	if(++count1 == 10)
	{
		count1 = 0;
		if((LED_state | 0xf7) == 0xf7)
		{
			select_573(4);
			P0 = LED_state & 0xf7;
			LED_state = P0;
		}
		else
		{
			select_573(4);
			P0 = LED_state | 0x04;
			LED_state = P0;
		}
	}
}

void LED_relay_working()
{
	if(t_m == 0 && t_s == 0)
	{
//		if(mode == 2)
//		{
//			select_573(5);
//			P0 = relay_state | 0x10;
//			relay_state = P0;
//			relay_switch = 1;
//		}
		select_573(4);
		P0 = LED_state & 0xfe;
		LED_state = P0;
		TR0 = 1;
	}
	if(mode == 1)
	{
		select_573(4);
		P0 = LED_state & 0xfd;
		LED_state = P0;
//		if(temp > tparam)
//		{
//			select_573(5);
//			P0 = relay_state | 0x10;
//			relay_state = P0;
//			relay_switch = 1;
//		}
//		else
//		{
//			select_573(5);
//			P0 = relay_state & 0xef;
//			relay_state = P0;
//			relay_switch = 0;
//		}
	}
	else
	{
		select_573(4);
		P0 = LED_state | 0x02;
		LED_state = P0;
	}

	if(relay_switch)
		TR1 = 1;
	else
		TR1 = 0;
}

void scan_key()
{
	P35 = 0;
	P34 = P32 = P33 = 1;
	if(P32 == 0)                //s13
	{
		delay_key(300);
		if(P32 == 0)
		{
			if(mode == 1)
				mode = 2;
			else
				mode = 1;
			while(P32 == 0)
			{
				rd_1302();
				rd_18b20();
				display_nixie();
				LED_relay_working();
			}
		}
	}
	
	if(P33 == 0)                //s12
	{
		delay_key(300);
		if(P33 == 0)
		{
			if(view == 1)
				view = 2;
			else if(view == 2)
				view = 3;
			else
				view = 1;
			while(P33 == 0)
			{
				rd_1302();
				rd_18b20();
				display_nixie();
				LED_relay_working();
			}
		}
	}
	
	P34 = 0;
	P35 = P32 = P33 = 1;
	if(P33 == 0)                //s16
	{
		delay_key(300);
		if(P33 == 0)
		{
			if(view == 3)
			{
				if(tparam < 99)
					tparam++;
			}
			while(P33 == 0)
			{
				rd_1302();
				rd_18b20();
				display_nixie();
				LED_relay_working();
			}
		}
	}

	if(P32 == 0)                //s17
	{
		delay_key(300);
		if(P32 == 0)
		{
			if(view == 3)
			{
				if(tparam > 10)
				{
					tparam--;
					while(P32 == 0)
					{
						rd_1302();
						rd_18b20();
						display_nixie();
						LED_relay_working();
					}
				}
			}
			else if(view == 2)
			{
				while(P32 == 0)
				{
					rd_1302();
					rd_18b20();
					t_h = t_m;
					t_m = t_s;
					display_nixie();
					LED_relay_working();
				}
			}
		}
	}
}

void main()
{
	init_sys();
	init_1302();
	Timer0Init();
	Timer1Init();
	while(1)
	{
		scan_key();
		rd_1302();
		rd_18b20();
		display_nixie();
		LED_relay_working();
	}
}