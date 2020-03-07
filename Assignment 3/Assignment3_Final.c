/*_____________________________________________________________________________________________________________________

Project: Assignment 3
Module:	ELEC2700
Author:	JB & Radhika Feron
Date:	02/06/2017

Description:
This program will detect objects that come within its field of "view" using a pair of 40kHz untrasonic transducers mounted 
on a servo motor mechanism. A detected object's relative angle angle and distance away from the servo is displayed on the 
LCD. The software for this assignment is written in C, and implemented via the C8051F120 development system in conjunction 
with the peripheral interface board. 

Innovation:
	Enter the innovation mode by pushing P3.7. 
	Basic radar screen emulation selected when PB8 is pressed in innovation mode (using MPB to get in). The top line of the LCD 
corresponds to distances greater than 100mm, while the bottom line corresponds to distances less than 100mm. Each of the 16
boxes correspond to a different angle value at which an object is detected. Pressing PB8 again will reset the LCD screen.
	Direction of object travel detection is selected when PB7 is pressed in innovation mode. When an object has moved left from
the previous position then LD1 will light up. If it has moved right, LD2 will light up.
	If PB1 is pressed and held, the servo speed doubled. For PB2, the speed is tripled, and if both PB1 and PB2 are held then 
the speed is quadrupled.
	If PB4 pressed, the servo will move away from the user. If PB5 is pressed, the servo will move towards the user. This allows control
over the servo.
	Pressing PB6 will temporarily stop the servo while held.
	In innovation mode, if PB3 is pressed, the user has the option to stop the servo when in motion and still use other features.


_______________________________________________________________________________________________________________________*/

#include <c8051f120.h>     // SFR declarations
#include "A3_XX.h"
#include <math.h>

bit index = 0;		//	Used for servo control. When set to 1, will turn servo to 0 and wait 20ms. When set to 1, will wait certain time depending on angle. 
bit LCD_delay; 				// used to signal that timer 3 has overflowed and that 1ms has passed since last overflow. Set to 1 when overflowed, otherwise set to 0. 
code char letter[] = { 'J','o','s','h','u','a',' ','B','e','v','e','r','l','e','y','&',' ','R','a','d','h','i','k','a',' ','F','e','r','o','n' };	// list of characters to display “Joshua Beverley & Radhika Feron” on the LCD display. 
code char angle[] = { 'A','n','g','l','e',':',' ' };		// list of characters to display “Angle: ” on the LCD display. 
code char number[] = { '0','1','2','3','4','5','6','7','8','9','+','-',0xDF,'m' };	// list of numbers as characters to display on the LCD display. 0xDF is the degrees character, and m is used for millimeters units. 
code char distanceLCD[] = { 'D','i','s','t','a','n','c','e',':',' ',':' };	// list of characters to display “Distance: ” on the LCD display. 
code char out_of_range[] = { '-','-','-','m','m' };	// this displays instead of numbers on the LCD display when there is no object detected. 
code int higherreload[] = {
	0xFB,0xFB,0xFB,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xFA,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF9,0xF8,0xF8,0xF8,
	0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF7,0xF6,0xF6,0xF6,0xF6,0xF6,0xF6,0xF6,0xF6,0xF6,0xF6,
	0xF6,0xF6,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF5,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF4,0xF3,0xF3,0xF3,0xF3,0xF3,
	0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF3,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF2,0xF1,0xF1,0xF1,0xF1,0xF1,0xF1,0xF1,0xF1,0xF1,0xF1,0xF1,
	0xF1,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEF,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,
	0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xED,0xED,0xED,0xED,0xED,0xED,0xED,0xED,0xED,0xED,0xED,0xED,0xEC,0xEC };	// The higher reload value for the timer controlling the servo movement. 
code int lowerreload[] = {
	0x3D,0x28,0x14,0xFF,0xEB,0xD7,0xC2,0xAE,0x99,0x85,0x71,0x5C,0x48,0x34,0x1F,0x0B,0xF6,0xE2,0xCE,0xB9,0xA5,0x91,0x7C,0x68,0x53,0x3F,0x2B,0x16,0x02,0xEE,0xD9,0xC5,
	0xB0,0x9C,0x88,0x73,0x5F,0x4B,0x36,0x22,0x0D,0xF9,0xE5,0xD0,0xBC,0xA8,0x93,0x7F,0x6A,0x56,0x42,0x2D,0x19,0x04,0xF0,0xDC,0xC7,0xB3,0x9F,0x8A,0x76,0x61,0x4D,0x39,
	0x24,0x10,0xFC,0xE7,0xD3,0xBE,0xAA,0x96,0x81,0x6D,0x59,0x44,0x30,0x1B,0x07,0xF3,0xDE,0xCA,0xB6,0xA1,0x8D,0x78,0x64,0x50,0x3B,0x27,0x13,0xFE,0xEA,0xD5,0xC1,0xAD,
	0x98,0x84,0x6F,0x5B,0x47,0x32,0x1E,0x0A,0xF5,0xE1,0xCC,0xB8,0xA4,0x8F,0x7B,0x67,0x52,0x3E,0x29,0x15,0x01,0xEC,0xD8,0xC4,0xAF,0x9B,0x86,0x72,0x5E,0x49,0x35,0x21,
	0x0C,0xF8,0xE3,0xCF,0xBB,0xA6,0x92,0x7E,0x69,0x55,0x40,0x2C,0x18,0x03,0xEF,0xDA,0xC6,0xB2,0x9D,0x89,0x75,0x60,0x4C,0x37,0x23,0x0F,0xFA,0xE6,0xD2,0xBD,0xA9,0x94,
	0x80,0x6C,0x57,0x43,0x2F,0x1A,0x06,0xF1,0xDD,0xC9,0xB4,0xA0,0x8C,0x77,0x63,0x4E,0x3A,0x26,0x11,0xFD,0xE9 };	// The lower reload value for the timer controlling the servo movement. 
