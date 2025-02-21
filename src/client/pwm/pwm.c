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

#define PWM_SEQUENCER_MAX_CHANNEL_COUNT 3



static uint16_t _pwm_pin_entries[6];

static uint8_t _pwm_parallel_scheduler_update_flag;
static uint8_t _pwm_parallel_scheduler_value_entries[7];
static uint16_t _pwm_parallel_scheduler_delta_entries[7];
static uint8_t _pwm_parallel_scheduler_entry_index=0;
static uint8_t _pwm_parallel_scheduler_entry_count=0;

static uint8_t _pwm_sequencer_channel_count_plus_2=0;
static uint16_t _pwm_sequencer_sample_index;
static const uint8_t* _pwm_sequencer_data_ptr;
static uint8_t _pwm_sequencer_scratch_buffer[PWM_SEQUENCER_MAX_CHANNEL_COUNT*3];



ISR(TIMER1_COMPA_vect){
	if (_pwm_parallel_scheduler_entry_index!=_pwm_parallel_scheduler_entry_count){
		goto _process_next_scheduler_entry;
	}
	_pwm_parallel_scheduler_entry_index=0;
	if (!_pwm_parallel_scheduler_update_flag){
		goto _process_next_scheduler_entry;
	}
	_pwm_parallel_scheduler_update_flag=0;
	_pwm_parallel_scheduler_value_entries[0]=0x3f;
	_pwm_parallel_scheduler_entry_count=0;
	uint8_t mask=0x3f;
	uint16_t min=0;
	uint16_t pulse=0;
	do{
		uint8_t j=0;
		for (;_pwm_pin_entries[j]<min;j++);
		uint16_t next=_pwm_pin_entries[j];
		uint8_t clear_mask=1<<j;
		for (j++;j<6;j++){
			if (_pwm_pin_entries[j]==next){
				clear_mask|=1<<j;
			}
			else if (_pwm_pin_entries[j]>=min&&_pwm_pin_entries[j]<next){
				next=_pwm_pin_entries[j];
				clear_mask=1<<j;
			}
		}
		mask&=~clear_mask;
		min=next+1;
		uint16_t delta=next-pulse;
		if (delta>=PWM_MIN_PULSE_US*PWM_TIMER_TICKS_PER_US){
			pulse=next;
			_pwm_parallel_scheduler_delta_entries[_pwm_parallel_scheduler_entry_count]=delta;
			_pwm_parallel_scheduler_entry_count++;
		}
		_pwm_parallel_scheduler_value_entries[_pwm_parallel_scheduler_entry_count]=mask;
	} while (mask);
	_pwm_parallel_scheduler_delta_entries[_pwm_parallel_scheduler_entry_count]=(PWM_WINDOW_US+PWM_MIN_PULSE_US)*PWM_TIMER_TICKS_PER_US-pulse;
	_pwm_parallel_scheduler_entry_count++;
_process_next_scheduler_entry:
	uint8_t value=_pwm_parallel_scheduler_value_entries[_pwm_parallel_scheduler_entry_index];
	uint16_t delta=_pwm_parallel_scheduler_delta_entries[_pwm_parallel_scheduler_entry_index];
	_pwm_parallel_scheduler_entry_index++;
	PORTC=value;
	OCR1A=TCNT1+delta;
}



ISR(TIMER1_OVF_vect){
	uint8_t* ptr=_pwm_sequencer_scratch_buffer;
	for (uint8_t i=2;i<_pwm_sequencer_channel_count_plus_2;i++){
		if (ptr[0]){
			ptr[0]--;
			if (!ptr[1]){
				goto _next_channel;
			}
		}
		else{
			uint8_t token=ROM_LOAD_U8(_pwm_sequencer_data_ptr);
			_pwm_sequencer_data_ptr++;
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
		uint8_t pins=ROM_LOAD_U8(sequencer_generated_data+i);
		_pwm_pin_entries[pins&15]=pulse;
		_pwm_pin_entries[pins>>4]=(PWM_SEQUENCER_PULSE_ENCODING_MIN_PULSE+PWM_SEQUENCER_PULSE_ENCODING_MAX_PULSE)*PWM_SEQUENCER_PULSE_ENCODING_FACTOR*PWM_TIMER_TICKS_PER_US-pulse;
		_pwm_parallel_scheduler_update_flag=1;
_next_channel:
		ptr+=3;
	}
	_pwm_sequencer_sample_index--;
	if (!_pwm_sequencer_sample_index){
		_pwm_sequencer_channel_count_plus_2=0;
		PORTB=0;
		TIMSK1&=~(1<<TOIE1);
	}
}



void pwm_init(void){
	for (uint32_t i=0;i<6;i++){
		_pwm_pin_entries[i]=0;
	}
	_pwm_parallel_scheduler_update_flag=1;
	ADMUX=0;
	ADCSRA=0;
	DIDR0=0;
	DDRB=0x20;
	DDRC=0x3f;
	PORTB=0;
	PORTC=0;
	TCCR1A=0;
	TCCR1B=1<<CS11;
	TCNT1=0;
	TIFR1=(1<<TOV1)|(1<<OCF1A)|(1<<OCF1B)|(1<<ICF1);
	OCR1A=0;
	TIMSK1=1<<OCIE1A;
}



void pwm_set_pulse_width_us(uint8_t pin,uint16_t pulse){
	if (_pwm_sequencer_channel_count_plus_2){
		return;
	}
	pulse=(pulse>PWM_WINDOW_US?PWM_WINDOW_US:pulse)*PWM_TIMER_TICKS_PER_US;
	if (_pwm_pin_entries[pin]==pulse){
		return;
	}
	_pwm_pin_entries[pin]=pulse;
	_pwm_parallel_scheduler_update_flag=1;
}



void pwm_sequencer_start(void){
	if (_pwm_sequencer_channel_count_plus_2){
		return;
	}
	_pwm_sequencer_channel_count_plus_2=ROM_LOAD_U8(sequencer_generated_data);
	_pwm_sequencer_sample_index=ROM_LOAD_U8(sequencer_generated_data+1)|((_pwm_sequencer_channel_count_plus_2>>3)<<8);
	_pwm_sequencer_channel_count_plus_2=(_pwm_sequencer_channel_count_plus_2&7)+2;
	_pwm_sequencer_data_ptr=sequencer_generated_data+_pwm_sequencer_channel_count_plus_2;
	for (uint8_t i=0;i<(_pwm_sequencer_channel_count_plus_2-2)*3;i+=3){
		_pwm_sequencer_scratch_buffer[i]=0;
		_pwm_sequencer_scratch_buffer[i+1]=0;
		_pwm_sequencer_scratch_buffer[i+2]=ROM_LOAD_U8(_pwm_sequencer_data_ptr);
		_pwm_sequencer_data_ptr++;
	}
	PORTB=0x20;
	TIMSK1|=1<<TOIE1;
}



void pwm_sequencer_stop(void){
	_pwm_sequencer_channel_count_plus_2=0;
	PORTB=0;
	TIMSK1&=~(1<<TOIE1);
}
