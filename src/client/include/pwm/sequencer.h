/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 17/02/2025 by Krzesimir Hyżyk
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



void pwm_sequencer_start(void);



#endif
