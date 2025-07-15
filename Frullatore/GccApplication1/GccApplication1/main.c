/*
 * GccApplication1.c
 *
 * Created: 13/09/2024 09:54:47
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define l1 (1 << PINC1)
#define l2 (1 << PINC2)
#define l3 (1 << PINC3)
#define l4 (1 << PINC4)

#define b2 (1 << PIND2)
#define b3 (1 << PIND3)
#define b7 (1 << PIND7)

typedef enum{
	spento,
	minimo,
	medio,
	alto,
	massimo,
	reset
	}stato;
	stato currentstate = spento;
	
volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int tick = 0;
volatile int secondi = 0;
volatile int velocitacorrente = 0;

void cambio();
void led(uint8_t led1, uint8_t led2, uint8_t led3, uint8_t led4);
void velocita();


int main(void)
{	
	DDRC |= (l1|l2|l3|l4);
	PORTC &= ~(l1|l2|l3|l4);
	
	DDRD &= ~(b2|b3|b7);
	PORTD |= (b2|b3|b7);
	
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (b2|b3|b7);
	
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
			case spento:
			cambio();
			break;
			
			case minimo:
			TCCR0B |= (1<<CS00) | (1<<CS02);
			TCCR1B |= (1<<CS10) | (1<<CS12);
			cambio();
			led( l1, 0, 0, 0);
			
			if ((pulsante == b7) || (secondi == 10))
			{
				currentstate = reset;
			}
			break;
			
			
			case medio:
			TCCR0B |= (1<<CS00) | (1<<CS02);
			TCCR1B |= (1<<CS10) | (1<<CS12);
			cambio();
			led( l1,  l2, 0, 0);
			
			if ((pulsante == b7) || (secondi == 10))
			{
				currentstate = reset;
			}
			break;
			
			
			case alto:
			TCCR0B |= (1<<CS00) | (1<<CS02);
			TCCR1B |= (1<<CS10) | (1<<CS12);
			cambio();
			led( l1,  l2,  l3, 0);
			
			if ((pulsante == b7) || (secondi == 10))
			{
				currentstate = reset;
			}
			break;
			
			
			
			case massimo:
			TCCR0B |= (1<<CS00) | (1<<CS02);
			TCCR1B |= (1<<CS10) | (1<<CS12);
			cambio();
			led( l1,  l2,  l3,  l4);
			
			if ((pulsante == b7) || (secondi == 10))
			{
				currentstate = reset;
			}
			break;
			
			
			
			
			case reset:
			TCCR0B &=~ (1<<CS00) | (1<<CS02);
			TCCR1B &=~ (1<<CS10) | (1<<CS12);
			PORTC &=~ (l1|l2|l3|l4); ;
			tick = 0;
			secondi = 0;
			pulsante = 0;
			velocitacorrente = 0;
			currentstate = spento;
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	for (uint8_t i = PIND2; i <= PIND7; i++)
	{
		if ((change & (1 << i))  && !(PIND & (1 << i)))
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



void cambio(){
	switch(pulsante){
		case b3:
		if (velocitacorrente < 4)
		{
			velocitacorrente++;
			velocita();
		}
		pulsante = 0;
		break;
		
		
		case b2:
		if (velocitacorrente > 0)
		{
			velocitacorrente--;
			velocita();
		}
		pulsante = 0;
		break;
		
		
	}
}




void velocita(){
	switch(velocitacorrente){
		
		default:
		currentstate = spento;
		break;
		
		case 1: 
		currentstate = minimo;
		break;
		
		case 2:
		currentstate = medio;
		break;
		
		case 3:
		currentstate = alto;
		break;
		
		case 4:
		currentstate = massimo;
		break;
		
	}
}





void led(uint8_t led1, uint8_t led2, uint8_t led3, uint8_t led4){
	int period;
	int cycle;
	
	period = 20 ;
	cycle = 5;
	
	if (tick <= cycle)
	{
		PORTC |= led1;
}else
{
	PORTC &=~ led1;
}
	if (tick == period)
	{
		tick = 0;
	}
	
	
	
	
	if (tick <= 2*cycle)
	{
		PORTC |= led2;
	}else
	{
		PORTC &=~ led2;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
	
	
	if (tick <= 3*cycle)
	{
		PORTC |= led3;
	}else
	{
		PORTC &=~ led3;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
	
	
	
	if (tick <= 4*cycle)
	{
		PORTC |= led4;
	}else
	{
		PORTC &=~ led4;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
	
	
}