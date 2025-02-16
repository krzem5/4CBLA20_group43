/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <avr/io.h>
#include <common/memory.h>
#include <stdint.h>



const ROM_DECL uint16_t _gpio_pin_to_dir_reg[]={
	(uint16_t)(&DDRD),(uint16_t)(&DDRD),(uint16_t)(&DDRD),(uint16_t)(&DDRD),
	(uint16_t)(&DDRB),(uint16_t)(&DDRB),(uint16_t)(&DDRB),
	(uint16_t)(&DDRC),(uint16_t)(&DDRC),(uint16_t)(&DDRC)
};
const ROM_DECL uint16_t _gpio_pin_to_output_reg[]={
	(uint16_t)(&PORTD),(uint16_t)(&PORTD),(uint16_t)(&PORTD),(uint16_t)(&PORTD),
	(uint16_t)(&PORTB),(uint16_t)(&PORTB),(uint16_t)(&PORTB),
	(uint16_t)(&PORTC),(uint16_t)(&PORTC),(uint16_t)(&PORTC)
};
const ROM_DECL uint8_t _gpio_pin_to_mask[]={
	0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
	0x01,0x02,0x04,0x08,0x10,0x20,
	0x01,0x02,0x04,0x08,0x10,0x20,
};
