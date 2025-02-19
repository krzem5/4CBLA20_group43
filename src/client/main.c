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
// 0 A0  => left wheel
// 1 A1  => right wheel
// 2 A2  => 2nd linkage left
// 3 A3  => 2nd linkage right
// 4 A4  => 3rd linkage left
// 5 A5  => 3rd linkage right
// 6 D8  => camera yaw
// 7 D9  => camera pitch
// 8 D10 => buzzer



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
	servo_set_angle(6,90);
	servo_set_angle(7,90);
	pwm_set_pulse_width_us(8,0);
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
			servo_set_angle(6,90);
			servo_set_angle(7,90);
			pwm_set_pulse_width_us(8,0);
		}
		else if (packet.type==PACKET_TYPE_MANUAL_INPUT){
			servo_set_angle(0,packet.manual_input.wheel_left);
			servo_set_angle(1,packet.manual_input.wheel_right);
			servo_set_angle(2,packet.manual_input.linkage_middle);
			servo_set_angle(3,180-packet.manual_input.linkage_middle);
			servo_set_angle(4,90-(packet.manual_input.linkage_final_and_buzzer&0x7f));
			servo_set_angle(5,90+(packet.manual_input.linkage_final_and_buzzer&0x7f));
			servo_set_angle(6,packet.manual_input.camera_yaw);
			servo_set_angle(7,packet.manual_input.camera_pitch);
			pwm_set_pulse_width_us(8,(packet.manual_input.linkage_final_and_buzzer>>7?20000:0));
		}
		else if (packet.type==PACKET_TYPE_SEQUENCE_START){
			pwm_sequencer_start();
		}
		else if (packet.type==PACKET_TYPE_PORTB_ENABLE){
			pwm_portb_enable();
		}
		else if (packet.type==PACKET_TYPE_PORTB_DISABLE){
			pwm_portb_disable();
		}
		sei();
	}
}
