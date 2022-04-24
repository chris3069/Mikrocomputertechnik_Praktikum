void InitStepper(void);
void InitSysTick (int load);
void Step_Out(int step_pattern);

extern int stepdir;  		// Richtung des Schrittmotors
extern int akt_pos;				// aktuelle Position des Schrittmotors
extern int ziel_pos;				// Zielposition des Schrittmotors
extern int stepmode;					// Modus des Schrittmotors

#ifndef STEPMODE_NONE
	// Positionierungsmodi
	#define STEPMODE_NONE 0		// Schrittmotor aus
	#define STEPMODE_FWD 1		// Schrittmotor dreht vorwärts
	#define STEPMODE_BWD 2		// Schrittmotor dreht rückwärts
	#define STEPMODE_POS 3		// Schrittmotor im Positionierungsmodus
	#define STEPMODE_REF 4		// Schrittmotor im Positionierungsmodus
#endif
