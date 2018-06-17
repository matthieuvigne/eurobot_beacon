/*
  Code for the attiny841 on the infrared receiver.
  This code loops between all inputs, reads the value, and stores the results. It also fills I2C registers for communication with the master processor on the robot.
  See IC_Documentation_receiver.pdf for information about sensor number.
  Note: the device supports sequential reading of I2C registers.
  Note: the I2C interface exposed through WireS.h is quite crude. As a result, complexe I2C transactions will not be supported. The device can handle :
   - A write with register address, stop, and reading of a single register.
   - A write with register address, stop, and reading of several registers (but no more than one loop.
   - Writting to a register, and then reading directly will give back the old register value (a stop is required for a write operation to be performed).
*/

// Hardware I2C slave library.
#include "WireS.h"

// Pinout
#define LED 5
#define A 10
#define B 9
#define C 8
#define INH 7

// Caracteristics of the pulse
#define LENGTH 190  // Pulse length in microseconds
#define TOLERANCE 50  // Tolerance on pulse length, in microseconds

#define FREQUENCY_38 0
#define FREQUENCY_57 1

// Map between muptiplexer output and sensor number.
// multiplexerMap[frequency][i] makes each multiplexer read the ith sensor at desired frequency (for example, multiplexerMap[FREQUENCY_57][1] reads 57kHz sensor 1, 4 and 7, depending on the multiplexer).
// This array contains a number who, written in binary, will give 00000cba, where cba is the state of the A, B and C pins required to get the selected input.
const char multiplexerMap[2][3] = {{0,2,5},{3,1,4}};

// The current led frequency used.
unsigned char currentFrequency = FREQUENCY_57;
// The current sensor being read.
int currentSensor = 0;

// Whether a pulse has started since the beginning of the watching period.
bool hasStarted[3];
// Start time of the pulse.
unsigned long startTime[3];
// Whether a pulse was seen.
bool viewPulse[3];

// Current i2c register being read.
unsigned char currentRegister;

// Register definition, see Beacon_Coding_Documentation.pdf
#define N_REGISTERS 6

#define WHO_AM_I 0x00
#define IR_FREQUENCY 0x01
#define RAW_DATA1 0x02
#define RAW_DATA2 0x03
#define ROBOT1_POS 0x04
#define ROBOT2_POS 0x05

// Array to be filled with data to return to the master.
unsigned char I2CRegisters[N_REGISTERS];

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  // Enable multiplexer.
  pinMode(INH, OUTPUT);
  digitalWrite(INH, LOW);
  // Set the three interrupt pins as input (port A, INT0, port B).
  pinMode(3, INPUT);
  pinMode(1, INPUT);
  pinMode(0, INPUT);

  // Blink led to show startup.
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);

  //Activate interrupt on port A and B, as well as INT0.
  GIMSK = 0b01110000;
  //Set INT0 to trigger interrupt on every pin change.
  //Set SE to 1 as recommended in datasheet
  MCUCR = 0b00100001;
  //Set port A interrupt to trigger only on PA7 (pin 6).
  PCMSK0 =0b10000000;
  //Set port B interrupt to trigger only on PB0 (pin 2).
  PCMSK1 =0b00000001;

  // Choose frequency, for now hard-coded.
  currentFrequency = FREQUENCY_57;

  // Setup I2C registers
  for(int i = 0; i < N_REGISTERS; i++)
    I2CRegisters[i] = 0;
  I2CRegisters[WHO_AM_I] = 0x0f;
  I2CRegisters[IR_FREQUENCY] = currentFrequency;
  //Setup I2C communication, give address 0x42 to slave.
  Wire.begin(0x42);
  Wire.onAddrReceive(addressHandler);
  Wire.onReceive(receiveHandler);
  Wire.onRequest(requestHandler);
  Wire.onStop(stopHandler);
}

// Slave address has been recieved from master.
bool addressHandler(uint16_t address, uint8_t startCount)
{
  // If master wants to read, and there has been a previously written data, update currentRegister pointer to written value.
  if(address % 2 == 1)
 {
   if(startCount > 0 && Wire.available())
       currentRegister = Wire.read();
 }
 return true;
}

// Write command recieved from master (with stop, i.e. this is the end of a transaction).
void receiveHandler(size_t numBytes)
{
  // Get register address.
  if (Wire.available())
  {
    // Update register pointer.
    currentRegister = Wire.read();
    for(size_t i = 1; i < numBytes && Wire.available(); i++)
    {
      byte data = Wire.read();
      // All registers are read-only: do nothing with the data.
    }
  }
}

// Recieved read request from master.
void requestHandler()
{
  // Make sure to remain in range before reading.
  currentRegister = currentRegister % N_REGISTERS;
  // Loop through registers while the master wants data.
  for(int i = 0; i < N_REGISTERS; i++)
  {
    Wire.write(I2CRegisters[currentRegister]);
    currentRegister = (currentRegister + 1) % N_REGISTERS;
  }
}

// Stop command recieved from master after reading.
void stopHandler()
{
  // Empty
}

