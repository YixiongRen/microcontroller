
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/signal.h>
#include<util/delay.h>

unsigned char drink;
volatile float distance_fore,distance_later;

volatile uint32_t count=0;
volatile float distance;


#include "spi.h"			//2401�ܽ�����
#include "2401.h"		




void init()
{	DDRD|=0xFF;
	PORTD|=0xFF;
}



int main()
{	init();

	SPI_Init();
	sei();


//***********************************************************
	RF2401_Init();			//��ʼ��2401
 	Rx_Mode();				//����ģʽ

	PORT_SPI&=~(1<<CSN);
	SpiRW(1|W_REGSITER);	//д �Ĵ���1
	SpiRW(0x0);				//��ֹ�Զ�Ӧ��  
	PORT_SPI|=(1<<CSN);

	PORT_SPI&=~(1<<CSN);   
	SpiRW(0x31);			
	SpiRW(0X20);			//ͨ��0��Ч���ݿ��32
	PORT_SPI|=(1<<CSN);
	_delay_ms(1);


					_delay_ms(42000);
					PORTD=0x00;
					_delay_ms(2000);
					PORTD=0xFF;
	






	while(1)
	{


	}
	

return 0;


}
