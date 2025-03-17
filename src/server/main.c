/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <ds4/ds4.h>
#include <math.h>
#include <poll.h>
#include <serial/serial.h>
#include <stdint.h>
#include <stdio.h>
#include <terminal/terminal.h>
#include <unistd.h>



#define FLAG_EXIT_PROGRAM 1
#define FLAG_ESTOP_ENABLED 2
#define FLAG_ESTOP_BUTTON_DOWN 4
#define FLAG_LEFT_RESET_BUTTON_DOWN 8
#define FLAG_RIGHT_RESET_BUTTON_DOWN 16



static uint32_t _flags=0;
static uint8_t _manual_control_left_wheel=64;
static uint8_t _manual_control_right_wheel=64;
static uint8_t _manual_control_linkage_middle=64;
static uint16_t _manual_control_linkage_final=960;



static void _send_estop_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_ESTOP
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _send_manual_input_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_MANUAL_INPUT,
		.manual_input={
			.wheel_left=_manual_control_left_wheel,
			.wheel_right=_manual_control_right_wheel,
			.linkage_middle=_manual_control_linkage_middle,
			.linkage_final=_manual_control_linkage_final/10
		}
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _send_sequence_start_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_SEQUENCE_START
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _send_reset_packet(uint32_t flags){
	packet_t packet={
		.type=PACKET_TYPE_RESET,
		.reset={
			.flags=flags
		}
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _toggle_estop(void){
	if (_flags&FLAG_ESTOP_BUTTON_DOWN){
		return;
	}
	_flags=(_flags^FLAG_ESTOP_ENABLED)|FLAG_ESTOP_BUTTON_DOWN;
	if (_flags&FLAG_ESTOP_ENABLED){
		_send_estop_packet();
	}
}



static void _process_terminal_command(void){
	uint16_t command=terminal_get_command();
	if (command==0x03){
		_flags|=FLAG_EXIT_PROGRAM;
		return;
	}
	if (command==' '){
		_toggle_estop();
		return;
	}
	_flags&=~FLAG_ESTOP_BUTTON_DOWN;
	if (_flags&FLAG_ESTOP_ENABLED){
		return;
	}
	if (command=='s'){
		_send_sequence_start_packet();
	}
}



static void _process_controller_command(ds4_device_t* controller){
	ds4_recv(controller);
	if (_flags&FLAG_ESTOP_ENABLED){
		controller->led_red=0xff;
		controller->led_green=0x00;
		controller->led_blue=0x00;
	}
	else{
		controller->led_red=0x00;
		controller->led_green=0xff;
		controller->led_blue=0x00;
	}
	ds4_send(controller);
	if (controller->buttons&DS4_BUTTON_LOGO){
		_flags|=FLAG_EXIT_PROGRAM;
		return;
	}
	if (controller->buttons&(DS4_BUTTON_CIRCLE|DS4_BUTTON_TOUCHPAD)){
		_toggle_estop();
		return;
	}
	_flags&=~FLAG_ESTOP_BUTTON_DOWN;
	if (_flags&FLAG_ESTOP_ENABLED){
		return;
	}
	if (controller->buttons&DS4_BUTTON_CROSS){
		_send_sequence_start_packet();
		return;
	}
	float p=controller->lx/100.0f;
	float q=controller->ly/100.0f;
	float r=hypotf(p,q);
	if (r<0.3f){
		_manual_control_left_wheel=64;
		_manual_control_right_wheel=64;
	}
	else{
		int8_t u=(r>1.0f?1.0f:r)*64;
		int8_t v=(q*q-p*p)/r*64*(r>1.0f?1/r:1.0f);
		if (p>=0){
			if (q>=0){
				_manual_control_left_wheel=64+u;
				_manual_control_right_wheel=64+v;
			}
			else{
				_manual_control_left_wheel=64-v;
				_manual_control_right_wheel=64-u;
			}
		}
		else{
			if (q>=0){
				_manual_control_left_wheel=64+v;
				_manual_control_right_wheel=64+u;
			}
			else{
				_manual_control_left_wheel=64-u;
				_manual_control_right_wheel=64-v;
			}
		}
	}
	if (controller->l2>32){
		_manual_control_left_wheel=64+(controller->l2>>2);
		_manual_control_right_wheel=64+(controller->l2>>2);
	}
	else if (controller->r2>32){
		_manual_control_left_wheel=64-(controller->r2>>2);
		_manual_control_right_wheel=64-(controller->r2>>2);
	}
	if (controller->buttons&DS4_BUTTON_UP){
		_manual_control_linkage_middle=32;
	}
	else if (controller->buttons&DS4_BUTTON_DOWN){
		_manual_control_linkage_middle=96;
	}
	else{
		_manual_control_linkage_middle=64;
	}
	if ((controller->buttons&DS4_BUTTON_LEFT)&&_manual_control_linkage_final){
		_manual_control_linkage_final--;
	}
	if ((controller->buttons&DS4_BUTTON_RIGHT)&&_manual_control_linkage_final<960){
		_manual_control_linkage_final++;
	}
	if (controller->buttons&DS4_BUTTON_L1){
		if (!(_flags&FLAG_LEFT_RESET_BUTTON_DOWN)){
			_flags|=FLAG_LEFT_RESET_BUTTON_DOWN;
			_send_reset_packet(PACKET_RESET_FLAG_LEFT);
		}
	}
	else{
		_flags&=~FLAG_LEFT_RESET_BUTTON_DOWN;
	}
	if (controller->buttons&DS4_BUTTON_R1){
		if (!(_flags&FLAG_RIGHT_RESET_BUTTON_DOWN)){
			_flags|=FLAG_RIGHT_RESET_BUTTON_DOWN;
			_send_reset_packet(PACKET_RESET_FLAG_RIGHT);
		}
	}
	else{
		_flags&=~FLAG_RIGHT_RESET_BUTTON_DOWN;
	}
	_send_manual_input_packet();
}



static void _update_ui(const ds4_device_t* controller){
	printf("\x1b[2K\r\x1b[0mL: \x1b[1;95m%3u\x1b[0m, R: \x1b[1;95m%3u\x1b[0m, \x1b[0mA: \x1b[1;95m%3u\x1b[0m, B: \x1b[1;95m%3u\x1b[0m",_manual_control_left_wheel,_manual_control_right_wheel,_manual_control_linkage_middle,_manual_control_linkage_final/10);
	fflush(stdout);
}



int main(void){
	serial_init();
	terminal_init();
	ds4_device_t controller;
	ds4_init(&controller);
	struct pollfd fds[2]={
		{
			.fd=0,
			.events=POLLIN,
			.revents=0
		},
		{
			.fd=controller.fd,
			.events=POLLIN,
			.revents=0
		}
	};
	printf("\x1b[?25l");
	while (!(_flags&FLAG_EXIT_PROGRAM)&&poll(fds,1+(controller.fd>=0),-1)>=0){
		if (fds[0].revents&POLLIN){
			_process_terminal_command();
		}
		if (fds[1].revents&POLLIN){
			_process_controller_command(&controller);
		}
		_update_ui(&controller);
	}
	printf("\x1b[?25h\r\n");
	_send_estop_packet();
	ds4_deinit(&controller);
	terminal_deinit();
	serial_deinit();
	return 0;
}
