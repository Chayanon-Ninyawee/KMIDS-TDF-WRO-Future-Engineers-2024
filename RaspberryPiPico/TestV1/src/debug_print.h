#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdio.h>
#include "i2c_slave_utils.h"

#define DEBUG

// Define DEBUG_PRINT macro for conditional debug logging
#ifdef DEBUG
    // Define DEBUG_PRINT to log to console and append to logs
    #ifndef NO_LOG
        #define DEBUG_PRINT(...)                                        \
        do {                                                            \
            printf(__VA_ARGS__);                                        \
            char log_buffer[i2c_slave_mem_addr::LOGS_BUFFER_SIZE];      \
            int len = snprintf(log_buffer, sizeof(log_buffer), __VA_ARGS__); \
            if (len > 0 && len < sizeof(log_buffer)) {                  \
                append_logs(log_buffer, (size_t)len);                   \
            }                                                           \
        } while (0)
    #else
        #define DEBUG_PRINT(...) printf(__VA_ARGS__) // Only log to console
    #endif
#else
    #define DEBUG_PRINT(...) // Does nothing in production
#endif


#endif // DEBUG_PRINT_H
