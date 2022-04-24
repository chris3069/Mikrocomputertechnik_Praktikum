void initUSART2(void);
void Init_Systick(void);
void writeChar(char c);
void writeString(const char *str);
char readChar(void);


extern char inputBuffer[];
extern int cmdflag;
