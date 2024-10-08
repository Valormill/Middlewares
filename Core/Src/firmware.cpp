/*
 * firmware.cpp
 *
 *  Created on: Sep 3, 2024
 *      Author: bem012
 */

#include "firmware.h"
#include "InitArrayMap.h"
#include <cstring>
#include <iostream>
#include "flashFile.h"

// Define potential constants that might need to be changed
#define MAX_INT_COUNT 5            // Maximum number of integers in firmware
#define MAX_STRING_COUNT 5         // Maximum number of strings in firmware
#define BUFFER_SIZE 256            // Default buffer size for loading and flushing
#define STRING_ENTRY_SIZE 20       // Size of each string entry
#define INT_ENTRY_SIZE (sizeof(int) + sizeof(int) + sizeof(int)) // Type + ID + value size

#define MAX_NAME_ID_PAIRS 10       // Maximum number of name-ID pairs

NameIDPair firmwareNameIDStorage[MAX_NAME_ID_PAIRS];
int firmwareNameIDCount = 0;

InitArrayMap firmwareArrayMap = {{}, 0, {}, 0};

// Function to update an integer value based on ID
int firmwareUpdateInt(int id, int newValue) {
    if (id < 0) return 1; // Invalid ID

    bool updated = false;
    for (size_t i = 0; i < firmwareArrayMap.intCount; ++i) {
        if (firmwareArrayMap.intArray[i].id == id) {
            firmwareArrayMap.intArray[i].value = newValue;
            updated = true;
            break;
        }
    }

    return updated ? 0 : 1; // Return 0 for success, 1 for ID not found
}

// Function to update a string value based on ID
int firmwareUpdateString(int id, const char* newValue) {
    if (id < 0 || !newValue) return 1; // Invalid ID or value

    bool updated = false;
    for (size_t i = 0; i < firmwareArrayMap.stringCount; ++i) {
        if (firmwareArrayMap.stringArray[i].id == id) {
            std::strncpy(firmwareArrayMap.stringArray[i].value, newValue, MAX_STRING_LENGTH - 1);
            firmwareArrayMap.stringArray[i].value[MAX_STRING_LENGTH - 1] = '\0';  // Ensure null termination
            updated = true;
            break;
        }
    }

    return updated ? 0 : 1; // Return 0 for success, 1 for ID not found
}

int firmwareWrite(const char* name, int id, char type, const void* data) {
    if (id < 0 || !data) return 1; // Invalid ID or data

    int handleResult = firmwareSaveHandles(name, id);
    if (handleResult != 0) {
        return handleResult;  // Return the error if saving the handle fails
    }
    switch (type) {
        case 'i':
            firmwareWriteInt(id, *static_cast<const int*>(data));
            return 0; // Success
        case 's':
            firmwareWriteString(id, static_cast<const char*>(data));
            return 0; // Success
        default:
            return 1; // Unknown data type
    }
}

void firmwareWriteInt(int id, int value) {
    bool replaced = false;
    for (size_t i = 0; i < firmwareArrayMap.intCount; ++i) {
        if (firmwareArrayMap.intArray[i].id == id) {
            firmwareArrayMap.intArray[i].value = value;
            replaced = true;
            break;
        }
    }
    if (!replaced && firmwareArrayMap.intCount < MAX_INT_COUNT) {
        firmwareArrayMap.intArray[firmwareArrayMap.intCount++] = IntEntry(id, value);
    }
}

void firmwareWriteString(int id, const char* str) {
    bool replaced = false;
    for (size_t i = 0; i < firmwareArrayMap.stringCount; ++i) {
        if (firmwareArrayMap.stringArray[i].id == id) {
            std::strncpy(firmwareArrayMap.stringArray[i].value, str, MAX_STRING_LENGTH - 1);
            firmwareArrayMap.stringArray[i].value[MAX_STRING_LENGTH - 1] = '\0';
            replaced = true;
            break;
        }
    }
    if (!replaced && firmwareArrayMap.stringCount < MAX_STRING_COUNT) {
        firmwareArrayMap.stringArray[firmwareArrayMap.stringCount++] = StringEntry(id, str);
    }
}

void firmwareFlush(uint32_t* buffer, size_t& bufferSize) {
    size_t intArraySize = firmwareArrayMap.intCount * INT_ENTRY_SIZE;
    size_t stringArraySize = firmwareArrayMap.stringCount * (sizeof(int) + sizeof(int) + MAX_STRING_LENGTH); // Type + ID + value
    bufferSize = intArraySize + stringArraySize + 2 * sizeof(uint32_t); // Handle + int and string counts

    uint32_t* bufferPtr = buffer;

    *bufferPtr++ = firmwareArrayMap.intCount;
    *bufferPtr++ = firmwareArrayMap.stringCount;

    for (size_t i = 0; i < firmwareArrayMap.intCount; ++i) {
        std::memcpy(bufferPtr, &firmwareArrayMap.intArray[i].type, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, &firmwareArrayMap.intArray[i].id, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, &firmwareArrayMap.intArray[i].value, sizeof(int));
        bufferPtr += 1;
    }

    for (size_t i = 0; i < firmwareArrayMap.stringCount; ++i) {
        std::memcpy(bufferPtr, &firmwareArrayMap.stringArray[i].type, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, &firmwareArrayMap.stringArray[i].id, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, firmwareArrayMap.stringArray[i].value, MAX_STRING_LENGTH);
        bufferPtr += MAX_STRING_LENGTH / 4;
    }
}

