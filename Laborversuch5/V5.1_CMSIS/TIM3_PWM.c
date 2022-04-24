/*----------------------------------------------------------------------------
  Funktionen für PWM (Servo und RGB-LED)
 *----------------------------------------------------------------------------*/

#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_tim.h>

void InitTIM3_PWM(void)
// Initialisierung des Timer 3
{
	LL_GPIO_InitTypeDef GPIO_InitStructure;
	LL_TIM_InitTypeDef TIM_InitStructure;
	LL_TIM_OC_InitTypeDef TIM_OCInitStructure;
	
	// GPIO Init: PA6 Servo, PA7 R, PC8 G, PC9 B

	// PA6/7 initialisieren
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);			// GPIOA Takt einschalten
	GPIO_InitStructure.Pin=LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
	GPIO_InitStructure.Mode=LL_GPIO_MODE_ALTERNATE;							
	GPIO_InitStructure.Speed=LL_GPIO_SPEED_FREQ_LOW;				
	GPIO_InitStructure.OutputType=LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStructure.Pull=LL_GPIO_PULL_NO;
	GPIO_InitStructure.Alternate=LL_GPIO_AF_2;								// Alternate Function TIM3
	LL_GPIO_Init(GPIOA,&GPIO_InitStructure);

	// PC8/9 initialisieren
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);			// GPIOC Takt einschalten
	GPIO_InitStructure.Pin=LL_GPIO_PIN_8 | LL_GPIO_PIN_9;
	GPIO_InitStructure.Mode=LL_GPIO_MODE_ALTERNATE;							
	GPIO_InitStructure.Speed=LL_GPIO_SPEED_FREQ_LOW;				
	GPIO_InitStructure.OutputType=LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStructure.Pull=LL_GPIO_PULL_NO;
	GPIO_InitStructure.Alternate=LL_GPIO_AF_2;								// Alternate Function TIM3
	LL_GPIO_Init(GPIOC,&GPIO_InitStructure);

	// Timer initialisieren
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);				// TIM3 Takt einschalten
	TIM_InitStructure.Prescaler=159;													// Prescaler  => f = 16 MHz/160 = 100 kHz
	TIM_InitStructure.Autoreload=2000;												// Auto Reload-Wert => 100 kHz/2000 = 50 Hz für Grundfrequenz PWM/Servo
	TIM_InitStructure.ClockDivision=LL_TIM_CLOCKDIVISION_DIV1;
	TIM_InitStructure.CounterMode=LL_TIM_COUNTERMODE_UP;			// aufwärts zählen
	TIM_InitStructure.RepetitionCounter=0;										// nicht relevant bei TIM3
	LL_TIM_Init(TIM3,&TIM_InitStructure);
	
	// Kanal 1 (Servo)
	LL_TIM_OC_StructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.CompareValue=200;											// => 1 ms Pulsbreite
	TIM_OCInitStructure.OCMode=LL_TIM_OCMODE_PWM1;						// PWM Modus 1
	TIM_OCInitStructure.OCPolarity=LL_TIM_OCPOLARITY_HIGH;
	LL_TIM_OC_Init(TIM3,LL_TIM_CHANNEL_CH1,&TIM_OCInitStructure);
	LL_TIM_CC_EnableChannel(TIM3,LL_TIM_CHANNEL_CH1);

	// Kanal 2...4 (RGB-LED)
	LL_TIM_OC_StructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.CompareValue=0;												// LED aus
	TIM_OCInitStructure.OCMode=LL_TIM_OCMODE_PWM1;						// PWM Modus 1
	TIM_OCInitStructure.OCPolarity=LL_TIM_OCPOLARITY_LOW;
	LL_TIM_OC_Init(TIM3,LL_TIM_CHANNEL_CH2,&TIM_OCInitStructure);
	LL_TIM_OC_Init(TIM3,LL_TIM_CHANNEL_CH3,&TIM_OCInitStructure);
	LL_TIM_OC_Init(TIM3,LL_TIM_CHANNEL_CH4,&TIM_OCInitStructure);
	LL_TIM_CC_EnableChannel(TIM3,LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(TIM3,LL_TIM_CHANNEL_CH3);
	LL_TIM_CC_EnableChannel(TIM3,LL_TIM_CHANNEL_CH4);

	// Timer starten
	LL_TIM_EnableCounter(TIM3);
}

void TIM3_servo( int pos)
// Servo auf Position fahren (0...100)
{
  if (pos <0 || pos >100)										// Abbruch, weil ausserhalb des Wertebereichs
		return;
	
	LL_TIM_OC_SetCompareCH1(TIM3,100 + (100-pos));
}


void TIM3_RGB(int red, int green, int blue)
// RGB-LED-Helligkeit setzen (0...255)
{
	if ((red >=0) && (red <=255))
		LL_TIM_OC_SetCompareCH2(TIM3,2000 * red / 255);	
	if ((green >=0) && (green <=255))
		LL_TIM_OC_SetCompareCH3(TIM3,2000 * green / 255);	
	if ((blue >=0) && (blue <=255))
		LL_TIM_OC_SetCompareCH4(TIM3,2000 * blue / 255);	
}
