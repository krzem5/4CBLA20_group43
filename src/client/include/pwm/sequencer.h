/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _PWM_SEQUENCER_H_
#define _PWM_SEQUENCER_H_ 1
#include <pwm/pwm.h>
#include <stdint.h>



#define PWM_SEQUENCER_PIN_MASK 0x3f
#define PWM_SEQUENCER_PIN_FLAG_INVERTED 0x40
#define PWM_SEQUENCER_PIN_FLAG_LAST 0x80

#define PWM_SEQUENCER_PULSE_ENCODING_FACTOR 10
#define PWM_SEQUENCER_PULSE_ENCODING_CUTOFF 59



typedef struct _PWM_SEQUENCER_DATA{
	uint8_t channel_count;
	uint16_t sample_count;
	const uint8_t* data;
	uint8_t* scratch_data;
} pwm_sequencer_data_t;



void pwm_sequencer_start(void);



#endif
