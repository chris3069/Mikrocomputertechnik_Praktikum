/*----------------------------------------------------------------------------
  Funktionen für PWM (Servo und RGB-LED)
 *----------------------------------------------------------------------------*/

#include <stm32f4xx.h>

void InitTIM3_PWM(void)
// Initialisierung des Timer 3
{
	// GPIO Init
	// PA6 Servo, PA7 R, PC8 G, PC9 B
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;		// GPIOA Takt einschalten
	GPIOA->MODER &= 0xFFFF0FFF;							// GPIOA 6/7 auf Alternate Function schalten
	GPIOA->MODER |= 0x0000A000;
	GPIOA->AFR[0] &= 0x00FFFFFF;						// GPIOA 6/7 AF auf Timer 3 schalten (s. Tab 11 Datenblatt)
	GPIOA->AFR[0] |= 0x22000000;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;		// GPIOC Takt einschalten
	GPIOC->MODER &= 0xFFF0FFFF;							// GPIOC 8/9 auf Alternate Function schalten
	GPIOC->MODER |= 0x000A0000;
	GPIOC->AFR[1] &= 0xFFFFFF00;						// GPIOC AF auf Timer 3 schalten (s. Tab 11 Datenblatt)
	GPIOC->AFR[1] |= 0x00000022;

	// Timer init
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   	// Timer 3 Takt einschalten

	// ...
	
		TIM3->PSC = 160;		//10 yS
		TIM3->ARR = 2000; // 20 ms
	
		//Compare Register 1(Servo Motor)
	TIM3->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;	// Output Compare Mode: PWM Mode1(OC1M=110)
	TIM3->CCR1 = 150; // fahr den Servo in die Mittelstellung 1.5 ms
	TIM3->CCER = TIM_CCER_CC1E; 					//Capture Compare Enable, macht den Channel an
	// ...
	
			// Compare Register 2	(rote LED)
		TIM3->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
		TIM3->CCR2 = 100;		//95% rote Beleuchtung
		TIM3->CCER |= TIM_CCER_CC2P | TIM_CCER_CC2E; //low active

	
	// Compare Register 3	(grüne LED)
		TIM3->CCMR2 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
		TIM3->CCR3 = 100;		//gruene LED zu 5% an
		TIM3->CCER |= TIM_CCER_CC3P | TIM_CCER_CC3E; 

	  // ...
    // Compare Register 4	(blaue LED)
		TIM3->CCMR2 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
		TIM3->CCR4 = 100;		// blaue LED zu 5% an
		TIM3->CCER |= TIM_CCER_CC4P | TIM_CCER_CC4E; 
	
	
	  // ...
	  TIM3->EGR |= TIM_EGR_UG;          // Timer-Register initialisieren
	  TIM3->CR1 |= TIM_CR1_CEN;          // Counter Enable
}

void TIM3_servo( int pos)
// Servo auf Position fahren (0...100)
{
  if (pos <0 || pos >100)	
	{		// Abbruch, weil ausserhalb des Wertebereichs
		return;
	}
		TIM3->CCR1 = 100 + pos;
}





int TIM3_zahlenbereich(int zahl, int low, int high)
{
	if(zahl >= low && zahl <= high)		
	return 1;
	else
	return 0;
}


void map(unsigned int *value, int oberalt, int unteralt, int oberneu, int unterneu)
{
*value = oberneu/(double)oberalt * *value;  //oberneu - oberneu/(double)oberalt * *value
}


void TIM3_RGB(uint16_t red, uint16_t green, uint16_t blue)
// RGB-LED-Helligkeit setzen (0...255)
{
			map(&red, 255, 0, 2000, 0);
		map(&green, 255, 0, 2000, 0);
		map(&blue, 255, 0, 2000, 0);
	if (TIM3_zahlenbereich(red, 0, 2000))
	{

		TIM3->CCR2 = red;
	}
	
	if (TIM3_zahlenbereich(green, 0, 2000))
	{
		TIM3->CCR3 = green;
	}
	
		if (TIM3_zahlenbereich(blue, 0, 2000))
	{
		TIM3->CCR4 = blue;
	}

}
