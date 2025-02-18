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



#define PWM_TIMER_TICKS_PER_US (F_CPU/8000000)

#define PWM_MIN_PULSE_US 4
#define PWM_WINDOW_US 20000

#define PWM_SEQUENCER_SCRTACH_BUFFER_SIZE 12

#define PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB 1
#define PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC 2



static uint16_t __attribute__((section(".bss"))) _pwm_pin_entries[12];

static uint8_t _pwm_parallel_scheduler_update_flags=0;

static uint8_t _pwm_parallel_scheduler_portb_value_entries[7];
static uint16_t _pwm_parallel_scheduler_portb_delta_entries[7];
static uint8_t _pwm_parallel_scheduler_portb_entry_index=0;
static uint8_t _pwm_parallel_scheduler_portb_entry_count=0;

static uint8_t _pwm_parallel_scheduler_portc_value_entries[7];
static uint16_t _pwm_parallel_scheduler_portc_delta_entries[7];
static uint8_t _pwm_parallel_scheduler_portc_entry_index=0;
static uint8_t _pwm_parallel_scheduler_portc_entry_count=0;

static uint8_t _pwm_sequencer_running=0;
static uint8_t _pwm_sequencer_channel_count;
static uint16_t _pwm_sequencer_sample_count;
static uint16_t _pwm_sequencer_sample_index;
static uint16_t _pwm_sequencer_data_index;
static uint8_t _pwm_sequencer_scratch_buffer[PWM_SEQUENCER_SCRTACH_BUFFER_SIZE];



static uint8_t _recompute_scheduler(const uint16_t* entries,uint8_t* values,uint16_t* deltas){
	values[0]=0x3f;
	uint8_t mask=0x3f;
	uint16_t min=0;
	uint16_t pulse=0;
	uint8_t i=0;
	do{
		uint8_t j=0;
		for (;entries[j]<min;j++);
		uint16_t next=entries[j];
		uint8_t clear_mask=1<<j;
		for (j++;j<6;j++){
			if (entries[j]==next){
				clear_mask|=1<<j;
			}
			else if (entries[j]>=min&&entries[j]<next){
				next=entries[j];
				clear_mask=1<<j;
			}
		}
		mask&=~clear_mask;
		min=next+1;
		uint16_t delta=next-pulse;
		if (delta>=PWM_MIN_PULSE_US*PWM_TIMER_TICKS_PER_US){
			pulse=next;
			deltas[i]=delta;
			i++;
		}
		values[i]=mask;
	} while (mask);
	deltas[i]=(PWM_WINDOW_US+PWM_MIN_PULSE_US)*PWM_TIMER_TICKS_PER_US-pulse;
	return i+1;
}



ISR(TIMER1_COMPA_vect){
	if (__builtin_expect(_pwm_parallel_scheduler_portb_entry_index==_pwm_parallel_scheduler_portb_entry_count,0)){
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
	if (__builtin_expect(_pwm_parallel_scheduler_portc_entry_index==_pwm_parallel_scheduler_portc_entry_count,0)){
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
	uint8_t* ptr=_pwm_sequencer_scratch_buffer;
	for (uint8_t i=0;i<_pwm_sequencer_channel_count;i++){
		if (ptr[0]){
			ptr[0]--;
			if (!ptr[1]){
				goto _next_channel;
			}
		}
		else{
			uint8_t token=ROM_LOAD_U8(sequencer_generated_data+_pwm_sequencer_data_index);
			_pwm_sequencer_data_index++;
			if (!(token&1)){
				ptr[0]=token>>1;
				ptr[1]=0;
				goto _next_channel;
			}
			ptr[0]=token>>6;
			ptr[1]=((token>>2)&15)+1;
			if (token&2){
				ptr[1]=-ptr[1];
			}
		}
		ptr[2]+=(int8_t)(ptr[1]);
		uint16_t pulse=ptr[2]*PWM_SEQUENCER_PULSE_ENCODING_FACTOR*PWM_TIMER_TICKS_PER_US;
		uint8_t pins=ROM_LOAD_U8(sequencer_generated_data+i+2);
		_pwm_pin_entries[pins&15]=pulse;
		pins>>=4;
		if (pins!=15){
			_pwm_pin_entries[pins]=(255+PWM_SEQUENCER_PULSE_ENCODING_MIN_PULSE)*PWM_SEQUENCER_PULSE_ENCODING_FACTOR*PWM_TIMER_TICKS_PER_US-pulse;
		}
_next_channel:
		ptr+=3;
	}
	_pwm_parallel_scheduler_update_flags=PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB|PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC;
	_pwm_sequencer_sample_index++;
	if (_pwm_sequencer_sample_index>=_pwm_sequencer_sample_count){
		_pwm_sequencer_running=0;
		TIMSK1&=~(1<<TOIE1);
	}
}



void pwm_init(void){
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
	OCR1A=PWM_MIN_PULSE_US*PWM_TIMER_TICKS_PER_US;
	OCR1B=PWM_MIN_PULSE_US*PWM_TIMER_TICKS_PER_US;
	TIMSK1=(1<<OCIE1A)|(1<<OCIE1B);
}



void pwm_set_pulse_width_us(uint8_t pin,uint16_t pulse){
	if (_pwm_sequencer_running){
		return;
	}
	pulse=(pulse>PWM_WINDOW_US?PWM_WINDOW_US:pulse)*PWM_TIMER_TICKS_PER_US;
	if (_pwm_pin_entries[pin]==pulse){
		return;
	}
	_pwm_pin_entries[pin]=pulse;
	_pwm_parallel_scheduler_update_flags|=(pin>5?PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTC:PWM_PARALLEL_SCHEDULER_UPDATE_FLAG_PORTB);
}



void pwm_sequencer_start(void){
	if (_pwm_sequencer_running){
		return;
	}
	_pwm_sequencer_running=1;
	_pwm_sequencer_channel_count=ROM_LOAD_U8(sequencer_generated_data);
	_pwm_sequencer_sample_count=ROM_LOAD_U8(sequencer_generated_data+1)|((_pwm_sequencer_channel_count&0xf8)<<5);
	_pwm_sequencer_channel_count&=7;
	_pwm_sequencer_sample_index=0;
	_pwm_sequencer_data_index=_pwm_sequencer_channel_count+2;
	for (uint8_t i=0;i<_pwm_sequencer_channel_count*3;i++){
		_pwm_sequencer_scratch_buffer[i]=0;
	}
	TIMSK1|=1<<TOIE1;
}



void pwm_sequencer_stop(void){
	_pwm_sequencer_running=0;
	TIMSK1&=~(1<<TOIE1);
}

