/*
 * board_init.h
 *
 *  Created on: Apr 19, 2019
 *      Author: taylor
 */

#ifndef __BOARD_INIT_H__
#define __BOARD_INIT_H__

#include "mcu_init.h"
#ifdef USE_STDPERIPH_DRIVER
	#warning USE_STDPERIPH_DRIVER

#include "stm32f10x_conf.h"
#include "serialCommand.h"
#define DMA

#elif defined USE_HAL_DRIVER
	#warning USE_HAL_DRIVER
#include "main.h"

/* RESET */
#define W6100_RESET_PIN		    GPIO_PIN_8
#define W6100_RESET_PORT		GPIOD

/* SPI2 */
#define W6100_CS_PIN			GPIO_PIN_7
#define W6100_CS_PORT			GPIOD

#define W6100_SPI_SIZE          1
#define W6100_SPI_TIMEOUT       10
void spi_set_func(SPI_HandleTypeDef *SPI_n);
#else
	#error Error!! STD_DRIVER not defined

#endif

#include "wizchip_conf.h"
#include "wizchip_init.h"

#if (_WIZCHIP_IO_MODE_==_WIZCHIP_IO_MODE_BUS_INDIR_)
	#ifdef DMA
   	   #define BUS_DMA
	#endif
#elif(_WIZCHIP_IO_MODE_== _WIZCHIP_IO_MODE_SPI_VDM_)||(_WIZCHIP_IO_MODE_== _WIZCHIP_IO_MODE_SPI_FDM_)
	#ifdef DMA
		#define SPI_DMA
	#endif
#endif

void BoardInitialze(void);

uint8_t spiReadByte(void);
void spiWriteByte(uint8_t byte);

#if defined SPI_DMA
uint8_t spiReadBurst(uint8_t* pBuf, uint16_t len);
void spiWriteBurst(uint8_t* pBuf, uint16_t len);
#endif

iodata_t busReadByte(uint32_t addr);
void busWriteByte(uint32_t addr, iodata_t data);

#if defined BUS_DMA
void busReadBurst(uint32_t addr,uint8_t* pBuf, uint32_t len,uint8_t addr_inc);
void busWriteBurst(uint32_t addr, uint8_t* pBuf ,uint32_t len,uint8_t addr_inc);
#endif

void csEnable(void);
void csDisable(void);
void resetAssert(void);
void resetDeassert(void);

void W6100Reset(void);

//todo for test
void register_read(void);
void socket_register_read(uint8_t sn);

#endif /* W6100EVB_H_ */
