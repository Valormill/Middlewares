/*
 * flashFile.h
 *
 *  Created on: Aug 22, 2024
 *      Author: bem012
 */

#ifndef FLASHFILE_H
#define FLASHFILE_H

#include <cstddef>
#include <cstdint>

// Function to load data from flash, returning raw data to be processed
const char* readAndLoadFlashData(const char* handle, uint8_t* data, size_t& size);

// Function to open a file by its handle
const char* fileOpen(const char* handle);

// Function to write data to flash, passing buffer and size
const char* fileWrite(const char* handle, uint32_t* data, size_t size);

#endif // FLASHFILE_H
