/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#include <common/memory.h>
#include <common/packet.h>
#include <stdint.h>



static uint16_t _packet_crc_table[256];



void packet_init(void){
	for (uint16_t i=0;i<256;i++){
		uint16_t j=i<<8;
		for (uint8_t k=0;k<8;k++){
			j=(j<<1)^((j&0x8000)?0x1021:0);
		}
		_packet_crc_table[i]=j;
	}
}



uint16_t packet_compute_checksum(const packet_t* packet){
	uint16_t out=0;
	for (uint16_t i=__builtin_offsetof(packet_t,checksum)+sizeof(packet->checksum);i<sizeof(packet_t);i++){
		out=_packet_crc_table[(out&0xff)^packet->_raw_data[i]]^(out>>8);
	}
	return out;
}
