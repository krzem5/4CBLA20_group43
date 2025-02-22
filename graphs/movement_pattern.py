############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 22/02/2025 by Krzesimir Hyżyk                                 #
############################################################################



import math
import matplotlib.pyplot as plt



CORNER_TEXT_OFFSET=0.025
SCALE=0.2

STAIR_X_OFFSET=2
STAIR_Y_OFFSET=0.25
STAIR_LEDGE_WIDTH=0.3
STAIR_1_WIDTH=1.2
STAIR_1_HEIGHT=1.25
STAIR_2_HEIGHT=0.86

ROBOT_L1_LENGTH=1.5
ROBOT_L2_LENGTH=1.3
ROBOT_L3_LENGTH=1.1
ROBOT_L1_RADIUS=0.225
ROBOT_L2_RADIUS=0.2
ROBOT_L3_RADIUS=0.15
ROBOT_LINKAGE_CURVATURE_POINTS=16

# 1. lift 1st stage up
# 2. rotate onto 1st ledge
# 3. reset 3rd linkage
# 4. reset 2nd+3rd linkage
# 5. lift 1st stage up
# 6. rotate onto 2nd ledge
# 7. move forward on 2nd ledge
# 8. reset 3rd linkage
# 9. reset 2nd+3rd linkage
alpha=math.asin(STAIR_1_HEIGHT/ROBOT_L2_LENGTH)
beta=math.asin(STAIR_2_HEIGHT/ROBOT_L2_LENGTH)
p=0.18
STEP1_TIME=1.0
STEP2_TIME=0.6
STEP3_TIME=0.4
STEP4_TIME=0.6
STEP5_TIME=0.5
STEP6_TIME=0.6
STEP7_TIME=0.7
STEP8_TIME=0.5
STEP9_TIME=0.8
ROBOT_POSITIONS=[
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS,STAIR_Y_OFFSET+ROBOT_L1_RADIUS,alpha*STEP1_TIME,0,0),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha*STEP2_TIME)),STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha*STEP2_TIME),alpha*(1-STEP2_TIME),-alpha*STEP2_TIME,alpha*STEP2_TIME),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha)),STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha),0,-alpha,alpha*(1-STEP3_TIME)),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha))+p*STEP4_TIME,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha),0,-alpha-(2*math.pi-alpha)*STEP4_TIME,0),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha))+p,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha),beta*STEP5_TIME,0,0),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha)-math.cos(beta*STEP6_TIME))+p,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(beta*STEP6_TIME)),beta*(1-STEP6_TIME),-beta*STEP6_TIME,beta*STEP6_TIME),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha)-math.cos(beta+(math.pi/2-beta)*STEP7_TIME))+p,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(beta+(math.pi/2-beta)*STEP7_TIME)),math.asin((STAIR_1_HEIGHT+STAIR_2_HEIGHT-ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(beta+(math.pi/2-beta)*STEP7_TIME)))/ROBOT_L1_LENGTH),-beta-(math.pi/2-beta)*STEP7_TIME,beta+(math.pi/2-beta)*STEP7_TIME),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha))+p,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1+math.sin(alpha)),math.asin((STAIR_1_HEIGHT+STAIR_2_HEIGHT-ROBOT_L2_LENGTH*(1+math.sin(alpha)))/ROBOT_L1_LENGTH),-math.pi/2,math.pi/2*(1-STEP8_TIME)),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha))+p,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(math.pi/2-(beta-math.pi/2)*STEP9_TIME)),math.asin((STAIR_1_HEIGHT+STAIR_2_HEIGHT-ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(math.pi/2-(beta-math.pi/2)*STEP9_TIME)))/ROBOT_L1_LENGTH),-math.pi/2+(beta-math.pi/2)*STEP9_TIME,0)
]



