/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <pwm/pwm.h>
#include <serial/serial.h>



static packet_t _packet;
static pwm_t _test_led_pwm;



int main(void){
	init();
	serial_init();
	pwm_init();
	_test_led_pwm=pwm_alloc(LED_BUILTIN);
	while (1){
		if (!serial_read_packet(&_packet)){
			continue;
		}
		pwm_set_pulse_width_us(_test_led_pwm,_packet.led_state<<4);
	}
}
