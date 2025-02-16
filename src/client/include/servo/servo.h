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
#ifdef __cplusplus
extern "C" {
#endif



typedef pwm_t servo_t;



static inline servo_t servo_alloc(uint8_t pin){
	return pwm_alloc(pin);
}



void servo_set_angle(servo_t servo,uint8_t angle);



#ifdef __cplusplus
}
#endif
#endif
