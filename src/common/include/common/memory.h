/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _COMMON_MEMORY_H_
#define _COMMON_MEMORY_H_ 1
#include <stdint.h>



#ifdef ARDUINO
#include <avr/pgmspace.h>
#define ROM_DECL PROGMEM
#define ROM_LOAD_U8(ptr) (pgm_read_byte_near((ptr)))
#define ROM_LOAD_U16(ptr) (pgm_read_word_near((ptr)))
#else
#define ROM_DECL
#define ROM_LOAD_U8(ptr) (*((const uint8_t*)(ptr)))
#define ROM_LOAD_U16(ptr) (*((const uint16_t*)(ptr)))
#endif



#endif
