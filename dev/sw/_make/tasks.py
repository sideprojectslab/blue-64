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
add_abs_path(f"{PRJ_ROOT}/ext/eas_ip_lib/automation/sw/esp_idf")

import build_tasks_esp_idf as bte
from   build_tasks_esp_idf import *


def read_fw_ver():
	with open(f"{SRC}/main/bt64_macros.h", 'r') as f:
		lines = f.readlines()
		for l in lines:
			if "BT64_FW_VERSION" in l:
				ver = l.split()[2].strip('"')
				return ver
	raise Exception("Version number not found")

bte.SRC     = get_abs_path("../src")
bte.WORK    = get_abs_path('work')
bte.ESP_IDF = os.environ["ESP_IDF_5_5_4"]
bte.TARGET  = "BT-64-" + read_fw_ver()


@task
def btstack(c):
	shutil.rmtree(join_path(bte.SRC, "components", "btstack"), ignore_errors=True)
	os.makedirs(join_path(bte.SRC, "components", "btstack"))
	cwd = os.getcwd()
	os.chdir(join_path(PRJ_ROOT, "ext", "bluepad32", "external", "btstack", "port", "esp32"))
	cmd = "&&".join([
		f"export IDF_PATH={bte.SRC}",
		f"export IDF_TARGET={bte.MCU}",
		f"python3 -B integrate_btstack.py"
	])
	c.run(cmd)
	os.chdir(cwd)


@task(default=True)
def run(c):
	btstack(c)
	bte.run(c)
