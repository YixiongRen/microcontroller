
#include<stdint.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/signal.h>
#include<util/delay.h>


#include "spi.h"			//2401�ܽ�����
#include "2401.h"



volatile float wheelspeed1=0,wheelspeed2=0;
volatile uint16_t PWMhigh=0,PWMtotal=0;
volatile float start=0;



ISR(SIG_OVERFLOW2)//32msһ��//���ּ���
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


void SEARCH_WHITE_init()//PD5������Ѱ���ߵ͵�ƽ
{	DDRD&=~(1<<PD5);


}


void WHEEL_init()
{	cli();//��������ж�
	TIMSK|=1<<6;//TC2����ж�ʹ��
	TCCR2|=0x07;//1024��Ƶ
	TCNT2=0;
	DDRA|=0xFF;
	DDRB|=((1<<PB0)|(1<<PB1));
	sei();//ȫ���ж�ʹ��

}


int main()
{	WHEEL_init();
	SPI_Init();
	SEARCH_WHITE_init();


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
/*****************************************WAIT FOR START***************************/
	while(1) //�տ�ʼС���ȴ�����
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
		if(RxData[1]=='G')//С���ӵ�����ǰ��
			break;
	}
/*****************************************GO FORWARD TO MOTHERBOARD***********************/
	start=1;
	PORTA=0x5F;//01011111��ת
	PORTB|=0x03;//00000011
	_delay_ms(4000);

	PORTA=0x6F;//01101111һ��һ��
	_delay_ms(1500);
	
	PORTA=0xAF;//10101111��ת
	

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
		if(RxData[1]=='L'){//С���ӵ��������
			if(PWMhigh==0)
				PWMhigh=0;
			else
				PWMhigh--;
			
			}
		else if(RxData[1]=='R'){//С���ӵ������ҹ�
			if(PWMhigh==PWMtotal)
				PWMhigh=PWMtotal;
			else
				PWMhigh++;
			
			}
		else
		;




		if(!PIND&(1<<PD5)){//��⵽Ѱ�����źţ�ɲͣ
			PORTA|=0xFF;
			PORTB|=0x03;
			start=0;//�ر�PWM��
			break;
		}
	}//while
	_delay_ms(12000);

/*****************************************GO BACKWARD TO CONTROL HANDLES***********************/

	start=1;
	PORTA=0x5F;//01011111��ת
	PORTB|=0x03;//00000011
	_delay_ms(4000);

	PORTA=0x6F;//01101111һ��һ��
	_delay_ms(1500);
	
	PORTA=0xAF;//10101111��ת
	

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
		if(RxData[1]=='L'){//С���ӵ��������
			if(PWMhigh==PWMtotal)
				PWMhigh=PWMtotal;
			else
				PWMhigh++;
			
			}
		else if(RxData[1]=='R'){//С���ӵ������ҹ�
			if(PWMhigh==0)
				PWMhigh=0;
			else
				PWMhigh--;
			
			}
		else
		;




		if(!PIND&(1<<PD5)){//��⵽Ѱ�����źţ�ɲͣ
			PORTA|=0xFF;
			PORTB|=0x03;
			start=0;//�ر�PWM��
			break;
		}
	}//while


	while(1)
	{


	}




}//main
