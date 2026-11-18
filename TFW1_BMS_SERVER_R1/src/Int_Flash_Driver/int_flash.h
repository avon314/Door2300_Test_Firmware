/*****************************************************************************
* Module Name	: int_flash.h
* Description	: Minimal internal-flash (EEFC) erase/write for the NV-seed slots.
*****************************************************************************/
#ifndef INT_FLASH_H_
#define INT_FLASH_H_

#include <stdint.h>

int Int_Flash_Erase_4K(uint32_t addr);
int Int_Flash_Write_Page(uint32_t addr, const void *data, uint32_t len);

#endif /* INT_FLASH_H_ */
