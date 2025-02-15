/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <util/util.h>



static struct termios _terminal_restore_config;



void terminal_init(void){
	ASSERT(!tcgetattr(1,&_terminal_restore_config));
	struct termios tty=_terminal_restore_config;
	tty.c_iflag=(tty.c_iflag&(~(INLCR|IGNBRK)))|ICRNL;
	tty.c_lflag=(tty.c_lflag&(~(ISIG|ICANON|ECHO)))|IEXTEN;
	ASSERT(!tcsetattr(1,TCSANOW,&tty));
}



void terminal_deinit(void){
	tcsetattr(1,TCSANOW,&_terminal_restore_config);
}



uint16_t terminal_get_command(void){
	char buffer[4];
	int count=read(0,buffer,4);
	return buffer[0]+(count>2?buffer[2]<<8:0);
}
