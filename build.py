############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 12/02/2025 by Krzesimir Hyżyk                                 #
############################################################################

import os
import subprocess
import sys



def _get_source_files(*directories):
	for directory in directories:
		for root,_,files in os.walk(directory):
			for file in files:
				if (file.endswith(".c")):
					yield os.path.join(root,file)



if (not os.path.exists("build")):
	os.mkdir("build")
if (not os.path.exists("build/client")):
	os.mkdir("build/client")
if (not os.path.exists("build/server")):
	os.mkdir("build/server")
if ("--client" in sys.argv):
	if (subprocess.run(["arduino-cli","compile","--fqbn","arduino:avr:uno","src/client","--build-path","build/client","-u","-p","/dev/ttyACM0","--library","src/common/include/common"]).returncode!=0):
		sys.exit(1)
else:
	if ("--release" in sys.argv):
		object_files=[]
		error=False
		for file in _get_source_files("src/server"):
			object_file=f"build/server/{file.replace('/','$')}.o"
			object_files.append(object_file)
			if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-mno-avx256-split-unaligned-load","-ffast-math","-momit-leaf-frame-pointer","-Ofast","-g0","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/server/include","-Isrc/common/include"]).returncode!=0):
				error=True
		if (error or subprocess.run(["gcc","-O3","-g0","-o","build/server/server","-X","-znoexecstack"]+object_files).returncode!=0):
			sys.exit(1)
	else:
		object_files=[]
		error=False
		for file in _get_source_files("src/server"):
			object_file=f"build/server/{file.replace('/','$')}.o"
			object_files.append(object_file)
			if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-O0","-ggdb","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/server/include","-Isrc/common/include"]).returncode!=0):
				error=True
		if (error or subprocess.run(["gcc","-O0","-ggdb","-o","build/server/server","-X","-znoexecstack"]+object_files).returncode!=0):
			sys.exit(1)
	if ("--run" in sys.argv):
		subprocess.run(["build/server/server"])