def _generate_stair_path(x,y):
	points=[
		(0,0),
		(0,STAIR_Y_OFFSET),
		(STAIR_X_OFFSET,STAIR_Y_OFFSET),
		(STAIR_X_OFFSET,STAIR_Y_OFFSET+STAIR_1_HEIGHT),
		(STAIR_X_OFFSET-STAIR_LEDGE_WIDTH,STAIR_Y_OFFSET+STAIR_1_HEIGHT),
		(STAIR_X_OFFSET+STAIR_1_WIDTH,STAIR_Y_OFFSET+STAIR_1_HEIGHT),
		(STAIR_X_OFFSET+STAIR_1_WIDTH,STAIR_Y_OFFSET+STAIR_1_HEIGHT+STAIR_2_HEIGHT),
		(STAIR_X_OFFSET+STAIR_1_WIDTH-STAIR_LEDGE_WIDTH,STAIR_Y_OFFSET+STAIR_1_HEIGHT+STAIR_2_HEIGHT),
		(1/SCALE,STAIR_Y_OFFSET+STAIR_1_HEIGHT+STAIR_2_HEIGHT),
		(1/SCALE,0),
	]
	return ([x+e[0]*SCALE for e in points],[y+e[1]*SCALE for e in points])



def _render_linkage(ax,x0,y0,x1,y1,r,color):
	a=math.atan2(y1-y0,x1-x0)+math.pi/2
	x=[]
	y=[]
	for i in range(0,ROBOT_LINKAGE_CURVATURE_POINTS+1):
		x.append(x0+math.cos(a)*r*SCALE)
		y.append(y0+math.sin(a)*r*SCALE)
		a+=math.pi/ROBOT_LINKAGE_CURVATURE_POINTS
	for i in range(0,ROBOT_LINKAGE_CURVATURE_POINTS+1):
		x.append(x1+math.cos(a)*r*SCALE)
		y.append(y1+math.sin(a)*r*SCALE)
		a+=math.pi/ROBOT_LINKAGE_CURVATURE_POINTS
	x.append(x[0])
	y.append(y[0])
	ax.fill(x,y,color=color,alpha=0.5)
	ax.plot(x,y,"-",color=color)



def _render_robot(ax,x,y,a,b,c):
	_render_linkage(ax,x,y,x+math.cos(a)*(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)*SCALE,y+math.sin(a)*(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)*SCALE,ROBOT_L1_RADIUS,"#1f77b4")
	_render_linkage(ax,x,y,x+math.cos(b)*ROBOT_L2_LENGTH*SCALE,y+math.sin(b)*ROBOT_L2_LENGTH*SCALE,ROBOT_L2_RADIUS,"#2ca02c")
	_render_linkage(ax,x+math.cos(b)*ROBOT_L2_LENGTH*SCALE,y+math.sin(b)*ROBOT_L2_LENGTH*SCALE,x+math.cos(b)*ROBOT_L2_LENGTH*SCALE-math.cos(b+c)*ROBOT_L3_LENGTH*SCALE,y+math.sin(b)*ROBOT_L2_LENGTH*SCALE-math.sin(b+c)*ROBOT_L3_LENGTH*SCALE,ROBOT_L3_RADIUS,"#d62728")



plt.rcParams["text.usetex"]=True
fig,ax=plt.subplots(1)
for i in range(0,9):
	x,y=i%3,2-i//3
	ax.text(x+CORNER_TEXT_OFFSET,y+1-CORNER_TEXT_OFFSET,f"${i+1}$",ha="left",va="top",size="xx-large")
	ax.fill(*_generate_stair_path(x,y),color="#8c564b",alpha=0.5)
	ax.plot(*_generate_stair_path(x,y),"-",color="#8c564b")
	rx,ry,ra,rb,rc=ROBOT_POSITIONS[i]
	_render_robot(ax,rx*SCALE+x,ry*SCALE+y,ra,rb,rc)
ax.plot([1,1],[0,3],"-",color="#000000",lw=0.8)
ax.plot([2,2],[0,3],"-",color="#000000",lw=0.8)
ax.plot([0,3],[1,1],"-",color="#000000",lw=0.8)
ax.plot([0,3],[2,2],"-",color="#000000",lw=0.8)
fig.set_size_inches(9,9,forward=True)
ax.set_yticklabels([])
ax.set_xticklabels([])
ax.set_xticks([])
ax.set_yticks([])
ax.set_xlim(xmin=0,xmax=3)
ax.set_ylim(ymin=0,ymax=3)
plt.savefig("movement_pattern.png",dpi=300,bbox_inches="tight")
plt.show()
