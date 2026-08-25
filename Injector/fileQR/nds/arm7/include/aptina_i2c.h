#ifndef APTINA_I2C_H
#define APTINA_I2C_H

#include <nds.h>
#include <nds/arm7/i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

enum i2cFlags { I2C_NONE = 0x00, I2C_STOP = 0x01, I2C_START = 0x02, I2C_ACK = 0x10, I2C_READ = 0x20 };

u8 aptWriteRegister(u8 device, u16 reg, u16 data);
u16 aptReadRegister(u8 device, u16 reg);

#ifdef __cplusplus
}
#endif

#endif // APTINA_I2C_H
