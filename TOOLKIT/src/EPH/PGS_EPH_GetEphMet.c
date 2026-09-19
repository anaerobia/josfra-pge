/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_GetEphMet.c

DESCRIPTION:
  This file contains the function PGS_EPH_GetEphMet()
   This function returns the metadata associated with toolkit spacecraft
   ephemeris files.

AUTHORS:
  Guru Tej S. Khalsa / Applied Research Corporation
  Peter D. Noerdlinger / Applied Research Corporation
  Abe Taaheri          / Space Applications Corporation
  Phuong T. Nguyen     / L3 Communication Corp.
  Adura Adekunjo       / L3 Communication Corp.
HISTORY:
  01-Jul-1996  GTSK  Initial version
  27-May-1997  PDN   removed references to predicted leap seconds
  28-Sep-1998  AT    Added PGS_EPH_GetEphMetAux function to wrap around
                     PGS_EPH_GetEphMet. The new function (which has one more
                     argument than PGS_EPH_GetEphMet) is used by FORTRAN
                     binder to resolve out of bound write when the function is
                     called from a FORTRAN code (NCR ECSed17947 )
  07-Sep-1999  PDN   added checks for metadata out of order, etc. and
                     new return status: PGSEPH_W_CORRUPT_METADATA
  04-Jan-2000  PDN   added further checks for metadata out of order, etc.
                     Fixed examples to include the output "orbitNumber",
                      which had been omitted.
  10-Feb-2002  XW    Got ephemeris data from HDF files.
  21-Oct-2002  PTN   Convert the big-endian data read from the attitude and
                     ephemeris binary files into little-endian format.
  20-Nov-2002  AA    Added  the hdfcheck flag to distinguish between binary 
                     and hdf files (ECSed35587). 
  14-Feb-2003  AA    Included "lendcheck" flag so that data conversion can be
                     from big-endian to little-endian format and vice-versa.
  20-Mar-2003  AT    fixed problems with proper reading of HDF files, and 
                     speeding up by not doing unnecessary stuff.
  29-May-2003  AT    fixed problems with extracting orbit metadata from
                     multiple HDF format att/eph files.
  23-Jul-2003  AT    Modified code after the call to PGS_EPH_checkHDF2() so
                     that we do not treat PGSEPH_W_CORRUPT_METADATA, returned
                     from that function, as an error.
  12-Aug-2003  AT    Fixed problem with getting orbit metadata when using HDF
                     eph file and having start time as the end time of one 
                     file. Also added a few lines of code for using both
                     binary or hdf eph files and having start time such that
                     preceeding orbit has metadata in two files. Now code will
                     get both, however, will find duplication, and in case of
                     difference in their Ascending, or descending time,.. it
                     will issue warnimg and keep only one.
  15-Oct-2003  AT    Added code so that if tool finds 2 orbits with same orbit
                     number but different meradata values from two adjacent 
                     files, it issues a warning and sets returnstatus to 
                     (1) PGS_S_SUCCESS if the difference is smaller than a 
                     tolarable number (2) PGSEPH_W_CORRUPT_METADATA if the 
                     difference is not small
                     Note: Once PGE03 is fixed, returnstatus must be set
                           to PGSEPH_W_DBLVALUE_METADATA instead of 
                           PGS_S_SUCCESS in the part that uses binary eph/att
                           files. In the part that uses HDF eph/att files, 
                           currently correct warning code is returned.

END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Spacecraft Ephemeris Metadata

NAME:
   PGS_EPH_GetEphMet()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

    PGSt_SMF_status
    PGS_EPH_GetEphMet(
        PGSt_tag      spacecraftTag,
        PGSt_integer  numValues,
        char          asciiUTC[28],
        PGSt_double   offsets[],
        PGSt_integer* numOrbits,
        PGSt_integer  orbitNumber[],
        char          orbitAscendTime[][28],
        char          orbitDescendTime[][28],
        PGSt_double   orbitDownLongitude[])

FORTRAN:
      include 'PGS_EPH_5.f'
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_eph_getephmet(spacecrafttag,numvalues,
     >                                   asciiutc,offsets,numorbits,
     >                                   orbitnumber,orbitascendtime,
     >                                   orbitdescendtime,
     >                                   orbitdownlongitude)

      integer          spacecrafttag
      integer          numvalues
      character*27     asciiutc
      double precision offsets(*)
      integer          numorbits
      integer          orbitnumber(*)
      character*27     orbitascendtime(*)
      character*27     orbitdescendtime(*)
      double precision orbitdownlongitude(*)

DESCRIPTION:
   This function returns the metadata associated with toolkit spacecraft
   ephemeris files.

INPUTS:
   Name              Description               Units   Min         Max
   ----              -----------               -----   ---         ---
   spacecraftTag     spacecraft identifier     N/A

   numValues         num. of values requested  N/A

   asciiUTC          UTC time reference base ASCII   1961-01-01 see NOTES
                     time in CCSDS ASCII time
                     code A format

   offsets           array of time offsets in  sec    ** depends on asciiUTC **
                     seconds relative to
                     asciiUTC

OUTPUTS:
   Name               Description              Units   Min         Max
   ----               -----------              -----   ---         ---
   numOrbits          number of orbits         N/A
                      spanned by data set

   orbitNumber        array of orbit numbers   N/A
                      spanned by data set

   orbitAscendTime    array of times of        ASCII
                      spacecraft northward
                      equator crossings

   orbitDescendTime   array of times of        ASCII
                      spacecraft southward
                      equator crossings

   orbitDownLongitude array of longitudes      radians
                      or spacecraft
                      southward equator
                      crossings

RETURNS:
   PGS_S_SUCCESS               successful return
   PGSTD_E_SC_TAG_UNKNOWN      unknown/unsupported spacecraft tag
   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c ephem files could be found for 
                               input times
   PGSEPH_E_BAD_ARRAY_SIZE     array size specified is less than 0
   PGSTD_E_TIME_FMT_ERROR      format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR    value error in asciiUTC
   PGSEPH_W_CORRUPT_METADATA   one or more ephemeris files have out of order 
                               or corrupt metadata
   PGS_E_TOOLKIT               an unexpected error occurred

EXAMPLES:
C:
    #include <PGS_EPH.h>

    #define ORBIT_ARRAY_SIZE 5   /# maximum number of orbits expected #/
    #define EPHEM_ARRAY_SIZE 100 /# number of ephemeris data points #/

    PGSt_double     offsets[EPHEM_ARRAY_SIZE];
    PGSt_double     orbitDownLongitude[ORBIT_ARRAY_SIZE];

    PGSt_integer    numOrbits;

    PGSt_integer    orbitNumber[ORBIT_ARRAY_SIZE];

    char            asciiUTC[28];
    char            orbitAscendTime[ORBIT_ARRAY_SIZE][28];
    char            orbitDescendTime[ORBIT_ARRAY_SIZE][28];

    /# initialize asciiUTC and offsets array with the times for actual
       ephemeris records that will be processed (i.e. by some other tool) #/

    strcpy(asciiUTC,"1998-02-03T19:23:45.123");
    for (i=0;i<EPHEM_ARRAY_SIZE;i++)
    {
        offsets[i] = (PGSt_double) i*60.0;
    }

    /# get the ephemeris metadata associated with these times #/

    returnStatus = PGS_EPH_GetEphMet(PGSd_EOS_AM,EPHEM_ARRAY_SIZE,asciiUTC,
                                     offsets,&numOrbits,orbitNumber,
                                     orbitAscendTime,
                                     orbitDescendTime,orbitDownLongitude);

    if (returnStatus != PGS_S_SUCCESS)
    {
                :
     ** do some error handling ***
                :
    }

    /# numOrbits will now contain the number of orbits spanned by the data set
       (as defined by asciiUTC and EPHEM_ARRAY_SIZE offsets). orbitAscendTime
       will contain numOrbits ASCII UTC times representing the time of 
       northward equator crossing of the spacecraft for each respective orbit.
       orbitDescendTime will similarly contain the southward equator crossing
       times and orbitDownLongitude will contain the southward equator crossing
       longitudes #/

FORTRAN:
      implicit none

      include 'PGS_EPH_5.f'
      include 'PGS_TD.f'
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer orbit_array_size/5/   ! maximum number of orbits expected
      integer ephem_array_size/100/ ! number of ephemeris data points

      double precision offsets(ephem_array_size)
      double precision orbitdownlongitude(orbit_array_size)

      integer          numorbits
      integer          orbitnumber(orbit_array_size)

      character*27     asciiutc
      character*27     orbitascendtime(orbit_array_size)
      character*27     orbitdescendtime(orbit_array_size)

!    initialize asciiutc and offsets array with the times for actual
!    ephemeris records that will be processed (i.e. by some other tool)

      asciiutc = '1998-02-03T19:23:45.123'
      do 100 i=1,ephem_array_size
          offsets(i) = i*60.D0
 100  continue

!    get the ephemeris metadata associated with these times

      returnStatus = pgs_eph_getephmet(pgsd_eos_am,ephem_array_size,
     >                                 asciiutc,offsets,numorbits,orbitnumber,
     >                                 orbitascendtime,orbitdescendtime,
     >                                 orbitdownlongitude)

      if (returnStatus .ne. pgs_s_success) then
                  :
       ** do some error handling ***
                  :
      endif

!    numOrbits will now contain the number of orbits spanned by the data set
!    (as defined by asciiUTC and EPHEM_ARRAY_SIZE offsets). orbitAscendTime
!    will contain numOrbits ASCII UTC times representing the time of northward
!    equator crossing of the spacecraft for each respective orbit.
!    orbitDescendTime will similarly contain the southward equator crossing
!    times and orbitDownLongitude will contain the southward equator crossing
!    longitudes

