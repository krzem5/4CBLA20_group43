############################################################################
# Copyright (c) Krzesimir Hyżyk - All Rights Reserved                      #
# Unauthorized copying of this file, via any medium is strictly prohibited #
# Proprietary and confidential                                             #
# Created on 12/02/2025 by Krzesimir Hyżyk                                 #
############################################################################

import os
import stat
import subprocess
import sys



def _get_serial_path():
	if (not os.path.exists("/dev/serial/by-id")):
		return None
	for k in os.listdir("/dev/serial/by-id"):
		path=f"/dev/serial/by-id/{k}"
		if (stat.S_ISCHR(os.stat(path).st_mode)):
			return path
	return None



def _get_source_files(*directories):
	for directory in directories:
		for root,_,files in os.walk(directory):
			for file in files:
				if (file.endswith(".c")):
					yield os.path.join(root,file)



def _generate_client_sketch(dst_file_path,*directories):
	if (not os.path.exists(dst_file_path)):
		os.mkdir(dst_file_path)
	open(os.path.join(dst_file_path,f"{dst_file_path.rstrip('/').split('/')[-1]}.ino"),"w").close()
	with open(os.path.join(dst_file_path,"weak_main_patch.h"),"w") as wf:
		wf.write("#ifndef __ASSEMBLER__\nint __attribute__((weak)) main(void);\n#endif\n")
	with open(os.path.join(dst_file_path,"generated.c"),"w") as wf:
		for file in _get_source_files(*directories):
			wf.write(f"#include <{os.path.abspath(file)}>\n")



if (not os.path.exists("build")):
	os.mkdir("build")
if (not os.path.exists("build/client")):
	os.mkdir("build/client")
if (not os.path.exists("build/server")):
	os.mkdir("build/server")
if ("--client" in sys.argv):
	serial_path=_get_serial_path()
	_generate_client_sketch("build/client_sketch","src/client","src/common")
	if (subprocess.run(["arduino-cli","compile","build/client_sketch","--build-path","build/client","--build-property","build.extra_flags=-Isrc/client/include -Isrc/common/include -Wno-sign-compare -Wno-unused-parameter -Wno-pointer-arith -fdiagnostics-color=always -g0 -Os --include build/client_sketch/weak_main_patch.h","-b","arduino:avr:uno","--warnings","all"]+(["-p",serial_path,"-u"] if serial_path is not None else [])).returncode):
		sys.exit(1)
else:
	if ("--release" in sys.argv):
		object_files=[]
		error=False
		for file in _get_source_files("src/server","src/common"):
			object_file=f"build/server/{file.replace('/','$')}.o"
			object_files.append(object_file)
			if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-mno-avx256-split-unaligned-load","-momit-leaf-frame-pointer","-O3","-g0","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/server/include","-Isrc/common/include"]).returncode):
				error=True
		if (error or subprocess.run(["gcc","-O3","-g0","-o","build/server/server","-X","-znoexecstack"]+object_files+["-ludev"]).returncode):
			sys.exit(1)
	else:
		object_files=[]
		error=False
		for file in _get_source_files("src/server","src/common"):
			object_file=f"build/server/{file.replace('/','$')}.o"
			object_files.append(object_file)
			if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-O0","-ggdb","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/server/include","-Isrc/common/include"]).returncode):
				error=True
		if (error or subprocess.run(["gcc","-O0","-ggdb","-o","build/server/server","-X","-znoexecstack"]+object_files+["-ludev"]).returncode):
			sys.exit(1)
	if ("--run" in sys.argv):
		subprocess.run(["build/server/server"])
