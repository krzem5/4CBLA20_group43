/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#ifndef _GPIO_GPIO_H_
#define _GPIO_GPIO_H_ 1
#include <avr/io.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif



static inline uint8_t _gpio_pin_to_mask(uint8_t pin){
	return 1<<(pin&7);
}



static inline volatile uint8_t* _gpio_pin_to_dir_reg(uint8_t pin){
	return (pin>7?&DDRB:&DDRD);
}



static inline volatile uint8_t* _gpio_pin_to_output_reg(uint8_t pin){
	return (pin>7?&PORTB:&PORTD);
}



static inline void gpio_init(uint8_t pin,_Bool output){
	uint8_t mask=_gpio_pin_to_mask(pin);
	volatile uint8_t* port=_gpio_pin_to_dir_reg(pin);
	if (output){
		(*port)|=mask;
	}
	else{
		(*port)&=~mask;
	}
}



static inline void gpio_write(uint8_t pin,_Bool state){
	uint8_t mask=_gpio_pin_to_mask(pin);
	volatile uint8_t* port=_gpio_pin_to_output_reg(pin);
	if (state){
		(*port)|=mask;
	}
	else{
		(*port)&=~mask;
	}
}



#ifdef __cplusplus
}
#endif
#endif
