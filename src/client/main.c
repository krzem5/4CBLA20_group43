/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <avr/interrupt.h>
#include <common/packet.h>
#include <pwm/pwm.h>
#include <serial/serial.h>



int main(void){
	sei();
	serial_init();
	pwm_init();
	pwm_t test_led_pwm=pwm_alloc(13);
	while (1){
		packet_t packet;
		if (!serial_read_packet(&packet)){
			continue;
		}
		pwm_set_pulse_width_us(test_led_pwm,packet.led_state<<4);
	}
}
