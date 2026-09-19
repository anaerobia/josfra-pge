#!/bin/csh -f
#-----------------------------------------------------------------------------
# file:		mkpgslib
#
# description:
# 	This file helps to automate the process of building and updating
# 	the PGS Toolkit object library.  It takes all the object files in
#       ${PGSLIB}/obj/<XXX>, strips off the leading PGS_<XXX>_ from the file 
#       name, and creates an updated version of the library $PGSLIB/libPGSTK.a 
#       which uses the shortened file names.  This is to help avoid name
#       collision in object module names in the library.  This script makes
#       heavy use of environment variables defined in pgs-dev-env.csh.
# 	
# usage:
# 	mkpgslib (<XXX> <XXX> ...)
#       where <XXX> are toolkit group mnemonics
#
# authors:        Mike Sucher - Applied Research Corporation
#                 Guru Tej S. Khalsa - Applied Research Corporation
#	          Graham J. Bland - EOSL
#                 Phuong T. Nguyen - L3 Communication, Corp. 
#
# history:
# 	25-Mar-1994 MES  Initial version
# 	31-Mar-1994 MES  Improve portability, strip off <XXX>_ from filename
#       25-Apr-1994 GTSK Improve portability (works on all PGS platforms) 
#			 Modified to build single library: libPGSTK.a
#       07-Sep-1994 GJB  Change copy command to move *.o rather than PGS_*
#			 this accounts for non PGS_ modules.
#       14-Sep-1994 GTSK Modified to use PGSOBJ where appropriate (instead of
#                        $PGSLIB/obj).  Modified script to make the creation of
#                        a backup of libPGSTK.a an option (was automatic).
#                        Default behavior is NO backup.  Make backup if first
#                        command line argument is '-b'.
#       20-Sep-1994 GTSK Added ranlib command for SunOS 4.x.  This script now
#                        depends on the environment variable BRAND defined in 
#                        pgs-dev-env.csh.
#       20-Sep-1994 MES  Patch to figure out sun4 in case BRAND is not defined.
# 			 If not, will run just a bit slower.
#       17-Aug-2001 AA   Modified for solaris8 support
#       17-Jun-2002 PTN  Modified to ignore using "basename" - this command
#                        doesnt work correctly on g++ compiler of linux.
#       17-Oct-2005 AT  Modified for Solaris9
#       17-Oct-2005 AT  Modified for Solaris10
#-----------------------------------------------------------------------------

#
# make sure no user-defined aliases interfere with commands
#

unalias rm 
unalias cp 
unalias cd 
unalias mv 
unalias ls 
unalias ar 
unalias chmod 
unalias mkdir

#
# make sure environment variables PGSLIB, PGSCPPO and BRAND are set
#

if($?PGSLIB == 0) then
    echo "The environment variable PGSLIB must be set."
    exit
endif

if($?PGSCPPO) then
    cd ${PGSCPPO}
else
    echo "The environment variable PGSCPPO must be set."
    exit
endif

if($?BRAND == 0) then
    if ("`uname`" == "SunOS") then
        set cbrand=`uname -r | awk -F. '{print $1, $2}'`
        if ("$cbrand" == "5 10") then
            setenv BRAND sun5.10
        else if ("$cbrand" == "5 9") then
            setenv BRAND sun5.9
        else if ("$cbrand" == "5 8") then
            setenv BRAND sun5.8
        else if ("$cbrand" == "5 5") then
            setenv BRAND sun5
        else
            setenv BRAND sun4
        endif
    else if ("`uname`" == "IRIX64") then
        setenv BRAND sgi
    else if ("`uname`" == "Linux") then
	if( "`uname -m | awk '{print $1}'`" == "x86_64" || "`uname -m | awk '{print $1}'`" == "ia64" ) then
	    if( "$LNX_COMP_FLAG" != "-m32" ) then
		setenv BRAND linux64
	    else
		setenv BRAND linux32
	    endif
	else
	    setenv BRAND linux
	endif
    else if ("`uname`" == "Darwin") then
      if("`uname -m | awk '{print $1}'`" == "i386") then
	setenv BRAND macintel
      else if("`uname -m | awk '{print $1}'`" == "i686") then
	setenv BRAND macintel
      else
	setenv BRAND macintosh
      endif
    else
        setenv BRAND NOT_SUN_OR_SGI
    endif
