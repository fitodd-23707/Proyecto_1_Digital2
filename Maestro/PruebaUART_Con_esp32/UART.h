/*
 * UART.h
 *
 * Created: 1/02/2026 18:57:24
 *  Author: ffdd3
 */ 

#define F_CPU 16000000
#include <avr/io.h>

#ifndef UART_H_
#define UART_H_

// Inicializar UART
void UART_init(void);

//Escribir una cadena en UART
void UART_Write_char(char caracter);

//Escribir un caracter en UART
void UART_Write_str(char* texto);

//Escribir Float en texto
void UART_TxFloat2(float v);

//Escribir Int en texto
void UART_TxUInt(uint16_t v);

#endif /* UART_H_ */