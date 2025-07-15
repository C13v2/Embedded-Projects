/*
 * GccApplication1.c
 *
 * Created: 12/09/2024 19:41:50
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

#define leds (L0 | L1 | L2 | L3 | L4 | L5)
#define buttons (B2 | B3 | B4 | B5 | B6 | B7)

volatile uint8_t press;
volatile uint8_t oldval = 0xff;
volatile int secondi = 0;
volatile int velocita[] = {900, 750, 600, 300};
volatile int velocitacorrente = 0;
volatile int tick = 0;
volatile bool orario = false;

typedef enum {
	riposo,
	rotazione,
	reset,
} states;

states currentstates = riposo;

void led();
void cambio();

int main(void) {
	DDRC |= leds;
	PORTC &=~ leds;

	DDRD &=~ buttons;
	PORTD |= buttons;

	
	PCICR |= (1<<PCIE2);
	PCMSK2 |= buttons;


	TCCR0A |= (1<<WGM01);
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = 79; //5ms

	TCCR1B |= (1<<WGM12);
	TIMSK1 |= (1<<OCIE1A);
	OCR1A = 15626; //1s


	TCCR1B |= ((1<<CS10) | (1<<CS12));
	TCCR0B |= ((1<<CS00) | (1<<CS02));


	sei();

	while (1) {
		switch (currentstates) {
			case riposo:
			PORTC |= L5;
			if((press == B4) || (press == B5)){
				currentstates = rotazione;
			}
			break;
			
			case rotazione:
			led();
			if (press == B4)
			{
				if (secondi>=1)
				{
					if (velocitacorrente < 3){
						velocitacorrente++;
					}
					secondi =0;
				}
			}
			else if(press == B5){
				if (secondi >= 1)
				{
					if (velocitacorrente > 0)
					{
						velocitacorrente--;
					}
					secondi = 0;
				}
				
			}
			if (press == B7)
			{
				if(secondi >= 1){
					cambio();
					secondi = 0;
				}
				press = 0;
			}
			
			if (press == B2){
				currentstates = reset;
			}
			break;
			
			
			
			
			case reset:
			TCCR1B &=~ ((1<<CS10) | (1<<CS12));
			TCCR0B &=~ ((1<<CS00) | (1<<CS02));
			PORTC &=~ leds;
			tick = 0;
			secondi = 0;
			press = 0;
			currentstates = riposo;
			break;
		}
	}
}

ISR(PCINT2_vect) {
	uint8_t change = oldval ^ PIND;
	oldval = PIND;
	for (uint8_t i = PIND2; i <= PIND7; i++) {
		if ((change & (1<<i)) && !(PIND & (1<<i))) {
			press = (1<<i);
		}
	}
}

ISR(TIMER0_COMPA_vect) {
	tick++;
}

ISR(TIMER1_COMPA_vect) {
	secondi++;
}


void led() {
	int period = velocita[velocitacorrente];
	int cycle = period/3;
	if(orario){
		if (tick <= cycle) {
			PORTC |= L1;
			PORTC &=~ (L3 | L2);
			} else if (tick <= 2*cycle) {
			PORTC |= L2;
			PORTC &=~ (L1 | L3);
			} else {
			PORTC |= L3;
			PORTC &=~ (L1 | L2);
		}
		if (tick >= period) {
			tick = 0;
		}
	}
	else{
		if (tick <= cycle) {
			PORTC |= L3;
			PORTC &=~ (L1 | L2);
			} else if (tick <= 2*cycle) {
			PORTC |= L2;
			PORTC &=~ (L1 | L3);
			} else {
			PORTC |= L1;
			PORTC &=~ (L3 | L2);
		}
		if (tick >= period) {
			tick = 0;
		}
	}
}
void cambio(){
	orario = !orario;
}
