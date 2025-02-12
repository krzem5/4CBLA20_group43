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
source_file_directory=("src/client" if "--client" in sys.argv else "src/server")
if ("--release" in sys.argv):
	object_files=[]
	error=False
	for file in _get_source_files(source_file_directory):
		object_file=f"build/{file.replace('/','$')}.o"
		object_files.append(object_file)
		if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-mno-avx256-split-unaligned-load","-ffast-math","-momit-leaf-frame-pointer","-Ofast","-g0","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,f"-I{source_file_directory}/include"]).returncode!=0):
			error=True
	if (error or subprocess.run(["gcc","-O3","-g0","-o","build/server","-X","-znoexecstack"]+object_files).returncode!=0):
		sys.exit(1)
else:
	object_files=[]
	error=False
	for file in _get_source_files(source_file_directory):
		object_file=f"build/{file.replace('/','$')}.o"
		object_files.append(object_file)
		if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-O0","-ggdb","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,f"-I{source_file_directory}/include"]).returncode!=0):
			error=True
	if (error or subprocess.run(["gcc","-O0","-ggdb","-o","build/server","-X","-znoexecstack"]+object_files).returncode!=0):
		sys.exit(1)
if ("--run" in sys.argv):
	subprocess.run(["build/server"])
