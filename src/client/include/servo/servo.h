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
	pwm_set_pulse_width_us(pin,SERVO_MIN_PULSE_US+((((uint32_t)(ticks>128?128:ticks))*(SERVO_MAX_PULSE_US-SERVO_MIN_PULSE_US))>>7));
}



#endif
