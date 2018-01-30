
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/signal.h>
#include<util/delay.h>

#include "spi.h"			//2401�ܽ�����
#include "2401.h"


		
//SCREEN
//�˿�λ����
#define RS PA4           //����/������ƶ� 0���1����
#define RW PA5           //��/дѡ����ƶ� 0д��1��
#define E PA6            //ʹ�ܶ�          �½��ض����ߵ�ƽд
//ȫ�ֱ�������
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



//SCREEN��������
void Port_Init(void);   //�˿ڳ�ʼ��
void LCD_Init(void);    //LCD��ʼ��
void Write_Com(unsigned char LCD_Com);   //LCDдָ��
void Write_Data(unsigned char LCD_Data);  //LCDд����
void Check_Busy(void);   //��д��⺯����ÿ�ζ�Һ������ǰ��Ҫ���ж�д���

/**********************************INPUT**************************/
void INPUT_init()
{
	DDRD&=~((1<<PD4)|(1<<PD5));//PD4 PD5����
	DDRD|=((1<<PD6)|(1<<PD7));
	PORTD|=((1<<PD6)|(1<<PD7));


}
/**********************************WAVE**************************/


ISR(SIG_OVERFLOW0,ISR_NOBLOCK)//��һ����0.032ms
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
		distance = ((count*256+TCNT0)*0.032-0.25)*340/2;//����
		
}



void WAVE_init()
{	cli();
	
	MCUCR |= (1<<ISC01)|(1<<ISC00);  //INT0��PD2�������ز����ж�
	MCUCR |= (1<<ISC11)|(0<<ISC10);  //INT1��PD3���½��ز����ж�
	GICR |= (1<<INT0);  //INT0 �ж�ʹ��
	GICR |= (1<<INT1);  //INT1 �ж�ʹ��

	TCCR0 |= (1<<CS02);  //T/C0��256��Ƶ
	TIMSK |= (1<<TOIE0);//enable overflow interrupt
	TCNT0 = 0;//start overflow interrupt

	DDRB|=0x01;//PB0�������䵥Ƭ�����ź�



	
	PORTA|=0x01;

	DDRA |=0xFF;//A�����ʹ��
	

	DDRD&= 0xF3;//PD2,PD3�����뷵������
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

	Port_Init();   //�˿ڳ�ʼ��
	LCD_Init();    //LCD��ʼ��
				
	SPI_Init();				//��ʼ��SPI�ӿ�
	WAVE_init();			//��ʼ��������
	RF2401_Init();			//��ʼ��2401
 	Rx_Mode();				//����ģʽ
/********************************wireless options***********************************/
 	Tx_Mode();				//����ģʽ

	PORT_SPI&=~(1<<CSN);
	SpiRW(1|W_REGSITER);	//д �Ĵ���1
	SpiRW(0x0);				//��ֹ�Զ�Ӧ��
	PORT_SPI|=(1<<CSN);

	PORTB&=~(1<<CSN);   
	SpiRW(0x31);			
	SpiRW(0X20);			//ͨ��0��Ч���ݿ��32
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
			PORTB&=~(1<<PB0);//PB���õ͵�ƽ,������һ��Ƭ��С�����ػ�е����
			break;
			}
		else
			;
			
	}
****/

	_delay_ms(50000);
	PORTB|=1<<PB0;//PB���øߵ�ƽ,������һ��Ƭ��С�����ػ�е����
	while(1)
	{


	}
	
}



/**********************************SCREEN**************************/
void Port_Init()
{      
	//LCD���ݶ˿�����
	PORTC = 0X00;         //
	DDRC = 0XFF;           //���ö˿�PCȫ��Ϊ�����,LCD���ݶ˿�
	
	//LCD���ƶ˿�����
	PORTA = 0X00;         //
	DDRA |= (1 << RS) | (1 << RW) | (1 << E);
	//DDRA |= (1 << RS) | (1 << RW) | (1 << E);  //���ö˿�PA�ĵ�4��5��6Ϊ�����
	
}

void LCD_Init()
{
	Write_Com(0X01);  //����
	_delay_ms(5);
	Write_Com(0X38);  //��ʾģʽ���� 16x2��ʾ��5x7����8λ���ݽӿ�
	_delay_ms(5);
	//Write_Com(0X0f);  //��ʾ���ؿ��ƣ�����ʾ�������ʾ�������˸
	Write_Com(0X0c);  //��ʾ���ؿ��ƣ�����ʾ����겻��ʾ����겻��˸
	_delay_ms(5);
	Write_Com(0X06);  //������ã�����дһ���ַ��󣬵�ַָ���һ������һ���������ƶ�
	_delay_ms(5);
}

void Write_Com(unsigned char LCD_Com)
{
	Check_Busy();
	
	PORTA &= ~(1 << RS);     //RS=0��д����
	PORTA &= ~(1 << RW);     //RW=0��дָ��
	PORTA |= (1 << E);       //E=1��д����
	_delay_ms(5);
	PORTC = LCD_Com;         //ָ�������ݶ˿�
	PORTA &= ~(1 << E);     //E=0��ֹͣд����
	_delay_ms(5);
	
}

void Write_Data(unsigned char LCD_Data)
{
	Check_Busy();

	PORTA |= (1 << RS);      //RS=1��д����
	PORTA &= ~(1 << RW);    //RW=0��дָ��
	PORTA |= (1 << E);      //E=1��д����
	_delay_ms(5);  
	PORTC = LCD_Data;        // ���������ݶ˿�   
	PORTA &= ~(1 << E);    //E=0��ֹͣд����
	_delay_ms(5);
	
}

void Check_Busy()
{
    	
	DDRC = 0X00;             //PC����Ϊ����ڣ�׼����ȡ����
	PORTA &= ~(1 << RS);      //RS=0��������
	PORTA |= (1 << RW);    //RW=1����ָ��
	PORTA |= (1 << E);      //E=1��ʹ��
	
	while(0X80 & PINC);   //���æ�źţ�ֱ��æ�ź�Ϊ0�����ܽ��ж�д����
	PORTA &= ~(1 << E);   //E=0
	DDRC = 0XFF;          //PC����Ϊ����ڣ�׼����˿ڷ�������
}