NOTES:
   This function will determine the time span of the data set from the 
   input UTC reference time and the offsets array.  It will then determine 
   the metadata for all orbits spanned by the data set.

   TIME ACRONYMS:

     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second 
     (TAI-UTC) values to UTC Julian dates.  The file leapsec.dat starts at 
     Jan 1, 1961; therefore an error status message will be returned if 
     this function is called with a time before that date.  The file, 
     which is updated when a new leap second event is announced, 
     contains actual (definitive) and predicted (long term; very 
     approximate) dates of leap second events.  The latter can be used 
     only in simulations.   If an input date is outside of the range of
     dates in the file (or if the file cannot be read) the function will use a
     calculated value of TAI-UTC based on a linear fit of the data known to be
     in the table.  This value of TAI-UTC is relatively crude estimate and may
     be off by as much as 1.0 or more seconds.  Thus, when the function is used
     for dates in the future of the date and time of invocation, the user ought
     to carefully check the return status.  The status level of the return
     status of this function will be 'W' (at least) if the TAI-UTC value used
     is a predicted value.  The status level will be 'E' if the TAI-UTC value
     is calculated (although processing will continue in this case, using the
     calculated value of TAI-UTC).

   ASCII UTC:

     Toolkit functions that accept an ASCII time as input require the time to
     be UTC time in CCSDS ASCII Time Code A or CCSDS ASCII Time Code B format.
     Toolkit functions that return an ASCII time return the UTC time in CCSDS
     ASCII Time Code A format (see CCSDS 301.0-B-2 for details).
     (CCSDS => Consultative Committee for Space Data Systems)

      The general format is:

          YYYY-MM-DDThh:mm:ss.ddddddZ (Time Code A)
          YYYY-DDDThh:mm:ss.ddddddZ   (Time Code B)

          where:
              -,T,: are field delimiters
              Z is the (optional) time code terminator
              other fields are numbers as implied:
                Time Code A:
                   4 Year digits, 2 Month, 2 Day, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second
                Time Code B:
                   4 Year digits, 3 Day of year, 2 hour,
                   2 minute, 2 second, and up to 6 decimal
                   digits representing a fractional second

   TOOLKIT INTERNAL TIME (TAI):

     Toolkit internal time is the real number of continuous SI seconds since 
     the epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred 
     to in the toolkit as TAI (upon which it is based).

   TIME OFFSETS:

      This function accepts an ASCII UTC time, an array of time offsets and the
      number of offsets as input.  Each element in the offset array is an 
      offset in seconds relative to the initial input ASCII UTC time.

      An error will be returned if the number of offsets specified is less than
      zero.  If the number of offsets specified is actually zero, the offsets
      array will be ignored.  In this case the input ASCII UTC time will be
      converted to Toolkit internal time (TAI) and this time will be used to
      process the data.  If the number of offsets specified is one (1) or
      greater, the input ASCII UTC time will converted to TAI and each element
      'i' of the input data will be processed at the time: (initial time) +
      (offset[i]).

      Examples:

         if numValues is 0 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
         then input[0] will be processed at time 432000.0 and return output[0]

         if numValues is 1 and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
         then input[0] will be processed at time 432000.0 + offsets[0] and
         return output[0]

         if numValues is N and asciiUTC is "1993-001T12:00:00" (TAI: 432000.0),
         then each input[i] will be processed at time 43000.0 + offsets[i] and
         the result will be output[i], where i is on the interval [0,N)

REQUIREMENTS:
   PGSTK - 0720

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_IO_Gen_Open()
   PGS_IO_Gen_Close()
   PGS_TD_UTCtoTAI()
   PGS_TD_TAItoUTC()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_SetDynamicMsg()

END_PROLOG:
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_SMF.h>
#include <PGS_IO.h>
#include <PGS_TD.h>
#include <PGS_EPH.h>
#include <dirent.h>
#include <hdf.h>
#include <mfhdf.h>
#include <PGS_math.h>
#include <arpa/inet.h>

extern int byteswap(char *, int l);
#define THEPI 3.14159265
#define ADTIME_TOLERANCE 60.0     /* Ascending/Descending time tolerance */
#define DLONGI_TOLERANCE 0.004363 /* Descending Longitude tolerance, see
				     notes for crossing-longitude threshold
				     in this file */

/* name of this function */
#define FUNCTION_NAME "PGS_EPH_GetEphMet()"
#define FUNCTION_NAME2 "PGS_EPH_checkHDF2()"

#define DPREP_E_PGS_ERROR  "PGS Toolkit error "
#define MAX_NUM_ORBS 5001  /* max number of orbits in 24 hr. Note 
			     that for AM, PM and AURA actual value
			     is about 16 orbits in 24 hr. */

/* prototype for function used to get ephemeris data from HDF files */
static PGSt_SMF_status 
PGS_EPH_checkHDF2(PGSt_tag, PGSt_double, PGSt_double, PGSt_integer *,
		  PGSt_integer [MAX_NUM_ORBS], char [MAX_NUM_ORBS][28], 
		  char [MAX_NUM_ORBS][28], PGSt_double [MAX_NUM_ORBS]);

void PGS_EPH_ShellSort(PGSt_integer n, PGSt_integer hon[],
		       PGSt_double  hoa[], PGSt_double  hod[],
		       PGSt_double  hol[]);

