RCC			EQU	0x40023800		; Adresse des Clock Control Registers
AHB1ENR		EQU	0x30			;   Offset AHB1 Peripheral Clock Enable Register
IOPCEN		EQU	0x02			;   I/O Port C Clock Enable
IOPBEN		EQU	0x04			;	I/O Port B Clock Enable

GPIOB		EQU 0x40020400		; Adresse des GPIO-Ports B
GPIOC		EQU 0x40020800		; Adresse des GPIO-Ports C
MODER		EQU 0x00			; 	Offset Modus
ODR			EQU 0x14			;	Offset Output Data Register
IDR			EQU 0x10			;	Offset Input Data Register

LEDEN		EQU 0x5555			; 	Output Mode Push/Pull max speed 50 MHz
SWITCHEN	EQU 0x0000			;	Floating Input

SYSTICK		EQU 0xe000e010		; Adresse des SysTick-Timers
CTRL		EQU	0x00			;	Offset Kontrollregister
LOAD		EQU	0x04			;	Offset Load-Wert

CTRL_RUN	EQU	0x01			;	SysTick Starten
CTRL_IEN	EQU 0x02			;	Interrupt aktivieren
CTRL_SYSCLK	EQU 0x04			;	Taktvorgabe

SCB_SHPR3	EQU 0xE000ED20		; System Handler Priority Register
	
; Code memory region
			THUMB
			AREA	myIO, CODE
			
Start		PROC
			EXPORT Init_GPIO
			EXPORT Init_SysTick
			EXPORT SCB_SHPR3
			EXPORT GPIOB
;----------------------------------------------------
Init_GPIO
			push	{r0-r10,lr}

			ldr r3, = RCC			; I/O-Clock für Port B und C initialisieren
			mov r0,#IOPCEN+IOPBEN
			str	r0,[r3,#AHB1ENR]
		
			ldr r3, = GPIOB			; GPIO Output initialisieren Port B 0...7
			ldr r0, = LEDEN
			str r0,[r3,#MODER]
		
			ldr r4, = GPIOC			; GPIO Input initialisieren Port C 0...7
			ldr r0, = SWITCHEN
			str r0,[r4,#MODER]
			pop  {r0-r10,lr}
			bx		lr
;----------------------------------------------------
Init_SysTick
			push	{r0-r10,lr}
			ldr		r1, = SYSTICK	; Timer init
			ldr 	r3, =46875
			str 	r3, [r1, #LOAD]
			mov 	r3, #CTRL_RUN+CTRL_IEN
			str 	r3,[r1, #CTRL]


			;Taster einlesen


			
			; ...

			pop  {r0-r10,lr}
			bx		lr
			
			ENDP
			END
