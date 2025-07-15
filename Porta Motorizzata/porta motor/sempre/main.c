/*
 * sempre.c
 *
 * Created: 25/06/2024 11:38:35
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
	stazionamento,
	chiusura,
	reset
	}stato;
stato currentstate = inserimento;

volatile uint8_t pulsantepremuto;
volatile uint8_t oldvalue = 0xff;
volatile uint8_t password[]= {b2,b7,b7,b3,b2};
	volatile uint8_t guess[]= {0,0,0,0,0};
		volatile int currentpos=0;

volatile int tick = 0;
volatile int tick2 = 0;
volatile int nopress = 0;


void accensioneled();
bool check();
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
	OCR0A = 79; //5ms
	
	TCCR1B |= (1 << WGM12);
	TIMSK1 |= (1 << OCIE1A);
	OCR1A = 15626; //1sec
	
	TCCR2B |= (1 << WGM21);
	TIMSK2 |= (1 << OCIE2A);
	OCR2A = 157;
	
	TCCR1B |= ((1 << CS10) | (1 << CS12));
	TCCR0B |= ((1 << CS00) | (1 << CS02));
	TCCR2B |= ((1 << CS20) | (1 << CS21) |(1 << CS22));
	
	sei();
    while (1) 
    {
		switch (currentstate)
		{
			case inserimento:
			if (pulsantepremuto)
			{
				guess[currentpos]= pulsantepremuto;
				accensioneled();
				currentpos++;
				pulsantepremuto = 0;
				
			}
			if (currentpos == 5)
			{
				if (check())
				{
					currentstate = apertura;
				}
				else
				{
					currentstate = reset;
				}
			}
			if (nopress == 4)
			{
				currentstate = reset;
			}break;
			
		case apertura:
		lampeggio();
		if (pulsantepremuto == b5)
		{
			currentstate = reset;
			pulsantepremuto = 0;
			tick = 0;
			tick2 = 0;
		}break;
		
		case stazionamento:
		lampeggio();
		if (pulsantepremuto == b5)
		{
			currentstate = reset;
			pulsantepremuto = 0;
			tick = 0;
			tick2 = 0;
		}break;
		
		case chiusura:
		lampeggio();
		break;
			
		case reset:
		PORTC &=~ (l0|l1|l2|l3|l4|l5);
		tick2 = 0;
		tick = 0;
		nopress = 0;
		pulsantepremuto = 0;
		currentstate = inserimento;
			
			
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	bool hold = false;
	for (uint8_t i=PIND2; i <= PIND7; i++)
	{
		if ((change & (1 <<i)) && !(PIND &(1<<i)))
		{
			pulsantepremuto = (1 << i);
			hold = true;
		}
	}
	if (!(hold))
	{
		pulsantepremuto = 0;
	}
	
}

ISR(TIMER0_COMPA_vect){
	tick++;
}

ISR(TIMER1_COMPA_vect){
	tick2++;
}

ISR(TIMER2_COMPA_vect){
	nopress++;
}

void accensioneled(){
	switch(currentpos){
		default:
		PORTC |= l0;
		break;
		
		case 1:
		PORTC |=l1;
		break;
		
		case 2:
		PORTC |= l2;
		break;
		
		case 3:
		PORTC |= l3;
		break;
		
		case 4:
		PORTC |= l4;
		break;
	}
}


bool check(){
	for (int i = 0; i<5; i++)
	{
		if (guess[i] != password[i])
		{
			return false;
		}
	}
	return true;
}



void lampeggio(){
	int cycle;
	int period;
	stato nextstate;
	
	
	switch(currentstate){
	case apertura:
	cycle = 200;
	period = 800;
	nextstate = stazionamento;
	break;
	
	case stazionamento:
	period = 600;
	PORTC |= l5;
	nextstate  = chiusura;
	break;
	
	case chiusura:
	cycle = 525;
	period = 700;
	nextstate = reset;
	break;

}

	if (tick <= cycle)
	{
		PORTC |= l5;
	}else{
	PORTC &=~ l5;
	}
	
	if (tick >= period)
	{
		tick = 0;
	}
}
