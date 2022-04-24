; ** Startup Code für Assembler
; ** Haunstetter / 06.2012
; ** STM32F100RB - Value Line Discovery
; **
; ** Bereitet die Vektor Tabelle vor
; ** Verzweigt zu Adresse "Start" in anderem Modul des Projekts
; *********************************************************************

			;; Jump to Application Entry Point using THUMB Mode Instructions 
			THUMB
			AREA	init, CODE
Reset_Handler\
			PROC
			EXPORT	Reset_Handler
			IMPORT  Start
			IMPORT SysTick_Handler
			
			b		Start							; Starts right after vectors
			ENDP

; Assembler basic configuration  (32-Bit aligned Stack, THUMB Instruction Set)
			;; Stack Allocation: 1kB after User's Data
			AREA    stack, DATA, NOINIT, ALIGN=2
Stack		SPACE   0x00000400
			EXPORT	Stack
__initial_sp

			;; Vector Table: is Mapped to Address 0 at Reset
			AREA    RESET, DATA, READONLY
			EXPORT	__Vectors
;			IMPORT	SVC_Handler
;			IMPORT	SysTick_Handler
__Vectors	DCD     __initial_sp		            ; Top of Stack (max. 0x20002000)
			DCD     Reset_Handler                   ; Reset Handler
			DCD     NMI_Handler                     ; NMI Handler
			DCD     HardFault_Handler               ; Hard Fault Handler
			DCD		0								; 7 unused System Interruts
			DCD		0
			DCD		0
			DCD		0
			DCD		0
			DCD		0
			DCD		0
			DCD		SVC_Handler						; ** Supervisor Call **
			DCD		0								; 3 unused System Interruts
			DCD		0
			DCD		0
			DCD		SysTick_Handler								; ** System Tick Timer **
			;; EXTInt Space Starts Here

			AREA    handler, CODE, READONLY
Exception_Handler\
			PROC
NMI_Handler
HardFault_Handler
SVC_Handler
;SysTick_Handler
			B       .							; All exception handlers end here
;			B		.
;			B		.
			ENDP
			END
