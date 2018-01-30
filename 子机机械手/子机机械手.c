
#include<stdint.h>
#include<avr/io.h>
#include<avr/interrupt.h>
#include<avr/signal.h>
#include<util/delay.h>

#include "spi.h"
#include "2401.h"		//内部管脚连接

volatile uint16_t PWM1,PWM2,PWM3,PWM4,i;

volatile uint16_t PWM_count=0;



//WAVE
volatile uint32_t Distance_count=0;
volatile float distance;

volatile float distance_fore,distance_later;


/*****************************************WAVE*****************************************/
ISR(SIG_OVERFLOW2,ISR_NOBLOCK)
{	Distance_count+=1;


}


ISR(SIG_INTERRUPT0,ISR_NOBLOCK)
{	
 		TCNT2 = 0;
		Distance_count = 0;

		
}
ISR(SIG_INTERRUPT1,ISR_NOBLOCK)
{
		distance = ((Distance_count*256+TCNT2)*0.031-0.25)*340/2;//毫米
		
}

void set_distance()
{	PORTA &= ~(1<<PA0);//PA0=0
	_delay_us(10);
	PORTA |= (1<<PA0);//PA0=1
	_delay_ms(100);
}	




void WAVE_init()//随PWM波1分频，而不是默认的256分频，所以Distance_count相对会很大
{
	MCUCR |= (1<<ISC01)|(1<<ISC00);  //INT0（PD2）上升沿产生中断
	MCUCR |= (1<<ISC11)|(0<<ISC10);  //INT1（PD3）下降沿产生中断
	GICR |= (1<<INT0);  //INT0 中断使能
	GICR |= (1<<INT1);  //INT1 中断使能

	TCCR2|=((1<<CS22)|(1<<CS21)|(0<<CS20));//T/C2计时器256分频
	TIMSK|=(1<<TOIE2);
	TCNT2=0;

	
	PORTA|=0x01;

	DDRA|=0x01;//PA0口输出使能
	

	DDRD&=~((1<<PD2)|(1<<PD3));//PD2,PD3口输入返回脉冲



}

/****************************************PWM********************************************/

ISR(SIG_OVERFLOW0)//0.032ms计一轮16~78,,,625
{
	PWM_count+=1;
	if(PWM_count==PWM1)
		PORTA&=~(1<<3);//第四位置零
	if(PWM_count==PWM2)
		PORTA&=~(1<<4);//第五位置零
 	if(PWM_count==PWM3)
		PORTA&=~(1<<5);//第六位置零
	if(PWM_count==PWM4)
		PORTA&=~(1<<6);//第七位置零
	if(PWM_count==625){//20ms
		PWM_count=0;
		PORTA|=0x78;}
	

}

void PWM_init()
{	cli();//清除所有中断

	PWM_count=0;
 	DDRA|=0x78;//A口设为输出01111000

	PORTA|= 0x78;//A口设为高电平01111000

	TCCR0|=0x01;//T/C1计时器1分频
	TIMSK|=0x01;
	TCNT0=0;


	sei();//全局中断使能

}
void charge(volatile uint16_t PWM1pre,volatile uint16_t PWM1aft,
			volatile uint16_t PWM2pre,volatile uint16_t PWM2aft,
			volatile uint16_t PWM3pre,volatile uint16_t PWM3aft,
			volatile uint16_t PWM4pre,volatile uint16_t PWM4aft,volatile uint16_t i)
{	int t=0;
	float temp1=(PWM1aft-PWM1pre)/10;
	float temp2=(PWM2aft-PWM2pre)/10;		
	float temp3=(PWM3aft-PWM3pre)/10;
	float temp4=(PWM4aft-PWM4pre)/10;
	switch(i){
	
		case 1: for(t=1;t<=10;t++){
					PWM1=(int)(temp1*t+PWM1pre);
					_delay_ms(1000);
					}
				break;
		case 2: for(t=1;t<=10;t++){
					PWM1=(int)(temp1*t+PWM1pre);
					PWM2=(int)(temp2*t+PWM2pre);
					_delay_ms(1000);
					}
				break;
		case 3: for(t=1;t<=10;t++){
					PWM1=(int)(temp1*t+PWM1pre);
					PWM2=(int)(temp2*t+PWM2pre);
					PWM3=(int)(temp3*t+PWM3pre);
					_delay_ms(1000);
					}
				break;
		case 4: for(t=1;t<=10;t++){
					PWM1=(int)(temp1*t+PWM1pre);
					PWM2=(int)(temp2*t+PWM2pre);
					PWM3=(int)(temp3*t+PWM3pre);
					PWM4=(int)(temp4*t+PWM4pre);
					_delay_ms(1000);
					}
				break;
		default: 

				break;

		}

}		
void start_hands()
{

}



void move_hands_to_car()
{


}
void move_hands_back_to_man()
{

}


void straight_hands()
{

}
void back_hands()
{

}



void free_hands()
{

}
void close_hands()
{


}


void shut_mxg995()
{	TIMSK&=~(1<<TOIE0);
	DDRA|=0x78;//A口设为输出01111000
	PORTA|= 0x78;//A口设为高电平01111000

}
void open_mxg995()
{	TIMSK|=(1<<TOIE0);
	TCNT0=0;
	DDRA|=0x78;//A口设为输出01111000
	PORTA|= 0x78;//A口设为高电平01111000

}

/*******************************************light************************************/
void LIGHT_init()
{	DDRA&=~((1<<PA1)|(1<<PA2));//PA1,PA2 in

}




/********************************************main************************************/
int main()
{	unsigned char send_data=0;
	PWM_init();
	WAVE_init();
	LIGHT_init();
	SPI_Init();



	RF2401_Init();			//初始化2401
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

	start_hands();
	while(1){
		set_distance();
		if(distance>=40)
			;
		else
			close_hands();
			break;
			}
	move_hands_to_car();
	straight_hands();
	free_hands();
	back_hands();
	shut_mxg995();
	_delay_ms(4000);
	while(1) 
	{	
			TxData[0]=send_data;
			TxData[1]='G';

			
			W_Send_Data(2);
			Tx_Mode();

			set_distance();
			if(distance>=60)
				break;
				
	}//while

	while(1) 
	{		if(!(PINA&(1<<PA1)))
				TxData[1]='R';
			else
				TxData[1]='L';
			TxData[0]=send_data;


			
			W_Send_Data(2);
			Tx_Mode();

			set_distance();
			if(distance>=60)
				;
			else
				break;
				
	}//while


	straight_hands();
	close_hands();
	back_hands();
	move_hands_back_to_man();
	start_hands();
	
	while(1)
	{


	}


	return 0;



}
