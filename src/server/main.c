/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <ds4/ds4.h>
#include <poll.h>
#include <serial/serial.h>
#include <stdint.h>
#include <stdio.h>
#include <terminal/terminal.h>
#include <unistd.h>



static _Bool _exit_program=0;
static uint8_t _manual_control_x=0;
static uint8_t _manual_control_y=0;



static void _send_manual_input_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_MANUAL_INPUT,
		.manual_input={
			.test_servo_angle=_manual_control_x,
			.test_led_brightness=_manual_control_y*100/180
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



static void _process_terminal_command(void){
	switch (terminal_get_command()){
		case 3:
			_exit_program=1;
			return;
		case '1':
			_manual_control_x=0;
			_send_manual_input_packet();
			return;
		case '2':
			_manual_control_x=45;
			_send_manual_input_packet();
			return;
		case '3':
			_manual_control_x=90;
			_send_manual_input_packet();
			return;
		case '4':
			_manual_control_x=135;
			_send_manual_input_packet();
			return;
		case '5':
			_manual_control_x=180;
			_send_manual_input_packet();
			return;
		case 'q':
			_manual_control_y=0;
			_send_manual_input_packet();
			return;
		case 'w':
			_manual_control_y=45;
			_send_manual_input_packet();
			return;
		case 'e':
			_manual_control_y=90;
			_send_manual_input_packet();
			return;
		case 'r':
			_manual_control_y=135;
			_send_manual_input_packet();
			return;
		case 't':
			_manual_control_y=180;
			_send_manual_input_packet();
			return;
		case 's':
			_send_sequence_start_packet();
			return;
	}
}



static void _process_controller_command(ds4_device_t* controller){
	ds4_recv(controller);
	controller->led_green=0xff;
	ds4_send(controller);
	if (controller->buttons&DS4_BUTTON_LOGO){
		_exit_program=1;
		return;
	}
	if (controller->buttons&DS4_BUTTON_CROSS){
		_send_sequence_start_packet();
		return;
	}
	_manual_control_x=controller->l2*180/255;
	_manual_control_y=controller->r2*180/255;
	_send_manual_input_packet();
}



static void _update_ui(const ds4_device_t* controller){
	printf("\x1b[2K\r\x1b[0mX: \x1b[1;95m%3u\x1b[0m, Y: \x1b[1;95m%3u\x1b[0m",_manual_control_x,_manual_control_y);
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
	while (!_exit_program&&poll(fds,1+(controller.fd>=0),20)>=0&&!((fds[0].revents|fds[1].revents)&(POLLERR|POLLHUP|POLLNVAL))){
		if (fds[0].revents&POLLIN){
			_process_terminal_command();
		}
		if (fds[1].revents&POLLIN){
			_process_controller_command(&controller);
		}
		_update_ui(&controller);
	}
	printf("\x1b[?25h\r\n");
	packet_t packet={
		.type=PACKET_TYPE_ESTOP
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
	ds4_deinit(&controller);
	terminal_deinit();
	serial_deinit();
	return 0;
}
