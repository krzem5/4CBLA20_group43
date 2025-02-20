############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 17/02/2025 by Krzesimir Hyżyk                                 #
############################################################################



import json
import math



CPU_FREQ=16000000
TIMER_DIVISOR=8
TIMER_TICKS=65536

PWM_SEQUENCER_PULSE_ENCODING_FACTOR=10

ANGLE_TO_ENCODED_PULSE=lambda x:round((600+10*max(min(x,180),0))/PWM_SEQUENCER_PULSE_ENCODING_FACTOR)
ENCODED_PULSE_TO_ANGLE=lambda x:(x*PWM_SEQUENCER_PULSE_ENCODING_FACTOR-600)/10

SAMPLE_DELTA=TIMER_TICKS*TIMER_DIVISOR/CPU_FREQ



def compile_sequence(data):
	channel_count=len(data)
	out=bytearray((channel_count+1)<<1)
	last_time=0
	for i in range(0,channel_count):
		out[i+2]=data[i]["pin_a"]|((data[i]["pin_b"] if data[i]["pin_b"] is not None else data[i]["pin_a"])<<4)
		points=data[i]["points"]
		for j in range(0,len(points)):
			last_time=max(last_time,points[j][0])
	sample_count=math.ceil(last_time/SAMPLE_DELTA)+1
	out[0]=channel_count|((sample_count>>8)<<3)
	out[1]=sample_count&0xff
	channel_offsets=[0 for _ in range(0,channel_count)]
	channel_values=[0 for _ in range(0,channel_count)]
	channel_last_token_index=[0 for _ in range(0,channel_count)]
	for i in range(0,sample_count):
		t=i*SAMPLE_DELTA
		for j in range(0,channel_count):
			points=data[j]["points"]
			offset=channel_offsets[j]
			if (offset+1<len(points) and points[offset+1][0]<=t):
				channel_offsets[j]+=1
				offset+=1
			if (offset+1>=len(points)):
				angle=points[offset][1]
			else:
				a,b=points[offset],points[offset+1]
				angle=(0.5-math.cos((t-a[0])/(b[0]-a[0])*math.pi)/2)*(b[1]-a[1])+a[1]
			if (data[j]["pin_b"] is None):
				angle=180-angle
			value=ANGLE_TO_ENCODED_PULSE(angle)
			if (not i):
				out[channel_count+j+2]=value
				channel_values[j]=value
			delta=max(min(value-channel_values[j],16),-16)
			channel_values[j]+=delta
			last_token=out[channel_last_token_index[j]]
			if (not delta):
				if (not i or (last_token&1) or last_token==0xfe):
					channel_last_token_index[j]=len(out)
					out.append(0x00)
				else:
					out[channel_last_token_index[j]]+=0x02
			else:
				token=((abs(delta)-1)<<1)|(delta<0)
				if (not i or not (last_token&1) or ((last_token>>1)&31)!=token or (last_token>>6)==3):
					channel_last_token_index[j]=len(out)
					out.append((token<<1)+1)
				else:
					out[channel_last_token_index[j]]+=0x40
	return out



def compile_and_write_sequence(dst_file_path,data):
	sequencer_data=compile_sequence(data)
	with open(dst_file_path,"w") as wf:
		wf.write(f"/*\n * Copyright (c) Krzesimir Hyżyk - All Rights Reserved\n * Unauthorized copying of this file, via any medium is strictly prohibited\n * Proprietary and confidential\n * Created on 17/02/2025 by Krzesimir Hyżyk\n */\n\n\n\n#ifndef __SEQUENCER_GENERATED_H_\n#define __SEQUENCER_GENERATED_H_ 1\n#include <common/memory.h>\n#include <stdint.h>\n\n\n\nstatic const ROM_DECL uint8_t sequencer_generated_data[{len(sequencer_data)}]={{")
		for i in range(0,len(sequencer_data)):
			if (not (i&15)):
				wf.write("\n\t")
			wf.write(f"0x{sequencer_data[i]:02x},")
		wf.write(f"\n}};\n\n\n\n#endif\n")



if (__name__=="__main__"):
	with open("../data/sequence.json","r") as rf:
		compile_and_write_sequence("../src/client/include/_sequencer_generated.h",json.loads(rf.read()))
