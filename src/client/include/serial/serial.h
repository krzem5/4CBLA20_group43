/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#ifndef _SERIAL_SERIAL_H_
#define _SERIAL_SERIAL_H_ 1
#include <common/packet.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif



#define SERIAL_BAUD_RATE 115200



void serial_init(void);



_Bool serial_read(uint8_t* buffer,uint8_t length);



_Bool serial_read_packet(packet_t* out);



#ifdef __cplusplus
}
#endif
#endif
