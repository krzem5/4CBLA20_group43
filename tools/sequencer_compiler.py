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

PWM_SEQUENCER_PIN_FLAG_INVERTED=0x40
PWM_SEQUENCER_PIN_FLAG_LAST=0x80

PWM_SEQUENCER_PULSE_ENCODING_FACTOR=10

ANGLE_TO_ENCODED_PULSE=lambda x:round((600+10*max(min(x,180),0))/PWM_SEQUENCER_PULSE_ENCODING_FACTOR)



def _rasterize_channel(points,sample_count,sample_delta):
	out=[]
	offset=0
	for i in range(0,sample_count):
		t=i*sample_delta
		if (offset+1<len(points) and points[offset+1][0]<=t):
			offset+=1
		if (offset+1>=len(points)):
			angle=points[offset][1]
		else:
			a,b=points[offset],points[offset+1]
			angle=(0.5-math.cos((t-a[0])/(b[0]-a[0])*math.pi)/2)*(b[1]-a[1])+a[1]
		out.append(ANGLE_TO_ENCODED_PULSE(angle))
	return out



def _reencode_channel(points):
	value=0
	out=[]
	for i in range(0,len(points)):
		delta=points[i]-value
		if (delta<-16):
			delta=-16
		elif (delta>16):
			delta=16
		value+=delta
		out.append(delta)
	return out



def _compress_channel(points):
	out=[]
	for i in range(0,len(points)):
		if (not points[i]):
			if (not i or (out[-1]&1) or out[-1]==0xfe):
				out.append(0x00)
			else:
				out[-1]+=2
		else:
			token=((abs(points[i])-1)<<1)|(points[i]<0)
			if (not i or not (out[-1]&1) or ((out[-1]>>1)&31)!=token or (out[-1]>>6)==3):
				out.append((token<<1)+1)
			else:
				out[-1]+=0x40
	return out



def compile_sequence(dst_file_path,data):
	channel_count=len(data)
	sequencer_data=[0,0]
	last_time=0
	for i in range(0,channel_count):
		pins=sorted(data[i]["pins"],reverse=True)
		is_inverted=False
		for j in range(0,len(pins)):
			sequencer_data.append((pins[j]^(pins[j]>>7))|(PWM_SEQUENCER_PIN_FLAG_LAST if j==len(pins)-1 else 0))
			if (pins[j]<0 and not is_inverted):
				is_inverted=True
				sequencer_data[-1]|=PWM_SEQUENCER_PIN_FLAG_INVERTED
		points=data[i]["points"]
		for j in range(0,len(points)):
			last_time=max(last_time,points[j][0])
	sample_delta=TIMER_TICKS*TIMER_DIVISOR/CPU_FREQ
	sample_count=math.ceil(last_time/sample_delta)+1
	sequencer_data[0]=channel_count|((sample_count>>8)<<3)
	sequencer_data[1]=sample_count&0xff
	channel_points=[_rasterize_channel(data[i]["points"],sample_count,sample_delta) for i in range(0,channel_count)]
	for i in range(0,channel_count):
		channel_points[i]=_reencode_channel(channel_points[i])
		channel_points[i]=_compress_channel(channel_points[i])
	sequencer_data.extend(channel_points[0])
	with open(dst_file_path,"w") as wf:
		wf.write(f"/*\n * Copyright (c) Krzesimir Hyżyk - All Rights Reserved\n * Unauthorized copying of this file, via any medium is strictly prohibited\n * Proprietary and confidential\n * Created on 17/02/2025 by Krzesimir Hyżyk\n */\n\n\n\n#ifndef __SEQUENCER_GENERATED_H_\n#define __SEQUENCER_GENERATED_H_ 1\n#include <common/memory.h>\n#include <stdint.h>\n\n\n\nstatic const ROM_DECL uint8_t sequencer_generated_data[{len(sequencer_data)}]={{")
		for i in range(0,len(sequencer_data)):
			if (not (i&15)):
				wf.write("\n\t")
			wf.write(f"0x{sequencer_data[i]:02x},")
		wf.write(f"\n}};\n\n\n\n#endif\n")



with open("../data/sequence.json","r") as rf:
	compile_sequence("../src/client/include/_sequencer_generated.h",json.loads(rf.read()))
