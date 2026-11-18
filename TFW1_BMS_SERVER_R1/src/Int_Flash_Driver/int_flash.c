/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: int_flash.c
* Description	: Minimal internal-flash (EEFC) driver: unlock, erase 8 pages,
*				  write one page. Used only for the TLS NV-seed slots in the last
*				  8 KB lock region (0x004FE000 - 0x004FFFFF).
*
* Notes			: The SAM4E has a single flash bank, so code must not execute from
*				  flash while a command runs. The command/poll routine is placed in
*				  RAM (.ramfunc, copied by the startup code via .relocate) and
*				  interrupts are disabled for the duration of each command
*				  (interrupt vectors and handlers live in flash).
*				  EPA with 8 pages (FARG[1:0] = 1) - confirm against the SAM4E
*				  datasheet "Erase Pages" table for the last (large) sector.
*****************************************************************************/
#include <string.h>
#include "asf.h"
#include "int_flash.h"

#define EFC_ERR_MASK	(EEFC_FSR_FCMDE | EEFC_FSR_FLOCKE | EEFC_FSR_FLERR)
#define EPA_8_PAGES		(1u)

/* Runs from RAM: start the command and wait until the flash is ready again. */
RAMFUNC __attribute__((noinline)) static uint32_t efc_cmd_ram(uint32_t fcr)
{
	uint32_t status;

	EFC->EEFC_FCR = fcr;
	do
	{
		status = EFC->EEFC_FSR;
	} while ((status & EEFC_FSR_FRDY) != EEFC_FSR_FRDY);

	return status & EFC_ERR_MASK;
}

static uint32_t efc_cmd(uint32_t cmd, uint32_t arg)
{
	uint32_t primask = __get_PRIMASK();
	uint32_t status;

	__disable_irq();
	while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) != EEFC_FSR_FRDY)
	{
		/* wait for any previous operation */
	}
	status = efc_cmd_ram(EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FARG(arg) | cmd);
	if (primask == 0u)
	{
		__enable_irq();
	}
	return status;
}

static uint32_t page_number(uint32_t addr)
{
	return (addr - IFLASH_ADDR) / IFLASH_PAGE_SIZE;
}

/*****************************************************************************
* Function name	: int Int_Flash_Erase_4K(uint32_t addr)
* Returns		: 0 = OK, -1 = error.
* Arguments		: addr - 4 KB aligned flash address.
* Description	: Clears the lock bit of the region, then erases 8 pages (4 KB).
*****************************************************************************/
int Int_Flash_Erase_4K(uint32_t addr)
{
	if ((addr < IFLASH_ADDR) || ((addr & 0xFFFu) != 0u))
	{
		return -1;
	}
	if (efc_cmd(EEFC_FCR_FCMD_CLB, page_number(addr)) != 0u)
	{
		return -1;
	}
	return (efc_cmd(EEFC_FCR_FCMD_EPA, page_number(addr) | EPA_8_PAGES) == 0u) ? 0 : -1;
}

/*****************************************************************************
* Function name	: int Int_Flash_Write_Page(uint32_t addr, const void *data, uint32_t len)
* Returns		: 0 = OK, -1 = error.
* Arguments		: addr - page aligned address of an ERASED page.
*				  data/len - up to IFLASH_PAGE_SIZE bytes; the rest of the page
*				  stays 0xFF.
* Description	: Fills the page latch buffer with 32-bit writes, then programs it.
*****************************************************************************/
int Int_Flash_Write_Page(uint32_t addr, const void *data, uint32_t len)
{
	static uint32_t page[IFLASH_PAGE_SIZE / 4u];
	volatile uint32_t *dst = (volatile uint32_t *)addr;
	uint32_t i;

	if ((addr < IFLASH_ADDR) || ((addr % IFLASH_PAGE_SIZE) != 0u) || (len > IFLASH_PAGE_SIZE))
	{
		return -1;
	}
	memset(page, 0xFF, sizeof(page));
	memcpy(page, data, len);

	for (i = 0u; i < (IFLASH_PAGE_SIZE / 4u); i++)
	{
		dst[i] = page[i];				/* goes to the latch buffer */
	}
	__DSB();

	return (efc_cmd(EEFC_FCR_FCMD_WP, page_number(addr)) == 0u) ? 0 : -1;
}
