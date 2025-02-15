/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _COMMON_PACKET_H_
#define _COMMON_PACKET_H_ 1
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif



typedef union __attribute__((packed)) _PACKET{
	uint8_t _raw_data[0];
	struct __attribute__((packed)){
		uint16_t checksum;
		uint8_t led_state;
	};
} packet_t;



uint16_t packet_compute_checksum(const packet_t* packet);



#ifdef __cplusplus
}
#endif
#endif
