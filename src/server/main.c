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



int main(void){
	serial_init();
	terminal_init();
	struct pollfd fds[1]={
		{
			.fd=0,
			.events=POLLIN
		}
	};
	packet_t packet;
	while (poll(fds,1,-1)>0){
		if (fds[0].revents&POLLIN){
			switch (terminal_get_command()){
				case 3:
					goto _cleanup;
				case '1':
					packet.led_state=0;
					packet.checksum=packet_compute_checksum(&packet);
					serial_send(&packet,sizeof(packet_t));
					break;
				case '2':
					packet.led_state=1;
					packet.checksum=packet_compute_checksum(&packet);
					serial_send(&packet,sizeof(packet_t));
					break;
			}
		}
	}
_cleanup:
	terminal_deinit();
	serial_deinit();
	return 0;
}
