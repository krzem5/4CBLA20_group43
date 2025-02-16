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
#define ROM_DECL PROGMEM
#define ROM_LOAD(ptr) (pgm_read_word_near((ptr)))
#else
#define ROM_DECL
#define ROM_LOAD(ptr) (*((const uint16_t*)(ptr)))
#endif



#endif
