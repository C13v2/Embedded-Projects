/*
 * porta motorizzata.c
 *
 * Created: 21/06/2024 15:26:20
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

typedef enum {
	inserimento,
	apertura,
	reset
	}stato;
	stato currentstate = inserimento;

volatile uint8_t pulsantepremuto;
volatile uint8_t oldvalue = 0xff;

volatile uint8_t password[] = {b2 , b7, b7, b3, b2};
volatile uint8_t guess[] = {0,0,0,0,0};
volatile int currentpos = 0;
									
volatile int tick = 0;
volatile int nopress = 0;

void accendi();
bool passcheck();

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
	OCR0A = 156;
	
	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 15625;
	 
	sei();
    while (1) 
    {
		switch (currentstate)
		{
			case inserimento:
			if (pulsantepremuto)
			{
			guess[currentpos] = pulsantepremuto;
			accendi();
			currentpos++;
			pulsantepremuto = 0;
			nopress = 0; 
			}
			
			if (currentpos == 5)
			{
				if (passcheck());
				{
					currentstate = apertura;
					TCCR0B |= ((1<<CS00) | (1<<CS02));
					TCCR2B |= ((1<<CS20) | (1<<CS21) | (1<<CS22));
				}
			}else {
			currentstate = reset;
			}
			
			if (nopress == 4)
			{
				currentstate = reset;
			}
			break;
			
			case apertura:
			PORTC |= l5;
			break;
			
			case reset:
			pulsantepremuto = 0;
			nopress = 0;
			tick = 0;
			currentpos = 0;
			currentstate = inserimento;
			
		}
    }
}


ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	bool hold = false;
	for(uint8_t i = PIND2; i <= PIND7; i++){
		if((change & (1<<i)) && !(PIND & (1<<i))){
			pulsantepremuto = (1<<i);
			hold = true;
		}
	}
	if(!(hold)){
		pulsantepremuto = 0;
	}
}

ISR(TIMER0_COMPA_vect){
	tick++;
}

ISR(TIMER1_COMPA_vect){
	nopress++;
}

void accendi(){
	switch(currentpos){
		default:
		PORTC |= l0;
		break;
		case 1:
		PORTC |= l1;
		break;
		case 2:
		PORTC |= l3;
		break;
		case 3:
		PORTC |= l4;
		break;
	}
	pulsantepremuto = 0;
}

bool passcheck(){
		 for(int i = 0; i<5; i++){
			 if(guess[i] != password[i]){
				 return false;
			 }
		 }
		 return true;
}
