/*
 * GccApplication1.c
 *
 * Created: 14/09/2024 10:33:38
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
#define b5 (1 << PIND5)
#define b6 (1 << PIND6)
#define b7 (1 << PIND7)


typedef enum{
	livelli,
	cottura,
	reset
	}stato;
stato currentstate = livelli;


volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int tick = 0;
volatile int secondi = 0;
volatile int presstimer = 0;

volatile int level = 0;
void livellicott();
void incredecre();
void lampeggio();



int cycle = 0;
int period = 0;



int main(void)
{
    DDRC |= (l0|l1|l2|l3|l4|l5);
	PORTC &=~ (l0|l1|l2|l3|l4|l5);
	
	DDRD &=~(b2|b3|b4|b5|b6|b7);
	PORTD |= (b2|b3|b4|b5|b6|b7);
	
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (b2|b3|b4|b5|b6|b7);
	
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A =  157;
	
	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A =  15626;
	
	TCCR2A |= (1 << WGM21);
	TIMSK2 |= (1 << OCIE2A);
	OCR2A =  157;
	
	
	
	TCCR2B |= ((1<<CS20)|(1<<CS21)|(1<<CS22));
	sei();
    while (1) 
    {
		switch(currentstate){
			
			case livelli:
			TCCR0B |= ((1<<CS00)|(1<<CS02));
			TCCR1B |= ((1<<CS10)|(1<<CS12));
			livellicott();
			incredecre();
			
			if (pulsante == b7)
			{
				currentstate = cottura;
				pulsante = 0;
			}break;
			
			
			case cottura:
			lampeggio();
			
			if ((secondi == 16) || (pulsante == b4))
			{
				currentstate = reset;
				pulsante = 0;
			}break;
			
			
			
			case reset:
			TCCR0B &=~ ((1<<CS00)|(1<<CS02));
			TCCR1B &=~ ((1<<CS10)|(1<<CS12));
			PORTC &=~ (l0|l1|l2|l3|l4|l5);
			tick = 0;
			secondi = 0;
			presstimer = 0;
			level = 0;
			pulsante = 0;
			period = 0;
			cycle = 0;
			currentstate = livelli;
			break;
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	bool hold = false;
	for (uint8_t i = PIND2; i <= PIND7; i++)
	{
		if ((change &(1 << i)) && !(PIND & (1 << i)))
		{
			pulsante = (1 << i);
			hold = true;
		}
	}
	if (!(hold))
	{
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
}



void livellicott(){
	switch(level){
		default:
		PORTC &=~ (l2|l1|l0);
		break;
		
		case 1:
		PORTC |=l0;
		PORTC &=~ (l2|l1);
		cycle = 10;
		period = 100;
		break;
		
		case 2:
		PORTC |=l1;
		PORTC &=~ (l2|l0);
		cycle = 20;
		period = 100;
		break;
		
		case 3:
		PORTC |=(l0|l1);
		PORTC &=~ l2;
		cycle = 40;
		period = 100;
		break;
		
		case 4:
		PORTC |=l2;
		PORTC &=~ (l0|l1);
		cycle = 25;
		period = 50;
		break;
		
		
		case 5:
		PORTC |=(l0|l2);
		PORTC &=~ l1;
		cycle = 35;
		period = 50;
		break;
		
		case 6:
		PORTC &=~l0;
		PORTC |= (l2|l1);
		cycle = 45;
		period = 50;
		break;
		
		case 7:
		PORTC |=(l2|l1|l0);
		break;
	
	
	
	}
}



void incredecre(){
	switch(pulsante){
		
		case b2:
		if (presstimer == 100)
		{
			presstimer = 0;
			if (level < 7)
			{
				level++;
			}
		}
		break;
		
		
		case b3:
		if (presstimer == 100)
		{
			presstimer = 0;
			if (level > 0)
			{
				level--;
			}
		}
		break;
		
		
	}
	
}


void lampeggio(){
	if (level == 0)
	{
		PORTC |= l4;
		PORTC &=~ l5;
	}
	else if (level == 7)
	{
		PORTC |= l5;
		PORTC &=~ l4;
	}
	
	else{
		 if (tick <= cycle)
	{
		PORTC |= l4;
		PORTC &=~ l5;
	}else{
		PORTC |= l5;
		PORTC &=~ l4;
	}
	if (tick == period)
	{
		tick = 0;
	}
		
	}
}