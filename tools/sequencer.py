############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 17/02/2025 by Krzesimir Hyżyk                                 #
############################################################################



import math



CPU_FREQ=16000000
TIMER_DIVISOR=8
TIMER_TICKS=65536

PWM_SEQUENCER_PIN_FLAG_INVERTED=0x40
PWM_SEQUENCER_PIN_FLAG_LAST=0x80

PWM_SEQUENCER_PULSE_ENCODING_FACTOR=10
PWM_SEQUENCER_PULSE_ENCODING_CUTOFF=59

ANGLE_TO_ENCODED_PULSE=lambda x:round((600+10*max(min(x,180),0))/PWM_SEQUENCER_PULSE_ENCODING_FACTOR)

if (ANGLE_TO_ENCODED_PULSE(0)!=PWM_SEQUENCER_PULSE_ENCODING_CUTOFF+1):
	raise RuntimeError("Incorrect cutoff")



def _generate_sequencer_header(dst_file_path,data):
	channel_count=len(data)
	sequencer_data=[channel_count,0,0]
	last_time=0
	for channel in data:
		for i,k in enumerate(channel["pins"]):
			sequencer_data.append(abs(k)+(PWM_SEQUENCER_PIN_FLAG_INVERTED if k<0 else 0)+(PWM_SEQUENCER_PIN_FLAG_LAST if i==len(channel["pins"])-1 else 0))
		for k in channel["points"]:
			last_time=max(last_time,k[0])
	sample_delta=TIMER_TICKS*TIMER_DIVISOR/CPU_FREQ
	sample_count=math.ceil(last_time/sample_delta)+1
	sequencer_data[1]=sample_count&0xff
	sequencer_data[2]=sample_count>>8
	last_channel_values=[-1 for _ in range(0,channel_count)]
	last_channel_offsets=[0 for _ in range(0,channel_count)]
	last_channel_repeat_byte_index=[0 for _ in range(0,channel_count)]
	for i in range(0,sample_count):
		t=i*sample_delta
		for j in range(0,channel_count):
			points=data[j]["points"]
			if (last_channel_offsets[j]+1<len(points) and points[last_channel_offsets[j]+1][0]<=t):
				last_channel_offsets[j]+=1
			if (last_channel_offsets[j]+1>=len(points)):
				angle=points[last_channel_offsets[j]][1]
			else:
				a,b=points[last_channel_offsets[j]],points[last_channel_offsets[j]+1]
				angle=(0.5-math.cos((t-a[0])/(b[0]-a[0])*math.pi)/2)*(b[1]-a[1])+a[1]
			encoded_pulse=ANGLE_TO_ENCODED_PULSE(angle)
			if (last_channel_values[j]!=encoded_pulse):
				last_channel_values[j]=encoded_pulse
				last_channel_repeat_byte_index[j]=0
				sequencer_data.append(encoded_pulse)
				continue
			if (last_channel_repeat_byte_index[j] and sequencer_data[last_channel_repeat_byte_index[j]]<PWM_SEQUENCER_PULSE_ENCODING_CUTOFF):
				sequencer_data[last_channel_repeat_byte_index[j]]+=1
			else:
				last_channel_repeat_byte_index[j]=len(sequencer_data)
				sequencer_data.append(0)
	with open(dst_file_path,"w") as wf:
		wf.write(f"/*\n * Copyright (c) Krzesimir Hyżyk - All Rights Reserved\n * Unauthorized copying of this file, via any medium is strictly prohibited\n * Proprietary and confidential\n * Created on 17/02/2025 by Krzesimir Hyżyk\n */\n\n\n\n#ifndef __SEQUENCER_GENERATED_H_\n#define __SEQUENCER_GENERATED_H_ 1\n#include <common/memory.h>\n#include <stdint.h>\n#if F_CPU!={CPU_FREQ}\n#error Unsupported CPU frequency\n#endif\n\n\n\nstatic const ROM_DECL uint8_t sequencer_generated_data[{len(sequencer_data)}]={{")
		for i in range(0,len(sequencer_data)):
			if (not (i&15)):
				wf.write("\n\t")
			wf.write(f"0x{sequencer_data[i]:02x},")
		wf.write(f"\n}};\n\n\n\n#endif\n")



_generate_sequencer_header("../src/client/include/_sequencer_generated.h",[
	{
		"pins": [0,-1],
		"points": [
			(0.0,0),
			(1.0,0),
			(2.0,180),
			(3.0,180),
			(3.5,90)
		]
	}
])
