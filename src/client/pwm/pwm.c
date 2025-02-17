/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <_sequencer_generated.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <gpio/gpio.h>
#include <pwm/pwm.h>
#include <pwm/sequencer.h>
#include <stdint.h>



#if (F_CPU/8000000)&(F_CPU/8000000-1)
#error Timer ticks per us is not a power of 2
#endif

#define PWM_TIMER_TICKS_PER_US_SHIFT (__builtin_ffs(F_CPU/8000000)-1)

#define PWM_MIN_PERIOD_US 20000
#define PWM_MIN_PULSE_US 4
#define PWM_MAX_PULSE_US ((1<<((sizeof(_pwm_rr_scheduler_entries[0])<<3)-PWM_ENTRY_PIN_BIT_COUNT))-1)

#define PWM_MAX_ENTRY_COUNT 8
#define PWM_ENTRY_PIN_BIT_COUNT 4

#define PWM_SEQUENCER_SCRTACH_BUFFER_SIZE 8



static uint16_t __attribute__((section(".bss"))) _pwm_rr_scheduler_entries[PWM_MAX_ENTRY_COUNT+1];
static uint16_t _pwm_rr_scheduler_start_time=0;
static pwm_t _pwm_rr_scheduler_entry_index=0;
static uint8_t _pwm_sequencer_running=0;
static uint8_t _pwm_sequencer_channel_count;
static uint16_t _pwm_sequencer_sample_count;
static uint16_t _pwm_sequencer_sample_index;
static uint16_t _pwm_sequencer_data_index;
static uint8_t _pwm_sequencer_scratch_buffer[PWM_SEQUENCER_SCRTACH_BUFFER_SIZE];



static inline void _update_entry_pulse_width(uint8_t i,uint16_t pulse){
	_pwm_rr_scheduler_entries[i]=(_pwm_rr_scheduler_entries[i]&((1<<PWM_ENTRY_PIN_BIT_COUNT)-1))|pulse;
}



ISR(TIMER1_COMPA_vect){
	uint16_t entry=_pwm_rr_scheduler_entries[_pwm_rr_scheduler_entry_index];
	if (entry){
		gpio_write(entry&((1<<PWM_ENTRY_PIN_BIT_COUNT)-1),0);
		_pwm_rr_scheduler_entry_index++;
	}
	else{
_reset_rr_scheduler:
		_pwm_rr_scheduler_start_time=TCNT1;
		_pwm_rr_scheduler_entry_index=0;
	}
	entry=_pwm_rr_scheduler_entries[_pwm_rr_scheduler_entry_index];
	if (entry){
		gpio_write(entry&((1<<PWM_ENTRY_PIN_BIT_COUNT)-1),1);
		OCR1A=TCNT1+(entry>>(PWM_ENTRY_PIN_BIT_COUNT-PWM_TIMER_TICKS_PER_US_SHIFT));
	}
	else if (TCNT1-_pwm_rr_scheduler_start_time<(PWM_MIN_PERIOD_US<<PWM_TIMER_TICKS_PER_US_SHIFT)){
		OCR1A=_pwm_rr_scheduler_start_time+((PWM_MIN_PERIOD_US+PWM_MIN_PULSE_US)<<PWM_TIMER_TICKS_PER_US_SHIFT);
	}
	else{
		goto _reset_rr_scheduler;
	}
}



