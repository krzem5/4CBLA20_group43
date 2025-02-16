/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <serial/serial.h>
#include <stdint.h>



#define SERIAL_BUFFER_SIZE 16



static volatile uint8_t _serial_buffer_head=0;
static volatile uint8_t _serial_buffer_tail=0;
static uint8_t _serial_buffer[SERIAL_BUFFER_SIZE];



ISR(USART_RX_vect){
	uint8_t c=UDR0;
	if (UCSR0A&(1<<UPE0)){
		return;
	}
	uint8_t i=(_serial_buffer_head+1)&(SERIAL_BUFFER_SIZE-1);
	if (i==_serial_buffer_tail){
		return;
	}
	_serial_buffer[_serial_buffer_head]=c;
	_serial_buffer_head=i;
}



void serial_init(void){
	UCSR0A=1<<U2X0;
	UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
	UBRR0=(F_CPU/4/SERIAL_BAUD_RATE-1)>>1;
	UCSR0B=(1<<RXEN0)|(1<<RXCIE0);
}



_Bool serial_read_packet(packet_t* out){
	_Static_assert(SERIAL_BUFFER_SIZE>=sizeof(packet_t),"Serial buffer too small");
	while (((_serial_buffer_head-_serial_buffer_tail+SERIAL_BUFFER_SIZE)&(SERIAL_BUFFER_SIZE-1))>=sizeof(packet_t)){
		uint8_t checksum=PACKET_CHECKSUM_START_VALUE;
		for (uint8_t i=__builtin_offsetof(packet_t,checksum)+sizeof(out->checksum);i<sizeof(packet_t);i++){
			checksum=packet_process_checksum_byte(checksum,_serial_buffer[(_serial_buffer_tail+i)&(SERIAL_BUFFER_SIZE-1)]);
		}
		if (_serial_buffer[_serial_buffer_tail]!=checksum){
			_serial_buffer_tail=(_serial_buffer_tail+1)&(SERIAL_BUFFER_SIZE-1);
			continue;
		}
		for (uint8_t i=__builtin_offsetof(packet_t,checksum)+sizeof(out->checksum);i<sizeof(packet_t);i++){
			out->_raw_data[i]=_serial_buffer[(_serial_buffer_tail+i)&(SERIAL_BUFFER_SIZE-1)];
		}
		_serial_buffer_tail=(_serial_buffer_tail+sizeof(packet_t))&(SERIAL_BUFFER_SIZE-1);
		return 1;
	}
	return 0;
}
