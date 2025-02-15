/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _SERIAL_SERIAL_H_
#define _SERIAL_SERIAL_H_ 1



#define SERIAL_DEVICE "/dev/ttyACM0"
#define SERIAL_SPEED B115200
#define SERIAL_PARITY 0



void serial_init(void);



void serial_send(const void* buffer,unsigned int length);



#endif
