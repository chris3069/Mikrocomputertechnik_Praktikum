;----------------------------------------------------------------------------
;	MCT-Praktikumsversuch V1.2 Nucleo
;	mit Lauflicht/SysTick in Assembler
;	
;	06/2020
; ----------------------------------------------------------------------------*/

ODR			EQU 0x14				;	Offset Output Data Register
LOAD		EQU	0x04			;	Offset Load-Wert
IDR			EQU 0x10			;	Offset Input Data Register
GPIOC		EQU 0x40020800		; Adresse des GPIO-Ports C
SYSTICK		EQU 0xe000e010		; Adresse des SysTick-Timers	




			THUMB
			AREA	myCode, CODE
			IMPORT Init_GPIO
			IMPORT Init_SysTick
			IMPORT SCB_SHPR3
			IMPORT GPIOB
;			IMPORT GPIOC	
;			IMPORT LOAD
;			IMPORT IDR
;			IMPORT SYSTICK


			EXPORT Start
			EXPORT SysTick_Handler

Start		PROC

			; I/O initialisieren
			bl Init_GPIO
 			ldr r3, = GPIOB	    	; LED0 setzen
			mov r0, #0x01
			str r0,[r3,#ODR]

			bl Init_SysTick

FOREVER	
			b		FOREVER			; nix


;-----------------------------------------------------------------------
SysTick_Handler

	;LED ansteuern
			;push {r0,lr}			; wird automatisch gerettet
			ldr r0,[r3,#ODR]		; Status LED einlesen
			cmp r0,#0x80
			beq	 INIT
SHIFT  		lsl r0,r0,#1
			b 	OUT
INIT		mov r0,#0x01
			b 	OUT
OUT			str r0,[r3,#ODR]		; LED	ausgeben
			;pop {r0,lr}			; wird automatisch zurueckgelesen

									
	;Taster zuerst einlesen, ausmaskieren, da Pegel HIGH sind, mit XOR umdrehen
	ldr r4, = GPIOC			; GPIO Input initialisieren Port C 0...7
	ldr r7, [r4, #IDR]
	mov r8, #0xFFFF
	eor r7, r7, r8
	mov r6, r7
	and r6, #0x3 	;nur die Taster 1-2 werden angeguckt
						;Systick Timer auf Taster Stellung anpassen 
	ldr		r1, = SYSTICK	; Timer init
	ldr 	r3, =46875
	lsl r3, r6 
	str r3, [r1, #LOAD]
	bx 		lr
;-----------------------------------------------------------------------		
			ENDP
			END
			