/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#ifndef _GPIO_GPIO_H_
#define _GPIO_GPIO_H_ 1
#include <common/memory.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif



extern const ROM_DECL uint16_t _gpio_pin_to_dir_reg[];
extern const ROM_DECL uint16_t _gpio_pin_to_output_reg[];
extern const ROM_DECL uint8_t _gpio_pin_to_mask[];



static inline volatile uint8_t* _gpio_load_port(const void* base,uint8_t pin){
	return (volatile uint8_t*)(void*)ROM_LOAD_U16(base+(pin&0xfe));
}



static inline void gpio_init(uint8_t pin,_Bool output,_Bool state){
	uint8_t mask=ROM_LOAD_U8(_gpio_pin_to_mask+pin);
	volatile uint8_t* port=_gpio_load_port(_gpio_pin_to_output_reg,pin);
	if (state){
		(*port)|=mask;
	}
	else{
		(*port)&=~mask;
	}
	port=_gpio_load_port(_gpio_pin_to_dir_reg,pin);
	if (output){
		(*port)|=mask;
	}
	else{
		(*port)&=~mask;
	}
}



static inline void gpio_write(uint8_t pin,_Bool state){
	uint8_t mask=ROM_LOAD_U8(_gpio_pin_to_mask+pin);
	volatile uint8_t* port=_gpio_load_port(_gpio_pin_to_output_reg,pin);
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