PGSt_SMF_status
PGS_EPH_GetEphMet(                           /* get metadata associated with a
                                                toolkit ephemeris file */
    PGSt_tag         spacecraftTag,          /* spacecraft ID tag */
    PGSt_integer     numValues,              /* number of values requested */
    char             asciiUTC[28],           /* reference base time */
    PGSt_double      offsets[],              /* time offsets relative to
                                                asciiUTC */
    PGSt_integer*    numOrbits,              /* number of orbits spanned by
                                                data set */
    PGSt_integer     orbitNumber[],          /* array of orbit numbers */
    char             orbitAscendTime[][28],  /* array of ascend times */
    char             orbitDescendTime[][28], /* array of descend times */
    PGSt_double      orbitDownLongitude[])   /* array of s/c downward equator
                                                crossing longitudes */
{
    PGSt_ephemHeader   fileHeader;           /* s/c ephemeris file header */
    PGSt_SMF_status    pgsStatus;
    static int         hdfcheck=0;    /* checks if HDF/Binary file is needed */
    static int         anum = 0;

    PGSt_integer       lendcheck= -1; /* little/big endian flag */
    char               File[256];

    int                i;                    /* loop counter */
    int                j;                    /* loop counter */

    size_t             numCheck;             /* used to check calls to fread */

    PGSt_integer       num_files;            /* number of physical files
                                                 spanned by data set */
    PGSt_integer       totalOrbits=0;        /* total number of orbits spanned
						by ephemeris files containing
						at least some orbit data for
						the requested input times. This
						total may include duplicates, 
						as the same orbit number may 
						occur in different files. 
						See next item */
    PGSt_double       laggedorbitDescendLongitude;
    PGSt_integer      laggedOrbitNumber=0;   /* lagged orbit number for tests*/
    PGSt_double       laggedAscendTime;      /* lagged Ascend time for tests*/
    PGSt_double       laggedDescendTime;     /* lagged Descend time for tests. 
                                                 orbitNumber[*numOrbits] right
						 after an orbit number is 
						 "promoted" from the list of 
						 those that are found in one 
						 of the data files to one that
						 is recorded. */
    PGSt_double       laggedAscendTime1;
    PGSt_double       laggedDescendTime1;
    PGSt_integer      laggedOrbitNumber1=0;

    PGSt_integer      start_file_num;        /* file number of first physical
                                                 file */
    PGSt_integer      stop_file_num;         /* file number of last physical
                                                 file */
    PGSt_integer      previous_orbit_num=-2; /* stop orbit number of previous
                                                 file */
    PGSt_integer      *sortedIndex=NULL;     /* pointer to array holding 
						 sorted indices of time offset
						 array (offsets) */

    PGSt_scTagInfo    scTagInfo;              /* s/c tag info. structure */

    char              specifics[PGS_SMF_MAX_MSG_SIZE];   /* message */
    char              errorBuf[PGS_SMF_MAX_MSGBUF_SIZE]; /* appended error msg */

    PGSt_ephemMetadata *orbMetadata=NULL;     /* pointer to s/c metadata */

    PGSt_double       startTAI93;             /* initial time of user request
						 interval */
    PGSt_double       stopTAI93;              /* final time of user request 
						 interval */

    FILE              *ephemFilePtr=NULL;     /* ptr to s/c ephemeris file */

    PGSt_SMF_status   returnStatus;           /* status of this  call */

    PGSt_integer      jdata = 0;
    
    PGSt_SMF_status   returnStatus1;          /* status of TK func. calls */

    PGSt_hdrSummary   *header_array=NULL;     /* pointer to array of header
                                                 summaries */
    PGSt_boolean      foundSome = PGS_FALSE;
    char 	      XhdfAscendTime[MAX_NUM_ORBS][28]; /*Ascend times */
    char 	      XhdfDescendTime[MAX_NUM_ORBS][28];/*Descend times */
    PGSt_double       XhdforbitLongitude[MAX_NUM_ORBS]; /*Downward equator
							  crossing longitudes*/
    PGSt_integer      XnumOrbits=0; 	                /* Number of orbits 
							   spanned by data 
							   set */
    PGSt_integer      XorbitNumber[MAX_NUM_ORBS]; 	/* Orbit numbers */
    PGSt_integer      ifile, ithfile;
    PGSt_integer      num_files_eph = 0;
    PGSt_integer      hdfcheck_next=0;
    PGSt_boolean      replace_flag = PGS_FALSE;

    /*******************
     * BEGIN EXECUTION *
     *******************/

    /* check spacecraft tag for validity */

    returnStatus1 = PGS_EPH_GetSpacecraftData(spacecraftTag, NULL,
                                             PGSe_TAG_SEARCH, &scTagInfo);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
        break;

      case PGSTD_E_SC_TAG_UNKNOWN:
      case PGS_E_TOOLKIT:
        return returnStatus1;

      default:
        PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
        return PGS_E_TOOLKIT;
    }

    /* convert input asciiUTC to equivalent TAI value */

    returnStatus1 = PGS_TD_UTCtoTAI(asciiUTC, &startTAI93);
    switch (returnStatus1)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
        break;

      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
        return returnStatus1;

      default:
        PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
        return PGS_E_TOOLKIT;
    }

    if (numValues > 0)
    {
        returnStatus1 = PGS_MEM_Calloc((void **) &sortedIndex, 1,
                                      sizeof(PGSt_integer)*numValues);
        if (returnStatus1 != PGS_S_SUCCESS)
            return returnStatus1;

        PGS_TD_sortArrayIndices(offsets,numValues,sortedIndex);

        stopTAI93 = startTAI93 + offsets[sortedIndex[numValues-1]];
        startTAI93 = startTAI93 + offsets[sortedIndex[0]];

        PGS_MEM_Free(sortedIndex);
    }
    else if (numValues == 0)
    {
        stopTAI93 = startTAI93;
    }
    else
    {
        PGS_SMF_SetStaticMsg(PGSEPH_E_BAD_ARRAY_SIZE, FUNCTION_NAME);
        return PGSEPH_E_BAD_ARRAY_SIZE;
    }

    /* check that all eph files are HDF or BINARY */
    if (hdfcheck == 0 && anum == 0)
      { 
	pgsStatus =PGS_PC_GetNumberOfFiles(PGSd_SC_EPHEM_DATA, &num_files_eph);
	
	if (pgsStatus != PGS_S_SUCCESS)
	  {
	    pgsStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	    PGS_SMF_SetDynamicMsg(pgsStatus,
				  "error in accessing spacecraft ephemeris file(s)",
				  FUNCTION_NAME);
	    return pgsStatus;
	  }
	
	for(ifile = 1; ifile<= num_files_eph; ifile++)
	  {
	    ithfile = ifile;
	    pgsStatus =PGS_PC_GetReference(PGSd_SC_EPHEM_DATA, &ithfile, File);
	    
	    if (pgsStatus != PGS_S_SUCCESS)
	      {
		PGS_SMF_SetDynamicMsg(pgsStatus,
				      "Error getting reference for eph files",
				      FUNCTION_NAME);
		return pgsStatus;
	      }
	    
	    hdfcheck_next = Hishdf(File);
	    if(ifile != 1 && hdfcheck != hdfcheck_next)
	      {
		PGS_SMF_SetDynamicMsg(PGSEPH_E_NO_SC_EPHEM_FILE,
				      "All ephemeris files must be either HDF or BINARY",
				      FUNCTION_NAME);
		pgsStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		return pgsStatus;
	      }

		anum = 1;
		hdfcheck = hdfcheck_next;
	  }
      }


    if ( hdfcheck == 0)    /* EPH data files are BINARY */
      {
        /* get the header summary array */
	
	returnStatus1 = PGS_EPH_getEphemHeaders(&scTagInfo, &header_array,
						&num_files, &lendcheck);
	if (returnStatus1 != PGS_S_SUCCESS)
	  {
	    PGS_SMF_SetDynamicMsg(returnStatus1,
				  " error in accessing spacecraft "
				  "file(s)", FUNCTION_NAME);
	    return returnStatus1;
	  }
      }
    
    if (hdfcheck == 1)    /* EPH data files are HDF */
      {
	/* To get ephemeris data from HDF files */
	returnStatus = PGS_EPH_checkHDF2(spacecraftTag, startTAI93,stopTAI93, 
					 &XnumOrbits, XorbitNumber, 
					 XhdfAscendTime, XhdfDescendTime, 
					 XhdforbitLongitude );

	if ((returnStatus == PGS_S_SUCCESS) || 
	    (returnStatus == PGSEPH_W_CORRUPT_METADATA) ||
	    (returnStatus == PGSEPH_W_DBLVALUE_METADATA))
	  {
	    *numOrbits = XnumOrbits;

	    if ( *numOrbits == 0 )
	      {
		returnStatus = PGSEPH_W_CORRUPT_METADATA;
		PGS_SMF_SetDynamicMsg(returnStatus,
				      "no meaningful orbital metadata found",
				      FUNCTION_NAME);
		return returnStatus;
	      }

	    for ( j = 0; j < XnumOrbits; j++)
	      {
		orbitNumber[j] = XorbitNumber[j];
		orbitDownLongitude[j] = XhdforbitLongitude[j];
		strcpy(orbitAscendTime[j], XhdfAscendTime[j]);
		strcpy(orbitDescendTime[j], XhdfDescendTime[j]);
	      }
	    foundSome = PGS_TRUE;
	    return returnStatus;
	  }
	else
	  {
	    PGS_SMF_SetDynamicMsg(returnStatus,
				  " error in accessing spacecraft "
				  "file(s)", FUNCTION_NAME);
	    return returnStatus;
	  }
      }

    /* Files are not HDF type */
    if (startTAI93 < header_array[0].startTAI93 ||
        stopTAI93 > header_array[num_files-1].stopTAI93)
    {
      returnStatus1 = PGSEPH_E_NO_SC_EPHEM_FILE;
      PGS_SMF_SetDynamicMsg(returnStatus1,
			    "input time outside range of staged ephemeris "
			    "file(s)", FUNCTION_NAME);
      return returnStatus1;
    }
    
    
    /* determine which of the staged physical s/c ephemeris files are 
       spanned by the data set (as defined by the start and stop times) */
    
    if(header_array[0].stopTAI93 <=  header_array[0].startTAI93)
      {
	goto ERROR_IN_FILE; /* special case because next loop starts at 1 */
      }
    
    start_file_num = 0;
    
    for (i=1; i<num_files; i++)
      {
	/* check for consistent time order in file headers before using */
        if((header_array[i].stopTAI93 <=  header_array[i].startTAI93)
	   /* files already sorted by start time ! */
	   || (header_array[i-1].stopTAI93 >  header_array[i].stopTAI93))
	  {
	  ERROR_IN_FILE:
            returnStatus = PGSEPH_W_CORRUPT_METADATA;
            sprintf(specifics,"\nNear TAI93 time # %f "
		    ,header_array[i-1].stopTAI93);
            PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
            strcat(errorBuf,specifics);
            PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
            return returnStatus;
	  }
        if (startTAI93 < header_array[i].startTAI93)
	  {
            start_file_num = i-1;
            break;
	  }
        else
	  {
            start_file_num = i;
	  }
      }
    
    stop_file_num = 0;
    
    for (i=1; i<num_files; i++)
      {
        if (stopTAI93 < header_array[i].startTAI93)
	  {
            if ( stopTAI93 > header_array[i-1].stopTAI93  )
	      {
                stop_file_num = i;
	      }
            else
	      {
                stop_file_num = i-1;
	      }
            break;
	  }
        else
	  {
            stop_file_num = i;
	  }
      }
    
    /* extract the data set metadata */
    
    *numOrbits = 0;
    
    for (i=start_file_num;i<=stop_file_num;i++)
      {
        /* open s/c ephemeris file at requested time */
	
        returnStatus1 = PGS_IO_Gen_Open(PGSd_SC_EPHEM_DATA,
					PGSd_IO_Gen_Read,
					&ephemFilePtr,
					header_array[i].file_version_num);
	
        if (returnStatus1 != PGS_S_SUCCESS)
	  {
            returnStatus = PGS_E_TOOLKIT;
            PGS_SMF_SetDynamicMsg(returnStatus,
                                  "Unexpected failure opening ephemeris file.",
                                  FUNCTION_NAME);
            if (orbMetadata != NULL)
	      {
                free(orbMetadata);
	      }
            return returnStatus;
	  }
	
        /* read the full ephemeris file header */
	
        numCheck = fread(&fileHeader, sizeof(PGSt_ephemHeader), 1,
                         ephemFilePtr);
        if (numCheck != 1)
        {
            returnStatus = PGS_E_TOOLKIT;
            PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
                                  "ephemeris file header.", FUNCTION_NAME);
            if (orbMetadata != NULL)
            {
                free(orbMetadata);
            }
            return returnStatus;
        }


	if(lendcheck == 1) /* Machine Endianness does not agree with 
			      binary eph file Endianness; need swaping
			      data read from that file */
	  {
	    /* Convert to local byte order, assume IEEE */
	    
	    char *ptr, *ptr1, *ptr2, *ptr3, *ptr4;
	    int ll, ll1, n, m, q;
	    
	    ptr = (char *) (&fileHeader.startTime);
	    ll  = sizeof(PGSt_double);
	    byteswap(ptr, ll);
	    
	    ptr1 = (char *) (&fileHeader.endTime);
	    ll1   = sizeof(PGSt_double);
	    byteswap(ptr1, ll1);
	    
	    ptr2 = (char *) (&fileHeader.interval);
	    for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
	    
	    ptr3 = (char *) (&fileHeader.keplerElements);
	    for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
	    
	    ptr4 = (char *) (&fileHeader.qaParameters);
	    for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
	  }
    
	/* verify that this file is actually the next in line (in case 
	   there are gaps in the staged files) */
	
	if (i != start_file_num)
	  {
            if (fileHeader.orbitNumberStart != previous_orbit_num &&
                fileHeader.orbitNumberStart != previous_orbit_num+1)
	      {
                if (stopTAI93 > header_array[i-1].stopTAI93)
		  {
		    if (orbMetadata != NULL)
		      {
			free(orbMetadata);
		      }
		    returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		    sprintf(specifics,
			    "\nGap in staged data or orbit number sequence at orbits # %d and %d"
			    ,fileHeader.orbitNumberStart,previous_orbit_num);
		    PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
		    strcat(errorBuf,specifics);
		    PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
		    return returnStatus;
		  }
		break;
	      }
	  }
	
        previous_orbit_num = fileHeader.orbitNumberEnd;
        totalOrbits += fileHeader.nOrbits;

        /* move to the end of the records (i.e. to the beginning of the s/c
           ephemeris file metadata */
	
        fseek(ephemFilePtr,
              (long)(sizeof(PGSt_ephemRecord)*fileHeader.nRecords) +
              (long)(fileHeader.nURs*PGSd_UR_FIELD_SIZE),
              SEEK_CUR);
	
        /* extract the appropriate metadata for the data set */
	
        orbMetadata = (PGSt_ephemMetadata*) realloc((void*)orbMetadata,
				  (totalOrbits+1)*sizeof(PGSt_ephemMetadata));
        numCheck = fread(orbMetadata+(totalOrbits-fileHeader.nOrbits),
                         sizeof(PGSt_ephemMetadata),
                         fileHeader.nOrbits, ephemFilePtr);
	
        if (numCheck != fileHeader.nOrbits)
	  {
            returnStatus = PGS_E_TOOLKIT;
            PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading"
                                  " ephemeris file header.", FUNCTION_NAME);
            if (orbMetadata != NULL)
	      {
                free(orbMetadata);
	      }
            return returnStatus;
	  }
	
	if(lendcheck == 1)
	  {
	    for (jdata = 0; jdata < fileHeader.nOrbits; jdata++)
	      {
		/* Convert to local byte order, assume IEEE */
		
		int n;
		char *c;
		
		c = (char *)(orbMetadata +
			     (totalOrbits - fileHeader.nOrbits) + jdata);
		byteswap(c, 4);
		c+=8;
		for(n=0; n<3; n++){
		  byteswap(c, 8);
		  c+=8;
		}
	      }
	  }
	
        /* close this physical file */
	
        returnStatus = PGS_IO_Gen_Close(ephemFilePtr);

        if (returnStatus != PGS_S_SUCCESS)
        {
            PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
                                  "Unexpected failure closing ephemeris "
                                  "file.", FUNCTION_NAME);
            if (orbMetadata != NULL)
	      {
                free(orbMetadata);
	      }
            return PGS_E_TOOLKIT;
        }
      } /* end if (numCheck != fileheader... */
    
    orbMetadata[totalOrbits].orbitAscendTime =
      orbMetadata[totalOrbits-1].orbitAscendTime + PGSd_GEO_ERROR_VALUE;
    orbMetadata[totalOrbits].orbitNumber =
      orbMetadata[totalOrbits-1].orbitNumber + MAX_NUM_ORBS;
    for (j=0;j<totalOrbits;j++)
      {
	/* these metadata are only relevant if any part of this orbit is
	   spanned by the data set */
	
	if (startTAI93 >= orbMetadata[j+1].orbitAscendTime)
	  {
	    /* we do not want to miss first 
	       orbit metadata that is taken
	       from two consecutive eph files
	       that have parts of the same orbit */
	      
	      if(orbMetadata[j].orbitNumber != orbMetadata[j+1].orbitNumber)
		{
		  continue;
		}
	      else if(orbMetadata[j].orbitNumber == orbMetadata[j+1].orbitNumber)
		{
		  if(*numOrbits == 0) 
		    {
		      if(startTAI93 >= orbMetadata[j+2].orbitAscendTime)
			{
			  continue;
			}
		    }
		}
	    }
	
	if (stopTAI93 < orbMetadata[j].orbitAscendTime)
	  {
	    foundSome = PGS_TRUE;
	    break;
	  }
	
	/* if this is not the first pass through (i.e. we have already recorded
	   information about at least one orbit) check the current orbit number
	   against the last orbit number recorded, don't record duplicate orbit
	   numbers (this can happen when the data for an orbit is found in two
	   separate physical files since each file will contain the metadata 
	   for that particular orbit and this metadata will obviously be the 
	   same in both cases) */
	
	if ( *numOrbits > 0 )
	  {
	    replace_flag = PGS_FALSE; /* flag that is set to TRUE if we find
					 two orbits with the same orbit 
					 number */

	    if ( orbMetadata[j].orbitNumber ==
		 orbitNumber[(*numOrbits) - 1] )
	      {
		/* first check that the other metadata are consistent for this
		   orbit number.  This checks for the (unlikely?) event that
		   somehow the orbit number failed to increment yet the 
		   spacecraft is on a new orbit, or that two files with the 
		   same orbit contain inconsistent accompanying metadata. 
		   If these checks are passed, go on to the next value of j */
		/*Note: There are situations where the orbit metadata for 
		  the same orbit (orbit metadata which are associated with
		  different granules) can be different.  This occurs when 
		  an orbit completes in the granule following the one 
		  currently being processed by DPREP, but the following 
		  granule is not available to DPREP processing.  The granules 
		  that DPREP needs to do the best possible job with the orbit
		  metadata aren't always available in forward processing.  
		  With Terra, for example, DPREP waits 4 hours for the 
		  following granule to become available so that the orbit 
		  metadata can be properly generated.  If after 4 hours 
		  the granule is not available, however, DPREP proceeds 
		  without it, and extrpolates metadata for the incomplete 
		  orbit.  When the following granule is processed, DPREP 
		  produces orbit metadata using the actual node crossing that 
		  is now available.  The extrapolated orbit metadata 
		  from the preceeding granule then differs from the "actual" 
		  orbit metadata from the following granule.  DPREP don't go 
		  back and correct the orbit metadata in the preceeding 
		  granule, for operational reasons, unless the data are 
		  reprocessed later during one of the reprocessing efforts. 
		  A similar situation exists with Aqua/Aura ephemeris 
		  processing, with the exception that DPREP does not look 
		  forward to the following granule at all; reprocessing would 
		  not correct orbit metadata inconsistencies in Aqua/Aura 
		  metadata.  This latter situation can be changed in DPREP 
		  so that at least a reprocessing effort corrects the 
		  orbit metadata.  The problem definitely originates with 
		  DPREP, but it's not clear to me that the solution 
		  necessarily resides in DPREP because of  operational 
		  restrictions.  Production rules could be changed for 
		  Terra DPREP so that DPREP waits until the following 
		  granule is available before processing; this slows data 
		  production however.
		  This certainly isn't the solution either for Aqua/Aura 
		  DPREP because of waiting 24+ hours for the following 
		  granule to become available. 

		  The problem just mentioned fails MODIS PGE03. To avoid 
		  this we will modify this code so that: 
		  (1) return the WARNING when the metadata differ by some 
		  threshold, and 
		  (2) return PGS_S_SUCCESS when within-threshold 
		  inconsistencies are encountered, and provide the latter 
		  (corrected) orbit metadata values to the user. This will be
		  in place until MODIS PGE03 fixes are in place (corrections 
		  to managing return value from PGS_EPH_GetEphMet() in the 
		  file GEO_write_granule_metadata.c, that does not consider
		  warnings as errors).
		*/
		/* Per DPREP's Robert Kummerer's suggestion a crossing-time 
		   threshold (dTime) = 60 seconds will be used. The 
		   crossing-longitude threshold (dLong) derives
		   from the crossing-time threshold as follows: 

		   dLong = (dTime / 86400) * 2 * pi = 0.004363   radians 
		   (86400 is the number of seconds in a day) 

		   We need to be careful when verifying that the 
		   OrbitDescendingLongitude are within threshold. 
		   OrbitDescendingLongitude ranges from -pi to +pi.  
		   We need to be cautious of the fact that the 2
		   OrbitDescendingLongitude We are comparing may 
		   cross the -pi / +pi boundary (a slim chance of 
		   this happening but nonetheless it could happen). 

		 */

		
		if( ( orbMetadata[j].orbitAscendTime !=
		      laggedAscendTime ) )
		  /*  orbMetadata[(*numOrbits)-1].orbitAscendTime ) )*/
		  {
		    if(fabs(orbMetadata[j].orbitAscendTime -
			    laggedAscendTime) < ADTIME_TOLERANCE)
		      {
			returnStatus = PGSEPH_W_DBLVALUE_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nSmall difference in Ascending node times in different data files"
				" for the same orbit, namely # %d; Keeping the one from second file"
				,orbMetadata[j].orbitNumber);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
			/* For the time being replace warning return 
			   with success */
			returnStatus = PGS_S_SUCCESS;
		      }
		    else
		      {
			returnStatus = PGSEPH_W_CORRUPT_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nDifferent Ascending node times in different data files"
				" for same orbit, namely # %d; Keeping the one from second file"
				,orbMetadata[j].orbitNumber);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
		      }
		  }
		if( ( orbMetadata[j].orbitDescendLongitude !=
		      laggedorbitDescendLongitude ) )
		    /*orbMetadata[(*numOrbits)-1].orbitDescendLongitude ) )*/
		  {
		    if((fabs(orbMetadata[j].orbitDescendLongitude -
			     laggedorbitDescendLongitude)<DLONGI_TOLERANCE) ||
		       (fabs(2.0*THEPI - fabs(orbMetadata[j].orbitDescendLongitude -
					      laggedorbitDescendLongitude)) <
			DLONGI_TOLERANCE))
		      {
			returnStatus = PGSEPH_W_DBLVALUE_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nSmall difference in Descending longitudes in different data files"
				" for the same orbit, namely # %d; Keeping the one from second file"
				,orbMetadata[j].orbitNumber);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
			/* For the time being replace warning return 
			   with success */
			returnStatus = PGS_S_SUCCESS;
		      }
		    else
		      {
			returnStatus = PGSEPH_W_CORRUPT_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nDifferent Descending longitudes in different data files"
				" for same orbit, namely # %d; Keeping the one from second file"
				,orbMetadata[j].orbitNumber);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
		      }
		  }
		if( ( orbMetadata[j].orbitDescendTime !=
		      laggedDescendTime ) )	
		  {
		    if(fabs(orbMetadata[j].orbitDescendTime - 
			    laggedDescendTime) < ADTIME_TOLERANCE)
		    /*orbMetadata[(*numOrbits)-1].orbitDescendTime) )*/
		      {
			returnStatus = PGSEPH_W_DBLVALUE_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nSmall difference in Descending node times in different data files"
				" for the same orbit, namely # %d; Keeping the one from second file"
				,orbMetadata[j].orbitNumber);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
			/* For the time being replace warning return 
			   with success */
			returnStatus = PGS_S_SUCCESS;
		      }
		    else
		      {
			returnStatus = PGSEPH_W_CORRUPT_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nDifferent Descending node times in different data files"
				" for same orbit, namely # %d; Keeping the one from second file"
				,orbMetadata[j].orbitNumber);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
		      }
		  }
		/* continue */
		replace_flag = PGS_TRUE;  /* we have found same orbit in 
					     two different files*/
	      } /* end comparison of same orbit in different files */
	    
	    /* Now test for correct order of AscendTimes.
	       Assume an orbit takes at least 5000 s; minimum period
	       for an Earth-grazing satellite is about 5080 s . The
	       current orbit number is j and the most recent previous
	       one is ((*numOrbits) - 1). */
	    if(replace_flag == PGS_TRUE)
	      {
		if(j > 1)
		  {
		    laggedAscendTime1 = orbMetadata[j-2].orbitAscendTime;
		    laggedOrbitNumber1 = orbMetadata[j-2].orbitNumber;
		    laggedDescendTime1 = orbMetadata[j-2].orbitDescendTime;
		  }
		else
		  {
		    /* If this is 1st or 2nd number, no need to go into
		       next if statement */
		    laggedAscendTime1 = orbMetadata[j].orbitAscendTime-5000;
		    laggedOrbitNumber1 = orbMetadata[j].orbitNumber-1;
		    laggedDescendTime1 = orbMetadata[j].orbitDescendTime-5000;
		  }
	      }
	    else
	      {
		laggedAscendTime1 = laggedAscendTime;
		laggedOrbitNumber1 = laggedOrbitNumber;
		laggedDescendTime1 = laggedDescendTime;
	      }
	    
	    if( (orbMetadata[j].orbitAscendTime < 
		 laggedAscendTime1 + 5000.0))
	      { 
		returnStatus = PGSEPH_W_CORRUPT_METADATA;
		if(orbMetadata[j].orbitNumber == laggedOrbitNumber1)
		  {
		    sprintf(specifics,
			    "\nInconsistent orbit number or Ascending node time at orbit # %d"
			    ,orbMetadata[j].orbitNumber);
		  }
		else
		  {
		    sprintf(specifics,
			    "\nAscending node times out of order or too close"
			    " for orbits # %d and %d"
			    ,orbMetadata[j].orbitNumber,laggedOrbitNumber1);
		  }
		PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
		strcat(errorBuf,specifics);
		PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
	      } /* end if( (orbMetadata[j].orbitAscendTime <  ... */
	    if ( orbMetadata[j].orbitNumber < laggedOrbitNumber1)
	      {
		returnStatus = PGSEPH_W_CORRUPT_METADATA;
		PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
		sprintf(specifics,"\norbit number found to be decreasing in time");
		strcat(errorBuf,specifics);
		PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
	      } /* end if(orbMetadata[j].orbitNumber < laggedOrbitNumber1 */
	    
	  } /* end if ( *numOrbits > 0 ) */
	
	/* we are in new data now; 
	   record the orbit number 
	   and then the other metadata.  */

	/* if current orbit number is the same as the previous one, replace 
	   the metadata values for the orbit with those from the second file */
	if(replace_flag == PGS_TRUE)
	  {
	    *numOrbits = *numOrbits - 1;
	  }

	orbitNumber[*numOrbits] = orbMetadata[j].orbitNumber;
	
	/* convert orbit start time to ASCII string and record it */
	
	returnStatus1 = PGS_TD_TAItoUTC(orbMetadata[j].orbitAscendTime,
					orbitAscendTime[*numOrbits]);
	switch (returnStatus1)
	  {
	  case PGS_S_SUCCESS:
	  case PGSTD_E_NO_LEAP_SECS:
	    foundSome = PGS_TRUE;
            break;
	    
	  case PGS_E_TOOLKIT:
            if (orbMetadata != NULL)
	      {
                free(orbMetadata);
	      }
            return returnStatus1;
	    
          default:
            PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
            if (orbMetadata != NULL)
	      {
                free(orbMetadata);
	      }
            return PGS_E_TOOLKIT;
	  } /* end switch */
	
        /* Perform another check before accepting the downcrossing time. The
           descend time should be about 1/2 orbit after the ascend or start
           time. This is expected to be between 2500 and 45000 s - the latter
           would  enable working with half-geosynchronous spacecraft, which
           is pretty liberal.  Geostationary spacecraft will not have useful
           up-  and down- crossings anyway.  The main point here is to detect
           for example accidental zero values (1993-01-01 in Toolkit TAI. 
           Nevertheless, we include testing that the Descend time exceeds the
           Ascend time; the latter is start of orbit so the Descend time
           needs to me about 1/2 orbit later. */
	
        if( ((orbMetadata[j].orbitDescendTime - 
	      orbMetadata[j].orbitAscendTime) < 2400.0) || 
            ((orbMetadata[j].orbitDescendTime - 
	      orbMetadata[j].orbitAscendTime) > 45000.0) || 
            ( (*numOrbits > 1) && 
	      (((orbMetadata[j].orbitAscendTime - 
		 laggedDescendTime1) < 2400.0 ) || 
	       ((orbMetadata[j].orbitAscendTime - 
		 laggedDescendTime1) > 45000.0 ))))
	  {
	    returnStatus = PGSEPH_W_CORRUPT_METADATA;
	    sprintf(specifics,
		    "\nInconsistent Descending vs Ascending node times at orbit # %d"
		    ,orbMetadata[j].orbitNumber);
	    PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
	    strcat(errorBuf,specifics);
            PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
	  }
	
	/* certain checks need to be imposed from orbit to orbit so save info.
	   This is needed for the times because they are put in the OUTPUTs
	   only in ASCII form; we may as well save the orbit number in a
	   similar way  */
	
	laggedOrbitNumber =  orbMetadata[j].orbitNumber; /* (just for parallel
							    notation) */
	laggedAscendTime = orbMetadata[j].orbitAscendTime; 
	laggedDescendTime = orbMetadata[j].orbitDescendTime;
	

        /* convert equator downcrossing time to ASCII string and record it */
        /* This is approximately mid-orbit and is used as a reference time
           for the downcrossing longitude  */
	
	returnStatus1 = PGS_TD_TAItoUTC(orbMetadata[j].orbitDescendTime,
					orbitDescendTime[*numOrbits]);
	switch (returnStatus1)
	  {
	 case PGS_S_SUCCESS:
	 case PGSTD_E_NO_LEAP_SECS:
	   break;
	   
	 case PGS_E_TOOLKIT:
	   if (orbMetadata != NULL)
	     {
	       free(orbMetadata);
	     }
	   return returnStatus1;
	   
	 default:
	   PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
	   if (orbMetadata != NULL)
	     {
	       free(orbMetadata);
	     }
	   return PGS_E_TOOLKIT;
	 }
       
       /* record longitude of downward (i.e. south-bound) equator
	  crossing for this orbit */
       
       orbitDownLongitude[*numOrbits] = orbMetadata[j].orbitDescendLongitude;
       laggedorbitDescendLongitude = orbMetadata[j].orbitDescendLongitude;
       /* increment the number of relevant orbits found */
       
       *numOrbits = (*numOrbits) + 1;
      } /* end for (j=0;j<totalOrbits;j++)  */
    
    if (orbMetadata != NULL)
      {
        free(orbMetadata);
      }
    if ((foundSome == PGS_TRUE) && (*numOrbits >0) && 
	((returnStatus != PGSEPH_W_CORRUPT_METADATA) && 
	 (returnStatus != PGSEPH_W_DBLVALUE_METADATA)))
      {
        return PGS_S_SUCCESS;
      }
    else if ((foundSome == PGS_TRUE) && (*numOrbits >0) && 
	     (returnStatus == PGSEPH_W_DBLVALUE_METADATA))
      {
	returnStatus = PGSEPH_W_DBLVALUE_METADATA;
        return returnStatus;
      }
    else if (foundSome !=  PGS_TRUE || (*numOrbits ==0) )
      {
        returnStatus = PGSEPH_W_CORRUPT_METADATA;
        PGS_SMF_SetDynamicMsg(returnStatus,
                              "no meaningful orbital metadata found",
                              FUNCTION_NAME);
        return returnStatus;
      }
    else
      {
	returnStatus = PGSEPH_W_CORRUPT_METADATA;
	PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME);
	return returnStatus;
      }
}


