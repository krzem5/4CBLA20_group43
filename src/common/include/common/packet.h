/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _COMMON_PACKET_H_
#define _COMMON_PACKET_H_ 1
#include <common/memory.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif



#define PACKET_VALID_CHECKSUM 0xca



typedef union __attribute__((packed)) _PACKET{
	uint8_t _raw_data[0];
	struct __attribute__((packed)){
		uint8_t led_state;
		uint8_t checksum;
	};
} packet_t;



extern const ROM_DECL uint8_t _packet_crc_table[256];



static inline uint8_t packet_process_checksum_byte(uint8_t x,uint8_t y){
	return ROM_LOAD(_packet_crc_table+(x^y));
}



void packet_append_checksum(packet_t* packet);



#ifdef __cplusplus
}
#endif
#endif
