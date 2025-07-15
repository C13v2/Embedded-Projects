/*
 * GccApplication1.c
 *
 * Created: 14/09/2024 17:31:34
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define L0 (1<<PINC0)
#define L1 (1<<PINC1)
#define L2 (1<<PINC2)
#define L3 (1<<PINC3)
#define L4 (1<<PINC4)
#define L5 (1<<PINC5)

#define B2 (1<<PIND2)
#define B4 (1<<PIND4)
#define B5 (1<<PIND5)
#define B6 (1<<PIND6)
#define B7 (1<<PIND7)

#define leds (L0 | L1 | L2 | L3 | L4 | L5)
#define buttons (B2 | B4 | B5 | B6 | B7)


typedef enum{
	accensione,
	avvio,
	freeze,
	allarme,
	reset
	}stato;
	stato currentstate = accensione;

volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;
volatile int tick = 0;
volatile int secondi = 0;
volatile int presstimer = 0;
volatile int period = 600;
volatile int alarmperiod = 0;

void cambiovelocita();
void lampeggio();
void lampeggioallarme();

int main(void)
{
    DDRC |= leds;
    PORTC &=~ leds;
	
    DDRD &=~ buttons;
    PORTD |= buttons;
	
    PCICR |= (1<<PCIE2);
    PCMSK2 |= buttons;
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 79;

	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 15626;

	TCCR2A |= (1 << WGM21);
	TIMSK2 |= (1 << OCIE2A);
	OCR2A = 157;

	sei();
	
	TCCR2B |= ((1 << CS20) | (1 << CS21) | (1 << CS22));
	
    while (1) 
    {
		switch(currentstate){
			
			case accensione:
			PORTC |= L0;
			if (pulsante == B5)
			{
				cambiovelocita();
			}pulsante = 0;
			
			if (pulsante == B7)
			{
				currentstate = avvio;
			}pulsante = 0;
			break;
			
			case avvio:
			TCCR0B |= ((1 << CS00) | (1 << CS02));
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			
			lampeggio();
			
			if (secondi == 10)
			{
				currentstate = reset;
			}
			
			switch(pulsante){
			
			case B4:
			currentstate = freeze;
			pulsante = 0;
			break;
			
			
			case B2:
				if (presstimer = 100)
				{
					currentstate = reset;
				}
			break;
			
	
			case B6:
			currentstate = allarme;
			pulsante = 0;
			break;
		}break;
			
			case freeze:
			PORTC |= L0;
			TCCR0B &=~ ((1<<CS00) | (1<<CS02));
			TCCR1B &=~ ((1<<CS10) | (1<<CS12));
			
			if (pulsante == B7)
			{
				currentstate = avvio;
			}pulsante = 0;
			break;
			
			
			
			
			case allarme: 
			lampeggioallarme();
			
			if (pulsante == B2)
			{
				if ( presstimer >= 400)
				{
					currentstate = reset;
				}
			}
			
			
			
			case reset:
			TCCR0B &= ~((1 << CS00) | (1 << CS02));
			TCCR1B &=~ ((1 << CS10) | (1 << CS12));
			PORTC &=~ leds;
			tick = 0;
			secondi = 0;
			presstimer = 0;
			pulsante = 0;
			period = 0;
			currentstate = accensione;
			break;
			
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	bool hold = false;
	for(uint8_t i = PIND2; i<=PIND7; i++){
		if((change & (1<<i)) && !(PIND & (1<<i))){
			pulsante = (1<<i);
			hold = true;
		}
	}
	if(!(hold)){
		pulsante = 0;
	}
}

ISR(TIMER0_COMPA_vect){
	tick++;
}

ISR(TIMER1_COMPA_vect){
	secondi++;
}

ISR(TIMER2_COMPA_vect){
	presstimer++;
}void cambiovelocita(){	switch(period){		default:		period = 600;		break;				case 1:		period = 400;		break;				 case 2:		 period = 200;		 break;		 		 case 3:		 period = 120;		 break;			}}void lampeggio(){		int cycle = (period)*(0.125);	int icycle = (int) cycle;			if (tick <= icycle)
	{
		PORTC |= L1;
		PORTC &=~ L2;
	}	else if (tick <= 2*icycle)
	{
		PORTC |= L2;
		PORTC &= ~L1;
	}	else if (tick <= 3*icycle)
	{
		PORTC |= L3;
		PORTC &= ~L2;
	}	else if (tick <= 4*icycle)
	{
		PORTC |= L4;
		PORTC &= ~L3;
	}	else if (tick <= 5*icycle)
	{
		PORTC |= L5;
		PORTC &= ~L4;
	}	else if (tick <= 6*icycle)
	{
		PORTC |= L4;
		PORTC &= ~L5;
	}	else if (tick <= 7*icycle)
	{
		PORTC |= L3;
		PORTC &= ~L4;
	}	else if (tick <= 8*icycle)
	{
		PORTC |= L2;
		PORTC &= ~L3;
	}		if (tick == period)
	{
		tick = 0;
	}	}void lampeggioallarme(){	int cycle;		alarmperiod = 200;	cycle = 50;		if ( tick <= cycle)
	{
		PORTC |= L0;
	}else  {		PORTC &=~ L0;	}	if (tick == alarmperiod)
	{
		tick = 0;
	}		}