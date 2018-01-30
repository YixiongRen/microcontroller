
#include<stdint.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/signal.h>
#include<util/delay.h>


#include "spi.h"			//2401管脚连接
#include "2401.h"



volatile float wheelspeed1=0,wheelspeed2=0;
volatile uint16_t PWMhigh=0,PWMtotal=0;
volatile float start=0;



ISR(SIG_OVERFLOW2)//32ms一轮//车轮计速
{	if(start){
	PWMhigh+=1;
	PWMtotal+=1;
	if(PWMhigh==15)
		PORTB&=0xFE;//PB0=0
	if(PWMtotal==18){
		PORTB|=0x01;//PB0=1
		PWMtotal=0;
		PWMhigh=0;
		}
	}
		
}


void SEARCH_WHITE_init()//PD5口输入寻白线低电平
{	DDRD&=~(1<<PD5);


}


void WHEEL_init()
{	cli();//清除所有中断
	TIMSK|=1<<6;//TC2溢出中断使能
	TCCR2|=0x07;//1024分频
	TCNT2=0;
	DDRA|=0xFF;
	DDRB|=((1<<PB0)|(1<<PB1));
	sei();//全局中断使能

}


int main()
{	WHEEL_init();
	SPI_Init();
	SEARCH_WHITE_init();


//***********************************************************
	RF2401_Init();			//初始化2401
 	Rx_Mode();				//接收模式

	PORT_SPI&=~(1<<CSN);
	SpiRW(1|W_REGSITER);	//写 寄存器1
	SpiRW(0x0);				//禁止自动应答  
	PORT_SPI|=(1<<CSN);

	PORT_SPI&=~(1<<CSN);   
	SpiRW(0x31);			
	SpiRW(0X20);			//通道0有效数据宽度32
	PORT_SPI|=(1<<CSN);
	_delay_ms(1);
/*****************************************WAIT FOR START***************************/
	while(1) //刚开始小车等待命令
	{
		if(!(PINB&(1<<IRQ)))
		{
			unsigned char irq_sta;
			irq_sta=Read_IRQ();
			if(irq_sta&(1<<RX_DR))
			{
				Clr_IRQ(1<<RX_DR);
				Read_Rx(2);
	
			}
			if(irq_sta&(1<<TX_DS))
			{
				Clr_IRQ(1<<TX_DS);
				Rx_Mode();
			}
		}
		if(RxData[1]=='G')//小车接到命令前进
			break;
	}
/*****************************************GO FORWARD TO MOTHERBOARD***********************/
	start=1;
	PORTA=0x5F;//01011111反转
	PORTB|=0x03;//00000011
	_delay_ms(4000);

	PORTA=0x6F;//01101111一正一反
	_delay_ms(1500);
	
	PORTA=0xAF;//10101111正转
	

	while(1)
	{
		if(!(PINB&(1<<IRQ)))
		{
			unsigned char irq_sta;
			irq_sta=Read_IRQ();
			if(irq_sta&(1<<RX_DR))
			{
				Clr_IRQ(1<<RX_DR);
				Read_Rx(2);
	
			}
			if(irq_sta&(1<<TX_DS))
			{
				Clr_IRQ(1<<TX_DS);
				Rx_Mode();
			}
		}
		if(RxData[1]=='L'){//小车接到命令左拐
			if(PWMhigh==0)
				PWMhigh=0;
			else
				PWMhigh--;
			
			}
		else if(RxData[1]=='R'){//小车接到命令右拐
			if(PWMhigh==PWMtotal)
				PWMhigh=PWMtotal;
			else
				PWMhigh++;
			
			}
		else
		;




		if(!PIND&(1<<PD5)){//检测到寻白线信号，刹停
			PORTA|=0xFF;
			PORTB|=0x03;
			start=0;//关闭PWM波
			break;
		}
	}//while
	_delay_ms(12000);

/*****************************************GO BACKWARD TO CONTROL HANDLES***********************/

	start=1;
	PORTA=0x5F;//01011111反转
	PORTB|=0x03;//00000011
	_delay_ms(4000);

	PORTA=0x6F;//01101111一正一反
	_delay_ms(1500);
	
	PORTA=0xAF;//10101111正转
	

	while(1)
	{
		if(!(PINB&(1<<IRQ)))
		{
			unsigned char irq_sta;
			irq_sta=Read_IRQ();
			if(irq_sta&(1<<RX_DR))
			{
				Clr_IRQ(1<<RX_DR);
				Read_Rx(2);
	
			}
			if(irq_sta&(1<<TX_DS))
			{
				Clr_IRQ(1<<TX_DS);
				Rx_Mode();
			}
		}
		if(RxData[1]=='L'){//小车接到命令左拐
			if(PWMhigh==PWMtotal)
				PWMhigh=PWMtotal;
			else
				PWMhigh++;
			
			}
		else if(RxData[1]=='R'){//小车接到命令右拐
			if(PWMhigh==0)
				PWMhigh=0;
			else
				PWMhigh--;
			
			}
		else
		;




		if(!PIND&(1<<PD5)){//检测到寻白线信号，刹停
			PORTA|=0xFF;
			PORTB|=0x03;
			start=0;//关闭PWM波
			break;
		}
	}//while


	while(1)
	{


	}




}//main