/******************************************************************************
BEGIN_PROLOG:

NAME:
  PGS_EPH_GetEphMetAux.c

DESCRIPTION:
  This function is a wrapper around the function PGS_EPH_GetEphMet()
  This function (which has one more argument than  PGS_EPH_GetEphMet), is used
  to make the FORTRAN binding work properly, where the size of 
  orbitAscendTime, and orbitDescendTime arrays are required in the 
  FORTRAN binder, but not present in the argument list of PGS_EPH_GetEphMet.
  The size of orbitAscendTime, and orbitDescendTime arrays passed to this 
  function will be maxorbitarraysize.
  See PGS_EPH_GetEphMetF.f for more information

AUTHOR:
  Abe Taaheri / Space Applications Corporation

HISTORY:
  29-SEPT-1998  AT  Initial version

END_PROLOG:
******************************************************************************/

PGSt_SMF_status
PGS_EPH_GetEphMetAux(                        /* get metadata associated with a
                                                toolkit ephemeris file */
    PGSt_tag         spacecraftTag,          /* spacecraft ID tag */
    PGSt_integer     numValues,              /* number of values requested */
    PGSt_integer     maxorbitarraysize,      /* number of elements in
                                                orbitAscendTime and
                                                orbitDescendTime for temporary
                                                arrays defined in the FORTRAN
                                                function pgs_eph_getephmet2 */
    char             asciiUTC[28],           /* reference start time */
    PGSt_double      offsets[],              /* time offsets relative to
                                                asciiUTC */
    PGSt_integer*    numOrbits,              /* number of orbits spanned by
                                                data set */
    PGSt_integer     orbitNumber[],          /* array of orbit numbers */
    char             orbitAscendTime[][28],  /* array of ascend times */
    char             orbitDescendTime[][28], /* array of descend times */
    PGSt_double      orbitDownLongitude[])   /* array of s/c downward equator
                                                crossing longitudes */
{

    PGSt_SMF_status    returnStatus;          /* status of TK func. calls */
    
    returnStatus =  PGS_EPH_GetEphMet(spacecraftTag, numValues, asciiUTC,
                                      offsets, numOrbits, orbitNumber,
                                      orbitAscendTime, orbitDescendTime,
                                      orbitDownLongitude);
    return returnStatus;
    
}


