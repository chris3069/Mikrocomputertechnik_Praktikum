//Schmid Christian
// 19.08.2020
// Hier steht die Systick Interrupt vom Timer in C


#define __IO volatile							// Datentyp flüchtig
#define __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define __IOM    volatile            /*! Defines 'read / write' structure member permissions */
	
#define PERIPH_BASE         0x40000000UL /*!< Peripheral base address in the alias region                                */
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)	// Basisadresse Peripherie
#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800UL)	// RCC-Basisadresse
#define RCC               	((RCC_TypeDef *) RCC_BASE)		// RCC-Register
#define GPIOB_BASE          (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE          (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address */
#define SysTick_BASE        (SCS_BASE +  0x0010UL)                    /*!< SysTick Base Address */
#define SysTick             ((SysTick_TypeDef   *)     0xe000e010  )   /*!< SysTick configuration struct */

typedef unsigned int uint32_t;		// Datentyp 32bit Integer

typedef struct
// Struktur für RCC-Register
{
  __IO uint32_t CR;
  __IO uint32_t PLLCFGR;
  __IO uint32_t CFGR;
  __IO uint32_t CIR;
  __IO uint32_t AHB1RSTR;
  __IO uint32_t AHB2RSTR;
  __IO uint32_t AHB3RSTR;
  uint32_t      RESERVED0;
  __IO uint32_t APB1RSTR;
  __IO uint32_t APB2RSTR;
  uint32_t      RESERVED1[2];
  __IO uint32_t AHB1ENR;
  __IO uint32_t AHB2ENR;
  __IO uint32_t AHB3ENR;
  uint32_t      RESERVED2;
  __IO uint32_t APB1ENR;
  __IO uint32_t APB2ENR;
  uint32_t      RESERVED3[2];
  __IO uint32_t AHB1LPENR;
  __IO uint32_t AHB2LPENR;
  __IO uint32_t AHB3LPENR;
  uint32_t      RESERVED4;
  __IO uint32_t APB1LPENR;
  __IO uint32_t APB2LPENR;
  uint32_t      RESERVED5[2];
  __IO uint32_t BDCR;
  __IO uint32_t CSR;
  uint32_t      RESERVED6[2];
  __IO uint32_t SSCGR;
  __IO uint32_t PLLI2SCFGR;
  __IO uint32_t PLLSAICFGR;
  __IO uint32_t DCKCFGR;
  __IO uint32_t CKGATENR;
  __IO uint32_t DCKCFGR2;
} RCC_TypeDef;

typedef struct
// Struktur für GPIO-Register
{
  __IO uint32_t MODER;    // Modus
  __IO uint32_t OTYPER;   // Ausgabetyp
  __IO uint32_t OSPEEDR;  // Geschwindigkeit
  __IO uint32_t PUPDR;    // Pull up/down
  __IO uint32_t IDR;      // Eingangsdaten
  __IO uint32_t ODR;      // Ausgangsdaten
  __IO uint32_t BSRR;     // Regsiter, um einzelne Bits zu setzen/rücksetzen
  __IO uint32_t LCKR;     // Sperrung
  __IO uint32_t AFR[2];   // Alternative Funktionen
} GPIO_TypeDef;


typedef struct
	//Struktur für Systick Timer
{
  __IO uint32_t STK_CTRL;                    /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
  __IO uint32_t STK_LOAD;                    /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register       */
  __IO uint32_t STK_VAL;                     /*!< Offset: 0x008 (R/W)  SysTick Current Value Register      */
  __IO uint32_t STK_CALIB;                   /*!< Offset: 0x00C (R/ )  SysTick Calibration Register        */
} SysTick_TypeDef;





