/*
 * config.cpp
 *
 *  Created on: Aug 22, 2024
 *      Author: bem012
 */

#include "config.h"
#include "InitArrayMap.h"
#include <cstring>
#include <iostream>

// Define potential constants that might need to be changed
#define MAX_INT_COUNT 5            // Maximum number of integers in config
#define MAX_STRING_COUNT 5         // Maximum number of strings in config
#define BUFFER_SIZE 256            // Default buffer size for loading and flushing
#define CONFIG_BIN_ADDRESS 0x81fe000 // Flash address for config.bin
#define STRING_ENTRY_SIZE 20       // Size of each string entry
#define INT_ENTRY_SIZE (sizeof(int) + sizeof(int) + sizeof(int)) // Type + ID + value size
#define MAX_NAME_ID_PAIRS 10  // Maximum number of name-ID pairs

NameIDPair configNameIDStorage[MAX_NAME_ID_PAIRS];
int configNameIDCount = 0;

InitArrayMap configArrayMap = {{}, 0, {}, 0};

// Function to update an integer value based on ID
const char* configUpdateInt(int id, int newValue) {
    if (id < 0) return "Error: Invalid ID";

    bool updated = false;
    for (size_t i = 0; i < configArrayMap.intCount; ++i) {
        if (configArrayMap.intArray[i].id == id) {
            configArrayMap.intArray[i].value = newValue;
            updated = true;
            break;
        }
    }

    if (!updated) {
        return "Error: ID not found in config";
    }

    return "Success: Integer value updated in config";
}

// Function to update a string value based on ID
const char* configUpdateString(int id, const char* newValue) {
    if (id < 0 || !newValue) return "Error: Invalid ID or value";

    bool updated = false;
    for (size_t i = 0; i < configArrayMap.stringCount; ++i) {
        if (configArrayMap.stringArray[i].id == id) {
            std::strncpy(configArrayMap.stringArray[i].value, newValue, MAX_STRING_LENGTH - 1);
            configArrayMap.stringArray[i].value[MAX_STRING_LENGTH - 1] = '\0';  // Ensure null termination
            updated = true;
            break;
        }
    }

    if (!updated) {
        return "Error: ID not found in config";
    }

    return "Success: String value updated in config";
}

const char* configWrite(const char* name, int id, char type, const void* data) {
    if (id < 0 || !data) return "not success: invalid ID or data";

    const char* handleResult = configSaveHandles(name, id);
    if (std::strcmp(handleResult, "success: name-ID pair saved for config") != 0 &&
        std::strcmp(handleResult, "success: ID updated for existing config name") != 0) {
        return handleResult;  // Return the error if saving the handle fails
    }
    switch (type) {
        case 'i':
            configWriteInt(id, *static_cast<const int*>(data));
            return "success: integer data written to config";
        case 's':
            configWriteString(id, static_cast<const char*>(data));
            return "success: string data written to config";
        default:
            return "not success: unknown data type";
    }
}

void configWriteInt(int id, int value) {
    bool replaced = false;
    for (size_t i = 0; i < configArrayMap.intCount; ++i) {
        if (configArrayMap.intArray[i].id == id) {
            configArrayMap.intArray[i].value = value;
            replaced = true;
            break;
        }
    }
    if (!replaced && configArrayMap.intCount < MAX_INT_COUNT) {
        configArrayMap.intArray[configArrayMap.intCount++] = IntEntry(id, value);
    }
}

void configWriteString(int id, const char* str) {
    bool replaced = false;
    for (size_t i = 0; i < configArrayMap.stringCount; ++i) {
        if (configArrayMap.stringArray[i].id == id) {
            std::strncpy(configArrayMap.stringArray[i].value, str, MAX_STRING_LENGTH - 1);
            configArrayMap.stringArray[i].value[MAX_STRING_LENGTH - 1] = '\0';
            replaced = true;
            break;
        }
    }
    if (!replaced && configArrayMap.stringCount < MAX_STRING_COUNT) {
        configArrayMap.stringArray[configArrayMap.stringCount++] = StringEntry(id, str);
    }
}

void configFlush(uint32_t* buffer, size_t& bufferSize) {
    size_t intArraySize = configArrayMap.intCount * INT_ENTRY_SIZE;
    size_t stringArraySize = configArrayMap.stringCount * (sizeof(int) + sizeof(int) + MAX_STRING_LENGTH); // Type + ID + value
    bufferSize = intArraySize + stringArraySize + 2 * sizeof(uint32_t); // Handle + int and string counts

    uint32_t* bufferPtr = buffer;

    // Store the number of int and string entries
    *bufferPtr++ = configArrayMap.intCount;
    *bufferPtr++ = configArrayMap.stringCount;

    // Copy int entries
    for (size_t i = 0; i < configArrayMap.intCount; ++i) {
        std::memcpy(bufferPtr, &configArrayMap.intArray[i].type, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, &configArrayMap.intArray[i].id, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, &configArrayMap.intArray[i].value, sizeof(int));
        bufferPtr += 1;
    }

    // Copy string entries
    for (size_t i = 0; i < configArrayMap.stringCount; ++i) {
        std::memcpy(bufferPtr, &configArrayMap.stringArray[i].type, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, &configArrayMap.stringArray[i].id, sizeof(int));
        bufferPtr += 1;
        std::memcpy(bufferPtr, configArrayMap.stringArray[i].value, MAX_STRING_LENGTH);
        bufferPtr += MAX_STRING_LENGTH / 4;
    }
}

