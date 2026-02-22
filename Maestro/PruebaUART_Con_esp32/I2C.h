
/*
 * I2C.h
 *
 * Created: 10/02/2026 17:42:20
 *  Author: Usuario
 */ 

#ifndef I2C_H_
#define I2C_H_

#ifndef	F_CPU 
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <stdint.h>

void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Preescaler);

uint8_t I2C_Master_Start(void);

uint8_t I2C_Master_RepeatedStart(void);

void I2C_Master_Stop(void);

uint8_t I2C_MasterWrite(uint8_t dato);

uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack);

void I2C_Slave_Init(uint8_t address);
#endif
