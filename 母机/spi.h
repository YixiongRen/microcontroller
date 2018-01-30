///SPI宏定义
#define DDR_SPI		DDRB
#define PORT_SPI	PORTB
#define CSN 		PB4	
#define MOSI 		PB5
#define MISO 		PB6
#define SCK 		PB7
#define CE			PB3
#define IRQ			PB2

//SPI初始化
void SPI_Init(void)
{
	DDR_SPI|=((1<<MOSI)|(1<<SCK)|(1<<CSN)|(1<<CE));	//设置MOSI、SCK、CSN输出，其余输入
	DDR_SPI&=~((1<<MISO)|(1<<IRQ));					//一直为主机模式
	SPCR|=((1<<SPE)|(1<<MSTR));						//使能SPI、主机，set clock rate fck/4
													//
}

// SPI基本读写函数
uint8_t SpiRW(uint8_t val)  	
{
	uint8_t temp; 
    SPDR=val; 
	while (!(SPSR&(1<<SPIF)))
		; 
	temp=SPDR; 
    return temp; 
} 
