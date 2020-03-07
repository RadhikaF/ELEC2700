# Assignment 3 - Ultrasonic Radar
The objective of this assignment is to develop an ultrasonic radar. The radar is able to detect objects that come within its field of view using a pair of 40kHz ultrasonic transducers mounted on a servo motor mechanism. One of the transducers is a transmitter and the other is a receiver. The distance of objects is determined since the speed of sound is known to be 343m/s and the time for an echo of a short pulse of sound can be measured. The software is written in C, for the C8051F120 development system in conjunction with the peripheral interface board.

It has the following features:
* A detected object's relative position is displayed on the LCD, in terms of its relative angle and distance.
* When an object is within 100mm the radar stops sweeping and sends out a pulsating warning alarm using the speaker, at a frequency of 260Hz. The alarm continues until the object leaves the boundary radius. 

# Innovation
* Basic radar screen emulation with the top line of the LCD corresponds to distances greater than 100mm, while the bottom line corresponds to distances less than 100mm. Each of the 16 boxes correspond to a different angle value at which an object is detected.
* Direction of object travel detection: when an object has moved left from the previous position then LD1 will light up. If it has moved right, LD2 will light up.
* User has the option to double, triple, or quadruple the servo speed.
* Allow user control over the servo.