#define N_SENSORS 9
// If size is greater than maxSize, split in half.
const int maxSize = 6;
// This function groups the raw data into two robots, and fills the corresponding registers.
void computeObstacleRegisters()
{
	int robotSize[2] = {0,0};
	int robotPos[2] = {0,0};
	int currentSize = 0;
	// Loop around receiver, find the size of each block, and keep the largest two.
	int start = 0;
	// If the first receiver sees something, loop backward to beginning of block.
	if(I2CRegisters[RAW_DATA1] % 2 == 1)
	{
		if(I2CRegisters[RAW_DATA2] == 1)
		{
			start = N_SENSORS - 1;
			while(start > 0 && (I2CRegisters[RAW_DATA1] >>  (start - 1)) % 2 == 1)
				start --;
		}
	}
	for(int i = 0; i < N_SENSORS; i++)
	{
		int sensorNumber = (start + i) % N_SENSORS;
		bool seen = false;
		if((sensorNumber == 8 && I2CRegisters[RAW_DATA2] == 1) || ((I2CRegisters[RAW_DATA1] >> sensorNumber) % 2 == 1))
		{
			currentSize ++;
			seen = true;
		}
		// If we see no objects, or it's the last iteration: check if the zone is big enough to be kept.
		if(i == N_SENSORS - 1 || !seen)
		{
			//If we are here because it's the last iteration : increase sensorNumber to fake new iteration.
			if(i == N_SENSORS - 1 && seen)
				sensorNumber = (sensorNumber + 1) % N_SENSORS;
			if(currentSize > 0)
			{
				if(currentSize >= robotSize[0])
				{
					robotSize[1] = robotSize[0];
					robotPos[1] = robotPos[0];
					robotSize[0] = currentSize;
					robotPos[0] = (sensorNumber - 1) * 2 - (currentSize - 1);
					if(robotPos[0] < 0)
						robotPos[0] = 2 * N_SENSORS + robotPos[0];
				}
				else if(currentSize >=robotSize[1])
				{
					robotSize[1] = currentSize;
					robotPos[1] = (sensorNumber - 1) * 2 - (currentSize - 1);
					if(robotPos[1] < 0)
						robotPos[1] = 2 * N_SENSORS + robotPos[1];
				}
				currentSize = 0;
			}
		}
	}
	// Split in half if a single large zone is seen.
	if(robotSize[0] > maxSize)
	{
		int newSize = (int) (ceil(robotSize[0] / 2.0));
		robotSize[1] = newSize;
		robotPos[1] = robotPos[0] + newSize;
		robotSize[0] = newSize;
		robotPos[0] = robotPos[0] - newSize;

		if(robotPos[0] < 0)
			robotPos[0] += 2 * N_SENSORS;
		if(robotPos[1] >= -2 * N_SENSORS)
			robotPos[1] -= 2 * N_SENSORS;
	}

	// Format this into the registers
	I2CRegisters[ROBOT1_POS] = (robotSize[0] << 5) + robotPos[0];
	I2CRegisters[ROBOT2_POS] = (robotSize[1] << 5) + robotPos[1];
}

// Set multiplexer to read specified sensor number (0-2) at specified frequency.
void setMultiplexer(int frequency, int sensorNumber)
{
  if(frequency < 0)
    frequency = 0;
  else if(frequency > 1)
    frequency = 1;
  if(sensorNumber < 0)
    sensorNumber = 0;
  else if(sensorNumber > 2)
    sensorNumber = 2;

    digitalWrite(A, multiplexerMap[frequency][sensorNumber]%2);
    digitalWrite(B, (multiplexerMap[frequency][sensorNumber]/2)%2);
    digitalWrite(C, (multiplexerMap[frequency][sensorNumber]/4)%2);
}

// Interruption handling: this function is called with a different argument depending on wether the interruption
// was triggered by port A (muliplexer 0), INT0 (multiplexer 1) or port B (multiplexer 2).
void interruptionTriggered(int port)
{
  if(port < 0)
    port = 0;
  if(port > 2)
    port = 2;
  bool isSignalLow = false;
  switch(port)
  {
    case 0: isSignalLow = (PINA  & 0b10000000) == 0; break;
    case 1: isSignalLow = (PINB  & 0b00000010) == 0; break;
    case 2: isSignalLow = (PINB  & 0b00000001) == 0; break;
  }

  // If its low, start timer.
  if(isSignalLow)
  {
    startTime[port] = micros();
    hasStarted[port] = true;
  }
  else
  {
    if(hasStarted[port])
    {
      // If it's high and the timer was started, look at the time and set viewPulse to true if it matches.
      unsigned long pulse = abs(micros() - startTime[port]);  //abs in case of a reset
      if(pulse > LENGTH - TOLERANCE && pulse < LENGTH + TOLERANCE)
        viewPulse[port] = true;
    }
    hasStarted[port] = false;
  }
}

// Interruption on port A.
ISR(PCINT0_vect)
{
  interruptionTriggered(0);
}

// Interruption on INT0.
ISR(INT0_vect)
{
  interruptionTriggered(1);
}

// Interruption on port B.
ISR(PCINT1_vect)
{
  interruptionTriggered(2);
}

void loop()
{
  //Reset interrupt variables.
  for(int i = 0; i< 3; i++)
  {
    hasStarted[i] = false;
    viewPulse[i] = false;
  }
  // Choose sensor to read.
  currentSensor = (currentSensor + 1) % 3;
  setMultiplexer(currentFrequency, currentSensor);

  // Start interrupt (i.e. listen to IR receiver).
  sei();
  // Sleep 11ms : this enables us to read 2 sensor pulse, thus have more stable readings (only one is required to trigger the sensor).
  delayMicroseconds(11000);
  // End interrupt.
  cli();

  //Process results
  if(viewPulse[0] || viewPulse[1] || viewPulse[2])
    digitalWrite(LED, HIGH);
  else
    digitalWrite(LED, LOW);

   // Set raw register values.
   for(int i = 0; i < 3; i++)
   {
     if(i == 2 && currentSensor == 2)
     {
       if(viewPulse[i])
         I2CRegisters[RAW_DATA2] = 1;
       else
         I2CRegisters[RAW_DATA2] = 0;
     }
     else
     {
       I2CRegisters[RAW_DATA1] &= ~(1 << 3 * i + currentSensor);
       if(viewPulse[i])
         I2CRegisters[RAW_DATA1] |= (1 << 3 * i + currentSensor);
     }
   }
   computeObstacleRegisters();
}
