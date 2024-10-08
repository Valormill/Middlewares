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
int readAndLoadFlashData(uint8_t* data, size_t& size, uint32_t addr);

// Function to open a file by its handle
int fileOpen(const char* handle);

// Function to write data to flash, passing buffer and size
int fileWrite(uint32_t* data, size_t size, uint32_t addr);

#endif // FLASHFILE_H
