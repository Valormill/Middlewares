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
#include "main.h"
#include <string>
#include "util.h"
#include <iostream>
#include "debug.h"
#include <stdio.h>
#include <cstring>
#include <vector>
#include "firmware.h"

// Define constants for easy modification
#define MAX_SAVED_HANDLES 10        // Maximum number of saved handles
#define MAX_HANDLE_NAME_LENGTH 20   // Maximum length of handle name (including null terminator)

// Array to store saved handles
char savedHandles[MAX_SAVED_HANDLES][MAX_HANDLE_NAME_LENGTH];
int savedHandlesCount = 0;

int readAndLoadFlashData(const char* handle, uint8_t* data, size_t& size, uint32_t addr)
{
    int result;

    // Use the provided address instead of hardcoded addresses
    result = flash_read(addr, reinterpret_cast<uint32_t*>(data), size);
    return (result == 0) ? 0 : 1;  // Return 0 for success, 1 for failure
}

// Implementation of the fileOpen function with numeric return
int fileOpen(const char* handle) {
    bool inUse = false;
    for (int i = 0; i < savedHandlesCount; ++i) {
        if (std::strcmp(savedHandles[i], handle) == 0) {
            inUse = true;
            break;
        }
    }

    if (inUse) {
        return 1;  // Handle is already in use
    }

    if (std::strcmp(handle, "config.bin") == 0 || std::strcmp(handle, "firmware.bin") == 0) {
        if (savedHandlesCount < MAX_SAVED_HANDLES) {
            std::strncpy(savedHandles[savedHandlesCount], handle, MAX_HANDLE_NAME_LENGTH - 1);
            savedHandles[savedHandlesCount][MAX_HANDLE_NAME_LENGTH - 1] = '\0';  // Ensure null-termination
            ++savedHandlesCount;
            return 0;  // Handle opened successfully
        }
        return 1;  // Max handles reached
    }

    return 1;  // Unknown handle
}

int fileWrite(const char* handle, uint32_t* data, size_t size, uint32_t addr) {
    int result;

    // Use the provided address instead of hardcoded addresses
    result = flash_pageEraseWriteVerify(data, size, addr);
    return (result == 0) ? 0 : 1;  // Return 0 for success, 1 for failure
}


