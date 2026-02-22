#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "I2C.h"

#define SLAVE_ADDR 0x12

// Humedad digital D2
#define HUM_DDR   DDRD
#define HUM_PORT  PORTD
#define HUM_PINR  PIND
#define HUM_BIT   PD2

// Motor D8 (en tu código es PB0)
#define MOTOR_DDR  DDRB
#define MOTOR_PORT PORTB
#define MOTOR_BIT  PB0

#define MOTOR_ON()  (MOTOR_PORT |= (1<<MOTOR_BIT))
#define MOTOR_OFF() (MOTOR_PORT &= ~(1<<MOTOR_BIT))

volatile uint8_t tx_value = 0;
volatile uint8_t motor_cmd = 0;

static inline uint8_t read_hum(void)
{
	return (HUM_PINR & (1<<HUM_BIT)) ? 1 : 0;
}

ISR(TWI_vect)
{
	uint8_t st = TWSR & 0xF8;

	switch(st)
	{
		// Maestro quiere escribir (mandar comando motor)
		case 0x60:
		case 0x68:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0x80: // dato recibido
		case 0x90:
		motor_cmd = TWDR;   // 0 o 1
		if(motor_cmd) MOTOR_ON();
		else MOTOR_OFF();

		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		// Lectura del maestro (humedad)
		case 0xA8:
		case 0xB0:
		tx_value = read_hum();
		TWDR = tx_value;
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		case 0xB8:
		TWDR = tx_value;
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;

		default:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
	}
}

int main(void)
{
	// Humedad input
	HUM_DDR &= ~(1<<HUM_BIT);
	HUM_PORT |= (1<<HUM_BIT);

	// Motor output
	MOTOR_DDR |= (1<<MOTOR_BIT);
	MOTOR_OFF();

	I2C_Slave_Init(SLAVE_ADDR);
	sei();

	while(1){}
}