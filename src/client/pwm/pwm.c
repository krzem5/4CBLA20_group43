/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <pwm/pwm.h>
#include <stdint.h>



#define PWM_TICKS_PER_US_SHIFT (__builtin_ffs(F_CPU/8000000)-1)

#define PWM_MIN_PERIOD_US 20000
#define PWM_MIN_PULSE_US 2
#define PWM_MAX_PULSE_US (0xffff>>PWM_PIN_BIT_COUNT)

#define PWM_MAX_PIN_COUNT 8
#define PWM_PIN_BIT_COUNT 4



static volatile uint16_t __attribute__((section(".bss"))) _pwm_data[PWM_MAX_PIN_COUNT+1];
static pwm_t _pwm_entry_index=0;



SIGNAL(TIMER1_COMPA_vect){
	uint16_t entry=_pwm_data[_pwm_entry_index];
	if (entry){
		digitalWrite(entry&((1<<PWM_PIN_BIT_COUNT)-1),0);
		_pwm_entry_index++;
	}
	else{
_next_pin:
		TCNT1=0;
		_pwm_entry_index=0;
	}
	entry=_pwm_data[_pwm_entry_index];
	if (entry){
		digitalWrite(entry&((1<<PWM_PIN_BIT_COUNT)-1),1);
		OCR1A=TCNT1+((entry>>4)<<PWM_TICKS_PER_US_SHIFT);
	}
	else if (TCNT1<(PWM_MIN_PERIOD_US<<PWM_TICKS_PER_US_SHIFT)){
		OCR1A=(PWM_MIN_PERIOD_US+PWM_MIN_PULSE_US)<<PWM_TICKS_PER_US_SHIFT;
	}
	else{
		goto _next_pin;
	}
}



void pwm_init(void){
	TCCR1A=0;
	TCCR1B=1<<CS11; // divide by 8
	TCNT1=0;
	TIFR1=1<<OCF1A;
	OCR1A=PWM_MIN_PERIOD_US;
	TIMSK1=1<<OCIE1A;
}



pwm_t pwm_alloc(uint8_t pin){
	pwm_t out=0;
	for (;_pwm_data[out];out++);
	if (out==PWM_MAX_PIN_COUNT){
		for (;;);
	}
	pin&=(1<<PWM_PIN_BIT_COUNT)-1;
	pinMode(pin,OUTPUT);
	_pwm_data[out]=pin|(PWM_MIN_PULSE_US<<PWM_PIN_BIT_COUNT);
	return out;
}



void pwm_set_pulse_width_us(pwm_t index,uint16_t us){
	_pwm_data[index]=(_pwm_data[index]&((1<<PWM_PIN_BIT_COUNT)-1))|((us<PWM_MIN_PULSE_US?PWM_MIN_PULSE_US:(us>PWM_MAX_PULSE_US?PWM_MAX_PULSE_US:us))<<PWM_PIN_BIT_COUNT);
}
