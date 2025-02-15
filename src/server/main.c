/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>



#define SERIAL_DEVICE "/dev/ttyACM0"
#define SERIAL_SPEED B115200
#define SERIAL_PARITY 0

#define ASSERT(x) if (!(x)){printf("%u(%s): %s: Assertion failed\n",__LINE__,__func__,#x);_Exit(1);}



int open_serial_interface(void){
	int fd=open(SERIAL_DEVICE,O_RDWR|O_NOCTTY|O_SYNC);
	ASSERT(fd>=0);
	struct termios tty;
	ASSERT(!tcgetattr(fd,&tty));
	tty.c_iflag&=~(IXON|IXOFF|IXANY|IGNBRK);
	tty.c_oflag=0;
	tty.c_cflag=(tty.c_cflag&(~(CSIZE|PARENB|PARODD|CSTOPB|CRTSCTS)))|CS8|CLOCAL|CREAD|SERIAL_PARITY;
	tty.c_lflag=0;
	tty.c_cc[VMIN]=0;
	tty.c_cc[VTIME]=5;
	ASSERT(!cfsetospeed(&tty,SERIAL_SPEED)&&!cfsetispeed(&tty,SERIAL_SPEED));
	ASSERT(!tcsetattr(fd,TCSANOW,&tty));
	return fd;
}



int main(void){
	int fd=open_serial_interface();
	for (uint32_t i=0;i<10;i++){
		if (write(fd,"A",1));
		usleep(100000);
		if (write(fd,"B",1));
		usleep(100000);
	}
	return 0;
}
