#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_usart.h>
#include <stdio.h>
#include <string.h>
#include "USART2.h"

//---------------------------------------------------------------------------
// Lua -Programm 
// Ersetzungen: " 		--> \"
//              \ 		--> \\
//							\"		--> \\\"
//							\r\n	-->	\\r\\n


//Globale Variablen fuer USART3
char _inputBufferESP[100];
int _cmdflagESP=0;
int _bufferPosESP=0;
int _ESP_fertig=0;

// LUA-Programm für den ESP
static const char lua_txt[] ={"\
w([[]]);\
w([[print(\"> ok\")]]);\
w([[s,r,g,b=50,50,50,50]]);\
w([[if srv then srv:close() srv=nil end]]);\
w([[srv=net.createServer(net.TCP) ]]);\
w([[srv:listen(80,function(conn) ]]);\
w([[    conn:on(\"receive\", function(client,request)]]);\
w([[        local buf = \"HTTP/1.1 200 OK\\r\\nContent-type: text/html\\r\\nConnection: close\\r\\n\\r\\n\";        ]]);\
w([[        --print(request)]]);\
w([[        local _, _, method, path, vars = string.find(request, \"([A-Z]+) (.+)?(.+) HTTP\");]]);\
w([[        if(method == nil)then ]]);\
w([[            _, _, method, path = string.find(request, \"([A-Z]+) (.+) HTTP\"); ]]);\
w([[        end]]);\
w([[        local _GET = {}]]);\
w([[        ]]);\
w([[        -- extract parameters out of query string]]);\
w([[        if (vars ~= nil)then             ]]);\
w([[            for k, v in string.gmatch(vars, \"(%w+)=([%w\\.]+)&*\") do ]]);\
w([[               _GET[k] = v    ]]);\
w([[            end         ]]);\
w([[        end]]);\
w([[        ]]);\
w([[        --react to HTTP-Request (TX)          ]]);\
w([[        if (_GET.cmd ~= nil) then]]);\
w([[            print(_GET.cmd)]]);\
w([[        elseif (_GET.s ~= nil) then]]);\
w([[            print(\"s\".._GET.s..\".\")]]);\
w([[            s=_GET.s]]);\
w([[        elseif ((_GET.r ~= nil) and (_GET.g ~= nil) and (_GET.b ~= nil)) then]]);\
w([[            print(\"r\".._GET.r..\"g\".._GET.g..\"b\".._GET.b..\".\")]]);\
w([[            r,g,b=_GET.r,_GET.g,_GET.b]]);\
w([[        end   ]]);\
w([[        ]]);\
w([[        ]]);\
w([[        -- html source code]]);\
w([[        buf = buf..\"<h1> MCT Steuerung</h1>\";]]);\
w([[        ]]);\
w([[        -- input text]]);\
w([[        buf = buf..\"<form name=\\\"input\\\">\";]]);\
w([[        buf = buf..\"<span style=\\\"width: 30%; display: inline-block;\\\">Kommando: <input type=\\\"text\\\" name=\\\"cmd\\\" /></span>\";]]);\
w([[        buf = buf..\"<input type=\\\"submit\\\" value=\\\"Absenden\\\" />\";]]);\
w([[        buf = buf..\"</form>\";]]);\
w([[        ]]);\
w([[        -- input RGB]]);\
w([[        buf = buf..\"<form oninput=\\\"numerisch.value=auswertung.value\\\">\";]]);\
w([[        buf = buf..\"<span style=\\\"width: 30%; display: inline-block;\\\">\";]]);\
w([[        buf = buf..\"R <input name=\\\"r\\\" type=\\\"number\\\" min=\\\"0\\\" max=\\\"100\\\" step=\\\"1\\\" value=\\\"\"..r..\"\\\" style=\\\"width: 3.5em;\\\"> \";]]);\
w([[        buf = buf..\"G <input name=\\\"g\\\" type=\\\"number\\\" min=\\\"0\\\" max=\\\"100\\\" step=\\\"1\\\" value=\\\"\"..g..\"\\\" style=\\\"width: 3.5em;\\\"> \";]]);\
w([[        buf = buf..\"B <input name=\\\"b\\\" type=\\\"number\\\" min=\\\"0\\\" max=\\\"100\\\" step=\\\"1\\\" value=\\\"\"..b..\"\\\" style=\\\"width: 3.5em;\\\"> \";]]);\
w([[        buf = buf..\"</span>\";]]);\
w([[        buf = buf..\"<input type=\\\"submit\\\" value=\\\"Absenden\\\">\";]]);\
w([[        buf = buf..\"</form>\";]]);\
w([[        ]]);\
w([[        -- input slider]]);\
w([[        buf = buf..\"<form oninput=\\\"x.value=s.value\\\">\";]]);\
w([[        buf = buf..\"<span style=\\\"width: 30%; display: inline-block;\\\">Servo: <input type=\\\"range\\\" name=\\\"s\\\"  min=\\\"0\\\" max=\\\"100\\\" value=\\\"\"..s..\"\\\"><output name=\\\"x\\\">50</output>\";]]);\
w([[        buf = buf..\"</span>\";]]);\
w([[        buf = buf..\"<input type=\\\"submit\\\"value=\\\"Absenden\\\">\";]]);\
w([[        buf = buf..\"</form>\";]]);\
w([[        ]]);\
w([[        client:send(buf);]]);\
w([[        client:close();]]);\
w([[        collectgarbage();]]);\
w([[    end)]]);\
w([[end)]]);\
w([[]]);\
w([[uart.on(\"data\",\"\\n\",function(data)]]);\
w([[    if(string.find(data,\"ping\",1,true))then]]);\
w([[        print(\"pong\")]]);\
w([[    end]]);\
w([[end,0)]]);\
w([[]]);\
"};


