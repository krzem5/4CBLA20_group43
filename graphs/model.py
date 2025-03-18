############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 18/03/2025 by Krzesimir Hyżyk                                 #
############################################################################



import math
import matplotlib.animation as animation
import matplotlib.pyplot as plt



CURVE_POINTS=16
SCALE=0.2
FPS=24



# axle-to-end length
L1=1.5
# axle-to-axle lengths
L2=1.3
L3=1.1

# radii
R1=0.225
R2=0.2
R3=0.175

# masses and positions
C1=0.6
C2=0.5
C3=0.5
M1=0.6
M2=0.2
M3=0.1

# step geometry
SX=2.25
SY=0.25
W1=1.25
H1=1.26
H2=0.86
LEDGE=0.25

# motion
alpha=math.asin(H1/L2)
beta=math.asin(H2/L2)
gamma=0.03 # 3rd linkage ground-touching angle
p=0.4
q=0.25

MOTION=lambda t:[
	lambda t:(SX-L1-R1,SY+R1,alpha*t,0,0),
	lambda t:(SX-L1-R1+L2*(1-math.cos(alpha*t)),SY+R1+L2*math.sin(alpha*t),alpha*(1-t),-alpha*t,alpha*t+gamma),
	lambda t:(SX-L1-R1+L2*(1-math.cos(alpha)),SY+R1+L2*math.sin(alpha),0,-alpha,alpha*(1-t)),
	lambda t:(SX-L1-R1+L2*(1-math.cos(alpha))+p,SY+R1+L2*math.sin(alpha),0,-alpha-(2*math.pi-alpha)*t,0),
	lambda t:(SX-L1-R1+L2*(1-math.cos(alpha))+q,SY+R1+L2*math.sin(alpha),beta*t,0,0),
	lambda t:(SX-L1-R1+L2*(2-math.cos(alpha)-math.cos(beta*t))+q,SY+R1+L2*(math.sin(alpha)+math.sin(beta*t)),beta*(1-t),-beta*t,beta*t+gamma),
	lambda t:(SX-L1-R1+L2*(2-math.cos(alpha)-math.cos(beta+(math.pi/2-beta)*t))+q,SY+R1+L2*(math.sin(alpha)+math.sin(beta+(math.pi/2-beta)*t)),math.asin((H2-L2*math.sin(beta+(math.pi/2-beta)*t))/(L1-R1)),-beta-(math.pi/2-beta)*t,beta+(math.pi/2-beta)*t+gamma),
	lambda t:(SX-L1-R1+L2*(2-math.cos(alpha))+q,SY+R1+L2*(1+math.sin(alpha)),math.asin((H2-L2)/(L1-R1)),-math.pi/2,math.pi/2*(1-t)),
	lambda t:(SX-L1-R1+L2*(2-math.cos(alpha))+q,SY+R1+L2*(math.sin(alpha)+math.sin(math.pi/2-(beta-math.pi/2)*t)),math.asin((H2-L2*math.sin(math.pi/2-(beta-math.pi/2)*t))/(L1-R1)),-math.pi/2+(beta-math.pi/2)*t,0),
	lambda t:(SX-L1-R1+L2*(2-math.cos(alpha))+q,SY+R1+L2*(math.sin(alpha)+math.sin(beta)),0,beta-math.pi-(math.pi+beta)*t,0)
][int(t)](t%1)
GROUND=lambda t,p,x:[
	lambda t,p,x:(p[0],p[2]),
	lambda t,p,x:(p[3],p[2]),
	lambda t,p,x:(min(p[2],x+(SX-LEDGE)*SCALE),p[1]),
	lambda t,p,x:(x+(SX-LEDGE)*SCALE,p[1]),
	lambda t,p,x:(x+(SX-LEDGE)*SCALE,p[2]),
	lambda t,p,x:(max(x+(SX-LEDGE)*SCALE,p[3]),p[2]),
	lambda t,p,x:(max(x+(SX-LEDGE)*SCALE,p[3]),p[1]),
	lambda t,p,x:(p[2],p[1]),
	lambda t,p,x:(p[2],p[1]),
	lambda t,p,x:(x+(SX+W1-LEDGE)*SCALE,p[1]),
][int(t)](t%1,p,x)
STAIR_POINTS=[
	(0,0),
	(0,SY),
	(SX,SY),
	(SX,SY+H1),
	(SX-LEDGE,SY+H1),
	(SX+W1,SY+H1),
	(SX+W1,SY+H1+H2),
	(SX+W1-LEDGE,SY+H1+H2),
	(1/SCALE,SY+H1+H2),
	(1/SCALE,0),
	(0,0)
]



