#include "W6100EVB.h"

DMA_InitTypeDef		DMA_RX_InitStructure, DMA_TX_InitStructure;

#if 0
//================================================================

#ifdef USE_STDPERIPH_DRIVER
	#warning USE_STDPERIPH_DRIVER

#elif defined USE_HAL_DRIVER
	#warning USE_HAL_DRIVER

#else
	#warning STD_DRIVER not defined

#endif

//================================================================
#endif

#ifdef USE_STDPERIPH_DRIVER
void W6100EVBInitialze(void)
{
	RCCInitialize();
	gpioInitialize();
	usartInitialize();
	timerInitialize();
	W6100EVBSPICallBack();

	printf("System start.\r\n");

	#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_
	FSMCInitialize();
#else
	spiInitailize();
#endif

	resetAssert();
	delay(10);
	resetDeassert();
	delay(10);

	W6100Initialze();
}
#endif

void W6100EVBSPICallBack(void)
{

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
	/* SPI method callback registration */
	#if defined SPI_DMA
		reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte,spiReadBurst,spiWriteBurst);
	#else
		reg_wizchip_spi_cbfunc(spiReadByte, spiWriteByte,0,0);
	#endif
	/* CS function register */
	reg_wizchip_cs_cbfunc(csEnable,csDisable);
#else
	/* Indirect bus method callback registration */
	#if defined BUS_DMA
			reg_wizchip_bus_cbfunc(busReadByte, busWriteByte,busReadBurst,busWriteBurst);
	#else
			reg_wizchip_bus_cbfunc(busReadByte, busWriteByte,0,0);
	#endif
#endif

}

void W6100Initialze(void)
{
	intr_kind temp;
	unsigned char W6100_AdrSet[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
	//unsigned char W6100_AdrSet[2][8] = {{8,0,0,0,0,0,0,0},{8,0,0,0,0,0,0,0}};
	/*
	 */
	do{
			if(ctlwizchip(CW_GET_PHYLINK,(void*)&temp) == -1){
				printf("Unknown PHY link status.\r\n");
			}
		}while(temp == PHY_LINK_OFF);
	printf("PHY OK.\r\n");

	temp = IK_DEST_UNREACH;

	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)W6100_AdrSet) == -1)
	{
		printf("W6100 initialized fail.\r\n");
	}

	if(ctlwizchip(CW_SET_INTRMASK,&temp) == -1)
	{
		printf("W6100 interrupt\r\n");
	}
	//printf("interrupt mask: %02x\r\n",getIMR());
}

