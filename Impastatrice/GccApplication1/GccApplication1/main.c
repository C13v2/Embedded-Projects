/*
 * GccApplication1.c
 *
 * Created: 12/09/2024 18:21:33
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define l1 (1 << PINC1)
#define l2 (1 << PINC2)
#define l3 (1 << PINC3)
#define l5 (1 << PINC5)

#define b2 (1 << PIND2)
#define b4 (1 << PIND4)
#define b5 (1 << PIND5)
#define b7 (1 << PIND7)

typedef enum{
	riposo,
	avvio,
	reset
	}stato;
	stato currentstate = riposo;
	
volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int tick = 0;
volatile int secondi = 0;

volatile int velocita[]= {900, 750, 600, 300};
	volatile int velocitacorrente = 0;

volatile bool direzione = false;		

void inverti();	
void incrementodecremento();
void lampeggio();

int main(void)
{
    DDRD &= ~(b2|b7|b4|b5);
	PORTD |= (b2|b7|b4|b5);
	
	DDRC |= (l1|l2|l3|l5);
	PORTC &= ~(l1|l2|l3|l5);
	
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (b2|b7|b4|b5);
	
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 79;
	
	TCCR1A |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 15626;
	
	sei();
    while (1) 
    {
		switch(currentstate){
			case riposo:
			PORTC |= l5;
			if ((pulsante == b4) || (pulsante == b5))
			{
				currentstate = avvio;
			}pulsante = 0;
			break;
			
			case avvio:	
			TCCR0B |= ((1 << CS00) | (1 << CS02));
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			
			lampeggio();
			
			if ((pulsante == b4) || (pulsante == b5)){
				incrementodecremento();
			}
			
			if (pulsante == b7)
			{
				if (secondi	>= 1)
				{
					inverti();
			}
				secondi = 0;
			}
				pulsante = 0;
			
			
			
			if (pulsante == b2)
			{
				currentstate = reset;
			}
			break;
			
			
			case reset:
			TCCR0B &= ~((1 << CS00) | (1 << CS02));
			TCCR1B &= ~((1 << CS10) | (1 << CS12));
			PORTC &= ~(l1|l2|l3|l5);
			tick = 0;
			secondi = 0;
			pulsante = 0;
			currentstate = riposo;
			break;
			
		}	
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	for(uint8_t i = PIND2; i <= PIND7; i++){
		if ((change & (1 << i)) && !(PIND & (1 << i)))
		{
			pulsante = (1 << i);
		}
	}
}

ISR(TIMER0_COMPA_vect){
	tick++;
}

ISR(TIMER1_COMPA_vect){
	secondi++;
}


void inverti(){
	direzione = !direzione;
}


void incrementodecremento(){
	switch(pulsante){
		case b4:
		if (secondi	>= 1)
		{
			if (velocitacorrente < 3)
			{
				velocitacorrente++;
				
			}
			secondi  = 0;
			pulsante = 0;
		}break;
		
		
		case b5:
		if (secondi	>= 1)
		{
			if (velocitacorrente > 0)
			{
				velocitacorrente--;
				
			}
			secondi  = 0;
			pulsante = 0;
		}break;
		
	}
	
}


void lampeggio(){
	int period = velocita[velocitacorrente];
	int cycle = period/3;
	if (direzione)
	{
	
	if(tick <= cycle){
		PORTC |= l1;
		PORTC &= ~(l2|l3);
	}else if (tick <= 2*cycle)
	{
		PORTC &= ~(l1|l3);
		PORTC |= l2;
	}else{
		PORTC &= ~(l2|l1);
		PORTC |= l3;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
	}else{
		
		if(tick <= cycle){
			PORTC |= l3;
			PORTC &= ~(l2|l1);	
		}else if (tick <= 2*cycle)
		{
			PORTC &= ~(l1|l3);
			PORTC |= l2;
			}else{
			PORTC &= ~(l2|l3);
			PORTC |= l1;
		}
		if (tick == period)
		{
			tick = 0;
		}
	}
}