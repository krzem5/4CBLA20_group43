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



#define PWM_SEQUENCER_PULSE_ENCODING_FACTOR 10
#define PWM_SEQUENCER_PULSE_ENCODING_MIN_PULSE 50
#define PWM_SEQUENCER_PULSE_ENCODING_MAX_PULSE 250



void pwm_sequencer_start(void);



void pwm_sequencer_stop(void);



#endif
