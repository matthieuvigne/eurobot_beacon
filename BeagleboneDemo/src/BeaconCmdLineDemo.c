// BeaconCmdLineDemo.c
// Matthieu Vigne <matthieu.vigne@laposte.net>
//
// This is a demo code for the beacon, which gives sensor status in the command line.

#include <stdio.h>
#include <unistd.h>
#include "BeaconDriver.h"
#include "I2C-Wrapper.h"

int main(int argc, char **argv)
{
	Beacon beacon;
	// Init communication with the beacon.
	int dev = i2c_open("/dev/i2c-1");
	if(dev < 0)
	{
		printf("Error opening i2c port: is i2c enabled?\n");
		return -1;
	}
	if(!IR_init(&beacon, dev, IR_RECEIVER_ADDRESS))
	{
		printf("Error connecting to beacon: check wiring.\n");
		return -1;
	}
	printf("Successfully connected to beacon.\n");

	printf("Sensor state (0: off, 1: active, emitter is seen)\n");
	for(int i = 0; i < 9; i++)
		printf("%d\t", i);
	printf("\n");
	// In a loop, update beacon status.
	while(1)
	{
		// Get raw sensor reading.
		int rawData = IR_getRawValue(beacon);
		for(int i = 0; i < 9; i++)
			printf("%d\t", (rawData >> i) % 2);
		printf("\r");
		fflush(stdout);
		usleep(50000);
	}
	return 0;
}