/*
 * function: file_split_path()
 * author:   mike sucher
 * purpose:  split path prefix from file name
 * arguments:
 *     char *name;  input:  full name of file including path
 *     char *path;  output: the path, split from the filename
 *     char *fname; output: the filename, split from the path
 * returns:
 *     (1) Success
 * notes:
 *     the path separator is the slash ('/') character, thus this
 *     routine is only intended for Unix filesystems
 */
static int file_split_path(char *name, char *path, char *fname )
{
    char c, *p1, *p2, *q;

    p1 = p2 = name;

    while(1)
    {
        for(q=p2; (*q != '/') && *q; q++);
        if(*q) p2 = q+1;
        else break;
    }

    strcpy(fname, p2);

    if(p2 > p1)
    {
        c = *(p2-1);
        *(p2-1) = 0;
        strcpy(path, p1);
        *(p2-1) = c;
    }
    else path[0] = 0;

    return 1;
}

void Xget_scTag(char *temp_sc_string, PGSt_tag *temp_spacecraftTag)
{

     if(!strcmp(temp_sc_string,"PGSd_EOS_AM"))
        *temp_spacecraftTag=PGSd_EOS_AM;
     else if(!strcmp(temp_sc_string,"EOSAM1"))
        *temp_spacecraftTag=PGSd_EOS_AM;
     else if(!strcmp(temp_sc_string,"EOSAURA"))
        *temp_spacecraftTag=PGSd_EOS_AURA;
     else if(!strcmp(temp_sc_string,"PGSd_EOS_PM_GIIS"))
         *temp_spacecraftTag=PGSd_EOS_PM_GIIS;
     else if(!strcmp(temp_sc_string,"EOSPM1"))
         *temp_spacecraftTag=PGSd_EOS_PM_GIIS;
     else if(!strcmp(temp_sc_string,"PGSd_EOS_PM_GIRD"))
         *temp_spacecraftTag=PGSd_EOS_PM_GIRD;
     else if(!strcmp(temp_sc_string,"PGSd_TRMM"))
        *temp_spacecraftTag=PGSd_TRMM;
     else if(!strcmp(temp_sc_string,"TRMM"))
        *temp_spacecraftTag=PGSd_TRMM;

     else
        *temp_spacecraftTag=atoi(temp_sc_string);
}

/*
 * function: file_exists()
 * author:   mike sucher
 * purpose:  check to see if file exists
 * arguments:
 *     char *name; name, including path, of the file to check
 * returns:
 *     FOUND     (1) file does exist
 *     NOT_FOUND (0) file does not exist
 */

#define FOUND 1
#define NOT_FOUND 0

static int file_exists(char *name)
{
    DIR *dirp;
    struct dirent *dp;
    char fname[128], path[128];

    file_split_path(name,path,fname);
    if(strlen(path) != 0) dirp = opendir(path);
    else dirp = opendir( "." );
    if (dirp == NULL)
      return NOT_FOUND;
    while ( (dp = readdir( dirp )) != NULL )
    if( strcmp( dp->d_name, fname ) == 0 )
    {
        closedir(dirp);
        return FOUND;
    }
    closedir(dirp);
    return NOT_FOUND;
}

