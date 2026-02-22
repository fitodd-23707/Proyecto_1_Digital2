/*
 * Servo.c
 *
 * Created: 17/02/2026 17:18:15
 *  Author: Usuario
 */ 
#include "servo.h"
#define F_CPU 16000000UL
#include <avr/io.h>

void servo_init() {
	// Configurar PB1 (OC1A) como salida para el  servo
	DDRB |= (1 << DDB1);

	// Modo Fast PWM con ICR1 como TOP
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler de 8

	
	ICR1 = 39999;
}

void servoCodo(float angle1) {
	if(angle1 > 180) angle1 = 180;
	if(angle1 < 0) angle1 = 0;
	
	OCR1A = 1000 + (uint16_t)((angle1 / 180.0) * 4000);
}