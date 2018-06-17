/*
 Code of the infrared emitter to detect the other robot.
 Leaving switch 2 open uses 57kHz frequency ; switch close sends 38kHz.
 Note : the attiny85 should be reprogrammed to work with the 8MHz internal clock.
 */

 // Current frequency: HIGH for 57.6kHz, LOW for 38.4 kHz
char currentFrequency = HIGH;
long delayTime = 5000L;

// Set carrier frequency.
void setFrequency(char frequency)
{
  if(frequency == HIGH)
  {
      OCR0A = 138;          // Every OCR0A, the value of the output will change : at 16MHz, changing every 139 gives a 57.55kHz signal
      delayTime = 5000L;
  }
  else
  {
      OCR0A = 207;          // Every OCR0A, the value of the output will change : at 16MHz, changing every 208 gives a 38.46kHz signal
      delayTime = 25000L;
  }
}

void setup()
{
  //Set pin 0 to output pwm at carrier frequency: default 57.5kHz.
  //Note: because we are modifying timer0, arduino functions like delay will no longer give correct results
  //View: datasheet, page 77
  pinMode(0, OUTPUT);
  TCCR0A = 0b01000011;  // Set Pin 0 to react to counter, and set fast PWM mode from bottom to top
  TCCR0B = 0b00001001;  // No prescaling: runs at clock frequency
  currentFrequency = HIGH;
  setFrequency(currentFrequency);

  //Set pin 1 as pwm output
  pinMode(1, OUTPUT);

  TCCR1  = 0b01101010; //Set Pin 1 to react to timer A, set prescale to CK/512
  OCR1C = 155;       //Set frequency at 200Hz -> 5ms signal (actual value: 4.992ms)
  OCR1A = 149;       //192us high pulse. This gives a duty cycle of 0.04
                     //and a burst lenght of 11 cycles at 57k, 7.4 cycles at 38k.

  // Blink led.
  pinMode(3, OUTPUT);

  // Frequency choosing input.
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
}


void loop()
{
  char newFrequency = digitalRead(2);
  // Update frequency, if needed.
  if(newFrequency != currentFrequency)
  {
    setFrequency(newFrequency);
  }
  currentFrequency = newFrequency;

  // Blink led
  digitalWrite(3, HIGH);
  delay(delayTime);
  digitalWrite(3, LOW);
  delay(5000L);
}



