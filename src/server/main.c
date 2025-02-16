/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <poll.h>
#include <serial/serial.h>
#include <stdint.h>
#include <terminal/terminal.h>
#include <unistd.h>



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



int main(void){
	serial_init();
	terminal_init();
	struct pollfd fds[2]={
		{
			.fd=0,
			.events=POLLIN
		},
		{
			.fd=0/*ds4_fd*/,
			.events=POLLIN,
			.revents=0
		}
	};
	packet_t packet={
		.test_servo_angle=90
	};
	while (!_exit_program&&poll(fds,1+(!!fds[1].fd),20)>=0&&!((fds[0].revents|fds[1].revents)&(POLLERR|POLLHUP|POLLNVAL))){
		if (fds[0].revents&POLLIN){
			_process_terminal_command(&packet);
		}
		packet_generate_checksum(&packet);
		serial_send(&packet,sizeof(packet_t));
	}
	packet.test_servo_angle=90;
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
	terminal_deinit();
	serial_deinit();
	return 0;
}
