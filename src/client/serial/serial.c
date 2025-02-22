/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <avr/interrupt.h>
#include <avr/io.h>
#include <common/packet.h>
#include <serial/serial.h>
#include <stdint.h>



#define SERIAL_BUFFER_SIZE 16



static volatile uint8_t _serial_buffer_head=0;
static volatile uint8_t _serial_buffer_tail=0;
static volatile uint8_t _serial_buffer[SERIAL_BUFFER_SIZE];



ISR(USART_RX_vect){
	_serial_buffer[_serial_buffer_tail]=UDR0;
	_serial_buffer_tail=(_serial_buffer_tail+1)&(SERIAL_BUFFER_SIZE-1);
	if (_serial_buffer_tail==_serial_buffer_head){
		_serial_buffer_head=(_serial_buffer_head+1)&(SERIAL_BUFFER_SIZE-1);
	}
}



void serial_init(void){ // 2 Mbps
	UCSR0B=0;
	UCSR0A=1<<U2X0;
	UBRR0H=0;
	UBRR0L=0;
	UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
	UCSR0B=(1<<RXEN0)|(1<<RXCIE0);
}



_Bool serial_read_packet(packet_t* out){
	_Static_assert(SERIAL_BUFFER_SIZE>=sizeof(packet_t),"Serial buffer too small");
	_Static_assert(!__builtin_offsetof(packet_t,checksum),"Incorrect checksum placement");
	while (1){
		uint8_t head=_serial_buffer_head;
		if (((_serial_buffer_tail-head+SERIAL_BUFFER_SIZE)&(SERIAL_BUFFER_SIZE-1))<sizeof(packet_t)){
			return 0;
		}
		uint8_t checksum=PACKET_CHECKSUM_START_VALUE;
		out->checksum=_serial_buffer[head];
		for (uint8_t i=__builtin_offsetof(packet_t,checksum)+sizeof(out->checksum);i<sizeof(packet_t);i++){
			uint8_t c=_serial_buffer[(head+i)&(SERIAL_BUFFER_SIZE-1)];
			checksum=packet_process_checksum_byte(checksum,c);
			out->_raw_data[i]=c;
		}
		if (out->checksum==checksum){
			_serial_buffer_head=(head+sizeof(packet_t))&(SERIAL_BUFFER_SIZE-1);
			return 1;
		}
		_serial_buffer_head=(head+1)&(SERIAL_BUFFER_SIZE-1);
	}
}
