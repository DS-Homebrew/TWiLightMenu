#ifndef I2C_HANDLER_H
#define I2C_HANDLER_H

#include "pxi_vars.h"

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

extern Thread s_i2cPxiThread;
extern u8 s_i2cPxiThreadStack[1024];

int i2cPxiThreadMain(void* arg);

#ifdef __cplusplus
}
#endif

#endif // I2C_HANDLER_H
