/*
 * dispenser.c
 *
 * Created: 25/06/2024 14:41:18
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

 typedef enum {
	pulsante,
	erogazione,
	aspetto,
	reset
 } stato;
 stato currentstate = pulsante;
 
 volatile uint8_t pulsantepremuto;
 volatile uint8_t oldvalue = 0xff;
 volatile int tick = 0;
 volatile int secondi = 0;
 volatile int presstimer = 0;
 
 
 void croccantini();
 void gatto();
 
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
	 OCR0A = 79;  //5ms
	 
	 TCCR1B |= (1 << WGM12);
	 TIMSK1 |= (1 << OCIE1A);
	 OCR1A = 15625;  // 1sec
	 
	 TCCR2A |= (1 << WGM21);
	 TIMSK2 |= (1 << OCIE2A);
	 OCR2A = 157;  //10ms
	 
	 sei();
	 
	 TCCR0B |= ((1 << CS00) | (1 << CS02));
	 TCCR1B |= ((1 << CS10) | (1 << CS12));
	 TCCR2B |= ((1<<CS20) | (1<<CS21) | (1<<CS22));


    while (1) 
    {
		switch(currentstate){
			
			case pulsante:
			if (pulsantepremuto == b2)
			{
				currentstate = erogazione;
				pulsantepremuto = 0;
			}
			else if(pulsantepremuto == b7){
				if (presstimer <= 5)
				{
					gatto();
				}
			}else{
				PORTC |= l0;
				currentstate = erogazione;
				pulsantepremuto = 0;
				tick = 0;
				secondi = 0;
			}break;
		
			
			case erogazione:
			croccantini();
			if (pulsantepremuto == b4)
			{
				currentstate = aspetto;
				pulsantepremuto = 0;
				tick = 0;
				secondi = 0;
			}break;
			
			case aspetto:
			PORTC &=~ (l0 | l5);
			PORTC |= l3;
		if (secondi == 15)
		{
			currentstate = reset;
		}
		break;
			
			case reset:
			TCCR0B &=~ ((1 << CS00) | (1 << CS02));
			TCCR1B &=~ ((1 << CS10) | (1 << CS12));
			PORTC &=~ (l0 | l3 | l5);
			tick = 0;
			secondi = 0;
			pulsantepremuto = 0;
			presstimer = 0;
			currentstate = pulsante;
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
	secondi++;
}
ISR(TIMER2_COMPA_vect){
	presstimer++;
}



void croccantini(){
	int cycle;
	int period;
	
	cycle = 250;
	period = 750;
	
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
	
	if (secondi == 8)
	{
		currentstate = reset;
	}
	
}


void gatto(){
	int cycle;
	int period;
	
	cycle = 250;
	period = 500;
	
	if (tick <= cycle)
	{
		PORTC |= l0;
		}else{
		PORTC &=~ l0;
	}
	if (tick >= period)
	{
		tick = 0;
	}
}