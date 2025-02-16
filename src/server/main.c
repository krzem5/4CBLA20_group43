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



static const char* _horizontal_box_characters[]={
	" ",
	"▕",
	"🮇",
	"🮈",
	"▐",
	"🮉",
	"🮊",
	"🮋",
	"█"
};

static _Bool _exit_program=0;



static void _process_terminal_command(packet_t* packet){
	switch (terminal_get_command()){
		case 3:
			_exit_program=1;
			return;
		case '1':
			packet->test_servo_angle=0;
			return;
		case '2':
			packet->test_servo_angle=45;
			return;
		case '3':
			packet->test_servo_angle=90;
			return;
		case '4':
			packet->test_servo_angle=135;
			return;
		case '5':
			packet->test_servo_angle=180;
			return;
	}
}



static void _process_controller_command(ds4_device_t* controller,packet_t* packet){
	ds4_recv(controller);
	if (controller->buttons&DS4_BUTTON_LOGO){
		_exit_program=1;
		return;
	}
	packet->test_servo_angle=controller->l2*180/255;
}



static void _update_ui(const ds4_device_t* controller,const packet_t* packet){
	char battery_string[32];
	if (controller->battery){
		snprintf(battery_string,sizeof(battery_string),"\x1b[100;92m%02u%%\x1b[0m",controller->battery);
	}
	else{
		snprintf(battery_string,sizeof(battery_string),"\x1b[100;92m🮙🮙🮙🮙\x1b[0m");
	}
	(void)_horizontal_box_characters;
	printf("\x1b[2K\r\x1b[0mX: \x1b[1;95m%3u\x1b[0m, Y:   \x1b[1;95m0\x1b[0m, Battery: %s",packet->test_servo_angle,battery_string);
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
	packet_t packet={
		.test_servo_angle=90
	};
	printf("\x1b[?25l");
	while (!_exit_program&&poll(fds,1+(controller.fd>=0),20)>=0&&!((fds[0].revents|fds[1].revents)&(POLLERR|POLLHUP|POLLNVAL))){
		if (fds[0].revents&POLLIN){
			_process_terminal_command(&packet);
		}
		if (fds[1].revents&POLLIN){
			_process_controller_command(&controller,&packet);
		}
		packet_generate_checksum(&packet);
		serial_send(&packet,sizeof(packet_t));
		_update_ui(&controller,&packet);
	}
	printf("\x1b[?25h\r\n");
	packet.test_servo_angle=90;
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
	ds4_deinit(&controller);
	terminal_deinit();
	serial_deinit();
	return 0;
}
