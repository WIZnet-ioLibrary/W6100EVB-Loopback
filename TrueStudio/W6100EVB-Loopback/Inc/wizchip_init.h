#ifndef __WIZCHIP_INIT_H__
#define __WIZCHIP_INIT_H__

//#define WIZCHIP_REGISTER_DUMP

#include "board_init.h"

typedef struct wiz_InitInfo_t
{
    uint8_t (*spi_rb)(void);
    void (*spi_wb)(uint8_t);
    void (*spi_rbuf)(uint8_t*, datasize_t);
    void (*spi_wbuf)(uint8_t*, datasize_t);
    void (*cs_sel)(void);
    void (*cs_desel)(void);

    iodata_t (*bus_rd)(uint32_t); 
    void (*bus_wd)(uint32_t, iodata_t);
    void (*bus_rbuf)(uint32_t, uint8_t*, datasize_t, uint8_t);
    void (*bus_wbuf)(uint32_t, uint8_t*, datasize_t, uint8_t);

    void (*resetAssert)(void);
    void (*resetDeassert)(void);

}wiz_InitInfo;

void W6100Initialze(void);
void io6LibraryCallBack(void);
void W6100Reset(void);

#ifdef WIZCHIP_REGISTER_DUMP
void register_read(void);
void socket_register_read(uint8_t sn);
#endif

#endif
