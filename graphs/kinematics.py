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



end_time=0
with open(FILE_PATH,"r") as rf:
	data=json.loads(rf.read())
theoretical_y=[[] for _ in range(0,len(data))]
for i,channel in enumerate(data):
	points=channel["points"]
	t=points[0][0]
	for j in range(0,len(points)-1):
		a,b=points[j:j+2]
		while (t<b[0]):
			theoretical_y[i].append((0.5-math.cos((t-a[0])/(b[0]-a[0])*math.pi)/2)*(b[1]-a[1])+a[1])
			t+=DELTA_TIME
		end_time=max(end_time,t)
	theoretical_y[i].append(points[-1][1])
theoretical_x=[i*DELTA_TIME for i in range(0,math.ceil(end_time/DELTA_TIME)+1)]
size_uncompressed=(math.ceil(end_time/sequencer_compiler.SAMPLE_DELTA)+1)*4*len(data)
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
plt.rcParams["text.usetex"]=True
fig,ax=plt.subplots(1)
avg_err=0
max_err=0
for i in range(0,channel_count):
	ax.plot(theoretical_x,theoretical_y[i],"--",color=CHANNELS[i][0],alpha=0.5,linewidth=5)
	j=0
	for k in range(0,len(theoretical_x)):
		if (j+1<len(x) and theoretical_x[k]>=x[j+1]):
			j+=1
		if (j+1==len(x)):
			break
		err=abs((theoretical_x[k]-x[j])/(x[j+1]-x[j])*(y[i][j+1]-y[i][j])+y[i][j]-theoretical_y[i][k])
		avg_err+=err
		max_err=max(max_err,err)
for i in range(0,channel_count):
	ax.plot(x,y[i],"-",color=CHANNELS[i][0],label=CHANNELS[i][1])
print(f"avg={avg_err/(channel_count*end_time/DELTA_TIME):.3f} deg, max={max_err:.3f} deg\ncompressed: {size_compressed}, uncompressed: {size_uncompressed} => {size_compressed/size_uncompressed-1:.1%}")
fig.set_size_inches(13,4,forward=True)
plt.xlabel("Time $\\left[\\mathrm{s}\\right]$")
plt.ylabel("Angle $\\left[\\mathrm{deg}\\right]$")
ax.set_xlim(xmin=0,xmax=end_time+0.04)
ax.set_yticks(range(0,200,45))
ax.set_ylim(ymin=-10,ymax=190)
ax.legend(loc="upper right")
plt.savefig("kinematics.png",dpi=300,bbox_inches="tight")
plt.show()
