#ifndef BSP_H
#define BSP_H

#include "main.h"
#include <stdbool.h>

extern volatile uint32_t system_ticks;

#define WAIT_FLAG(wait_flag_function, timeout) ({\
    uint32_t _start = system_ticks;              \
    bool _result = true;                         \
    while (!(wait_flag_function)) {              \
        if(system_ticks - _start >= timeout) {   \
            _result = false;                     \
            break;                               \
        }                                        \
    }                                            \
    _result;                                     \
})

typedef enum {
    ERR_NO_ERROR = 0x0000U
} BSP_ErrMask_t;

void BSP_PeriphModeActive();
void BSP_PeriphModeSleep();

#endif