############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 21/02/2025 by Krzesimir Hyżyk                                 #
############################################################################



import json
import math



FILE_PATH="../data/sequence.json"

SERVIO_MAX_ANGULAR_VELOCITY=360 # deg/s
SERVIO_ACCELERATION_PROFILE_MARGIN=0.4

WHEEL_RADIUS=0.45 # dm

LINKAGE_PINS=((0,1),(2,3),(4,5))

WHEEL_MOTION=[
	0,
	0,
	0,
	0,
	0.2,
	0,
	0,
	0.912766745835,
	0,
	0.0680966985331,
	0
]

MIDDLE_LINKAGE_MOTION=[
	90,
	16,
	16,
	16,
	-270,
	49-360,
	49-360,
	17-360,
	17-360,
	-49-360,
	-270-360
]

FINAL_LINKAGE_MOTION=[
	90,
	90,
	164,
	90,
	90,
	90,
	131,
	180,
	90,
	90,
	90
]



def _encode_delta(delta,out):
	profile=SERVIO_ACCELERATION_PROFILE_MARGIN
	value=90+delta/(1-profile)/SERVIO_MAX_ANGULAR_VELOCITY*90
	if (value<0):
		profile=1+delta/SERVIO_MAX_ANGULAR_VELOCITY
		value=0
	elif (value>180):
		profile=1-delta/SERVIO_MAX_ANGULAR_VELOCITY
		value=180
	t=out[-1][0]+1
	out.append([t-1+profile,value])
	out.append([t-profile,value])
	out.append([t,90])



def _compute_wheel_channel():
	points=[[0,90]]
	for i in range(1,len(WHEEL_MOTION)):
		_encode_delta(WHEEL_MOTION[i]/(2*math.pi*WHEEL_RADIUS)*360,points)
	return {"pin_a":LINKAGE_PINS[0][0],"pin_b":LINKAGE_PINS[0][1],"points":points}



def _compute_middle_linkage_channel():
	points=[[0,90]]
	for i in range(1,len(MIDDLE_LINKAGE_MOTION)):
		_encode_delta(MIDDLE_LINKAGE_MOTION[i]-MIDDLE_LINKAGE_MOTION[i-1],points)
	return {"pin_a":LINKAGE_PINS[1][0],"pin_b":LINKAGE_PINS[1][1],"points":points}



def _compute_final_linkage_channel():
	return {"pin_a":LINKAGE_PINS[2][0],"pin_b":LINKAGE_PINS[2][1],"points":[(i,e) for i,e in enumerate(FINAL_LINKAGE_MOTION)]}



with open(FILE_PATH,"w") as wf:
	wf.write(json.dumps([_compute_wheel_channel(),_compute_middle_linkage_channel(),_compute_final_linkage_channel()]))
