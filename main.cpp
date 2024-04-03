#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include "ATMega32_utility_bib.h"
//#include "rfid.h"
//#include "can.h"
#include "lcd.h"



/*
       Schritt   A   B   C   D
       1 	    H 	H 	L 	L
       2 	    L 	H 	H 	L
       3 	    L 	L 	H 	H
       4 	    H 	L 	L 	H
A = Blue (Spule 1)
B = Pink (Spule 2)
C = Yellow (Spule 1)
D = Orange (Spule 2)
E = Midpoint

PC6 --> IN1
PC7 --> IN2
PB0 --> IN3
PB1 --> IN4
PD4 --> EN1
PD5 --> EN2

*/
//------------------------------------------------------------------------------
//  Private Variablen
//------------------------------------------------------------------------------
uint8_t InputStatus = 2; // 0--> Wischvorgang gestartet , 1--> Wischvorgang wird beendet, 2 -->Wischvorgang beendet
uint16_t OvCnt = 0;
uint16_t StepPhase=0;
uint16_t StepCnt = 0;
char stufe=2;

//Schrittmotor:
uint8_t Stepper[4] [4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 1},
    {1, 0, 0, 1}};

uint16_t StepTime = 6; 

USART UART(8,0,1,9600);	// USART init 8 Zeichenbits , keien Paritätsbits , 1 Stoppbit, 9600 Zeichen pro Sekunde

//------------------------------------------------------------------------------
//  Private Funktionen
//------------------------------------------------------------------------------
void setBitToValue(volatile uint8_t* reg, uint8_t a, bool value){
if (value)
    SET_BIT(*reg, a);
else
    CLR_BIT(*reg, a);
}

void stepperInit(){
    SET_BIT(DDRC, 6);
    SET_BIT(DDRC, 7);
    SET_BIT(DDRB, 0);
    SET_BIT(DDRB, 1);
    SET_BIT(DDRD, 4);
    SET_BIT(DDRD, 5);

    SET_BIT(PORTD, 4);
    SET_BIT(PORTD, 5);
}

//------------------------------------------------------------------------------
//  Interrupt Service Routinen
//------------------------------------------------------------------------------
// Interrupt-Service-Routine für den Interrupt bei Vergleich des Timer0
// ISR: Schlüsselwort für Compiler, dass dies eine ISR ist
// TIMER0_COMP_vect: Information an den Compiler, mit welchem Interrupt
//                   diese ISR verknüpft werden soll. Der Bezeichner "TIMER0_COMP_vect"
//                   ist wie alle anderen ISR-Bezeichner in "avr/interrupt.h" definiert.
ISR(TIMER0_COMP_vect)
{
    OvCnt++;
    TCNT0 = 0;

   // StepTime erreicht und Wischvorgang aktiv ?
    if(OvCnt >= StepTime && InputStatus != 2)
    {
        OvCnt = 0;

    //Bewegung des Motors 
	  setBitToValue(&PORTC, 6, Stepper[StepPhase][0]);
    setBitToValue(&PORTC, 7, Stepper[StepPhase][1]);
    setBitToValue(&PORTB, 0, Stepper[StepPhase][2]);
    setBitToValue(&PORTB, 1, Stepper[StepPhase][3]);
        

    //Hinweg 20 Schritte, 0 bis 19 => 150°/360° = 20 Schritte / 48 Schritte // Getriebe
          if(StepCnt < 800)
          {
            if(StepPhase == 3)
            {
              StepPhase = 0;
            }
            else 
            {
              StepPhase++;
            }
            StepCnt++;
         }

    //Rückweg xx Schritte
         if((StepCnt > 799) && (StepCnt < 1600))
          {
            if(StepPhase == 0)
            {
              StepPhase = 3;
            }
            else 
            {
              StepPhase--;
            }

            StepCnt++;
          }
    // Schritt xx: Wischvorgang beenden ?
          if(StepCnt >= 1600)
          {
            StepPhase = 0;
            
            if(stufe!=3)
            {
              StepCnt = 0;
            }else{
              if(StepCnt==1934){
                StepCnt = 0;
              }else{
                 StepCnt ++;
              }
            }

            //Stopp ?
            if(InputStatus == 0){
              //Wischer aus:
              StepPhase = 0;
              StepCnt = 0;
              UART.UsartPuts("Wischvorgang gestoppt.\n");
              lcd_gotoxy(0,4);  
 	            lcd_puts("Wischvorgang gestoppt.");
              InputStatus = 2;
            }
          }
          
    }

}	

