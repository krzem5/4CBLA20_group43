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



#define SERVO_MIN_PULSE_US 600
#define SERVO_US_PER_DEGREE 10



static inline void servo_set_angle(uint8_t pin,uint8_t angle){
	pwm_set_pulse_width_us(pin,SERVO_MIN_PULSE_US+SERVO_US_PER_DEGREE*((uint16_t)(angle>180?180:angle)));
}



#endif