code int distancefordisplay[] = { 0,0,0,0,0,0,0,0,8,12,15,18,21,23,26,28,31,33,36,38,40,42,45,47,49,51,54,56,58,60,63,65,67,69,71,74,76,78,80,82,84,87,89,91,93,95,
	97,100,102,104,106,108,110,113,115,117,119,121,123,126,128,130,132,134,136,139,141,143,145,147,149,151,154,156,158,160,162,164,167,169,171,173,175,177,179,182,184,
	186,188,190,192,195,197,199,201,203,205,207,210,212,214,216,218,220,222,225,227,229,231,233,235,237,240,242,244,246,248,250,253,255,257,259,261,263,265,268,270,
	272,274,276,278,280,283,285,287,289,291,293,295,298,300,302,304,306,308,310,313,315,317,319,321,323,326,328,330,332,334,336,338,341,343,345,347,349,351,353,356,
	358,360,362,364,366,368,371,373,375,377,379,381,383,386,388,390,392,394,396,398,401,403,405,407,409,411,413,416,418,420,422,424,426,428,431,433,435,437,439,441,
	444,446,448,450,452,454,456,459,461,463,465,467,469,471,474,476,478,480,482,484,486,489,491,493,495,497,499 };	// Converts distance read (index) to actual distance. 
code float pulsetodistance = 2.14375;	// Constant used to convert pulse_count (measurement of time from signal sent to received) to distance. 
int pulse_count = 0;		//when Timer 4 overflows, pulse count is incremented. Used to know when to toggle USonicTX to send a pulse and to find the timer for a signal to be received (or not)
bit signal_received = 0;		// if signal_received=1, a low to high transition on USonicTX pin detected (reflection signal received)
int distance = 0;			// distance value used to control the DAC and the servo control. 
int distanceforprint = 0;		// the distance of the object sent to the LCD
int anglenumber = 0;			// number between 0 and 180, specifies the angle the servo is currently at and the index of the distancefordisplay variables that is currently being read. 
bit timer1interrupt = 0;		// set to one when timer 0 overflows
bit timer4interrupt = 0;		// set to one when timer 4 overflows
bit forwardorback = 0;		// if =1 then the servo motor will move to the -90 side towards where the time the waveform is high is 600 micro-sec
int LCDdelaytime = 0x68;		// is used to adjust the speed of the LCD time delays. 
int alarm_count = 0;		//used to get the alarm 1 second on and 1 second off by sounding the alarm until it is incremented to 500.
bit Sound_bit = 0;		//used to represent when the square wave sent to the DAC is high or low (if =0, wave is low) for the alarm
bit stop = 0;			// stop = 0 when the servo is to move, stop = 1 when the servo is to stop. 
int stopnumber = 0;		// when an object is seen within 100mm, this value is set to 1000. A signal needs to be not received 1000 times before the servo will move again. 
int receive_count = 0;		//used to count how many times a signal is not received after an initial receival
bit radarmode = 0;		//if =1 then radar mode is selected and a basic radar screen is emulated on the LCD
int saveangle = 0;		//saves the previous angle position, used to detect the direction of movement of an object. 
bit direction = 0;		//determines if an object has moved in a certain direction (left or right)
int anglediff = 0;		//difference between the last angle and current angle. If positive, object has moved left; if negative, object has moved right. 
int copyangle = 0;		//copy of the anglenumber variable. Is copied so it can be modified in the LCD functions to display the angle. 
int copydistance = 0;		//copy of the distanceforprint variable. Is copied so it can be modified in the LCD functions to display the distance. 
int d = 0;				//used to capture ~10 distance values before finding the average of them all. 
int c = 0;				//of the 10 distance values, the number of actual readings that were taken is stored in this variable
int distancesum = 0;		//sum of all the distances taken. This variable is also used to hold the average, before it is printed. 
bit dacinit = 0;		//if the DAC is currently initialised, will be set to 1. Prevents reinitialising the DAC unnecessarily. 
bit stoponcommand = 0;	//when switched on in innovation mode, will allow player to stop the servo at the push of a button. 
bit stoporgo = 0;		//whenn set to 1, stops the movement of the servo. 
int f=0;				//f is used to control the number of signals sent per servo increment. 

