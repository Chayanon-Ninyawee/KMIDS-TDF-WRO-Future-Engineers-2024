#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdio.h>

#define DEBUG

// Define DEBUG_PRINT macro for conditional debug logging
#ifdef DEBUG
    #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...) // Does nothing in production
#endif

#endif // DEBUG_PRINT_H
