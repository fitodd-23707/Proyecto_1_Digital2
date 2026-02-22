/*
 * UART.c
 *
 * Created: 1/02/2026 18:57:53
 *  Author: ffdd3
 */ 

#define F_CPU 16000000
#include <avr/io.h>

#include "UART.h"

void UART_init(){
	DDRD |= (1 << DDD1);
	DDRD &= ~(1 << DDD0);
	UCSR0A = 0;
	UCSR0B = ((1 << RXCIE0) | (1<< RXEN0) | (1 << TXEN0));
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UBRR0 = 103;
}

void UART_Write_char(char caracter)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = caracter;
}

void UART_Write_str(char* texto)
{
	for(uint8_t i = 0; *(texto+i) != '\0'; i++)
	{
		UART_Write_char(*(texto+i));
	}
}

void UART_TxFloat2(float v)
{
	char b[20];
	dtostrf(v, 6, 2, b);
	UART_Write_str(b);
}

void UART_TxUInt(uint16_t v)
{
	char b[8];
	itoa(v, b, 10);
	UART_Write_str(b);
}