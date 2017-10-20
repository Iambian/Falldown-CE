; Game mode routine.


KG1	EQU 0F50012h
KG7 EQU 0F5001Eh
MAXRAMP EQU 10
LCD_RAM_START EQU 0D40000h
LCD_CTRL_REG EQU 0E30018h
LCD_BASE_ADR EQU 0E30010h

;----------------------
SEGMENT BSS
randData:
	ds 3
INTERNAL_DATA:
	ds 30
;----------------------
SEGMENT CODE
.ASSUME ADL=1

XDEF _clearRectFast

XREF _kb_Scan
XREF _randInt


_clearRectFast:
	POP DE
	EX (SP),HL
	PUSH DE
	LD A,L
	CP 240
	RET NC
	LD H,160
	MLT HL
	ADD HL,HL
	LD DE,(0E30014h)
	ADD HL,DE
	PUSH HL
	POP DE
	INC DE
	LD (HL),0FFh
	CPL
	ADD A,241
	ADD A,A
	LD C,A
	LD B,160
	MLT BC
	DEC BC
	LDIR
	RET


	
	