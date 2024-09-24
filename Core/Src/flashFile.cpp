/*
 * flashFile.cpp
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
#include "firmware.h"

// Define constants for easy modification
#define MAX_SAVED_HANDLES 10        // Maximum number of saved handles
#define MAX_HANDLE_NAME_LENGTH 20   // Maximum length of handle name (including null terminator)
#define CONFIG_FLASH_ADDRESS 0x81fe000 // Flash memory address for config.bin
#define FIRMWARE_FLASH_ADDRESS 0x81fc000 // Flash memory address for firmware.bin

// Array to store saved handles
char savedHandles[MAX_SAVED_HANDLES][MAX_HANDLE_NAME_LENGTH];
int savedHandlesCount = 0;

const char* readAndLoadFlashData(const char* handle, uint8_t* data, size_t& size)
{
    int result;

    if (std::strcmp(handle, "config.bin") == 0) {
        result = flash_read(CONFIG_FLASH_ADDRESS, reinterpret_cast<uint32_t*>(data), size);
        if (result == 0) {
            return "Success: Config data read from flash.";
        } else {
            return "Error: Failed to read config data from flash.";
        }
    } else if (std::strcmp(handle, "firmware.bin") == 0) {
        result = flash_read(FIRMWARE_FLASH_ADDRESS, reinterpret_cast<uint32_t*>(data), size);
        if (result == 0) {
            return "Success: Firmware data read from flash.";
        } else {
            return "Error: Failed to read firmware data from flash.";
        }
    }

    return "Error: Unknown handle.";
}

// Implementation of the fileOpen function with text-based return
const char* fileOpen(const char* handle) {
    bool inUse = false;
    for (int i = 0; i < savedHandlesCount; ++i) {
        if (std::strcmp(savedHandles[i], handle) == 0) {
            inUse = true;
            break;
        }
    }

    if (inUse) {
        return "Error: Handle is already in use.";
    }

    if (std::strcmp(handle, "config.bin") == 0 || std::strcmp(handle, "firmware.bin") == 0) {
        if (savedHandlesCount < MAX_SAVED_HANDLES) {
            std::strncpy(savedHandles[savedHandlesCount], handle, MAX_HANDLE_NAME_LENGTH - 1);
            savedHandles[savedHandlesCount][MAX_HANDLE_NAME_LENGTH - 1] = '\0';  // Ensure null-termination
            ++savedHandlesCount;
            return "Success: Handle opened.";
        }
        return "Error: Max handles reached.";
    }

    return "Error: Unknown handle.";
}

const char* fileWrite(const char* handle, uint32_t* data, size_t size) {
    const char* result;

    if (std::strcmp(handle, "config.bin") == 0) {
        result = flash_pageEraseWriteVerify(data, size, CONFIG_FLASH_ADDRESS);
        // Return the result of the write operation
        if (std::strncmp(result, "Success", 7) == 0) {
            return "Success: Config data written to flash.";
        } else {
            return result;  // Return the specific error from flash_pageEraseWriteVerify
        }
    } else if (std::strcmp(handle, "firmware.bin") == 0) {
        result = flash_pageEraseWriteVerify(data, size, FIRMWARE_FLASH_ADDRESS);
        // Return the result of the write operation
        if (std::strncmp(result, "Success", 7) == 0) {
            return "Success: Firmware data written to flash.";
        } else {
            return result;  // Return the specific error from flash_pageEraseWriteVerify
        }
    }

    return "Error: Unknown handle.";
}



