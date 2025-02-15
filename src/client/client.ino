/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */




static unsigned char _packet[1];



void setup(void){
	Serial.begin(115200);
	pinMode(LED_BUILTIN,OUTPUT);
}



void loop(void){
	if (Serial.available()>0){
		if (Serial.readBytes(_packet,sizeof(_packet))<sizeof(_packet)){
			return;
		}
		digitalWrite(LED_BUILTIN,_packet[0]&1);
	}
}