// ESP-Befehle
static const char lua_cmd1[] ={"wifi.setmode(wifi.SOFTAP)\r\n"};
static const char lua_cmd2[] ={"wifi.ap.config({ssid=\"ESP-WLAN%i\",pwd=\"%s\"})\r\n"};
static const char lua_cmd3[] ={"=file.open(\"V5_neu.lua\",\"r\")\r\n"};
static const char lua_cmd4[] ={"=file.open(\"V5_neu.lua\",\"w+\")\r\n"};
static const char lua_cmd5[] ={"w = file.writeline\r\n"};
static const char lua_cmd6[] ={"=file.close()\r\n"};
static const char lua_cmd7[] ={"=dofile(\"V5_neu.lua\")\r\n"};
static const char lua_cmd8[] ={"=file.remove(\"V5_neu.lua\")\r\n"};

//===========================================================================
// USART3 zur Kommunikation mit ESP
//===========================================================================
void InitUSART3(void)
// USART3 initialisieren
{
	LL_GPIO_InitTypeDef GPIO_InitStructure;
	LL_USART_InitTypeDef USART_InitStructure;									//Strukturen anlegen:

	// PC10/11 initialisieren
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);			// GPIOC Takt einschalten
	LL_GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.Pin=LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
	GPIO_InitStructure.Mode=LL_GPIO_MODE_ALTERNATE;							
	GPIO_InitStructure.Speed=LL_GPIO_SPEED_FREQ_LOW;				
	GPIO_InitStructure.Alternate=LL_GPIO_AF_7;								// Alternate Function USART
	LL_GPIO_Init(GPIOC,&GPIO_InitStructure);

	// USART3 initialisieren
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);			// USART2 Takt einschalten
	USART_InitStructure.BaudRate=9600;
  USART_InitStructure.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStructure.StopBits = LL_USART_STOPBITS_1;
  USART_InitStructure.Parity = LL_USART_PARITY_NONE ;
  USART_InitStructure.TransferDirection= LL_USART_DIRECTION_TX_RX;
	USART_InitStructure.HardwareFlowControl=LL_USART_HWCONTROL_NONE;
	USART_InitStructure.OverSampling=LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART3,&USART_InitStructure);
	
	LL_USART_Enable(USART3);																//USART2 einschalten:
	LL_USART_EnableIT_RXNE(USART3);													//USART2 RXNE-Interrupt einschalten:
	NVIC_EnableIRQ(USART3_IRQn);														// NVIC aktivieren
}

