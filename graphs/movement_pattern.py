############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 22/02/2025 by Krzesimir Hyżyk                                 #
############################################################################



import math
import matplotlib.pyplot as plt



GENERATE_SQUARE=False

CORNER_TEXT_OFFSET=0.025
SCALE=0.2

STAIR_X_OFFSET=2.25
STAIR_Y_OFFSET=0.25
STAIR_LEDGE_WIDTH=0.25
STAIR_1_WIDTH=1.25
STAIR_1_HEIGHT=1.26
STAIR_2_HEIGHT=0.86

ROBOT_L1_LENGTH=1.5
ROBOT_L2_LENGTH=1.3
ROBOT_L3_LENGTH=1.1
ROBOT_L1_RADIUS=0.225
ROBOT_L2_RADIUS=0.2
ROBOT_L3_RADIUS=0.15
CURVE_POINTS=16

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
gamma=0.03
p=0.4
q=0.25
STEP1_TIME=1.0
STEP2_TIME=0.6
STEP3_TIME=0.4
STEP4_TIME=0.6
STEP5_TIME=0.5
STEP6_TIME=0.6
STEP7_TIME=0.7
STEP8_TIME=0.5
STEP9_TIME=0.8
FIRST_ROBOT_POSITION=(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS,STAIR_Y_OFFSET+ROBOT_L1_RADIUS,0,0,0)
LAST_ROBOT_POSITION=(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha))+q,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(beta)),0,0,0)
ROBOT_POSITIONS=[
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS,STAIR_Y_OFFSET+ROBOT_L1_RADIUS,alpha*STEP1_TIME,0,0),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha*STEP2_TIME)),STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha*STEP2_TIME),alpha*(1-STEP2_TIME),-alpha*STEP2_TIME,alpha*STEP2_TIME+gamma),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha)),STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha),0,-alpha,alpha*(1-STEP3_TIME)),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha))+p,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha),0,-alpha-(2*math.pi-alpha)*STEP4_TIME,0),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1-math.cos(alpha))+q,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*math.sin(alpha),beta*STEP5_TIME,0,0),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha)-math.cos(beta*STEP6_TIME))+q,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(beta*STEP6_TIME)),beta*(1-STEP6_TIME),-beta*STEP6_TIME,beta*STEP6_TIME+gamma),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha)-math.cos(beta+(math.pi/2-beta)*STEP7_TIME))+q,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(beta+(math.pi/2-beta)*STEP7_TIME)),math.asin((STAIR_2_HEIGHT-ROBOT_L2_LENGTH*math.sin(beta+(math.pi/2-beta)*STEP7_TIME))/(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)),-beta-(math.pi/2-beta)*STEP7_TIME,beta+(math.pi/2-beta)*STEP7_TIME+gamma),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha))+q,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(1+math.sin(alpha)),math.asin((STAIR_2_HEIGHT-ROBOT_L2_LENGTH)/(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)),-math.pi/2,math.pi/2*(1-STEP8_TIME)),
	(STAIR_X_OFFSET-ROBOT_L1_LENGTH-ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(2-math.cos(alpha))+q,STAIR_Y_OFFSET+ROBOT_L1_RADIUS+ROBOT_L2_LENGTH*(math.sin(alpha)+math.sin(math.pi/2-(beta-math.pi/2)*STEP9_TIME)),math.asin((STAIR_2_HEIGHT-ROBOT_L2_LENGTH*math.sin(math.pi/2-(beta-math.pi/2)*STEP9_TIME))/(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)),-math.pi/2+(beta-math.pi/2)*STEP9_TIME,0)
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
		(0,0)
	]
	return ([x+e[0]*SCALE for e in points],[y+e[1]*SCALE for e in points])



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



def _generate_arrow(ax,cx,cy,r,a,b):
	x=[]
	y=[]
	if (b is not None):
		b=(b-a)/CURVE_POINTS
		for i in range(0,CURVE_POINTS+1):
			x.append(cx+math.cos(a)*r*SCALE)
			y.append(cy+math.sin(a)*r*SCALE)
			a+=b
		ax.plot(x[:-1],y[:-1],"-",color="#000000")
	else:
		x.append(cx)
		x.append(cx+math.cos(a)*r*SCALE)
		y.append(cy)
		y.append(cy+math.sin(a)*r*SCALE)
		print(x,y)
	ax.arrow(x[-2],y[-2],x[-1]-x[-2],y[-1]-y[-2],length_includes_head=True,head_width=0.03,head_length=0.04,color="#000000")



def _draw_annotations(ax,i,points,angles):
	margin=0.3
	scale=0.75
	straight_offset=0.5
	straight_length=0.75
	if (not i):
		_generate_arrow(ax,*points[0],ROBOT_L1_LENGTH*scale,margin,angles[0]-margin)
	elif (i==1):
		_generate_arrow(ax,*points[2],ROBOT_L2_LENGTH/scale,angles[1]+math.pi+margin,angles[1]+math.pi-margin)
	elif (i==2):
		_generate_arrow(ax,*points[2],ROBOT_L3_LENGTH/scale,angles[1]+angles[2]+math.pi+margin,angles[1]+angles[2]+math.pi-margin)
	elif (i==3):
		_generate_arrow(ax,*points[0],ROBOT_L2_LENGTH/scale,angles[1]+margin,angles[1]-margin)
		_generate_arrow(ax,points[0][0]-(straight_length+straight_offset)*SCALE,points[0][1],straight_length,0,None)
	elif (i==4):
		_generate_arrow(ax,*points[0],ROBOT_L1_LENGTH*scale,angles[0]+margin,angles[0]+3*margin)
	elif (i==5):
		_generate_arrow(ax,*points[2],ROBOT_L2_LENGTH/scale,angles[1]+math.pi+margin,angles[1]+math.pi-margin)
	elif (i==6):
		_generate_arrow(ax,*points[2],ROBOT_L2_LENGTH/scale,angles[1]+math.pi+margin,angles[1]+math.pi-margin)
		_generate_arrow(ax,points[1][0]+straight_offset*scale*SCALE,points[1][1],straight_length*scale,0,None)
	elif (i==7):
		_generate_arrow(ax,*points[2],ROBOT_L3_LENGTH/scale,angles[1]+angles[2]+math.pi+margin,angles[1]+angles[2]+math.pi-margin)
	elif (i==8):
		_generate_arrow(ax,*points[0],ROBOT_L2_LENGTH,angles[1]-margin,-math.pi-margin)
		_generate_arrow(ax,points[0][0],points[0][1]+(straight_length+straight_offset)*SCALE,straight_length,-math.pi/2,None)



