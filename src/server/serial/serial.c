/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <errno.h>
#include <fcntl.h>
#include <serial/serial.h>
#include <termios.h>
#include <unistd.h>
#include <util/util.h>



static int _serial_fd=-1;



void serial_init(void){
	_serial_fd=open(SERIAL_DEVICE,O_RDWR|O_NOCTTY|O_SYNC);
	ASSERT(_serial_fd>=0||!"Serial device not found");
	struct termios tty;
	ASSERT(!tcgetattr(_serial_fd,&tty));
	tty.c_iflag&=~(IXON|IXOFF|IXANY|IGNBRK);
	tty.c_oflag=0;
	tty.c_cflag=(tty.c_cflag&(~(CSIZE|PARENB|PARODD|CSTOPB|CRTSCTS)))|CS8|CLOCAL|CREAD|SERIAL_PARITY;
	tty.c_lflag=0;
	tty.c_cc[VMIN]=0;
	tty.c_cc[VTIME]=5;
	ASSERT(!cfsetospeed(&tty,SERIAL_SPEED)&&!cfsetispeed(&tty,SERIAL_SPEED));
	ASSERT(!tcsetattr(_serial_fd,TCSANOW,&tty));
}



void serial_send_buffer(const void* buffer,unsigned int length){
	ASSERT(write(_serial_fd,buffer,length)==length);
}
