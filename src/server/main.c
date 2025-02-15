/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <serial/serial.h>
#include <stdint.h>
#include <unistd.h>



int main(void){
	serial_init();
	for (uint32_t i=0;i<10;i++){
		serial_send_buffer("A",1);
		usleep(100000);
		serial_send_buffer("B",1);
		usleep(100000);
	}
	return 0;
}
