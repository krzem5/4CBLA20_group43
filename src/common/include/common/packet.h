/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _COMMON_PACKET_H_
#define _COMMON_PACKET_H_ 1
#include <stdint.h>



typedef union __attribute__((packed)) _PACKET{
	uint8_t _bytes[0];
	struct __attribute__((packed)){
		uint8_t led_state;
	};
} packet_t;



#endif
