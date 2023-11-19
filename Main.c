/*
 * Miniproject2.c
 *
 *  Created on: Sep 16, 2023
 *      Author: hp
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//use bit fields to optimize the memory using
union {
	struct {
		char ones :4;
		char tens :4;
	} seconds;
} sec;
union {
	struct {
		char ones :4;
		char tens :4;
	} minutes;
} min;
union {
	struct {
		char ones :4;
		char tens :4;
	} hours;
} hour;
//set up interrupts settings
void INT_enable(void) {
	//choose falling edge for INT0 & INT2 and rising edge for INT1
	MCUCR |= (1 << ISC01) | (1 << ISC11) | (1 << ISC10);
	MCUCSR &= ~(1 << ISC2);
	//make the pins inputs
	DDRD &= ~((1 << 2) | (1 << 3));
	DDRB &= ~(1 << 2);
	//Enable internal pull-up
	PORTB |= (1 << 2);
	PORTD |= (1 << 2);
	//Enable module interrupt
	GICR |= (1 << INT0) | (1 << INT1) | (1 << INT2);
}
//set up Timer 1 settings
void Timer1_comp(void) {
	//As we don't use PWM mode, set this bit = 1
	TCCR1A = (1 << FOC1A);
	TCNT1 = 0;
	//setting the compare register value
	OCR1A = 3700;
	//Enable the module interrupt
	TIMSK |= (1 << OCIE1A);
	//choose the CTC mode and prescaler = 256
	TCCR1B = (1 << WGM12) | (1 << CS12);
}
ISR (TIMER1_COMPA_vect) {
	sec.seconds.ones++;
}
//if the button is pushed, reset the timer
ISR (INT0_vect) {
	sec.seconds.ones = 0;
	min.minutes.ones = 0;
	hour.hours.ones = 0;
	sec.seconds.tens = 0;
	min.minutes.tens = 0;
	hour.hours.tens = 0;
}
//disable the timer if the button is pushed
ISR (INT1_vect) {
	TCCR1B &= ~(1 << CS12);
}
//re-enable the timer if the button is pushed
ISR (INT2_vect) {
	TCCR1B |= (1 << CS12);
}

int main(void) {
	//PORTA & C are output
	DDRC |= 0x0F;
	DDRA |= 0x3F;
	//Put an initial value = 0
	PORTC &= ~(0x0F);
	PORTA &= ~(0x3F);
	INT_enable();
	Timer1_comp();
	SREG |= (1 << 7);
	while (1) {
		if (sec.seconds.ones > 9) {
			sec.seconds.ones = 0;
			sec.seconds.tens++;
		}
		if (sec.seconds.tens > 5) {
			sec.seconds.tens = 0;
			min.minutes.ones++;
		}
		if (min.minutes.ones > 9) {
			min.minutes.tens++;
			min.minutes.ones = 0;
		}
		if (min.minutes.tens > 5) {
			hour.hours.ones++;
			min.minutes.tens = 0;
		}
		if (hour.hours.ones > 9) {
			hour.hours.tens++;
			hour.hours.ones = 0;
		}
		//First, display ones of the seconds value ,delay 50 micro seconds then display the tens of the seconds value and so on
		//return to the initial value of PORTA to enable just the targeted 7-segmant
		PORTA &= ~(0x3F);
		//enable first 7-segmant
		PORTA |= 1;
		//insert the ones of the seconds value to first 4 bits of PORTC
		PORTC = ((PORTC & 0xF0) | (sec.seconds.ones & 0x0F));
		//delay 50 micro seconds before displaying the next value
		_delay_us(50);
		//same steps which explained above
		PORTA &= ~(0x3F);
		PORTA |= (1 << 1);
		PORTC = ((PORTC & 0xF0) | (sec.seconds.tens & 0x0F));
		_delay_us(50);
		PORTA &= ~(0x3F);
		PORTA |= (1 << 2);
		PORTC = ((PORTC & 0xF0) | (min.minutes.ones & 0x0F));
		_delay_us(50);
		PORTA &= ~(0x3F);
		PORTA |= (1 << 3);
		PORTC = ((PORTC & 0xF0) | (min.minutes.tens & 0x0F));
		_delay_us(50);
		PORTA &= ~(0x3F);
		PORTA |= (1 << 4);
		PORTC = ((PORTC & 0xF0) | (hour.hours.ones & 0x0F));
		_delay_us(50);
		PORTA &= ~(0x3F);
		PORTA |= (1 << 5);
		PORTC = ((PORTC & 0xF0) | (hour.hours.tens & 0x0F));
		_delay_us(50);
	}
	return 0;
}

