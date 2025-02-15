/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <serial/serial.h>
#include <stdint.h>
#include <unistd.h>



int main(void){
	serial_init();
	packet_t packet;
	for (uint32_t i=0;i<20;i++){
		packet.led_state=(i&1)^1;
		serial_send_buffer(&packet,sizeof(packet_t));
		usleep(100000);
	}
	return 0;
}
