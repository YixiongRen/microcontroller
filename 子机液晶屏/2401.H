///***************************
///***************************
//2401，基本函数 
///***************************
//基本命令定义
#define R_REGSITER	  0X00	//读寄存器	直接与地址按位或
#define W_REGSITER	  0X20	//写寄存器	直接与地址按位或
#define R_RX_PAYLOAD  0X61	//读数据 1-32 字节  从0字节开始
#define W_TX_PAYLOAD  0XA0	//写数据 1-32 字节  从0字节开始
#define FLUSH_TX	  0xe1	//清除TX FIFO 寄存器
#define FLUSH_RX      0XE2	//清除RX FIFO 寄存器  传输应答信号时，不应使用此命令
#define RESUSE_TX_PL  0XE3	//重新使用上一包有效数据，当CE为高时，数据包不断重新发射
#define NOP			  0XFF	//空指令 用于取回数据

//状态寄存器中标志位
#define RX_DR	6	//接收中断
#define	TX_DS	5	//发送完毕中断
#define MAX_RT	4	//最大重发中断

#define RECE_DATA_NUM	32	//接收数据最大量
#define TRAN_DATA_NUM	32	//接收数据最大量
#define TxAddNum		5	//发送地址宽度
#define RxAddNum		5	//发送地址宽度

unsigned char TxData[TRAN_DATA_NUM]={1,2,3,4};	//要发送的数据
unsigned char RxData[TRAN_DATA_NUM]={};	//要发送的数据
unsigned char TxAdd[TxAddNum]={0XE7,0XE7,0XE7,0XE7,0XE7};
///*********************
//2401初始化
///*********************
void RF2401_Init(void)
{
	DDR_SPI|=((1<<MOSI)|(1<<SCK)|(1<<CSN)|(1<<CE));	//设置MOSI、SCK、CSN输出，其余输入
	DDR_SPI&=~((1<<MISO)|(1<<IRQ));	
	PORT_SPI|=(1<<CSN);
}
///*********************
//发送模式
///*********************
void Tx_Mode(void)
{
	PORT_SPI&=~(1<<CE);
	PORT_SPI&=~(1<<CSN);
	SpiRW(0|W_REGSITER);			//写寄存器 0
	SpiRW(0x12);		//使能所有中断，crc使能，8位crc，上电，发送模式
	PORT_SPI|=(1<<CSN);
	PORT_SPI|=(1<<CE);
	_delay_ms(1);
}
///*********************
//接收模式
///*********************
void Rx_Mode(void)
{
	PORT_SPI|=(1<<CE);
	PORT_SPI&=~(1<<CSN);
	SpiRW(0|W_REGSITER);			//写寄存器 0
	SpiRW(0x13);		//使能所有中断，crc使能，8位crc，上电，	接收模式
	PORT_SPI|=(1<<CSN);
	_delay_ms(2);
}
///*********************
//待机1模式
///*********************
void Sleep1_Mode(void)
{
	PORT_SPI&=~(1<<CE);
}
///*********************
//写 要发送的有效数据
///*********************
void W_Send_Data(unsigned char send_data_num)
{
	PORT_SPI&=~(1<<CSN);
	SpiRW(FLUSH_TX);	//清除TX fifo寄存器
	PORT_SPI|=(1<<CSN);
	PORT_SPI&=~(1<<CSN);
	SpiRW(W_TX_PAYLOAD);//写命令
	unsigned char i;
	for(i=0;i<send_data_num;i++)
	{
		SpiRW(TxData[i]);
	}
	PORT_SPI|=(1<<CSN);
}
///*********************
//读 接收到的有效数据
///*********************
void Read_Rx(unsigned char rece_data_num)//输入要读取的数据数目
{
	PORT_SPI&=~(1<<CSN);
	SpiRW(R_RX_PAYLOAD);//读命令	RX FIFO
	unsigned char i;
	for(i=0;i<rece_data_num;i++)
	{
		RxData[i]=SpiRW(NOP);
	}
	PORT_SPI|=(1<<CSN);

	PORT_SPI&=~(1<<CSN);
	SpiRW(FLUSH_RX);	//清除RX 	fifo寄存器
	PORT_SPI|=(1<<CSN);
}
///*********************
//读 中断标志
///*********************
unsigned char Read_IRQ(void)
{
	PORT_SPI&=~(1<<CSN);
	unsigned char sta;
	sta=SpiRW(0X07);					//读状态寄存器	用NOP命令也可将状态读出
	PORT_SPI|=(1<<CSN);
	return sta;
}
///*********************
//清除标志位
///*********************
void Clr_IRQ(unsigned char note)	//示例：Cle_IRQ(((1<<tx_dr)|(1<<max_rt)))
{
	PORT_SPI&=~(1<<CSN);
	SpiRW(0X07|W_REGSITER);		//写状态寄存器
	SpiRW(note);
	PORT_SPI|=(1<<CSN);
}
