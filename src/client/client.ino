/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



#include "packet.h"



static packet_t _packet;



void setup(void){
	Serial.begin(115200);
	pinMode(LED_BUILTIN,OUTPUT);
}



void loop(void){
	if (Serial.available()<sizeof(packet_t)||Serial.readBytes(_packet._bytes,sizeof(packet_t))<sizeof(packet_t)){
		return;
	}
	if (_packet.checksum!=packet_compute_checksum(&_packet)){
		while (Serial.available()>0&&Serial.read()>=0);
		return;
	}
	digitalWrite(LED_BUILTIN,!!_packet.led_state);
}
