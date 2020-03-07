;JB
;Radhika Feron
;ELEC2700 Computer Engineering 2
;Assignment 1: Pong 1D
;Date Due: 07/04/2017

$include (c8051f120.inc)        				; Includes register definition file

org   0000H
	ljmp  Start              							; Locate a jump to the start of code
org   0100H

;-----------------------------------------------------------------------------
; 			Initialisation for Peripheral Board
;-----------------------------------------------------------------------------

Start: 	
	mov 	WDTCN, 	#0DEh
	mov 	WDTCN, 	#0ADh
	mov 	SFRPAGE, 	#CONFIG_PAGE					; Use SFRs on the Configuration Page
	mov 	P0MDOUT, 	#00000000b						; Inputs 
	mov 	P1MDOUT, 	#00000000b						; Inputs
	mov 	P2MDOUT, 	#11111111b						; Outputs
	mov 	XBR2, 	#40h	          				; Enable the Port I/O Crossbar 
	ljmp	StopState

;--------------------------------- Memory Definitions ------------------------
; This all comments section is included to explain what is stored in all the memory locations in the program. 

; R0: Register 0 stores what speed has been selected (number from 0 to 9, used for the lookup table). 
SpeedPosition 			equ		R0
; R1: Register 1 stores the speed value and is used in the time delays to alter the delay length.
SpeedVariable				equ		R1 
; R2: Register 2 stores what player mode is selected (0 for one player mode, 1 for two player mode).
PlayerSelection			equ		R2
; R3: Register 3 is set to 5 and decremented to 0 as the ball moves left. Used to display the position of the ball. 
LeftMoveVariable		equ		R3
; R4: Register 4 is set to 5 and decremented to 0 as the ball moves right. Used to display the position of the ball. 
RightMoveVariable		equ		R4
; R5: Register 5 stores player 1's score
P1Score							equ		R5
; R6: Register 6 stores player 2's score 
P2Score							equ		R6
; R7: Register 7 is used for miscelanious calculations when more than the accumulator is needed. E.g. displaying score in pause state.
Miscellaneous				equ		R7
; 20h, 21h, 22h: Used in delay loops to pass time
DelayVariable1			equ		20h
DelayVariable2			equ		21h
DelayVariable3			equ		22h
; 23h: Stores what speed has been selected (the binary number used to light up the LEDs in the stop state).
SpeedSelection			equ		23h
; 24h, 25h: Duplicate scores that are made to be decremented by the scores flashing. 
P1ScoreDuplicate		equ		24h
P2ScoreDuplicate		equ		25h
; 26h: Stores whether the game has been completed or not (1 is stored here when a player reaches 15, otherwise 0 will be stored). 
HasTheGameBeenWon		equ		26h
; 27h: To pop from the stack when a subroutine is exited without returning. 
PopFromStack				equ		27h
; 28h: Stores what the winning score will be. Changed in the innovation state. 
WinningScore				equ		28h
; 29h: Stores whether the speed of the game will increase as the game goes on (1 if it will, 0 if it won't). Toggled in the innovation state. 
SpeedChangeMode			equ		29h
; 2Ah: Stores the control for probability mode (where the AI may or may not return the ball in 1P mode). Toggled in the innovation state. 
ProbabilityMode			equ		2Ah
; 2Bh: Stores the position of the data pointer in the probability lookup table. 
ProbDPTRPosition		equ		2Bh


;--------------------------------- Main Loop (Stop State) -----------------------
; StopState is the main loop for the 1D pong game. When the game is initially started, or whenever it is reset mid game, it will return to the stop state.
; In StopState, the player mode and speed is selected. 
; How does it work? A continuous loop is used repeatedly checking when buttons have been pressed. 
; When a button is pressed, loop is exited and program jumps to the debouncing check, before returning or progressing to another state. 
StopState:	
	; Whenever StopState is entered, everything is reset (scores, speeds etc.).
	; The accumulator, all registers (except R0) are reset to 0. 
	; P2 is set to 3, lighting up player 1 mode and initial selected speed
	mov 	A, #0h
	mov 	SpeedPosition, #01h									; The first value in the lookup table is the default speed. 
	mov		SpeedVariable, #00h									; Set R1 to 0, the speed will initially be 0 and will be changed in the stop loop. 
	mov 	PlayerSelection, #00h								; Set R2 to 0, one player mode will be the initial setting. 
	mov 	LeftMoveVariable, #00h							; Set R3 to 0, stops the ball from unnecessarily moving left.
	mov 	RightMoveVariable, #00h							; Set R4 to 0, stops the ball from unnecessarily moving right.
	mov 	P1Score, #00h												; Reset player 1's score to 0. 
	mov 	P2Score, #00h												; Reset player 2's score to 0. 
	mov 	Miscellaneous, #00h									; Set miscellanious register to 0. 
	mov 	HasTheGameBeenWon, #00h							; The game hasn't been won yet, so set this flag to 0. 
	mov 	WinningScore, #0Fh									; Winning score set to 15, can be changed in innovation mode.
	mov		SpeedChangeMode, #00h								; Speed change mode initially turned off, flag set to 0. 
	mov 	ProbabilityMode, #00h								; Probability mode initially turned off, flag set to 0. 
	mov 	ProbDPTRPosition, #00h							; Probability data pointer position variable set to 0 initially, is incremented when probability mode is selected. 
	mov 	P2, #03h														; Stop state begins with the first two LED's turned on. 

	; StopLoop is the continuous loop that will be exited when a button is pressed. 
	StopLoop: 
		jnb 	P1.3, SendToDebouncingPlay				; If pushbutton 4 is pressed, game is started. 
		jnb 	P1.2, Speed												; If pushbutton 3 is pressed, speed will be selected. 
		jnb 	P1.1, PlayerMode									; If pushbutton 2 is pressed, player mode will be selected.
		jnb		P1.5, SendToInnovationDebounce		;	If pushbutton 6 is pressed, innovation mode will be selected.
		jmp 	StopLoop
	
	; Short jump made here, long jump made from here to the debouncing code before starting the game. 
	SendToDebouncingPlay:
		ljmp	DebouncingStartGame								; Debouncing will be used to ensure the button is pressed once only. 

	; Short jump made here, long jump made from here to the debouncing code before entering the innovation state. 
	SendToInnovationDebounce:
		mov		P2, #0F0h													; When in innovation mode, LED's 5 to 8 will light up to show the change in mode. 
		mov 	SpeedChangeMode, #00h							; Turn speed increase mode off upon entering innovation. 
		ljmp 	InnovationDebounce								; Debouncing will be used to ensure the button is pressed once only. 
	
	; Player mode selects whether one or two players will play. 
	PlayerMode: 
		lcall DebouncingPlayers									; Debouncing will be used to ensure the button is pressed once only. 
		jnb 	P2.7, TwoPlayer										; If the 8th LED is currently off, jump to TwoPlayer to turn it on
		clr 	P2.7															; Turn 8th LED off. One player mode selected
		mov 	PlayerSelection, #00h							; Assign 0 to R2 to select one player mode
		jmp 	StopLoop
		
		TwoPlayer: 
			setb 	P2.7														; Turn 8th LED on. Two player mode selected
			mov 	PlayerSelection, #01h						; Assign 1 to R1 to select two player mode
			jmp 	StopLoop

	; Speed selects the speed the game is played. 
	Speed: 
		lcall 	DebouncingSpeed									; Debouncing will be used to ensure the button is pressed once only. 
		cjne 	SpeedPosition, #0Ah, Continue			; Checks that the data pointer hasn't reached the limit of the lookup table (10 numbers).
		mov 	SpeedPosition, #0h								; If so, reset R0 to 0 to reset the register dictating the data pointer position. 
		; With a corrected data pointer position, continue continues the speed function. 
		Continue: 
			mov 	DPTR, #SpeedSetting							; Move data pointer to the SpeedSetting lookup table. 
			mov 	A, SpeedPosition								; Move R0 to A, so the data pointer can use it. 
			movc	A, @A+DPTR											; Grab the value specified by A from the lookup table. 
			mov 	SpeedSelection, A								; Store the speed LED value in 23h so the accumulator can be used for other purposes
			mov 	DPTR, #SpeedValue								; Move data pointer to the SpeedValue lookup table.
			mov 	A, SpeedPosition								; Move R0 to A, so the data pointer to use it. 
			movc	A, @A+DPTR											; Grab the value specified by A from the lookup table. 
			mov 	SpeedVariable, A								; Saves the speed value in R1 so it can be used 
			jb 		P2.7, P2Mode										; If player 2 mode is currently set, jump to P2Mode
			jnb 	P2.7, P1Mode										; If player 1 mode is currently set, jump to P1Mode
			
			; Adds 80 to the value used to light up the LED's, so that the 8th LED can remain on if it was on previously. 
			P2Mode:
				mov 	A, SpeedSelection							; Moves the speed LED value to the accumulator. 
				add 	A, #80h												; Adds 80 to the speed LED value to continue to light up LED 8
				mov 	SpeedSelection, A							; Returns the new speed LED value to 23h. 
			
			; Skips P2 mode if P1 is selected. This function outputs the speed selection on the LED's
			P1Mode:
				mov 	P2, SpeedSelection						; Moves the speed LED value to the LED's to light up the speed. 
				mov 	A, SpeedPosition							; Increase R0 by one so that the data pointer will point to the next speed value. 
				add 	A, #01h
				mov 	SpeedPosition, A
				jmp 	StopLoop	

;--------------------------------- Innovation State -----------------------------
				
; A separate Innovation state is used to select "innovative" game modes.
; Innovations included:
;	The winning score can be changed to anything below 15. Setting it to 1 makes the game a "sudden death" mode, setting it to 2 means a player needs 2 to win etc. 
;	Speed change mode: as the game progresses, the speed the ball moves increases. If the maximum speed is reached, it is reset to the slowest speed. 
;	AI Probability mode: when turned on in one player mode, the AI will occasionally miss the ball. Can't be used in two player mode. 
Innovation:
	jnb 	P1.3, SendToDebouncingPlay					; If pushbutton 4 is pressed, just like in stop state, game is started. 
	jnb 	P1.5, SendToStopDebounce1						; If pushbutton 6 is pressed, the game will return to the stop state. SETTINGS MADE IN INNOVATION WILL BE FORGOTTEN. 
	jnb		P1.0, SendToDebounceScoreChange			; If pushbutton 1 is pressed, move to score change function where the winning score can be changed. 
	jnb 	P1.1, SendToDebounceSpeedChange			; If pushbutton 2 is pressed, speed change setting will be toggled. 
	jnb		P1.2, SendToDebounceAIProbability		; If pushbutton 3 is pressed, AI probability mode will be toggled. 
	jmp 	Innovation 

	; ScoreChange allows the score required to win to be changed. 
	ScoreChange: 
		mov 	P2, WinningScore													; Display the current winning score. 
		jnb		P1.1, SendToDebounceIncrease			; If pushbutton 2 is pressed, the winning score will be increased. 
		jnb 	P1.3, SendToDebouncingPlay				; If pushbutton 4 is pressed, just like in stop state, game is started. 
		jnb 	P1.5, SendToStopDebounce1					; If pushbutton 6 is pressed, the game will return to the stop state. SETTINGS MADE IN INNOVATION WILL BE FORGOTTEN. 
		jnb		P1.0, SendToInnovationDebounce		; If pushbutton 1 is pressed, will return to the innovation state. Winning score will be saved. 
		jmp 	ScoreChange

	; Short jump made here, long jump made from here to the increase function where the winning score will be incremented. 
	SendToDebounceIncrease:
		ljmp DebounceIncrease

	; Increase checks what the current score is; if it is less than 15 it will increment it by 1, if not it will be reset to 1. 
	Increase:
		mov A, WinningScore											; Move the current maximum score into the accumulator. 
		xrl A, #0Fh															; Exclusive OR with the current score, will result in 0 if the score is 15. 
		jz Reset																; If the score is 15, send to a subroutine to reset it the score to 1. 
		mov 	A, WinningScore										; Move the score back to the accumulator. 
		add 	A, #01h														; Increase the score by 1. 
		mov		WinningScore, A										; Move score back from accumulator to 28h. 
		jmp		ScoreChange												; Return to ScoreChange. 
	
		; Reset resets the winning score to 1 (sudden death mode). 
		Reset:
			mov 	WinningScore, #01h
			jmp ScoreChange

	; SpeedChange, when turned on, will increase the speed of the ball during a game. 
	SpeedChange:
		mov 	A, SpeedChangeMode								; Move the current on/off speed change setting to the accumulator. 
		jz DisplaySC														; If the setting is currently 0 (off), jump to DisplaySC to turn it on. 
		clr P2.1																; Turn off the speed change LED.
		mov SpeedChangeMode, #00h								; Set the on/off setting to off, speed change will be turned off. 
		jmp Innovation													; Return to the innovation state. 

		; DisplaySC is only entered when speed change mode is to be turned on, and it does exactly that. 
		DisplaySC:
			setb P2.1															; Turn on the speed change LED. 
			mov SpeedChangeMode, #01h							; Set the on/off setting to on, speed change will be turned on. 
			jmp Innovation												; Return to the innovation state. 

	; AIProbability, when turned on, will occasionally cause the AI in one player mode to miss the ball, allowing one person to have a competitive game. 
	AIProbability:
		mov 	A, PlayerSelection								; Move the player mode selection bit to the accumulator. 
		jnz		Innovation												; If two player mode has been selected in the stop state, AI probaility will be ignored and game will return to the innovation state. 
		mov		A, ProbabilityMode								; Move the current on/off AI probability setting to the accumulator. 
		jz 	DisplayAIProbability								; If the setting is currently 0 (off), jump to DisplayAI mode to turn it on. 
		clr 		P2.2														; Turn off the AI probability LED. 
		mov 	ProbabilityMode, #00h							; Set the on/off setting to off, AI probability will be turned off. 
		jmp Innovation													; Return to the innovation state. 

		DisplayAIProbability:
			setb P2.2															; Turn on the AI probability LED. 
			mov ProbabilityMode, #01h							; Set the on/off setting to on, AI probability will be turned on. 
			jmp Innovation												; Return to innovation. 

	; Jumped to after MovementLeft in the PlayState, AIProbCheckP1 will be executed when P1 mode is on, and will check if the probability game mode is turned on. 
	AIProbCheck1P:
		mov A, ProbabilityMode								 	; If the game mode is turned on, continue. If not, return the ball as normal. 				
		jz MovementRight												; Jump back to MovementRight to return the ball as normal. 
		mov 	Miscellaneous, ProbDPTRPosition		; Moves the data pointer to the spare register temporarily to allow the use of the CJNE command. 
		cjne 	Miscellaneous, #1Fh, ProbContinue	; Checks that the data pointer hasn't reached the limit of the lookup table (32 numbers).
		mov 	ProbDPTRPosition, #0h							; If so, reset 2Bh to 0 to reset the register dictating the data pointer position. 
		
		; With a corrected data pointer position, ProbContinue continues the AIProbCheck1P function. 
		ProbContinue:
		mov 	DPTR, #Probability								; Move data pointer to the Probability lookup table. 
		mov 	A, ProbDPTRPosition								; Move 2Bh to A, so the data pointer can use it. 
		movc	A, @A+DPTR												; Grab the value specified by A from the lookup table. 
		INC		ProbDPTRPosition									; Increment the data pointer position (stored in 2Bh). 
		jz MovementRight												; If the AI returns the ball (a 0 in the lookup table), will return the ball in MovementRight. 
		jmp P2Lose															; If the AI misses the ball (a 1 in the lookup table), will send to P2Lose, signalling the AI missing the ball. 
		
; Small jumps are made to here so that a long jump can be made to the DebouncingAIProbability function. 
SendToDebounceAIProbability:
	ljmp DebounceAIProbability

; Small jumps are made to here so that a long jump can be made to the DebouncingScoreChange function. 
SendToDebounceScoreChange:
	ljmp DebounceScoreChange

; Small jumps are made to here so that a long jump can be made to the DebouncingStop function. 
SendToStopDebounce1:
	ljmp DebouncingStop

; Small jumps are made to here so that a long jump can be made to the DebouncingSpeedChange function. 
SendToDebounceSpeedChange:
	ljmp DebounceSpeedChange

;--------------------------------- Play State -----------------------------------
	
; PlayState is the state when the ball is in motion. 
; How does it work? The ball is moved by displaying the LED's sequentially. 
; If a player misses the ball or swings their bat prematurely, P1Lose and P2Lose functions will add a point to the opponent's score. 
PlayState:
	mov		P2, #01h														; Start with LED 2 on. 
	mov 	LeftMoveVariable, #07h							; Sets number of bits to move left. 

	; InitServe is used to serve the ball to start the game. 
	InitServe:
		jnb 	P1.0, MovementLeft								; Serve successful, send to MovementLeft. 
		jnb 	P1.3, SendToStopDebounce1					; Debounce before moving to the stop state. 
		jnb 	P1.4, SendToPauseDebounce1				; If the pause button is pressed mid game, send to PauseState via a debouncing function. 
		jmp 	InitServe													; Serve hasn't occurred yet, repeat loop. 
	
	; MovementLeft moves the LED light 5 steps (the value of R3) to the left. 
	MovementLeft:
		mov 	A, P2															; Move the ball LED position into the accumulator
		rl 		A																	; In the accumulator, move the 1 bit to the left
		mov 	P2, A															; When moved back to LED, moves ball one to the left
		jnb		P1.7, P2Lose											; If player 2 presses button 8 before receiving the ball, it jumps to P2lose and P1 gains a point
		lcall ShortWait													; Time delay between ball movement
		djnz 	LeftMoveVariable, MovementLeft		; Repeat loop if ball hasn't reached the end

	mov 	RightMoveVariable, #07							; Sets number of bits to move right
	mov 	A, PlayerSelection									; Move R2 (player mode setting) to accumulator
	jz 		AIProbCheck1P												; If one player mode is selected, jump to AIProbCheck1P (in innovation) to check if the AI probability mode is turned on. 
	mov 	A, SpeedChangeMode									; Move 29h (speed change setting) to the accumulator. 
	jnz		ChangeSpeedP2												; If change speed mode is turned on, jump to ChangeSpeed to change the speed. 
	mov 	A, #00h								
	jnb 	P1.7, MovementRight									; Checks if player 2 hit the ball back. If so, move to MovementRight. 
	jmp 	P2Lose															; If not, send to P2Lose. 

;	JumpToDebounceP2Hit:
;	ljmp	DebouncingP2Hit											; Ball has been hit, send to debouncing to ensure the button has been held for long enough. 

	; MovementRight moves the LED light 5 steps (the value of R4) to the right. 	
	MovementRight:
		mov 	A, P2															; Move the ball LED position into the accumulator
		rr 		A 																; In the accumulator, move the 1 bit to the right
		mov 	P2, A															; When moved back to LED, moves ball one to the right
		jnb 	P1.0, P1Lose											; If player 1 presses button 1 before receiving the ball, it jumps to P1lose and P2 gains a point
		lcall 	ShortWait												; Time delay between ball movement
		djnz 	RightMoveVariable, MovementRight	; Repeat loop if ball hasn't reached the end

	mov 	LeftMoveVariable, #07								; Sets number of bits to move left
	mov 	A, SpeedChangeMode 									; Move 29h (speed change setting) to the accumulator. 
	jnz		ChangeSpeedP1												; If change speed mode is turned on, jump to ChangeSpeed to change the speed.
	mov 	A, #00h
	jnb 	P1.0, MovementLeft									; Checks if player 1 hit the ball back. If so, move to MovementLeft. 
	jmp 	P1Lose															; If not, send to P1Lose. 

	; If the change speed setting was turned on in innovation, the speed will be increased. Increments speed for when the ball is moving away from player 1. 
	ChangeSpeedP1:
		mov	 		A, SpeedVariable								; Moves the current speed value to the accumulator. 
		subb 		A, #0Ah													; Subtract 10 from the speed value, decreasing the inner loop controlling the delay by 10 loops
		lcall Speed0														; Calls the Speed0 function, which will take care of the situation caused by R1=0.  
		mov 	SpeedVariable, A									; Save the speed setting in R1. 
		jnb 	P1.0, MovementLeft								; Checks if player 1 hit the ball back. If so, move to MovementLeft. 
		jmp 	P1Lose														; If not, send to P1Lose. 						

	; If the change speed setting was turned on in innovation, the speed will be increased. Increments speed for when the ball is moving away from player 2. 
	ChangeSpeedP2:
		mov	 		A, SpeedVariable								; Moves the current speed value to the accumulator. 
		subb 		A, #0Ah													; Subtract 10 from the speed value, decreasing the inner loop controlling the delay by 10 loops
		lcall Speed0														; Calls the Speed0 function, which will take care of the situation caused by R1=0.  
		mov 		SpeedVariable, A								; Save the speed setting in R1. 
		jnb 	P1.7, MovementRight								; Checks if player 2 hit the ball back. If so, move to MovementRight. 
		jmp 	P2Lose														; If not, send to P2Lose. 

	; Checks if the speed value is 0. If not, will return. 
	Speed0:
		jz 		R1is0															; If speed value is 0, jump to R1is0. 
		ret
	
	; If the speed value is 0, reset to 250 (the slowest speed defined in the lookup table). 
	R1is0:
		mov 	SpeedVariable, #0FAh							; Reset speed value to 250 (0FAh). 
		ret

; Small jumps are made to here so that a long jump can be made to the DebouncingPause function. 
SendToPauseDebounce1:
	ljmp 	DebouncingPause

;	JumpToDebounceP1Hit:
;	ljmp	DebouncingP1Hit											; Ball has been hit, send to debouncing to ensure the button has been held for long enough. 
		
	; When player 1 loses a point, this will add a point to player 2, check whether the maximum score has been reached, and flash the score on the LED's. 
	P1Lose:
		mov 	A, P2Score												; Moves player 2 score into accumulator 
		add 	A, #01h														; Add 1 to score
		mov 	P2Score, A												; Move score back to register
		lcall CheckFor15												; Call to subroutine to check whether the score has reached 15
		mov 	P2ScoreDuplicate, P2Score					; Move player 2 score to 25h creating a duplicate that can be decremented in a loop. 
		lcall OuterLoopP2Win										; Call to subroutine that flashes the score for player 2 on LED 8. 

		; From here, moves to ServeP1 below
	
	; ServeP1 is used to serve the ball after player 1 loses a point. 
	ServeP1:
		mov 	P2, #01h													; Start on LED 2, wait for the serve
		mov 	LeftMoveVariable, #07							; Reset number of bits to move left
		jnb 	P1.0, SuccessfulServeP1			
		jnb 	P1.3, SendToStopDebounce					; Debounce before moving to the stop state. 
		jnb 	P1.4, SendToPauseDebounce1				; If the pause button is pressed mid game, send to PauseState via a debouncing function. 
		jmp 	ServeP1														; Serve hasn't occured yet, repeat loop. 
		
	; Small jump is made to SuccessfulServeP1 so a long jump can be made back to MovementLeft
	SuccessfulServeP1: 
		ljmp MovementLeft												; Serve successful, send to MovementLeft. 
	
	; When player 2 loses a point, this will add a point to player 1, check whether the maximum score has been reached, and flash the score on the LED's.
	P2Lose:
		mov 	A, P1Score												; Moves player 1 score into accumulator
		add 	A, #01h														; Add 1 to score
		mov 	P1Score, A												; Move score back to register
		lcall CheckFor15												; Call to subroutine to check whether the score has reached 15
		mov 	P1ScoreDuplicate, P1Score					; Move player 1 score to 24h creating a duplicate that can be decremented in a loop. 
		lcall 	OuterLoopP1Win									; Call to subroutine that flashes the score for player 1 on LED 1. 

		; From here, moves to ServeP2 below

	; ServeP2 is used to serve the ball after player 2 loses a point. 
	ServeP2:
		mov 	A, PlayerSelection
		jz		ServeP1
		mov 	P2, #80h													; Start on LED 7, wait for the serve
		mov 	RightMoveVariable, #07						; Reset number of bits to move right
		jnb 	P1.7, SuccessfulServeP2
		jnb 	P1.3, SendToStopDebounce					; Debounce before moving to the stop state. 
		jnb 	P1.4, SendToPauseDebounce1				; If the pause button is pressed mid game, send to PauseState via a debouncing function. 
		jmp 	ServeP2														; Serve hasn't occured yet, repeat loop. 

	; Small jump is made to SuccessfulServeP2 so a long jump can be made back to MovementRight
	SuccessfulServeP2: 
		ljmp MovementRight											; Serve successful, send to MovementRight.

;--------------------------------- Pause State ----------------------------------
		
; PauseState displays the score of the current game. Can exit back to the game, where players can continue playing where they left off. 
; Can also exit to the stop state, where the game will be reset. 	
PauseState:
	mov 	A, #00h															; Set accumulator to 0
	mov 	DPTR, #ScoreValue										; Move data pointer to the ScoreValue lookup table. 
	mov 	A, P2Score													; Move player 2's score to A, so the data pointer can return that selected value. 
	movc 	A, @A+DPTR 													; Grab the value specified by A from the lookup table. 
	swap 	A																		; Move the score returned from the lookup table to the higher nibble of A. 
	mov 	Miscellaneous, A										; Move the score to R7 for temporary storage. 
	mov 	DPTR, #ScoreValue										; Move data pointer to the ScoreValue lookup table. 
	mov 	A, P1Score													; Move player 1's score to A, so the data pointer can return that selected value
	movc 	A, @A+DPTR													; Grab the value specified by A from the lookup table. 
	add 	A, Miscellaneous										; Add the score from R7 (player 2's score stored in higher nibble) to player 1's score in the accumulator
	mov 	P2, A																; Display score
	
	; Repause is a loop that will be exited by either moving to the stop or play state. 
	RePause:
		jnb 	P1.4, SendToPlayDebounce					; Debounce before moving to the play state to continue the game. 
		jnb 	P1.3, SendToStopDebounce					; Debounce before moving to the stop state. 
		jmp 	RePause
		
; After debouncing, this function will move the progam to either MovementLeft or MovementRight to continue the game. 
SendToPlay:
	mov 	A, LeftMoveVariable									; Moves R3 into the accumulator. Should be the same value from when the game was paused. 
	jz 		ServeP2															; If R3=0, MovementRight was in progress when the game was paused. Jump to lower function, which will jump back to MovementRight. 
	mov 	A, RightMoveVariable								; Moves R4 into the accumulator. Should be the same value from when the game was paused. 
	jz 		ServeP1															; If R4=0, MovementLeft was in progress when the game was paused. Jump to lower function, which will jump back to MovementLeft. 
	ljmp 	StopState														; If this fails for whatever reason, instead of the system doing random things, it will just reset back to StopState
	
; Small jumps are made to here so that a long jump can be made to the DebouncingStop function. 
SendToStopDebounce:
	ljmp 	DebouncingStop

; Small jumps are made to here so that a long jump can be made to the DebouncingPause function. 
SendToPauseDebounce:
	ljmp 	DebouncingPause

; Small jumps are made to here so that a long jump can be made to the DebouncingStop function. 
SendToPlayDebounce:
	mov 	A, HasTheGameBeenWon								; The win game condition is moved to the accumulator
	jnz		Repause 														; If the play state is selected when a player has already scored 15, the program will return to the pause state. 
	ljmp 	DebouncingUnPause
	
	
;--------------------------------- Functions / Subroutines ----------------------
; ShortWait runs for 1 second when R1 = 250 (0FAh). Used as a delay while the ball is moving in game. 
; ShortWait isn't the average delay, it will also exit if the stop or pause buttons are pressed. Also, its speed changes depending on the value of R1. 
ShortWait: 									
	mov		DelayVariable1, #04h			
	ShortLoop2:  
		mov		DelayVariable2, #0FAh
		ShortLoop1:  
			mov		DelayVariable3, SpeedVariable		; The speed delay value. 
			ShortLoop0:  
				jnb 	P1.4, SendToPauseDebounce			; If the pause button is pressed mid game, send to PauseState via a debouncing function. 
				jnb 	P1.3, SendToStopDebounce			; If the stop button is pressed mid game, send to StopState via a debouncing function. 
				djnz  	DelayVariable3, ShortLoop0
			djnz	DelayVariable2, ShortLoop1
		djnz	DelayVariable1, ShortLoop2
	ret

; This function checks whether player 1 has reached 15 points
CheckFor15:
	mov A, P1Score														; Move player 1's score into the accumulator. 
	xrl A, WinningScore												; Exclusive OR logic will return a 1 if the two bits are not the same, and a 0 if they are. 
	jz WinGame																; If the bits are the same as a score of 15 (#0Fh), a player has won.
	mov A, P2Score														; Move player 2's score into the accumulator. 
	xrl A, WinningScore												; Exclusive OR logic will return a 1 if the two bits are not the same, and a 0 if they are. 
	jz WinGame																; If the bits are the same as a score of 15 (#0Fh), a player has won. 
	ret																				; If a player hasn't won the game, return. 

	; Long jumps back to the pause state once a player reaches 15 points. 
	WinGame:
		pop PopFromStack												; To avoid stack issues, pop one from the stack to account for exiting a subroutine with a jump not a return. 
		mov HasTheGameBeenWon, #01h							; Game has been completed. Stops the player from hitting play, only lets them exit by pressing stop. 
		ljmp PauseState													; Moves to PauseState

; This delay waits for ~60 milliseconds. Used for debouncing. 
DebounceWait60ms:
	mov 	DelayVariable1, #0FAh
	DebounceWait60msLoop2:
		mov 	DelayVariable2, #0FAh
		DebounceWait60msLoop1:
			mov 	DelayVariable3, #03h
			DebounceWait60msLoop0:
				djnz 	DelayVariable3, DebounceWait60msLoop0
			djnz	DelayVariable2, DebounceWait60msLoop1
		djnz 	DelayVariable1, DebounceWait60msLoop2
	ret

; When player 1 wins a point, flash the score on LED 1. 
OuterLoopP1Win:
	mov 	P2, #00h														; Turns off all lights on the LED screen, removing the ball. 
	setb 	P2.7																; Turn LED 1 on. 
	lcall 	FlashLoop													; Wait 500 ms. 
	clr 	P2.7																; Turn LED 1 off. 
	lcall 	Flashloop													; Wait 500 ms. 
	djnz 	P1ScoreDuplicate, OuterLoopP1Win		; Decrement the duplicate score and repeat the loop
	ret																				; Return when the loop is completed. 

; When player 2 wins a point, flash the score on LED 8. 	
OuterLoopP2Win:
	mov 	P2, #00h														; Turns off all lights on the LED screen, removing the ball. 
	setb 	P2.0																; Turn LED 8 on. 
	lcall 	FlashLoop													; Wait 500 ms. 
	clr 	P2.0																; Turn LED 8 off. 
	lcall 	Flashloop													; Wait 500 ms. 
	djnz 	P2ScoreDuplicate, OuterLoopP2Win		; Decrement the duplicate score and repeat the loop
	ret																				; Return when the loop is completed. 

; This delay waits for 500 milliseconds. Used for when the score is being flashed on either LED 1 or LED 8. 
FlashLoop:
	mov		DelayVariable1, #03h
	Flash2:
		mov		DelayVariable2, #0FAh
		Flash1:
			mov		DelayVariable3, #0FAh
			Flash0:
				jnb 	P1.3, SendToStopDebounce			; If the stop button is pressed while flashing, send to StopState via a debouncing function. 
				jnb 	P1.4, SendToPauseDebounce			; If the pause button is pressed while flashing, send to PauseState via a debouncing function. 
				djnz	DelayVariable3, Flash0
			djnz	DelayVariable2, Flash1
		djnz	DelayVariable1, Flash2
	ret

;--------------------------------- Debouncing -----------------------------------

; This debouncing loop used for push button 2 in the stop state when selecting one player/two player mode. 
DebouncingPlayers:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.1, DebouncingPlayers								; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ret																				; Successful debounce, return to the stop state.

; This debouncing loop used for push button 3 in the stop state when selecting the game speed. 
DebouncingSpeed:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.2, DebouncingSpeed									; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ret																				; Successful debounce, return to the stop state.

; This debouncing loop used for push button 5 in the pause state when unpausing the game (returning to the play state). 
DebouncingUnPause:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.4, DebouncingUnPause								; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp SendToPlay														; Successful debounce, jump to SendToPlay. 

; This debouncing loop used for push button 4 in the stop state when starting the game. 
DebouncingStartGame:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.3, DebouncingStartGame							; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp PlayState														; Successful debounce, jump to the PlayState

; This debouncing loop used for push button 5 in the play state when pausing the game. 	
DebouncingPause:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.4, DebouncingPause									; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp PauseState														; Successful debounce, jump to the PauseState

; This debouncing loop used for push button 4 in the play state when stopping the game. 
DebouncingStop:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.3, DebouncingStop									; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp StopState														; Successful debounce, jump to the StopState

; These debouncing loops were used for debouncing the ball being hit in game, though these aren't necessary and were removed. 
;DebouncingP1Hit:
;	lcall DebounceWait60ms
;	jnb P1.0, RepeatDebounceP1Hit
	;SuccessfulDebounce
;	ljmp P1Lose
;	RepeatDebounceP1Hit:
;	ljmp MovementLeft

;DebouncingP2Hit:
;	lcall DebounceWait60ms
;	jnb P1.7, RepeatDebounceP2Hit
	;SuccessfulDebounce
;	ljmp P2Lose
;	RepeatDebounceP2Hit:
;	ljmp MovementRight

;---------------------------- Debouncing (Innovation) ---------------------------

; This debouncing loop used for push button 6 in the stop state when entering the innovation state. 
InnovationDebounce:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.5, InnovationDebounce							; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp Innovation														; Successful debounce, jump to the Innovation state

; This debouncing loop used for push button 1 in the innovation state when selecting winning score change mode. 
DebounceScoreChange:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.0, DebounceScoreChange							; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp ScoreChange					 								; Successful debounce, jump to the score change mode. 

; This debouncing loop used for push button 2 in the innovation state when increasing the winning score in score change mode. 
DebounceIncrease:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.1, DebounceIncrease								; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp Increase															; Successful debounce, increase the winning score in score change mode. 
	
; This debouncing loop used for push button 2 in the innovation state when selecting speed change mode. 
DebounceSpeedChange:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.1, DebounceSpeedChange							; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp SpeedChange													; Successful debounce, jump to the speed change mode. 

; This debouncing loop used for push button 3 in the innovation state when selecting AI probability mode. 
DebounceAIProbability:
	lcall DebounceWait60ms										; Call the 60ms debouncing loop. 
	jnb P1.2, DebounceAIProbability						; If no button press is detected, repeat the loop. 
	;SuccessfulDebounce
	ljmp AIProbability												; Successful debounce, jump to the AI probability mode. 

;--------------------------------Lookup Table--------------------------------------
; To use this table, in main code use "mov dptr,#Some_Value"
; then move table index value into accumulator, then use "movc  a,@a+dptr",
; and finally output accumulator to the related output port

; Values for displaying the score in the pause state. 
ScoreValue:     	db	    	0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15  

; Values for displaying the score selected in the stop state. 
SpeedSetting:			db				3,7,15,31,63,127,63,31,15,7

; Speed values for the in game speed delay. Stored in P1. 
SpeedValue:				db				250,200,150,100,50,20,50,100,150,200,250

; Probability values for deciding when the AI will return the ball in AI probability mode (innovation state). 
Probability:			db				0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,1

END
