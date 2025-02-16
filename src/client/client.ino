/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <pwm/pwm.h>
#include <serial/serial.h>



static packet_t _packet;
static pwm_t _test_led_pwm;



void setup(void){
	serial_init();
	pwm_init();
	_test_led_pwm=pwm_alloc(LED_BUILTIN);
}



void loop(void){
	if (!serial_read(_packet._raw_data,sizeof(packet_t))||_packet.checksum!=packet_checksum_compute(&_packet)){
		return;
	}
	pwm_set_pulse_width_us(_test_led_pwm,_packet.led_state<<4);
}
