/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include <common/packet.h>
#include <ds4/ds4.h>
#include <math.h>
#include <poll.h>
#include <serial/serial.h>
#include <stdint.h>
#include <stdio.h>
#include <terminal/terminal.h>
#include <unistd.h>



#define FLAG_EXIT_PROGRAM 1
#define FLAG_ESTOP_ENABLED 2
#define FLAG_ESTOP_BUTTON_DOWN 4
#define FLAG_CAMERA_FAST 8
#define FLAG_CAMERA_FAST_BUTTON_DOWN 16
#define FLAG_PORTB_ENABLE 32
#define FLAG_PORTB_ENABLE_BUTTON_DOWN 64
#define FLAG_BUZZER 128



static uint32_t _flags=0;
static uint8_t _manual_control_left_wheel=90;
static uint8_t _manual_control_right_wheel=90;
static uint8_t _manual_control_linkage_middle=90;
static uint16_t _manual_control_linkage_final=900;
static uint8_t _manual_control_camera_yaw=90;
static uint8_t _manual_control_camera_pitch=90;



static void _send_estop_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_ESTOP
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _send_manual_input_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_MANUAL_INPUT,
		.manual_input={
			.wheel_left=_manual_control_left_wheel,
			.wheel_right=_manual_control_right_wheel,
			.linkage_middle=_manual_control_linkage_middle,
			.linkage_final_and_buzzer=(_manual_control_linkage_final/10)|((_flags&FLAG_BUZZER)?128:0),
			.camera_yaw=180-_manual_control_camera_yaw,
			.camera_pitch=180-_manual_control_camera_pitch
		}
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _send_sequence_start_packet(void){
	packet_t packet={
		.type=PACKET_TYPE_SEQUENCE_START
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _send_portb_control_packet(_Bool enable){
	packet_t packet={
		.type=(enable?PACKET_TYPE_PORTB_ENABLE:PACKET_TYPE_PORTB_DISABLE)
	};
	packet_generate_checksum(&packet);
	serial_send(&packet,sizeof(packet_t));
}



static void _toggle_estop(void){
	if (_flags&FLAG_ESTOP_BUTTON_DOWN){
		return;
	}
	_flags=(_flags^FLAG_ESTOP_ENABLED)|FLAG_ESTOP_BUTTON_DOWN;
	if (_flags&FLAG_ESTOP_ENABLED){
		_send_estop_packet();
	}
}



static void _process_terminal_command(void){
	uint16_t command=terminal_get_command();
	if (command==0x03){
		_flags|=FLAG_EXIT_PROGRAM;
		return;
	}
	if (command==' '){
		_toggle_estop();
		return;
	}
	_flags&=~FLAG_ESTOP_BUTTON_DOWN;
	if (_flags&FLAG_ESTOP_ENABLED){
		return;
	}
	if (command=='s'){
		_send_sequence_start_packet();
	}
}



static void _process_controller_command(ds4_device_t* controller){
	ds4_recv(controller);
	if (_flags&FLAG_ESTOP_ENABLED){
		controller->led_red=0xff;
		controller->led_green=0x00;
		controller->led_blue=0x00;
	}
	else if (_flags&FLAG_PORTB_ENABLE){
		controller->led_red=0x00;
		controller->led_green=0xff;
		controller->led_blue=0x00;
	}
	else{
		controller->led_red=0x00;
		controller->led_green=0x00;
		controller->led_blue=0xff;
	}
	ds4_send(controller);
	if (controller->buttons&DS4_BUTTON_LOGO){
		_flags|=FLAG_EXIT_PROGRAM;
		return;
	}
	if (controller->buttons&(DS4_BUTTON_CIRCLE|DS4_BUTTON_TOUCHPAD)){
		_toggle_estop();
		return;
	}
	_flags&=~FLAG_ESTOP_BUTTON_DOWN;
	if (_flags&FLAG_ESTOP_ENABLED){
		return;
	}
	if (controller->buttons&DS4_BUTTON_CROSS){
		_send_sequence_start_packet();
		return;
	}
	if (controller->buttons&DS4_BUTTON_SQUARE){
		if (!(_flags&FLAG_PORTB_ENABLE_BUTTON_DOWN)){
			_flags^=FLAG_PORTB_ENABLE;
			_flags|=FLAG_PORTB_ENABLE_BUTTON_DOWN;
			_send_portb_control_packet(!!(_flags&FLAG_PORTB_ENABLE));
		}
	}
	else{
		_flags&=~FLAG_PORTB_ENABLE_BUTTON_DOWN;
	}
	if (controller->buttons&DS4_BUTTON_R1){
		_flags|=FLAG_BUZZER;
	}
	else{
		_flags&=~FLAG_BUZZER;
	}
	float p=controller->lx/100.0f;
	float q=controller->ly/100.0f;
	float r=hypotf(p,q);
	if (r<0.3f){
		_manual_control_left_wheel=90;
		_manual_control_right_wheel=90;
	}
	else{
		int8_t u=(r>1.0f?1.0f:r)*90;
		int8_t v=(q*q-p*p)/r*90*(r>1.0f?1/r:1.0f);
		if (p>=0){
			if (q>=0){
				_manual_control_left_wheel=90+u;
				_manual_control_right_wheel=90+v;
			}
			else{
				_manual_control_left_wheel=90-v;
				_manual_control_right_wheel=90-u;
			}
		}
		else{
			if (q>=0){
				_manual_control_left_wheel=90+v;
				_manual_control_right_wheel=90+u;
			}
			else{
				_manual_control_left_wheel=90-u;
				_manual_control_right_wheel=90-v;
			}
		}
	}
	if (controller->buttons&DS4_BUTTON_TRIANGLE){
		if (!(_flags&FLAG_CAMERA_FAST_BUTTON_DOWN)){
			_flags^=FLAG_CAMERA_FAST;
			_flags|=FLAG_CAMERA_FAST_BUTTON_DOWN;
		}
	}
	else{
		_flags&=~FLAG_CAMERA_FAST_BUTTON_DOWN;
	}
	if (_flags&FLAG_CAMERA_FAST){
		int16_t x=_manual_control_camera_yaw+(-45<controller->rx&&controller->rx<45?0:controller->rx/100);
		int16_t y=_manual_control_camera_pitch+(-45<controller->ry&&controller->ry<45?0:controller->ry/100);
		_manual_control_camera_yaw=(x<0?0:(x>180?180:x));
		_manual_control_camera_pitch=(y<30?30:(y>150?150:y));
	}
	else{
		_manual_control_camera_yaw=(controller->rx+128)*180/255;
		_manual_control_camera_pitch=(controller->ry+128)*120/255+30;
	}
	if (controller->buttons&DS4_BUTTON_UP){
		_manual_control_linkage_middle=45;
	}
	else if (controller->buttons&DS4_BUTTON_DOWN){
		_manual_control_linkage_middle=135;
	}
	else{
		_manual_control_linkage_middle=90;
	}
	if ((controller->buttons&DS4_BUTTON_LEFT)&&_manual_control_linkage_final){
		_manual_control_linkage_final--;
	}
	if ((controller->buttons&DS4_BUTTON_RIGHT)&&_manual_control_linkage_final<900){
		_manual_control_linkage_final++;
	}
	_send_manual_input_packet();
}



static void _update_ui(const ds4_device_t* controller){
	printf("\x1b[2K\r\x1b[0mL: \x1b[1;95m%3u\x1b[0m, R: \x1b[1;95m%3u\x1b[0m, \x1b[0mA: \x1b[1;95m%3u\x1b[0m, B: \x1b[1;95m%3u\x1b[0m, \x1b[0mY: \x1b[1;95m%3u\x1b[0m, P: \x1b[1;95m%3u\x1b[0m",_manual_control_left_wheel,_manual_control_right_wheel,_manual_control_linkage_middle,_manual_control_linkage_final/10,_manual_control_camera_yaw,_manual_control_camera_pitch);
	fflush(stdout);
}



int main(void){
	serial_init();
	terminal_init();
	ds4_device_t controller;
	ds4_init(&controller);
	struct pollfd fds[2]={
		{
			.fd=0,
			.events=POLLIN,
			.revents=0
		},
		{
			.fd=controller.fd,
			.events=POLLIN,
			.revents=0
		}
	};
	printf("\x1b[?25l");
	_send_portb_control_packet(0);
	while (!(_flags&FLAG_EXIT_PROGRAM)&&poll(fds,1+(controller.fd>=0),-1)>=0&&!((fds[0].revents|fds[1].revents)&(POLLERR|POLLHUP|POLLNVAL))){
		if (fds[0].revents&POLLIN){
			_process_terminal_command();
		}
		if (fds[1].revents&POLLIN){
			_process_controller_command(&controller);
		}
		_update_ui(&controller);
	}
	printf("\x1b[?25h\r\n");
	_send_estop_packet();
	_send_portb_control_packet(0);
	ds4_deinit(&controller);
	terminal_deinit();
	serial_deinit();
	return 0;
}
