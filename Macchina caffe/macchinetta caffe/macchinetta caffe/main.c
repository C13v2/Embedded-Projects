/*
 * macchinetta caffe.c
 *
 * Created: 26/06/2024 17:53:14
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
#define b4 (1<<PIND4)
#define b7 (1<<PIND7)

typedef enum{
	zucchero,
	avvio,
	reset
	}stato;
	stato currentstate =zucchero;
	
volatile uint8_t pulsantepremuto;
volatile uint8_t oldvalue = 0xff;

volatile int tick = 0;
volatile int secondi = 0;
volatile int timer = 0;	
volatile int stop = 0;

void lampeggio();
void pressione();
void anomalia();

int main(void)
{
    DDRC |= (l0|l1|l2|l3|l4|l5);
	PORTC &=~ (l0|l1|l2|l3|l4|l5);
	
	DDRD &=~ (b2|b4|b7);
	PORTD |= (b2|b4|b7);
	
	PCICR |= (1<<PCIE2);
	PCMSK2 |= (b2|b4|b7);
	
	TCCR0A |= (1<<WGM01);
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = 79; //5ms
	
	TCCR1B |= (1<<WGM12);
	TIMSK1 |= (1<< OCIE1A);
	OCR1A = 15626; //1s
	
	TCCR2A |= (1<<WGM21);
	TIMSK2 |= (1<< OCIE2A);
	OCR2A = 156; //10ms
	
	sei();
	
	TCCR0B |= ((1 << CS00) | (1 << CS02));
	TCCR1B |= ((1 << CS10) | (1 << CS12));
	TCCR2B |= ((1 << CS21) | (1<< CS22) | (1<<CS20));
	
    while (1) 
    {
		switch(currentstate){
			case zucchero:
			if (pulsantepremuto == b7)
			{
				PORTC|=l5;
				pulsantepremuto = 0;
			}
			
			if (pulsantepremuto == b2)
			{
				currentstate = avvio;
				pulsantepremuto = 0;
			}
			break;
			
			case avvio:
			if(pulsantepremuto == b2){
				lampeggio();
				pressione();
			pulsantepremuto = 0;
			stop = 0;
			timer = 0;
			}
			
			if (pulsantepremuto == b4)
			{
				if (secondi == 4)
				{
					anomalia();
					currentstate = reset;
				}
				
				pulsantepremuto = 0;
			}break;
			
			
			case reset:
			TCCR0B &=~ ((1 << CS00) | (1 << CS02));
			TCCR1B &=~ ((1 << CS10) | (1 << CS12));
			PORTC &=~ (l0|l1|l2|l3|l4|l5);
			pulsantepremuto = 0;
			timer = 0;
			stop = 0;
			tick = 0;
			secondi = 0;
			currentstate = zucchero;
			break;
		
			
			
			
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	bool hold = false;
	for (uint8_t i= PIND2; i<= PIND7; i++)
	{
		if ((change & (1<<i)) && !(PIND &(1<<i)))
		{
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
	secondi++;
}

ISR(TIMER2_COMPA_vect){
	timer++;
}




void lampeggio(){
	
	int	cycle = 67;
	int	period = 200;
		if (tick <= cycle)
		{
			PORTC |= l0;
		}
		else if (tick <= 2*cycle)
		{
			PORTC |= l1;
			PORTC &=~ l0;
		}
		else{
			PORTC |= l2;
			PORTC &=~ l1;
		}
		if (tick >= period)
		{
			tick = 0;
		}
	}



void pressione(){
	
	if (timer <= 400)
	{
		stop = 4;
	}
	else if (timer <= 600)
	{
		stop = 6;
	}
	
	else (timer >= 600)
	{
		currentstate = reset;
	}
	
}	

void anomalia(){
	int cycle = 167;
	int period = 333;
	
	if (tick <= cycle)
	{
		PORTC |= (l3|l4);
	}
	else{
		PORTC &=~ (l3|l4);
	}
	if (tick >= period)
	{
		tick = 0;
	}

}