uint8_t spiReadByte(void)
{
#ifdef USE_STDPERIPH_DRIVER

	while (SPI_I2S_GetFlagStatus(W6100_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(W6100_SPI, 0xff);
	while (SPI_I2S_GetFlagStatus(W6100_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(W6100_SPI);

#elif defined USE_HAL_DRIVER
	uint8_t rx = 0, tx = 0xFF;
	HAL_SPI_TransmitReceive(&W6100_SPI, &tx, &rx, W6100_SPI_SIZE, W6100_SPI_TIMEOUT);
	return rx;
#endif
}

void spiWriteByte(uint8_t byte)
{
#ifdef USE_STDPERIPH_DRIVER

	while (SPI_I2S_GetFlagStatus(W6100_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(W6100_SPI, byte);
	while (SPI_I2S_GetFlagStatus(W6100_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(W6100_SPI);

#elif defined USE_HAL_DRIVER
	uint8_t rx;
	HAL_SPI_TransmitReceive(&W6100_SPI, &byte, &rx, W6100_SPI_SIZE, W6100_SPI_TIMEOUT);
#endif

}

uint8_t spiReadBurst(uint8_t* pBuf, uint16_t len)
{
#ifdef USE_STDPERIPH_DRIVER

	unsigned char tempbuf =0xff;
	DMA_TX_InitStructure.DMA_BufferSize = len;
	DMA_TX_InitStructure.DMA_MemoryBaseAddr = &tempbuf;
	//DMA_TX_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_Init(W6100_DMA_CHANNEL_TX, &DMA_TX_InitStructure);

	DMA_RX_InitStructure.DMA_BufferSize = len;
	DMA_RX_InitStructure.DMA_MemoryBaseAddr = pBuf;
	DMA_Init(W6100_DMA_CHANNEL_RX, &DMA_RX_InitStructure);
	/* Enable SPI Rx/Tx DMA Request*/
	DMA_Cmd(W6100_DMA_CHANNEL_RX, ENABLE);
	DMA_Cmd(W6100_DMA_CHANNEL_TX, ENABLE);
	/* Waiting for the end of Data Transfer */
	while(DMA_GetFlagStatus(DMA_TX_FLAG) == RESET);
	while(DMA_GetFlagStatus(DMA_RX_FLAG) == RESET);


	DMA_ClearFlag(DMA_TX_FLAG | DMA_RX_FLAG);

	DMA_Cmd(W6100_DMA_CHANNEL_TX, DISABLE);
	DMA_Cmd(W6100_DMA_CHANNEL_RX, DISABLE);

#elif defined USE_HAL_DRIVER

#endif

}

void spiWriteBurst(uint8_t* pBuf, uint16_t len)
{
#ifdef USE_STDPERIPH_DRIVER

	unsigned char tempbuf;
	DMA_TX_InitStructure.DMA_BufferSize = len;
	DMA_TX_InitStructure.DMA_MemoryBaseAddr = pBuf;
	DMA_Init(W6100_DMA_CHANNEL_TX, &DMA_TX_InitStructure);

	DMA_RX_InitStructure.DMA_BufferSize = 1;
	DMA_RX_InitStructure.DMA_MemoryBaseAddr = &tempbuf;
	DMA_RX_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_Init(W6100_DMA_CHANNEL_RX, &DMA_RX_InitStructure);

	DMA_Cmd(W6100_DMA_CHANNEL_RX, ENABLE);
	DMA_Cmd(W6100_DMA_CHANNEL_TX, ENABLE);

	/* Enable SPI Rx/Tx DMA Request*/



	/* Waiting for the end of Data Transfer */
	while(DMA_GetFlagStatus(DMA_TX_FLAG) == RESET);
	while(DMA_GetFlagStatus(DMA_RX_FLAG) == RESET);

	DMA_ClearFlag(DMA_TX_FLAG | DMA_RX_FLAG);

	DMA_Cmd(W6100_DMA_CHANNEL_TX, DISABLE);
	DMA_Cmd(W6100_DMA_CHANNEL_RX, DISABLE);

#elif defined USE_HAL_DRIVER

#endif

}

//(*bus_wb)(uint32_t addr, iodata_t wb);
void busWriteByte(uint32_t addr, iodata_t data)
{

//	(*((volatile uint8_t*)(W6100Address+1))) = (uint8_t)((addr &0xFF00)>>8);
//	(*((volatile uint8_t*)(W6100Address+2))) = (uint8_t)((addr) & 0x00FF);
//	(*((volatile uint8_t*)(W6100Address+3))) = data;
	(*(volatile uint8_t*)(addr)) = data;
}

//iodata_t (*bus_rb)(uint32_t addr);
iodata_t busReadByte(uint32_t addr)
{

//	(*((volatile uint8_t*)(W6100Address+1))) = (uint8_t)((addr &0xFF00)>>8);
//	(*((volatile uint8_t*)(W6100Address+2))) = (uint8_t)((addr) & 0x00FF);
//	return  (*((volatile uint8_t*)(W6100Address+3)));
	return (*((volatile uint8_t*)(addr)));
}

void busWriteBurst(uint32_t addr, uint8_t* pBuf ,uint32_t len,uint8_t addr_inc)
{
#ifdef USE_STDPERIPH_DRIVER

	if(addr_inc){
	 	DMA_TX_InitStructure.DMA_MemoryInc  = DMA_MemoryInc_Enable;

	}
	else 	DMA_TX_InitStructure.DMA_MemoryInc  = DMA_MemoryInc_Disable;


	DMA_TX_InitStructure.DMA_BufferSize = len;
	DMA_TX_InitStructure.DMA_MemoryBaseAddr = addr;
	DMA_TX_InitStructure.DMA_PeripheralBaseAddr = pBuf;

	DMA_Init(W6100_DMA_CHANNEL_TX, &DMA_TX_InitStructure);

	DMA_Cmd(W6100_DMA_CHANNEL_TX, ENABLE);

	/* Enable SPI Rx/Tx DMA Request*/


	/* Waiting for the end of Data Transfer */
	while(DMA_GetFlagStatus(DMA_TX_FLAG) == RESET);


	DMA_ClearFlag(DMA_TX_FLAG);

	DMA_Cmd(W6100_DMA_CHANNEL_TX, DISABLE);

#elif defined USE_HAL_DRIVER

#endif

}


void busReadBurst(uint32_t addr,uint8_t* pBuf, uint32_t len,uint8_t addr_inc)
{
#ifdef USE_STDPERIPH_DRIVER

	DMA_RX_InitStructure.DMA_BufferSize = len;
	DMA_RX_InitStructure.DMA_MemoryBaseAddr =pBuf;
	DMA_RX_InitStructure.DMA_PeripheralBaseAddr =addr;

	DMA_Init(W6100_DMA_CHANNEL_RX, &DMA_RX_InitStructure);

	DMA_Cmd(W6100_DMA_CHANNEL_RX, ENABLE);
	/* Waiting for the end of Data Transfer */
	while(DMA_GetFlagStatus(DMA_RX_FLAG) == RESET);


	DMA_ClearFlag(DMA_RX_FLAG);


	DMA_Cmd(W6100_DMA_CHANNEL_RX, DISABLE);

#elif defined USE_HAL_DRIVER

#endif

}


inline void csEnable(void)
{
#ifdef USE_STDPERIPH_DRIVER

	GPIO_ResetBits(W6100_CS_PORT, W6100_CS_PIN);

#elif defined USE_HAL_DRIVER
	HAL_GPIO_WritePin(W6100_CS_PORT, W6100_CS_PIN, GPIO_PIN_RESET);
#endif

}

inline void csDisable(void)
{
#ifdef USE_STDPERIPH_DRIVER

	GPIO_SetBits(W6100_CS_PORT, W6100_CS_PIN);

#elif defined USE_HAL_DRIVER
	HAL_GPIO_WritePin(W6100_CS_PORT, W6100_CS_PIN, GPIO_PIN_SET);
#endif

}

inline void resetAssert(void)
{
#ifdef USE_STDPERIPH_DRIVER

	GPIO_ResetBits(W6100_RESET_PORT, W6100_RESET_PIN);

#elif defined USE_HAL_DRIVER
	HAL_GPIO_WritePin(W6100_RESET_PORT, W6100_RESET_PIN, GPIO_PIN_RESET);
#endif

}

inline void resetDeassert(void)
{
#ifdef USE_STDPERIPH_DRIVER

	GPIO_SetBits(W6100_RESET_PORT, W6100_RESET_PIN);
#elif defined USE_HAL_DRIVER
	HAL_GPIO_WritePin(W6100_RESET_PORT, W6100_RESET_PIN, GPIO_PIN_SET);
#endif

}

void W6100Reset(void)
{
#ifdef USE_STDPERIPH_DRIVER

	GPIO_ResetBits(W6100_RESET_PORT,W6100_RESET_PIN);
	CoTickDelay(10);
	GPIO_SetBits(W6100_RESET_PORT,W6100_RESET_PIN);

#elif defined USE_HAL_DRIVER
	HAL_GPIO_WritePin(W6100_RESET_PORT, W6100_RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(W6100_RESET_PORT, W6100_RESET_PIN, GPIO_PIN_SET);
#endif

}
//
//void register_read(void)
//{
//
//
//	int i;
//	printf("                    ----register read----\r\n");
//	printf("Address | ");
//	for(i = 0 ; i < 16 ; i++)
//	  printf("%02x ",i);
//	printf("\r\n---------------------------------------------------------");
//	for(i = 0 ; i < 0x0090 ; i++)
//	{
//	  if(i%16 == 0) printf("\r\n  %04x  | ", i);
//	  printf("%02x ",WIZCHIP_READ(_W6100_IO_BASE_ + (i << 8) + (WIZCHIP_CREG_BLOCK << 3)));
//	}
//	printf("\r\n");
//}
//
//void socket_register_read(uint8_t sn)
//{
//	int i;
//	printf("                    ----Sn register read----\r\n");
//	printf("Address | ");
//	for(i = 0 ; i < 16 ; i++)
//	  printf("%02x ",i);
//	printf("\r\n---------------------------------------------------------");
//	for(i = 0x400+(sn*(0x100)) ; i < 0x400+(sn*(0x100)+0x35) ; i++)
//	{
//	  if(i%16 == 0) printf("\r\n0x%04x  | ", i);
//	  printf("%02x ",WIZCHIP_READ(i));
//	}
//	printf("\r\n");
//}
