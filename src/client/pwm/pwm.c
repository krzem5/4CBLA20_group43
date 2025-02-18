/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <_sequencer_generated.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <pwm/pwm.h>
#include <pwm/sequencer.h>
#include <stdint.h>



#if (F_CPU/8000000)&(F_CPU/8000000-1)
#error Timer ticks per us is not a power of 2
#endif

#define PWM_TIMER_TICKS_PER_US_SHIFT (__builtin_ffs(F_CPU/8000000)-1)

#define PWM_MIN_PERIOD_US 20000
#define PWM_MIN_PULSE_US 4
#define PWM_MAX_PULSE_US (0xffff>>PWM_TIMER_TICKS_PER_US_SHIFT)

#define PWM_SEQUENCER_SCRTACH_BUFFER_SIZE 8

#define PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB 1
#define PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC 2



static uint16_t _pwm_pin_entries[12];

static uint8_t _pwm_parallel_scheduler_update_flags=0;

static uint8_t _pwm_parallel_scheduler_portb_value_entries[7];
static uint16_t _pwm_parallel_scheduler_portb_delta_entries[7];
static uint8_t _pwm_parallel_scheduler_portb_entry_index=0;
static uint8_t _pwm_parallel_scheduler_portb_entry_count;

static uint8_t _pwm_parallel_scheduler_portc_value_entries[7];
static uint16_t _pwm_parallel_scheduler_portc_delta_entries[7];
static uint8_t _pwm_parallel_scheduler_portc_entry_index=0;
static uint8_t _pwm_parallel_scheduler_portc_entry_count;

static uint8_t _pwm_sequencer_running=0;
static uint8_t _pwm_sequencer_channel_count;
static uint16_t _pwm_sequencer_sample_count;
static uint16_t _pwm_sequencer_sample_index;
static uint16_t _pwm_sequencer_data_index;
static uint8_t _pwm_sequencer_scratch_buffer[PWM_SEQUENCER_SCRTACH_BUFFER_SIZE];



static inline void _update_pin_pulse(uint8_t pin,uint16_t pulse){
	if (_pwm_pin_entries[pin]==pulse){
		return;
	}
	_pwm_pin_entries[pin]=pulse;
	_pwm_parallel_scheduler_update_flags|=(pin>5?PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC:PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB);
}



static uint8_t _recompute_scheduler(const uint16_t* entries,uint8_t* values,uint16_t* deltas){
	values[0]=0;
	for (uint8_t i=0;i<6;i++){
		if (entries[i]){
			values[0]|=1<<i;
		}
	}
	uint16_t pulse=0;
	uint8_t i=1;
	for (;values[i-1];i++){
		uint8_t j=0;
		for (;entries[j]<=pulse;j++);
		uint16_t next=entries[j];
		uint8_t mask=1<<j;
		for (j++;j<6;j++){
			if (entries[j]==next){
				mask|=1<<j;
			}
			else if (entries[j]>pulse&&entries[j]<next){
				next=entries[j];
				mask=1<<j;
			}
		}
		uint16_t delta=next-pulse;
		pulse=next;
		values[i]=values[i-1]&(~mask);
		deltas[i-1]=(delta<(PWM_MIN_PULSE_US<<PWM_TIMER_TICKS_PER_US_SHIFT)?PWM_MIN_PULSE_US<<PWM_TIMER_TICKS_PER_US_SHIFT:delta);
	}
	deltas[i-1]=(pulse<((PWM_MIN_PERIOD_US-PWM_MIN_PULSE_US)<<PWM_TIMER_TICKS_PER_US_SHIFT)?(PWM_MIN_PERIOD_US<<PWM_TIMER_TICKS_PER_US_SHIFT)-pulse:PWM_MIN_PULSE_US<<PWM_TIMER_TICKS_PER_US_SHIFT);
	return i;
}



ISR(TIMER1_COMPA_vect){
	if (_pwm_parallel_scheduler_portb_entry_index==_pwm_parallel_scheduler_portb_entry_count){
		_pwm_parallel_scheduler_portb_entry_index=0;
		if (__builtin_expect(_pwm_parallel_scheduler_update_flags&PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB,0)){
			_pwm_parallel_scheduler_update_flags&=~PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB;
			_pwm_parallel_scheduler_portb_entry_count=_recompute_scheduler(_pwm_pin_entries,_pwm_parallel_scheduler_portb_value_entries,_pwm_parallel_scheduler_portb_delta_entries);
		}
	}
	uint8_t value=_pwm_parallel_scheduler_portb_value_entries[_pwm_parallel_scheduler_portb_entry_index];
	uint16_t delta=_pwm_parallel_scheduler_portb_delta_entries[_pwm_parallel_scheduler_portb_entry_index];
	_pwm_parallel_scheduler_portb_entry_index++;
	PORTB=value;
	OCR1A=TCNT1+delta;
}



