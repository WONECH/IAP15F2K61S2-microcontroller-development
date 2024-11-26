/******************************************************************************************************************************************************************************************************************
                                                题目要求
1-基本功能描述

      通过单片机控制8个LED指示灯按照特定的顺序（工作模式）亮灭；指示灯的流转间隔可通过按键调整，亮度可由电位器RB2进行控制；各工作模式的流转间隔时间需在E2PROM中保存，并可在硬件重新上电后，自动载入。

2-设计说明

  <1> 关闭蜂鸣器、继电器等与本试题程序设计无关的外设资源。

  <2> 设备上电后默认数码管、LED指示灯均为熄灭状态。

  <3> 流转间隔可调整范围为400ms-1200ms。

  <4> 设备固定安照模式1、模式2、模式3、模式4的次序循环往复运行。

3-LED指示灯工作模式

  <1> 模式1：按照L1、L2...L8的顺序，从左到右单循环点亮。

  <2> 模式2：按照L8、L7...L1的顺序，从右到左单循环点亮。

  <3> 模式3：
	
	<4> 模式4：

4-亮度等级控制

      检测电位器RB2的输出电压，控制8个LED指示灯的亮度，要求在0V~5V的可调区间内，实现4个均匀分布的LED指示灯亮度等级。

5-按键功能

<1> 按键S7定义为"启动/停止"按键，按下后启动或停止LED的流转。

<2> 按键S6定义为"设置"按键，按键按下后数码管进入"流转间隔"设置界面，如下图所示：

<3> 按键S5定义为"加"按键，在设置界面下，按下该键，若当前选择的是运行模式，则运行模式编号加1，若当前选择的是流转间隔，则流转间隔增加100ms。

  <4> 按键S4定义为"减"按键，在设置界面下，按下该键，若当前选择的是运行模式，则运行模式编号减1，若当前选择的是流转间隔，则流转间隔减少100ms。

  <5> 按键S4、S5的"加"、"减"功能只在"设置状态"下有效，数值的调整应注意边界属性。

  <6> 在非"设置状态"下，按键S4按键可显示指示灯当前的亮度等级，4个亮度等级从暗到亮，依次用数字1、2、3、4表示；松开按键S4，数码管显示关闭，亮度等级的显示格式如下图所示：


————————————————
版权声明：本文为CSDN博主「小蜜蜂老师」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/ohy3686/article/details/87199951
*****************************************************************************************************************************************************************************************************************/




/**********************************************************************************
                                心得经验
第一套真题，感觉好难。。。
1.PWM控制LED在占空比小于100的情况下会闪，可尝试将一个周期分成的份数减小
  本次实验中，一个周期分成100份，PWM控制LED一直闪，久久之后，尝试将一个
	周期分成20份，才解决问题。
2.除了周期分成的份数有限制以外，还有一小份的时间，即timer溢出的时间不
	能太短，即不能太快，否则按键难以识别到，Nixie也会无法动态显示，原因
	应该是程序跳入中断函数的次数太多，占过多时间，导致主函数中的ScanKey
	函数和Nixie动态显示函数无法在较短时间内重复进行。
3.还有一些小但有较大影响的细节在程序中已经标注且注释了，注意浏览。
***********************************************************************************/

#include "iic.h"
#include <STC15F2K60S2.H>
#include "absacc.h"

unsigned char LED_Level = 1 , Nixie_State = 0 , LED_State = 1 , Move_Flag = 0 , count_t1 = 0 , LED_Lock = 0 , LED_Dat , stat = 0;
unsigned int LED_t = 400 , count_t0 = 0 ;
code unsigned char duanma_Nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};

/******************************************************************
                             函数声明
******************************************************************/
void InitSystem();
void delay();
void Display_Bit_Nixie(unsigned char pos , unsigned char dat);
void Write_E2PROM(unsigned char dat);
unsigned char Read_E2PROM();
unsigned char Read_PCF8591();
void Display_Nixie();
void LED_Working();
void Judgment_LED_Level();
/******************************************************************/

void InitSystem()
{
	XBYTE[0x8000] = 0xff;
	XBYTE[0xa000] = 0x00;
	XBYTE[0xc000] = 0xff;
	XBYTE[0xe000] = 0xff;
}

void delay(unsigned int t)
{
	while(t--);
}

void Display_Bit_Nixie(unsigned char pos , unsigned char dat)
{
	XBYTE[0xe000] = 0xff;
	XBYTE[0xc000] = 0x01 << pos;
	XBYTE[0xe000] = dat;
}

