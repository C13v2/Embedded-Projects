/*
 * GccApplication1.c
 *
 * Created: 15/09/2024 15:39:42
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define l0 (1 << PINC0)
#define l5 (1 << PINC5)
#define l3 (1 << PINC3)

#define b2 (1 << PIND2)
#define b4 (1 << PIND4)
#define b7 (1 << PIND7)

typedef enum{
	erogazione,
	wait,
	reset
	}stato;
stato currentstate = erogazione;

volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xff;

volatile int tick = 0;
volatile int secondi = 0;
volatile int presstimer = 0;
void lampeggio();
void lampeggio2();


int main(void)
{
    DDRC |= (l0 | l3 | l5);
    PORTC &=~ (l0 | l3 | l5);
	
    DDRD &=~ (b2 | b4 | b7);
    PORTD |= (b2 | b4 | b7);
	
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (b2 | b4 | b7);
	
    TCCR0A |= (1 << WGM01);
    TIMSK0 |= (1 << OCIE0A);
    OCR0A = 79; //5ms
	
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 15625; // 1sec
	
    TCCR2A |= (1 << WGM21);
    TIMSK2 |= (1 << OCIE2A);
    OCR2A = 157; //10ms
	
    sei();
	
    TCCR2B |= ((1<<CS20) | (1<<CS21) | (1<<CS22));


    while (1) 
    {
		switch(currentstate){
			
			case erogazione:
			TCCR0B |= ((1 << CS00) | (1 << CS02));
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			switch(pulsante){
				case b2:
				lampeggio();
				if (secondi == 8)
				{
					currentstate = reset;
				}break;
				
				case b7:
				if (presstimer <= 500)
				{
					lampeggio2();
				}else{
					PORTC |= l0;
				}break;
				
				case b4:
				currentstate = wait;
				break;
			
			}
		
			case wait:
			TCCR0B |= ((1 << CS00) | (1 << CS02));
			TCCR1B |= ((1 << CS10) | (1 << CS12));
			PORTC &=~ (l0 | l5);
			PORTC |= l3;
			if(secondi == 15){
				currentstate = reset;
			}
			break;			
			
			
			case reset:
			TCCR0B &=~ ((1 << CS00) | (1 << CS02));
			TCCR1B &=~ ((1 << CS10) | (1 << CS12));
			PORTC &=~ (l0 | l3 | l5);
			tick = 0;
			secondi = 0;
			presstimer = 0;
			pulsante = 0;
			currentstate = erogazione;
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
		if ((change & (1 << i)) && !(PIND & (1<<i)))
		{
			pulsante = (1 << i);
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
}


void lampeggio(){
	int cycle;
	int period;
	
	cycle = 50;
	period = 150;
	
	if(tick <= cycle){
		PORTC |= l5;
	}
	else{
		PORTC &=~ l5;
	}
	if(tick == period){
		tick = 0;
	}
}



void lampeggio2(){
	int cycle;
	int period;
	
	cycle = 50;
	period = 100;
	
	if(tick <= cycle){
		PORTC |= l0;
	}
	else{
		PORTC &=~ l0;
	}
	if(tick == period){
		tick = 0;	}
}