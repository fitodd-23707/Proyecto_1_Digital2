#ifndef SI1133_H_
#define SI1133_H_

#include <stdint.h>

uint8_t SI1133_Init(void);
uint16_t SI1133_ReadUV(void);
uint16_t SI1133_ReadIR(void);

#endif
