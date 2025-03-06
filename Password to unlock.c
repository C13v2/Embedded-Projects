/*
 * GccApplication1.c
 *
 * Created: 14/09/2024 12:26:02
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
 
 #define b3 (1<<PIND3)
 #define b4 (1<<PIND4)
 #define b5 (1<<PIND5)
 #define b7 (1<<PIND7)
 
 volatile uint8_t pulsante = 0;
 volatile uint8_t oldvalue = 0xff; volatile uint8_t password[] = {b3, b4, b7, b5}; volatile uint8_t guess[] = {0, 0, 0, 0};
volatile int tick = 0;
volatile int secondi = 0;

volatile int currentpos = 0;
volatile int loopcount = 0;

volatile int b7presscount = 0;

typedef enum{
	combinazione,
	apertura,
	reset
	}stato;
	stato currentstate = combinazione ;


void check();
void lampeggio();


int main(void)
{
    DDRC |= (l0 | l1| l2 | l3 | l5);
    PORTC &=~ (l0 | l1| l2 | l3 | l5);
	
    DDRD &=~ (b3 | b4 | b5 | b7);
    PORTD |= (b3 | b4 | b5 | b7);
	
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (b3 | b4 | b5 | b7);
	
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 1564; //100ms
	
    sei();
	

    while (1) 
    {
		switch(currentstate){
			case combinazione:
			if (pulsante)
			{
				guess[currentpos] = pulsante;
				switch(currentpos){
					default:
					PORTC |= l3;
					break;
					
					case 1:
					PORTC |= l2;
					break;
					
					case 2:
					PORTC |= l1;
					break;
					
					case 3:
					PORTC |= l0;
					break;
					
				}
				currentpos++;
				pulsante = 0;
			}
			
			if (currentpos == 4)
			{
				check();
			}
			break;
			
			
			
			
			case apertura:
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			PORTC |= l5;
			lampeggio();
			
			if (pulsante == b7)
			{
				b7presscount++;
				if (b7presscount == 2)
				{
					currentstate = reset;
				}pulsante = 0;
			}break;
			
			
			case reset:
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			PORTC &=~ (l0 | l1| l2 | l3 | l5);
			tick = 0;
			pulsante = 0;
			loopcount = 0;
			b7presscount = 0;
			currentpos = 0;
			currentstate = combinazione;
			break;
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	for (uint8_t i = PIND2; i <= PIND7; i++)
	{
		if ((change & (1 << i)) && !(PIND & (1<<i)))
		{
			pulsante = (1 << i);
		}
	}

}


ISR(TIMER1_COMPA_vect){
	tick++;
}

void check(){
	bool errorcheck = false;
	for (uint8_t i = 0; i < 4; i++)
	{
		if (guess[i] != password[i])
		{
			currentstate = reset;
			errorcheck = true;
		}
	}
	
	if (!(errorcheck))
	{
		currentstate = apertura;
		tick = 0;
	}
}


void lampeggio(){
	if (tick < 5 && loopcount < 3)
	{
		PORTC |= (l0 | l1| l2 | l3);
	}
	else{
		PORTC &=~ (l0 | l1| l2 | l3);
	}
	if (tick == 7)
	{
		tick = 0;
		loopcount++;
	}
}