/*--------------------------------------------------------------------------------------------------------------------
Function:         Main
Description:      Main routine
--------------------------------------------------------------------------------------------------------------------*/
void main(void)
{
	SFRPAGE = CONFIG_PAGE;	//set the SFR page to the configuration page
	OSCICN = 0x83;					// Need a faster clock....24.5MHz selected
	General_Init();		//calls the general initialisation function to initialise watchdog timer, ports, cross bar etc
	Voltage_Reference_Init();		//calls the voltage reference initialisation function
	Timer1_Init();		//calls the timer 0 initialisation function
	Timer2_Init();		//calls the timer 2 initialisation function
	Interrupts_Init();	//calls the interrupt initialisation function
	LCD_Init();		//calls the LCD initialisation function
	AngleWord();		//calls AngleWord function to display the word “Angle” on the LCD
	DispAngle();		//calls DispAngle function to display the angle of the servo on the LCD
	DistanceWord();		//calls DistanceWord function to display the word “Distance” on the LCD
	DispDistance();		//calls DispDistance function to display the distance of any detected object on the LCD
	P2=0;
	USonicTX = 0;	//clear USonicTX
	while (1)
	{
		if (!PB8 && radarmode == 1)	// if pushbutton 8 is selected and radar mode currently activated, clear the LCD screen. 
		{
			while (!PB8)	// debounces the pushbutton
			{
				LD8 = 0;	// briefly turns LED 8 off
			}
			LD8 = 1;		// turns LED8 on again 
			LCD_Init();	// reinitialise the LCD screen to wipe the current radar detection. 
		}
		if (!PB3 && stoponcommand == 1)	// if pushbutton 3 is pressed and stop mode is currently activated, stop the servo movement.
		{
			while (!PB3)
			{
			}
			stoporgo = ~stoporgo;	// if servo is stopped, move it; if it isn't stopped, stop it. 
			LD3 = stoporgo;			// LED3 lights up when the servo is forcibly stopped. 
	
		}
		if (!PB4)		// pushing PB4 moves the servo backwards
		{
			if (anglenumber > 1)	// if the angle is larger than 1, can't move angle past 0. 
			{
				anglenumber--;		// decrement angle to stop the movement
				anglenumber--;		// decrement angle to move the servo backwards. 
			}
		}
		else if (!PB5)	// pushing PB5 moves the servo forwards
		{
			if (anglenumber < 179)	// if the angle is larger than 179, can't move angle past 180. 
			{
				anglenumber++;		// increment angle to stop the movement
				anglenumber++;		// increment angle to move the servo forwards. 
			}
		}
		else if (!PB6)	// pushing PB6 will halt the movement of the servo
		{
			if (forwardorback == 1 && anglenumber < 179)	// if servo is currently moving forwards
			{
				anglenumber--;		// subtract the angle number to stop movement. 
			}
			else if (forwardorback == 0 && anglenumber > 1)	// if servo is currently moving backwards
			{
				anglenumber++;		// add to the angle number to stop movement. 
			}
		
		}
		else if (!PB1 && !PB2)	// pushing both PB1 and PB2 will move the servo at 4x the movement speed of before. 
		{
			if (forwardorback == 1 && anglenumber < 179)	// if servo is currently moving forwards
			{
				anglenumber++;	// increment the angle number to double the speed. 
				anglenumber++;	// increment the angle number to triple the speed. 
				anglenumber++;	// increment the angle number to quadruple the speed. 
			}
			else if (forwardorback == 0 && anglenumber > 1)	// if servo is currently moving backwards
			{
				anglenumber--;	// decrement the angle number to double the speed. 
				anglenumber--;	// decrement the angle number to triple the speed. 
				anglenumber--;	// decrement the angle number to quadruple the speed. 
			}
		
		}
		else if (!PB1)	// pushing PB1 will move the servo at 2x the movement speed of before. 
		{
			if (forwardorback == 1 && anglenumber < 179)	// if servo is currently moving forwards
			{
				anglenumber++;	// increment the angle number to double the speed. 
			}
			else if (forwardorback == 0 && anglenumber > 1)	// if servo is currently moving backwards
			{
				anglenumber--;	// decrement the angle number to double the speed. 
			}
		
		}
		else if (!PB2)
		{
			if (forwardorback == 1 && anglenumber < 179)	// if servo is currently moving forwards
			{
				anglenumber++;	// increment the angle number to double the speed. 
				anglenumber++;	// increment the angle number to triple the speed. 
			}
			else if (forwardorback == 0 && anglenumber > 1)	// if servo is currently moving backwards
			{
				anglenumber--;	// decrement the angle number to double the speed. 
				anglenumber--;	// decrement the angle number to triple the speed. 
			}
		
		}
		if (!MPB)		//if mode pushbutton is pressed (=0)
		{
			while (!MPB)		//debounces the pushbutton by checking if the button is still pressed until it is released
			{
			}
			Display_Joshua_Radhika();	//display our names on the LCD screen. 
			while (MPB)		//while mode pushbutton is not pressed
			{
				if (!PB8)	//if pushbutton 8 is pressed (=0)
				{
					while (!PB8)	//debounces the pushbutton by checking if the button is still pressed until it is released
					{
					}
					if (radarmode == 0)		//if the radarmode bit equals 0
					{
						radarmode = 1;		//let radarmode equal 1
					}
					else	//else if the radarmode bit equals 0
					{
						radarmode = 0;	//let radarmode equal 0
					}
					LD8 = radarmode;  	//display the radarmode on LED3 to show the selection of radar mode or normal distance and angle display mode			
				}
				if (!PB7)		//if pushbutton 8 is pressed (=0)
				{
					while (!PB7)	//debounces the pushbutton by checking if the button is still pressed until it is released
					{
					}
					if (direction == 0)		//if the direction bit equals 0
					{
						direction = 1;		//let direction equal 1
					}
					else 		//if the direction bit equals 1
					{
						direction = 0;		//let direction equal 0
					}
					LD7 = direction;	//display the direction bit on LED7 to show the selection of detecting object direction or not
				}
				if (!PB3)		//if pushbutton 8 is pressed (=0)
				{
					while (!PB3)	//debounces the pushbutton by checking if the button is still pressed until it is released
					{
					}
					if (stoponcommand == 0)		//if the direction bit equals 0
					{
						stoponcommand = 1;		//let direction equal 1
					}
					else 		//if the direction bit equals 1
					{
						stoponcommand = 0;		//let direction equal 0
					}
					LD3 = stoponcommand;	//display the direction bit on LED7 to show the selection of detecting object direction or not
				}
			}
			if (radarmode == 0)		//if radarmode has not been selected (=0), revert to displaying all the assignment stuff on the LCD (angle, distance). 
			{
				LCD_Init();		//call the LCD initialisation function
				AngleWord();	//call the AngleWord function to display 'Angle: ' on the LCD
				DispAngle();	//call the DispAngle function to display the angle the servo motor is at
				DistanceWord();	//call the DistanceWord function to display 'Distance: ' on the LCD
				DispDistance();	//call the DispDistance function to display the distance the detected object is at
			}
			else 	//if radarmode has not been selected (=1)
			{
				LCD_Init();		//call the LCD initialisation function, to erase it ready for radar mode. 
			}
			LD1 = 0;		//clear LD1
			LD2 = 0;		//clear LD2
			copyangle = 0;	//clear copyangle
			copydistance = 0;	//clear copydistance
			LD3 = stoporgo;
			while (!MPB)		//while mode pushbutton is pressed (=0)
			{

			}
		}
		while (timer1interrupt == 0)		//while the timer1interrupt variable equals 0 (becomes 1 when Timer 0 overflows). When this loop is exited, timer 1 has overflowed. 
		{
		}
		timer1interrupt = 0;		//clear timer1interrupt
		index = 0;			//set index to 0, so when timer 0 is initialised, the servo is set to 0 and the 20ms wait is initialised. 
		Timer1_Init();		//call Timer1 initialisation function to start timer 0

		Timer4_Init();		//call Timer 4 initialisation function to start timer 4
		
		for (f=0;f<=5;f++) {	//This for loop allows 6 measurements to be taken per degree moved by the servo. 
			while (!(pulse_count >= 233 || signal_received == 1))		//while the pulse count is not greater than 140 (object is not out of range yet) and/or signal_received is not equal to 1 (signal has not been received)
			{
				while (timer4interrupt == 0)		//while the timer4interrupt variable equals 0 (becomes 1 when Timer 4 overflows)
				{
				}
				pulse_count++;		//increment pulse_count
				timer4interrupt = 0;	//clear timer4interrupt
				if (pulse_count<9) {	//for the first 8 counts
					USonicTX = ~USonicTX;	//toggle the transceiver to send pulse
				}
				else if (pulse_count<14) {		//if pulse count is less than 4 do nothing (mask time)
				}
				else if (pulse_count>13 && pulse_count<233 && signal_received == 0) {		//if pulse count is greater than 13 (out of mask time) and less than 233 (not out of range yet) and no signal has been received
					if (USonicRX) {		//if receive signal detected (USonicRX=1)
						signal_received = 1;		//set signal received bit
						distance = distancefordisplay[pulse_count];		//set the distance to the number found at the position of pulse_count in the array distancefordisplay
						distance = distance - 50;
						distanceforprint = distance;		//let distanceforprint equal distance
						//DispDistance();
						if (radarmode == 1) 		//if radarmode equals 1 (radar mode selected)
						{
							radarLCD();		//call the radarLCD function to display the distance in basic radar screen emulation
						}
						if (direction == 1)	//if direction tracking mode has been selected
						{
							anglediff = saveangle - anglenumber;	//work out the difference between the angle this time and the angle last time. 
							if (anglediff > 0)	//if the angle last time was larger than this time
							{
								LD1 = 1;		//turn on LED1 and turn off LED2, to show the object has moved left. 
								LD2 = 0;
							}
							else if (anglediff < 0)	//if the angle last time was smaller than this time
							{
								LD1 = 0;		//turn off LED1 and turn on LED2, to show the object has moved right. 
								LD2 = 1;
							}
						}
						saveangle = anglenumber;	//remember the current angle in saveangle, so it can be checked against the next angle. 
					}
					else {
						signal_received = 0;		//no signal has been received, so set the bit to 0. 
						distance = 0;			//set distance to 0, as no signal has been received. 
						stop = 0;				//no object, so don’t stop the servo. 
						if (stopnumber != 0)		// if stopnumber doesn’t equal 0
						{
							stopnumber--;			// decrement stopnumber, decreases the number of times nothing has to be seen before moving again. 
						}
					}
				}
			}
			
			if (signal_received == 1)		//if a signal has been received
			{
				distancesum = distancesum + distanceforprint;		//add the distance calculated to the distance sum
				c++;		//increment c to represent the number of distances calculated
			}
			d++;			//increment n to represent the number of signals sent
			if (d == 180)		//if 10 signals have been sent
			{
				if (c == 0)		//if no signal has been received and so c has not been incremented (=0). c will later be on the denominator of a division, so it needs to be not zero. 
				{
					distancesum = 0;	//set distancesum to 0, so the division still results in 0. 
					c = 1;				//set c to something not zero. 
				}
				distanceforprint = distancesum / c;	//to find the average of the distances, divide the distancesum by c (the number of distances calculated)
				d = 0;		//clear d
				c = 0;		//clear c
				if (radarmode == 0)
				{
					DispDistance();		//call DispDistance function to display the distance on the LCD
				}
				distancesum = 0;	//clear distancesum
			}

			if ((distanceforprint < 100 && distanceforprint > 0) || (stoporgo == 1 && stoponcommand == 1))		//if the distance of the detected object is less than 100 and greater than 0
			{
				stop = 1;		//set stop bit to 1, object is closer than 100mm
				stopnumber = 500;		//let stopnumber equal 1000, needs to not see something 1000 times before moving and turning off the DAC. 
			}
			else
			{
				stop = 0;		//object is further away than 100mm, turn off stop.
				if (stopnumber != 0)	// if stopnumber doesn’t equal 0
				{
					stopnumber--;	// decrement stopnumber, decreases the number of times nothing has to be seen before moving again. 
				}
			}
			pulse_count = 0;		//clear pulse_count for the sending of the sending of the next signal.
			signal_received = 0; 
		}

		TMR4CN = 0x00; //turns the timer off, will be re initialised next iteration of the for loop.

		if (radarmode == 0)	//if radarmode has not been selected
		{
			DispAngle();		//call DispAngle function to display the angle of the servo on the LCD
		}
		while (timer1interrupt == 0)		//while timer1interrup is zero (while timer 0 has not overflowed)
		{
		}
		timer1interrupt = 0;		//clear timer1interrupt
		index = 1;		//set index to 1, so when timer 0 is initialised, the servo is set to 1 and the reload values used are for the angle. 
		signal_received = 0;	//reset the signal_received bit for next use.  
		if (stop == 1 || stopnumber != 0) {		//if object is closer than 100mm, or the stopnumber hasn’t decremented enough for movement yet. 
			Timer1_Init();		//use the same anglenumber as last time, so the servo doesn’t move
			if (dacinit == 0)	//if the DAC is currently off
			{
				DAC_Init();	//turn the DAC on, object is closer than 100mm
				dacinit = 1;	//since DAC is now on, so should the dacinit bit. 
			}
		}
		else //if (stop == 0) 
		{					//if nothing detect, object is out of range, more than 100mm away. 
			if (dacinit == 1)	//if the DAC is currently on
			{
				DAC_Deinit();	//turn the DAC off, object is not closer than 100mm
				dacinit = 0;	//since DAC is now off, so should the dacinit bit. 
			}
			if (anglenumber == 0)	// If the angle has reached the reload value required for a logic high on the servo to last 2400ms
			{
				forwardorback = 1; //move toward 600ms now
			}
			else if (anglenumber == 180) // If the angle has reached the reload value required for a logic high on the servo to last 600ms
			{
				forwardorback = 0; //move toward 2400ms now
			}
			if (forwardorback == 1)	//if we are moving towards 600ms now
			{
				anglenumber++;		//increment anglenumber, so the angle of the servo can be incremented. 
				Timer1_Init();		//turn the timer on. 
			}
			else if (forwardorback == 0)
			{
				anglenumber--;		//decrement anglenumber, so the angle of the servo can be decremented. 
				Timer1_Init();		//turn the timer on. 
			}
		}
	}
}

