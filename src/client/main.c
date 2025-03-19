/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <avr/interrupt.h>
#include <common/packet.h>
#include <pwm/pwm.h>
#include <pwm/sequencer.h>
#include <reset/reset.h>
#include <serial/serial.h>
#include <servo/servo.h>



// Pins:
// A0  => left wheel
// A1  => right wheel
// A2  => 2nd linkage left
// A3  => 2nd linkage right
// A4  => 3rd linkage left
// A5  => 3rd linkage right
// D2  => 2nd linkage left reed switch
// D3  => 2nd linkage right reed switch



int main(void){
	sei();
	serial_init();
	pwm_init();
	reset_init(2,3);
	servo_set_ticks(0,64);
	servo_set_ticks(1,64);
	servo_set_ticks(2,64);
	servo_set_ticks(3,64);
	servo_set_ticks(4,96);
	servo_set_ticks(5,32);
	while (1){
		packet_t packet;
		if (!serial_read_packet(&packet)||packet.type==PACKET_TYPE_NONE){
			continue;
		}
		cli();
		uint8_t reset_flags=reset_get_flags();
		if (packet.type==PACKET_TYPE_ESTOP){
			pwm_sequencer_stop();
			reset_stop();
			servo_set_ticks(0,64);
			servo_set_ticks(1,64);
			servo_set_ticks(2,64);
			servo_set_ticks(3,64);
			servo_set_ticks(4,96);
			servo_set_ticks(5,32);
		}
		else if (packet.type==PACKET_TYPE_RESET){
			pwm_sequencer_stop();
			reset_start(packet.reset.flags);
		}
		else if (packet.type==PACKET_TYPE_MANUAL_INPUT){
			servo_set_ticks(0,128-packet.manual_input.wheel_left);
			servo_set_ticks(1,packet.manual_input.wheel_right);
			if (!(reset_flags&PACKET_RESET_FLAG_LEFT)){
				servo_set_ticks(2,packet.manual_input.linkage_middle);
			}
			if (!(reset_flags&PACKET_RESET_FLAG_RIGHT)){
				servo_set_ticks(3,128-packet.manual_input.linkage_middle);
			}
			servo_set_ticks(4,packet.manual_input.linkage_final);
			servo_set_ticks(5,128-packet.manual_input.linkage_final);
		}
		else if (!reset_flags&&packet.type==PACKET_TYPE_SEQUENCE_START){
			pwm_sequencer_start();
		}
		sei();
	}
}