endif


#
# check to see if backup has been requested
#

if ("$1" == "-b") then
   set MAKE_BACKUP=1
   set FIRST=$2
   shift argv
else
   set FIRST=$1
endif

#
# set shell variable GROUPS to all arguments
# if any, otherwise set it to contain all
# directories found in $PGSCPPO
#

if ("$FIRST" != "") then
    set GROUPS=($argv)
else
    set GROUPS=`/bin/ls`
endif

#
# save old library if requested (and if found) 
#

if ( -e $PGSLIB/libPGSTKcpp.a ) then
    echo ""
    if ($?MAKE_BACKUP) then
	echo "Saving old library as $PGSLIB/libPGSTKcpp.a.old ..."
       mv $PGSLIB/libPGSTKcpp.a $PGSLIB/libPGSTKcpp.a.old
       cp $PGSLIB/libPGSTKcpp.a.old $PGSLIB/libPGSTKcpp.a
    endif
    echo "Updating ${PGSLIB}/libPGSTKcpp.a..."
else
    echo ""
    if ($?MAKE_BACKUP) then
       echo "Could not find old library $PGSLIB/libPGSTKcpp.a.  No backup made."
    endif
    echo "Creating ${PGSLIB}/libPGSTKcpp.a..."
endif

#
# Build the library 
#
# NOTE: This script replaces old object files
#       with their newer counterparts.  This
#       means that the library can be updated
#       for a single group without affecting any
#       any other groups.  It also means that
#       there will be conflicts if an object file
#       has a different name between builds.  In
#       this case it is likely that warnings will
#       be issued at compile time that certain routine
#       are multiply defined.

foreach GRP ($GROUPS)

    if ($GRP == "-b") then
	echo ""
	echo "Ignoring backup switch '-b': must be first command line argument."
	echo "WARNING: no backup library created."
 	continue
    endif

    if( -d  "${PGSCPPO}/$GRP" ) then        # must be valid tool group to proceed


        cd  ${PGSCPPO}/${GRP}
        echo ""
        echo "Building group: $GRP to PGS Toolkit library"
    
        #
        # copy all object files to ./tmp without the leading PGS_<XXX>_ in filename
        #

	if ( -d "tmp" ) then
	    \rm -rf tmp
	    if ( -d "tmp") then
		echo "unable to delete old 'tmp' directory in $PGSCPPO/$GRP,"
		echo "skipping $GRP ($PGSLIB/libPGSTKcpp.a will NOT be updated"
		echo "with $GRP)"
		continue
	    endif
	endif

        mkdir tmp
        if (-d "tmp" ) then
        else
            echo "Unable to create local directory ${PGSCPPO}/${GRP}/tmp"
            echo "$GRP not built to PGS Toolkit library"
	    echo ""
            exit
        endif

        # Sun, SGI, DEC versions: awk cannot handle multi-char field separators
        # so we do it the hard way, using for loops and printf statements

	if ( -e ".cp-tmp" ) then
	    \rm -f .cp-tmp
	    if ( -e ".cp-tmp") then
		echo "unable to delete old '.cp-tmp' file in $PGSCPPO/$GRP,"
		echo "skipping $GRP ($PGSLIB/libPGSTKcpp.a will NOT be updated"
		echo "with $GRP)"
		continue
	    endif
	endif

	set PGSFILES=""
	if ( -e $PGSLIB/libPGSTKcpp.a ) then
	   if ($BRAND == "linux" || $BRAND == "linux32" || $BRAND == "linux64" || $BRAND == "macintosh" || $BRAND == "macintel") then
	      set tmp_PGSFILES=`/bin/ls -C1 *.o`
	   else
		if ($BRAND == "sun5.9" || $BRAND == "sun5.10") then
		    set tmp_PGSFILES=`/bin/ls -C1 *.o`
		else
		    set tmp_PGSFILES=`/bin/ls -C1 -t *.o $PGSLIB/libPGSTK.a | sort -r`
		endif
	   endif
	    foreach tmp_PGSFILE ( $tmp_PGSFILES )
		if ( "`basename $tmp_PGSFILE`" == "libPGSTKcpp.a" ) then
                break
		endif
		set PGSFILES=($PGSFILES `basename $tmp_PGSFILE`)
	    end
	else
	    set PGSFILES=`/bin/ls *.o`
	endif

