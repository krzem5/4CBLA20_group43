/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <pwm/pwm.h>
#include <stdint.h>



#define PWM_WINDOW_US 16000
#define PWM_MIN_PULSE_US 4
#define PWM_MAX_PULSE_US (0xffff>>PWM_PIN_BIT_COUNT)

#define PWM_MAX_PIN_COUNT 8
#define PWM_PIN_BIT_COUNT 4



static uint16_t _pwm_pin_data[PWM_MAX_PIN_COUNT+1];
static pwm_t _pwm_pin_index=0;



SIGNAL(TIMER1_COMPA_vect){
	if (_pwm_pin_data[_pwm_pin_index]){
		digitalWrite(_pwm_pin_data[_pwm_pin_index]&((1<<PWM_PIN_BIT_COUNT)-1),0);
		_pwm_pin_index++;
	}
	else{
		TCNT1=0;
		_pwm_pin_index=0;
	}
	if (_pwm_pin_data[_pwm_pin_index]){
		digitalWrite(_pwm_pin_data[_pwm_pin_index]&((1<<PWM_PIN_BIT_COUNT)-1),1);
		OCR1A=TCNT1+(_pwm_pin_data[_pwm_pin_index]>>4);
	}
	else{
		OCR1A=(TCNT1+PWM_MIN_PULSE_US<PWM_WINDOW_US?PWM_WINDOW_US:TCNT1+PWM_MIN_PULSE_US);
	}
}



void pwm_init(void){
	for (pwm_t i=0;i<PWM_MAX_PIN_COUNT+1;i++){
		_pwm_pin_data[i]=0;
	}
	TCCR1A=0;
	TCCR1B=1<<CS11; // divide 8 MHz clock by 8 to obtain 1 us tick duration
	TCNT1=0;
	TIFR1=1<<OCF1A;
	OCR1A=PWM_WINDOW_US;
	TIMSK1=1<<OCIE1A;
}



pwm_t pwm_alloc(uint8_t pin){
	pwm_t out=0;
	for (;_pwm_pin_data[out];out++);
	if (out==PWM_MAX_PIN_COUNT){
		return PWM_NO_MORE_PINS;
	}
	pin&=(1<<PWM_PIN_BIT_COUNT)-1;
	pinMode(pin,OUTPUT);
	_pwm_pin_data[out]=pin|(PWM_MIN_PULSE_US<<PWM_PIN_BIT_COUNT);
	return out;
}



void pwm_set_pulse_width_us(pwm_t index,uint16_t us){
	_pwm_pin_data[index]=(_pwm_pin_data[index]&((1<<PWM_PIN_BIT_COUNT)-1))|((us<PWM_MIN_PULSE_US?PWM_MIN_PULSE_US:(us>PWM_MAX_PULSE_US?PWM_MAX_PULSE_US:us))<<PWM_PIN_BIT_COUNT);
}