ISR(TIMER1_COMPB_vect){
	if (_pwm_parallel_scheduler_portc_entry_index==_pwm_parallel_scheduler_portc_entry_count){
		_pwm_parallel_scheduler_portc_entry_index=0;
		if (__builtin_expect(_pwm_parallel_scheduler_update_flags&PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC,0)){
			_pwm_parallel_scheduler_update_flags&=~PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC;
			_pwm_parallel_scheduler_portc_entry_count=_recompute_scheduler(_pwm_pin_entries+6,_pwm_parallel_scheduler_portc_value_entries,_pwm_parallel_scheduler_portc_delta_entries);
		}
	}
	uint8_t value=_pwm_parallel_scheduler_portc_value_entries[_pwm_parallel_scheduler_portc_entry_index];
	uint16_t delta=_pwm_parallel_scheduler_portc_delta_entries[_pwm_parallel_scheduler_portc_entry_index];
	_pwm_parallel_scheduler_portc_entry_index++;
	PORTC=value;
	OCR1B=TCNT1+delta;
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
			if (!(token&1)){
				_pwm_sequencer_scratch_buffer[j+1]=token>>1;
				_pwm_sequencer_scratch_buffer[j+2]=0;
				i=_pwm_sequencer_scratch_buffer[j];
				continue;
			}
			_pwm_sequencer_scratch_buffer[j+1]=token>>6;
			_pwm_sequencer_scratch_buffer[j+2]=((token>>2)&15)+1;
			if (token&2){
				_pwm_sequencer_scratch_buffer[j+2]=-_pwm_sequencer_scratch_buffer[j+2];
			}
		}
		_pwm_sequencer_scratch_buffer[j+3]+=((int8_t)(_pwm_sequencer_scratch_buffer[j+2]));
		uint16_t pulse=(_pwm_sequencer_scratch_buffer[j+3]*PWM_SEQUENCER_PULSE_ENCODING_FACTOR)<<PWM_TIMER_TICKS_PER_US_SHIFT;
		for (uint16_t k=_pwm_sequencer_scratch_buffer[j];i<k;i++){
			uint8_t k=ROM_LOAD_U8(sequencer_generated_data+i);
			if (k&PWM_SEQUENCER_PIN_FLAG_INVERTED){
				pulse=(((255+PWM_SEQUENCER_PULSE_ENCODING_CUTOFF+1)*PWM_SEQUENCER_PULSE_ENCODING_FACTOR)<<PWM_TIMER_TICKS_PER_US_SHIFT)-pulse;
			}
			_update_pin_pulse(k&PWM_SEQUENCER_PIN_MASK,pulse);
		}
	}
	_pwm_sequencer_sample_index++;
	if (_pwm_sequencer_sample_index>=_pwm_sequencer_sample_count){
		_pwm_sequencer_running=0;
		TIMSK1&=~(1<<TOIE1);
	}
}



void pwm_init(void){
	for (uint8_t i=0;i<12;i++){
		_pwm_pin_entries[i]=PWM_MIN_PULSE_US<<PWM_TIMER_TICKS_PER_US_SHIFT;
	}
	_pwm_parallel_scheduler_portb_value_entries[0]=0;
	_pwm_parallel_scheduler_portb_delta_entries[0]=PWM_MIN_PERIOD_US<<PWM_TIMER_TICKS_PER_US_SHIFT;
	_pwm_parallel_scheduler_portb_entry_count=1;
	_pwm_parallel_scheduler_portc_value_entries[0]=0;
	_pwm_parallel_scheduler_portc_delta_entries[0]=PWM_MIN_PERIOD_US<<PWM_TIMER_TICKS_PER_US_SHIFT;
	_pwm_parallel_scheduler_portc_entry_count=1;
	ADMUX=0;
	ADCSRA=0;
	DIDR0=0;
	DDRB=0x3f;
	DDRC=0x3f;
	PORTB=0;
	PORTC=0;
	TCCR1A=0;
	TCCR1B=1<<CS11; // divide by 8
	TCNT1=0;
	TIFR1=(1<<TOV1)|(1<<OCF1A)|(1<<OCF1B)|(1<<ICF1);
	OCR1A=PWM_MIN_PERIOD_US<<PWM_TIMER_TICKS_PER_US_SHIFT;
	OCR1B=PWM_MIN_PERIOD_US<<PWM_TIMER_TICKS_PER_US_SHIFT;
	TIMSK1=(1<<OCIE1A)|(1<<OCIE1B);
}



void pwm_set_pulse_width_us(uint8_t pin,uint16_t pulse){
	if (_pwm_sequencer_running){
		return;
	}
	_update_pin_pulse(pin,(pulse<PWM_MIN_PULSE_US?PWM_MIN_PULSE_US:(pulse>PWM_MAX_PULSE_US?PWM_MAX_PULSE_US:pulse))<<PWM_TIMER_TICKS_PER_US_SHIFT);
}



void pwm_sequencer_start(void){
	if (_pwm_sequencer_running){
		return;
	}
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
