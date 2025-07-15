/*
 * bistecchiera.c
 *
 * Created: 24/06/2024 17:52:34
 * Author : Christian
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

 #define L0 (1<<PINC0)
 #define L1 (1<<PINC1)
 #define L2 (1<<PINC2)
 #define L4 (1<<PINC4)
 #define L5 (1<<PINC5)
 
 #define B2 (1<<PIND2)
 #define B3 (1<<PIND3)
 #define B4 (1<<PIND4)
 #define B7 (1<<PIND7)
 
 typedef enum {
	 selezione,
	 cottura,
	 reset
	 }stato;
	 stato currentstate = selezione;
 volatile uint8_t oldvalue = 0xff;
 volatile uint8_t pulsantepremuto;
 volatile int tick = 0;
 volatile int secondi = 0;
 volatile int pulsantetenutopremuto = 0;
 volatile int level = 0;
 volatile int cycle = 0;
 volatile int period = 0;


void incrementodecremento();
	void livelli(); 
	void blink();
	
int main(void)
{
    DDRD &=~ (B2|B3|B4|B7);
	PORTD |= (B2|B3|B4|B7);
	
	DDRC |= (L0|L1|L2|L4|L5);
	PORTC &=~ (L0|L1|L2|L4|L5);
	
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (B2|B3|B4|B7);
	
	TCCR0A |= (1 << WGM01);
	TIMSK1 |=(1 << OCIE0A);
	OCR0A =157; //10ms
	
	TCCR1B |= (1<<WGM12);
	TIMSK1 |= (1<<OCIE1A);
	OCR1A = 15626; //1s
	 
	TCCR2A |= (1<<WGM21);
	TIMSK2 |= (1<<OCIE2A);
	OCR2A = 157; //10ms
	 
	TCCR2B |= ((1<<CS20) | (1<<CS21) | (1<<CS22));
	 
	sei();
    while (1) 
    {
		switch(currentstate){
			case selezione:
			livelli();
			incrementodecremento();
			
			 if(pulsantepremuto == B7){
				 TCCR0B |= ((1<<CS00) | (1<<CS02));
				 TCCR1B |= ((1<<CS10) | (1<<CS12));
				 currentstate = cottura;
				 pulsantepremuto = 0;
			 }
			 break;
			 
			 
			 case cottura:
			 blink();
			 if((secondi == 16) || (pulsantepremuto == B4)){
				 currentstate = reset;
				pulsantepremuto = 0;
				}break;
				
			case reset:
			 TCCR0B &=~ ((1<<CS00) | (1<<CS02));
			 TCCR1B &=~ ((1<<CS10) | (1<<CS12));
			 PORTC &=~ (L0|L1|L2|L4|L5);;
			 secondi = 0;
			 tick = 0;
			 pulsantetenutopremuto = 0;
			 level = 0;
			 pulsantepremuto = 0;
			 cycle = 0;
			 period = 0;
			 currentstate = selezione;
			 break;
						
		 }
    }
}

 ISR(PCINT2_vect){
	 uint8_t change = oldvalue ^ PIND;
	 bool hold = false;
	 oldvalue = PIND;
	 for(uint8_t i = PIND2; i<=PIND7; i++){
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
	 secondi++;
 }
 ISR(TIMER2_COMPA_vect){
	 pulsantetenutopremuto++;
 }
 
 
 
 
 void incrementodecremento(){
  switch(pulsantepremuto){
	  case B3:
	  if(pulsantetenutopremuto == 100){
		  pulsantetenutopremuto = 0;
		  if(level > 0){
			  level--;
		  }
	  }break;
	  
	  case B2:
	  if(pulsantetenutopremuto == 100){
		  pulsantetenutopremuto = 0;
		  if(level < 7){
			  level++;
		  }
	  } break;
	  
	  default:
	  pulsantetenutopremuto = 0;
	  break;
  }
	  }
	  
	  
	  
	void livelli(){
	  	switch(level){
		  	case 7:
		  	PORTC |= (L0 | L1 | L2);
		  	break;
		  	
		  	case 6:
		  	PORTC &=~ (L0);
		  	PORTC |= (L1 | L2);
		  	cycle = 45;
		  	period = 50;
		  	break;
		  	
		  	case 5:
		  	PORTC &=~ (L1);
		  	PORTC |= (L0 | L2);
		  	cycle = 35;
		  	period = 50;
		  	break;
		  	
		  	case 4:
		  	PORTC &=~ (L0 | L1);
		  	PORTC |= (L2);
		  	cycle = 25;
		  	period = 50;
		  	break;
		  	
		  	case 3:
		  	PORTC &=~ (L2);
		  	PORTC |= (L0 | L1);
		  	cycle = 40;
		  	period = 100;
		  	break;
		  	
		  	case 2:
		  	PORTC &=~ (L0 | L2);
		  	PORTC |= (L1);
		  	cycle = 20;
		  	period = 100;
		  	break;
		  	
		  	case 1:
		  	PORTC &=~ (L1 | L2);
		  	PORTC |= (L0);
		  	cycle = 10;
		  	period = 100;
		  	break;
		  	
		  	default:
		  	PORTC &=~ (L0 | L1 | L2);
		  	break;
	  	}
  	}
	  
	  
	  
	  void blink(){
		  if(level == 0){
			PORTC |= L4;
			PORTC &=~ L5;
	   }
			else if(level == 7){
			 PORTC |= L5;
			PORTC &=~ L4;
		}
		else{
			if(tick <= cycle){
			 PORTC |= L5;
			 PORTC &=~ L4;
		}
			else{

			PORTC |= L4;
			PORTC &=~ L5;
		}
		if(tick == period){
		 tick = 0;		  
	  }
		}
	  }