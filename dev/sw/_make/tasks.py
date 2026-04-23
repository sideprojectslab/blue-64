#!/usr/bin/python3
## -------------------------------------------------------------------------- ##
##                                   .xXXx.                                   ##
##                                 .xXXXXXXx.                                 ##
##                                xXXX'  'XXXx                                ##
##                               xXXY      YXXx                               ##
##    XXXXXXXXXXXXXXXX          xXXY        YXXx          XXXXXXXXXXXXXXXX    ##
##    XXXXXXXXXXXXXXXX         xXXY          YXXx         XXXXXXXXXXXXXXXX    ##
##    XXXX                    xXXY            YXXx        XXXX                ##
## XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX ##
## XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX ##
##    XXXX                .XXXY                  YXXX.                XXXX    ##
##    XXXXXXXXXXXXXXXX XXXXXXY                    YXXXXXX XXXXXXXXXXXXXXXX    ##
##    XXXXXXXXXXXXXXXX XXXXX^                      ^XXXXX XXXXXXXXXXXXXXXX    ##
##                                                                            ##
## -------------------------------------------------------------------------- ##
## Author:  Vittorio Pascucci                                                 ##
## Company: EAS Embedded Audio Systems B.V.                                   ##
##                                                                            ##
## ALL RIGHTS RESERVED                                                        ##
##                                                                            ##
## This file is subject to the terms and conditions defined in the file       ##
## 'LICENSE.txt', which is part of this IP package.                           ##
## -------------------------------------------------------------------------- ##

import sys
sys.dont_write_bytecode = True

import subprocess as sp
from   ezpath     import *
from   invoke     import task, Context
from   abc        import ABC, abstractmethod

################################################################################
# SETTING UP ENVIRONMENT

PRJ_ROOT = find_root(get_abs_path('.'), root_elem=[".git/"])
add_abs_path(f"{PRJ_ROOT}/ext/eas_ip_lib/automation/common")

import build_tasks_common as btc
from   build_tasks_common import *
import build_utils_common as buc
from   build_utils_common import *

def read_fw_ver():
	with open(f"{SRC}/main/bt64_macros.h", 'r') as f:
		lines = f.readlines()
		for l in lines:
			if "BT64_FW_VERSION" in l:
				ver = l.split()[2].strip('"')
				return ver
	raise Exception("Version number not found")

SRC     = get_abs_path("../src")
WORK    = get_abs_path('work')
ESP_IDF = os.environ["ESP_IDF_5_5_4"]
EXPORT  = join_path(ESP_IDF, "export.sh")
TARGET  = "BT-64-" + read_fw_ver()

################################################################################

@task
def btstack(c):
	shutil.rmtree(join_path(SRC, "components", "btstack"), ignore_errors=True)
	os.makedirs(join_path(SRC, "components", "btstack"))
	cwd = os.getcwd()
	os.chdir(join_path(PRJ_ROOT, "ext", "bluepad32", "external", "btstack", "port", "esp32"))
	cmd = "&&".join([
		f"export IDF_PATH={SRC}",
		f"export IDF_TARGET=esp32",
		f"python3 -B integrate_btstack.py"
	])
	c.run(cmd)
	os.chdir(cwd)


@task(default=True)
def run(c):
	btstack(c)

	NAMES = [
		'**/application.bin',
		'**/bootloader.bin',
		'**/partition-table.bin']

	cmd = "&&".join([
		f"source {EXPORT}",
		f"idf.py -B {WORK} -C {SRC} build"
	])
	ok = check(c.run(cmd))
	buc.dump(f"{WORK}", names=NAMES, ok=ok, target=TARGET)
	shutil.copy(f"{WORK}/compile_commands.json", f"{PRJ_ROOT}/compile_commands.json")


@task
def erase(c):
	cmd = "&&".join([
		f"source {EXPORT}",
		f"idf.py -B {WORK} -C {SRC} erase-flash"
	])
	check(c.run(cmd), hard=True)


@task
def monior(c):
	cmd = "&&".join([
		f"source {EXPORT}",
		f"idf.py -B {WORK} -C {SRC} monitor"
	])
	run_cmd(cmd, capture_output=False)


@task
def config(c):
	cmd = "&&".join([
		f"source {EXPORT}",
		f"idf.py -B {WORK} -C {SRC} menuconfig"
	])
	run_cmd(cmd, capture_output=False)


@task
def flash(c):
	cmd = "&&".join([
		f"source {EXPORT}",
		f"idf.py -B {WORK} -C {SRC} flash"
	])
	check(c.run(cmd), hard=True)


@task
def clean(c, full=False):
	keep = ['tasks.py']
	if not full:
		keep.append('work')
	clean_folder(".", keep=keep)


@task
def package(c):
	shutil.copytree(f"dump/pass/{TARGET}", f"dump/output/{TARGET}", dirs_exist_ok=True)
