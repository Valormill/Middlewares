#ifndef INIT_ARRAY_MAP_H
#define INIT_ARRAY_MAP_H

#include <cstring> // For std::strncpy

// Define constants for easy modification
#define MAX_INT_ENTRIES 5         // Maximum number of integer entries
#define MAX_STRING_ENTRIES 5      // Maximum number of string entries
#define MAX_STRING_LENGTH 20      // Maximum length of string (including null terminator)

enum {
    TYPE_INT,
    TYPE_STRING,
};

// Define struct for integer entries
struct IntEntry {
    int type;    // 0 for int
    int id;      // Use int ID to identify entry
    int value;

    // Constructor for easy initialization
    IntEntry(int i = 0, int v = 0) : type(TYPE_INT), id(i), value(v) {}
};

// Define struct for string entries
struct StringEntry {
    int type;    // 1 for string
    int id;      // Use int ID to identify entry
    char value[MAX_STRING_LENGTH];  // Array for string value

    // Constructor for easy initialization
    StringEntry(int i = 0, const char* v = "") : type(TYPE_STRING), id(i) {
        std::strncpy(value, v, sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0'; // Ensure null-termination
    }
};

struct NameIDPair {
    char name[MAX_STRING_LENGTH];
    int id;
};

// Define struct to hold Init array map
struct InitArrayMap {
    IntEntry intArray[MAX_INT_ENTRIES];    // Array to store integer entries
    size_t intCount;                       // Count of integer entries
    StringEntry stringArray[MAX_STRING_ENTRIES];  // Array to store string entries
    size_t stringCount;                    // Count of string entries
};

#endif // INIT_ARRAY_MAP_H

