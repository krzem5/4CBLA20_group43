/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 24/02/2025 by Krzesimir Hyżyk
 */



#include <avr/interrupt.h>
#include <avr/io.h>
#include <common/packet.h>
#include <servo/servo.h>
#include <stdint.h>



static uint8_t _reset_pin_left=0;
static uint8_t _reset_pin_right=0;
static uint8_t _reset_flags=0;



ISR(INT0_vect){
	if (_reset_flags&PACKET_RESET_FLAG_LEFT){
		_reset_flags&=~PACKET_RESET_FLAG_LEFT;
		servo_set_ticks(_reset_pin_left,64);
	}
}



ISR(INT1_vect){
	if (_reset_flags&PACKET_RESET_FLAG_RIGHT){
		_reset_flags&=~PACKET_RESET_FLAG_RIGHT;
		servo_set_ticks(_reset_pin_right,64);
	}
}



void reset_init(uint8_t left,uint8_t right){
	_reset_pin_left=left;
	_reset_pin_right=right;
	DDRD=0x00;
	PORTD=0x0c;
	EICRA=(1<<ISC11)|(1<<ISC01);
	EIMSK=(1<<INT1)|(1<<INT0);
}



void reset_start(uint8_t flags){
	_reset_flags|=flags;
	if (flags&PACKET_RESET_FLAG_LEFT){
		servo_set_ticks(_reset_pin_left,32);
	}
	if (flags&PACKET_RESET_FLAG_RIGHT){
		servo_set_ticks(_reset_pin_right,96);
	}
}



void reset_stop(void){
	if (_reset_flags&PACKET_RESET_FLAG_LEFT){
		servo_set_ticks(_reset_pin_left,64);
	}
	if (_reset_flags&PACKET_RESET_FLAG_RIGHT){
		servo_set_ticks(_reset_pin_right,64);
	}
	_reset_flags=0;
}



_Bool reset_is_enabled(void){
	return !!_reset_flags;
}
