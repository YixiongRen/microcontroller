
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/signal.h>
#include<util/delay.h>

#include "spi.h"			//2401管脚连接
#include "2401.h"


		
//SCREEN
//端口位定义
#define RS PA4           //数据/命令控制端 0命令，1数据
#define RW PA5           //读/写选择控制端 0写，1读
#define E PA6            //使能端          下降沿读，高电平写
//全局变量声明
unsigned char LCD_DispBuff0[16]="Drink Info :    ";
unsigned char LCD_DispBuff1[16]="Cola selected   ";
unsigned char LCD_DispBuff2[16]="Coffee selected ";
unsigned char LCD_DispBuff3[16]="The distance is:";
unsigned char LCD_DispBuff4[16]="Please select   ";
unsigned char LCD_DispBuff5[16]="the drink:      ";
unsigned char LCD_DispBuff6[16];
volatile uint16_t temp=0,i;
unsigned char drink,qian,bai,shi,ge;

//WAVE
volatile uint32_t count=0;
volatile float distance;

volatile float distance_fore,distance_later;


//wileless
unsigned char send_data=0;
unsigned char Disp_Number;



//SCREEN函数声明
void Port_Init(void);   //端口初始化
void LCD_Init(void);    //LCD初始化
void Write_Com(unsigned char LCD_Com);   //LCD写指令
void Write_Data(unsigned char LCD_Data);  //LCD写数据
void Check_Busy(void);   //读写检测函数，每次对液晶操作前都要进行读写检测

/**********************************INPUT**************************/
void INPUT_init()
{
	DDRD&=~((1<<PD4)|(1<<PD5));//PD4 PD5输入
	DDRD|=((1<<PD6)|(1<<PD7));
	PORTD|=((1<<PD6)|(1<<PD7));


}
/**********************************WAVE**************************/


ISR(SIG_OVERFLOW0,ISR_NOBLOCK)//记一次是0.032ms
{
	count+=1;
}


ISR(SIG_INTERRUPT0,ISR_NOBLOCK)
{	
 		TCNT0 = 0;
		count = 0;

		
}
ISR(SIG_INTERRUPT1,ISR_NOBLOCK)
{
		distance = ((count*256+TCNT0)*0.032-0.25)*340/2;//毫米
		
}



void WAVE_init()
{	cli();
	
	MCUCR |= (1<<ISC01)|(1<<ISC00);  //INT0（PD2）上升沿产生中断
	MCUCR |= (1<<ISC11)|(0<<ISC10);  //INT1（PD3）下降沿产生中断
	GICR |= (1<<INT0);  //INT0 中断使能
	GICR |= (1<<INT1);  //INT1 中断使能

	TCCR0 |= (1<<CS02);  //T/C0，256分频
	TIMSK |= (1<<TOIE0);//enable overflow interrupt
	TCNT0 = 0;//start overflow interrupt

	DDRB|=0x01;//PB0用来传输单片机间信号



	
	PORTA|=0x01;

	DDRA |=0xFF;//A口输出使能
	

	DDRD&= 0xF3;//PD2,PD3口输入返回脉冲
	sei();


}


void set_distance()
{	PORTA &= ~(1<<PA0);//PA0=0
	_delay_us(10);
	PORTA |= (1<<PA0);//PA0=1
	_delay_ms(100);
	qian=(unsigned char) distance/1000;
	bai=(unsigned char) (distance-qian*1000)/100;
	shi=(unsigned char) (distance-qian*1000-bai*100)/10;
	ge=(unsigned char) (distance-qian*1000-bai*100-shi*10);
}	




/**********************************SCREEN**************************/




void show_on_screen()
{		if(drink=='a'){
		Write_Com(0X80+0X00);
		for(Disp_Number = 0;Disp_Number < 15;Disp_Number++)
			{
				Write_Data(LCD_DispBuff0[Disp_Number]);
				_delay_ms(5);
			}

		Write_Com(0X80+0X40);
		for(Disp_Number = 0;Disp_Number < 15;Disp_Number++)
			{
				Write_Data(LCD_DispBuff1[Disp_Number]);
				_delay_ms(5);
			}
			
			}
		else{
		Write_Com(0X80+0X00);
		for(Disp_Number = 0;Disp_Number < 15;Disp_Number++)
			{
				Write_Data(LCD_DispBuff0[Disp_Number]);
				_delay_ms(5);
			}
		Write_Com(0X80+0X40);
		for(Disp_Number = 0;Disp_Number < 15;Disp_Number++)
			{
				Write_Data(LCD_DispBuff2[Disp_Number]);
				_delay_ms(5);
			}
			}
}
void show_distance()
{




		LCD_DispBuff6[0]=qian;
		LCD_DispBuff6[1]=bai;
		LCD_DispBuff6[2]=shi;
		LCD_DispBuff6[3]=ge;
		Write_Com(0X80+0X00);
		for(Disp_Number = 0;Disp_Number < 16;Disp_Number++)
			{
				Write_Data(LCD_DispBuff3[Disp_Number]);
				_delay_ms(5);
			}
			
	
		Write_Com(0X80+0X40);
		for(Disp_Number = 0;Disp_Number < 4;Disp_Number++)
			{
				Write_Data(LCD_DispBuff6[Disp_Number]);
				_delay_ms(5);
			}
			
}
void show_menu()
{	
		Write_Com(0X80+0X00);
		for(Disp_Number = 0;Disp_Number < 15;Disp_Number++)
			{
				Write_Data(LCD_DispBuff4[Disp_Number]);
				_delay_ms(5);
			}
			
	
		Write_Com(0X80+0X40);
		for(Disp_Number = 0;Disp_Number < 15;Disp_Number++)
			{
				Write_Data(LCD_DispBuff5[Disp_Number]);
				_delay_ms(5);
			}
			

}


