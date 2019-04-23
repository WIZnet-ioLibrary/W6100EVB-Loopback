#include "wizchip_init.h"

wiz_InitInfo myW6100;

void W6100Initialze(void)
{
	myW6100.resetAssert();
	delay(10);
	myW6100.resetDeassert();
	delay(10);

	io6LibraryCallBack();

	intr_kind temp;
	unsigned char W6100_AdrSet[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
	do
	{
		if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
		{
			printf("Unknown PHY link status.\r\n");
		}
	} while (temp == PHY_LINK_OFF);
	printf("PHY OK.\r\n");

	temp = IK_DEST_UNREACH;

	if (ctlwizchip(CW_INIT_WIZCHIP, (void *)W6100_AdrSet) == -1)
	{
		printf("W6100 initialized fail.\r\n");
	}

	if (ctlwizchip(CW_SET_INTRMASK, &temp) == -1)
	{
		printf("W6100 interrupt\r\n");
	}
	//printf("interrupt mask: %02x\r\n",getIMR());
}

void io6LibraryCallBack(void)
{
#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
/* SPI method callback registration */
#if defined SPI_DMA
	reg_wizchip_spi_cbfunc(myW6100.spi_rb, myW6100.spi_wb, myW6100.spi_rbuf, myW6100.spi_wbuf);
#else
	reg_wizchip_spi_cbfunc(myW6100.spi_rb, myW6100.spi_wb, 0, 0);
#endif
	/* CS function register */
	reg_wizchip_cs_cbfunc(myW6100.cs_sel, myW6100.cs_desel);
#else
/* Indirect bus method callback registration */
#if defined BUS_DMA
	reg_wizchip_bus_cbfunc(myW6100.bus_rd, myW6100.bus_wd, myW6100.bus_rbuf, myW6100.bus_wbuf);
#else
	reg_wizchip_bus_cbfunc(myW6100.bus_rd, myW6100.bus_wd, 0, 0);
#endif
#endif
}

void W6100Reset(void)
{
	myW6100.resetAssert();
	delay(10);
	myW6100.resetDeassert();
	delay(10);
}

#ifdef WIZCHIP_REGISTER_DUMP

void register_read(void)
{
	int i;
	printf("                    ----register read----\r\n");
	printf("Address | ");
	for (i = 0; i < 16; i++)
		printf("%02x ", i);
	printf("\r\n---------------------------------------------------------");
	for (i = 0; i < 0x0090; i++)
	{
		if (i % 16 == 0)
			printf("\r\n  %04x  | ", i);
		printf("%02x ", WIZCHIP_READ(_W6100_IO_BASE_ + (i << 8) + (WIZCHIP_CREG_BLOCK << 3)));
	}
	printf("\r\n");
}

void socket_register_read(uint8_t sn)
{
	int i;
	printf("                    ----Sn register read----\r\n");
	printf("Address | ");
	for (i = 0; i < 16; i++)
		printf("%02x ", i);
	printf("\r\n---------------------------------------------------------");
	for (i = 0x400 + (sn * (0x100)); i < 0x400 + (sn * (0x100) + 0x35); i++)
	{
		if (i % 16 == 0)
			printf("\r\n0x%04x  | ", i);
		printf("%02x ", WIZCHIP_READ(i));
	}
	printf("\r\n");
}

#endif
