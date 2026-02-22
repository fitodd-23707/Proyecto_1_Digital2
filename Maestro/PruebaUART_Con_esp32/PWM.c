/*
 * PWM.c
 *
 * Created: 16/02/2026 17:58:31
 *  Author: ffdd3
 */ 

#include <avr/io.h>
#include "PWM.h"


void PWM_Servo_Init(void) {
	// Servo 1 (PB1 - OC1A): 50Hz (20ms período)
	DDRB |= (1 << PB1);                // PB1 como salida
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Modo 14 (Fast PWM), prescaler 8
	ICR1 = 19999;                      // TOP value para 20ms
	OCR1A = 1500;                      // Posición inicial (1.5ms)
}
