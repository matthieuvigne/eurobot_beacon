# BeaconCode

Both the emitter and the reciever contain an ATTiny microcontroller (respectively ATTiny85 and ATTiny841):
 - the emitter code generates the signal sent to the LEDs.
 - the reciever code reads individual recievers, processes the results to get an estimate of robot position, and handles
 I2C communication.

The choice of Atmel microcontroller was done because they are easy to flash: all that one needs is a simple Arduino board !
Direct wire connections between the IC and the Arduino can be made - for convenience an Arduino shield is proposed in the
[PCBs](../PCBs) folder but this is not mandatory. There are plenty of explainations online on how to flash the code in these
microcontroller: for instance, have a look at [highlowtech's tutorial](http://highlowtech.org/?p=1695).

Note: while the electronics of both the emitter and the receiver support 38.4kHz and 57.6kHz signals, the current code
does not fully handle this. It wasn't developped as the 38.4kHz sensors of the prototype receiver were not solder. This
should however be easy to add.
