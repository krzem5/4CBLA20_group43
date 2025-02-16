/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <serial/serial.h>
#include <stdint.h>



#define SERIAL_BUFFER_SIZE 32



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



_Bool serial_read(uint8_t* buffer,uint8_t length){
	if (((_serial_buffer_head-_serial_buffer_tail+SERIAL_BUFFER_SIZE)&(SERIAL_BUFFER_SIZE-1))<length){
		return 0;
	}
	for (uint8_t i=0;i<length;i++){
		buffer[i]=_serial_buffer[_serial_buffer_tail];
		_serial_buffer_tail=(_serial_buffer_tail+1)&(SERIAL_BUFFER_SIZE-1);
	}
	return 1;
}