void Write_E2PROM(unsigned char dat)
{
	IIC_Start(); 
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(0x01);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

unsigned char Read_E2PROM()
{
	unsigned char dat;
	
	IIC_Start(); 
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(0x01);
	IIC_WaitAck();

	IIC_Start(); 
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(1); 
	IIC_Stop();
	
	return dat;
}

/**************************************************
                数码管显示内容
**************************************************/
void Display_Nixie()
{
	if(Nixie_State == 0)     //数码管不显示
	{
		XBYTE[0xc000] = 0xff;
		XBYTE[0xe000] = 0xff;
	}
	else if(Nixie_State == 1)  //数码管进入"流转间隔"设置界面
	{
		Display_Bit_Nixie(0 , 0xbf);
		delay(100);
		Display_Bit_Nixie(1 , ~duanma_Nixie[LED_State]);
		delay(100);
		Display_Bit_Nixie(2 , 0xbf);
		delay(100);
		Display_Bit_Nixie(3 , 0xff);
		delay(100);
		if(LED_t > 999)
		{
			Display_Bit_Nixie(4 , ~duanma_Nixie[LED_t / 1000]);
			delay(100);
		}
		if(LED_t > 99)
		{
			Display_Bit_Nixie(5 , ~duanma_Nixie[(LED_t % 1000) / 100]);
			delay(100);
		}
		if(LED_t > 9)
		{
			Display_Bit_Nixie(6 , ~duanma_Nixie[(LED_t % 100) / 10]);
			delay(100);
		}
		Display_Bit_Nixie(7 , ~duanma_Nixie[LED_t % 10]);
		delay(100);
	}
	else if(Nixie_State == 2)                                  //数码管显示指示灯当前的亮度等级
	{
		Display_Bit_Nixie(6 , 0xbf);
		delay(100);
		Display_Bit_Nixie(7 , ~duanma_Nixie[LED_Level]);
		delay(100);
	}
	XBYTE[0xc000] = 0xff; //消影
	XBYTE[0xe000] = 0xff;
}

unsigned char Read_PCF8591()
{
	unsigned char dat;
	
	IIC_Start(); 
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x03);
	IIC_WaitAck();
	IIC_Stop();

	IIC_Start(); 
	IIC_SendByte(0x91);
	IIC_WaitAck();
	dat = IIC_RecByte();
	IIC_SendAck(1); 
	IIC_Stop();
	
	return dat;
}

/*********************************************
      利用定时器0控制LED流转间隔时间
**********************************************/
void Timer0Init()		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TMOD |= 0x01;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TR0 = 1;		//定时器0开始计时              /***************************/
	EA = 1;                                    /****这里不能加上TR1 = 1****/
	ET0 = 1;                                   /***************************/
	ET1 = 1;
}

void Timer0_Routine() interrupt 1
{
	TL0 = 0xCD;
	TH0 = 0xD4;	
	if(!LED_Lock)
	{
		count_t0++;
		if(count_t0 >= LED_t)
		{
			stat++;
			count_t0 = 0;
		}
		if(stat == 25)
			stat = 0;
	}
}
                                                             /*********************************/
void LED_Working()       //                                  /需使用这种形式才可用pwm信号来控制/
{                                                            /*********************************/
	switch(stat)
	{
		case 0 : LED_Dat = 0xff;XBYTE[0x8000] = LED_Dat;break;
		case 1 : LED_Dat = ~(0x01 << 0);break;
		case 2 : LED_Dat = ~(0x01 << 1);break;
		case 3 : LED_Dat = ~(0x01 << 2);break;
		case 4 : LED_Dat = ~(0x01 << 3);break;
		case 5 : LED_Dat = ~(0x01 << 4);break;
		case 6 : LED_Dat = ~(0x01 << 5);break;
		case 7 : LED_Dat = ~(0x01 << 6);break;
		case 8 : LED_Dat = ~(0x01 << 7);break;
		case 9 : LED_Dat = ~(0x80 >> 0);break;
		case 10 : LED_Dat = ~(0x80 >> 1);break;
		case 11 : LED_Dat = ~(0x80 >> 2);break;
		case 12 : LED_Dat = ~(0x80 >> 3);break;
		case 13 : LED_Dat = ~(0x80 >> 4);break;
		case 14 : LED_Dat = ~(0x80 >> 5);break;
		case 15 : LED_Dat = ~(0x80 >> 6);break;
		case 16 : LED_Dat = ~(0x80 >> 7);break;
		case 17 : LED_Dat = ~((0x80 >> 0) | (0x01 << 0));break;
		case 18 : LED_Dat = ~((0x80 >> 1) | (0x01 << 1));break;
		case 19 : LED_Dat = ~((0x80 >> 2) | (0x01 << 2));break;
		case 20 : LED_Dat = ~((0x80 >> 3) | (0x01 << 3));break;
		case 21 : LED_Dat = ~((0x80 >> 4) | (0x01 << 4));break;
		case 22 : LED_Dat = ~((0x80 >> 5) | (0x01 << 5));break;
		case 23 : LED_Dat = ~((0x80 >> 6) | (0x01 << 6));break;
		case 24 : LED_Dat = ~((0x80 >> 7) | (0x01 << 7));break;
	}
	if(stat >= 21)
		LED_State = 4;
	else if(stat >= 17)
		LED_State = 3;
	else if(stat >= 9)
		LED_State = 2;
	else
		LED_State = 1;
}