/*--------------------------------------------------------------------------------------------------------------------
Function:         RadarLCD
Description:      This function is used to emulated a basic radar screen on the LCD
--------------------------------------------------------------------------------------------------------------------*/

void radarLCD()
{
	int n = 0;		//variable used for the for loops in this function. 
	copydistance = 0;	//variable used to change whether the LCD blocks appear on the top row (further than 100mm) or bottom row (closer than 100mm). In radar mode, the distance isn’t displayed on the LCD, so this variable (otherwise used in the DispDistance() function) is used here instead. 
	if (distance > 100)	//if distance is larger than 
	{
		copydistance = 4;	//set copydistance to 4, the difference between the addresses for the two LCD lines (most significant bit)
	}
	copyangle = anglenumber / 16;	//divide the angle by 16, representing the 16 columns of LCD blocks. Again, copyangle is a variable used to display the angle on the LCD, but isn’t doing this in radar mode. 
	for (n = 0;n != 16;n++)	//for each column in the LCD screen
	{
		if (copyangle == n)	//if the angle is equal to n for this iteration of the for loop
		{
			LCD = (0x8C - copydistance);	// Sets the address for the distance. copydistance selects either top or bottom row. 
			Toggle_E();				// Toggle the E line
			LCD = (0x80 + n);			// Sets the address for the angle. n selects which column the block is to be displayed. 
			Toggle_E();				// Toggle the E line
		}
	}
	LCD = (0xFF >> 4 & 0x0F) | 0x90;	// Display a black block of pixels on the LCD screen for the address selected. 
	Toggle_E();	// Toggle the E line
	LCD = (0xFF & 0x0F) | 0x90;
	Toggle_E();	// Toggle the E line
}