/* To get ephemeris data from HDF files */
static PGSt_SMF_status 
PGS_EPH_checkHDF2(PGSt_tag spacecraftTag, PGSt_double startTAI93,
		  PGSt_double stopTAI93, PGSt_integer* XnumOrbits, 
		  PGSt_integer XhdforbitNumber[MAX_NUM_ORBS],
		  char XhdfAscendTime[MAX_NUM_ORBS][28],
		  char XhdfDescendTime[MAX_NUM_ORBS][28], 
		  PGSt_double XhdforbitLongitude[MAX_NUM_ORBS])
{
  const int DpCPrMaxScIdLen          = 24;
  const int DpCPrMaxTimeRangeLen     = 48;
  const int DpCPrMaxSourceLen        = 32;
  const int DpCPrMaxVersionLen       = 8;
  const int DpCPrMaxFrameLen         = 8;
  const int DpCPrMaxKepler           = 6;
  const int DpCPrMaxQaParameters     = 16;
  const int DpCPrMaxQaStatistics     = 4;
  const int DpCPrMaxUrLen            = 256;
  /*  const int DpCPrMaxUrs              = 10;*/
  
  char     hdfFile[256]; 
  PGSt_SMF_status pgsStatus;  
  char     fields[512];
  char     vDataName[64];
  unsigned char *hdfData;
  unsigned char *pntr;
  char     hdfScId[24];
  char     hdfTimeRange[48];
  char     hdfSource[32];
  char     hdfVersion[8];
  float64  hdfStartTai;
  float64  hdfEndTai;
  float32  hdfInterval;
  int32    hdfNUrs;
  int32    hdfNRecords;
  int32    hdfNOrbits;
  int32    hdfOrbitStart;
  int32    hdfOrbitEnd;
  char     hdfRefFrame[8];
  float64  keplerElements[6];
  float64  keplerEpochTai;
  float32  hdfQaParams[16];
  float32  hdfQaStats[4];
  float64  hdfPeriod;
  float64  hdfDescProp;
  int32    hdfFddReplace;
  char     hdfURs[10][256];
  int      iElm;

  int32    hdfId = -1;
  int32    vDataRef;
  int32    vDataId = -1;
  int32    hdrNRecs;
  int32    interlace;
  int32    vDataSize;
  int32    ephNRecs;
  PGSt_integer     recordCount = 0;
  /*PGSt_ephemRecord hdfRecord;*/
  int32            orbNRecs;
  PGSt_integer     hdforbitNumber[MAX_NUM_ORBS];
  PGSt_double      hdforbitAscend[MAX_NUM_ORBS];
  PGSt_double      hdforbitDescend[MAX_NUM_ORBS];
  PGSt_double      hdforbitLongitude[MAX_NUM_ORBS];
  /*
  char   	   mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];
  char   	   pgsMessage[PGS_SMF_MAX_MSG_SIZE];
  */
  PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
  PGSt_SMF_status  returnStatus1;          /* status of TK func. calls */
  PGSt_tag         hdfspacecraftTag;
  PGSt_integer     num_files, num;
  PGSt_integer     file_num=1;
  PGSt_integer     counter;
  PGSt_boolean     gotData = PGS_FALSE;
  PGSt_integer     j=0;
  PGSt_integer     sizeint, sizechar, sizeflt, sizedbl;
  PGSt_integer     totalOrbits;
  PGSt_integer     nfiles_read=0;
  /*PGSt_boolean     foundSome = PGS_FALSE;*/
  char             specifics[PGS_SMF_MAX_MSG_SIZE];   /* message */
  char             errorBuf[PGS_SMF_MAX_MSGBUF_SIZE]; /* appended error msg */
  PGSt_double       laggedorbitDescendLongitude;
  PGSt_integer      laggedOrbitNumber=0;   /* lagged orbit number for tests*/
  PGSt_double       laggedAscendTime;      /* lagged Ascend time for tests*/
  PGSt_double       laggedDescendTime;     /* lagged Descend time for tests. 
					      orbitNumber[*numOrbits] right
					      after an orbit number is 
					      "promoted" from the list of 
					      those that are found in one 
					      of the data files to one that
					      is recorded. */
  PGSt_double       laggedAscendTime1;
  PGSt_double       laggedDescendTime1;
  PGSt_integer      laggedOrbitNumber1=0;

  PGSt_boolean      replace_flag = PGS_FALSE;



  sizeflt = sizeof(float32);
  sizedbl = sizeof(float64);
  sizechar = sizeof(char);
  sizeint = sizeof(int32);
  totalOrbits = 0;


  PGS_PC_GetNumberOfFiles(PGSd_SC_EPHHDF_DATA, &num_files);
  for ( num = 1; num <= num_files; num++)
    {
      file_num = num;
      pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA, &file_num,hdfFile );
      
      if (pgsStatus != PGS_S_SUCCESS)
	{
	  pgsStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	  PGS_SMF_SetDynamicMsg(pgsStatus,
				"error in accessing spacecraft ephemeris HDF file(s)",
				FUNCTION_NAME2);
	  return pgsStatus;
	}
      
      if (file_exists(hdfFile))
	{
	  /* Open the HDF fomat ephemeris file */
	  hdfId = Hopen( hdfFile, DFACC_RDONLY, 0 );
	  /* Initialize the Vset interface */
	  Vstart( hdfId );
	  /* Get the reference number of the first Vdata in the file */
	  vDataRef = -1;
	  vDataRef = VSgetid( hdfId, vDataRef );
	  /* Attach to the first Vdata in read mode */
	  vDataId = VSattach( hdfId, vDataRef, "r" );
	  /* Get the list of field names */
	  VSinquire( vDataId, (int32 *)&hdrNRecs, (int32 *)&interlace,
		     fields, (int32 *)&vDataSize, vDataName );
	  /* Name the fields to be read */
	  (void) VSsetfields( vDataId, fields );
	  hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader) +
					     100*DpCPrMaxUrLen * sizechar +
					     2*sizedbl+sizeint);
	  /* Read a packed Vdata record */
	  (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
	  /* Unpack the packed Vdata record */
	  pntr = hdfData;
	  memmove( &hdfScId, pntr, DpCPrMaxScIdLen*sizechar );
	  pntr += DpCPrMaxScIdLen*sizechar;
	  memmove( &hdfTimeRange, pntr, DpCPrMaxTimeRangeLen*sizechar );
	  pntr += DpCPrMaxTimeRangeLen*sizechar;
	  memmove( &hdfSource, pntr, DpCPrMaxSourceLen*sizechar );
	  pntr += DpCPrMaxSourceLen*sizechar;
	  memmove( &hdfVersion, pntr, DpCPrMaxVersionLen*sizechar );
	  pntr += DpCPrMaxVersionLen*sizechar;
	  memmove( &hdfStartTai, pntr, sizedbl );
	  pntr += sizedbl;
	  memmove( &hdfEndTai, pntr, sizedbl );
	  pntr += sizedbl;
	  memmove( &hdfInterval, pntr, sizeflt );
	  pntr += sizeflt;
	  memmove( &hdfNUrs, pntr, sizeint );
	  pntr += sizeint;
	  memmove( &hdfNRecords, pntr, sizeint );
	  pntr += sizeint;
	  memmove( &hdfNOrbits, pntr, sizeint );
	  pntr += sizeint;
	  memmove( &hdfOrbitStart, pntr, sizeint );
	  pntr += sizeint;
	  memmove( &hdfOrbitEnd, pntr, sizeint );
	  pntr += sizeint;
	  memmove( &hdfRefFrame, pntr, DpCPrMaxFrameLen*sizechar );
	  pntr += DpCPrMaxFrameLen*sizechar;
	  memmove( &keplerElements[0], pntr, DpCPrMaxKepler*sizedbl );
	  pntr += DpCPrMaxKepler*sizedbl;
	  memmove( &keplerEpochTai, pntr, sizedbl );
	  pntr += sizedbl;
	  memmove( &hdfQaParams[0], pntr, DpCPrMaxQaParameters*sizeflt);
	  pntr += DpCPrMaxQaParameters*sizeflt;
	  memmove( &hdfQaStats[0], pntr, DpCPrMaxQaStatistics*sizeflt);
	  pntr += DpCPrMaxQaStatistics*sizeflt;
	  memmove( &hdfPeriod, pntr, sizedbl );
	  pntr += sizedbl;
	  memmove( &hdfDescProp, pntr, sizedbl );
	  pntr += sizedbl;
	  memmove( &hdfFddReplace, pntr, sizeint );
	  pntr += sizeint;
	  counter = (hdfNUrs) ? hdfNUrs : 1; 
	  for (iElm=0; iElm<counter; iElm++)
	    {
	      memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
	      pntr += DpCPrMaxUrLen*sizechar;
	    }
	  /* Detach from the Vdata */
	  VSdetach( vDataId );
	  vDataId = -1;
	  free(hdfData);
	  Xget_scTag(hdfScId, &hdfspacecraftTag);

	  /* get orbit info from all files that include the 
	     requested time range */
	  if( (hdfspacecraftTag == spacecraftTag &&
	       startTAI93 >= hdfStartTai  && startTAI93 < hdfEndTai) ||
	      (hdfspacecraftTag == spacecraftTag &&
	       stopTAI93 >= hdfStartTai  && stopTAI93 < hdfEndTai) ||
	      (hdfspacecraftTag == spacecraftTag &&
	       startTAI93 <= hdfStartTai  && stopTAI93 >= hdfEndTai))
	    {
	      nfiles_read = nfiles_read + 1;
	      /* Get the reference number of the ephemeris Vdata */
	      vDataRef = VSgetid( hdfId, vDataRef );
	      vDataId = VSattach( hdfId, vDataRef, "r" );
	      /* Get the list of field names */
	      VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
			 fields, (int32 *)&vDataSize, vDataName );
	      /* Name the fields to be read */
	      (void) VSsetfields( vDataId, fields );
	      /* Detach from the Vdata */
	      VSdetach( vDataId );
	      vDataId = -1;
	      /* Get the reference number of the metadata Vdata */
	      vDataRef = VSgetid( hdfId, vDataRef );
	      /* Attach to the first Vdata in read mode */
	      vDataId = VSattach( hdfId, vDataRef, "r" );
	      /* Get the list of field names */
	      VSinquire( vDataId, (int32 *)&orbNRecs, (int32 *)&interlace,
			 fields, (int32 *)&vDataSize, vDataName );
	      /* Name the fields to be read */
	      (void) VSsetfields( vDataId, fields );
	      
	      hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemMetadata));
	      /* Read a packed Vdata record */
	      (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
	      
	      for ( recordCount= 0; recordCount<orbNRecs; recordCount++) 
		{
		  /* only maximum MAX_NUM_ORBS orbits can be extracted */
		  if((totalOrbits ) > (MAX_NUM_ORBS - 1))
		    {
		      break;
		    }
		  
		  /* Unpack the packed Vdata record */
		  pntr = hdfData;
		  
		  memmove( &hdforbitNumber[totalOrbits],
			   pntr, sizeint );
		  pntr += sizeint;
		  memmove( &hdforbitAscend[totalOrbits],
			   pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdforbitDescend[totalOrbits],
			   pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdforbitLongitude[totalOrbits],
			   pntr, sizedbl );
		  
		  /* Read a packed Vdata record */
		  (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
		  totalOrbits = totalOrbits + 1;
		  
		}
	      
	      hdforbitAscend[totalOrbits] = 
		hdforbitAscend[totalOrbits-1] + PGSd_GEO_ERROR_VALUE;
	      hdforbitNumber[totalOrbits] =  
		hdforbitNumber[totalOrbits-1] + MAX_NUM_ORBS;
	      /* Detach from the Vdata */
	      VSdetach( vDataId );
	      vDataId = -1;
	      free(hdfData);
	      if (gotData == PGS_TRUE)
		{
		  returnStatus = PGS_S_SUCCESS;
		  /* Close the Vset interface */
		  Vend( hdfId );
		  /* Close the HDF format ephemeris file */
		  Hclose( hdfId );
		  hdfId = -1;
		  break;
		}
	      
	    } /* end check spacrcrafttag */
	  else
	    {
	      if(num == num_files )
		{
		  if(nfiles_read == 0) /* no appropriate files were found */
		    {
		      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		      sprintf(specifics,"%s"," Error; Cannot find any ephemeris HDF files for the spacecraft"); 
		      PGS_SMF_SetDynamicMsg(returnStatus, specifics, 
					    FUNCTION_NAME2);
		      /* Close the Vset interface */
		      Vend( hdfId );
		      /* Close the HDF format ephemeris file */
		      Hclose( hdfId );
		      hdfId = -1;
		      break;
		    }
		  else
		    {
		      returnStatus = PGS_S_SUCCESS;
		      /* Close the Vset interface */
		      Vend( hdfId );
		      /* Close the HDF format ephemeris file */
		      Hclose( hdfId );
		      hdfId = -1;
		      break;
		    }
		}
	      else
		{
		  continue;
		}
	      /*	      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;*/
	    }
	  /* Close the Vset interface */
	  Vend( hdfId );
	  /* Close the HDF format ephemeris file */
	  Hclose( hdfId );
	  hdfId = -1;
	}
      else
	{
	  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	  sprintf(specifics,"%s%s %s"," Error; Spacecraft ephemeris HDF file ", 
		  hdfFile,"does not exist");
	  PGS_SMF_SetDynamicMsg(returnStatus, specifics,FUNCTION_NAME2);

	  break;
	}
      if(vDataId != -1)
	{
	  VSdetach( vDataId );
	  vDataId = -1;
	}
      if(hdfId != -1)
	{
	  /* Close the Vset interface */
	  Vend( hdfId );
	  /* Close the HDF format ephemeris file */
	  Hclose( hdfId );
	  hdfId = -1;
	}

    } /* end for ( num = 1; .... )*/
  
  
  /* Sort orbits read and select only those needed */
  if(returnStatus != PGS_S_SUCCESS)
    {
      return returnStatus;
    }
  else
    {
      /* first sort orbits according to their orbit numbers */

      PGS_EPH_ShellSort(totalOrbits, hdforbitNumber,
			hdforbitAscend, hdforbitDescend,
			hdforbitLongitude);

      /*now select orbits that are needed */

      for (j=0;j<totalOrbits;j++)
	{
	  /* these metadata are only relevant if any part of this orbit is
	     spanned by the data set */
	  
	  if (startTAI93 >= hdforbitAscend[j+1])
	    {                            /* we do not want to miss first 
					 orbit metadata that is taken
					 from two consecutive eph files
					 that have parts of the same orbit */
	      
	      if(hdforbitNumber[j] != hdforbitNumber[j+1])
		{
		  continue;
		}
	      else if(hdforbitNumber[j] == hdforbitNumber[j+1])
		{
		  if(*XnumOrbits == 0) 
		    {
		      if(startTAI93 >= hdforbitAscend[j+2])
			{
			  continue;
			}
		    }
		}
	    }
	  
	  if (stopTAI93 < hdforbitAscend[j])
	    {
	      /*foundSome = PGS_TRUE;*/
	      break;
	    }
	  
	  /* if this is not the first pass through (i.e. we have already 
	     recorded information about at least one orbit) check the 
	     current orbit number against the last orbit number recorded, 
	     don't record duplicate orbit numbers (this can happen when 
	     the data for an orbit is found in two separate physical 
	     files since each file will contain the metadata for that 
	     particular orbit and this metadata will obviously be the 
	     same in both cases) */
	  
	  if ( *XnumOrbits > 0 )
	    {
	      replace_flag = PGS_FALSE; /* flag that is set to TRUE if we find
					   two orbits with the same orbit 
					   number */

	      if ( hdforbitNumber[j] ==
		   XhdforbitNumber[(*XnumOrbits) - 1] )
		{
		  /* first check that the other metadata are consistent 
		     for this orbit number.  This checks for the (unlikely?)
		     event that somehow the orbit number failed to increment
		     yet the spacecraft is on a new orbit, or that two files
		     with the same orbit contain inconsistent accompanying 
		     metadata. If these checks are passed, go on to the 
		     next value of j */
		  		/*Note: There are situations where the orbit metadata for 
		  the same orbit (orbit metadata which are associated with
		  different granules) can be different.  This occurs when 
		  an orbit completes in the granule following the one 
		  currently being processed by DPREP, but the following 
		  granule is not available to DPREP processing.  The granules 
		  that DPREP needs to do the best possible job with the orbit
		  metadata aren't always available in forward processing.  
		  With Terra, for example, DPREP waits 4 hours for the 
		  following granule to become available so that the orbit 
		  metadata can be properly generated.  If after 4 hours 
		  the granule is not available, however, DPREP proceeds 
		  without it, and extrpolates metadata for the incomplete 
		  orbit.  When the following granule is processed, DPREP 
		  produces orbit metadata using the actual node crossing that 
		  is now available.  The extrapolated orbit metadata 
		  from the preceeding granule then differs from the "actual" 
		  orbit metadata from the following granule.  DPREP don't go 
		  back and correct the orbit metadata in the preceeding 
		  granule, for operational reasons, unless the data are 
		  reprocessed later during one of the reprocessing efforts. 
		  A similar situation exists with Aqua/Aura ephemeris 
		  processing, with the exception that DPREP does not look 
		  forward to the following granule at all; reprocessing would 
		  not correct orbit metadata inconsistencies in Aqua/Aura 
		  metadata.  This latter situation can be changed in DPREP 
		  so that at least a reprocessing effort corrects the 
		  orbit metadata.  The problem definitely originates with 
		  DPREP, but it's not clear to me that the solution 
		  necessarily resides in DPREP because of  operational 
		  restrictions.  Production rules could be changed for 
		  Terra DPREP so that DPREP waits until the following 
		  granule is available before processing; this slows data 
		  production however.
		  This certainly isn't the solution either for Aqua/Aura 
		  DPREP because of waiting 24+ hours for the following 
		  granule to become available. 

		  The problem just mentioned fails MODIS PGE03. To avoid 
		  this we will modify this code so that: 
		  (1) return the WARNING when the metadata differ by some 
		  threshold, and 
		  (2) return PGS_S_SUCCESS when within-threshold 
		  inconsistencies are encountered, and provide the latter 
		  (corrected) orbit metadata values to the user. This will be
		  in place until MODIS PGE03 fixes are in place (corrections 
		  to managing return value from PGS_EPH_GetEphMet() in the 
		  file GEO_write_granule_metadata.c, that does not consider
		  warnings as errors).
		*/
		/* Per DPREP's Robert Kummerer's suggestion a crossing-time 
		   threshold (dTime) = 60 seconds will be used. The 
		   crossing-longitude threshold (dLong) derives
		   from the crossing-time threshold as follows: 

		   dLong = (dTime / 86400) * 2 * pi = 0.004363   radians 
		   (86400 is the number of seconds in a day) 

		   We need to be careful when verifying that the 
		   OrbitDescendingLongitude are within threshold. 
		   OrbitDescendingLongitude ranges from -pi to +pi.  
		   We need to be cautious of the fact that the 2
		   OrbitDescendingLongitude We are comparing may 
		   cross the -pi / +pi boundary (a slim chance of 
		   this happening but nonetheless it could happen). 

		 */
		  if( ( hdforbitAscend[j] !=
			laggedAscendTime ) )
		    {
		    if(fabs(hdforbitAscend[j] -
			    laggedAscendTime) < ADTIME_TOLERANCE)
		      {
			returnStatus = PGSEPH_W_DBLVALUE_METADATA;
			PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			sprintf(specifics,
				"\nSmall difference in Ascending node times in different data files"
				" for the same orbit, namely # %d; Keeping the one from second file"
				,hdforbitNumber[j]);
			strcat(errorBuf,specifics);
			PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
			/* For the time being replace warning return 
			   with success */
			/*returnStatus = PGS_S_SUCCESS;*//*let we return 
							   warning as 
							   intended */ 
		      }
		    else
		      {
		      returnStatus = PGSEPH_W_CORRUPT_METADATA;
		      
		      PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
		      sprintf(specifics,
			      "\nDifferent Ascending node times in different data files"
			      " for same orbit, namely # %d; Keeping the one from second file"
			      , hdforbitNumber[j]);
		      strcat(errorBuf,specifics);
		      PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
		      }
		    }
		  if( ( hdforbitLongitude[j] !=
			laggedorbitDescendLongitude ) )
		    {
		      if((fabs(hdforbitLongitude[j] -
			     laggedorbitDescendLongitude)<DLONGI_TOLERANCE) ||
			 (fabs(2.0*THEPI - fabs(hdforbitLongitude[j] -
						laggedorbitDescendLongitude)) <
			  DLONGI_TOLERANCE))
			{
			  returnStatus = PGSEPH_W_DBLVALUE_METADATA;
			  PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			  sprintf(specifics,
				  "\nSmall difference in Descending longitudes in different data files"
				  " for the same orbit, namely # %d; Keeping the one from second file"
				  , hdforbitNumber[j]);
			  strcat(errorBuf,specifics);
			  PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
			  /* For the time being replace warning return 
			     with success */
			  /*returnStatus = PGS_S_SUCCESS;*//*let we return 
							     warning as 
							     intended */ 
			}
		      else
			{
			  returnStatus = PGSEPH_W_CORRUPT_METADATA;
			  PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			  sprintf(specifics,
				  "\nDifferent Descending longitudes in different data files"
				  " for same orbit, namely # %d"
				  , hdforbitNumber[j]);
			  strcat(errorBuf,specifics);
			  PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
			}
		    }
		  if( ( hdforbitDescend[j] !=
			laggedDescendTime ) )	
		    {
		      if(fabs(hdforbitDescend[j] - 
			      laggedDescendTime) < ADTIME_TOLERANCE)
			{
			  returnStatus = PGSEPH_W_DBLVALUE_METADATA;
			  PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			  sprintf(specifics,
				  "\nSmall difference in Descending node times in different data files"
				  " for the same orbit, namely # %d; Keeping the one from second file"
				  , hdforbitNumber[j]);
			  strcat(errorBuf,specifics);
			  PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
			  /* For the time being replace warning return 
			     with success */
			  /*returnStatus = PGS_S_SUCCESS;*//*let we return 
							     warning as 
							     intended */ 
			}
		      else
			{
			  returnStatus = PGSEPH_W_CORRUPT_METADATA;
			  PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
			  sprintf(specifics,
				  "\nDifferent Descending node times in different data files"
				  " for same orbit, namely # %d"
				  , hdforbitNumber[j]);
			  strcat(errorBuf,specifics);
			  PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
			}
		    }
		  /*continue;*/
		  replace_flag = PGS_TRUE;  /* we have found same orbit in
					       two different files */
		} /* end comparison of same orbit in different files */
	      
	      /* Now test for correct order of AscendTimes.
		 Assume an orbit takes at least 5000 s; minimum period
		 for an Earth-grazing satellite is about 5080 s . The
		 current orbit number is j and the most recent previous
		 one is ((*numOrbits) - 1). */
	      if(replace_flag == PGS_TRUE)
		{
		  if(j > 1)
		    {
		      laggedAscendTime1 = hdforbitAscend[j-2];
		      laggedOrbitNumber1 = hdforbitNumber[j-2];
		      laggedDescendTime1 = hdforbitDescend[j-2];
		    }
		  else
		    {
		      /* If this is 1st or 2nd number, no need to go into
			 next if statement */
		      laggedAscendTime1 = hdforbitAscend[j]-5000;
		      laggedOrbitNumber1 = hdforbitNumber[j]-1;
		      laggedDescendTime1 = hdforbitDescend[j]-5000;
		    }
		}
	      else
		{
		  laggedAscendTime1 = laggedAscendTime;
		  laggedOrbitNumber1 = laggedOrbitNumber;
		  laggedDescendTime1 = laggedDescendTime;
		}


	      if( (hdforbitAscend[j] < 
		   laggedAscendTime1 + 5000.0))
		{ 
		  returnStatus = PGSEPH_W_CORRUPT_METADATA;
		  if((hdforbitNumber[j] == laggedOrbitNumber1))
		    {
		      sprintf(specifics,
			      "\nInconsistent orbit number or Ascending node time at orbit # %d"
			      ,hdforbitNumber[j]);
		    }
		  else
		    {
		      sprintf(specifics,
			      "\nAscending node times out of order or too close"
			      " for orbits # %d and %d"
			      ,hdforbitNumber[j],laggedOrbitNumber1);
		    }
		  PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
		  strcat(errorBuf,specifics);
		  PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
		} /* end if( (hdforbitAscendTime[j] <  ... */
	      if ( hdforbitNumber[j] < laggedOrbitNumber1)
		{
		  returnStatus = PGSEPH_W_CORRUPT_METADATA;
		  PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
		  sprintf(specifics,"\norbit number found to be decreasing in time");
		  strcat(errorBuf,specifics);
		  PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
		} /* end if(hdforbitNumber[j] < laggedOrbitNumber1 */
	      
	    } /* end if ( *XnumOrbits > 0 ) */
	  
	  /* we are in new data now; 
	     record the orbit number 
	     and then the other metadata.  */
	  
	if(replace_flag == PGS_TRUE)
	  {
	    *XnumOrbits = *XnumOrbits - 1;
	  }

	  XhdforbitNumber[*XnumOrbits] = hdforbitNumber[j];
	  
	  /* c1onvert orbit start time to ASCII string and record it */
	  
	  returnStatus1 = PGS_TD_TAItoUTC(hdforbitAscend[j],
					  XhdfAscendTime[*XnumOrbits]);
	  switch (returnStatus1)
	    {
	    case PGS_S_SUCCESS:
	    case PGSTD_E_NO_LEAP_SECS:
	      /*foundSome = PGS_TRUE;*/
	      break;
	      
	    case PGS_E_TOOLKIT:
	      return returnStatus1;
	      
	    default:
	      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
	      return PGS_E_TOOLKIT;
	    } /* end switch */
	  
	  /* Perform another check before accepting the downcrossing time.
	     The  descend time should be about 1/2 orbit after the ascend 
	     or start time. This is expected to be between 2500 and 
	     45000 s - the latter would  enable working with 
	     half-geosynchronous spacecraft, which is pretty liberal.  
	     Geostationary spacecraft will not have useful up-  and 
	     down- crossings anyway.  The main point here is to detect
	     for example accidental zero values (1993-01-01 in Toolkit TAI. 
	     Nevertheless, we include testing that the Descend time exceeds
	     the Ascend time; the latter is start of orbit so the Descend 
	     time needs to me about 1/2 orbit later. */
	  
	  if( ((hdforbitDescend[j] - 
		hdforbitAscend[j]) < 2400.0) || 
	      ((hdforbitDescend[j] - 
		hdforbitAscend[j]) > 45000.0) || 
	      ( (*XnumOrbits > 1) && 
		(((hdforbitAscend[j] - 
		   laggedDescendTime1) < 2400.0 ) || 
		 ((hdforbitAscend[j] - 
		   laggedDescendTime1) > 45000.0 ))))
	    {
	      returnStatus = PGSEPH_W_CORRUPT_METADATA;
	      sprintf(specifics,
		      "\nInconsistent Descending vs Ascending node times at orbit # %d"
		      ,hdforbitNumber[j]);
	      PGS_SMF_GetMsgByCode(returnStatus,errorBuf);
	      strcat(errorBuf,specifics);
	      PGS_SMF_SetDynamicMsg(returnStatus,errorBuf,FUNCTION_NAME2);
	    }
	  
	  /* certain checks need to be imposed from orbit to orbit so 
	     save info. This is needed for the times because they are put 
	     in the OUTPUTs only in ASCII form; we may as well save the 
	     orbit number in a similar way  */
	  
	  laggedOrbitNumber =  hdforbitNumber[j]; /*(just for parallel
						    notation) */
	  laggedAscendTime = hdforbitAscend[j]; 
	  laggedDescendTime = hdforbitDescend[j];
	  
	  
	  /* convert equator downcrossing time to ASCII string and 
	     record it */
	  /* This is approximately mid-orbit and is used as a reference time
	     for the downcrossing longitude  */
	  
	  returnStatus1 = PGS_TD_TAItoUTC(hdforbitDescend[j],
					  XhdfDescendTime[*XnumOrbits]);
	  switch (returnStatus1)
	    {
	    case PGS_S_SUCCESS:
	    case PGSTD_E_NO_LEAP_SECS:
	      break;
	      
	    case PGS_E_TOOLKIT:
	      return returnStatus1;
	      
	    default:
	      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
	      return PGS_E_TOOLKIT;
	    }
	  
	  /* record longitude of downward (i.e. south-bound) equator
	     crossing for this orbit */
	  
	  XhdforbitLongitude[*XnumOrbits] = hdforbitLongitude[j];
	  laggedorbitDescendLongitude = hdforbitLongitude[j];
	  /* increment the number of relevant orbits found */
	  
	  *XnumOrbits = (*XnumOrbits) + 1;
	} /* end for (j=0;j<totalOrbits;j++)  */
      
    }
  
  return returnStatus;
}



