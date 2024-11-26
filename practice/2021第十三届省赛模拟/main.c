/**********************************************************************************************************************
																										题目要求
												                   见第十二届省赛模拟题三月版(2021)
本地文件地址："D:\1竞赛\蓝桥杯\竞赛\蓝桥杯试题-省赛\蓝桥杯单片机历年省赛真题模拟题汇编【蚂蚁工厂】.pdf"
***********************************************************************************************************************/
/**********************************************************************************************************************
																										心得体会
属于比较简单的题目，但LED一直有一点余辉，熄灭得不彻底。
解决方法：使用一个变量每次LED变换后都记录LED状态，确保不会因为数码管等操作遗留的P0口数据影响LED	
***********************************************************************************************************************/
#include <STC15F2K60S2.H>
#include "iic.H"

void delay_nixie(unsigned char t);

//共阴断码
unsigned char code duanma_nixie[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
//共阴断码带小数点
unsigned char code duanma_nixie_dian[] = {0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF};

int vain1 = 0 , vain3 = 0 , vp1 = 250 , vp3 = 300 ;
int num = 1 ; //通道数,1为通道1,3为通道3；
int view = 1 ;//页面，1为数据页面，2为参数页面；
int	flag1 = 0 , flag3 = 0; //用于记录参数的加减，主要是因为题目要求退出参数页面后，加减才生效。

void select573(unsigned char channel)
{
	switch(channel)
	{
		case 4 : P2 = (P2 & 0x1f) | 0x80;break;
		case 5 : P2 = (P2 & 0x1f) | 0xa0;break;
		case 6 : P2 = (P2 & 0x1f) | 0xc0;break;
		case 7 : P2 = (P2 & 0x1f) | 0xe0;break;
		case 0 : P2 = (P2 & 0x1f) | 0x00;break;
	}
}

void initsy() //初始化
{
	select573(4);
	P0 = 0xff;
	select573(5);
	P0 = 0x00;
	select573(7);
	P0 = 0xff;
}

void display_bit_nixie(unsigned char pos , unsigned char value)
{
	select573(7);
	P0 = 0xff;
	select573(6);
	P0 = 0x01 << pos;
	select573(7);
	P0 = value;
}

void read_8591()                //读取PCF8591数据
{
	IIC_Start();                  //采集vain1
	IIC_SendByte(0x90); 
	IIC_WaitAck();
	IIC_SendByte(0x01); 
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91); 
	IIC_WaitAck();
	vain3 = IIC_RecByte();       //为什么这里明明得到的应该是vain1的值，却是给vain3赋值(只有这样赋值才正确...)，
	vain3 = vain3 * (5.0 / 255) * 100; //PCF8591输出的是0—255，要将其转换为0-5V的电压值就要乘以(5.0 / 255)，再乘以100是为了数码管显示方便
	IIC_SendAck(1);              
	IIC_Stop();
	
	IIC_Start();                  //采集vain3
	IIC_SendByte(0x90); 
	IIC_WaitAck();
	IIC_SendByte(0x03); 
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91); 
	IIC_WaitAck();
	vain1 = IIC_RecByte();
	vain1 = vain1 * (5.0 / 255) * 100;
	IIC_SendAck(1);
	IIC_Stop();
}

void delay_nixie(unsigned char t)
{
	while(t--);
}

