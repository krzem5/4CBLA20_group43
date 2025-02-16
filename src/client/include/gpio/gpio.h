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



extern const ROM_DECL uint16_t _gpio_to_mode_reg[];
extern const ROM_DECL uint16_t _gpio_to_output_reg[];
extern const ROM_DECL uint8_t _gpio_to_mask[];



static inline volatile uint8_t* _gpio_load_port(const uint16_t* ptr){
	return (volatile uint8_t*)(void*)ROM_LOAD_U16(ptr);
}



static inline void gpio_init(uint8_t pin){
	uint8_t mask=ROM_LOAD_U8(_gpio_to_mask+pin);
	(*_gpio_load_port(_gpio_to_output_reg+pin))&=~mask;
	(*_gpio_load_port(_gpio_to_mode_reg+pin))|=mask;
}



static inline void gpio_write(uint8_t pin,_Bool state){
	uint8_t mask=ROM_LOAD_U8(_gpio_to_mask+pin);
	volatile uint8_t* port=_gpio_load_port(_gpio_to_output_reg+pin);
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
