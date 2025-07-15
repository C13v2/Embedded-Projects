/*
 * GccApplication1.c
 *
 * Created: 13/09/2024 16:05:35
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define l0 (1 << PINC0)
#define l1 (1 << PINC1)
#define l2 (1 << PINC2)
#define l3 (1 << PINC3)
#define l4 (1 << PINC4)
#define l5 (1 << PINC5)

#define b2 (1 << PIND2)
#define b3 (1 << PIND3)
#define b4 (1 << PIND4)
#define b6 (1 << PIND6)
#define b7 (1 << PIND7)


typedef enum{
	avvio,
	scansione,
	caricamento,
	stampa,
	reset
	}stato;
stato currentstate = avvio;

volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int tick = 0;
volatile int secondi = 0;

volatile uint8_t led[] = {l0, l1, l2, l3};
	volatile int ledcorrente = 0;

void incrementodecremento();
void ledaccesi();

int main(void)
{
    DDRC |= (l1|l2|l3|l4|l5);
	PORTC &= ~(l1|l2|l3|l4|l5);
	
	DDRD &= ~(b2|b3|b4|b7|b6);
	PORTD |= (b2|b3|b4|b7|b6);
	
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (b2|b3|b4|b7|b6);
	
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 79;
	
	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 15626;
	sei();
	
    while (1) 
    {
		switch(currentstate){
			case avvio:
			incrementodecremento();
			ledaccesi();
			
			if (pulsante == b7)
			{
				
			}
		
		
		
		
		
		
		
		
		
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	for(uint8_t i = PIND2; i<=PIND7; i++){
		if((change & (1<<i)) && !(PIND & (1<<i))){
			pulsante = (1<<i);		}	}}ISR(TIMER0_COMPA_vect){	tick++;}ISR(TIMER1_COMPA_vect){	secondi++;}void incrementodecremento(){	switch(pulsante){		case b3:		if (ledcorrente < 4)
		{
			ledcorrente++;
		}pulsante = 0;		break;				case b2:		if (ledcorrente > 0)
		{
			ledcorrente--;
		}pulsante = 0;		break;			}}void ledaccesi(){	switch(ledcorrente)
	{
		default : 
		PORTC |= l0;
		break;
		
		case 1: 
		PORTC |= l1;
		break;
		
		case 2: 
		PORTC |= l2;
		break;
		
		case 3:
		PORTC |= l3;
		break;
		
	}	}