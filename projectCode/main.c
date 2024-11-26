#include <STC15F2K60S2.H>
#include "iic.H"
#include "intrins.H"

void display_nixie();
void ledworking();
void pwmjugment();
void display_nixie();
void work_8591();
void relayworking();
void scankey();

unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
unsigned char code duanma_nixie_dot[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};
unsigned char w = 0 , w_p = 40;
unsigned int f = 0 , f_p = 9000;
unsigned int count_f = 0 , count_t = 0;
unsigned char count_relay = 0 , relay_flag = 1 , case5state = 0x00;
unsigned char view1 = 1 , view11 = 1 , view13 = 1 , view14 = 1;
unsigned int len = 0 , len_p = 60;
unsigned char pwmduty = 1 , count_t_pwm = 0;
unsigned char count_led = 0 , ledstate = 0xff;

void delay(unsigned char t)
{
	while(t--);
}

void select573(unsigned char channel)
{
	switch(channel)
	{
		case 0:P2 = (P2 & 0x1f) | 0x00;break;
		case 4:P2 = (P2 & 0x1f) | 0x80;break;
		case 5:P2 = (P2 & 0x1f) | 0xa0;break;
		case 6:P2 = (P2 & 0x1f) | 0xc0;break;
		case 7:P2 = (P2 & 0x1f) | 0xe0;break;
	}
}

void init_sys()
{
	select573(4);
	P0 = 0xff;
	select573(5);
	P0 = 0x00;
	select573(7);
	P0 = 0xff;
}

void work_8591()
{
	unsigned int v = 0;
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x43);
	I2CWaitAck();
	I2CSendByte(((4/(80-w_p))*w+((80-w_p)/(4*w_p)))*51);
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	v = I2CReceiveByte();
	v = v * 100 / 51;
	w = v * 2 / 10;
	I2CSendAck(1);
	I2CStop();
}

void Timer0Init()		//@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD |= 0x04;		//设置定时器模式
	TL0 = 0xff;		//设置定时初值
	TH0 = 0xff;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	EA = 1;
	ET0 = 1;
}

void t0_routine() interrupt 1
{
	count_f++;
}

void Timer2Init()		//200微秒@12.000MHz
{
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0x38;		//设置定时初值
	T2H = 0xFF;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
	EA = 1;
	IE2 |= 0x04;
}

void t2_routine() interrupt 12
{
	count_led++;
	count_t_pwm++;
	if(count_t_pwm <= pwmduty)
	{
		case5state |= 0x20;
		select573(5);
		P0 = case5state;
		select573(0);
	}
	else if(count_t_pwm >= 5)
	{
		case5state &= 0xdf;
		select573(5);
		P0 = case5state;
		select573(0);
		count_t_pwm = 0;
	}
	if(++count_t >= 5000)
	{
		f = count_f;
		count_t = 0;
		count_f = 0;
	}
}

void write_24c02()
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(0x00);
	I2CWaitAck();
	I2CSendByte(count_relay);
	I2CWaitAck();
	I2CStop();
}

void read_24c02()
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(0x00);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	count_relay = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
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

void Timer1Init()		//@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x00;		//设置定时初值
	TH1 = 0x00;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 0;		//定时器暂不计时
}

