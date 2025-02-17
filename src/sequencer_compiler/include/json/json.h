/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 17/02/2025 by Krzesimir Hyżyk
 */



#ifndef _JSON_JSON_H_
#define _JSON_JSON_H_ 1
#include <stdint.h>



#define JSON_OBJECT_TYPE_INT 0
#define JSON_OBJECT_TYPE_STRING 1
#define JSON_OBJECT_TYPE_ARRAY 2
#define JSON_OBJECT_TYPE_OBJECT 3



struct _JSON_OBJECT_ENTRY;



typedef struct _JSON_VALUE{
	uint32_t type;
	union{
		int64_t int_value;
		struct{
			uint64_t length;
			char* data;
		} string_value;
		struct{
			uint64_t length;
			struct _JSON_VALUE** data;
		} array_value;
		struct{
			uint64_t length;
			struct _JSON_OBJECT_ENTRY* data;
		} object_value;
	};
} json_value_t;



typedef struct _JSON_OBJECT_ENTRY{
	char* key;
	json_value_t* value;
} json_object_entry_t;



json_value_t* json_create_int(int64_t value);



json_value_t* json_create_string(const char* data,uint64_t length);



json_value_t* json_create_array(uint64_t length,...);



json_value_t* json_create_object(uint64_t length,...);



void json_delete(json_value_t* value);



void json_append_to_array(json_value_t* array,json_value_t* value);



void json_append_to_object(json_value_t* object,const char* key,json_value_t* value,_Bool duplicate_key);



json_value_t* json_get(json_value_t* value,const char* path);



json_value_t* json_deserialize(const char* data,uint64_t length);



#endif