int configGetInt(int id) {
    for (size_t i = 0; i < configArrayMap.intCount; ++i) {
        if (configArrayMap.intArray[i].id == id && configArrayMap.intArray[i].type == 0) {
            return configArrayMap.intArray[i].value;
        }
    }
    return -1;
}

const char* configGetString(int id) {
    for (size_t i = 0; i < configArrayMap.stringCount; ++i) {
        if (configArrayMap.stringArray[i].id == id && configArrayMap.stringArray[i].type == 1) {
            return configArrayMap.stringArray[i].value;
        }
    }
    return "-1";
}

const char* loadConfig() {
    uint8_t byteBuffer[BUFFER_SIZE];  // Allocate a buffer for raw data
    size_t numberOfWords = BUFFER_SIZE / sizeof(uint32_t);

    const char* result = readAndLoadFlashData("config.bin", byteBuffer, numberOfWords);
    if (std::strncmp(result, "Success", 7) != 0) {
        return result;  // Return the error message
    }

    processConfigBuffer(byteBuffer, BUFFER_SIZE);
    return "Success: Config loaded from flash.";
}

const char* flashConfig() {
    size_t bufferSize = BUFFER_SIZE;
    uint32_t buffer[BUFFER_SIZE / sizeof(uint32_t)];  // Statically allocate the buffer

    configFlush(buffer, bufferSize);  // Flush config data to the buffer

    const char* result = fileWrite("config.bin", buffer, bufferSize);
    return result;  // Return success or failure message
}

void processConfigBuffer(uint8_t* bufferPtr, size_t bufferSize) {
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
            for (size_t j = 0; j < configArrayMap.intCount; ++j) {
                if (configArrayMap.intArray[j].id == id) {
                    configArrayMap.intArray[j].value = value;
                    replaced = true;
                    break;
                }
            }
            if (!replaced && configArrayMap.intCount < MAX_INT_COUNT) {
                configArrayMap.intArray[configArrayMap.intCount++] = IntEntry(id, value);
            }
        } else if (type == 1) { // It's a string
            char value[MAX_STRING_LENGTH] = {0};
            std::memcpy(value, bufferPtr, STRING_ENTRY_SIZE);
            bufferPtr += STRING_ENTRY_SIZE;

            bool replaced = false;
            for (size_t j = 0; j < configArrayMap.stringCount; ++j) {
                if (configArrayMap.stringArray[j].id == id) {
                    std::strncpy(configArrayMap.stringArray[j].value, value, MAX_STRING_LENGTH - 1);
                    configArrayMap.stringArray[j].value[MAX_STRING_LENGTH - 1] = '\0';
                    replaced = true;
                    break;
                }
            }
            if (!replaced && configArrayMap.stringCount < MAX_STRING_COUNT) {
                configArrayMap.stringArray[configArrayMap.stringCount++] = StringEntry(id, value);
            }
        }
    }
}

// configOpen: Relays the text-based result from fileOpen
const char* configOpen(const char* str) {
    return fileOpen(str);  // Directly return the message from fileOpen
}
// Function to save name-ID pairs for config and return a success or error message
const char* configSaveHandles(const char* name, int id) {
    if (!name || id < 0) return "not success: invalid name or ID";

    for (int i = 0; i < configNameIDCount; ++i) {
        if (std::strcmp(configNameIDStorage[i].name, name) == 0) {
            configNameIDStorage[i].id = id;
            return "success: ID updated for existing config name";
        }
    }

    if (configNameIDCount < MAX_NAME_ID_PAIRS) {
        std::strncpy(configNameIDStorage[configNameIDCount].name, name, MAX_STRING_LENGTH - 1);
        configNameIDStorage[configNameIDCount].name[MAX_STRING_LENGTH - 1] = '\0';
        configNameIDStorage[configNameIDCount].id = id;
        ++configNameIDCount;
        return "success: name-ID pair saved for config";
    }

    return "not success: config name-ID storage is full";
}

int configGetIDFromName(const char* name) {
    for (int i = 0; i < configNameIDCount; ++i) {
        if (std::strcmp(configNameIDStorage[i].name, name) == 0) {
            return configNameIDStorage[i].id;
        }
    }
    return -1;  // Return -1 to indicate failure
}

