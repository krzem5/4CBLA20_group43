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
#include <serial/serial.h>
#include <servo/servo.h>



// PWM/Servo pin numbering:
//  0 => D8
//  1 => D9
//  2 => D10
//  3 => D11
//  4 => D12
//  5 => D13
//  6 => A0
//  7 => A1
//  8 => A2
//  9 => A3
// 10 => A4
// 11 => A5



int main(void){
	sei();
	serial_init();
	pwm_init();
	servo_set_angle(2,90);
	servo_set_angle(3,90);
	servo_set_angle(4,90);
	while (1){
		packet_t packet;
		if (!serial_read_packet(&packet)||packet.type==PACKET_TYPE_NONE){
			continue;
		}
		cli();
		if (packet.type==PACKET_TYPE_ESTOP){
			pwm_sequencer_stop();
			servo_set_angle(2,90);
			servo_set_angle(3,90);
			servo_set_angle(4,90);
		}
		else if (packet.type==PACKET_TYPE_MANUAL_INPUT){
			servo_set_angle(2,packet.manual_input.test_servo_angle3);
			servo_set_angle(3,packet.manual_input.test_servo_angle);
			servo_set_angle(4,packet.manual_input.test_servo_angle2);
		}
		else if (packet.type==PACKET_TYPE_SEQUENCE_START){
			pwm_sequencer_start();
		}
		sei();
	}
}