int main()
{
    INPUT_init();

	Port_Init();   //端口初始化
	LCD_Init();    //LCD初始化
				
	SPI_Init();				//初始化SPI接口
	WAVE_init();			//初始化超声波
	RF2401_Init();			//初始化2401
 	Rx_Mode();				//接收模式
/********************************wireless options***********************************/
 	Tx_Mode();				//发送模式

	PORT_SPI&=~(1<<CSN);
	SpiRW(1|W_REGSITER);	//写 寄存器1
	SpiRW(0x0);				//禁止自动应答
	PORT_SPI|=(1<<CSN);

	PORTB&=~(1<<CSN);   
	SpiRW(0x31);			
	SpiRW(0X20);			//通道0有效数据宽度32
	PORTB|=(1<<CSN);
	_delay_ms(1);

	show_menu();
/********************************PICK DRINK***********************************/
	while(1)
	{	if(PIND&(1<<PD4)){
			drink='a';
			break;
			}
		else if(PIND&(1<<PD5)){			
			drink='b';
			break;
			}
		else
			;
	}
	_delay_ms(10);
	LCD_Init();	
	_delay_ms(10);	
	show_on_screen();

/*********************************WIRELESS rounds**************************************/
	while(1) 
	{		
		
			TxData[0]=send_data;
			if(drink=='a')
				TxData[1]='a';
			else
				TxData[1]='b';
			for(i=1;i<32;i++)
				TxData[i]=TxData[1];
			W_Send_Data(32);
			Tx_Mode();
			temp++;
			if(temp==1200){
				temp=0;
				break;
				}


	
	}
	_delay_ms(1000);


	
/*********************************DISTANCE ROUNDS**************************************
	while(1)
	{	set_distance();
			distance_fore=distance;
			show_distance();
			_delay_ms(500);
		set_distance();
			distance_later=distance;
			show_distance();
			_delay_ms(500);
		if((int)distance_fore==(int)distance_later&&distance_fore<=100&&distance_later<=100){
			PORTB&=~(1<<PB0);//PB口置低电平,告诉另一单片机小车返回机械手了
			break;
			}
		else
			;
			
	}
****/

	_delay_ms(50000);
	PORTB|=1<<PB0;//PB口置高电平,告诉另一单片机小车返回机械手了
	while(1)
	{


	}
	
}



/**********************************SCREEN**************************/
void Port_Init()
{      
	//LCD数据端口设置
	PORTC = 0X00;         //
	DDRC = 0XFF;           //配置端口PC全部为输出口,LCD数据端口
	
	//LCD控制端口设置
	PORTA = 0X00;         //
	DDRA |= (1 << RS) | (1 << RW) | (1 << E);
	//DDRA |= (1 << RS) | (1 << RW) | (1 << E);  //配置端口PA的第4、5、6为输出口
	
}

void LCD_Init()
{
	Write_Com(0X01);  //清屏
	_delay_ms(5);
	Write_Com(0X38);  //显示模式设置 16x2显示，5x7点阵，8位数据接口
	_delay_ms(5);
	//Write_Com(0X0f);  //显示开关控制，开显示，光标显示，光标闪烁
	Write_Com(0X0c);  //显示开关控制，开显示，光标不显示，光标不闪烁
	_delay_ms(5);
	Write_Com(0X06);  //光标设置，读或写一个字符后，地址指针加一，光标加一，整屏不移动
	_delay_ms(5);
}

void Write_Com(unsigned char LCD_Com)
{
	Check_Busy();
	
	PORTA &= ~(1 << RS);     //RS=0，写命令
	PORTA &= ~(1 << RW);     //RW=0，写指令
	PORTA |= (1 << E);       //E=1，写操作
	_delay_ms(5);
	PORTC = LCD_Com;         //指令送数据端口
	PORTA &= ~(1 << E);     //E=0，停止写操作
	_delay_ms(5);
	
}

void Write_Data(unsigned char LCD_Data)
{
	Check_Busy();

	PORTA |= (1 << RS);      //RS=1，写数据
	PORTA &= ~(1 << RW);    //RW=0，写指令
	PORTA |= (1 << E);      //E=1，写操作
	_delay_ms(5);  
	PORTC = LCD_Data;        // 数据送数据端口   
	PORTA &= ~(1 << E);    //E=0，停止写操作
	_delay_ms(5);
	
}

void Check_Busy()
{
    	
	DDRC = 0X00;             //PC口置为输入口，准备读取数据
	PORTA &= ~(1 << RS);      //RS=0，读命令
	PORTA |= (1 << RW);    //RW=1，读指令
	PORTA |= (1 << E);      //E=1，使能
	
	while(0X80 & PINC);   //监测忙信号，直到忙信号为0，才能进行读写操作
	PORTA &= ~(1 << E);   //E=0
	DDRC = 0XFF;          //PC口置为输出口，准备向端口发送数据
}

