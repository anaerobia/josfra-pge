#!/bin/csh
# Shell script to transfer files (get/put) using ftp.
# Usage:
#       ftp.csh <cmd> <remotehost> <srcfile> <destfile>
# where
#       cmd is either get or put       
#       destfile is optional
#
# If destfile is omitted, then destfile is same as srcfile.
# This script is executed from the PGS API.
#
# Note that .netrc file in the home directory of the user must be setup for
# this script to work (eg. mode must be rw-------)
# For more information on .netrc, type "man netrc".

#
# Check number of parameters (must be at least 3). If not exit.
#
#echo $1 $2 $3
if ($#argv <  3) then
#   echo usage: ftp.csh <cmd> <remotehost> <srcfile> <destfile>
	exit(1)
endif

#
# Make sure the remove command does not prompt user.
#
alias rm rm

#
# Variables.
#
set cmd=$1
set host=$2
set srcfile=$3
set destfile=$4
set errfile=ftp.$$

#echo cmd=$cmd
#echo host=$host
#echo srcfile=$srcfile
#echo destfile=$destfile

if ($cmd == "get") then
	ftp $host << % >& $errfile
get $srcfile $destfile
quit
%
else
	ftp $host << % >& $errfile
put $srcfile $destfile
quit
%
endif

#
# Check for ftp error.
#
if ($cmd == "get") then
	if (-z $errfile) then
		set exitstatus = 0
	else
		set exitstatus = 1
	endif
else
	if (-z $errfile) then
		set exitstatus = 0
	else
		fgrep -x "netout: write returned 0?" < $errfile
#		echo status=$status
		if (status == 0) then
			set exitstatus = 0
		else
			set exitstatus = 1
		endif
	endif
endif

#echo exitstatus=$exitstatus

rm $errfile

exit($exitstatus)

