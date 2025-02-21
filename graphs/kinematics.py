############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 20/02/2025 by Krzesimir Hyżyk                                 #
############################################################################



__import__("sys").path.insert(0,"../tools")
import json
import math
import matplotlib.pyplot as plt
import sequencer_compiler



FILE_PATH="../data/sequence.json"

CHANNELS=[
	("#1f77b4","1\\textsuperscript{st} linkage"),
	("#2ca02c","2\\textsuperscript{nd} linkage"),
	("#d62728","3\\textsuperscript{rd} linkage"),
]

# #1f77b4 #ff7f0e #2ca02c #d62728 #9467bd #8c564b #e377c2 #7f7f7f #bcbd22 #17becf



DELTA_TIME=0.001



plt.rcParams["text.usetex"]=True
fig,ax=plt.subplots(1)
end_time=0
with open(FILE_PATH,"r") as rf:
	data=json.loads(rf.read())
	for index,channel in enumerate(data):
		points=channel["points"]
		x=[]
		y=[]
		for i in range(0,len(points)-1):
			a,b=points[i:i+2]
			t=a[0]
			while (t<b[0]):
				x.append(t)
				y.append((0.5-math.cos((t-a[0])/(b[0]-a[0])*math.pi)/2)*(b[1]-a[1])+a[1])
				t+=DELTA_TIME
			end_time=max(end_time,t)
		x.append(points[-1][0])
		y.append(points[-1][1])
		ax.plot(x,y,"--",color=CHANNELS[index][0],alpha=0.5,linewidth=5)
	size_float32=(math.ceil(end_time/sequencer_compiler.SAMPLE_DELTA)+1)*4*len(data)
	size_uint8=(math.ceil(end_time/sequencer_compiler.SAMPLE_DELTA)+1)*len(data)
	data=sequencer_compiler.compile_sequence(data)
	size_compressed=len(data)
	channel_count=data[0]&7
	sample_count=data[1]|((data[0]>>3)<<8)
	state=[None for _ in range(0,channel_count*3)]
	x=[]
	y=[[] for _ in range(0,channel_count)]
	idx=channel_count+2
	for i in range(0,channel_count*3,3):
		state[i]=0
		state[i+1]=0
		state[i+2]=data[idx]
		idx+=1
	for i in range(0,sample_count):
		x.append(i*sequencer_compiler.SAMPLE_DELTA)
		for j in range(0,channel_count*3,3):
			if (state[j]):
				state[j]-=1
			else:
				token=data[idx]
				idx+=1
				if (token&1):
					state[j]=token>>6
					state[j+1]=((token>>2)&15)+1
					if (token&2):
						state[j+1]=-state[j+1]
				else:
					state[j]=token>>1
					state[j+1]=0
			state[j+2]+=state[j+1]
			y[j//3].append(sequencer_compiler.ENCODED_PULSE_TO_ANGLE(state[j+2]))
	for i in range(0,channel_count):
		ax.plot(x,y[i],"-",color=CHANNELS[i][0],label=CHANNELS[i][1])
	print(f"compressed: {size_compressed}, uint8: {size_uint8}, float32: {size_float32}\nw.r.t. uint8: {size_compressed/size_uint8-1:.1%}, w.r.t. float32 {size_compressed/size_float32-1:.1%}")
fig.set_size_inches(13,4,forward=True)
plt.xlabel("Time $\\left[\\mathrm{s}\\right]$")
plt.ylabel("Angle $\\left[\\mathrm{deg}\\right]$")
ax.set_xlim(xmin=0,xmax=end_time+0.04)
ax.set_yticks(range(0,200,45))
ax.set_ylim(ymin=-10,ymax=190)
ax.legend(loc="upper right")
plt.savefig("kinematics.png",dpi=300,bbox_inches="tight")
plt.show()
