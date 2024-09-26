/*
 * firmware.h
 *
 *  Created on: Sep 3, 2024
 *      Author: bem012
 */

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <flashFile.h>
#include <InitArrayMap.h>
#include <cstdint>
#include <vector>
#include <string>

// Functions to write firmware data with success/error messages
int firmwareWrite(const char* name, int id, char type, const void* data);
void firmwareWriteInt(int id, int value);
void firmwareWriteString(int id, const char* str);
int firmwareUpdateString(int id, const char* newValue);

// Functions to retrieve firmware data with success/error messages
int firmwareGetInt(int id);       // Returns success/error message
const char* firmwareGetString(int id);  // Returns success/error message

// Flash and load operations with success/error messages
int flashFirmware(uint32_t address);     // Flushes data to flash
int loadFirmware(uint32_t address);      // Loads data from flash
void processFirmwareBuffer(uint8_t* bufferPtr, size_t bufferSize);

// Handle management
int firmwareOpen(const char* str);  // Open file, returns success or error message
int firmwareSaveHandles(const char* name, int id);  // Save handle and return status
int firmwareGetIDFromName(const char* name); // Get ID by name

#endif // FIRMWARE_H

