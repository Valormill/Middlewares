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
const char* firmwareWrite(const char* name, int id, char type, const void* data);
void firmwareWriteInt(int id, int value);
void firmwareWriteString(int id, const char* str);
const char* firmwareUpdateString(int id, const char* newValue);
const char* firmwareUpdateString(int id, const char* newValue);

// Functions to retrieve firmware data with success/error messages
int firmwareGetInt(int id);       // Returns success/error message
const char* firmwareGetString(int id);  // Returns success/error message

// Flash and load operations with success/error messages
const char* flashFirmware();     // Flushes data to flash
const char* loadFirmware();      // Loads data from flash
void processFirmwareBuffer(uint8_t* bufferPtr, size_t bufferSize);

// Handle management
const char* firmwareOpen(const char* str);  // Open file, returns success or error message
const char* firmwareSaveHandles(const char* name, int id);  // Save handle and return status
int firmwareGetIDFromName(const char* name); // Get ID by name

#endif // FIRMWARE_H

