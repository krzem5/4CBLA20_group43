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



#define BAUD SERIAL_BAUD_RATE
#define BAUD_TOL 3
#include <util/setbaud.h>
#if USE_2X
#define UCSR0A_INIT_VALUE (1<<U2X0)
#else
#define UCSR0A_INIT_VALUE 0
#endif
#define UBRR0H_INIT_VALUE UBRRH_VALUE
#define UBRR0L_INIT_VALUE UBRRL_VALUE



static volatile uint8_t _serial_buffer_head=0;
static volatile uint8_t _serial_buffer_tail=0;
static uint8_t _serial_buffer[SERIAL_BUFFER_SIZE];



ISR(USART_RX_vect){
	uint8_t c=UDR0;
	if (UCSR0A&(1<<UPE0)){
		return;
	}
	uint8_t i=(_serial_buffer_tail+1)&(SERIAL_BUFFER_SIZE-1);
	if (i==_serial_buffer_head){
		return;
	}
	_serial_buffer[_serial_buffer_tail]=c;
	_serial_buffer_tail=i;
}



void serial_init(void){
	UCSR0B=0;
	UCSR0A=UCSR0A_INIT_VALUE;
	UBRR0H=UBRR0H_INIT_VALUE;
	UBRR0L=UBRR0L_INIT_VALUE;
	UCSR0C=(1<<UCSZ00)|(1<<UCSZ01);
	UCSR0B=(1<<RXEN0)|(1<<RXCIE0);
}



_Bool serial_read_packet(packet_t* out){
	_Static_assert(SERIAL_BUFFER_SIZE>=sizeof(packet_t),"Serial buffer too small");
	_Static_assert(!__builtin_offsetof(packet_t,checksum),"Incorrect checksum placement");
	while (((_serial_buffer_tail-_serial_buffer_head+SERIAL_BUFFER_SIZE)&(SERIAL_BUFFER_SIZE-1))>=sizeof(packet_t)){
		uint8_t checksum=PACKET_CHECKSUM_START_VALUE;
		for (uint8_t i=__builtin_offsetof(packet_t,checksum)+sizeof(out->checksum);i<sizeof(packet_t);i++){
			checksum=packet_process_checksum_byte(checksum,_serial_buffer[(_serial_buffer_head+i)&(SERIAL_BUFFER_SIZE-1)]);
		}
		if (_serial_buffer[_serial_buffer_head]!=checksum){
			_serial_buffer_head=(_serial_buffer_head+1)&(SERIAL_BUFFER_SIZE-1);
			continue;
		}
		for (uint8_t i=__builtin_offsetof(packet_t,checksum)+sizeof(out->checksum);i<sizeof(packet_t);i++){
			out->_raw_data[i]=_serial_buffer[(_serial_buffer_head+i)&(SERIAL_BUFFER_SIZE-1)];
		}
		_serial_buffer_head=(_serial_buffer_head+sizeof(packet_t))&(SERIAL_BUFFER_SIZE-1);
		return 1;
	}
	return 0;
}
