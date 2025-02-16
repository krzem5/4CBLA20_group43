/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 16/02/2025 by Krzesimir Hyżyk
 */



#include <ds4/ds4.h>
#include <fcntl.h>
#include <libudev.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <util/util.h>



void ds4_init(ds4_device_t* out){
	out->fd=-1;
	struct udev* udev_ctx=udev_new();
	struct udev_enumerate* dev_list=udev_enumerate_new(udev_ctx);
	udev_enumerate_add_match_subsystem(dev_list,"hidraw");
	udev_enumerate_scan_devices(dev_list);
	struct udev_list_entry* entry=udev_enumerate_get_list_entry(dev_list);
	while (out->fd<0&&entry){
		struct udev_device* dev=udev_device_new_from_syspath(udev_ctx,udev_list_entry_get_name(entry));
		struct udev_device* parent=udev_device_get_parent(dev);
		if (!parent||strcmp(udev_device_get_subsystem(parent),"hid")){
			goto _cleanup_entry;
		}
		const char* name=udev_device_get_property_value(parent,"HID_NAME");
		if (!name||(strcmp(name,"Sony Interactive Entertainment Wireless Controller")&&strcmp(name,"Sony Computer Entertainment Wireless Controller"))){
			goto _cleanup_entry;
		}
		const char* path=udev_device_get_devnode(dev);
		if (path){
			out->fd=open(path,O_RDWR|O_NONBLOCK);
		}
_cleanup_entry:
		udev_device_unref(dev);
		entry=udev_list_entry_get_next(entry);
	}
	udev_enumerate_unref(dev_list);
	udev_unref(udev_ctx);
}



void ds4_deinit(ds4_device_t* device){
	if (device->fd>=0){
		close(device->fd);
		device->fd=-1;
	}
}



void ds4_recv(ds4_device_t* device){
	uint8_t buffer[64];
	ASSERT(read(device->fd,buffer,sizeof(buffer))==sizeof(buffer));
	device->buttons=0;
	uint8_t dp=buffer[5]&0x0f;
	if (dp==0||dp==1||dp==7){
		device->buttons|=DS4_BUTTON_UP;
	}
	if (dp>=3&&dp<=5){
		device->buttons|=DS4_BUTTON_DOWN;
	}
	if (dp>=5&&dp<=7){
		device->buttons|=DS4_BUTTON_LEFT;
	}
	if (dp>=1&&dp<=3){
		device->buttons|=DS4_BUTTON_RIGHT;
	}
	if (buffer[5]&32){
		device->buttons|=DS4_BUTTON_CROSS;
	}
	if (buffer[5]&64){
		device->buttons|=DS4_BUTTON_CIRCLE;
	}
	if (buffer[5]&16){
		device->buttons|=DS4_BUTTON_SQURARE;
	}
	if (buffer[5]&128){
		device->buttons|=DS4_BUTTON_TRIANGLE;
	}
	if (buffer[6]&1){
		device->buttons|=DS4_BUTTON_L1;
	}
	if (buffer[6]&2){
		device->buttons|=DS4_BUTTON_R1;
	}
	if (buffer[6]&4){
		device->buttons|=DS4_BUTTON_L2;
	}
	if (buffer[6]&8){
		device->buttons|=DS4_BUTTON_R2;
	}
	if (buffer[6]&16){
		device->buttons|=DS4_BUTTON_SHARE;
	}
	if (buffer[6]&32){
		device->buttons|=DS4_BUTTON_OPTIONS;
	}
	if (buffer[6]&64){
		device->buttons|=DS4_BUTTON_L3;
	}
	if (buffer[6]&128){
		device->buttons|=DS4_BUTTON_R3;
	}
	if (buffer[7]&1){
		device->buttons|=DS4_BUTTON_LOGO;
	}
	if (buffer[7]&2){
		device->buttons|=DS4_BUTTON_TOUCHPAD;
	}
	device->lx=-128+buffer[1];
	device->ly=127-buffer[2];
	device->rx=-128+buffer[3];
	device->ry=127-buffer[4];
	device->l2=buffer[8];
	device->r2=buffer[9];
	device->battery=((buffer[30]&0xf)==11?0:buffer[30]<<5);
}
