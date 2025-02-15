/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <serial/serial.h>
#include <termios.h>
#include <unistd.h>
#include <util/util.h>



static int _serial_fd=-1;



void serial_init(void){
	DIR* dir=opendir("/dev/serial/by-id/");
	if (!dir){
		goto _error;
	}
	for (struct dirent* entry=readdir(dir);entry;entry=readdir(dir)){
		if (entry->d_type!=DT_LNK){
			continue;
		}
		_serial_fd=openat(dirfd(dir),entry->d_name,O_RDWR|O_NOCTTY|O_SYNC);
		if (_serial_fd<0){
			continue;
		}
		struct termios tty;
		if (tcgetattr(_serial_fd,&tty)){
			close(_serial_fd);
			continue;
		}
		tty.c_iflag&=~(IGNBRK|IXON|IXANY|IXOFF);
		tty.c_oflag=0;
		tty.c_cflag=(tty.c_cflag&(~(CSIZE|CSTOPB|PARENB|PARODD|CRTSCTS)))|CS8|CREAD|CLOCAL|SERIAL_PARITY;
		tty.c_lflag=0;
		tty.c_cc[VMIN]=0;
		tty.c_cc[VTIME]=0;
		if (cfsetospeed(&tty,SERIAL_SPEED)||cfsetispeed(&tty,SERIAL_SPEED)||tcsetattr(_serial_fd,TCSANOW,&tty)){
			close(_serial_fd);
			continue;
		}
		closedir(dir);
		return;
	}
	closedir(dir);
_error:
	ASSERT(!"No serial device found");
}



void serial_deinit(void){
	ASSERT(!tcdrain(_serial_fd)&&!close(_serial_fd));
	_serial_fd=-1;
}



void serial_send(const void* buffer,unsigned int length){
	ASSERT(write(_serial_fd,buffer,length)==length);
}
