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



static void _process_terminal_command(void){
	packet_t packet;
	switch (terminal_get_command()){
		case 3:
			_exit_program=1;
			return;
		case '1':
			packet.led_state=0;
			break;
		case '2':
			packet.led_state=1;
			break;
	}
	packet.checksum=packet_compute_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
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
	while (!_exit_program&&poll(fds,1+(!!fds[1].fd),-1)>=0&&!((fds[0].revents|fds[1].revents)&(POLLERR|POLLHUP|POLLNVAL))){
		if (fds[0].revents&POLLIN){
			_process_terminal_command();
		}
	}
	packet_t packet;
	packet.led_state=0;
	packet.checksum=packet_compute_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
	terminal_deinit();
	serial_deinit();
	return 0;
}
