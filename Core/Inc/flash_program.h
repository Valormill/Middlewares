/*
 * flash_program.h
 *
 *  Created on: Aug 22, 2024
 *      Author: bem012
 */

#ifndef FLASH_PROGRAM_H
#define FLASH_PROGRAM_H

#include "defs.h"
#include "util.h"

//-----------------------------------------------------------------------------
//
// Public Definitions
//
//-----------------------------------------------------------------------------

#define FLASH_MAGCAL_PAGE  126
#define FLASH_MAGCAL_BANK  FLASH_BANK_2

#define FLASH_USER_PAGE    127
#define FLASH_USER_BANK    FLASH_BANK_2

//-----------------------------------------------------------------------------
//
// Public Functions
//
//-----------------------------------------------------------------------------

uint32_t flash_write(uint32_t StartSectorAddress, uint32_t word,
uint16_t numberofwords);
int flash_read(uint32_t StartSectorAddress, uint32_t *RxBuf,
uint16_t numberofwords);
uint32_t flash_getPage(uint32_t Address);
uint32_t flash_getBank(uint32_t Address);
uint32_t flash_checkProgram(uint32_t StartAddress, uint32_t len, UINT8 *data);
uint32_t flash_getPageAddress(uint32_t bank, uint32_t page);

#endif

#ifdef __cplusplus
extern "C" {
#endif

const char* flash_pageEraseWriteVerify(uint32_t *data, uint32_t size, uint32_t addr);

#ifdef __cplusplus
}
#endif


