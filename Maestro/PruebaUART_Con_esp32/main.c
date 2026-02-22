/*
 * PruebaUART_Con_esp32.c
 *
 * Created: 14/02/2026 18:25:48
 * Author : ffdd3
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdio.h>
#include "UART.h"
#include "I2C.h"
#include "LCD.h"

#define SHT31_ADDR  0x44
#define SLAVE_ADDR1  0x12
#define SLAVE_ADDR2  0x14

uint8_t last_cmd = 0;
uint8_t last_cmd_t = 0;
uint8_t on_roof = 0;
uint8_t on_window = 0;
uint8_t on_bomb = 0;
uint8_t abierta = 0;

char CompleteData[10];
char dato = '0';
char modo = 'a';

volatile uint32_t timer0_millis = 0;

ISR(TIMER0_COMPA_vect) {
	timer0_millis++;
}

ISR(USART_RX_vect){
	char dato = UDR0; // Leer dato recibido
	
	// Ejemplo: eco del dato recibido
	while (!(UCSR0A & (1<<UDRE0))); // Esperar a que el transmisor esté libre
}

void init_millis() {
	// Configurar Timer0 en modo CTC
	TCCR0A |= (1 << WGM01);
	
	// Prescaler de 64
	TCCR0B |= (1 << CS01) | (1 << CS00);
	
	// Valor de comparación para 1ms a 16MHz
	OCR0A = 249;
	
	// Habilitar interrupción por comparación
	TIMSK0 |= (1 << OCIE0A);
}

uint32_t millis() {
	uint32_t millis_copy;
	// Bloque atómico para evitar que la interrupción cambie el valor mientras se lee
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		millis_copy = timer0_millis;
	}
	return millis_copy;
}

static uint8_t SHT31_ReadTemp(float *tempC)
{
	uint8_t d[6];

	if(!I2C_Master_Start()) return 0;
	if(!I2C_MasterWrite((SHT31_ADDR<<1) | 0)) { I2C_Master_Stop(); return 0; }

	// comando single shot high repeatability: 0x2400
	I2C_MasterWrite(0x24);
	I2C_MasterWrite(0x00);
	I2C_Master_Stop();

	_delay_ms(20);

	if(!I2C_Master_Start()) return 0;
	if(!I2C_MasterWrite((SHT31_ADDR<<1) | 1)) { I2C_Master_Stop(); return 0; }

	for(uint8_t i=0;i<5;i++){
		if(!I2C_MasterRead(&d[i], 1)) { I2C_Master_Stop(); return 0; }
	}
	if(!I2C_MasterRead(&d[5], 0)) { I2C_Master_Stop(); return 0; }

	I2C_Master_Stop();

	uint16_t rawT = ((uint16_t)d[0]<<8) | d[1];
	*tempC = -45.0f + 175.0f * ((float)rawT / 65535.0f);
	return 1;
}

static uint8_t Slave_ReadHum(uint8_t *hum)
{
	if(!I2C_Master_Start()) return 0;

	// SLA+R
	if(!I2C_MasterWrite((SLAVE_ADDR1<<1) | 1)) {
		I2C_Master_Stop();
		return 0;
	}

	// leer 1 byte con NACK
	if(!I2C_MasterRead(hum, 0)) {
		I2C_Master_Stop();
		return 0;
	}

	I2C_Master_Stop();
	return 1;
}

static uint8_t Slave_ReadRain(uint8_t *rain)
{
	if(!I2C_Master_Start()) return 0;

	// SLA+R
	if(!I2C_MasterWrite((SLAVE_ADDR2<<1) | 1)) {
		I2C_Master_Stop();
		return 0;
	}

	// leer 1 byte con NACK
	if(!I2C_MasterRead(rain, 0)) {
		I2C_Master_Stop();
		return 0;
	}

	I2C_Master_Stop();
	return 1;
}

static uint8_t Slave1_WriteMotor(uint8_t cmd)
{
	if(!I2C_Master_Start()) return 0;

	// SLA+W
	if(!I2C_MasterWrite((SLAVE_ADDR1<<1) | 0)) {
		I2C_Master_Stop();
		return 0;
	}

	if(!I2C_MasterWrite(cmd)) {
		I2C_Master_Stop();
		return 0;
	}

	I2C_Master_Stop();
	return 1;
}

static uint8_t Slave2_WriteMotor(uint8_t cmd)
{
	if(!I2C_Master_Start()) return 0;

	// SLA+W
	if(!I2C_MasterWrite((SLAVE_ADDR2<<1) | 0)) {
		I2C_Master_Stop();
		return 0;
	}

	if(!I2C_MasterWrite(cmd)) {
		I2C_Master_Stop();
		return 0;
	}

	I2C_Master_Stop();
	return 1;
}

int main(void)
{
	cli();
	
	UART_init();
	I2C_Master_Init(100000, 1);
	
	UART_Write_str("MASTER: SHT31 + Humedad(Slave) + Motor\r\n\r\n");
	
	initLCD();
	
	LCD_Set_Cursor(1,1);
	LCD_Write_String("TEMP");
	
	LCD_Set_Cursor(8,1);
	LCD_Write_String("HUM");
	
	LCD_Set_Cursor(13,1);
	LCD_Write_String("RAIN");
	
	LCD_Set_Cursor(18,1);
	LCD_Write_String("WATER");
	
	LCD_Set_Cursor(24,1);
	LCD_Write_String("ROOF");
	
	LCD_Set_Cursor(29,1);
	LCD_Write_String("WINDOW");
	
	init_millis(); // Inicializar el cronómetro
	
	uint32_t anterior1_ms = 0;
	uint32_t anterior2_ms = 0;
	const uint32_t intervalo1 = 500;
	const uint32_t intervalo2 = 1000;
	
	sei();
	
    while (1) 
    {
		uint32_t actual_ms = millis();
		
		//UART_Write_str("entramos al while");
		
		switch(dato){
			case 'a':
				on_roof = 1;
				break;
			case 'w':
				on_window = 1;
				break;
			case 'b':
				on_bomb = 1;
				break;
			default:
				on_roof = 0;
				on_window = 0;
				on_bomb = 0;
				break;
		}
		
		if (actual_ms - anterior1_ms >= intervalo1) {
			
			anterior1_ms = actual_ms;
			
			float tC = 0;
			uint8_t okT = SHT31_ReadTemp(&tC);

			// ---- Leer humedad del esclavo ----
			uint8_t hum = 0;
			uint8_t okH = Slave_ReadHum(&hum);
			
			uint8_t rain = 0;
			uint8_t okR = Slave_ReadRain(&rain);
			
			if ((tC > 24) && (!abierta)){
				Slave2_WriteMotor(1);
				abierta = 1;
			}
			else if ((tC <= 24) && (abierta)){
				Slave2_WriteMotor(0);
				abierta = 0;
			}

			// ---- Decidir motor por humedad (ajusta si tu sensor trabaja al revés) ----
			// Regla actual: hum==0 -> motor ON, hum==1 -> motor OFF
			uint8_t motor_cmd = (hum == 0) ? 0 : 1;
			if (!motor_cmd)
			{
				LCD_Set_Cursor(8,2);
				LCD_Write_String("YES");
				LCD_Set_Cursor(19,2);
				LCD_Write_String("OFF");
			}
			else
			{
				LCD_Set_Cursor(8,2);
				LCD_Write_String("NO ");
				LCD_Set_Cursor(19,2);
				LCD_Write_String("ON ");
			}
			

			// ---- Enviar comando al esclavo solo si cambia ----
			if(okH && motor_cmd != last_cmd) {
				Slave1_WriteMotor(motor_cmd);
				last_cmd = motor_cmd;
			}
			
			//Descición de motor para techo
			uint8_t motor_cmd_t = (rain == 1) ? 0 : 1;
			if (!motor_cmd_t)
			{
				LCD_Set_Cursor(13,2);
				LCD_Write_String("YES");
				LCD_Set_Cursor(24,2);
				LCD_Write_String("OFF");
			}
			else
			{
				LCD_Set_Cursor(13,2);
				LCD_Write_String("NO ");
				LCD_Set_Cursor(24,2);
				LCD_Write_String("ON ");
			}
			

			// ---- Enviar comando al esclavo solo si cambia ----
			if(okR && motor_cmd_t != last_cmd_t) {
				//Slave2_WriteMotor(motor_cmd_t);
				last_cmd_t = motor_cmd_t;
			}
			
			if(okT) { UART_TxFloat2(tC);
				char g[20];
				dtostrf(tC, 6, 2, g);
				LCD_Set_Cursor(1,2);
				LCD_Write_String(g);
			}
			else    { UART_Write_str("ERR"); }
			
			UART_Write_char('!');

			//UART_Write_str(" | Humedad(Slave): ");
			if(okH) UART_TxUInt(hum);
			else    UART_Write_str("ERR");
			
			UART_Write_char('!');
			
			if(okR) UART_TxUInt(rain);
			else    UART_Write_str("ERR");
			
			//Char de división
			UART_Write_char('!');

			//UART_Write_str(" | MotorCmd: ");
			UART_TxUInt(motor_cmd);
			
			//Char de división
			UART_Write_char('!');
			
			UART_TxUInt(motor_cmd_t);
			
			//Char de terminación
			UART_Write_str("\r\n");
		}
		
		if (actual_ms - anterior2_ms >= intervalo2) {
			anterior2_ms = actual_ms;
			LCD_Shift_Left();
		}
    }
}