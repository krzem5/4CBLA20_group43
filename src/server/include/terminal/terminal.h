/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 15/02/2025 by Krzesimir Hyżyk
 */



#ifndef _TERMINAL_TERMINAL_H_
#define _TERMINAL_TERMINAL_H_ 1
#include <stdint.h>



void terminal_init(void);



void terminal_deinit(void);



uint16_t terminal_get_command(void);



#endif