void WriteCharUSART3(char c)
// schreibt Zeichen c an USART3
{
	while(!LL_USART_IsActiveFlag_TXE(USART3));							//Auf TXE Bit warten:
	LL_USART_TransmitData8(USART3,c);												//Daten ins Data-Register schreiben:
}

void WriteStringUSART3(char *str)
// schreibt STring an USART3
{
	while (*str != 0)
	{
		WriteCharUSART3(*(str++));
	}
}

// Interruptroutine für USART3
// optimiert für ESP
void USART3_IRQHandler (void)
{
	char inputChar;								// Eingelesenes Zeichen
	
	inputChar = LL_USART_ReceiveData8(USART3);
	_inputBufferESP[_bufferPosESP] = inputChar;				// und in Puffer speichern
	WriteChar(inputChar);															// Echo an Terminal
	
	//Sobald der ESP laeuft muss seine Ausgabe in den anderen inputBuffer
	//kopiert werden, damit das restliche Programm auch funktioniert
	if(_ESP_fertig){
		inputBuffer[_bufferPosESP]=inputChar;
	}
	
	_bufferPosESP++;																// Pufferzeiger erhöhen

	if (inputChar == '\n') 													// wenn Zeilenenderückmeldung von ESP
	{
		_inputBufferESP[_bufferPosESP] = 0; 			    // String abschließen	
		_cmdflagESP=1;																// Kommandoflag setzen
		if(_ESP_fertig){
			cmdflag=1;																	// zur Kommunikation mit anderem Programm über USART1-Puffer
			inputBuffer[_bufferPosESP]=0;								// Kommandoflag setzen und String abschließen
		}
		_bufferPosESP=0;								// Pufferzeiger zurücksetzen
	}
}
//===========================================================================

void wait(int ms){
	int i;
	for (i=0;i< 10000 *ms; i++)
	{};
}

/*Der ESP schickt immer ein Echo zurueck, gefolgt von CR+LF
*in der naechsten Zeile schickt er die Zeichen "> ",
*dadurch wird signalisiert, dass die naechste Eingabe
*erfolgen kann. Auf diesen Zustand wird mit diesem
*Programm gewartet.
*Das ganze hat den Nachteil, dass der USART-Interrupt
*die Eingabe bei LF als beendet ansieht und wieder an
*den Anfang des Puffers springt, der ESP schickt dann
*aber noch die Zeichen "> " hinterher und macht damit
*die ersten zwei Zeichen im Puffer platt.*/
void wait_for_esp(void){
	while(1){
		//Pruefen, ob die Zeichen "> " im Puffer stehen
		if(_cmdflagESP) {
			if (_inputBufferESP[0]=='>') {
				if(_inputBufferESP[1]==' '){
					//Flag loeschen
					_cmdflagESP=0;
					//Kurz warten
					wait(100);
					return ;

				}
			}
		}
	}
}

void Lua_to_ESP(char *lua)
// schickt Programm an ESP
{
	char c;
	
	while (*lua != 0)
	{
		c = *lua;
		//nicht mehr noetig, Zeichen schon im Text ersetzt
		//if (c == '§') c = '"';
		//if (c == '$') c = '\\'; // ¬ in Backslash
		WriteCharUSART3(c);
		
		//Saubere Variante, dauert aber ewig
		//while(ReadCharUSART1()!=c);
		
		//_DEUTLICH_ kuerzere Hochladezeit
		wait(2);
		if ((c == ';')
			   && (*(lua-1)==')')
			   && (*(lua-2)==']')
		     && (*(lua-3)==']'))
		{
		  WriteStringUSART3("\r\n");
			//warten auf ESP, ginge auch mit wait(40),
			//ist so aber schoener
			wait_for_esp();
		}
		lua++;		
	}
	
}

