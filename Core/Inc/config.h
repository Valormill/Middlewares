/*
 * config.h
 *
 *  Created on: Aug 22, 2024
 *      Author: bem012
 */

#ifndef FLASH_SIMULATION_H
#define FLASH_SIMULATION_H

#include <flashFile.h>
#include <cstdint>
#include <vector>
#include <string>

// Functions to write configuration data with success/error messages
int configWrite(const char* name, int id, char type, const void* data);
void configWriteInt(int id, int value);
void configWriteString(int id, const char* str);
int configUpdateString(int id, const char* newValue);

// Functions to retrieve configuration data with success/error messages
int configGetInt(int id);       // Returns success/error message
const char* configGetString(int id);  // Returns success/error message

// Flash and load operations with success/error messages
int flashConfig(uint32_t address);     // Flushes data to flash
int loadConfig(uint32_t address);      // Loads data from flash
void processConfigBuffer(uint8_t* bufferPtr, size_t bufferSize);

// Handle management
int configOpen(const char* str);  // Open file, returns success or error message
int configSaveHandles(const char* name, int id);  // Save handle and return status
int configGetIDFromName(const char* name); // Get ID by name

#endif // FLASH_SIMULATION_H