/*Shell sort for sorting orbit numbers */

void PGS_EPH_ShellSort(PGSt_integer ntotal, PGSt_integer hon[],
		       PGSt_double  hoa[], PGSt_double  hod[],
		       PGSt_double  hol[])
{
  PGSt_integer  n;
  PGSt_integer  i, j, inc;
  PGSt_integer thon;
  PGSt_double thoa, thod, thol;
  inc = 1; /* Determine the starting increment */
  n = ntotal -1;
  do {
    inc *= 3;
    inc++;
  } while (inc <= n);
  do { /* Loop over the partial sorts */
    inc /=3;
    for (i=inc+1;i<=n;i++) { /* Outer loop of staraight insertion */
      thon = hon[i];
      thoa = hoa[i];
      thod = hod[i];
      thol = hol[i];
      j=i;
      while (hon[j-inc] > thon) { /* Inner loop of staraight insertion */
	hon[j] =  hon[j-inc];
	hoa[j] =  hoa[j-inc];
	hod[j] =  hod[j-inc];
	hol[j] =  hol[j-inc];
	j -= inc;
	if(j <= inc) break;
      }
      hon[j] =  thon;
      hoa[j] =  thoa;
      hod[j] =  thod;
      hol[j] =  thol;
    }
  } while ( inc > 1);
  
  /* if first element were not sorted, replace elents 1 & 2, and sort again */
  if(hon[0] > hon[1])
    {
      thon = hon[0];
      thoa = hoa[0];
      thod = hod[0];
      thol = hol[0];
      hon[0] =  hon[1];
      hoa[0] =  hoa[1];
      hod[0] =  hod[1];
      hol[0] =  hol[1];
      hon[1] =  thon;
      hoa[1] =  thoa;
      hod[1] =  thod;
      hol[1] =  thol;
      inc = 1; /* Determine the starting increment */
      
      do {
	inc *= 3;
	inc++;
      } while (inc <= n);
      do { /* Loop over the partial sorts */
	inc /=3;
	for (i=inc+1;i<=n;i++) { /* Outer loop of staraight insertion */
	  thon = hon[i];
	  thoa = hoa[i];
	  thod = hod[i];
	  thol = hol[i];
	  j=i;
	  while (hon[j-inc] > thon) { /* Inner loop of staraight insertion */
	    hon[j] =  hon[j-inc];
	    hoa[j] =  hoa[j-inc];
	    hod[j] =  hod[j-inc];
	    hol[j] =  hol[j-inc];
	    j -= inc;
	    if(j <= inc) break;
	  }
	  hon[j] =  thon;
	  hoa[j] =  thoa;
	  hod[j] =  thod;
	  hol[j] =  thol;
	}
      } while ( inc > 1);
    }
}
