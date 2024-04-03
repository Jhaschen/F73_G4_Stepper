/*
 * ATMega32_utility_bib.h
 *
 * Created: 24.08.2020
 *  Author: J-H. Aschen
 */ 


#ifndef ATMEGA32_UTILITY_BIB_H
#define ATMEGA32_UTILITY_BIB_H


// Bibliotheken includierden
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>




// Makro definitionen
#define CLR_LED(REG,BIT)  REG |= (1<<BIT)  // LED ausschalten
#define SET_LED(REG,BIT)  REG &= ~(1<<BIT)  // LED einschalten

#define SET_BIT(REG,BIT)     REG  |=  (1<<BIT)        /* Setzen        des Bits BIT in Register REG */
#define CLR_BIT(REG,BIT)     REG  &= ~(1<<BIT)        /* Löschen       des Bits BIT in Register REG */
#define TGL_BIT(REG,BIT)     REG  ^=  (1<<BIT)        /* Komplemtieren des Bits BIT in Register REG */
#define BIT_IS_SET(REG,BIT)  ((REG & (1<<BIT)) != 0)  /* Testen, ob das Bits BIT in Register REG gesetzt  ('1') ist */
#define BIT_IS_CLR(REG,BIT)  ((REG & (1<<BIT)) == 0)  /* Testen, ob das Bits BIT in Register REG gelöscht ('0') ist */

//USART Parität
#define USART_EVEN_PARITY 1
#define USART_ODD_PARITY 2

#define LED_DDR DDRC
#define LED_PORT PORTC

// ADC-INIT und Ansteuerung
class ADC_read{

public:
ADC_read(uint8_t _kanal);
uint16_t adcwert(void);
private:
uint8_t kanal;

};
// Button einlesen

class Button{
public:
Button();
uint8_t Button_read(void);

};

class Timer{
public:
Timer();
// Timer0 init Overflow und Compareregister
void Timer_0_Overflow_ISR_init(void);
void Timer_0_Compare_ISR_init(void);
};


// UART 
class USART{
public:
USART(uint8_t _CharBits, uint8_t _ParBit, uint8_t _StopBits, uint32_t _Baudrate);
void UsartInit(void);
void UsartPutc(uint8_t Data);
void UsartPuts(char* pString);
uint8_t UsartGetc(void);
private:
int8_t CharBits ;
uint8_t ParBit; 
uint8_t StopBits; 
uint32_t Baudrate;
};
#endif /* ATMEGA32_UTILITY_BIB_H*/

