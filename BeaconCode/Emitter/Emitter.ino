/*
 Code of the attiny85 of the infrared emitter.
 For this to work the attiny must use the 8MHz internal clock.
 */
 
void setup() {


  //Set pin 0 to output pwm at 57kHz
  //Note: because we are modifying timer0, arduino functions like delay will no longer give correct results
  //View: datasheet, page 77
  pinMode(0, OUTPUT);
  TCCR0A = 0b01000011;  //Set Pin 0 to react to counter, and set fast PWM mode from bottom to top
  TCCR0B = 0b00001001;  //No prescaling: runs at clock frequency
  OCR0A = 138;          //every OCR0A, the value of the output will change : at 8MHz, changing every 139 gives a 57.55kHz signal

  //Set pin 1 as pwm output
  pinMode(1, OUTPUT);

  TCCR1  = 0b01101010; //Set Pin 1 to react to timer A, set prescale to CK/512
  OCR1C = 155;       //Set frequency at 200Hz -> 5ms signal (actual value: 4.992ms)
  OCR1A = 149;       //192us high pulse. This gives a duty cycle of 0.04
                     //and a burst lenght of 11 cycles at 57k, 7.4 cycles at 38k.
  
  //Blink led.
  pinMode(3, OUTPUT);

}


void loop() {
  // Simply blink led to show that the code has started well !
  digitalWrite(3, HIGH);
  delay(25000L);
  digitalWrite(3, LOW);
  delay(25000L); 
}



