; Game mode routine.
; Input:  Speed (2=slow 1=medium 0=fast)
; Output: Score

.ASSUME ADL=1

KG1	EQU 0F50012h
KG7 EQU 0F5001Eh
MAXRAMP EQU 10
LCD_RAM_START EQU 0D40000h
LCD_CTRL_REG EQU 0E30018h
LCD_BASE_ADR EQU 0E30010h

;----------------------
SEGMENT BSS
INTERNAL_DATA:
	ds 30
;----------------------
SEGMENT CODE
XDEF _startGame
XREF _kb_Scan
_startGame:
	RET

	
	
	
	
	
	
	
	
	
	
	
	
	
	