/*--------------------------------------------------------------------------------------------------------------------
Function:         General_Init
Description:      Initialise ports, watchdog....
--------------------------------------------------------------------------------------------------------------------*/
void General_Init()
{
	WDTCN = 0xde;		//turn off watchdog timer
	WDTCN = 0xad;
	SFRPAGE = CONFIG_PAGE;		//set SFR page to the configuration page
	P0MDOUT = 0x14;		// sets servo control to pushpull
	P1MDOUT = 0x00;		// Ensures port 1 has no pushpull outputs
	P2MDOUT = 0xff;		// Need to make pushpull outputs to drive LEDs properly
	XBR2 = 0x40;		//enable crossbar
	Servo_Ctrl = 0;	// Initialise servo control pin to 0
}

/*--------------------------------------------------------------------------------------------------------------------
Function:         Timer1_Init
Description:
--------------------------------------------------------------------------------------------------------------------*/
void Timer1_Init()
{
	SFRPAGE = TIMER01_PAGE;
	TCON = 0x40;			//enable timer
	TMOD = 0x10;		//16 bit counter/timer

	if (index == 0)		//if index equals 0
	{
		Servo_Ctrl = 0;		//clear Servo_Ctrl bit
		TH1 = 0x5F;			//let the higher nibble of the timer 0 register equal 5F hexadecimal (to get time of servo_ctrl at a low ~20ms)
		TL1 = 0xF5;			//let the lower nibble of the timer 0 register equal F5 hexadecimal (to get time of servo_ctrl at a low ~20ms)
	}
	else {					//else if index equals 1
		Servo_Ctrl = 1;		//set Servo_Ctrl to 1
		TH1 = higherreload[anglenumber];	//let the higher nibble of the timer 0 register equal the value at position of anglenumber in the array of higherreload
		TL1 = lowerreload[anglenumber];		//let the lower nibble of the timer 0 register equal the value at position of anglenumber in the array of lowerreload
	}
}

/*--------------------------------------------------------------------------------------------------------------------
Function:  Timer2_Init
Description:  Initialise timer 2 ports and registers. Timer 2 is used to create the alarm when an object is within 100 mm
--------------------------------------------------------------------------------------------------------------------*/
void Timer2_Init()
{
	SFRPAGE = TMR2_PAGE;		//set SFR page to Timer 2 page
	TMR2CN = 0x04;		//enable timer
	TMR2CF = 0x0A;		//select SYSCLK as timer clock source
	RCAP2L = 0x62;
	RCAP2H = 0x45;		//these reload values ensure a square wave of 260 Hz can be generated.
}

/*--------------------------------------------------------------------------------------------------------------------
Function:  Timer3_Init
Description:  Initialise timer 3 ports and registers. Timer3 overflows every 1ms, and is used for the LCD initialisation.
--------------------------------------------------------------------------------------------------------------------*/
void Timer3_Init()
{
	SFRPAGE = TMR3_PAGE;  //set the SFR page to Timer 3 page
	TMR3CN = 0x04;  //enable the timer
	TMR3CF = 0x0A;  //select SYSCLK as timer clock source
	RCAP3L = 0x00;	//clear lower nibble of timer 1 register
	RCAP3H = LCDdelaytime;  //these reload values ensure a 1ms overflow for the timer. 
}

/*--------------------------------------------------------------------------------------------------------------------
Function:  Timer4_Init
Description:  Initialise timer 4 ports and registers. Timer4 is used to send the 40kHz pulse and to find the time is takes for an
Object to be detected or not.
--------------------------------------------------------------------------------------------------------------------*/
void Timer4_Init()
{
	SFRPAGE = TMR4_PAGE;		//set the SFR page to Timer 3 page
	TMR4CN = 0x04;		//enable the timer
	TMR4CF = 0x08;		//select SYSCLK as timer clock source
	RCAP4L = 0xCE;
	RCAP4H = 0xFE;		//these reload values ensure a 12.5 micro-sec overflow for the timer.

}

/*--------------------------------------------------------------------------------------------------------------------
Function:  Voltage_Reference_Init
Description:  Initialise voltage references
--------------------------------------------------------------------------------------------------------------------*/
void Voltage_Reference_Init()
{
	SFRPAGE = ADC0_PAGE; //set the SRF Page to the ADC0 page
	REF0CN = 0x03; // Turn on internal bandgap reference and output buffer to get 2.4V reference
}

/*--------------------------------------------------------------------------------------------------------------------
Function:         DAC_Init
Description:      Initialise DAC0
--------------------------------------------------------------------------------------------------------------------*/

void DAC_Init()
{
	SFRPAGE = DAC0_PAGE; //set the SRF Page to the DAC0 page
	DAC0CN = 0x98; //Set DAC output to update on Timer 2 overflow and enable DAC
}


