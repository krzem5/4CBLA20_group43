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



#define PACKET_CHECKSUM_START_VALUE 0x55

#define PACKET_TYPE_NONE 0x00
#define PACKET_TYPE_ESTOP 0x01
#define PACKET_TYPE_MANUAL_INPUT 0x02
#define PACKET_TYPE_SEQUENCE_START 0x03



typedef union __attribute__((packed)) _PACKET{
	uint8_t _raw_data[0];
	struct __attribute__((packed)){
		uint8_t checksum;
		uint8_t type;
		union __attribute__((packed)){
			struct __attribute__((packed)){
				uint8_t wheel_left; // 180
				uint8_t wheel_right; // 180
				uint8_t linkage_middle; // 180
				uint8_t linkage_final; // 90
			} manual_input;
		};
	};
} packet_t;



extern const ROM_DECL uint8_t _packet_crc_table[256];



static inline uint8_t packet_process_checksum_byte(uint8_t x,uint8_t y){
	return ROM_LOAD_U8(_packet_crc_table+(x^y));
}



void packet_generate_checksum(packet_t* packet);



#endif