/*********************************************
          利用定时器1控制LED亮度
**********************************************/
void Timer1Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x40;		//定时器时钟1T模式
	TMOD &= 0x0F;		//设置定时器模式
	TMOD |= 0x10;		//设置定时器模式
	TL1 = 0xCD;		//设置定时初值
	TH1 = 0xD4;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	EA = 1;
	ET1 = 1;
}
//void Timer1Init()		//1微秒@11.0592MHz
//{
//	AUXR |= 0x40;		//定时器时钟1T模式
//	TMOD &= 0x0F;		//设置定时器模式
//	TMOD |= 0x10;		//设置定时器模式
//	TL1 = 0xF5;		//设置定时初值
//	TH1 = 0xFF;		//设置定时初值
//	TF1 = 0;		//清除TF1标志
//	TR1 = 1;		//定时器1开始计时
//	EA = 1;
//	ET1 = 1;
//}

void Judgment_LED_Level()
{
	unsigned char dat = Read_PCF8591();
	if( dat >= 192 )
		LED_Level = 4;
	else if(dat >= 128)
		LED_Level = 3;
	else if(dat >= 64)
		LED_Level = 2;
	else
		LED_Level = 1;
}

void Timer1_Routine() interrupt 3
{
	TL1 = 0xCD;		//设置定时初值
	TH1 = 0xD4;		//设置定时初值
	Judgment_LED_Level();
	if (!LED_Lock)
		if( ++count_t1 >= (20 - (LED_Level * 5)) )
		{
			XBYTE[0x8000] = LED_Dat;
			if(count_t1 >= 20)
			{
				count_t1 = 0;
				XBYTE[0x8000] = 0xff;
				LED_Working();
			}
		}
}

/*********************************************
              独立按键扫描函数
**********************************************/
void ScanKey()
{
	if(P30 == 0) //S7
	{
		delay(100);
		if(P30 == 0)
		{
			LED_Lock = ~LED_Lock;
			while(!P30)
			{
				Display_Nixie();
			}
		}
	}
	if(P31 == 0) //S6
	{
		delay(100);
		if(P31 == 0)
		{
			if(Nixie_State == 0)
				Nixie_State = 1;
			else
				Nixie_State = 0;
			while(!P31)
			{
				Display_Nixie();
			}
		}
	}
	if(P32 == 0) //S5
	{
		delay(100);
		if(P32 == 0 && LED_t < 1200 && Nixie_State == 1)
		{
			LED_t += 100;
			Write_E2PROM(LED_t);
			delay(3000);                                          //！！！！！！！！！！！！！
			while(!P32)
			{
				Display_Nixie();
			}
		}
	}
	if(P33 == 0) //S4
	{
		delay(100);
		if(P33 == 0 && LED_t > 400 && Nixie_State == 1)
		{
			LED_t -= 100;
			Write_E2PROM(LED_t);
			delay(3000);                                          //！！！！！！！！！！！！！
			while(!P33)                                          //可简化
			{
				Display_Nixie();
			}
		}
		if(P33 == 0 && Nixie_State == 0)
		{
			Nixie_State = 2;
			while(!P33)                                          //可简化
			{
				Display_Nixie();
			}
			Nixie_State = 0;
		}
	}
}

void main()
{
	Timer0Init();
	Timer1Init();
	InitSystem();
	LED_t = Read_E2PROM();
	while(1)
	{
		ScanKey();
		Display_Nixie();
	}
}