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
import time
import subprocess as sp

from ezpath import *

DUMP      = 'dump'
DUMP_DEST = {None: "misc", True: "pass", False: "fail"}


def run_cmd(cmd:str, capture_output=True, text=True, shell=False):
	sp.run(['bash', '-c', cmd], capture_output=capture_output, text=text, shell=shell)


def get_current_venv():
	return to_os_path(sys.executable)


def invoke_in_venv(c, cmd, warn=True, hide=False, venv=None):
	if venv == None:
		venv = get_current_venv()
	return c.run(f"{venv} -m invoke {cmd}", warn=warn, hide=hide)


def find(path, names, excludes=[], recursive=True):
	path = to_os_path(path)

	found = []
	remove = []
	abs_names = names[:]
	abs_excludes = excludes[:]

	# check end of recursion
	if (not os.path.isdir(path)):
		return found

	# initializing paths
	for i in range(len(names)):
		abs_names[i] = os.path.join(path, names[i])

	for i in range(len(excludes)):
		abs_excludes[i] = os.path.join(path, excludes[i])

	for k in abs_names:
		found.extend([to_os_path(f) for f in glob.glob(k, recursive=recursive)])

	for e in abs_excludes:
		remove.extend([to_os_path(f) for f in glob.glob(e, recursive=recursive)])

	found = [i for i in found if i not in remove]
	return found


def clean_folder(path, keep=[], depth = -1, init=True, verbose=False):
	path = to_os_path(path)

	# check end of recursion
	if (depth == 0) or (not os.path.isdir(path)):
		return

	if init:
		keep = find(path, keep, [])

	# search targets
	for f in os.listdir(path):
		# retrieving file's absolute path
		fabs = os.path.join(path, f)

		# check if the file/folder must be kept, including wildcards. Keep always
		# has precedence over deleting
		if fabs in keep:
			# recurse into directories that are being kept
			if os.path.isdir(fabs):
				if verbose: print(f"Keeping folder {fabs}")
				clean_folder(fabs, keep, depth - 1, init=False, verbose=verbose)
			else:
				if verbose: print(f"Keeping file {fabs}")
			# go to the next item
			continue

		try:
			if os.path.isdir(fabs):
				if verbose: print(f"deleting folder {fabs}")
				shutil.rmtree(fabs)
			else:
				if verbose: print(f"deleting file {fabs}")
				os.remove(fabs)
		except Exception as e:
			print(f"Error removing file {fabs}: {e}")


def check(res, hard=False, report=True):
	if hasattr(res, "ok"):
		res = res.ok
	if hasattr(res, "returncode"):
		res = res.returncode == 0
	if not res:
		if report:
			print("FAILED")
		if hard:
			sys.exit(1)
	else:
		if report:
			print("PASSED")
	return res


def terminate(ok):
	check(ok, hard=True)


def make_in_place(c, path, cmd=''):
	path = to_os_path(path)
	cwd = os.getcwd()
	os.chdir(os.path.join(path, "_make"))
	print(f"running _make in {os.path.abspath(path)}")
	res = invoke_in_venv(c, cmd)
	os.chdir(cwd)
	return res


def dump(path, names, dump=DUMP, ok=None, dest=DUMP_DEST, target=''):
	path = to_os_path(path)
	if names is None:
		files = [path]
	else:
		files = find(path, names, recursive=True)
	if (ok == None) or (dest == None):
		d = os.path.join(dump, target)
	else:
		d = os.path.join(dump, dest[ok], target)
	os.makedirs(d, exist_ok=True)
	for n in files:
		if os.path.isdir(n):
			shutil.copytree(n, os.path.join(d, os.path.basename(n)), dirs_exist_ok=True)
		else:
			shutil.copy(n, d)


def collect_dumps(path, names=["**/_make/dump/misc/*", "**/_make/dump/fail/*", "**/_make/dump/pass/*"], excludes=[], dest="dump", delete_orig=True, do_not_sort=False):
	path = to_os_path(path)
	print(f"Starting Dump Collection from {path}")
	dumps = find(path, names, excludes, True)

	if len(dumps) == 0:
		print("No dumps found")
		return 0

	print("Found dumps")
	if do_not_sort:
		dest_pass = dest
		dest_fail = dest
		dest_misc = dest
	else:
		dest_pass = os.path.join(dest, "pass")
		dest_fail = os.path.join(dest, "fail")
		dest_misc = os.path.join(dest, "misc")

	failed = 0
	passed = 0
	orig   = []

	print(f"Startig {len(dumps)} iterations")
	for d in dumps:
		print(f"Dumping {d}")
		target = get_basename(d)

		if "pass" in d:
			os.makedirs(dest_pass, exist_ok=True)
			target = os.path.join(dest_pass, target)
			passed += 1
		elif "fail" in d:
			os.makedirs(dest_fail, exist_ok=True)
			target = os.path.join(dest_fail, target)
			failed += 1
		else:
			os.makedirs(dest_misc, exist_ok=True)
			target = os.path.join(dest_misc, target)

		if os.path.exists(target):
			print("Deleting existing target")
			if os.path.isdir(d):
				shutil.rmtree(target)
			else:
				os.remove(target)

		t = time.time()
		while time.time() < (t + 10):
			try:
				print(f"Copying {d} to {target}")
				if os.path.isdir(d):
					shutil.copytree(d, target, dirs_exist_ok=True)
				else:
					shutil.copy(d, target)
				break
			except:
				time.sleep(0.1)
				pass
		orig.append(d)

	if delete_orig:
		for d in orig:
			print("Deleting Original Dumps")
			if os.path.isdir(d):
				shutil.rmtree(d)
			else:
				os.remove(d)
			pass

	print(f"{passed} targets passed, {failed} targets failed")
	return failed


def create_link(c, link, target, force=False):
	if os.path.lexists(link):
		if not force:
			return
		os.remove(link)

	if os.name == 'posix':
		os.symlink(target, link)
	elif os.name == 'nt':
		if os.path.isdir(target):
			os.symlink(target, link, target_is_directory=True)
		else:
			os.symlink(target, link)


def calc_prj_depth(prj_dir=None, root_elem=[], root_name=[]):
	prj_dir  = os.path.abspath(prj_dir)
	prj_root = find_root(prj_dir, root_elem, root_name)
	rel_path = os.path.relpath(prj_dir, prj_root)
	return rel_path.count(os.sep) + 1
