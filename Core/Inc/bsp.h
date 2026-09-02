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
    ERR_NO_ERROR = 0x00U,
    ERR_I2C_TXIS = 0x01U,
} BSP_ErrMask_t;

void BSP_ErrorSet(BSP_ErrMask_t error_mask);
void BSP_ErrorReset(BSP_ErrMask_t error_mask);
uint8_t BSP_GetErrors(void);

void BSP_PeriphModeActive();
void BSP_PeriphModeSleep();

#endif