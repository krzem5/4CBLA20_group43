/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 17/02/2025 by Krzesimir Hyżyk
 */



#include <json/json.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef struct _DESERIALIZATION_CONTEXT{
	const char* data;
	uint64_t offset;
	uint64_t length;
} deserialization_context_t;



static void _skip_whitespace(deserialization_context_t* ctx){
	for (;ctx->offset<ctx->length&&(ctx->data[ctx->offset]==' '||ctx->data[ctx->offset]=='\t'||ctx->data[ctx->offset]=='\r'||ctx->data[ctx->offset]=='\n');ctx->offset++);
}



static char* _deserialize_string(deserialization_context_t* ctx,uint64_t* out_length){
	_skip_whitespace(ctx);
	if (ctx->offset>=ctx->length||ctx->data[ctx->offset]!='\"'){
		return NULL;
	}
	char* out=malloc(1);
	uint64_t length=0;
	for (ctx->offset++;ctx->data[ctx->offset]!='\"';ctx->offset++){
		if (ctx->offset>=ctx->length){
			goto _error;
		}
		char c=ctx->data[ctx->offset];
		if (c=='\\'){
			ctx->offset++;
			if (ctx->offset>=ctx->length){
				goto _error;
			}
			if (ctx->data[ctx->offset]=='t'){
				c='\t';
			}
			else if (ctx->data[ctx->offset]=='n'){
				c='\n';
			}
			else if (ctx->data[ctx->offset]=='r'){
				c='\r';
			}
			else if (ctx->data[ctx->offset]=='\"'){
				c='\"';
			}
			else if (ctx->data[ctx->offset]=='\''){
				c='\'';
			}
			else if (ctx->data[ctx->offset]=='\\'){
				c='\\';
			}
			else if (ctx->data[ctx->offset]=='x'){
				ctx->offset++;
				if (ctx->offset+1>=ctx->length){
					goto _error;
				}
				c=0;
				for (uint32_t i=0;i<2;i++){
					c=(c<<4)+ctx->data[ctx->offset];
					if (ctx->data[ctx->offset]>47&&ctx->data[ctx->offset]<58){
						c-=48;
					}
					else if (ctx->data[ctx->offset]>64&&ctx->data[ctx->offset]<91){
						c-=55;
					}
					else if (ctx->data[ctx->offset]>96&&ctx->data[ctx->offset]<123){
						c-=87;
					}
					else{
						goto _error;
					}
					ctx->offset++;
				}
			}
			else if (ctx->data[ctx->offset]=='u'){
				ctx->offset++;
				if (ctx->offset+1>=ctx->length){
					goto _error;
				}
				uint32_t code=0;
				for (uint32_t i=0;i<4;i++){
					code=(code<<4)+ctx->data[ctx->offset];
					if (ctx->data[ctx->offset]>47&&ctx->data[ctx->offset]<58){
						code-=48;
					}
					else if (ctx->data[ctx->offset]>64&&ctx->data[ctx->offset]<91){
						code-=55;
					}
					else if (ctx->data[ctx->offset]>96&&ctx->data[ctx->offset]<123){
						code-=87;
					}
					else{
						goto _error;
					}
					ctx->offset++;
				}
				c=0x80|(code&0x3f);
				if (code<0x0080){
					c=code;
				}
				else if (code<0x0800){
					length++;
					out=realloc(out,length+1);
					out[length-1]=0xc0|(code>>6);
				}
				else{
					length+=2;
					out=realloc(out,length+2);
					out[length-2]=0xe0|(code>>12);
					out[length-1]=0xc0|((code>>6)&0x3f);
				}
			}
			else{
				printf("json_deserialize: unknown escape sequence '\\%c'\n",ctx->data[ctx->offset]);
				goto _error;
			}
		}
		length++;
		out=realloc(out,length+1);
		out[length-1]=c;
	}
	ctx->offset++;
	*out_length=length;
	out[length]=0;
	return out;
_error:
	free(out);
	return NULL;
}



