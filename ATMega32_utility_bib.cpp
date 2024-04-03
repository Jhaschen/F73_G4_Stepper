/*
 * ATMega32_utility_bib.cpp
 *
 * Created: 24.08.2020
 *  Author: J-H. Aschen
 */ 


#include "ATMega32_utility_bib.h"


ADC_read::ADC_read(uint8_t _kanal):kanal(_kanal){}

uint16_t ADC_read::adcwert(void)
{
	uint16_t adcwert=0;
	// REFS1:0 = 00 => AREF externe Referenzspannung (=5V beim RNCTRL1.4)
  	// ADLAR   =  1 => Wandlungsergebnis ist linksausgerichtet
  	//                 ADCH: ADC9...ADC2
  	//                 ADCL: ADC1...ADC0
	ADMUX  = (0<<REFS1) | (0<<REFS0) | (0<<ADLAR);
	uint8_t ADChan = kanal;
	ADMUX= (ADMUX & 0b11100000) | (ADChan & 0b00011111);
	
	// ADEN = 1 => AD-Wandler freigeben
	// ADSC = 1 => AD-Wandlung starten
	// ADATE = 1 => Auto-Trigger freigeben
	// ADPS2:0 = 111 => Taktvorteiler festlegen: 128
	//                  Muss so eingestellt werden, dass der AD-Wandlertakt
	//                  50..200kHz beträgt.
	//                  16MHz uC-Takt : 128 = 125kHz  
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<< ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	
	// AD-Wandlung starten
	
	ADCSRA |= (1<<ADSC);
	while(BIT_IS_CLR(ADCSRA,ADIF))
	{
	}
	
	adcwert=ADCW;
	
	return adcwert;
}
Button::Button(){}
uint8_t Button::Button_read(void)
{
uint8_t taste=0;
CLR_BIT(DDRA,7); // PIN A7.
SET_BIT(PORTA,7); // Pull-Up Widerstand PortA7 einschalten
ADC_read pin(7);
uint16_t analog7 = pin.adcwert(); // ADC in Pin 7
  
       
	if((analog7>=402) && (analog7<=404)) {taste = 1; }
	else if((analog7>=335) && (analog7<=337)) {taste = 2;}
	else if((analog7>=262) && (analog7<=264)) {taste = 3;}
	else if((analog7>=185) && (analog7<=1187)) {taste = 4;}
	else if((analog7>=103) && (analog7<=105)) {taste = 5;}
	else     {taste=0xff;}
	
	return taste;
}

// Timer0
Timer::Timer(){}

// Init Timer0 Overflow ISR
// Vorteiler = 64 ; 16 MHZ / 64 =250000 => 4 uS pro Tick
void Timer::Timer_0_Overflow_ISR_init()
{


// Timer0 initialisieren
  	// Zählerstandsregister zurücksetzen
  	TCNT0 = 0;// Startwert
  	// Vergleichsregister zurücksetzen
  	OCR0  = 0; // => Zähler zählt bis 255
  	
  	// Konfigurationsregister:
  	// WGM01:0 = Normaler Betrieb
  	// COM01:0 = Normaler Betrieb
  	// CS02:0  = Vorteiler 64
	TCCR0 = (0<<WGM01) | (0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<CS02) | (1<<CS01) | (1<<CS00);

	
	//Interruptmaskenregister setzen
	//TOIE0 = INT auslösen bei Überlauf Timer0 aktiv
	TIMSK=(1<<TOIE0);
	
	//Interrupts global freigeben
	sei();
	
	// Interrupts nicht mehr freigeben
	// cli()
return;
}
/*
// ISR Aufruf im Hauptprogramm
// Overflow Interrupt
//------------------------------------------------------------------------------
//  Interrupt Service Routinen
//------------------------------------------------------------------------------
// Interrupt-Service-Routine für den Interrupt bei Überlauf des Timer0
// ISR: Schlüsselwort für Compiler, dass dies eine ISR ist
// TIMER0_OVF_vect: Information an den Compiler, mit welchem Interrupt
//                  diese ISR verknüpft werden soll. Der Bezeichner "TIMER0_OVF_vect"
//                  ist wie alle anderen ISR-Bezeichner in "avr/interrupt.h" definiert.
ISR(TIMER0_OVF_vect)
{
 
 // tue etwas beim Überlauf von TCNT0
 
 }

*/

