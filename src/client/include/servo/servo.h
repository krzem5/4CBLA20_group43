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



void servo_init_pin(uint8_t pin);



void servo_set_angle(uint8_t pin,uint8_t angle);



#endif
