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
#include <stdio.h>



void ds4_init(ds4_device_t* out){
	out->fd=-1;
	out->buttons=0;
	out->lx=0;
	out->ly=0;
	out->rx=0;
	out->ry=0;
	out->l2=0;
	out->r2=0;
	struct udev* udev_ctx=udev_new();
	struct udev_enumerate* dev_list=udev_enumerate_new(udev_ctx);
	udev_enumerate_add_match_subsystem(dev_list,"hidraw");
	udev_enumerate_scan_devices(dev_list);
	for (struct udev_list_entry* entry=udev_enumerate_get_list_entry(dev_list);out->fd<0&&entry;entry=udev_list_entry_get_next(entry)){
		struct udev_device* dev=udev_device_new_from_syspath(udev_ctx,udev_list_entry_get_name(entry));
		struct udev_device* parent=udev_device_get_parent(dev);
		if (!parent||strcmp(udev_device_get_subsystem(parent),"hid")){
			goto _cleanup_entry;
		}
		const char* name=udev_device_get_property_value(parent,"HID_NAME");
		if (!name||(strcmp(name,"Wireless Controller")&&strcmp(name,"Sony Interactive Entertainment Wireless Controller")&&strcmp(name,"Sony Computer Entertainment Wireless Controller"))){
			goto _cleanup_entry;
		}
		const char* path=udev_device_get_devnode(dev);
		if (path){
			out->fd=open(path,O_RDWR|O_NONBLOCK);
		}
_cleanup_entry:
		udev_device_unref(dev);
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
	const uint8_t* ptr=buffer+(buffer[0]==0x11?2:0);
	device->buttons=0;
	uint8_t dp=ptr[5]&0x0f;
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
	if (ptr[5]&32){
		device->buttons|=DS4_BUTTON_CROSS;
	}
	if (ptr[5]&64){
		device->buttons|=DS4_BUTTON_CIRCLE;
	}
	if (ptr[5]&16){
		device->buttons|=DS4_BUTTON_SQURARE;
	}
	if (ptr[5]&128){
		device->buttons|=DS4_BUTTON_TRIANGLE;
	}
	if (ptr[6]&1){
		device->buttons|=DS4_BUTTON_L1;
	}
	if (ptr[6]&2){
		device->buttons|=DS4_BUTTON_R1;
	}
	if (ptr[6]&4){
		device->buttons|=DS4_BUTTON_L2;
	}
	if (ptr[6]&8){
		device->buttons|=DS4_BUTTON_R2;
	}
	if (ptr[6]&16){
		device->buttons|=DS4_BUTTON_SHARE;
	}
	if (ptr[6]&32){
		device->buttons|=DS4_BUTTON_OPTIONS;
	}
	if (ptr[6]&64){
		device->buttons|=DS4_BUTTON_L3;
	}
	if (ptr[6]&128){
		device->buttons|=DS4_BUTTON_R3;
	}
	if (ptr[7]&1){
		device->buttons|=DS4_BUTTON_LOGO;
	}
	if (ptr[7]&2){
		device->buttons|=DS4_BUTTON_TOUCHPAD;
	}
	device->lx=-128+ptr[1];
	device->ly=127-ptr[2];
	device->rx=-128+ptr[3];
	device->ry=127-ptr[4];
	device->l2=ptr[8];
	device->r2=ptr[9];
}