void Program_to_ESP( int nr)
// testet, ob ESP schon läuft
// startet ESP
// testet ob Programm vorhanden und startbar
// ansonsten Download des Programms an ESP
{ 
	char cmd[100];
	
	InitUSART3();		// USART3 fuer ESP-Verbindung 

	WriteString("*** Start ESP-Programmierung\n\r");
	wait(100);
	
	//Sekunde warten, bis der ESP gestartet ist
	wait(1000);
	//Flag zurücksetzen
	_cmdflagESP=0;
	
	// Test, ob ESP läuft
	WriteStringUSART3("ping\r\n");
	wait(100);
	if(_inputBufferESP[1]=='o'){
		_ESP_fertig=1;
		WriteString("ESP laeuft schon!\n\r");
		return;
	}
	
	//Fortschritts-ausgabe
	WriteString("*** WLAN konfigurieren...\r\n");
	
	//WLAN-Modus festlegen
	//=wifi.setmode(wifi.STATIONAP)\r\n"
	WriteStringUSART3((char*)lua_cmd1);
	//Warten auf Rueckmeldung
	wait_for_esp();
	
	//WLAN-Namen und Passwort festlegen
	//=wifi.ap.config({ssid=\"ESP-WLAN%i\",pwd=\"%s\"})\r\n
	sprintf(cmd,(char*)lua_cmd2,nr,"1234567890");
	WriteStringUSART3(cmd);
	//Warten auf Rueckmeldung
	wait_for_esp();
	
	WriteString("*** Testen, ob das Programm vorhanden ist...\r\n");
	//Testen, ob Skript vorhanden ist, falls ja wird vom
	//ESP "true" zurueckgegeben, falls nein "false"
	//=file.open(\"V5_neu.lua\",\"r\")\r\n
	WriteStringUSART3((char*)lua_cmd3);
	//Warten auf Rueckmeldung
	wait_for_esp();
	
	//Testen, ob das dritte Zeichen im Puffer kein 'u' ist,
	//siehe Kommentar bei wait_for_ESP(),
	//falls zutreffend -> Skript hochladen
	if(_inputBufferESP[2]!='u'){
		WriteString("*** Programm ist nicht vorhanden, wird hochgeladen...\r\n");
		//Datei neu anlegen
		//=file.open(\"V5_neu.lua\",\"w+\")\r\n
		WriteStringUSART3((char*)lua_cmd4);
		wait_for_esp();
		
		//w = file.writeline\r\n
		WriteStringUSART3((char*)lua_cmd5);
		wait_for_esp();
		
		Lua_to_ESP((char*)lua_txt);
		//Warten schon integriert
	}
	else{
		WriteString("*** Programm ist vorhanden\r\n");
	}
	//Datei wieder schliessen
	//=file.close()\r\n
	WriteStringUSART3((char*)lua_cmd6);
	wait_for_esp();
	
	WriteString("*** Programm wird gestartet\r\n");
	//Skript ausfuehren
	//=dofile(\"V5_neu.lua\")\r\n
	WriteStringUSART3((char*)lua_cmd7);
	wait_for_esp();
	
	wait(10);
	
	WriteString("\n\r*** Test, ob Programmstart erfolgreich war\r\n");
	if(!(_inputBufferESP[2]=='o'&&_inputBufferESP[3]=='k')){
		WriteString("*** Nicht erfolgreich, wird neu hochgeladen...\r\n");
		//Alte Datei loeschen
		//=file.remove(\"V5_neu.lua\")\r\n
		WriteStringUSART3((char*)lua_cmd8);
		wait_for_esp();
		
		//Datei neu anlegen
		//=file.open(\"V5_neu.lua\",\"w+\")\r\n
		WriteStringUSART3((char*)lua_cmd4);
		wait_for_esp();
		
		//w = file.writeline\r\n
		WriteStringUSART3((char*)lua_cmd5);
		wait_for_esp();
		
		Lua_to_ESP((char*)lua_txt);
		//Warten schon integriert
		
		WriteString("*** Programmstart\r\n");
		//Skript ausfuehren
		//=dofile(\"V5_neu.lua\")\r\n
		WriteStringUSART3((char*)lua_cmd7);
		wait_for_esp();
	}
	else{
		WriteString("*** Erfolgreich!\r\n");
	}
	
	_bufferPosESP=0;		// Puffer zurücksetzen, um das Kommandoprompt "> " aus Puffer zu entfernen
	
	_ESP_fertig=1;
	WriteString("*** Ende ESP-Programmierung\n\r");
}
