#include "BeaconDriver.h"
#include "I2C-Wrapper.h"
#include <stdio.h>

#define N_SENSORS 9
const int SENSOR_ANGULAR_PLACEMENT = 360 / N_SENSORS;

int IR_init(Beacon *ir, int port, int address)
{
	ir->port = port;
	ir->address = address;
	if(port < 0)
		return 0;
	if(i2c_readRegister(ir->port, ir->address, IR_WHO_AM_I) != IR_WHO_AM_I_VALUE)
    {
		printf("Error : beacon not detected\n");
		return 0;
	}
	IR_getFrequency(ir);
	return 1;
}


int IR_getRawValue(Beacon ir)
{
	unsigned char rawValues[2];
	i2c_readRegisters(ir.port, ir.address, IR_RAW_DATA1, 2, rawValues);
	int result = rawValues[0] + (rawValues[1] << 8);
	return result;
}

int IR_getObstacle(Beacon ir, int obstacle, int *size, int *position)
{
	int registerAddress = IR_ROBOT1_POS;
	if(obstacle != 0)
		registerAddress = IR_ROBOT2_POS;
	int result = i2c_readRegister(ir.port, ir.address, registerAddress);

	*size = (result & 0b11100000) >> 5;
	*position = result & 0b00011111;
	*position = *position * SENSOR_ANGULAR_PLACEMENT / 2;
	if(ir.frequency == IR_FREQUENCY_38k)
	{
		*position += SENSOR_ANGULAR_PLACEMENT / 2;
		if(*position == 2 * N_SENSORS)
			*position = 0;
	}
	return (size > 0 ? 1 : 0);
}


unsigned char IR_getFrequency(Beacon *ir)
{
	ir->frequency = i2c_readRegister(ir->port, ir->address, IR_FREQUENCY);
	return ir->frequency;
}
