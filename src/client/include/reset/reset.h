/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 24/02/2025 by Krzesimir Hyżyk
 */



#ifndef _RESET_RESET_H_
#define _RESET_RESET_H_ 1
#include <stdint.h>



void reset_init(uint8_t left,uint8_t right);



void reset_start(uint8_t flags);



void reset_stop(void);



uint8_t reset_get_flags(void);



_Bool reset_is_enabled(void);



_Bool reset_is_left_enabled(void);



_Bool reset_is_right_enabled(void);



#endif
