; Game mode routine.
; Input:  Speed (2=slow 1=medium 0=fast)
; Output: Score

.ASSUME ADL=1

KG1	EQU 0F50012h
KG7 EQU 0F5001Eh
MAXRAMP EQU 10
LCD_RAM_START EQU 0D40000h
LCD_CTRL_REG EQU 0E30018h


;DO NOT REARRANGE THESE THINGS. STUFF WILL BREAK IF YOU DO.
SPEED        EQU 0  ;1 byte speed 1-3 (1=FAST, 2=MEDIUM 3=SLOW)
CURY         EQU 1  ;1 byte height 0-239
BLOCKS       EQU 2  ;3 byte mask (blocks 16px wide, 20 blocks per line, left aligned)
FLAGS        EQU 5  ;1 byte flags 
DISTANCE     EQU 6  ;1 byte distance in lines before blockspace state change
EMPTYMAXDIST EQU 7  ;1 byte maximum empty distance
CURRAMP      EQU 8  ;1 byte number of state switches before decreasing EMPTYMAXDIST
SCORE        EQU 9  ;4 byte score achieved (24.8 fp). Upper byte always stays 0
BALLX        EQU 13 ;1 byte ball X, resolution 0:159 (half-res)
BALLY        EQU 14 ;1 byte ball Y
TEMP         EQU 15 ;1 byte temporary looping variable



F_BLOCKSPACE EQU 0 ;-- 0:empty (distance EMPTYMAXDIST) 1:blockthickness (distance 8)
F_RESERVED1  EQU 1
F_RESERVED2  EQU 2
F_RESERVED3  EQU 3
F_RESERVED4  EQU 4
F_RESERVED5  EQU 5
F_RESERVED6  EQU 6
F_RESERVED7  EQU 7
;----------------------
	SEGMENT BSS
INTERNAL_DATA:
	ds 30
;----------------------
	SEGMENT DATA
INITIAL_DATA:
	db   0       ;height
	dw24 0       ;blocks
	db   0       ;flags
	db   239     ;distance
	db   120     ;emptymaxdist
	db   MAXRAMP ;curramp
	dl   0       ;score (24.8 fp)
	db   0       ;bally
	db   76      ;ballx
;----------------------
	SEGMENT CODE
	XDEF _startGame
	XREF _kb_Scan
_startGame:
	POP DE		;RETURN ADDRESS
	EX (SP),HL	;FETCH FUNCTION ARGUMENT WITHOUT CHANGING STACK LEVELS
	PUSH DE		;RESTORE RETURN ADDRESS
	PUSH IX
		;-- CLEAN INITIALIZE VARIABLE SPACE
		LD IX,INTERNAL_DATA
		LD (IX+SPEED),L
		LEA DE,IX+1  ;TO SKIP PAST SPEED
		LD HL,INITIAL_DATA
		LD BC,14
		LDIR
		;-- SET 1BPP SCREEN MODE
		LD HL,LCD_CTRL_REG
		LD A,(HL)
		AND 11110001b  ;blank out bits to set 1bpp mode
		LD (HL),A
		;-- SET BLACK & WHITE PALETTE
		LD HL,0E30200h
		DEC BC         ;BC NOW -1 (WHITE)
		LD (HL),BC
		INC HL
		INC HL
		INC BC         ;BC NOW 0 (BLACK)
		LD (HL),BC
		;-- RESET SCREEN BUFFER POINTER
		LD HL,LCD_RAM_START
		CALL SET_SCREEN_POINTER
		;-- START THE GAME
		CALL GAME_MODE
		LD HL,(IX+SCORE)
	POP IX
	RET
	


	
GAME_MODE:
	CALL _kb_Scan
	;IMPLEMENTING THROTTLE
;	LD B,(IX+SPEED)
;	INC B
GAME_MODE_THROTTLE:
;	LD HL,0E30028h
;	SET 3,(HL)
;	LD L,0020h
GAME_MODE_THROTTLE_WAITLOOP:
;	BIT 3,(HL)
;	JR Z,GAME_MODE_THROTTLE_WAITLOOP
;	DJNZ GAME_MODE_THROTTLE
	;ERASE BALL'S PREVIOUS LOCATION
	;-
	
	;DETECT BALL'S NEXT LOCATION. QUIT IF BALL GOES ABOVE PLAY AREA
	;-
	;DETECT USER QUIT
	LD A,(0F50012h)  ;KEYGROUP 1
	BIT 6,A          ;MODE
	RET NZ
	;RENDER BALL'S NEW LOCATION
	;-
	;UPDATE SCORE BASED ON HEIGHT AND DISTANCE
	;-
	;DRAW NEXT LINE DOWN. JUST BEYOND THE SCREEN'S REACH
	
	LEA HL,IX+DISTANCE     ;-1 FLAGS +0 DIST +1 MAXDIST
	DEC (HL)   ;DIST
	JR NZ,GAME_MODE_NO_LINE_MODE_CHANGE
	;DISTANCE IS NOW ZERO. CHECK FOR MODE CHANGE
	DEC HL
	BIT F_BLOCKSPACE,(HL)
	JR Z,GAME_MODE_CHANGE_TO_LINES
	RES F_BLOCKSPACE,(HL)    ;MODE NOW CHANGING TO EMPTY SPACE. 
	INC HL
	LD DE,HL      ;DE = DIST
	INC HL        ;HL = EMPTYMAXDIST
	LDI           ;STORE MAX DIST TO DIST. DE = EMPTYMAXDIST, HL = CURRAMP
	DEC (HL)
	JR NZ,GAME_MODE_FINISH_CHANGING_LINE_MODE
	LD (HL),MAXRAMP  ;RESET RAMP BACK UP TO WAIT FOR NEXT MAX DISTANCE REDUCTION
	DEC HL
	DEC (HL)         ;REDUCE MAX DISTANCE
	JR GAME_MODE_FINISH_CHANGING_LINE_MODE