void sonic()
{
	sendwave();
	TR1 = 1;
	while((TF1 == 0)&&(P11 == 1));
	TR1 = 0;
	if(TF1 == 1)
	{
		len = 999;
		TF1 = 0;
	}
	else
	{
		len = TH1;
		len = (len << 8) | TL1;
		len = 17 * len / 1000;
		TL1 = 0x00;		//设置定时初值
		TH1 = 0x00;		//设置定时初值
	}
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
			display_bit_nixie(0 , ~0x71);
			display_bit_nixie(1 , 0xff);
			if(f>99999)
			{
				display_bit_nixie(2 , ~duanma_nixie[f/100000]);
			}
			else
			{
				display_bit_nixie(2 , 0xff);
			}
			if(f>9999)
			{
				display_bit_nixie(3 , ~duanma_nixie[f%100000/10000]);
			}
			else
			{
				display_bit_nixie(3 , 0xff);
			}
			if(f>999)
			{
				display_bit_nixie(4 , ~duanma_nixie[f%10000/1000]);
			}
			else
			{
				display_bit_nixie(4 , 0xff);
			}
			if(f>99)
			{
				display_bit_nixie(5 , ~duanma_nixie[f%1000/100]);
			}
			else
			{
				display_bit_nixie(5 , 0xff);
			}
			if(f>9)
			{
				display_bit_nixie(6 , ~duanma_nixie[f%100/10]);
			}
			else
			{
				display_bit_nixie(6 , 0xff);
			}
			display_bit_nixie(7 , ~duanma_nixie[f%10]);
		}
		else
		{
			unsigned char add = 0;
			if((f%100) > 50)
				add = 1;
			else
				add = 0;
			display_bit_nixie(0 , ~0x71);
			display_bit_nixie(1 , 0xff);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			if((f/100)>999)
			{
				display_bit_nixie(4 , ~duanma_nixie[((f/100)+add)%10000/1000]);
			}
			else
			{
				display_bit_nixie(4 , 0xff);
			}
			if((f/100)>99)
			{
				display_bit_nixie(5 , ~duanma_nixie[((f/100)+add)%1000/100]);
			}
			else
			{
				display_bit_nixie(5 , 0xff);
			}
			if((f/100)>9)
			{
				display_bit_nixie(6 , ~duanma_nixie_dot[((f/100)+add)%100/10]);
			}
			else
			{
				display_bit_nixie(6 , ~duanma_nixie_dot[0]);
			}
			display_bit_nixie(7 , ~duanma_nixie[((f/100)+add)%10]);
		}
	}
	else if(view1 == 2)
	{
		display_bit_nixie(0 , ~0x76);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		display_bit_nixie(5 , 0xff);
		if(w>9)
		{
			display_bit_nixie(6 , ~duanma_nixie[w/10]);
		}
		else
		{
			display_bit_nixie(6 , 0xff);
		}
		display_bit_nixie(7 , ~duanma_nixie[w%10]);
	}
	else if(view1 == 3)
	{
		display_bit_nixie(0 , ~0x77);
		display_bit_nixie(1 , 0xff);
		display_bit_nixie(2 , 0xff);
		display_bit_nixie(3 , 0xff);
		display_bit_nixie(4 , 0xff);
		if(view13 == 1)
		{
			if(len>99)
			{
				display_bit_nixie(5 , ~duanma_nixie[len%1000/100]);
			}
			else
			{
				display_bit_nixie(5 , 0xff);
			}
			if(len>9)
			{
				display_bit_nixie(6 , ~duanma_nixie[len%100/10]);
			}
			else
			{
				display_bit_nixie(6 , 0xff);
			}
			display_bit_nixie(7 , ~duanma_nixie[len%10]);
		}
		else
		{
			display_bit_nixie(5 , ~duanma_nixie_dot[len%1000/100]);
			display_bit_nixie(6 , ~duanma_nixie[len%100/10]);
			display_bit_nixie(7 , ~duanma_nixie[len%10]);
		}
	}
	else
	{
		if(view14 == 1)
		{
			display_bit_nixie(0 , ~0x73);
			display_bit_nixie(1 , ~duanma_nixie[1]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			if((f_p/100)>99)
			{
				display_bit_nixie(5 , ~duanma_nixie[((f_p/100))%1000/100]);
			}
			else
			{
				display_bit_nixie(5 , 0xff);
			}
			if((f_p/100)>9)
			{
				display_bit_nixie(6 , ~duanma_nixie_dot[((f_p/100))%100/10]);
			}
			else
			{
				display_bit_nixie(6 , ~duanma_nixie_dot[0]);
			}
			display_bit_nixie(7 , ~duanma_nixie[((f_p/100))%10]);
		}
		else if(view14 == 2)
		{
			display_bit_nixie(0 , ~0x73);
			display_bit_nixie(1 , ~duanma_nixie[2]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			display_bit_nixie(5 , 0xff);
			if(w_p>9)
			{
				display_bit_nixie(6 , ~duanma_nixie[w_p/10]);
			}
			else
			{
				display_bit_nixie(6 , 0xff);
			}
			display_bit_nixie(7 , ~duanma_nixie[w_p%10]);
		}
		else
		{
			display_bit_nixie(0 , ~0x73);
			display_bit_nixie(1 , ~duanma_nixie[3]);
			display_bit_nixie(2 , 0xff);
			display_bit_nixie(3 , 0xff);
			display_bit_nixie(4 , 0xff);
			display_bit_nixie(5 , 0xff);
			display_bit_nixie(6 , ~duanma_nixie_dot[len_p%1000/100]);
			display_bit_nixie(7 , ~duanma_nixie[len_p%100/10]);
		}
	}
}

void relayworking()
{
	if(len>len_p)
	{
		case5state |= 0x10;
		select573(5);
		P0 = case5state;
		select573(0);
		if(relay_flag == 1)
		{
			count_relay++;
			relay_flag = 0;
			write_24c02();
		}
	}
	else
	{
		case5state &= 0xef;
		select573(5);
		P0 = case5state;
		select573(0);
		relay_flag = 1;
	}
}

void ledworking()
{
	if(view1 == 1)
	{
		ledstate &= 0xfe;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else if((view1 == 4) && (view14 == 1))
	{
		if(count_led >= 200)
		{
			count_led = 0;
			if((ledstate & 0x01) == 0x00)
			{
				ledstate |= 0x01;
				select573(4);
				P0 = ledstate;
				select573(0);
			}
			else
			{
				ledstate &= 0xfe;
				select573(4);
				P0 = ledstate;
				select573(0);
			}
		}
	}
	else
	{
		ledstate |= 0x01;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(view1 == 2)
	{
		ledstate &= 0xfd;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else if((view1 == 4) && (view14 == 2))
	{
		if(count_led >= 200)
		{
			count_led = 0;
			if((ledstate & 0x02) == 0x00)
			{
				ledstate |= 0x02;
				select573(4);
				P0 = ledstate;
				select573(0);
			}
			else
			{
				ledstate &= 0xfd;
				select573(4);
				P0 = ledstate;
				select573(0);
			}
		}
	}
	else
	{
		ledstate |= 0x02;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(view1 == 3)
	{
		ledstate &= 0xfb;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else if((view1 == 4) && (view14 == 3))
	{
		if(count_led >= 200)
		{
			count_led = 0;
			if((ledstate & 0x04) == 0x00)
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
		}
	}
	else
	{
		ledstate |= 0x04;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(f > f_p)
	{
		ledstate &= 0xf7;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate |= 0x08;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(w > w_p)
	{
		ledstate &= 0xef;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate |= 0x10;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	if(len > len_p)
	{
		ledstate &= 0xdf;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
	else
	{
		ledstate |= 0x20;
		select573(4);
		P0 = ledstate;
		select573(0);
	}
}

void pwmjugment()
{
	if(f>f_p)
		pwmduty = 4;
	else
		pwmduty = 1;
}

void scankey()
{
	if(P30 == 0)//S7
	{
		delay(100);
		if(P30 == 0)
		{
			if(view1 == 1)
			{
				if(view11 == 1)
					view11 = 2;
				else
					view11 = 1;
				while(P30 == 0)															//////
				{
					display_nixie();
					work_8591();
				}
			}
			else if(view1 == 2)
			{
				count_relay = 0;
				write_24c02();
				work_8591();
				display_nixie();														////
				while(P30 == 0)															//////
				{
					display_nixie();
					work_8591();
				}
			}
			else if(view1 == 4)
			{
				if(view14 == 1)
				{
					if(f_p > 1000)
						f_p -= 500;
					else
						f_p = 12000;
				}
				else if(view14 == 2)
				{
					if(w_p > 10)
						w_p -= 10;
					else
						w_p = 60;
				}
				else
				{
					if(len_p > 10)
						len_p -= 10;
					else
						len_p = 120;
				}
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
				if(view13 == 1)
					view13 = 2;
				else
					view13 = 1;
			}
			else if(view1 == 4)
			{
				if(view14 == 1)
				{
					if(f_p < 12000)
						f_p += 500;
					else
						f_p = 1000;
				}
				else if(view14 == 2)
				{
					if(w_p < 60)
						w_p += 10;
					else
						w_p = 10;
				}
				else
				{
					if(len_p < 120)
						len_p += 10;
					else
						len_p = 10;
				}
			}
			while(P31 == 0)															//////
			{
				display_nixie();
				work_8591();
			}
		}
	}
	if(P32 == 0) //S5
	{
		delay(100);
		if(P32 == 0)
		{
			if(view1 == 4)
			{
				if(view14 == 1)
					view14 = 2;
				else if(view14 == 2)
					view14 = 3;
				else
					view14 = 1;
			}
			while(P32 == 0)															//////
			{
				display_nixie();
				work_8591();
			}
		}
	}
	if(P33 == 0) //S4
	{
		delay(100);
		if(P33 == 0)
		{
			if(view1 == 1)
				view1 = 2;
			else if(view1 == 2)
				view1 = 3;
			else if(view1 == 3)
				view1 = 4;
			else
				view1 = 1;
			while(P33 == 0)															//////
			{
				display_nixie();
				work_8591();
			}
		}
	}
}

void main()
{
	Timer0Init();
	Timer1Init();
	Timer2Init();
	init_sys();
	read_24c02();
	while(1)
	{
		unsigned char i = 10;
		sonic();
		while(i--)
		{
			ledworking();
			pwmjugment();
			display_nixie();
			work_8591();
			relayworking();
			scankey();
		}
	}
}