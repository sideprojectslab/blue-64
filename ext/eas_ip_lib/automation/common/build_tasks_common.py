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
import os
import glob
import shutil

from ezpath import *
from invoke import task


add_rel_path(f".")
from build_utils_common import *

################################################################################
# SETTING UP ENVIRONMENT

add_rel_path(".")

def autosearch(c, path, cmd=''):
	ok = True
	# if a _make folder is present in the current folder we just run the tasks
	# contained in it
	if "_make" in os.listdir(path):
		ok = make_in_place(c, path, cmd).ok

	# if no _make folder is available, we start looking into folders recursively
	else:
		for i in os.listdir(path):
			iabs = os.path.join(path, i)
			if os.path.isdir(iabs):
				ok &= autosearch(c, iabs, cmd)

	return ok


def autorun(c, path="../", cmd='', skip_root=True):
	# the default autorun task is a lot like autosearch, but it skips the _make
	# floder it runs from. In general it is expected that the default task is
	# overridden by something more useful
	ok = True
	for i in os.listdir(path):
		if os.path.isdir(os.path.join(path, i)):
			if i != "_make":
				iabs = os.path.join(path, i)
				if os.path.isdir(iabs):
					ok &= autosearch(c, iabs, cmd)
			elif not skip_root:
				ok = make_in_place(c, path, cmd).ok
	return ok

@task(default=True)
def run(c, path="../", cmd=''):
	terminate(autorun(c, path, cmd))

@task
def setup(c):
	print("Nothing to setup")


@task
def clean(c):
	autorun(c, cmd="clean")
