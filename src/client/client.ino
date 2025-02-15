/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <pwm/pwm.h>



static packet_t _packet;
static pwm_t _test_led_pwm;



void setup(void){
	pwm_init();
	_test_led_pwm=pwm_alloc(LED_BUILTIN);
	Serial.begin(115200);
}



void loop(void){
	if (Serial.available()<sizeof(packet_t)||Serial.readBytes(_packet._raw_data,sizeof(packet_t))<sizeof(packet_t)){
		return;
	}
	if (_packet.checksum!=packet_compute_checksum(&_packet)){
		while (Serial.available()>0&&Serial.read()>=0);
		return;
	}
	pwm_set_pulse_width_us(_test_led_pwm,_packet.led_state<<4);
}
