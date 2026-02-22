#include "SI1133.h"
#include "I2C.h"
#include <util/delay.h>

#define SI1133_ADDR 0x55

/* REGISTERS */
#define REG_PART_ID     0x00
#define REG_HOSTIN0     0x0A
#define REG_COMMAND     0x0B
#define REG_RESPONSE0   0x11
#define REG_IRQ_STATUS  0x12
#define REG_HOSTOUT0    0x13

/* COMMANDS */
#define CMD_RESET       0x01
#define CMD_FORCE_CH    0x11
#define CMD_PARAM_QUERY 0x40
#define CMD_PARAM_SET   0x80

/* PARAMETERS */
#define PARAM_CHAN_LIST     0x01

#define PARAM_ADCCONFIG0    0x02
#define PARAM_ADCSENS0      0x03
#define PARAM_ADCPOST0      0x04
#define PARAM_MEASCONFIG0   0x05

#define PARAM_ADCCONFIG1    0x06
#define PARAM_ADCSENS1      0x07
#define PARAM_ADCPOST1      0x08
#define PARAM_MEASCONFIG1   0x09

#define PARAM_MEASRATE_H    0x1A
#define PARAM_MEASRATE_L    0x1B
#define PARAM_MEASCOUNT0    0x1C

/* ADCMUX (según datasheet) */
#define ADCMUX_IR_SMALL     0x00
#define ADCMUX_UV           0x18

/* ===== LOW LEVEL ===== */

static void SI1133_WriteReg(uint8_t reg, uint8_t value)
{
	I2C_Master_Start();
	I2C_MasterWrite((SI1133_ADDR<<1) | 0);
	I2C_MasterWrite(reg);
	I2C_MasterWrite(value);
	I2C_Master_Stop();
}

static uint8_t SI1133_ReadReg(uint8_t reg)
{
	uint8_t data = 0;

	I2C_Master_Start();
	I2C_MasterWrite((SI1133_ADDR<<1) | 0);
	I2C_MasterWrite(reg);

	I2C_Master_RepeatedStart();
	I2C_MasterWrite((SI1133_ADDR<<1) | 1);
	I2C_MasterRead(&data, 0);
	I2C_Master_Stop();

	return data;
}

static void SI1133_WaitCmdDone(uint8_t prev_rsp0)
{
	for (uint16_t i = 0; i < 5000; i++) {
		uint8_t rsp0 = SI1133_ReadReg(REG_RESPONSE0);
		if ((rsp0 & 0x1F) != (prev_rsp0 & 0x1F)) return;
		_delay_us(50);
	}
}

static void SI1133_ParamSet(uint8_t param, uint8_t value)
{
	uint8_t rsp0 = SI1133_ReadReg(REG_RESPONSE0);

	SI1133_WriteReg(REG_HOSTIN0, value);
	SI1133_WriteReg(REG_COMMAND, CMD_PARAM_SET | (param & 0x1F));

	SI1133_WaitCmdDone(rsp0);
}

/* ===== INIT ===== */

uint8_t SI1133_Init(void)
{
	uint8_t id = SI1133_ReadReg(REG_PART_ID);
	if (id != 0x33) return 0;

	SI1133_WriteReg(REG_COMMAND, CMD_RESET);
	_delay_ms(30);

	SI1133_WriteReg(REG_IRQ_STATUS, 0xFF);

	// Canal 0 = UV, Canal 1 = IR (bits bajos habilitan canal 0..5)
	SI1133_ParamSet(PARAM_CHAN_LIST, 0x03); // ch0 + ch1

	// CH0 UV
	SI1133_ParamSet(PARAM_ADCCONFIG0, (0<<5) | ADCMUX_UV);
	SI1133_ParamSet(PARAM_ADCSENS0,   0x01);  // ganancia simple
	SI1133_ParamSet(PARAM_ADCPOST0,   0x00);
	SI1133_ParamSet(PARAM_MEASCONFIG0,0x00);

	// CH1 IR
	SI1133_ParamSet(PARAM_ADCCONFIG1, (0<<5) | ADCMUX_IR_SMALL);
	SI1133_ParamSet(PARAM_ADCSENS1,   0x01);
	SI1133_ParamSet(PARAM_ADCPOST1,   0x00);
	SI1133_ParamSet(PARAM_MEASCONFIG1,0x00);

	// Rate global
	SI1133_ParamSet(PARAM_MEASRATE_H, 0x00);
	SI1133_ParamSet(PARAM_MEASRATE_L, 0x20);
	SI1133_ParamSet(PARAM_MEASCOUNT0, 0x01);

	return 1;
}

/* ===== READ ===== */

static uint16_t SI1133_ReadChannel16(uint8_t hostout_base)
{
	uint8_t lo = 0, hi = 0;

	SI1133_WriteReg(REG_COMMAND, CMD_FORCE_CH);
	_delay_ms(30);

	I2C_Master_Start();
	I2C_MasterWrite((SI1133_ADDR<<1) | 0);
	I2C_MasterWrite(hostout_base);

	I2C_Master_RepeatedStart();
	I2C_MasterWrite((SI1133_ADDR<<1) | 1);

	I2C_MasterRead(&lo, 1);
	I2C_MasterRead(&hi, 0);

	I2C_Master_Stop();

	return ((uint16_t)hi << 8) | lo;
}

uint16_t SI1133_ReadUV(void)
{
	// CH0 => HOSTOUT0/1
	return SI1133_ReadChannel16(REG_HOSTOUT0);
}

uint16_t SI1133_ReadIR(void)
{
	// CH1 => HOSTOUT2/3
	return SI1133_ReadChannel16(REG_HOSTOUT0 + 2);
}