static json_value_t* _deserialize(deserialization_context_t* ctx){
	_skip_whitespace(ctx);
	if (ctx->offset>=ctx->length){
		return NULL;
	}
	if (ctx->data[ctx->offset]=='+'||ctx->data[ctx->offset]=='-'||(ctx->data[ctx->offset]>47&&ctx->data[ctx->offset]<58)){
		_Bool is_negative=0;
		if (ctx->data[ctx->offset]=='+'){
			ctx->offset++;
		}
		else if (ctx->data[ctx->offset]=='-'){
			is_negative=1;
			ctx->offset++;
		}
		int64_t value=0;
		for (;ctx->data[ctx->offset]>47&&ctx->data[ctx->offset]<58;ctx->offset++){
			value=value*10+ctx->data[ctx->offset]-48;
		}
		if (ctx->data[ctx->offset]!='e'&&ctx->data[ctx->offset]!='E'&&ctx->data[ctx->offset]!='.'){
			return json_create_int((is_negative?-value:value));
		}
		printf("json_deserialize: float\n");
		return NULL;
	}
	if (ctx->data[ctx->offset]=='\"'){
		uint64_t length;
		char* value=_deserialize_string(ctx,&length);
		if (!value){
			return NULL;
		}
		return json_create_string(value,length);
	}
	if (ctx->data[ctx->offset]=='['){
		ctx->offset++;
		_skip_whitespace(ctx);
		if (ctx->offset>=ctx->length){
			return NULL;
		}
		if (ctx->data[ctx->offset]==']'){
			ctx->offset++;
			return json_create_array(0);
		}
		json_value_t* out=json_create_array(0);
		while (1){
			json_value_t* value=_deserialize(ctx);
			if (!value){
				json_delete(out);
				return NULL;
			}
			json_append_to_array(out,value);
			_skip_whitespace(ctx);
			if (ctx->offset>=ctx->length){
				json_delete(out);
				return NULL;
			}
			if (ctx->data[ctx->offset]==']'){
				ctx->offset++;
				return out;
			}
			if (ctx->data[ctx->offset]!=','){
				json_delete(out);
				return NULL;
			}
			ctx->offset++;
		}
	}
	if (ctx->data[ctx->offset]=='{'){
		ctx->offset++;
		_skip_whitespace(ctx);
		if (ctx->offset>=ctx->length){
			return NULL;
		}
		if (ctx->data[ctx->offset]=='}'){
			ctx->offset++;
			return json_create_object(0);
		}
		json_value_t* out=json_create_object(0);
		while (1){
			uint64_t key_length;
			char* key=_deserialize_string(ctx,&key_length);
			if (!key){
				json_delete(out);
				return NULL;
			}
			if (strlen(key)!=key_length){
				printf("json_deserialize: object key contains NUL byte\n");
				json_delete(out);
				return NULL;
			}
			_skip_whitespace(ctx);
			if (ctx->offset>=ctx->length||ctx->data[ctx->offset]!=':'){
				free(key);
				json_delete(out);
				return NULL;
			}
			ctx->offset++;
			json_value_t* value=_deserialize(ctx);
			if (!value){
				free(key);
				json_delete(out);
				return NULL;
			}
			json_append_to_object(out,key,value,0);
			_skip_whitespace(ctx);
			if (ctx->offset>=ctx->length){
				json_delete(out);
				return NULL;
			}
			if (ctx->data[ctx->offset]=='}'){
				ctx->offset++;
				return out;
			}
			if (ctx->data[ctx->offset]!=','){
				json_delete(out);
				return NULL;
			}
			ctx->offset++;
		}
	}
	return NULL;
}



json_value_t* json_create_int(int64_t value){
	json_value_t* out=malloc(sizeof(json_value_t));
	out->type=JSON_OBJECT_TYPE_INT;
	out->int_value=value;
	return out;
}



json_value_t* json_create_string(const char* data,uint64_t length){
	if (!length){
		length=strlen(data);
	}
	json_value_t* out=malloc(sizeof(json_value_t));
	out->type=JSON_OBJECT_TYPE_STRING;
	out->string_value.length=length;
	out->string_value.data=malloc(length+1);
	memcpy(out->string_value.data,data,length);
	out->string_value.data[length]=0;
	return out;
}



json_value_t* json_create_array(uint64_t length,...){
	json_value_t* out=malloc(sizeof(json_value_t));
	out->type=JSON_OBJECT_TYPE_ARRAY;
	out->array_value.length=length;
	out->array_value.data=malloc(length*sizeof(json_value_t*));
	va_list va;
	va_start(va,length);
	for (uint64_t i=0;i<length;i++){
		out->array_value.data[i]=va_arg(va,json_value_t*);
	}
	va_end(va);
	return out;
}



