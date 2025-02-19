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



// Pins:
// 0 => left wheel
// 1 => right wheel
// 2 => 2nd linkage left
// 3 => 2nd linkage right
// 4 => 3rd linkage left
// 5 => 3rd linkage right



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
	pwm_set_pulse_width_us(5,0);
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
			pwm_set_pulse_width_us(5,0);
		}
		else if (packet.type==PACKET_TYPE_MANUAL_INPUT){
			servo_set_angle(0,packet.manual_input.wheel_left);
			servo_set_angle(1,packet.manual_input.wheel_right);
			servo_set_angle(2,packet.manual_input.linkage_middle);
			servo_set_angle(3,180-packet.manual_input.linkage_middle);
			servo_set_angle(4,90-packet.manual_input.linkage_final);
			// servo_set_angle(5,90+packet.manual_input.linkage_final);
			pwm_set_pulse_width_us(5,packet.manual_input.volume*100);
		}
		else if (packet.type==PACKET_TYPE_SEQUENCE_START){
			pwm_sequencer_start();
		}
		sei();
	}
}