def _generate_stair_path(x,y):
	return ([x+e[0]*SCALE for e in STAIR_POINTS],[y+e[1]*SCALE for e in STAIR_POINTS])



def _render_linkage(ax,x0,y0,x1,y1,r,color):
	a=math.atan2(y1-y0,x1-x0)+math.pi/2
	x=[]
	y=[]
	for i in range(0,CURVE_POINTS+1):
		x.append(x0+math.cos(a)*r*SCALE)
		y.append(y0+math.sin(a)*r*SCALE)
		a+=math.pi/CURVE_POINTS
	for i in range(0,CURVE_POINTS+1):
		x.append(x1+math.cos(a)*r*SCALE)
		y.append(y1+math.sin(a)*r*SCALE)
		a+=math.pi/CURVE_POINTS
	x.append(x[0])
	y.append(y[0])
	ax.fill(x,y,color=color,alpha=0.5)
	ax.plot(x,y,"-",color=color)



def _render_tile(ax,t,dx,dy):
	x,y,a,b,c=MOTION(t)
	x=x*SCALE+dx
	y=y*SCALE+dy
	points=[(x,y),(x+math.cos(a)*(L1-R1)*SCALE,y+math.sin(a)*(L1-R1)*SCALE),(x+math.cos(b)*L2*SCALE,y+math.sin(b)*L2*SCALE),(x+math.cos(b)*L2*SCALE-math.cos(b+c)*L3*SCALE,y+math.sin(b)*L2*SCALE-math.sin(b+c)*L3*SCALE)]
	com=((x+C1*(points[1][0]-x))*M1+(x+C2*(points[2][0]-x))*M2+(points[2][0]+C3*(points[3][0]-points[2][0]))*M3)/(M1+M2+M3)
	# print(com)
	l,r=GROUND(t,[e[0] for e in points],dx)
	ax.fill([l,l,r,r],[dy,dy+1,dy+1,dy],color="#9467bd",alpha=0.2)
	ax.fill(*_generate_stair_path(dx,dy),color="#7f7f7f",alpha=0.5)
	ax.plot(*_generate_stair_path(dx,dy),"-",color="#7f7f7f")
	_render_linkage(ax,*points[0],*points[1],R1,"#1f77b4")
	_render_linkage(ax,*points[0],*points[2],R2,"#2ca02c")
	_render_linkage(ax,*points[2],*points[3],R3,"#d62728")
	ax.plot([com,com],[dy,dy+1],"--",color="#ff7f0e",linewidth=5)



def update(t,ax):
	plt.cla()
	_render_tile(ax,(t/FPS)%10,0,0)



plt.rcParams["text.usetex"]=True
fig,ax=plt.subplots(1)
fig.set_size_inches(3,3,forward=True)
ax.set_xlim(xmin=0,xmax=1)
ax.set_ylim(ymin=0,ymax=1)
ax.set_yticklabels([])
ax.set_xticklabels([])
ax.set_xticks([])
ax.set_yticks([])
anim=animation.FuncAnimation(fig=fig,func=lambda t:update(t,ax),frames=FPS*10,interval=1000//FPS)
plt.show()
