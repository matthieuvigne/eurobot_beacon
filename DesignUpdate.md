This file presents some consideration on the current design, arising from further testing of the V1 beacon, that
acted as guidelines for the design of the next version of the beacon.

## Testing of the "dual frequency" feature.

In order to avoid perturbations from a possible beacon from the other team, I designed the receiver to work with
two different sensors, tuned respectively at 38.4kHz and 57.6kHz. The idea was that I could choose a frequency not used
by the other team. For this to work, it requires that the 57.6kHz sensor ignores the 38.4kHz signal. Unfortunately, these
two frequencies are rather close to each other...

I had done some quick testing with a 38.4kHz sensor on a 57kHz signal, wich gave the impression that this feature could work.
However, more precise testing with the full receiver proved this wrong. Using a fonctional 57.6kHz receiver, I changed
the emitter frequency... and realized that the receiver could still be seen ! There was some atenuation of course,
but not that much: which the device tuned such that 4 receiver saw the beacon at 20cm, changing frequency still left
the two best align receiver activated. Overall it diminished the detection distance, but did not provide complete insolation
like what was hoped for.

Thus, this design of dual receiver looks of little interest now: even if we stand on one frequency, we might still
see the other team's beacon, providing it is emitting strongly enough. This familly of Vishay sensors exists only between
30kHz and 57kHz - for this feature to work, we need a greater frequency difference. Thus, for the next design, I am
rather considering using a simple analog sensor, with no band pass filter, and then do numerical filtering. This imposes
a much stronger numerical circuit (i.e. microcontroller), but has several advantages: we are now free to choose any frequency
we want, including frequencies not explored by the Vishay sensors. As the filter is made through coding, we can make it
more selective if we want, and we can choose freely the frequency: for instance, making it work at 100kHz and 200kHz,
frequencies far enough from each other to have a good separation. This will require an in-depth redesign of the receiver however...

## Using two emitters.

This is a problem I overlooked when designing the first beacon, as I was much concerned about one emitter-receiver pair.
But actually, we might have two of our own emitters on the field. If both are far from each other, this will not pause a problem
to the receiver, as each signal will be seen by different sensors. However, there is the case when both emitters are close to
each other, and shining at the same receiver. Then, the second emitter acts as a perturbation on the first one, and, by design,
is pulsing the right type of signal, at the right frequency. As we have no syncronization between them, it is possible,
for instance, that both 200us pulses will happen just after each other, forming a single 400us pulse, that will then be ignored
by the receiver as being too long. In other words, having two emitter might blind the receiver !

To solve this, I'm considering adding some form of IR-based syncronization between both emitter. But the design remains to be done.
