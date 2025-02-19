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



int main(void){
	sei();
	serial_init();
	pwm_init();
	servo_set_angle(0,90);
	servo_set_angle(1,90);
	servo_set_angle(2,90);
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
		}
		else if (packet.type==PACKET_TYPE_MANUAL_INPUT){
			servo_set_angle(0,packet.manual_input.test_servo_angle);
			servo_set_angle(1,packet.manual_input.test_servo_angle2);
			servo_set_angle(2,packet.manual_input.test_servo_angle3);
		}
		else if (packet.type==PACKET_TYPE_SEQUENCE_START){
			pwm_sequencer_start();
		}
		sei();
	}
}
