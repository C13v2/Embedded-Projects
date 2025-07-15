/*
 * GccApplication1.c
 *
 * Created: 13/09/2024 17:11:24
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define B2 (1<<PIND2)
#define B3 (1<<PIND3)
#define B4 (1<<PIND4)
#define B5 (1<<PIND5)
#define B6 (1<<PIND6)
#define B7 (1<<PIND7)

#define L0 (1<<PINC0)
#define L1 (1<<PINC1)
#define L2 (1<<PINC2)
#define L3 (1<<PINC3)
#define L4 (1<<PINC4)
#define L5 (1<<PINC5)
#define Leds (L0 | L1 | L2 | L3 | L4 | L5)
#define Buttons (B2 | B3 | B4 | B5 | B6 | B7)

typedef enum{
	combinazione,
	zucchero,
	erogazione,
	anomalia,
	reset
	}stato;
stato currentstate = combinazione;


typedef enum{
	the_bevanda,
	cioccolata_bevanda,
	americano_bevanda,
	espresso_bevanda,
}type;
type currenttype = the_bevanda;


volatile uint8_t pulsante = 0;
volatile uint8_t oldvalue = 0xFF;

volatile int tick = 0;
volatile int secondi = 0;
volatile int press = 0;

volatile uint8_t password[] = {B2, B4, B2};
volatile uint8_t password1[] = {B3, B5, B3};
volatile uint8_t password2[] = {B4, B6, B4};
volatile uint8_t password3[] = {B5, B7, B5};
	
volatile uint8_t guess[] = {0, 0, 0};
	
volatile int currentpos  = 0;

void check();
void lampeggio();
void eroga();
void lampeggiozucchero1();
void lampeggiozucchero2();
void lampeggioanomalia();

int main(void)
{
   DDRC |= Leds;
   PORTC &=~ Leds;
   
   DDRD &=~ Buttons;
   PORTD |= Buttons;
   
   PCICR |= (1<<PCIE2);
   PCMSK2 |= Buttons;
   
   TCCR0A |= (1<<WGM01);
   TIMSK0 |= (1<<OCIE0A);
   OCR0A = 79; //10ms
   
   TCCR1B |= (1<<WGM12);
   TIMSK1 |= (1<<OCIE1A);
   OCR1A = 15626; //1s
   
   TCCR2A |= (1<<WGM21);
   TIMSK2 |= (1<<OCIE2A);
   OCR2A =156;
   
   sei();
   

   TCCR2B |= ((1 << CS20) |(1 << CS21) | (1 << CS22));
    while (1) 
    {
		switch(currentstate){
			case combinazione:
			if (pulsante)
			{
				guess[currentpos] = pulsante;
				currentpos++;
				
			}pulsante = 0;
			if (currentpos == 3)
			{
				check();
			}break;
			
		switch(currenttype){
		case the_bevanda:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		PORTC |= L5;
		if (pulsante == B7)
		{
			currentstate = zucchero;
		}
		pulsante = 0;
		break;
		
		
		case cioccolata_bevanda:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		PORTC |= (L4|L5);
		if (pulsante == B7)
		{
			currentstate = zucchero;
		}break;
		
		
		
		case americano_bevanda:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		PORTC |= L4;
		if (pulsante == B7)
		{
			currentstate = zucchero;
		}break;
		
		
		case espresso_bevanda:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		lampeggio();
		if (pulsante == B7)
		{
			currentstate = zucchero;
		}break;
		
	}
		
		
		case zucchero:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		if (pulsante == B7)
		{
			if (press <= 300)
			{
				lampeggiozucchero1();
			}
		}pulsante = 0;
		if (secondi == 3)
		{
			currentstate = erogazione;
		}
		
		else if (pulsante == B7)
		{
			if (press > 300)
			{
				lampeggiozucchero2();
			}
		}pulsante = 0;
		if (secondi == 6)
		{
			currentstate = erogazione;
		}
		break;
		
		
		case erogazione:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		eroga();
		if (secondi == 5)
		{
			currentstate = reset;
		}
		if (pulsante == B4)
		{
			currentstate = anomalia;
		}pulsante = 0;
		break;
		
		case anomalia:
		TCCR0B |= ((1 << CS00)|(1<<CS02));
		TCCR1B |= ((1 << CS10) |(1 << CS12));
		lampeggioanomalia();
		if (secondi == 4)
		{
			currentstate = reset;
		}break;
		
		case reset:
		TCCR0B &=~ ((1 << CS00)|(1<<CS02));
		TCCR1B &=~ ((1 << CS10) |(1 << CS12));
		PORTC &=~ Leds;
		tick = 0;
		secondi = 0;
		press = 0;
		pulsante = 0;
		currentpos = 0;
		currentstate = combinazione;
		break;
		
		}
    }
}

ISR(PCINT2_vect){
	uint8_t change = oldvalue ^ PIND;
	oldvalue = PIND;
	bool hold = false;
	for(uint8_t i = PIND2; i <= PIND7; i++){
		if((change & (1<<i)) && !(PIND & (1<<i))){
			pulsante = (1<<i);
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


void check(){
		bool errorcheck = false;
		for(uint8_t i= 0; i<3; i++){
			if (guess[i] != password[i]){
				currentstate = reset;
				errorcheck = true;
				break;
			}
		}
		if (!errorcheck){
			currenttype = the_bevanda;
			return;
		}
		
		
		errorcheck = false;
		for(uint8_t i= 0; i<3; i++){
			if (guess[i] != password1[i]){
				currentstate = reset;
				errorcheck = true;
				break;
			}
		}
		if (!errorcheck){
			currenttype = cioccolata_bevanda;
			return;
		}
		
		
		errorcheck = false;
		for(uint8_t i= 0; i<3; i++){
			if (guess[i] != password2[i]){
				currentstate = reset;
				errorcheck = true;
				break;
			}
		}
		if (!errorcheck){
			currenttype = americano_bevanda;
			return;
		}
		
		
		errorcheck = false;
		for(uint8_t i= 0; i<3; i++){
			if (guess[i] != password3[i]){
				currentstate = reset;
				errorcheck = true;
				break;
			}
		}
		if (!errorcheck){
			currenttype = espresso_bevanda;
		}

}

void lampeggio(){
	int period;
	int cycle;
	
	period = 100;
	cycle = 10;
	
	if (tick <= cycle)
	{
		PORTC |= (L4|L5);
	}else{
		PORTC &=~ (L4|L5);
	}
	if (tick == period)
	{
		tick = 0;
	}
}



void lampeggiozucchero1(){
	int period;
	int cycle;
	
	period = 80;
	cycle = 40;
	
	if (tick <= cycle)
	{
		PORTC |= L1;
		PORTC &=~ L0;
		
		}else{
		PORTC &=~ L1;
		PORTC |= L0;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
}


void lampeggiozucchero2(){
	int period;
	int cycle;
	
	period = 160;
	cycle = 80;
	
	if (tick <= cycle)
	{
		PORTC |= L1;
		PORTC &=~ L0;
		
		}else{
		PORTC &=~ L1;
		PORTC |= L0;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
}


void eroga(){
	int period;
	int cycle;
	
	period = 140;
	cycle = 70;
	
	if (tick <= cycle)
	{
		PORTC |= L2;
		}else{
		PORTC &=~ L2;
	}
	if (tick == period)
	{
		tick = 0;
	}
	
}

void lampeggioanomalia(){
	int period;
	int cycle;
	
	period = 140;
	cycle = 70;
	
	if (tick <= cycle)
	{
		PORTC |= (L1|L0);
		}else{
		PORTC &=~ (L1|L0);
	}
	if (tick == period)
	{
		tick = 0;
	}
	
}