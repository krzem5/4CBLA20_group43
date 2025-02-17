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



def _get_client_device_path():
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



if (not os.path.exists("build")):
	os.mkdir("build")
if (not os.path.exists("build/client")):
	os.mkdir("build/client")
if (not os.path.exists("build/sequencer_compiler")):
	os.mkdir("build/sequencer_compiler")
if (not os.path.exists("build/server")):
	os.mkdir("build/server")
object_files=[]
error=False
for file in _get_source_files("src/sequencer_compiler"):
	object_file=f"build/sequencer_compiler/{file.replace('/','$')}.o"
	object_files.append(object_file)
	if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-momit-leaf-frame-pointer","-O3","-g0","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/sequencer_compiler/include"]).returncode):
		error=True
if (error or subprocess.run(["gcc","-O3","-g0","-o","build/sequencer_compiler/compiler"]+object_files).returncode):
	sys.exit(1)
subprocess.run(["build/sequencer_compiler/compiler","data/sequence.json","build/sequence.bin"])
quit()
if ("--client" in sys.argv):
	object_files=[]
	error=False
	for file in _get_source_files("src/client","src/common"):
		object_file=f"build/client/{file.replace('/','$')}.o"
		object_files.append(object_file)
		if (subprocess.run(["avr-gcc","-Wall","-Werror","-mmcu=atmega328p","-flto","-fno-fat-lto-objects","-ffunction-sections","-fdata-sections","-O3","-g0","-c","-DNULL=((void*)0)","-DF_CPU=16000000l",file,"-o",object_file,"-Isrc/client/include","-Isrc/common/include"]).returncode):
			error=True
	if (error or subprocess.run(["avr-gcc","-Wall","-Werror","-mmcu=atmega328p","-flto","-fuse-linker-plugin","-Wl,--gc-sections","-O3","-g0","-o","build/client/client.elf"]+object_files).returncode or subprocess.run(["avr-objcopy","-O","ihex","-R",".eeprom","build/client/client.elf","build/client/client.hex"]).returncode):
		sys.exit(1)
	serial_path=_get_client_device_path()
	if (serial_path is not None and subprocess.run(["avrdude","-C/etc/avrdude.conf","-V","-q","-q","-patmega328p","-carduino",f"-P{serial_path}","-b115200","-D","-Uflash:w:build/client/client.hex:i"])):
		sys.exit(1)
else:
	if ("--release" in sys.argv):
		object_files=[]
		error=False
		for file in _get_source_files("src/server","src/common"):
			object_file=f"build/server/{file.replace('/','$')}.o"
			object_files.append(object_file)
			if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-momit-leaf-frame-pointer","-O3","-g0","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/server/include","-Isrc/common/include"]).returncode):
				error=True
		if (error or subprocess.run(["gcc","-O3","-g0","-o","build/server/server"]+object_files+["-ludev"]).returncode):
			sys.exit(1)
	else:
		object_files=[]
		error=False
		for file in _get_source_files("src/server","src/common"):
			object_file=f"build/server/{file.replace('/','$')}.o"
			object_files.append(object_file)
			if (subprocess.run(["gcc","-Wall","-lm","-Werror","-march=native","-mno-red-zone","-Wno-strict-aliasing","-O0","-ggdb","-c","-D_GNU_SOURCE","-DNULL=((void*)0)",file,"-o",object_file,"-Isrc/server/include","-Isrc/common/include"]).returncode):
				error=True
		if (error or subprocess.run(["gcc","-O0","-ggdb","-o","build/server/server"]+object_files+["-ludev"]).returncode):
			sys.exit(1)
	if ("--run" in sys.argv):
		subprocess.run(["build/server/server"])
