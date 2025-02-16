/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <pwm/pwm.h>
#include <servo/servo.h>
#include <stdint.h>



#define SERVO_MIN_PULSE_US 600
#define SERVO_US_PER_DEGREE 10



servo_t servo_alloc(uint8_t pin){
	pwm_t out=pwm_alloc(pin);
	servo_set_angle(out,90);
	return out;
}



void servo_set_angle(servo_t servo,uint8_t angle){
	pwm_set_pulse_width_us(servo,SERVO_MIN_PULSE_US+SERVO_US_PER_DEGREE*((uint16_t)(angle>180?180:angle)));
}
