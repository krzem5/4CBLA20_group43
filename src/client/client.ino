/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 12/02/2025 by Krzesimir Hyżyk
 */



void setup(void){
	pinMode(LED_BUILTIN,OUTPUT);
}



void loop(void){
	digitalWrite(LED_BUILTIN,1);
	delay(200);
	digitalWrite(LED_BUILTIN,0);
	delay(200);
}
