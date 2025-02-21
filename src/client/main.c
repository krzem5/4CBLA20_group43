/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */
// last commit w/ port B scheduler https://github.com/krzem5/4CBLA20_group43/tree/a9eac37e9de94c45057ebfaa40ac0d402c17ee5a



#include <avr/interrupt.h>
#include <common/packet.h>
#include <pwm/pwm.h>
#include <pwm/sequencer.h>
#include <serial/serial.h>
#include <servo/servo.h>



// Pins:
// 0 A0  => left wheel
// 1 A1  => right wheel
// 2 A2  => 2nd linkage left
// 3 A3  => 2nd linkage right
// 4 A4  => 3rd linkage left
// 5 A5  => 3rd linkage right



int main(void){
	sei();
	serial_init();
	pwm_init();
	servo_set_angle(0,90);
	servo_set_angle(1,90);
	servo_set_angle(2,90);
	servo_set_angle(3,90);
	servo_set_angle(4,90);
	servo_set_angle(5,90);
	while (1){
		packet_t packet;
		if (!serial_read_packet(&packet)||packet.type==PACKET_TYPE_NONE){
			continue;
		}
		cli();
		if (packet.type==PACKET_TYPE_ESTOP){
			pwm_sequencer_stop();
			servo_set_angle(0,90);
			servo_set_angle(1,90);
			servo_set_angle(2,90);
			servo_set_angle(3,90);
			servo_set_angle(4,90);
			servo_set_angle(5,90);
		}
		else if (packet.type==PACKET_TYPE_MANUAL_INPUT){
			servo_set_angle(0,packet.manual_input.wheel_left);
			servo_set_angle(1,packet.manual_input.wheel_right);
			servo_set_angle(2,packet.manual_input.linkage_middle);
			servo_set_angle(3,180-packet.manual_input.linkage_middle);
			servo_set_angle(4,90-packet.manual_input.linkage_final);
			servo_set_angle(5,90+packet.manual_input.linkage_final);
		}
		else if (packet.type==PACKET_TYPE_SEQUENCE_START){
			pwm_sequencer_start();
		}
		sei();
	}
}