//Interrupt-Service-Routine für die USART-Schnittstelle:
ISR(USART_RXC_vect)
{
  //Eingabe einlesen:
   stufe = UART.UsartGetc() - 48;

  //Eingabe auf 1 bis 3 beschraenken 
  if((stufe >= 1) && (stufe <=3))
  {
    if(stufe==1){
      StepTime = 3;
    }else{
      StepTime = 6;
    }
    
    //Gewählte Stufe ausgeben:
    UART.UsartPutc(stufe + 48);
    UART.UsartPutc('\n');
    lcd_gotoxy(0,6);  
 	  lcd_puts("Stufe: ");
    lcd_putc(stufe + 48);  

  }
  else
  {
    UART.UsartPuts("Ungueltige Eingabe!\n");
  }
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

int main ()
{
	// I2C PC 0 und PC 1 für SDA und SCL belegt, nicht diese LEDs nutzen !
	lcd_init(LCD_DISP_ON);    // init lcd and turn on

	stepperInit(); // Init Schrittmotor an L293D
	
	Button B; // Taster init

	Timer Timer; // Instanz der Klasse Timer
	
 	UCSRB |= (1<<RXCIE); //USART RX Interrupt aktivieren:

	Timer.Timer_0_Compare_ISR_init();// Timer0 init + sei()
	
	
  	//Status Buttons
 	 uint8_t Taster= 0;

  	// Beginnen mit Stepper[0]
  	StepPhase = 0;

	 //char buffer[100];		// Buffer zur Zwschischenspeicherung von Zeichenketten
	
  	lcd_gotoxy(5,0); 	 // set cursor to first column at line 0
  	UART.UsartPuts("Scheibenwischer\n");
    lcd_puts("Scheibenwischer ");  // put string from RAM to display (TEXTMODE) or buffer (GRAPHICMODE)
 	  lcd_gotoxy(0,1); 
    UART.UsartPuts("Taster 1 == Start \n"); 
 	  lcd_puts("Taster 1 == Start ");  // put string from RAM to display (TEXTMODE) or buffer (GRAPHICMODE)
    lcd_gotoxy(0,2);  
    UART.UsartPuts("Taster 2 == Stopp\n");
 	  lcd_puts("Taster 2 == Stopp");   
     

	for (;;)
	{
	  
	
    switch (B.Button_read())
    {
    case 1:
       //Wischvorgang wird gestartet wenn Taster 0 gedrückt:
      //Zählerstand zuruecksetzen
      TCNT0 = 0;

      UART.UsartPuts("Wischvorgang wird gestartet.\n");
      lcd_gotoxy(0,4);  
 	    lcd_puts("Wischvorgang wird gestartet.");
       lcd_gotoxy(0,6);  
 	     lcd_puts("Stufe: ");
      lcd_putc(stufe + 48);  
      InputStatus = 1;
      break;
    case 2:
       //Wischvorgang soll beendet beendet:
      UART.UsartPuts("Wischvorgang wird beendet ...\n");
      lcd_gotoxy(0,4);  
 	    lcd_puts("Wischvorgang wird beendet ...");
      InputStatus = 0;
      break;
    case 3:
    //....
      break;
    case 4:
       //....
      break;
    case 5:
      //....
      break;
    
    default:
      break;
    }
	  

	_delay_ms(100);
		
	}
	return 0;
}

