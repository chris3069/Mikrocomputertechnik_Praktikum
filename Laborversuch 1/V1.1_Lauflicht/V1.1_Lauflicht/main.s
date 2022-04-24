;----------------------------------------------------------------------------
;	MCT-Praktikumsversuch V1.1 Nucleo
;	mit Lauflicht/Warteschleife in Assembler
;	
;	06/2020
; ----------------------------------------------------------------------------*/

RCC			EQU	0x40023800		; Adresse des Clock Control Registers
AHB1ENR		EQU	0x30			;   Offset AHB1 Peripheral Clock Enable Register
IOPBEN		EQU	0x02			;   I/O Port C Clock Enable
IOPCEN		EQU	0x04			;	I/O Port B Clock Enable

GPIOB		EQU 0x40020400		; Adresse des GPIO-Ports B
GPIOC		EQU 0x40020800		; Adresse des GPIO-Ports C
MODER		EQU 0x00			; 	Offset Modus
ODR			EQU 0x14			;	Offset Output Data Register
IDR			EQU 0x10			;	Offset Input Data Register

LEDEN		EQU 0x5555			; 	Output Mode Push/Pull max speed 50 MHz
SWITCHEN	EQU 0x0000			;	Floating Input

			THUMB
			AREA	myCode, CODE
			EXPORT 	Start
Start		PROC

		ldr r3, = RCC			; I/O-Clock initialisieren
		mov r0,#IOPCEN+IOPBEN
		str	r0,[r3,#AHB1ENR]
		
		ldr r3, = GPIOB			; GPIO Output initialisieren Port B 8...15
		ldr r0, = LEDEN
		str r0,[r3,#MODER]
		
		ldr r4, = GPIOC			; GPIO Input initialisieren Port C 0...7
		ldr r0, = SWITCHEN		
		str r0, [r4, #MODER]		;Control Register of GPIOC = Floating Input
		; ...
		
init_lauf
	    ldr r5, = WZ			; 1. Wartezeit
		mov r0,#0x01			; Startwert
		str r0,[r3,#ODR];Wert an ODR schreiben
		
loop			
	;Taster zuerst einlesen, ausmaskieren, da Pegel HIGH sind, mit XOR umdrehen
	ldr r7, [r4, #IDR]
	mov r8, #0xFFFF
	eor r7, r7, r8
	mov r6, r7
	
					;LED ansteuern
	str r0,[r3,#ODR];Wert an ODR schreiben
	
	
	ands r6, #0x1	;Taster, der die Richtung umkehrt?
	

	
	;r0,[r3,#ODR];Wert an ODR schreiben
	
	;Abfrage, ob T0 gedrueckt ist
	;T0 = 0, lsr logic shift right
	;else lsl logic shift left
	bne goleft

goright
	cmp r0, #0x01
	bne	cont		; Falls D1 leuchtet und lsr, starte bei D7
	mov r0, #0x100

cont
	lsr r0, #0x01 	;Bitverschiebung nach rechts
	;str r0,[r3,#ODR];Wert an ODR schreiben
	b mwait
goleft
	lsl r0, #0x01 	;Biterschiebung nach links
	;str r0,[r3,#ODR];Wert an ODR schreiben
	b mwait
		
mwait				;Bestimmtung der Wartedauer
	mov r6, r7
	and r6, #0x6 	;nur die Taster 1-2 werden angeguckt
	lsl r6, #0x1 	;die Zahl wird auf *4 angepasst, Taster 0 wird nicht angeguckt
	;daher nur mal 2. Die Wartezeit wird als Double gespeichert, 4 Byte
		
		; ...
		ldr r1,[r5, r6]				; Wartewert holen
		bl wait
		cmp r0,#0x080			; Ende erreicht?
		bgt	init_lauf			; ja  von vorn
		b loop				    ; nein weiter

;--------------------------------------
wait	push {r1,lr}
l_wait	subs r1,#1
		bne l_wait
		pop {r1,lr}
		bx   lr
;************************************************************
; Wartezeiten
WZ     DCD    0xC0000,0x60000,0x30000,0x18000
		
			ENDP
			END
			