GAME_MODE_CHANGE_TO_LINES:
	SET F_BLOCKSPACE,(HL)
	INC HL
	LD (HL),04h    ;BLOCKS ARE 4 PIXELS HIGH (DISTANCE)
	;GENERATE LINES (and not just make a big dark line across the screen like we're doing now for debugging purposes)
	LD HL,0FFFFFFh
	LD (IX+BLOCKS),HL
	;-----
GAME_MODE_NO_LINE_MODE_CHANGE:
GAME_MODE_FINISH_CHANGING_LINE_MODE:
	LD DE,LCD_RAM_START + ((320/8)*240)
	LD H,40
	LD A,(IX+CURY)
	LD L,A
	PUSH AF
		MLT HL      ;OFFSET TO LINE START
		ADD HL,DE   ;POINTER TO LINE JUST BELOW THE CURRENT SCREEN
		CALL DRAW_LINE_SEGMENT  ;DRAW LINE JUST PAST THE TOP OF THE SCREEN
		PUSH HL     ;SAVE POINTER TO JUST PAST THE END.
			LD DE,#-((320/8)*(240-1))+39
			ADD HL,DE   ;SET POINTER TO START OF ONSCREEN ROW 1
			PUSH HL     ;SAVE THAT.
				CALL SET_SCREEN_POINTER  ;AND SET NEW SCREEN COORDS TO THIS.
			POP HL
			DEC HL		;GET POINTER LESS ONE
		POP DE			;AND THE END OF THE LINE THAT WAS WRITTEN.
		LD BC,39		;AND
		LDDR			;COPY THAT LINE TO THE PART OF THE SCREEN WE JUST PUSHED OUT
	POP AF
	INC A
	CP 240
	JR NZ,GM_SKIP_CURY_RESET
	XOR A
GM_SKIP_CURY_RESET:
	LD (IX+CURY),A
	JP GAME_MODE
	
;----------------------------
DRAW_LINE_SEGMENT:
	BIT F_BLOCKSPACE,(IY+FLAGS)
	JR NZ,DRAW_LINE_BARRIERS
DRAW_LINE_EMPTY:
	LD BC,39
	LD DE,HL
	INC DE
	LD (HL),00h
	LDIR
	RET
DRAW_LINE_BARRIERS:
	LD BC,#(20*256)+0
	LD DE,(IX+BLOCKS)
	EX DE,HL
DRAW_LINE_BARRIERS_LOOP:
	XOR A
	ADD HL,HL
	SBC A,A
	LD (DE),A
	INC DE
	LD (DE),A
	INC DE
	DJNZ DRAW_LINE_BARRIERS_LOOP
	EX DE,HL
	DEC HL
	RET

	
;in: HL = ptr to set (reminder: this is in 1bpp mode). destroys HL.
SET_SCREEN_POINTER:
	LD (0E00010h),HL
	LD HL,0E30028h
	SET 2,(HL)
	LD L,0020h
SET_SCREEN_POINTER_LOOP:
	BIT 2,(HL)
	JR Z,SET_SCREEN_POINTER_LOOP
	RET
	
DRAW_SPRITE:	
	LD L,(IX+CURY)
	LD E,(IX+BALLY)
	LD H,40
	LD D,H
	MLT HL
	MLT DE
	ADD HL,DE
	LD DE,LCD_RAM_START
	LD A,(IX+BALLX+0)
	LD C,A
	AND A,11111100b
	
	
	
	AND A,00000111b
	XOR A,00000111b
	INC A     ;Oin->8out, 7in->1out
	LD C,A    ;C = MASK LOOP VARIABLE
DRAW_SPRITE_LOOP:
	LD DE,SPRITE_DATA
	LD (IX+TEMP),16
	
	
	
	
	PUSH BC
		PUSH HL
			
	
	
	LD A,(DE)
	XOR (HL)
	LD (HL),A
	INC HL
	INC DE
	
	
	
	


	
SPRITE_MASK
SPRITE_DATA:
db 00000111b, 11100000b
db 00011111b, 11111000b
db 00111111b, 11111100b
db 01111111b, 11111110b
db 01111111b, 11111110b
db 11111111b, 11111111b
db 11111111b, 11111111b
db 11111111b, 11111111b
db 11111111b, 11111111b
db 11111111b, 11111111b
db 11111111b, 11111111b
db 01111111b, 11111110b
db 01111111b, 11111110b
db 00111111b, 11111100b
db 00011111b, 11111000b
db 00000111b, 11100000b

