/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _PWM_PWM_H_
#define _PWM_PWM_H_ 1
#include <stdint.h>



void pwm_init(void);



void pwm_set_pulse_width_us(uint8_t pin,uint16_t pulse);



#endif
