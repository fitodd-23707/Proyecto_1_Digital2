/*
 * LCD.h
 *
 * Created: 27/01/2026 16:43:30
 *  Author: ffdd3
 */ 

#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

//Funcion para inicializar pantalla
void initLCD (void);
//Funcion para colocar en el puerto
void LCD_Port (char a);
//Funcion para enviar un comando
void LCD_CMD (char a);
//Funcion para enviar un caracter
void LCD_Write_char (char c);
//Funcion para enviar una cadena
void LCD_Write_String (char *a);
//Desplazamiento a la derecha
void LCD_Shift_Right(void);
//Desplazamiento a la izquierda
void LCD_Shift_Left(void);
//Establecer el cursor
void LCD_Set_Cursor(char c, char f);

#endif