def _render_robot(ax,x,y,a,b,c,i):
	points=[(x,y),(x+math.cos(a)*(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)*SCALE,y+math.sin(a)*(ROBOT_L1_LENGTH-ROBOT_L1_RADIUS)*SCALE),(x+math.cos(b)*ROBOT_L2_LENGTH*SCALE,y+math.sin(b)*ROBOT_L2_LENGTH*SCALE),(x+math.cos(b)*ROBOT_L2_LENGTH*SCALE-math.cos(b+c)*ROBOT_L3_LENGTH*SCALE,y+math.sin(b)*ROBOT_L2_LENGTH*SCALE-math.sin(b+c)*ROBOT_L3_LENGTH*SCALE)]
	_render_linkage(ax,*points[0],*points[1],ROBOT_L1_RADIUS,"#1f77b4")
	_render_linkage(ax,*points[0],*points[2],ROBOT_L2_RADIUS,"#2ca02c")
	_render_linkage(ax,*points[2],*points[3],ROBOT_L3_RADIUS,"#d62728")
	_draw_annotations(ax,i,points,[a,b,c])



plt.rcParams["text.usetex"]=True
fig,ax=plt.subplots(1)
if (GENERATE_SQUARE):
	for i in range(0,9):
		x,y=i%3,2-i//3
		ax.text(x+CORNER_TEXT_OFFSET,y+1-CORNER_TEXT_OFFSET,f"${i+1}$",ha="left",va="top",size="xx-large")
		ax.fill(*_generate_stair_path(x,y),color="#7f7f7f",alpha=0.5)
		ax.plot(*_generate_stair_path(x,y),"-",color="#7f7f7f")
		rx,ry,ra,rb,rc=ROBOT_POSITIONS[i]
		_render_robot(ax,rx*SCALE+x,ry*SCALE+y,ra,rb,rc,i)
	ax.plot([1,1],[0,3],"-",color="#000000",lw=0.8)
	ax.plot([2,2],[0,3],"-",color="#000000",lw=0.8)
	ax.plot([0,3],[1,1],"-",color="#000000",lw=0.8)
	ax.plot([0,3],[2,2],"-",color="#000000",lw=0.8)
	fig.set_size_inches(9,9,forward=True)
	ax.set_xlim(xmin=0,xmax=3)
	ax.set_ylim(ymin=0,ymax=3)
else:
	padding=0.2
	width=15
	fig.set_size_inches(width,width/(7+2*padding)*(2.5+2*padding),forward=True)
	for i in range(0,6):
		x=i*(1+padding)+padding
		y=1+padding*2
		ax.fill(*_generate_stair_path(x,y),color="#7f7f7f",alpha=0.5)
		ax.plot(*_generate_stair_path(x,y),"-",color="#7f7f7f")
		if (i):
			ax.text(x+CORNER_TEXT_OFFSET,y+1-CORNER_TEXT_OFFSET,f"${i}$",ha="left",va="top",size="xx-large")
			rx,ry,ra,rb,rc=ROBOT_POSITIONS[i-1]
			_render_robot(ax,rx*SCALE+x,ry*SCALE+y,ra,rb,rc,i-1)
		else:
			rx,ry,ra,rb,rc=FIRST_ROBOT_POSITION
			_render_robot(ax,rx*SCALE+x,ry*SCALE+y,ra,rb,rc,-1)
		ax.plot([x,x+1,x+1,x,x],[y,y,y+1,y+1,y],"-",color="#000000",lw=0.8)
	for i in range(0,5):
		x=(i+0.5)*(1+padding)+padding
		y=padding
		ax.fill(*_generate_stair_path(x,y),color="#7f7f7f",alpha=0.5)
		ax.plot(*_generate_stair_path(x,y),"-",color="#7f7f7f")
		if (i<4):
			ax.text(x+CORNER_TEXT_OFFSET,y+1-CORNER_TEXT_OFFSET,f"${i+6}$",ha="left",va="top",size="xx-large")
			rx,ry,ra,rb,rc=ROBOT_POSITIONS[i+5]
			_render_robot(ax,rx*SCALE+x,ry*SCALE+y,ra,rb,rc,i+5)
		else:
			rx,ry,ra,rb,rc=LAST_ROBOT_POSITION
			_render_robot(ax,rx*SCALE+x,ry*SCALE+y,ra,rb,rc,-1)
		ax.plot([x,x+1,x+1,x,x],[y,y,y+1,y+1,y],"-",color="#000000",lw=0.8)
	ax.axis("off")
	ax.set_xlim(xmin=0,xmax=7+2*padding)
	ax.set_ylim(ymin=0,ymax=2.5+2*padding)
ax.set_yticklabels([])
ax.set_xticklabels([])
ax.set_xticks([])
ax.set_yticks([])
plt.savefig("movement_pattern.png",dpi=300,bbox_inches="tight")
plt.show()