void display_nixie()
{
	if(view == 1)                                 //数据页面
	{
		read_8591();
		display_bit_nixie(0 , ~0x3E);
		delay_nixie(500);
		display_bit_nixie(1 , ~duanma_nixie[num]);
		delay_nixie(500);
		display_bit_nixie(2 , 0xff);
		delay_nixie(500);
		display_bit_nixie(3 , 0xff);
		delay_nixie(500);
		display_bit_nixie(4 , 0xff);
		delay_nixie(500);
		if(num == 1)
		{
			display_bit_nixie(5 , ~duanma_nixie_dian[vain1/100]);
			delay_nixie(500);
			display_bit_nixie(6 , ~duanma_nixie[(vain1%100)/10]);
			delay_nixie(500);
			display_bit_nixie(7 , ~duanma_nixie[vain1%10]);
			delay_nixie(500);
		}
		else
		{
			display_bit_nixie(5 , ~duanma_nixie_dian[vain3/100]);
			delay_nixie(500);
			display_bit_nixie(6 , ~duanma_nixie[(vain3%100)/10]);
			delay_nixie(500);
			display_bit_nixie(7 , ~duanma_nixie[vain3%10]);
			delay_nixie(500);
		}
	}
	else                                          //参数页面
	{
		read_8591();
		display_bit_nixie(0 , ~0x73);
		delay_nixie(500);
		display_bit_nixie(1 , ~duanma_nixie[num]);
		delay_nixie(500);
		display_bit_nixie(2 , 0xff);
		delay_nixie(500);
		display_bit_nixie(3 , 0xff);
		delay_nixie(500);
		display_bit_nixie(4 , 0xff);
		delay_nixie(500);
		if(num == 1)
		{
			display_bit_nixie(5 , ~duanma_nixie_dian[(vp1 + (flag1 * 20))/100]);
			delay_nixie(500);
			display_bit_nixie(6 , ~duanma_nixie[((vp1 + (flag1 * 20))%100)/10]);
			delay_nixie(500);
			display_bit_nixie(7 , ~duanma_nixie[(vp1 + (flag1 * 20))%10]);
			delay_nixie(500);
		}
		else
		{
			display_bit_nixie(5 , ~duanma_nixie_dian[(vp3 + (flag3 * 20))/100]);
			delay_nixie(500);
			display_bit_nixie(6 , ~duanma_nixie[((vp3 + (flag3 * 20))%100)/10]);
			delay_nixie(500);
			display_bit_nixie(7 , ~duanma_nixie[(vp3 + (flag3 * 20))%10]);
			delay_nixie(500);
		}
	}
}

void delay_key(unsigned char t)
{
	while(t--);
}

void LEDworking()
{
	select573(4);
	if(vain1 > vp1)
		P0 = (P0 & 0xfe) | 0xe0;
	else
		P0 = (P0 & 0xfe) | 0xe1;
	if(vain3 > vp3)
		P0 = (P0 & 0xfd) | 0xe0;
	else
		P0 = (P0 & 0xfd) | 0xe2;
	if(num == 1)
		P0 = (P0 & 0xfb) | 0xe0;
	else
		P0 = (P0 & 0xfb) | 0xe4;
	if(view == 1)
		P0 = (P0 & 0xf7) | 0xe0;
	else
		P0 = (P0 & 0xf7) | 0xe8;
	if(vain1 > vain3)
		P0 = (P0 & 0xef) | 0xe0;
	else
		P0 = (P0 & 0xef) | 0xf0;
	delay_key(60000);
}

void scankey()
{
	if(P33 == 0)                      //s4
	{
		delay_key(100);
		if(P33 == 0)
		{
			if(num == 1)
				num = 3;
			else
				num = 1;
			while(P33 == 0)
			{
				display_nixie();
			}
		}
	}
	
	if(P32 == 0)                      //s5
	{
		delay_key(100);
		if(P32 == 0)
		{
			if(view == 1)
				view = 2;
			else
			{
				view = 1;
				vp1 += flag1 * 20;
				vp3 += flag3 * 20;
				flag1 = 0;
				flag3 = 0;
			}
			while(P32 == 0)
			{
				display_nixie();
			}
		}
	}
	
	if(P31 == 0)                      //s6  加
	{
		delay_key(100);
		if(P31 == 0)
		{
			if(view == 2)
			{
				if(num == 1)
				{
					if( (vp1 + (flag1 * 20)) <= 480)
						flag1 ++;
				}
				else
				{
					if( (vp3 + (flag3 * 20)) <= 480)
						flag3 ++;
				}
			}
			while(P31 == 0)
			{
				display_nixie();
			}
		}
	}
	
	if(P30 == 0)                      //s7  减
	{
		delay_key(100);
		if(P30 == 0)
		{
			if(view == 2)
			{
				if(num == 1)
				{
					if( (vp1 + (flag1 * 20)) >= 20)
						flag1 --;
				}
				else
				{
					if( (vp3 + (flag3 * 20)) >= 20)
						flag3 --;
				}
			}
			while(P30 == 0)
			{
				display_nixie();
			}
		}
	}
}

void main()
{
	initsy();
	while(1)
	{
		LEDworking();
		display_nixie();
		scankey();
	}
}