int loadFirmware(uint32_t address) {
    uint8_t byteBuffer[BUFFER_SIZE];  // Statically allocate buffer for raw data
    size_t numberOfWords = BUFFER_SIZE / sizeof(uint32_t);

    int result = readAndLoadFlashData(byteBuffer, numberOfWords, address);
    if (result != 0) {
        return result;  // Return the error code
    }

    processFirmwareBuffer(byteBuffer, BUFFER_SIZE);
    return 0; // Success
}

int flashFirmware(uint32_t address) {
    size_t bufferSize = BUFFER_SIZE;
    uint32_t buffer[BUFFER_SIZE / sizeof(uint32_t)];  // Statically allocate buffer

    firmwareFlush(buffer, bufferSize);  // Flush firmware data to the buffer

    int result = fileWrite(buffer, bufferSize, address);
    return result;  // Return success or failure code
}

void processFirmwareBuffer(uint8_t* bufferPtr, size_t bufferSize) {
    uint32_t intCount = 0;
    uint32_t stringCount = 0;
    std::memcpy(&intCount, bufferPtr, sizeof(uint32_t));
    bufferPtr += sizeof(uint32_t);
    std::memcpy(&stringCount, bufferPtr, sizeof(uint32_t));
    bufferPtr += sizeof(uint32_t);

    for (size_t i = 0; i < intCount + stringCount && bufferPtr < bufferPtr + bufferSize; ++i) {
        int type = 0;
        std::memcpy(&type, bufferPtr, sizeof(int));
        bufferPtr += sizeof(int);

        int id = 0;
        std::memcpy(&id, bufferPtr, sizeof(int));
        bufferPtr += sizeof(int);

        if (type == 0) { // It's an integer
            int value = 0;
            std::memcpy(&value, bufferPtr, sizeof(int));
            bufferPtr += sizeof(int);

            bool replaced = false;
            for (size_t j = 0; j < firmwareArrayMap.intCount; ++j) {
                if (firmwareArrayMap.intArray[j].id == id) {
                    firmwareArrayMap.intArray[j].value = value;
                    replaced = true;
                    break;
                }
            }
            if (!replaced && firmwareArrayMap.intCount < MAX_INT_COUNT) {
                firmwareArrayMap.intArray[firmwareArrayMap.intCount++] = IntEntry(id, value);
            }
        } else if (type == 1) { // It's a string
            char value[MAX_STRING_LENGTH] = {0};
            std::memcpy(value, bufferPtr, STRING_ENTRY_SIZE);
            bufferPtr += STRING_ENTRY_SIZE;

            bool replaced = false;
            for (size_t j = 0; j < firmwareArrayMap.stringCount; ++j) {
                if (firmwareArrayMap.stringArray[j].id == id) {
                    std::strncpy(firmwareArrayMap.stringArray[j].value, value, MAX_STRING_LENGTH - 1);
                    firmwareArrayMap.stringArray[j].value[MAX_STRING_LENGTH - 1] = '\0';
                    replaced = true;
                    break;
                }
            }
            if (!replaced && firmwareArrayMap.stringCount < MAX_STRING_COUNT) {
                firmwareArrayMap.stringArray[firmwareArrayMap.stringCount++] = StringEntry(id, value);
            }
        }
    }
}

// firmwareOpen: Relays the result from fileOpen
int firmwareOpen(const char* str) {
    return fileOpen(str);  // Directly return the code from fileOpen
}

// Function to save name-ID pairs for firmware and return a success or error message
int firmwareSaveHandles(const char* name, int id) {
    if (!name || id < 0) return 1; // Invalid name or ID

    for (int i = 0; i < firmwareNameIDCount; ++i) {
        if (std::strcmp(firmwareNameIDStorage[i].name, name) == 0) {
            firmwareNameIDStorage[i].id = id;
            return 0; // Success: ID updated for existing firmware name
        }
    }

    if (firmwareNameIDCount < MAX_NAME_ID_PAIRS) {
        std::strncpy(firmwareNameIDStorage[firmwareNameIDCount].name, name, MAX_STRING_LENGTH - 1);
        firmwareNameIDStorage[firmwareNameIDCount].name[MAX_STRING_LENGTH - 1] = '\0';
        firmwareNameIDStorage[firmwareNameIDCount].id = id;
        ++firmwareNameIDCount;
        return 0; // Success: name-ID pair saved for firmware
    }

    return 1; // Error: firmware name-ID storage is full
}

// Function to get ID from name
int firmwareGetIDFromName(const char* name) {
    for (int i = 0; i < firmwareNameIDCount; ++i) {
        if (std::strcmp(firmwareNameIDStorage[i].name, name) == 0) {
            return firmwareNameIDStorage[i].id;
        }
    }
    return -1; // Return -1 to indicate failure
}