// Init Timer0 Compare ISR
// Vorteiler = 64 , OCR0 = 250 => 1ms pro Überlauf
void Timer::Timer_0_Compare_ISR_init()
{
// Timer0 initialisieren
  	// Zählerstandsregister zurücksetzen
  	TCNT0 = 0;// Startwert
  	// Vergleichsregister zurücksetzen
  	OCR0  = 250; // => Zähler zählt bis 250 => 1ms 
  	
  	// Konfigurationsregister:
  	// WGM01:0 = Normaler Betrieb
  	// COM01:0 = Normaler Betrieb
  	// CS02:0  = Vorteiler 64
	TCCR0 = (0<<WGM01) | (0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<CS02) | (1<<CS01) | (1<<CS00);

	
	//Interruptmaskenregister setzen
	//COIE0 = INT auslösen bei Überlauf Timer0 aktiv
	TIMSK=(1<<OCIE0);
	
	//Interrupts global freigeben
	sei();
	
	// Interrupts nicht mehr freigeben
	// cli()
	
	return;
}

/*
// Compare " Vergleichsregister" Interrupt
//------------------------------------------------------------------------------
//  Interrupt Service Routinen
//------------------------------------------------------------------------------
// Interrupt-Service-Routine für den Interrupt bei Überlauf des Timer0
// ISR: Schlüsselwort für Compiler, dass dies eine ISR ist
// TIMER0_COMP_vect: Information an den Compiler, mit welchem Interrupt
//                  diese ISR verknüpft werden soll. Der Bezeichner "TIMER0_COM_vect"
//                  ist wie alle anderen ISR-Bezeichner in "avr/interrupt.h" definiert.
ISR(TIMER0_COMP_vect)
{
// tue etwas beim Überlauf von OCR0
 
 }
*/


