#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "I2C.h"
#include "servo.h"

#define SLAVE_ADDR 0x14

// ===== Sensor lluvia digital (D2 = PD2) =====
#define RAIN_DDR   DDRD
#define RAIN_PORT  PORTD
#define RAIN_PINR  PIND
#define RAIN_BIT   PD2

static inline uint8_t Rain_Read(void)
{
    return (RAIN_PINR & (1 << RAIN_BIT)) ? 1 : 0;
}

// ===== ULN2003 IN1..IN4 -> D4..D7 =====
#define STEP_DDR   DDRD
#define STEP_PORT  PORTD

#define IN1 PD4
#define IN2 PD5
#define IN3 PD6
#define IN4 PD7

#define CMD_FWD  1
#define CMD_REV  0

volatile uint8_t stepper_cmd = 255;  
volatile uint8_t tx_status = 0;      

// Forward / Reverse (half-step)
static const uint8_t seq_fwd[8] = {
    (1<<IN1),
    (1<<IN1)|(1<<IN2),
    (1<<IN2),
    (1<<IN2)|(1<<IN3),
    (1<<IN3),
    (1<<IN3)|(1<<IN4),
    (1<<IN4),
    (1<<IN4)|(1<<IN1)
};

static const uint8_t seq_rev[8] = {
    (1<<IN1),
    (1<<IN4)|(1<<IN1),
    (1<<IN4),
    (1<<IN3)|(1<<IN4),
    (1<<IN3),
    (1<<IN2)|(1<<IN3),
    (1<<IN2),
    (1<<IN1)|(1<<IN2)
};

static inline void Stepper_Output(uint8_t pattern)
{
    STEP_PORT = (STEP_PORT & ~((1<<IN1)|(1<<IN2)|(1<<IN3)|(1<<IN4))) | pattern;
}

static inline void Stepper_Stop(void)
{
    STEP_PORT &= ~((1<<IN1)|(1<<IN2)|(1<<IN3)|(1<<IN4));
}

static void Stepper_Run_Time_ms(const uint8_t *seq, uint16_t total_ms)
{
    uint8_t i = 0;
    uint16_t elapsed = 0;

    while(elapsed < total_ms)
    {
        Stepper_Output(seq[i]);
        i = (i + 1) & 7;

        _delay_ms(2);   // velocidad
        elapsed += 2;
    }

    Stepper_Stop();
}

// ===== ISR I2C Slave =====
ISR(TWI_vect)
{
    uint8_t st = TWSR & 0xF8;

    switch(st)
    {
        // Maestro escribe (SLA+W)
        case 0x60:
        case 0x68:
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;

        case 0x80:
        case 0x90:
        {
            uint8_t v = TWDR;
            if(v == CMD_FWD || v == CMD_REV) {
                stepper_cmd = v;  // ejecutar en loop
            }
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;
        }

        // Maestro lee (SLA+R)
        case 0xA8:
        case 0xB0:
            TWDR = tx_status; // respondemos un byte (ej: lluvia)
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;

        case 0xB8:
            TWDR = tx_status;
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;

        default:
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;
    }
}

int main(void)
{
    // Lluvia input + pull-up
    RAIN_DDR  &= ~(1 << RAIN_BIT);
    RAIN_PORT |=  (1 << RAIN_BIT);

    // Stepper outputs
    STEP_DDR |= (1<<IN1)|(1<<IN2)|(1<<IN3)|(1<<IN4);
    Stepper_Stop();

    // Servo init
    servo_init();
    servoCodo(0);

    // I2C slave
    I2C_Slave_Init(SLAVE_ADDR);
    sei();

    uint8_t last_rain = 255;

    while(1)
    {
        // ===== 1) Lluvia -> Servo (automático) =====
        uint8_t rain = Rain_Read();
        tx_status = rain; 

        if(rain != last_rain)
        {
            
            if(rain) {
				stepper_cmd = 255;
				Stepper_Run_Time_ms(seq_fwd, 5000); 
				
			}
            else{
				stepper_cmd = 255;
				Stepper_Run_Time_ms(seq_rev, 5000); 
				
			}

            last_rain = rain;
        }

        // ===== 2) I2C -> Stepper (por comando) =====
        uint8_t cmd = stepper_cmd;

        if(cmd == CMD_FWD)
        {
			servoCodo(90);
        }
        else if(cmd == CMD_REV)
        {
			servoCodo(0);
        }

        _delay_ms(10);
    }
}