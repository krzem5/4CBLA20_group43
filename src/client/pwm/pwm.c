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
#define PWM_MIN_PULSE_US 4
#define PWM_MAX_PULSE_US ((1<<((sizeof(_pwm_data[0])<<3)-PWM_PIN_BIT_COUNT))-1)

#define PWM_MAX_PIN_COUNT 8
#define PWM_PIN_BIT_COUNT 4



static volatile uint16_t __attribute__((section(".bss"))) _pwm_data[PWM_MAX_PIN_COUNT+1];
static pwm_t _pwm_rr_scheduler_entry_index=0;



static inline void _fast_digital_pin_write(uint8_t pin,_Bool high){
	if (high){
		(*portOutputRegister(digitalPinToPort(pin)))|=digitalPinToBitMask(pin);
	}
	else{
		(*portOutputRegister(digitalPinToPort(pin)))&=~digitalPinToBitMask(pin);
	}
}



ISR(TIMER1_COMPA_vect){
	uint16_t entry=_pwm_data[_pwm_rr_scheduler_entry_index];
	if (entry){
		_fast_digital_pin_write(entry&((1<<PWM_PIN_BIT_COUNT)-1),0);
		_pwm_rr_scheduler_entry_index++;
	}
	else{
_reset_rr_scheduler:
		TCNT1=0;
		_pwm_rr_scheduler_entry_index=0;
	}
	entry=_pwm_data[_pwm_rr_scheduler_entry_index];
	if (entry){
		_fast_digital_pin_write(entry&((1<<PWM_PIN_BIT_COUNT)-1),1);
		OCR1A=TCNT1+((entry>>4)<<PWM_TICKS_PER_US_SHIFT);
	}
	else if (TCNT1<(PWM_MIN_PERIOD_US<<PWM_TICKS_PER_US_SHIFT)){
		OCR1A=(PWM_MIN_PERIOD_US+PWM_MIN_PULSE_US)<<PWM_TICKS_PER_US_SHIFT;
	}
	else{
		goto _reset_rr_scheduler;
	}
}



void pwm_init(void){
	TCCR1A=0;
	TCCR1B=1<<CS11; // divide by 8 (see p110 of the datasheet)
	TCNT1=0;
	TIFR1=(1<<TOV1)|(1<<OCF1A)|(1<<OCF1B)|(1<<ICF1);
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
	uint16_t value=(_pwm_data[index]&((1<<PWM_PIN_BIT_COUNT)-1))|((us<PWM_MIN_PULSE_US?PWM_MIN_PULSE_US:(us>PWM_MAX_PULSE_US?PWM_MAX_PULSE_US:us))<<PWM_PIN_BIT_COUNT);
	uint8_t status_reg_value=SREG;
	cli();
	_pwm_data[index]=value;
	SREG=status_reg_value;
}
