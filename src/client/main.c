/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <avr/interrupt.h>
#include <common/packet.h>
#include <pwm/sequencer.h>
#include <serial/serial.h>
#include <servo/servo.h>



int main(void){
	sei();
	serial_init();
	pwm_init();
	servo_t test_servo=servo_alloc(13);
	while (1){
		packet_t packet;
		if (!serial_read_packet(&packet)){
			continue;
		}
		cli();
		servo_set_angle(test_servo,packet.test_servo_angle);
		if (packet.start_sequence_token==PACKET_START_SEQUENCE_TOKEN){
			pwm_sequencer_start();
		}
		sei();
	}
}
