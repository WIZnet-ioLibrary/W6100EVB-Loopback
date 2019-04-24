#include "board_init.h"
#include "wizchip_init.h"

#if defined USE_HAL_DRIVER
SPI_HandleTypeDef W6100_SPI;
void spi_set_func(SPI_HandleTypeDef *SPI_n)
{
	W6100_SPI = *SPI_n;
}
#endif

void BoardInitialze(void)
{

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

	W6100Initialze();

}

