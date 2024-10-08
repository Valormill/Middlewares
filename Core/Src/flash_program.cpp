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
#include "main.h"
#include <string>
#include "util.h"
#include <iostream>
#include "debug.h"  // Ensure this file contains the declaration of debug_msg
#include <stdio.h>
#include <cstring> // For std::memcpy
#include <vector>
#include "stm32u5xx_hal.h"

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
int flash_pageEraseWriteVerify(uint32_t *data, uint32_t size, uint32_t addr)
{
    uint32_t PageError;
    uint32_t iter;
    uint32_t word[4] = { 0, 0, 0, 0 };
    uint32_t address;
    uint32_t written;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t associatedBank, associatedPage;

    // Find the page and bank based on the provided address
    findPageAndBank(addr, &associatedBank, &associatedPage);

    address = flash_getPageAddress(associatedBank, associatedPage);

//    // Disable instruction cache
//    if (HAL_ICACHE_Disable() != HAL_OK) {
//        return 1;
//    }

    // Unlock flash
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return 1;
    }

    // Erase the page
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks = associatedBank;
    EraseInitStruct.Page = associatedPage;
    EraseInitStruct.NbPages = 1;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return 1;
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
        iter += 4; // 4 words written
        address += (4 * sizeof(uint32_t)); // 16 bytes written
        written += (4 * sizeof(uint32_t)); // 16 bytes written
    }

    // Lock the flash
    if (HAL_FLASH_Lock() != HAL_OK) {
        return 1;
    }

//    // Enable instruction cache
//    if (HAL_ICACHE_Enable() != HAL_OK) {
//        return 1;
//    }

    // Verify the data in the page
    address = flash_getPageAddress(associatedBank, associatedPage);
    if (flash_checkProgram(address, size, (uint8_t*) data)) {
        return 1; // Verification failed
    }
    return 0; // Success
}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uint32_t flash_write(uint32_t StartSectorAddress, uint32_t word, uint16_t numberofwords)
{
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, StartSectorAddress, (uint32_t)word) == HAL_OK) {
        StartSectorAddress += 16; // Move to the next quadword
    } else {
        while (1) {
            Error_Handler(); // Handle error appropriately
        }
    }

    return 0; // Success
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int flash_read(uint32_t StartSectorAddress, uint32_t *RxBuf, uint16_t numberofwords)
{
    uint32_t start = numberofwords;
    while (1) {
        RxBuf[start - numberofwords] = *(__IO uint32_t*)StartSectorAddress;
        StartSectorAddress += 4; // Move to the next word
        if (!(numberofwords--))
            break;
    }

    return 0; // Success
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uint32_t flash_checkProgram(uint32_t StartAddress, uint32_t len, uint8_t *data)
{
    return util_memcmp((uint8_t*)StartAddress, data, len);
}



