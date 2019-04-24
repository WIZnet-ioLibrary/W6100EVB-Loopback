#include "board_init.h"

#ifdef USE_STDPERIPH_DRIVER

DMA_InitTypeDef		DMA_RX_InitStructure, DMA_TX_InitStructure;

#elif defined USE_HAL_DRIVER
SPI_HandleTypeDef W6100_SPI;
void spi_set_func(SPI_HandleTypeDef *SPI_n)
{
	W6100_SPI = *SPI_n;
}
#endif


void BoardInitialze(void)
{
	wiz_InitInfo W6100_Info;

#ifdef USE_STDPERIPH_DRIVER
	RCCInitialize();
	gpioInitialize();
	usartInitialize();
	timerInitialize();

	printf("System start.\r\n");

#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_BUS_INDIR_
	FSMCInitialize();
#else
	spiInitailize();
#endif
#endif

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
	/* SPI method callback registration */
	#if defined SPI_DMA
	W6100_Info.spi_rb = spiReadByte;
	W6100_Info.spi_wb = spiWriteByte;
	W6100_Info.spi_rbuf = spiReadBurst;
	W6100_Info.spi_wbuf = spiWriteBurst;
	#else
	W6100_Info.spi_rb = spiReadByte;
	W6100_Info.spi_wb = spiWriteByte;
	W6100_Info.spi_rbuf = spiDummyReadBurst;
	W6100_Info.spi_wbuf = spiDummyWriteBurst;
	#endif
	/* CS function register */
	W6100_Info.cs_sel = csEnable;
	W6100_Info.cs_desel = csDisable;
#else
	/* Indirect bus method callback registration */
	#if defined BUS_DMA
	W6100_Info.bus_rd = busReadByte;
	W6100_Info.bus_wd = busWriteByte;
	W6100_Info.bus_rbuf = busReadBurst;
	W6100_Info.bus_wbuf = busWriteBurst;
	#else
	W6100_Info.bus_rd = busReadByte;
	W6100_Info.bus_wd = busWriteByte;
	W6100_Info.bus_rbuf = spiDummyReadBurst;
	W6100_Info.bus_wbuf = spiDummyWriteBurst;
	#endif
#endif
	W6100_Info.resetAssert = resetAssert;
	W6100_Info.resetDeassert = resetDeassert;
	wizchip_init_info(&W6100_Info);
	
	W6100Initialze();

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

uint8_t spiDummyReadBurst(uint8_t* pBuf, uint16_t len)
{
	printf("spiReadBurst is not exist\r\n");
	return 0;
}

void spiDummyWriteBurst(uint8_t* pBuf, uint16_t len)
{
	printf("spiWriteBurst is not exist\r\n");
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

void busWriteByte(uint32_t addr, iodata_t data)
{

	(*(volatile uint8_t*)(addr)) = data;
}

iodata_t busReadByte(uint32_t addr)
{
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