# Basename command seem not work correctly for linux (g++ compiler), so we have to modify this part. 

     if ($BRAND == "linux" || $BRAND == "linux32" || $BRAND == "linux64" || $BRAND == "macintosh" || $BRAND == "macintel") then
	foreach PGSFILE (*.o)
	    set shortname = `echo $PGSFILE | awk -F_ '{for (i = 3; i < NF; ++i) printf "%s_", $i } {printf "%s\n", $NF} '`
	    set grpname=`echo $shortname | awk '{ min=13; len = length($1); start = 1+len-min; if (start<1) start=1; printf "%s\n", substr($1, start, min-2) }'`
	    echo "cp $PGSFILE tmp/$GRP$grpname" >>! .cp-tmp
	end
     else 
	foreach PGSFILE ($PGSFILES)
	    set shortname = `echo $PGSFILE | awk -F_ '{for (i = 3; i < NF; ++i) printf "%s_", $i } {printf "%s\n", $NF} '`
	    set grpname=`echo $shortname | awk '{ min=13; len = length($1); start = 1+len-min; if (start<1) start=1; printf "%s\n", substr($1, start, min-2) }'`
	    echo "cp $PGSFILE tmp/$GRP$grpname" >>! .cp-tmp
	end
      endif
	# 
	# copy files (as above) if .cp-tmp is not zero size
	# otherwise indicate that no PGS_ files were found
	#

	if ( -e ".cp-tmp" ) then
            echo "Copying files ..."
            echo ""
            cat    .cp-tmp
            echo ""

            source .cp-tmp


            #
            # add group $GRP to library
            #
            echo "Adding $GRP files to library ..."
            ar r $PGSLIB/libPGSTKcpp.a tmp/*

            #
            # clean up
            # set MADE_A_LIB to 1,
	    # this indicates at least one successful
 	    # change was made to libPGSTKcpp.a
	    # 

            echo "Cleaning up ${GRP}..."
            rm -rf tmp .cp-tmp
            echo ""
	    set MADE_A_LIB=1
	else
	    if ( `echo $tmp_PGSFILES | wc -l` > 0 ) then
		echo "$GRP is up to date"
		echo ""
		set UpToDate=1
	    else
		echo "no PGS files in $GRP"
		echo ""
	    endif
	
            rm -rf tmp
        endif
    else
	echo "$PGSCPPO/$GRP does not exist: cannont add $GRP to libPGSTKcpp.a"
    endif
end

#    
# if at least one successful change
# was made to libPGSTKcpp.a then set
# the appropriate permissions and quit
# otherwise move original library back
#

if ($?MADE_A_LIB) then
    if ($BRAND == "sun4") then
        ranlib ${PGSLIB}/libPGSTKcpp.a
    endif
    chmod 666 ${PGSLIB}/libPGSTKcpp.a
    switch ( $BRAND )
	case sgi:
	case sgi32:
	case sgi64:
	    sleep 1   # because the !#$*%&@ SGI gets confused
	breaksw
    endsw
    echo ""
    echo "Done."
    echo ""
else
    if ($?UpToDate != 0) then
	echo ""
	echo "$PGSLIB/libPGSTKcpp.a is up to date"
    else
	echo ""
	echo "failed to create new library"
    endif
    if ( -e ${PGSLIB}/libPGSTKcpp.a.old && $?MAKE_BACKUP != 0 ) then
	echo "restoring old library..."

        cp libPGSTKcpp.a.old libPGSTKcpp.a
    endif
    echo ""
    echo "Done."
    echo ""
endif










