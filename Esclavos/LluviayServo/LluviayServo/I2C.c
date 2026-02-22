#include "I2C.h"

#define I2C_TIMEOUT  60000UL

static uint8_t I2C_WaitTWINT(void)
{
	uint32_t t = I2C_TIMEOUT;
	while(!(TWCR & (1<<TWINT)))
	{
		if(--t == 0) return 0;
	}
	return 1;
}

// ===================== MASTER =====================

void I2C_Master_Init(unsigned long SCL_Clock, uint8_t Prescaler)
{
	// SDA/SCL como entrada
	DDRC &= ~((1<<DDC4) | (1<<DDC5));
	// Sin pull-ups internos (usa tus pull-ups externos)
	PORTC &= ~((1<<PORTC4) | (1<<PORTC5));

	// Prescaler
	switch(Prescaler){
		case 1:
		TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
		break;
		case 4:
		TWSR = (TWSR & ~(1<<TWPS1)) | (1<<TWPS0);
		break;
		case 16:
		TWSR = (TWSR & ~(1<<TWPS0)) | (1<<TWPS1);
		break;
		case 64:
		TWSR |= (1<<TWPS1)|(1<<TWPS0);
		break;
		default:
		TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
		Prescaler = 1;
		break;
	}

	// TWBR
	TWBR = (uint8_t)((((F_CPU / SCL_Clock) - 16UL) / 2UL) / Prescaler);

	// Enable TWI
	TWCR = (1<<TWEN);
}

uint8_t I2C_Master_Start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	if(!I2C_WaitTWINT()) return 0;

	uint8_t status = TWSR & 0xF8;
	return (status == 0x08); // START transmitted
}

uint8_t I2C_Master_RepeatedStart(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	if(!I2C_WaitTWINT()) return 0;

	uint8_t status = TWSR & 0xF8;
	return (status == 0x10); // Repeated START transmitted
}

void I2C_Master_Stop(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);

	
	uint32_t t = I2C_TIMEOUT;
	while((TWCR & (1<<TWSTO)) && --t);
}

uint8_t I2C_MasterWrite(uint8_t dato)
{
	TWDR = dato;
	TWCR = (1<<TWINT)|(1<<TWEN);

	if(!I2C_WaitTWINT()) return 0;

	uint8_t status = TWSR & 0xF8;

	// 0x18: SLA+W ACK, 0x28: DATA ACK, 0x40: SLA+R ACK
	if(status == 0x18 || status == 0x28 || status == 0x40) return 1;

	return 0;
}

uint8_t I2C_MasterRead(uint8_t *buffer, uint8_t ack)
{
	if(ack) TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	else    TWCR = (1<<TWINT)|(1<<TWEN);

	if(!I2C_WaitTWINT()) return 0;

	uint8_t status = TWSR & 0xF8;

	// 0x50: DATA received, ACK returned
	// 0x58: DATA received, NACK returned
	if(ack && status != 0x50) return 0;
	if(!ack && status != 0x58) return 0;

	*buffer = TWDR;
	return 1;
}

// ===================== SLAVE =====================


void I2C_Slave_Init(uint8_t address)
{
	DDRC &= ~((1<<DDC4) | (1<<DDC5));        
	PORTC &= ~((1<<PORTC4) | (1<<PORTC5));     

	TWAR = (address << 1); // 7-bit addr

	// Enable TWI, ACK, interrupt, clear TWINT
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWIE) | (1<<TWINT);
}