ISR(TIMER1_OVF_vect){
	uint8_t i=2;
	for (uint8_t j=0;j<_pwm_sequencer_channel_count;j+=4){
		if (_pwm_sequencer_scratch_buffer[j+1]){
			_pwm_sequencer_scratch_buffer[j+1]--;
			if (!_pwm_sequencer_scratch_buffer[j+2]){
				i=_pwm_sequencer_scratch_buffer[j];
				continue;
			}
		}
		else{
			uint8_t token=ROM_LOAD_U8(sequencer_generated_data+_pwm_sequencer_data_index);
			_pwm_sequencer_data_index++;
			if (token&1){
				_pwm_sequencer_scratch_buffer[j+1]=token>>1;
			}
			else{
				_pwm_sequencer_scratch_buffer[j+2]=((int8_t)token)>>1;
			}
		}
		_pwm_sequencer_scratch_buffer[j+3]+=((int8_t)(_pwm_sequencer_scratch_buffer[j+2]));
		uint16_t pulse=(_pwm_sequencer_scratch_buffer[j+3]*PWM_SEQUENCER_PULSE_ENCODING_FACTOR)<<PWM_ENTRY_PIN_BIT_COUNT;
		for (uint16_t k=_pwm_sequencer_scratch_buffer[j];i<k;i++){
			uint8_t k=ROM_LOAD_U8(sequencer_generated_data+i);
			if (k&PWM_SEQUENCER_PIN_FLAG_INVERTED){
				pulse=(((255+PWM_SEQUENCER_PULSE_ENCODING_CUTOFF+1)*PWM_SEQUENCER_PULSE_ENCODING_FACTOR)<<PWM_ENTRY_PIN_BIT_COUNT)-pulse;
			}
			_update_entry_pulse_width(k&PWM_SEQUENCER_PIN_MASK,pulse);
		}
	}
	_pwm_sequencer_sample_index++;
	if (_pwm_sequencer_sample_index>=_pwm_sequencer_sample_count){
		_pwm_sequencer_running=0;
		TIMSK1&=~(1<<TOIE1);
	}
}



void pwm_init(void){
	TCCR1A=0;
	TCCR1B=1<<CS11; // divide by 8
	TCNT1=0;
	TIFR1=(1<<TOV1)|(1<<OCF1A)|(1<<OCF1B)|(1<<ICF1);
	OCR1A=PWM_MIN_PERIOD_US;
	TIMSK1=1<<OCIE1A;
}



pwm_t pwm_alloc(uint8_t pin){
	pwm_t out=0;
	for (;_pwm_rr_scheduler_entries[out];out++);
	if (out==PWM_MAX_ENTRY_COUNT){
		for (;;);
	}
	pin&=(1<<PWM_ENTRY_PIN_BIT_COUNT)-1;
	gpio_init(pin,1);
	gpio_write(pin,0);
	_pwm_rr_scheduler_entries[out]=pin|(PWM_MIN_PULSE_US<<PWM_ENTRY_PIN_BIT_COUNT);
	return out;
}



void pwm_set_pulse_width_us(pwm_t index,uint16_t us){
	if (_pwm_sequencer_running){
		return;
	}
	_update_entry_pulse_width(index,(us<PWM_MIN_PULSE_US?PWM_MIN_PULSE_US:(us>PWM_MAX_PULSE_US?PWM_MAX_PULSE_US:us))<<PWM_ENTRY_PIN_BIT_COUNT);
}



void pwm_sequencer_start(void){
	TIMSK1&=~(1<<TOIE1);
	_pwm_sequencer_running=1;
	_pwm_sequencer_channel_count=ROM_LOAD_U8(sequencer_generated_data);
	_pwm_sequencer_sample_count=ROM_LOAD_U8(sequencer_generated_data+1)|((_pwm_sequencer_channel_count&0xf8)<<5);
	_pwm_sequencer_channel_count=(_pwm_sequencer_channel_count&7)<<2;
	_pwm_sequencer_sample_index=0;
	_pwm_sequencer_data_index=2;
	for (uint8_t i=0;i<_pwm_sequencer_channel_count;i+=4){
		for (_pwm_sequencer_data_index++;!(ROM_LOAD_U8(sequencer_generated_data+_pwm_sequencer_data_index-1)&PWM_SEQUENCER_PIN_FLAG_LAST);_pwm_sequencer_data_index++);
		_pwm_sequencer_scratch_buffer[i]=_pwm_sequencer_data_index;
		_pwm_sequencer_scratch_buffer[i+1]=0;
		_pwm_sequencer_scratch_buffer[i+2]=0;
		_pwm_sequencer_scratch_buffer[i+3]=0;
	}
	TIMSK1|=1<<TOIE1;
}
