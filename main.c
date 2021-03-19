#define F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

#define OCR0A_SLOW_TICK 61 
#define SLOWTICKS_PER_SECOND 64 

#define MODE_NIGHTTIME_MODE 1
#define MODE_WAKEUP_MODE 2
#define MODE_DAYTIME_MODE 3
#define MODE_WINDDOWN_MODE 4

#define WAKEUPTIME 21600
#define WINDDOWNTIME 79200 

int slow_tick_count = 0;
uint8_t mode = MODE_DAYTIME_MODE;
uint32_t seconds = 79200; //22:00
uint32_t oldSeconds = 0;
uint8_t _ocr1b = 0; //ocr follower so I can read from it
uint8_t _ocr0a = OCR0A_SLOW_TICK;

void enable_compb(){
	cli();
	TCCR1 = (1 << CS12); //set to zero, then clk/8 prescale. PWM mode clear OC1B on compare match
	GTCCR = (1 << COM1B1);
	TIMSK |= (1 << TOIE1);
	sei();	
}

void disable_compb(){
	cli();
	TCCR1 = 0;
	TIMSK &= ~(1 << OCIE1B);
	sei();
}

void tick_second(){
	seconds++;
	if (seconds > 86400) seconds = 0;
}

int toggle = 0;
void toggle_debug(){
	if (toggle){
		PORTB |= (1 << PORTB4);
	} else {
		PORTB &= ~(1 << PORTB4);
	}
	toggle = ~toggle;
}

int main(){
	cli();
	TCCR0A = 0;
	TCCR0B = 0;
	TCCR0B |= 1 << CS02; //256 prescaler
	OCR0A = _ocr0a;	
	TIMSK |= 1 << OCIE0A | 1 << TOIE0; //OCR0A compare match interrupt
	sei();

	DDRB = (1 << DDB3) | 1 << DDB4;

	while(1){
		switch(mode){
			case MODE_NIGHTTIME_MODE:
				if ((seconds < WAKEUPTIME + 500) && seconds >= WAKEUPTIME){
					mode = MODE_WAKEUP_MODE;
					enable_compb();
					OCR1B = _ocr1b = 0;
				}
			break;
			case MODE_WAKEUP_MODE:
				if (seconds != oldSeconds){
					OCR1B = ++_ocr1b;
					oldSeconds = seconds;
				}
				if (_ocr1b == 256 ){
					mode = MODE_DAYTIME_MODE;
					disable_compb();
				}
				break;
			case MODE_DAYTIME_MODE:
				OCR1B = 0;
				if (seconds >= WINDDOWNTIME){
					OCR1B = _ocr1b = 255;
					enable_compb();
					mode = MODE_WINDDOWN_MODE;
				}
				break;
			case MODE_WINDDOWN_MODE:
				if (seconds != oldSeconds){
					OCR1B = --_ocr1b;
					oldSeconds = seconds;
					toggle_debug();
				}
				if (_ocr1b == 0){
					mode = MODE_NIGHTTIME_MODE;
					disable_compb();
				}
				break;
		}
		if (slow_tick_count == SLOWTICKS_PER_SECOND){
			slow_tick_count = 0;
			tick_second();
		}
	}
}

ISR(TIMER0_COMPA_vect){
	slow_tick_count++;
	TCNT0 = 0;
}

ISR(TIMER1_OVF_vect){
	//toggle LED pin high, hardware controls the toggle low
	PORTB |= (1 << PORTB3);
}
