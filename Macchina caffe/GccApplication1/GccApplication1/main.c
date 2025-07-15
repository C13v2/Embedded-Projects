/*
 * GccApplication1.c
 *
 * Created: 12/09/2024 16:45:59
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
	zucchero,
	caffe,
	anomalia,
	reset
	}stato;
	stato currentstate = zucchero;


typedef enum{
	corto,
	medio,
	lungo
	}tipo;
	tipo currenttype = corto;

volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int secondi = 0;
volatile int tick = 0;
volatile int press = 0;

void lampeggioanomalia();
void lampeggio();

int main(void)
{
    DDRC |= (l0|l1|l2|l3|l4|l5);
	PORTC &=~ (l0|l1|l2|l3|l4|l5);
	
	DDRD &=~ (b2|b3|b4|b5|b6|b7);
	PORTD |= (b2|b3|b4|b5|b6|b7);
	
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (b2|b3|b4|b5|b6|b7);
	
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 79;
	
	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 15626;
	
	TCCR2A |= (1 << WGM21);
	TIMSK2 |= (1 << OCIE2A);
	OCR2A = 156;
	
	sei();
	
	TCCR2B |= ((1<<CS20) | (1<<CS21) | (1<<CS22));
	
    while (1) 
    {
		switch(currentstate)
		{
			case zucchero:
			switch(pulsante){
				
				case b7:
				PORTC |= l5;
				pulsante = 0;
				break;
				
				case b2:
				currentstate = caffe;
				pulsante = 0;
				break;
			
			}break;
			
			
			case caffe:
			TCCR0B |= ((1 << CS00) | (1 << CS02));
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			if (pulsante == b2){
			switch(currenttype){
			
			if (press < 400)
			{
				currenttype = corto;
				void lampeggio();
				if (secondi < 4)
				{
					currentstate = reset;
				}
				
			}
			else if (press < 600)
			{
				currenttype = medio;
				void lampeggio();
				if (secondi == press)
				{
					currentstate = reset;
				}
			}
			else
			{
				currenttype = lungo;
				void lampeggio();
				if (secondi == 6)
				{
					currentstate = reset;
				}
			}
				press = 0;
			}break;
		}
			
			if (pulsante == b4)
			{
				currentstate = anomalia;
				pulsante = 0;
			}break;
			
			
			case anomalia:
				lampeggioanomalia();
				if (secondi >= 4)
				{
					currentstate = reset;
				}
				pulsante = 0;
				break;
				
			
			
			case reset:
			TCCR0B &= ~((1 << CS00) | (1 << CS02));
			TCCR1B &= ~((1 << CS10) | (1 << CS12));
			PORTC &=~ (l0|l1|l2|l3|l4|l5);
			tick = 0;
			secondi = 0;
			press = 0;
			pulsante = 0;
			currentstate = zucchero;
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
		if ((change & (1 << i)) && !(PIND &(1 << i)))
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
	press++;
}


void lampeggioanomalia(){
	int cycle;
	int period;
	
	cycle = 34;
	period = 67;
	if (tick <= cycle)
	{
		PORTC |= (l4|l3);
	}else{
		PORTC &= ~(l4|l3);
	}
	if (tick >= period)
	{
		tick = 0;
	}
}



void lampeggio(){
	int cycle;
	int period;
	
	cycle = 67;
	period = 200;
	
	switch(currenttype){
		case corto:
		case medio:
		case lungo:
	
		if (tick < cycle)
		{
			PORTC |= l2;
			PORTC &=~ (l1|l0);
		}else if (tick <= 2*cycle)
		{
			PORTC |= l1;
			PORTC &=~ (l2|l0);
		}else{
			PORTC |= l0;
			PORTC &=~ (l2|l1);
		}
		if (tick == period)
		{
			tick = 0;
		}break;
		
	}
}