/*--------------------------------------------------------------------------------------------------------------------
Function:  DAC_Deinit
Description:  Turns off the DAC. DAC is turned off whenever there is no keys pressed.
--------------------------------------------------------------------------------------------------------------------*/
void DAC_Deinit()
{
	SFRPAGE = DAC0_PAGE;  //set the SRF Page to the DAC0 page
	DAC0CN = 0x00; //Disable DAC
}

/*--------------------------------------------------------------------------------------------------------------------
Function: Display_Joshua_Radhika
Description: Display “Joshua Beverley & Radhika Feron” when in the innovation mode
--------------------------------------------------------------------------------------------------------------------*/

void Display_Joshua_Radhika()
{
	int n = 0;		//variable used for the for loop. 

	LCD = 0x88;	//Change the address so that characters can be displayed on the first line of the LCD.
	Toggle_E();	//Toggle the E line
	LCD = 0x80;	//Change the address so that characters can be displayed from the first column of the LCD. 
	Toggle_E();	//Toggle the E line

	for (n = 0;n != 15;n++) //16 characters in the first line “Joshua Beverley ” so the for loop needs to repeated 16 times to print all characters. 
	{
		LCD = (letter[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the character to display. 
		Toggle_E();	//Toggle the E line
		LCD = (letter[n] & 0x0F) | 0x90;	//Select the lower nibble of the character to display. 
		Toggle_E();	//Toggle the E line
	}

	LCD = 0x8C;	//Change the address so that characters can be displayed on the second line of the LCD. 
	Toggle_E();	//Toggle the E line
	LCD = 0x80;	//Change the address so that characters can be displayed from the first column of the LCD. 
	Toggle_E();	//Toggle the E line

	for (n = 15;n != 30;n++) //16 characters in the first line “& Radhika Feron ” so the for loop needs to repeated 16 times to print all characters. 
	{
		LCD = (letter[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the character to display. 
		Toggle_E();	//Toggle the E line
		LCD = (letter[n] & 0x0F) | 0x90;	//Select the lower nibble of the character to display. 
		Toggle_E();	//Toggle the E line
	}
}

/*--------------------------------------------------------------------------------------------------------------------
Function: AngleWord
Description: Display the word “Angle: ” when not in radar mode. Will only run once.
--------------------------------------------------------------------------------------------------------------------*/

void AngleWord()
{
	int n = 0;		//variable used for the for loop. 
	LCD = 0x88;	//Change the address so that characters can be displayed on the first line of the LCD.
	Toggle_E();	//Toggle the E line
	LCD = 0x80;	//Change the address so that characters can be displayed from the first column of the LCD. 
	Toggle_E();	//Toggle the E line
	for (n = 0;n != 7;n++) 	//7 characters in the line “Angle: ” so the for loop needs to repeated 7 times to print all characters. 
	{
		LCD = (angle[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the character to display. 
		Toggle_E();	//Toggle the E line
		LCD = (angle[n] & 0x0F) | 0x90;		//Select the lower nibble of the character to display. 
		Toggle_E();	//Toggle the E line
	}

}

/*--------------------------------------------------------------------------------------------------------------------
Function: DispAngle
Description: Display the angle value the servo motor is at when not in radar mode.
--------------------------------------------------------------------------------------------------------------------*/

void DispAngle()
{
	int n = 0;		//variable used for the for loop. 
	copyangle = anglenumber - 90;	//Subtract 90 from the anglenumber. Angle to be displayed on the LCD needs to be -90 <= angle <= 90. 
	LCD = 0x88;	//Change the address so that characters can be displayed on the first line of the LCD.
	Toggle_E();	//Toggle the E line
	LCD = 0x87;	//Change the address so that characters can be displayed from the eighth column of the LCD. 
	Toggle_E();	//Toggle the E line
	if ((copyangle) > 0)	//If angle is positive
	{
		LCD = (number[10] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the character to display. Angle is positive, so display a +. 
		Toggle_E();	//Toggle the E line
		LCD = (number[10] & 0x0F) | 0x90;	//Select the lower nibble of the character to display. 
		Toggle_E();	//Toggle the E line
	}
	else 	//Else angle is negative
	{
		LCD = (number[11] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the character to display. Angle is negative, so display a -. 
		Toggle_E();	//Toggle the E line
		LCD = (number[11] & 0x0F) | 0x90;	//Select the lower nibble of the character to display. 
		Toggle_E();	//Toggle the E line
	}

	copyangle = copyangle / 10;	//Divide the angle by 10, to get a number between -9 and 9
	if (copyangle < 0)
	{
		copyangle = 0 - copyangle;	//Make the negative numbers positive, no need for negatives once the sign has been printed. 
	}
	for (n = 0;n != 10;n++)	//Check for all 10 numbers. 
	{
		if (copyangle == n)	//If the angle digit to be printed is equal to n (this iteration of the for loop)
		{
			LCD = (number[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the number n to be displayed. 
			Toggle_E();	//Toggle the E line
			LCD = (number[n] & 0x0F) | 0x90;	//Select the lower nibble of the number n to be displayed. 
			Toggle_E();	//Toggle the E line	
			n = 9;		//Increment n to exit the for loop once the number has been printed. 
		}
	}
	copyangle = (anglenumber - 90) % 10;	//Moduluo the angle by 10, to find the remainder after dividing by 10. Effectively extracts the units digit. 
	if (copyangle < 0)
	{
		copyangle = 0 - copyangle;	//Make the negative numbers positive, no need for negatives once the sign has been printed. 
	}
	for (n = 0;n != 10;n++)	//Check for all 10 numbers. 
	{
		if (copyangle == n)	//If the angle digit to be printed is equal to n (this iteration of the for loop)
		{
			LCD = (number[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the number n to be displayed. 
			Toggle_E();	//Toggle the E line
			LCD = (number[n] & 0x0F) | 0x90;	//Select the lower nibble of the number n to be displayed. 
			Toggle_E();	//Toggle the E line	
			n = 9;		//Increment n to exit the for loop once the number has been printed. 
		}
	}
	LCD = (number[12] >> 4 & 0x0F) | 0x90;	//Display the degrees symbol at the end of the angle. Select the higher nibble. 
	Toggle_E();	//Toggle the E line
	LCD = (number[12] & 0x0F) | 0x90;	//Select the lower nibble of the degrees symbol to be displayed. 
	Toggle_E();	//Toggle the E line
}

/*--------------------------------------------------------------------------------------------------------------------
Function: DistanceWord
Description: Display the word “Distance: ” when not in radar mode. Will only run once.
--------------------------------------------------------------------------------------------------------------------*/

void DistanceWord()
{
	int n = 0;		//variable used for the for loop. 
	LCD = 0x8C;	//Change the address so that characters can be displayed on the second line of the LCD.
	Toggle_E();	//Toggle the E line
	LCD = 0x80;	//Change the address so that characters can be displayed from the first column of the LCD. 
	Toggle_E();	//Toggle the E line
	for (n = 0;n != 10;n++)	//10 characters in the line “Distance: ” so the for loop needs to repeated 10 times to print all characters. 
	{
		LCD = (distanceLCD[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the character to display. 
		Toggle_E();	//Toggle the E line
		LCD = (distanceLCD[n] & 0x0F) | 0x90;		//Select the lower nibble of the character to display. 
		Toggle_E();	//Toggle the E line
	}
}

/*--------------------------------------------------------------------------------------------------------------------
Function: DistanceDistance
Description: Display the distance a detected object is located from the transducer platform.
--------------------------------------------------------------------------------------------------------------------*/

void DispDistance(void)
{
	int n = 0;		//variable used for the for loop. 
	copydistance = distanceforprint;	//Copy the distance value so we can modify it without messing with the actual distance value. 
	LCD = 0x8C;	//Change the address so that characters can be displayed on the second line of the LCD.
	Toggle_E();	//Toggle the E line
	LCD = 0x8A;	//Change the address so that characters can be displayed from the eleventh column of the LCD. 
	Toggle_E();	//Toggle the E line
	if (copydistance == 0)
	{
		for (n=0;n!=5;n++)
		{
			LCD = (out_of_range[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the number n to be displayed. 
			Toggle_E();	//Toggle the E line
			LCD = (out_of_range[n] & 0x0F) | 0x90;	//Select the lower nibble of the number n to be displayed. 
			Toggle_E();	//Toggle the E line
		}
	}
	else 
	{
		copydistance = copydistance / 100;	//Divide by 100 to extract the 100’s digit. 
		for (n = 0;n != 5;n++)	//Check for numbers 0-4. After that numbers are out of range. 
		{
			if (copydistance == n)	//If the distance digit to be printed is equal to n (this iteration of the for loop)
			{
				LCD = (number[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the number n to be displayed. 
				Toggle_E();	//Toggle the E line
				LCD = (number[n] & 0x0F) | 0x90;	//Select the lower nibble of the number n to be displayed. 
				Toggle_E();	//Toggle the E line
				copydistance = distanceforprint - (n * 100);	//Subtract the 100’s digit from the distance number. Leaves a two digit number. 	
				n = 4;		//Increment n to exit the for loop once the number has been printed. 
			}
		}
		copydistance = copydistance / 10;	//Divide by 10 to extract the 10’s digit. 
		for (n = 0;n != 10;n++)	//Check for all 10 numbers. 
		{
			if (copydistance == n)	//If the distance digit to be printed is equal to n (this iteration of the for loop)
			{
				LCD = (number[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the number n to be displayed. 
				Toggle_E();	//Toggle the E line
				LCD = (number[n] & 0x0F) | 0x90;	//Select the lower nibble of the number n to be displayed. 
				Toggle_E();	//Toggle the E line
				n = 9;		//Increment n to exit the for loop once the number has been printed. 
			}
		}
		copydistance = distanceforprint % 100;	//Moduluo the angle by 100, to find the remainder after dividing by 100. Effectively extracts the tens and units digit. 
		copydistance = distanceforprint % 10;		//Moduluo the angle by 10, to find the remainder after dividing by 10. Effectively extracts the units digit. 
		for (n = 0;n != 10;n++)	//Check for all 10 numbers. 
		{
			if (copydistance == n)	//If the distance digit to be printed is equal to n (this iteration of the for loop)
			{
				LCD = (number[n] >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the number n to be displayed. 
				Toggle_E();	//Toggle the E line
				LCD = (number[n] & 0x0F) | 0x90;	//Select the lower nibble of the number n to be displayed. 
				Toggle_E();	//Toggle the E line
				n = 9;		//Increment n to exit the for loop once the number has been printed. 
			}
		}
		LCD = ('m' >> 4 & 0x0F) | 0x90;	//Display the millimeters units at the end of the distance. Select the higher nibble. 
		Toggle_E();	//Toggle the E line
		LCD = ('m' & 0x0F) | 0x90;		//Select the lower nibble of the millimeters symbol to be displayed. 
		Toggle_E();	//Toggle the E line
		LCD = ('m' >> 4 & 0x0F) | 0x90;	//Select the higher nibble of the millimeters symbol to be displayed. 
		Toggle_E();	//Toggle the E line
		LCD = ('m' & 0x0F) | 0x90;		//Select the lower nibble of the millimeters symbol to be displayed. 
		Toggle_E();	//Toggle the E line
	}
}

/*--------------------------------------------------------------------------------------------------------------------
Function:  LCD_Init
Description:  Initialise LCD. Note that the LCD has a 4 bit hardware connection
P3.7 BL => MPB pushbutton on the C8051F120 board.
P3.6 E => Enable
P3.5 R/W => Read/Write line
P3.4 RS => Register Select
P3.3 D7 => Data line
P3.2 D6 => Data line
P3.1 D5 => Data line
P3.0 D4 => Data line
--------------------------------------------------------------------------------------------------------------------*/
void LCD_Init()
{
	LCDdelaytime = 0x68;	//Delay time needs to be larger for the initialisation. 
	LCD = 0x80;
	Xms_Delay(20); 	// 20ms delay
	LCD = 0x83; 	// Function Set 
	Toggle_E(); 	// Toggle the E line
	Xms_Delay(5); 	// 5ms delay 
	LCD = 0x83; 	// Function Set 
	Toggle_E(); 	// Toggle the E line
	Xms_Delay(1); 	// 1ms delay 
	LCD = 0x83; 	// Function Set 
	Toggle_E(); 	// Toggle the E line
	Xms_Delay(1); 	// 1ms delay 
	LCD = 0x82; 	// Function Set 
	Toggle_E(); 	// Toggle the E line
	LCD = 0x82; 	// Function Set 
	Toggle_E();	//Toggle the E line
	LCD = 0x88; 	// 2 lines, 5x8 display 
	Toggle_E();	//Toggle the E line
	LCD = 0x80; 	// Display off 
	Toggle_E();	//Toggle the E line
	LCD = 0x88;
	Toggle_E();	//Toggle the E line
	LCD = 0x80; 	// Display clear display 
	Toggle_E();	//Toggle the E line
	LCD = 0x81;
	Toggle_E();	//Toggle the E line
	LCD = 0x80; 	// Cursor increment
	Toggle_E();	//Toggle the E line
	LCD = 0x86;
	Toggle_E();	//Toggle the E line
	LCD = 0x80; 	// Display on
	Toggle_E();	//Toggle the E line
	LCD = 0x8C;
	Toggle_E();	//Toggle the E line
	LCDdelaytime = 0xFC;	//Faster delay time selected for any LCD operations after the initialisation. 
}
/*--------------------------------------------------------------------------------------------------------------------
Function:  Xms_Delay
Description:  Generates an X ms delay according to the integer number input.
--------------------------------------------------------------------------------------------------------------------*/

void Xms_Delay(int p)
{
	int m = 0;   //variable used for the for loop
	for (m = 0; m < p; m++) //m will increment every 1ms when the timer 1 overflows and is reset. When it reaches n, the specified number of milliseconds, the for loop will break. 
	{
		Timer3_Init();   //initialise timer 1. 
		while (LCD_delay == 0) //while the LCD_delay variable hasn’t been set to 1, infinitely loop. The loop will break when timer 3 overflows. 
		{
		}
		LCD_delay = 0;  //reset the flag to 0 for next use
		TR3 = 0;		//enable timer
						//TMR3CN = 0x00; //turns the timer off, will be re initialised next iteration of the for loop.
	}
}
/*--------------------------------------------------------------------------------------------------------------------
Function:  Toggle_E
Description:  Toggle the E line for the LCD. There is a 1ms wait between toggles.
--------------------------------------------------------------------------------------------------------------------*/

void Toggle_E()
{
	E = ~E;    // invert E
	Xms_Delay(1);  // 1ms delay 
	E = ~E;    // invert E again, so it returns to the value it held when entering the function.
}


/*--------------------------------------------------------------------------------------------------------------------
Function:         Interrupts_Init
Description:      Initialise interrupts
--------------------------------------------------------------------------------------------------------------------*/
void Interrupts_Init()
{
	IE = 0xA8;
	EIE2 = 0x05;	//set enable flag of Timer 3,4

}


/*--------------------------------------------------------------------------------------------------------------------
Function:         Timer1_ISR
Description:      Timer 1 interrupt service routine
--------------------------------------------------------------------------------------------------------------------*/
void Timer1_ISR(void) interrupt 3
{
	unsigned char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
	timer1interrupt = 1;		//set timer1interrupt
	TF1 = 0;		//clear timer 0 interrupt flag        														// Reset interrupt flag
	SFRPAGE = SFRPAGE_SAVE; 		// Restore SFR page					      // Restore SFR page
}

/*--------------------------------------------------------------------------------------------------------------------
Function:         Timer2_ISR
Description:      Timer 2 interrupt service routine is used to send values to the DAC0 for the alarm
--------------------------------------------------------------------------------------------------------------------*/
void Timer2_ISR(void) interrupt 5
{
	unsigned char SFRPAGE_SAVE = SFRPAGE;			// Save Current SFR page
	SFRPAGE = DAC0_PAGE; //set the SFR page to the DAC0 page

	if (alarm_count <= 500)		//if alarm_count is less than 500
	{
		Sound_bit = ~Sound_bit; //invert the sound bit, represents when the square wave will be high or low
		if (Sound_bit == 0) //if the sound bit == 0
		{
			DAC0L = 0x00; //the value sent to the dac is zero, when the square wave is low
			DAC0H = 0x00;
		}
		else //if the sound bit = 1
		{
			DAC0L = 0xFF; //the value sent to the dac is the volume level chosen to get the square wave high
			DAC0H = 0x0F;
		}
		alarm_count = alarm_count + 1;	//increment alarm_count
	}
	else if (alarm_count>500 && alarm_count <= 1000) {	//else if alarm_count is greater than 500 and less than 1000
		DAC0L = 0x00; //the value sent to the dac is zero to keep the wave low for the 1 second on and 1 second off alarm		
		DAC0H = 0x00;
		alarm_count = alarm_count + 1;	//increment alarm_count
	}
	else {	//else if alarm_count is greater than 1000
		alarm_count = 0;		//clear alarm_count
		Sound_bit = 0;	//clear Sound_bit;
	}

	TF2 = 0;				// Reset interrupt flag
	SFRPAGE = SFRPAGE_SAVE; 	// Restore SFR page
}

/*--------------------------------------------------------------------------------------------------------------------
Function:  Timer3_ISR
Description:  Timer 3 interrupt service routine used for the LCD initialisation. Timer 3 will overflow every 1ms, triggering a flag (LCD_delay) which will be used to update the initialisation.
--------------------------------------------------------------------------------------------------------------------*/
void Timer3_ISR(void) interrupt 14
{
	unsigned char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
	LCD_delay = 1;
	TF3 = 0;  // Reset Interrupt
	SFRPAGE = SFRPAGE_SAVE; 			// Restore SFR page
}

/*--------------------------------------------------------------------------------------------------------------------
Function:         Timer4_ISR
Description:      Timer 4 interrupt service routine is used to set the timer4intterupt bit which controls the sending
Of the 40kHz pulse and the polling of the receiver.
--------------------------------------------------------------------------------------------------------------------*/


void Timer4_ISR(void) interrupt 16
{
	unsigned char SFRPAGE_SAVE4 = SFRPAGE;        // Save Current SFR page

	timer4interrupt = 1;		//set timer4interrupt

	TF4 = 0;        				// Reset interrupt flag
	SFRPAGE = SFRPAGE_SAVE4; 			// Restore SFR page
}
