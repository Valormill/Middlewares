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
#define FIRMWARE_BIN_ADDRESS 0x81fc000 // Flash address for firmware.bin
#define STRING_ENTRY_SIZE 20       // Size of each string entry
#define INT_ENTRY_SIZE (sizeof(int) + sizeof(int) + sizeof(int)) // Type + ID + value size

#define MAX_NAME_ID_PAIRS 10  // Maximum number of name-ID pairs

NameIDPair firmwareNameIDStorage[MAX_NAME_ID_PAIRS];
int firmwareNameIDCount = 0;

InitArrayMap firmwareArrayMap = {{}, 0, {}, 0};

// Function to update an integer value based on ID
const char* firmwareUpdateInt(int id, int newValue) {
    if (id < 0) return "Error: Invalid ID";

    bool updated = false;
    for (size_t i = 0; i < firmwareArrayMap.intCount; ++i) {
        if (firmwareArrayMap.intArray[i].id == id) {
            firmwareArrayMap.intArray[i].value = newValue;
            updated = true;
            break;
        }
    }

    if (!updated) {
        return "Error: ID not found in firmware";
    }

    return "Success: Integer value updated in firmware";
}

// Function to update a string value based on ID
const char* firmwareUpdateString(int id, const char* newValue) {
    if (id < 0 || !newValue) return "Error: Invalid ID or value";

    bool updated = false;
    for (size_t i = 0; i < firmwareArrayMap.stringCount; ++i) {
        if (firmwareArrayMap.stringArray[i].id == id) {
            std::strncpy(firmwareArrayMap.stringArray[i].value, newValue, MAX_STRING_LENGTH - 1);
            firmwareArrayMap.stringArray[i].value[MAX_STRING_LENGTH - 1] = '\0';  // Ensure null termination
            updated = true;
            break;
        }
    }

    if (!updated) {
        return "Error: ID not found in firmware";
    }

    return "Success: String value updated in firmware";
}

const char* firmwareWrite(const char* name, int id, char type, const void* data) {
    if (id < 0 || !data) return "not success: invalid ID or data";

    const char* handleResult = firmwareSaveHandles(name, id);
    if (std::strcmp(handleResult, "success: name-ID pair saved for firmware") != 0 &&
        std::strcmp(handleResult, "success: ID updated for existing firmware name") != 0) {
        return handleResult;  // Return the error if saving the handle fails
    }
    switch (type) {
        case 'i':
            firmwareWriteInt(id, *static_cast<const int*>(data));
            return "success: integer data written to firmware";
        case 's':
            firmwareWriteString(id, static_cast<const char*>(data));
            return "success: string data written to firmware";
        default:
            return "not success: unknown data type";
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

int firmwareGetInt(int id) {
    for (size_t i = 0; i < firmwareArrayMap.intCount; ++i) {
        if (firmwareArrayMap.intArray[i].id == id && firmwareArrayMap.intArray[i].type == 0) {
            return firmwareArrayMap.intArray[i].value;
        }
    }
    return -1;
}

const char* firmwareGetString(int id) {
    for (size_t i = 0; i < firmwareArrayMap.stringCount; ++i) {
        if (firmwareArrayMap.stringArray[i].id == id && firmwareArrayMap.stringArray[i].type == 1) {
            return firmwareArrayMap.stringArray[i].value;
        }
    }
    return "-1";
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

const char* loadFirmware() {
    uint8_t byteBuffer[BUFFER_SIZE];  // Statically allocate buffer for raw data
    size_t numberOfWords = BUFFER_SIZE / sizeof(uint32_t);

    const char* result = readAndLoadFlashData("firmware.bin", byteBuffer, numberOfWords);
    if (std::strncmp(result, "Success", 7) != 0) {
        return result;  // Return the error message
    }

    processFirmwareBuffer(byteBuffer, BUFFER_SIZE);
    return "Success: Firmware loaded from flash.";
}

const char* flashFirmware() {
    size_t bufferSize = BUFFER_SIZE;
    uint32_t buffer[BUFFER_SIZE / sizeof(uint32_t)];  // Statically allocate buffer

    firmwareFlush(buffer, bufferSize);  // Flush firmware data to the buffer

    const char* result = fileWrite("firmware.bin", buffer, bufferSize);
    return result;  // Return success or failure message
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

// firmwareOpen: Relays the text-based result from fileOpen
const char* firmwareOpen(const char* str) {
    return fileOpen(str);  // Directly return the message from fileOpen
}

// Function to save name-ID pairs for firmware and return a success or error message
const char* firmwareSaveHandles(const char* name, int id) {
    if (!name || id < 0) return "not success: invalid name or ID";

    for (int i = 0; i < firmwareNameIDCount; ++i) {
        if (std::strcmp(firmwareNameIDStorage[i].name, name) == 0) {
            firmwareNameIDStorage[i].id = id;
            return "success: ID updated for existing firmware name";
        }
    }

    if (firmwareNameIDCount < MAX_NAME_ID_PAIRS) {
        std::strncpy(firmwareNameIDStorage[firmwareNameIDCount].name, name, MAX_STRING_LENGTH - 1);
        firmwareNameIDStorage[firmwareNameIDCount].name[MAX_STRING_LENGTH - 1] = '\0';
        firmwareNameIDStorage[firmwareNameIDCount].id = id;
        ++firmwareNameIDCount;
        return "success: name-ID pair saved for firmware";
    }

    return "not success: firmware name-ID storage is full";
}

// Function to get ID from name
int firmwareGetIDFromName(const char* name) {
    for (int i = 0; i < firmwareNameIDCount; ++i) {
        if (std::strcmp(firmwareNameIDStorage[i].name, name) == 0) {
            return firmwareNameIDStorage[i].id;
        }
    }
    return -1;
}
