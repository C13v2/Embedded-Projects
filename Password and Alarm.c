/*
 * Password e allarme.c
 *
 * Created: 24/02/2025 17:57:03
 * Author : Christian
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define l0 (1<<PINC0)
#define l1 (1<<PINC1)
#define l2 (1<<PINC2)
#define l3 (1<<PINC3)
#define l5 (1<<PINC5)  // LED per la porta aperta
#define b3 (1<<PIND3)
#define b4 (1<<PIND4)
#define b5 (1<<PIND5)
#define b7 (1<<PIND7)  // Pulsante di reset

#define BUZZER (1<<PINC4)  // Pin del buzzer

volatile uint8_t pulsantepremuto = 0;
volatile uint8_t oldval = 0xff;
volatile uint8_t password[] = {b3, b5, b4, b5};  // Password corretta
volatile uint8_t guess[] = {0, 0, 0, 0};  // Sequenza inserita dall'utente
volatile uint8_t b7presscount = 0;
volatile int currentpos = 0;
volatile int tick = 0;
volatile int timer = 0;
volatile int loopcount = 0;

typedef enum {
	inserimentopassword,
	avvio,    // Controllo password e lampeggio
	reset,    // Reset del sistema
	allarme   // Stato di allarme
} states;

states currentstate = inserimentopassword;

void flash_leds();
void check_password();
void start_alarm();
void stop_alarm();

int main(void) {
	DDRC |= (l1 | l2 | l3 | l0 | l5 | BUZZER);  // Imposta i LED e il buzzer come output
	PORTC &=~ (l1 | l2 | l3 | l0 | l5 | BUZZER);  // Spegni tutti i LED e il buzzer
	DDRD &=~ (b3 | b4 | b5 | b7);  // Pulsanti come input
	PORTD |= (b3 | b4 | b5 | b7);  // Attiva il pull-up sui pulsanti

	PCICR |= (1 << PCIE2);  // Abilita le interruzioni per il gruppo di pin D
	PCMSK2 |= (b3 | b4 | b5 | b7);  // Abilita l'interruzione per i pulsanti

	TCCR1B |= (1 << WGM12);  // Modalità CTC
	TIMSK1 |= (1 << OCIE1A);  // Abilita l'interruzione del timer
	OCR1A = 1564;  // Imposta il timer per un intervallo di 100 ms
	sei();  // Abilita le interruzioni globali

	while (1) {
		switch (currentstate) {
			case inserimentopassword:
			if (pulsantepremuto != 0) {
				guess[currentpos] = pulsantepremuto;
				switch (currentpos) {
					case 0: PORTC |= l0; break;
					case 1: PORTC |= l1; break;
					case 2: PORTC |= l2; break;
					case 3: PORTC |= l3; break;
				}
				currentpos++;
				pulsantepremuto = 0;
			}

			if (currentpos == 4) {
				check_password();
			}
			break;

			case avvio:
			TCCR1B |= (1 << CS10) | (1 << CS12);  // Avvia il timer
			flash_leds();
			PORTC |= l5;  // Porta aperta
			if (pulsantepremuto == b7) {
				b7presscount++;
				if (b7presscount == 2) {
					currentstate = reset;
				}
				pulsantepremuto = 0;
			}
			break;

			case reset:
			TCCR1B &=~ ((1 << CS10) | (1 << CS12));  // Ferma il timer
			PORTC &=~ (l0 | l1 | l2 | l3 | l5 | BUZZER);  // Spegni i LED e il buzzer
			currentpos = 0;
			tick = 0;
			b7presscount = 0;
			loopcount = 0;
			pulsantepremuto = 0;
			currentstate = inserimentopassword;
			break;

			case allarme:
			start_alarm();  // Avvia l'allarme
			if (pulsantepremuto == b7) {
				b7presscount++;
				if (b7presscount == 2) {
					currentstate = reset;  // Resetta il sistema
				}
				pulsantepremuto = 0;
			}
			break;
		}
	}
}

void check_password() {
	bool errorcheck = false;
	for (uint8_t i = 0; i < 4; i++) {
		if (guess[i] != password[i]) {
			currentstate = allarme;  // Se la password è errata, attiva l'allarme
			errorcheck = true;
		}
	}

	if (!errorcheck) {
		currentstate = avvio;  // Password corretta, sistema avviato
	}
	tick = 0;
}

void flash_leds() {
	if (tick < 5 && loopcount < 3) {
		PORTC |= (l0 | l1 | l2 | l3);
		} else {
		PORTC &=~ (l0 | l1 | l2 | l3);
	}
	if (tick == 7) {
		tick = 0;
		loopcount++;
	}
}

void start_alarm() {
	PORTC |= BUZZER;  // Accende il buzzer
	PORTC |= (l0 | l1 | l2 | l3);  // Accende tutti i LED per segnalare l'allarme
	if (tick % 2 == 0) {
		PORTC |= l5;  // Lampeggia il LED L5 per l'allarme
		} else {
		PORTC &=~ l5;
	}
}

void stop_alarm() {
	PORTC &=~ (BUZZER | l0 | l1 | l2 | l3);  // Spegne il buzzer e i LED
}

ISR(PCINT2_vect) {
	uint8_t change = oldval ^ PIND;
	oldval = PIND;
	for (uint8_t i = PIND2; i <= PIND7; i++) {
		if ((change & (1 << i)) && !(PIND & (1 << i))) {
			pulsantepremuto = (1 << i);
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	tick++;
}
