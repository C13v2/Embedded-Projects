/*
 * GccApplication1.c
 *
 * Created: 15/09/2024 17:47:59
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define l0 (1<<PINC0)
#define l1 (1<<PINC1)
#define l2 (1<<PINC2)
#define l3 (1<<PINC3)
#define l4 (1<<PINC4)
#define l5 (1<<PINC5)

#define b2 (1<<PIND2)
#define b3 (1<<PIND3)
#define b4 (1<<PIND4)
#define b5 (1<<PIND5)
#define b6 (1<<PIND6)
#define b7 (1<<PIND7)

typedef enum{
	accensione,
	avvio,
	
	reset
	}stato;
	stato currentstate = accensione;
	
volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int velocita[] = {900, 750, 600, 300};
volatile int velocitacorrente = 0;	
	
volatile int tick = 0;
volatile int secondi = 0;

volatile bool direzione = false;	

void lampeggio();
void change();
void inverti();

int main(void)
{
    DDRC |= (l0|l1|l2|l3|l4|l5);
	PORTC &= ~(l0|l1|l2|l3|l4|l5);
	
	DDRD &= ~(b2|b3|b4|b5|b6|b7);
	PORTD |= (b2|b3|b4|b5|b6|b7);
	
	PCICR |= (1<< PCIE2);
	PCMSK2 |= (b2|b3|b4|b5|b6|b7);
	
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
			case accensione:
			PORTC |= l5;
			if ((pulsante == b4) || (pulsante == b5))
			{
				currentstate = avvio;
			}break;
			
			
			case avvio:
			TCCR0B |= ((1<<CS00) | (1<<CS02));
			TCCR1B |= ((1<<CS10) | (1<<CS12));
			change();
			lampeggio();
			if (pulsante == b7)
			{
				if (secondi == 1)
				{
				inverti();
				}secondi = 0;
			}pulsante = 0;
			
			
			if (pulsante == b2)
			{
				currentstate = reset;
			}break;
			
			
			case reset:
			TCCR0B &=~ ((1<<CS00) | (1<<CS02));
			TCCR1B &=~ ((1<<CS10) | (1<<CS12));
			PORTC &= ~(l0|l1|l2|l3|l4|l5);
			tick = 0;
			secondi = 0;
			velocitacorrente = 0;
			pulsante = 0;
			currentstate = accensione;
			break;
			
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	for (uint8_t i = PIND2; i <= PIND7; i++)
	{
		if ((change &(1 << i)) && !(PIND & (1 << i)))
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


void change(){
	switch(pulsante){
		case b4:
		if (secondi == 1)
		{
			if (velocitacorrente < 3)
			{
				velocitacorrente++;
			}
		}secondi = 0;
		pulsante = 0;
		break;
		
		
		case b5:
		if (secondi == 1)
		{
			if (velocitacorrente > 0)
			{
				velocitacorrente--;
			}
		}secondi = 0;
		pulsante = 0;
		break;
		
	}
}


void inverti(){
	direzione = !direzione;
}


void lampeggio(){
	int period;
	int cycle;
	
	period = velocita[velocitacorrente];
	cycle = period /3;
	if (direzione)
	{
		if (tick <= cycle)
		{
			PORTC |= l1;
			PORTC &=~ (l2|l3);
		}else if(tick <= 2*cycle){
			PORTC |= l2;
			PORTC &=~ (l3|l1);
		}else{
			PORTC |= l3;
			PORTC &=~ (l1|l2);
		}
		if (tick == period)
		{
			tick = 0;
		}
		
		
	}else{
		if (tick <= cycle)
		{
			PORTC |= l3;
			PORTC &=~ (l1|l2);
			
			}else if(tick <= 2*cycle){
			PORTC |= l2;
			PORTC &=~ (l1|l3);
			}else{
			PORTC |= l1;
			PORTC &=~ (l3|l2);
		}
		if (tick == period)
		{
			tick = 0;
		}
		
	}
	
}