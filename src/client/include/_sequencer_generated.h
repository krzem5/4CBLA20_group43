/*
 * Copyright (c) Krzesimir Hyżyk - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Created on 17/02/2025 by Krzesimir Hyżyk
 */



#ifndef __SEQUENCER_GENERATED_H_
#define __SEQUENCER_GENERATED_H_ 1
#include <common/memory.h>
#include <pwm/sequencer.h>
#include <stdint.h>
#if F_CPU!=16000000
#error Unsupported CPU frequency
#endif



static const ROM_DECL uint8_t _sequencer_data[50]={
	0x00,0xc1,0x3c,0x1e,0x3d,0x3f,0x42,0x45,0x4a,0x4f,0x55,0x5c,0x64,0x6c,0x74,0x7d,
	0x86,0x8f,0x98,0xa1,0xaa,0xb3,0xbc,0xc4,0xcc,0xd3,0xd9,0xdf,0xe4,0xe8,0xec,0xee,
	0xef,0xf0,0x1e,0xee,0xea,0xe5,0xde,0xd7,0xce,0xc5,0xbb,0xb3,0xaa,0xa3,0x9d,0x99,
	0x97,0x96,
};



static uint8_t _sequencer_scratch_data[2];



static const pwm_sequencer_data_t sequencer_data={
	1,
	108,
	_sequencer_data,
	_sequencer_scratch_data
};



#endif
