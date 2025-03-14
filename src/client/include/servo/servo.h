/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#ifndef _SERVO_SERVO_H_
#define _SERVO_SERVO_H_ 1
#include <pwm/pwm.h>
#include <stdint.h>



#define SERVO_MIN_PULSE_US 500
#define SERVO_MAX_PULSE_US 2500



static inline void servo_set_ticks(uint8_t pin,uint8_t ticks){
	if (ticks>128){
		ticks=128;
	}
	pwm_set_pulse_width_us(pin,SERVO_MIN_PULSE_US+(((uint16_t)ticks)<<4)-(ticks>>2)-(ticks>>3));
}



#endif