json_value_t* json_create_object(uint64_t length,...){
	json_value_t* out=malloc(sizeof(json_value_t));
	out->type=JSON_OBJECT_TYPE_OBJECT;
	out->object_value.length=length;
	out->object_value.data=malloc(length*sizeof(json_object_entry_t));
	va_list va;
	va_start(va,length);
	for (uint64_t i=0;i<length;i++){
		out->object_value.data[i].key=strdup(va_arg(va,const char*));
		out->object_value.data[i].value=va_arg(va,json_value_t*);
	}
	va_end(va);
	return out;
}



void json_delete(json_value_t* value){
	if (!value){
		return;
	}
	if (value->type==JSON_OBJECT_TYPE_STRING){
		free(value->string_value.data);
	}
	else if (value->type==JSON_OBJECT_TYPE_ARRAY){
		for (uint64_t i=0;i<value->array_value.length;i++){
			json_delete(value->array_value.data[i]);
		}
		free(value->array_value.data);
	}
	else if (value->type==JSON_OBJECT_TYPE_OBJECT){
		for (uint64_t i=0;i<value->object_value.length;i++){
			free(value->object_value.data[i].key);
			json_delete(value->object_value.data[i].value);
		}
		free(value->object_value.data);
	}
	free(value);
}



void json_append_to_array(json_value_t* array,json_value_t* value){
	if (array->type!=JSON_OBJECT_TYPE_ARRAY){
		printf("json_append_to_array: invalid argument type\n");
		return;
	}
	array->array_value.length++;
	array->array_value.data=realloc(array->array_value.data,array->array_value.length*sizeof(json_value_t*));
	array->array_value.data[array->array_value.length-1]=value;
}



void json_append_to_object(json_value_t* object,const char* key,json_value_t* value,_Bool duplicate_key){
	if (object->type!=JSON_OBJECT_TYPE_OBJECT){
		printf("json_append_to_object: invalid argument type\n");
		return;
	}
	object->object_value.length++;
	object->object_value.data=realloc(object->object_value.data,object->object_value.length*sizeof(json_object_entry_t));
	object->object_value.data[object->object_value.length-1].key=(duplicate_key?strdup(key):(char*)key);
	object->object_value.data[object->object_value.length-1].value=value;
}



json_value_t* json_get(json_value_t* value,const char* path){
	if (!value){
		return NULL;
	}
	if (!path||!path[0]){
		return value;
	}
	for (;path[0];path++){
		if (path[0]=='['){
			if (value->type!=JSON_OBJECT_TYPE_ARRAY){
				return NULL;
			}
			path++;
			if (path[0]<48||path[0]>57){
				printf("json_get: invalid array index\n");
				return NULL;
			}
			uint64_t index=0;
			for (;path[0]>47&&path[0]<58;path++){
				index=index*10+path[0]-48;
			}
			if (path[0]!=']'){
				printf("json_get: invalid array index\n");
				return NULL;
			}
			if (value->array_value.length<=index){
				return NULL;
			}
			value=value->array_value.data[index];
		}
		else if (path[0]=='.'){
			if (value->type!=JSON_OBJECT_TYPE_OBJECT){
				return NULL;
			}
			char* key=malloc(1);
			uint64_t key_length=0;
			for (path++;path[0]&&path[0]!='.'&&path[0]!='[';path++){
				char c=path[0];
				if (c=='\\'){
					path++;
					if (!path[0]){
						free(key);
						return NULL;
					}
					c=path[0];
				}
				key_length++;
				key=realloc(key,key_length+1);
				key[key_length-1]=c;
			}
			key[key_length]=0;
			path--;
			for (uint64_t i=0;i<value->object_value.length;i++){
				if (!strcmp(value->object_value.data[i].key,key)){
					value=value->object_value.data[i].value;
					free(key);
					goto _key_found;
				}
			}
			free(key);
			return NULL;
_key_found:;
		}
		else{
			printf("json_get: invalid path character: '%c'\n",path[0]);
			return NULL;
		}
	}
	return value;
}



json_value_t* json_deserialize(const char* data,uint64_t length){
	deserialization_context_t ctx={
		data,
		0,
		length
	};
	return _deserialize(&ctx);
}
