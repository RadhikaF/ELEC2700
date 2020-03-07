# Assignment 1: Pong 1D
The objective of this assignment is to write an assembly language program to implement a simple 1D version of the game "Pong", using the C8051F120 microcontroller. Pong is a simple table tennis like arcade that was released by Atari in 1972, originally in 2D. It requires a player to return a ball sent by the opponent. If the opponent failed to return the ball, the player would score a point. The opponent could either be a second player, or a computer controlled opponent.

In this assignment, LEDs were used to simulate the moion of the ball in 1D, and to display the score. The pushbuttons control the 'bat' for the players, game mode, game speed, or pause. 
 
In a group of two, we successfully achieved the following outcomes;
1. Design a simple 8051 assembly language program
2. Build the associated machine code using Silicon Laboratories IDE
3. Download into the program memory of the microcontroller
4. Debug
5. Test software

## Innovation implemented
* The winning score can be changed to anything below 15.
* Speed change mode: as the game progresses, the speed the ball moves increases. If the maximum speed is reached, it is reset to the slowest speed. 
* AI Probability mode: when turned on in one player mode, the AI will occasionally miss the ball. Can't be used in two player mode. 
