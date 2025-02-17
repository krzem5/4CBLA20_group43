/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 17/02/2025 by Krzesimir Hyżyk
 */



#include <json/json.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



int main(int argc,const char*const* argv){
	if (argc<3){
		printf("Usage:\n\n%s <input> <output>\n",(argc?argv[0]:"sequencer_compiler"));
		return 1;
	}
	FILE* in=fopen(argv[1],"rb");
	if (!in){
		printf("Unable to open input file '%s'\n",argv[1]);
		return 1;
	}
	FILE* out=fopen(argv[2],"wb");
	if (!out){
		fclose(in);
		printf("Unable to open output file '%s'\n",argv[2]);
		return 1;
	}
	fseek(in,0,SEEK_END);
	uint64_t in_length=ftell(in);
	fseek(in,0,SEEK_SET);
	void* in_data=malloc(in_length);
	if (fread(in_data,1,in_length,in)!=in_length){
		free(in_data);
		fclose(in);
		fclose(out);
		printf("Unable to read input file '%s'\n",argv[1]);
		return 1;
	}
	fclose(in);
	json_value_t* data=json_deserialize(in_data,in_length);
	free(in_data);
	printf("%p\n",data);
	json_delete(data);
	fclose(out);
	printf("Unable to compile file\n");
	return 0;
}