// USART
USART::USART(uint8_t _CharBits, uint8_t _ParBit, uint8_t _StopBits, uint32_t _Baudrate) : CharBits(_CharBits), ParBit(_ParBit),StopBits(_StopBits), Baudrate(_Baudrate)
{
// Vorhandensein und Art des Paritäts-Bits festlegen:
	// Gerade   Parität: Anzahl der '1' wird auf gerade Anzahl ergänzt
	// Ungerade Parität: Anzahl der '1' wird auf ungerade Anzahl ergänzt
	// Keine    Parität: Paritäts-Bit entfällt
  if (ParBit == USART_EVEN_PARITY)
    UCSRC |= (1 << UPM1) | (0 << UPM0);  // äquivalent: UCSRC |= (1 << UPM1);
	else
  if (ParBit == USART_ODD_PARITY)
    UCSRC |= (1 << UPM1) | (1 << UPM0);
	else
    UCSRC |= (0 << UPM1) | (0 << UPM0);  // brauchen wir eigentlich nicht...

  // Anzahl der Stop-Bits festlegen: 1 oder 2
  if (StopBits == 1)
    UCSRC |= (0 << USBS);
  else
  if (StopBits == 2)
    UCSRC |= (1 << USBS);

  // Anzahl der Zeichenbits: 5..9
  // Üblich sind 8 Bits / Zeichen.
  // Bei 9 Zeichenbits muss man die besondere Behandlung des 9.Bits beim
  // Lesen und Schreiben beachten. Diese ist in unseren Lese- und Schreibroutinen
  // nicht berücksichtigt.
  switch (CharBits)
  {
    case 5: // 5 Zeichenbits
      break;

    case 6: // 6 Zeichenbits
      UCSRC |= (1 << UCSZ0);
      break;

    case 7: // 7 Zeichenbits
      UCSRC |= (1 << UCSZ1);
      break;

    case 8: // 8 Zeichenbits
      UCSRC |= (1 << UCSZ1) | (1 << UCSZ0);
      break;

    case 9: // 9 Zeichenbits
      UCSRB |= (1 << UCSZ2);
      UCSRC |= (1 << UCSZ1) | (1 << UCSZ0);
      break;
  }

  // Einstellen der Bitrate:
  // Bei der Wahl der Bitrate muss der relative Bitratenfehler
  // aufgrund der Taktfrequenz des uC beachtet werden.
  // Dieser wirkt sich bei höheren Bitraten stärker aus, daher
  // darf bei großem Bitratenfehler die Bitrate nicht zu groß
  // gewählt werden.
  UBRRL = (F_CPU/(16*Baudrate)-1) % 256;
  UBRRH = (F_CPU/(16*Baudrate)-1) / 256;

  // Freigabe der Sende-/Empfangs-Kanäle und uC-Pins
  UCSRB |= (1 << RXEN) | (1 << TXEN);





}
void USART::UsartInit()
{

/*
	// Vorhandensein und Art des Paritäts-Bits festlegen:
	// Gerade   Parität: Anzahl der '1' wird auf gerade Anzahl ergänzt
	// Ungerade Parität: Anzahl der '1' wird auf ungerade Anzahl ergänzt
	// Keine    Parität: Paritäts-Bit entfällt
  if (ParBit == USART_EVEN_PARITY)
    UCSRC |= (1 << UPM1) | (0 << UPM0);  // äquivalent: UCSRC |= (1 << UPM1);
	else
  if (ParBit == USART_ODD_PARITY)
    UCSRC |= (1 << UPM1) | (1 << UPM0);
	else
    UCSRC |= (0 << UPM1) | (0 << UPM0);  // brauchen wir eigentlich nicht...

  // Anzahl der Stop-Bits festlegen: 1 oder 2
  if (StopBits == 1)
    UCSRC |= (0 << USBS);
  else
  if (StopBits == 2)
    UCSRC |= (1 << USBS);

  // Anzahl der Zeichenbits: 5..9
  // Üblich sind 8 Bits / Zeichen.
  // Bei 9 Zeichenbits muss man die besondere Behandlung des 9.Bits beim
  // Lesen und Schreiben beachten. Diese ist in unseren Lese- und Schreibroutinen
  // nicht berücksichtigt.
  switch (CharBits)
  {
    case 5: // 5 Zeichenbits
      break;

    case 6: // 6 Zeichenbits
      UCSRC |= (1 << UCSZ0);
      break;

    case 7: // 7 Zeichenbits
      UCSRC |= (1 << UCSZ1);
      break;

    case 8: // 8 Zeichenbits
      UCSRC |= (1 << UCSZ1) | (1 << UCSZ0);
      break;

    case 9: // 9 Zeichenbits
      UCSRB |= (1 << UCSZ2);
      UCSRC |= (1 << UCSZ1) | (1 << UCSZ0);
      break;
  }

  // Einstellen der Bitrate:
  // Bei der Wahl der Bitrate muss der relative Bitratenfehler
  // aufgrund der Taktfrequenz des uC beachtet werden.
  // Dieser wirkt sich bei höheren Bitraten stärker aus, daher
  // darf bei großem Bitratenfehler die Bitrate nicht zu groß
  // gewählt werden.
  UBRRL = (F_CPU/(16*Baudrate)-1) % 256;
  UBRRH = (F_CPU/(16*Baudrate)-1) / 256;

  // Freigabe der Sende-/Empfangs-Kanäle und uC-Pins
  UCSRB |= (1 << RXEN) | (1 << TXEN);
  */
}



/*
 *  1 Zeichen auf USART-Kanal ausgeben
 */
void USART::UsartPutc(uint8_t Data)
{
  while (!(UCSRA & (1 << UDRE)));
  UDR = Data;
}


/*
 *  String auf USART-Kanal ausgeben
 */
void USART::UsartPuts(char* pString)
{
	char* pData = pString;
	
  while (*pData != 0)
  {
    UsartPutc(*pData);
	  pData++;
  }	
}

/*
 *  1 Zeichen von USART-Kanal einlesen
 */
uint8_t USART::UsartGetc(void)
{
	uint8_t Data;
	
  while (!(UCSRA & (1 << RXC)));
  Data = UDR;
  
  return(Data);
}


