/*
 * flash_program.cpp
 *
 *  Created on: Aug 22, 2024
 *      Author: bem012
 */

#include <config.h>
#include <flashFile.h>
#include "flash_program.h"
#include "defs.h"
#include <string>
#include "util.h"
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <vector>


#define FLASH_USER_START_ADDR    0x81fe000  /* Start @ of user Flash area */
#define FLASH_USER_START_ADDR_TWO    0x81fc000  /* Start @ of user Flash area */
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


uint32_t flash_getPageAddress(uint32_t bank, uint32_t page)
{
  bank = bank - 1; // See FLASH_BANK_ definitions
  bank = util_bound(bank, 0, 1);
  page = util_bound(page, 0, 127);

  return FLASH_BASE + (bank * FLASH_BANK_SIZE) + (page * FLASH_PAGE_SIZE);
}

void findPageAndBank(uint32_t address, uint32_t *bank, uint32_t *page)
{
  // Assuming FLASH_BANK_SIZE and FLASH_PAGE_SIZE are defined appropriately
  *bank = (address - FLASH_BASE) / FLASH_BANK_SIZE + 1;
  *page = ((address - FLASH_BASE) % FLASH_BANK_SIZE) / FLASH_PAGE_SIZE;
}

extern "C" {
const char* flash_pageEraseWriteVerify(uint32_t *data, uint32_t size, uint32_t addr)
{
  uint32_t PageError;
  uint32_t iter;
  uint32_t word[4] = { 0, 0, 0, 0 };
  uint32_t address;
  uint32_t written;
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t associatedBank, associatedPage;

  // Find the page and bank based on the address
  if (addr == FLASH_USER_START_ADDR_TWO) {
    findPageAndBank(FLASH_USER_START_ADDR_TWO, &associatedBank, &associatedPage);
  } else if (addr == FLASH_USER_START_ADDR) {
    findPageAndBank(FLASH_USER_START_ADDR, &associatedBank, &associatedPage);
  } else {
    return "Error: Invalid flash address.";
  }

  address = flash_getPageAddress(associatedBank, associatedPage);

  // Disable instruction cache
  if (HAL_ICACHE_Disable() != HAL_OK) {
    return "Error: Failed to disable instruction cache.";
  }

  // Unlock flash
  if (HAL_FLASH_Unlock() != HAL_OK) {
    return "Error: Failed to unlock flash.";
  }

  // Erase the page
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Banks = associatedBank;
  EraseInitStruct.Page = associatedPage;
  EraseInitStruct.NbPages = 1;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
    HAL_FLASH_Lock();
    return "Error: Failed to erase the flash page.";
  }

  // Program the page 1 quadword at a time
  iter = 0;
  written = 0;
  while (written < size) {
    // Build the quad word
    word[0] = data[iter + 0];
    word[1] = data[iter + 1];
    word[2] = data[iter + 2];
    word[3] = data[iter + 3];

    // Write the quad word
    flash_write(address, (uint32_t) word, 0);

    // Increment the trackers
    iter = iter + 4; // 4 words written
    address += (4 * sizeof(uint32_t)); // 16 bytes written
    written = written + (4 * sizeof(uint32_t)); // 16 bytes written
  }

  // Lock the flash
  if (HAL_FLASH_Lock() != HAL_OK) {
    return "Error: Failed to lock flash.";
  }

  // Enable instruction cache
  if (HAL_ICACHE_Enable() != HAL_OK) {
    return "Error: Failed to enable instruction cache.";
  }

  // Verify the data in the page
  address = flash_getPageAddress(associatedBank, associatedPage);
  if (flash_checkProgram(address, size, (uint8_t*) data)) {
    return "Error: Flash program verification failed.";
  } else {
    return "Success: Flash program successful.";
  }
}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uint32_t flash_write(uint32_t StartSectorAddress, uint32_t word, uint16_t numberofwords)
{

  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, StartSectorAddress,
      ((uint32_t) word)) == HAL_OK) {
    StartSectorAddress += 16;
  } else {
    while (1) {
      Error_Handler();
    }
  }

  return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int flash_read(uint32_t StartSectorAddress, uint32_t *RxBuf, uint16_t numberofwords)
{
    uint32_t start = numberofwords;
    while (1) {
        RxBuf[start - numberofwords] = *(__IO uint32_t*) StartSectorAddress;
        StartSectorAddress += 4;
        if (!(numberofwords--))
            break;
    }

    // Returning success for now, modify this logic based on your actual hardware check
    return 0;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uint32_t flash_checkProgram(uint32_t StartAddress, uint32_t len, uint8_t *data)
{
  return util_memcmp((uint8_t*) StartAddress, data, len);
}




