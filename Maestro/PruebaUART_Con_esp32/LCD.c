/*
 * LCD.c
 *
 * Created: 27/01/2026 16:43:12
 *  Author: ffdd3
 */ 

#include "LCD.h"

//Funcion para inicializar pantalla
void initLCD (void){
	DDRD = 0xFF;
	DDRB = 0xFF;
	
	PORTD = 0x00;
	PORTB = 0x00;
	
	_delay_ms(20);
	
	//Reset para 8 bits
	LCD_CMD(0x30);
	_delay_ms(5);
	LCD_CMD(0x30);
	_delay_us(150);
	LCD_CMD(0x30);
	_delay_us(150);
	
	LCD_CMD(0x38);
	_delay_us(50); // 8-bit, 2 líneas, 5x8
	LCD_CMD(0x08);
	_delay_us(50); // display OFF
	LCD_CMD(0x01);
	_delay_ms(3);  // clear
	LCD_CMD(0x06);
	_delay_us(50); // entry mode
	LCD_CMD(0x0C);
	_delay_us(50); // Display ON, cursor OFF
	
}

//Funcion para colocar dato en el puerto
void LCD_Port (char a){
	
	if (a & 1)
		PORTD |= (1 << PORTD5);
	else
		PORTD &= ~(1 << PORTD5);
	
	if (a & 2)
		PORTD |= (1 << PORTD6);
	else
		PORTD &= ~(1 << PORTD6);
	
	if (a & 4)
		PORTD |= (1 << PORTD7);
	else
		PORTD &= ~(1 << PORTD7);
		
	if (a & 8)
		PORTB |= (1 << PORTB0);
	else
		PORTB &= ~(1 << PORTB0);
		
	if (a & 16)
		PORTB |= (1 << PORTB1);
	else
		PORTB &= ~(1 << PORTB1);
		
	if (a & 32)
		PORTB |= (1 << PORTB2);
	else
		PORTB &= ~(1 << PORTB2);
		
	if (a & 64)
		PORTB |= (1 << PORTB3);
	else
		PORTB &= ~(1 << PORTB3);
		
	if (a & 128)
		PORTB |= (1 << PORTB4);
	else
		PORTB &= ~(1 << PORTB4);
	
}

//Funcion para enviar un comando
void LCD_CMD (char a){
	PORTD &= ~((1 << PORTD2)); //RS = 0
	
	LCD_Port(a);
	
	PORTD |= (1<<PORTD4); //E = 1
	_delay_us(4);
	PORTD &= ~(1<<PORTD4);
	_delay_us(50);
	
}

//Funcion para enviar un caracter
void LCD_Write_char (char c){
	// RS = 1
	PORTD |= (1<<PORTD2);
	
	LCD_Port(c);
	
	PORTD |= (1<<PORTD4);
	_delay_us(1);
	PORTD &= ~(1<<PORTD4);
	
	_delay_us(50);
}

//Funcion para enviar una cadena
void LCD_Write_String (char *a){
	
	int i;
	
	for (i=0; a[i] != '\0'; i++)
	LCD_Write_char(a[i]);
}

//Desplazamiento a la derecha
void LCD_Shift_Right(void){
	LCD_CMD(0x1C);
}

//Desplazamiento a la izquierda
void LCD_Shift_Left(void){
	LCD_CMD(0x18);
}

//Establecer el cursor
void LCD_Set_Cursor(char c, char f){
	char temp;
	
	if (f == 1){
		temp = 0x80 + c - 1;
	}
	
	else if (f == 2){
		temp = 0xC0 + c - 1;
	}
	
	LCD_CMD(temp);
}
