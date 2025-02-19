/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#ifndef _DS4_DS4_H_
#define _DS4_DS4_H_ 1
#include <stdint.h>



#define DS4_BUTTON_UP 0x00001
#define DS4_BUTTON_DOWN 0x00002
#define DS4_BUTTON_LEFT 0x00004
#define DS4_BUTTON_RIGHT 0x00008
#define DS4_BUTTON_L1 0x00010
#define DS4_BUTTON_R1 0x00020
#define DS4_BUTTON_L2 0x00040
#define DS4_BUTTON_R2 0x00080
#define DS4_BUTTON_L3 0x00100
#define DS4_BUTTON_R3 0x00200
#define DS4_BUTTON_CROSS 0x00400
#define DS4_BUTTON_CIRCLE 0x00800
#define DS4_BUTTON_SQUARE 0x01000
#define DS4_BUTTON_TRIANGLE 0x02000
#define DS4_BUTTON_OPTIONS 0x04000
#define DS4_BUTTON_SHARE 0x08000
#define DS4_BUTTON_LOGO 0x10000
#define DS4_BUTTON_TOUCHPAD 0x20000



typedef struct _DS4_DEVICE{
	int fd;
	uint8_t has_wired_connection;
	uint32_t buttons;
	int8_t lx;
	int8_t ly;
	int8_t rx;
	int8_t ry;
	uint8_t l2;
	uint8_t r2;
	uint8_t led_red;
	uint8_t led_green;
	uint8_t led_blue;
	uint8_t rumble_big;
	uint8_t rumble_small;
} ds4_device_t;



void ds4_init(ds4_device_t* out);



void ds4_deinit(ds4_device_t* device);



void ds4_recv(ds4_device_t* device);



void ds4_send(const ds4_device_t* device);



#endif
