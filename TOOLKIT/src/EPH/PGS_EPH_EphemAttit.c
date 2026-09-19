/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_EPH_EphemAttit.c

DESCRIPTION:
   This file contains the function PGS_EPH_EphemAttit()
   This function gets ephemeris and/or attitude data for the specified
   spacecraft at the specified times.

AUTHOR:
   Guru Tej S. Khalsa   / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation
   Abe Taaheri          / L3 Comm. EER corp.
   Adura Adekunjo       / L3 Comm. EER corp.

HISTORY:
   08-Apr-1994  GTSK  Initial version
   08-Sep-1994  GTSK  Updated variable types to latest PGS standards. Added ypr
                      and yprRate to calling sequence.  Improved error handling
		      somewhat.
   01-Oct-1994  GTSK  Improved error handling.  Fixed code to handle case of
                      numValues = 0.  Return error on numValues < 0.  Continues
		      execution on PGSTD_E_NO_LEAP_SECS error now.
   17-Oct-1994  GTSK  Entirely rewritten to read s/c ephemeris files from disk.
                      Approximately 0% of code is same as previous version.
   01-Jan-1995  GTSK  Removed code to read s/c ephemeris files and moved to a
                      separate function: PGS_EPH_getEphemRecords.  Fixed bug in
		      determining TRMM (geodetic-based attitude) s/c to ECI
		      rotation quaternion.
   01-Feb-1995  GTSK  Cast all pointers being passed in to the memcpy() 
                      function to type (char *).  This is to prevent issuance 
                      of spurious compiler messages about incompatible 
                      arguments from the SunOS 4.x platforms. It should have 
                      no effect on other platforms since the function memcpy()
                       takes void pointers 
                      and doesn't care what type of pointer is passed in.
   20-Mar-1995  GTSK  Replaced call to malloc() with call to PGS_MEM_Calloc().
                      Replaced string "PGS_EPH_EphemAttit()" with 
                      FUNCTION_NAME. Renamed variable startUTC to asciiUTC.
   23-May-1995  GTSK  Added check on time interval between actual records when
                      interpolating.  No interpolation is done if the time 
                      between actual records is > MAX_TIME_INTERVAL.  The
		      quality flag(s) is(are) set to indicate no data for a 
		      requested time for which interpolation can't be done.
   21-Dec-1995  GTSK  Fixed previously implemented scheme to avoid reloading
                      ephemeris data into memory from disk files by checking if
		      the required data was loaded in a previous call to this
		      function.  The time range information was being properly
		      saved but the s/c tag value was NOT being saved (causing
		      this routine to always assume that each time it was
		      called a new value of s/c tag had been passed in).  
		      This has been fixed.
   05-May-1996  GTSK  Implemented major changes to accommodate new
                      ephemeris/attitude file formats.
   27-May-1997  PDN   Deleted references to predicted leap seconds
   12-Feb-1999  PDN   Fixed nomenclature consistent with AM1 and later 
                      spacecraft note that the function is also OK for TRMM, 
                      but in that case
                      please observe that angularVelocity is really (yaw rate, 
                      pitch rate, and roll rate in that order).
   22-Feb-1999  PDN   Shortened the maximum interpolation interval from 121
                      seconds (useful in I&T) to 60 seconds, consistent with
                      DPREP capability for AM1 and probably later spacecraft.
   09-July-99   SZ    Updated for the thread-safe functionality
   24-Nov-1999  PDN   Altered the search through a block for the correct data
                      record(s) to use the (faster) method of bisection. Added 
                      comments. This is a rework of changes to Release 5A, July
                      1999 as the work was eclipsed by ThreadSafe changes
   10-Feb-2002  XW    Got ephemeris and/or attitude data from HDF files.
   08-Nov-2002  AT    Fixed problem with mem copying for non-existing array 
                      elements < 0 for ephemRecord1, ephemRecord2, 
                      attitRecord1, and attitRecord2. This was causing 
                      problem on DEC machine only.
  20-Nov-2002  AA     Added  the hdfcheck flag to distinguish between binary
                      and hdf files (ECSed35587).
  20-Mar-2003  AT     fixed problems with proper reading of HDF files.
  27-May-2003  AA,AT  increased efficiency of extracting records from HDF
		      formatted eph/att files.
  09-Jul-2003  AT     Modified   PGS_EPH_checkHDF function for closing datasets,
                      or hdf files that are really open (to avoid core dump in
                      linux platform), and fix problem for getting record for
                      a time that falls in a gap between two consecutive hdf
                      att or eph files, and there are back to back calls to the
                      function PGS_EPH_EphemAttit (or a function that calls
                      PGS_EPH_EphemAttit such as PGS_CSC_SCtoORB ) at the 
                      same ASCCI time and offsets in user's code.
  28-Jul-2009  AT     Modified code to assume that all eph/att file have the
                      same Endianness. So once lendcheck =0 or 1, it will be
                      0 or 1 for all eph/att files. This fixes NCR 8048837,
                      problem found in MOPITT daily processing in linux 64-bit
                      with Big Endian eph/att files.
                      
END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Ephemeris and Attitude

NAME:
   PGS_EPH_EphemAttit()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_EphemAttit(
        PGSt_tag        spacecraftTag,   
	PGSt_integer    numValues,        
	char            *asciiUTC,       
	PGSt_double     offsets[],       
	PGSt_boolean    orbFlag,          
	PGSt_boolean    attFlag,
        PGSt_integer    qualityFlags[][2],
	PGSt_double     positionECI[][3], 
	PGSt_double     velocityECI[][3], 
	PGSt_double     eulerAngles[][3],         
	PGSt_double     xyzRotRates[][3],    
	PGSt_double     attitQuat[][4])

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD.f'
      include 'PGS_TD_3.f'
      include 'PGS_EPH_5.f'
      include 'PGS_MEM_7.f'

      integer function pgs_eph_ephemattit(spacecrafttag,numvalues,
     >                                    asciiutc,offsets,orbflag,
     >                                    attflag,qualityflags,
     > 				          positioneci,velocityeci,
     >                                    eulerangles,xyzrotrates,
     > 				          attitquat)
      integer           spacecrafttag
      integer           numvalues
      character*27      asciiutc
      double precision  offsets(*)
      integer           orbflag
      integer           attflag
      integer           qualityflags(2,*)
      double precision  positioneci(3,*)
      double precision  velocityeci(3,*)
      double precision  eulerangles(3,*)
      double precision  xyzrotrates(3,*)
      double precision  attitquat(4,*)
     
DESCRIPTION:
   This tool gets ephemeris and/or attitude data for the specified spacecraft
   at the specified times.

INPUTS:
   Name               Description               Units  Min        Max
   ----               -----------               -----  ---        ---
   spacecraftTag      spacecraft identifier     N/A

   numValues          num. of values requested  N/A

   asciiUTC           UTC time reference start  ASCII  1961-01-01 see NOTES
                      time in CCSDS ASCII time
	              code A format

   offsets            array of time offsets in  sec    ** depends on asciiUTC **
                      seconds relative to 
	              asciiUTC

   orbFlag            set to true to get        T/F
                      ephemeris data

   attFlag            set to true to get        T/F
                      attitude data

OUTPUTS:
   Name              Description                Units        Min         Max
   ----              -----------                -----        ---         ---
   qualityFlags      quality flags for                 ** see NOTES **
                     position and attitude
		     
   positionECI       ECI position               meters

   velocityECI       ECI velocity               meters/sec

   eulerAngles       s/c attitude as a set of   radians
                     Euler angles

   xyzRotRates       angular rates about body   radians/sec
                     x, y and z axes

   attitQuat         spacecraft to ECI          N/A
                     rotation quaternion
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSEPH_W_BAD_EPHEM_VALUE    one or more values could not be determined
   PGSMEM_E_NO_MEMORY          unable to allocate required dynamic memory space
   PGSEPH_E_BAD_EPHEM_FILE_HDR no s/c ephemeris and/or attitude files had
                               readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c ephemeris and/or attitude files could be
                               found for input times
   PGSEPH_E_NO_DATA_REQUESTED  both orb and att flags are set to false
   PGSTD_E_SC_TAG_UNKNOWN      unrecognized/unsupported spacecraft tag
   PGSEPH_E_BAD_ARRAY_SIZE     array size specified is less than 0
   PGSTD_E_TIME_FMT_ERROR      format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR    value error in asciiUTC
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for initial
                               time (asciiUTC)
   PGS_E_TOOLKIT               an unexpected error occured
   PGSTSF_E_GENERAL_FAILURE    problem in the thread-safe code

EXAMPLES:
C:
        #define ARRAY_SIZE 10

	PGSt_double     offsets[ARRAY_SIZE];
	PGSt_double     positionECI[ARRAY_SIZE][3];
	PGSt_double     velocityECI[ARRAY_SIZE][3];
	PGSt_double     eulerAngles[ARRAY_SIZE][3];
	PGSt_double     xyzRotRates[ARRAY_SIZE][3];
	PGSt_double     attitQuat[ARRAY_SIZE][4];

	char            asciiUTC[28];

	PGSt_integer    qualityFlags[ARRAY_SIZE][2];

	int             i;
	
	PGSt_SMF_status returnStatus;


	** intialize asciiUTC and offsets array **

	strcpy(asciiUTC,"1998-02-03T19:23:45.123");
	for (i=0;i<ARRAY_SIZE;i++)
	  offsets[i] = (PGSt_double) i;

        returnStatus = PGS_EPH_EphemAttit(PGSd_EOS_AM,ARRAY_SIZE,asciiUTC,
	                                  offsets,PGS_TRUE,PGS_TRUE,
					  qualityFlags,positionECI,velocityECI,
					  eulerAngles,xyzRotRates,attitQuat);

	if (returnStatus != PGS_S_SUCCESS)
	{
	            :
	 ** do some error handling ***
		    :
	}
   
FORTRAN:
      implicit none

      include  'PGS_SMF.f'
      include  'PGS_TD.f'
      include  'PGS_TD_3.f'
      include  'PGS_EPH_5.f'
      include  'PGS_MEM_7.f'

      integer           pgs_eph_ephemattit

      integer           numvalues/10/
      integer           i
      integer           returnstatus
      integer           qualityflags(2,numvalues)
      
      character*27      asciiutc
      
      double precision  offsets(numvalues)
      double precision  positioneci(3,numvalues)
      double precision  velocityeci(3,numvalues)
      double precision  eulerangles(3,numvalues)
      double precision  xyzrotrates(3,numvalues)
      double precision  attitquat(4,numvalues)

!  intialize asciiutc and offsets array

      asciiutc = '1998-02-03T19:23:45.123'
      do 100 i = 1,numvalues
 100    offsets(i) = i-1

      returnstatus =  pgs_eph_ephemattit(pgsd_eos_am,numvalues,asciiutc,
     >                                   offsets,pgs_true,pgs_true,
     >                                   qualityflags,positioneci,
     >          			 velocityeci,eulerangles,
     >                                   xyzrotrates,attitquat)

      if (returnstatus .ne. pgs_s_success) then
                :
      *** do some error handling ***
                :
      endif

NOTES:
   QUALITY FLAGS:

     The quality flags are returned as integer quantities but should be
     interpreted as bit fields.  Only the first 32 bits of each quality flag is
     meaningfully defined, any additional bits should be ignored (currently
     integer quantities are 32 bits on most UNIX platforms, but this is not
     guaranteed to be the case--e.g. an integer is 64 bits on a Cray).

     Generally the quality flags are platform specific and are not defined by
     the Toolkit.  Two bits of these flags have, however, been reserved for SDP
     Toolkit usage.  Bit 12 will be set by the Toolkit if no data is available
     at a requested time, bit 14 will be set by the Toolkit if the data at the
     requested time has been interpolated (the least significant bit is "bit
     0").  Any other bits are platform specific and are the responsibility of
     the user to interpret.

   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a 
     new leap second event is announced, contains dates of leap second events.
     If an input date is outside of the range of dates in the file (or if the 
     file cannot be read) the function PGS_TD_LeapSec() which reads the file
     will return a calculated value of TAI-UTC based on a linear fit of the data
     in the table as of late 1997.  This value of TAI-UTC is relatively crude 
     estimate and may be off by as much as 1.0 or more seconds. The warning 
     return status from PGS_TD_LeapSec() is promoted to an error level by any 
     Toolkit function receiving it, which will terminate the present function.

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

     Toolkit internal time is the real number of continuous SI seconds since the
     epoch of UTC 12 AM 1-1-1993.  Toolkit internal time is also referred to in
     the toolkit as TAI (upon which it is based).

   TIME OFFSETS:

      This function accepts an ASCII UTC time, an array of time offsets and the
      number of offsets as input.  Each element in the offset array is an offset
      in seconds relative to the initial input ASCII UTC time.

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

   ERROR HANDLING:

      This function processes data over an array of times (specified by an input
      ASCII UTC time and an array of time offsets relative to that time). 

      If processing at each input time is successful the return status of this
      function will be PGS_S_SUCCESS (status level of 'S').

      If processing at ALL input times was unsuccessful the status level of the
      return status of this function will be 'E'.

      If processing at some (but not all) input times was unsuccessful the
      status level (see SMF) of the return status of this function will be 'W'
      AND all high precision real number (C: PGSt_double, FORTRAN: DOUBLE
      PRECISION) output variables that correspond to the times for which
      processing was NOT successful will be set to the value:
      PGSd_GEO_ERROR_VALUE.  In this case users may (should) loop through the
      output testing any one of the aforementioned output variables against
      the value PGSd_GEO_ERROR_VALUE.  This indicates that there was an error in
      processing at the corresponding input time and no useful output data was
      produced for that time.  

      Note: a return status with a status of level of 'W' does not necessarily
      mean that some of the data could not be processed.  The 'W' level may
      indicate a general condition that the user may need to be aware of but
      that did not prohibit processing.  For example, if an Earth ellipsoid
      model is required, but the user supplied value is undefined, the WGS84
      model will be used, and processing will continue normally,except that the
      return status will be have a status level of 'W' to alert the user that
      the default earth model was used and not the one specified by the user.
      The reporting of such general warnings takes precedence over the generic
      warning (see RETURNS above) that processing was not successful at some of
      the requested times.  Therefore in the case of any return status of level
      'W',the returned value of a high precision real variable generally should
      be examined for errors at each time offset, as specified above.

      Special Note: for this tool, the associated quality flags will also
      indicate that no data is available for those points that could not be
      successfully processed (see QUALITY FLAGS above).

REQUIREMENTS:
   PGSTK - 0141, 0720, 0740

DETAILS:
   None

GLOBALS:
   PGSg_TSF_EPHtotalEphemRecords
   PGSg_TSF_EPHtotalAttitRecords
   PGSg_TSF_EPHbeginTAI93eph
   PGSg_TSF_EPHendTAI93eph
   PGSg_TSF_EPHbeginTAI93att
   PGSg_TSF_EPHendTAI93att
   PGSg_TSF_EPHspacecraft   

FILES:
   The function requires the presence of the files:
      leapsec.dat
      earthfigure.dat
      utcpole.dat
   This function also requires ephemeris and attitude data files.

FUNCTIONS_CALLED:
   PGS_MEM_Calloc()              dynamically allocate (and initialize) memory
   PGS_TD_UTCtoTAI()             returns continuous seconds since 12AM 1/1/1993
   PGS_TD_sortArrayIndices()     sort offsets into ascending order
   PGS_EPH_getEphemRecords()     returns an array of s/c ephemeris records
   PGS_EPH_interpolatePosVel()   interpolates position and velocity
   PGS_EPH_interpolateAttitude() interpolates attitude Euler angles and rates
   PGS_CSC_getECItoORBquat()     returns the ECI to ORB quaternion
   PGS_CSC_EulerToQuat()         converts Euler angles to a quaternion
   PGS_CSC_quatMultiply()        multiplies two quaternions, returns result
   PGS_SMF_SetStaticMsg()        sets the message buffer
   PGS_SMF_SetDynamicMsg()       sets the message buffer
   PGS_SMF_SetUnknownMsg()       sets the message buffer
   PGS_TSF_LockIt()
   PGS_TSF_UnlockIt()
   PGS_SMF_TestErrorLevel()
   PGS_TSF_GetTSFMaster()
   PGS_TSF_GetMasterIndex()

END_PROLOG:
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_MEM.h>
#include <PGS_TSF.h>
#include <PGS_PC.h>
#include <PGS_SMF.h>
#include <PGS_IO.h>
#include <PGS_TD.h>
#include <dirent.h>
#include <hdf.h>
#include <mfhdf.h>
#include <PGS_math.h>

#define  MAX_ATT_EPH_FILES     1000    /* Max number of ATT or EPH files in PCF */
#define  GAP_COVER_BLKS        7       /*extra block to add to originally read
					 blocks to handle times that fall in 
					 the gap between two consecutive 
					 files */

#define  BLOCK_SIZE            6000    /* size of ephemDataBlock array */
#define  BLOCK_SIZE_HDF	       86500   /* size of hdfDataBlock array 
					  24hr of 1 sec interval records + 
					  100 extra records*/

#define  MAX_EPH_TIME_INTERVAL 60.0   /* maximum time interval (in seconds)
					  over which ephemeris interpolation
					  will be done */

#define  MAX_ATT_TIME_INTERVAL 60.0   /* maximum time interval (in seconds)
					  over which attitude interpolation
					  will be done */
typedef struct
{
    PGSt_boolean    orbFlag;               /* get/don't get orbit data */
    PGSt_boolean    attFlag;               /* get/don't get attitude data */
    PGSt_integer    (*qFlags)[2];          /* quality flags */
    PGSt_double     (*positionECI)[3];     /* ECI position */
    PGSt_double     (*velocityECI)[3];     /* ECI velocity */
    PGSt_double     (*eulerAngle)[3];      /* s/c attitude Euler angles */
    PGSt_double     (*angularVelocity)[3]; /* ang. rates about s/c body axes */
    PGSt_double     (*attitQuat)[4];       /* s/c to ECI rotation quaternion */
} PGSt_scData;

typedef struct
{
  PGSt_uinteger num_files;
  PGSt_integer  fileversion[MAX_ATT_EPH_FILES];          /* ephem file version in PCF */
  PGSt_double   startTime[MAX_ATT_EPH_FILES];            /* Ephemeris dataset start time,
							    secTAI93 */
  PGSt_double   endTime[MAX_ATT_EPH_FILES];              /* Ephemeris dataset end time,
							    secTAI93 */
  PGSt_tag      spacecraftTag[MAX_ATT_EPH_FILES];
} PGSt_ephemfiles;

typedef struct
{
  PGSt_uinteger num_files;
  PGSt_integer fileversion[MAX_ATT_EPH_FILES];          /* ephem file version in PCF */
  PGSt_double   startTime[MAX_ATT_EPH_FILES];            /* Ephemeris dataset start time,
							    secTAI93 */
  PGSt_double   endTime[MAX_ATT_EPH_FILES];              /* Ephemeris dataset end time,
							    secTAI93 */
  PGSt_tag      spacecraftTag[MAX_ATT_EPH_FILES];
} PGSt_attitfiles;

/* prototype for function used to set bad s/c data to PGSd_GEO_ERROR_VALUE */

static void
PGS_EPH_setBadEphemRecord(PGSt_integer,PGSt_scData*,PGSt_SMF_status*);

/* prototype for function used to get ephemeris and/or attitude data 
from HDF files */

static PGSt_SMF_status PGS_EPH_checkHDF(PGSt_tag, PGSt_double , 
					PGSt_integer , PGSt_integer [2], 
					PGSt_double [3], PGSt_double [3], 
					PGSt_double [3], PGSt_double [3], 
					PGSt_double [4]);

/* name of the functions */

#define  FUNCTION_NAME "PGS_EPH_EphemAttit()"
#define  FUNCTION_NAME2 "PGS_EPH_checkHDF()"

PGSt_SMF_status
PGS_EPH_EphemAttit(                      /* get ephemeris and attitude data */
    PGSt_tag        spacecraftTag,       /* spacecraft identifier */
    PGSt_integer    numValues,           /* number of values requested */
    char            *asciiUTC,           /* reference start time */
    PGSt_double     offsets[],           /* time offsets relative to asciiUTC*/
    PGSt_boolean    orbFlag,             /* get/don't get orbit data */
    PGSt_boolean    attFlag,             /* get/don't get attitude data */
    void            *qualityFlagsPtr,    /* quality flags */
    PGSt_double     positionECI[][3],    /* ECI position */
    PGSt_double     velocityECI[][3],    /* ECI velocity */
    PGSt_double     eulerAngle[][3],     /* s/c attitude Euler angles */
    PGSt_double     angularVelocity[][3],/* angular rates about s/c body axes*/
    PGSt_double     attitQuat[][4])      /* s/c to ECI rotation quaternion */
{
#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;         /* lock and unlock return */ 
#endif

#ifdef _PGS_THREADSAFE
    /* Create non-static variables for the thread-safe version */
    PGSt_ephemRecord *ephemRecordArray;
    PGSt_attitRecord *attitRecordArray;
    PGSt_integer     totalEphemRecords;
    PGSt_integer     totalAttitRecords;
    PGSt_double      beginTAI93eph;
    PGSt_double      endTAI93eph;
    PGSt_double      beginTAI93att;
    PGSt_double      endTAI93att;
    PGSt_tag         spacecraft;
    PGSt_integer     lendcheck_old;
    int              hdfcheck;
    int              anum;
    PGSt_SMF_status  retVal;
#else
    static PGSt_ephemRecord ephemRecordArray[BLOCK_SIZE]; /* s/c ephemeris file
							     records */
    static PGSt_attitRecord attitRecordArray[BLOCK_SIZE]; /* s/c ephemeris file
							     records */
    static PGSt_integer     totalEphemRecords=0;          /* number of records
							     in scRecordArray*/
    static PGSt_integer     totalAttitRecords=0;          /* number of records
							     in scRecordArray*/
    static PGSt_double      beginTAI93eph=1.0e50;         /* first TAI time in
							     ephemRecordArray*/
    static PGSt_double      endTAI93eph=-1.0e50;          /* last TAI time in
							     ephemRecordArray*/
    static PGSt_double      beginTAI93att=1.0e50;         /* first TAI time in
							     attitRecordArray*/
    static PGSt_double      endTAI93att=-1.0e50;          /* last TAI time in
							     attitRecordArray*/
    static PGSt_tag         spacecraft=0;                 /* spacecraftTag 
							     value from last 
							     call to this 
							     function */
    static PGSt_integer     lendcheck_old= -1;/* big/little endian m/c check */
                                             /* set to -1 here so that when its
					     setting by PGS_EPH_getAttitHeaders
					     is required, we check it to see
					     whether it is set to 0 or 1 */
    
    static int       hdfcheck=0;      /* checks if HDF/Binary file is needed */
    static int       anum = 0;
#endif
    PGSt_integer    lendcheck= -1;    /* big/little endian m/c check */
                                      /* set to -1 here so that when its
					 setting by PGS_EPH_getAttitHeaders
					 is required, we check it to see
					 whether it is set to 0 or 1 */
    PGSt_SMF_status  pgsStatus;
    PGSt_integer     hdfcheck_next=0;
    PGSt_integer     num_files_eph = 0;
    PGSt_integer     ifile, ithfile;
    char             File[256]; 
    PGSt_ephemRecord ephemRecord1;    /* s/c ephemeris file record */
    PGSt_ephemRecord ephemRecord2;    /* s/c ephemeris file record */

    PGSt_attitRecord attitRecord1;    /* s/c ephemeris file record */
    PGSt_attitRecord attitRecord2;    /* s/c ephemeris file record */

    PGSt_integer     endRecord=0;     /* end of search sub-block
                                         for binary search */
    PGSt_integer     midRecord =0;    /* recursively, 1/2, 1/4,..
                                         of the  records in scRecordArray */
    PGSt_scData      scData;          /* structure of pointers to output
					 arrays */
    PGSt_boolean     gotData;         /* indicates outcome of data search */

    PGSt_integer     *sortedIndex=NULL;/* pointer to array holding sorted 
					 indices
					 of time offset array (offsets) */
    PGSt_integer     maxValues;       /* number of records requested */
    PGSt_integer     cnt;             /* loop counter */
    PGSt_integer     count;           /* loop counter */
    PGSt_integer     ephRecordCounter=0; /* loop counter */
    PGSt_integer     attRecordCounter=0; /* loop counter */
    PGSt_integer     (*qualityFlags)[2]; /* pointer to quality flag data */
    PGSt_integer     numBadPoints=0;  /* count of input times for which no
					 ephem/attit data could be determined*/
   PGSt_double      secTAI93;        
				      /* time of requested s/c data */
    PGSt_double      startTAI93;      /* TAI equivalent of asciiUTC */
    PGSt_double      tempPosVel[6];   /* s/c position and velocity at requested
					 time */
    PGSt_double      *tempPosition;   /* s/c position at requested time */
    PGSt_double      *tempVelocity;   /* s/c velocity at requested time */
    PGSt_double      quatORBtoECI[4]; /* orbital ref. frame to ECI quaternion*/
    PGSt_double      quatEuler[4];    /* quaternion equivalent of s/c attitude
					 as defined by the s/c Euler angles */

    PGSt_scTagInfo   scTagInfo;       /* s/c tag info. structure */
    
    char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* error mnemonic */
    char             msg[PGS_SMF_MAX_MSG_SIZE];           /* error message */
    char             details[PGS_SMF_MAX_MSG_SIZE]; /* detailed error message */

    PGSt_SMF_status  returnStatus1;    /* return value of PGS function calls */
    PGSt_SMF_status  code;             /* returned from PGS_SMF_GetMsg() */
    PGSt_SMF_status  returnStatus;     /* return value of this function */

    PGSt_double      pos[3];           /* position */
    PGSt_double      vel[3];           /* velocity */
    PGSt_double      eul[3];           /* attitude Euler angles */
    PGSt_double      ang[3];           /* angular rates */
    PGSt_double      atti[4];          /* rotation quaternion */
    PGSt_integer     qua[2];           /* quality flags */

    int 	     attData=0;        /* get/don't get attitude data */
    int              j;                /* counter */
    int              number;           /* file number counter */
      
#ifdef _PGS_THREADSAFE
    /* Set up TSF globals, global index, and TSF key keeper for the 
       thread-safe */
    int masterTSFIndex;
    extern PGSt_integer PGSg_TSF_EPHtotalEphemRecords[];
    extern PGSt_integer PGSg_TSF_EPHtotalAttitRecords[];
    extern PGSt_double PGSg_TSF_EPHbeginTAI93eph[];
    extern PGSt_double PGSg_TSF_EPHendTAI93eph[];
    extern PGSt_double PGSg_TSF_EPHbeginTAI93att[];
    extern PGSt_double PGSg_TSF_EPHendTAI93att[];
    extern PGSt_tag PGSg_TSF_EPHspacecraft[];
    extern PGSt_tag PGSg_TSF_EPHlendcheck_old[];
    extern PGSt_tag PGSg_TSF_EPHhdfcheck[];
    extern PGSt_tag PGSg_TSF_EPHanum[];

    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(retVal) || 
	masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart and TSD key for the thread-safe */
    totalEphemRecords = PGSg_TSF_EPHtotalEphemRecords[masterTSFIndex];
    totalAttitRecords = PGSg_TSF_EPHtotalAttitRecords[masterTSFIndex];
    beginTAI93eph = PGSg_TSF_EPHbeginTAI93eph[masterTSFIndex];
    endTAI93eph = PGSg_TSF_EPHendTAI93eph[masterTSFIndex];
    beginTAI93att = PGSg_TSF_EPHbeginTAI93att[masterTSFIndex];
    endTAI93att = PGSg_TSF_EPHendTAI93att[masterTSFIndex];
    spacecraft = PGSg_TSF_EPHspacecraft[masterTSFIndex];
    lendcheck_old = PGSg_TSF_EPHlendcheck_old[masterTSFIndex];
    hdfcheck = PGSg_TSF_EPHhdfcheck[masterTSFIndex];
    anum = PGSg_TSF_EPHanum[masterTSFIndex];

    ephemRecordArray = (PGSt_ephemRecord *) pthread_getspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHEPHEMRECORDARRAY]);
    attitRecordArray = (PGSt_attitRecord *) pthread_getspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHATTITRECORDARRAY]);
#endif
   
    /* initialize secTAI93 and related times */
    ephemRecord1.secTAI93 = 0.0;
    ephemRecord2.secTAI93 = 0.0;
    attitRecord1.secTAI93 = 0.0;
    attitRecord2.secTAI93 = 0.0;
    secTAI93 = 0.0;

    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS;
    returnStatus1 = PGS_S_SUCCESS;

    /* initialize position and velocity pointers to point to the appropriate
       places is the array tmpPosVel */

    tempPosition = tempPosVel;
    tempVelocity = tempPosVel+3;


    /* The incoming pointer to the quality flags array must be cast to type:
       pointer to array of two integers.  This is what is expected. A void type
       is being used for now to handle some backward compatibility issues,
       namely that this slot used to be for an array of character strings. */

    qualityFlags = (PGSt_integer (*)[2]) qualityFlagsPtr;

    /* check that at least SOME data have been requested */

    if (orbFlag != PGS_TRUE && attFlag != PGS_TRUE)
    {
	PGS_SMF_SetStaticMsg(PGSEPH_E_NO_DATA_REQUESTED,FUNCTION_NAME);
	return PGSEPH_E_NO_DATA_REQUESTED;
    }

    /* check spacecraft tag for validity */

    returnStatus = PGS_EPH_GetSpacecraftData(spacecraftTag, NULL,
					     PGSe_TAG_SEARCH, &scTagInfo);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;

      case PGSTD_E_SC_TAG_UNKNOWN:
      case PGS_E_TOOLKIT:
	return returnStatus;

      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* if spacecraftTag is the same as in previous call to this function, then
       eph and att data files should be the same files as in previous call, and
       therefore they should have the same endianness */

    if((spacecraftTag == spacecraft) || (lendcheck_old == 0 || lendcheck_old == 1))
      {
	lendcheck = lendcheck_old;
      }

    /* check numValues to make sure it is not negative */

    if (numValues < 0)
    {
	PGS_SMF_SetStaticMsg(PGSEPH_E_BAD_ARRAY_SIZE,FUNCTION_NAME);
	return PGSEPH_E_BAD_ARRAY_SIZE;
    }

    /* The number of records requested is the number specified by numValues
       passed in the calling sequence unless this number is 0 in which case one
       record is still retrieved.  In the latter case this will be the record
       corresponding to the ASCII time input (asciiUTC). */

    maxValues = (numValues) ? numValues : 1;

    /* In order to expedite processing, it is desirable that the s/c data be
       searched for in monotonically increasing temporal order.  Therefore an
       array of PGSt_integers is allocated to hold the sorted indices of the
       input time offsets (offsets).  The array "offsets" is sorted into
       chronological order by storing the indices in "sortedIndex" in such a
       way that accessing the data by these indices accesses the data in
       chronological order.  The actual input array "offsets" is in no way
       affected. */

    returnStatus = PGS_MEM_Calloc((void **) &sortedIndex, 1,
				  sizeof(PGSt_integer)*maxValues);
    if (returnStatus != PGS_S_SUCCESS)
	return returnStatus;

    PGS_TD_sortArrayIndices(offsets,maxValues,sortedIndex);

    /* for simplified error handling the following structure is used to sort of
       keep a copy of the s/c data being returned by this function */

    scData.orbFlag = orbFlag;
    scData.attFlag = attFlag;
    scData.qFlags = qualityFlags;
    scData.positionECI = positionECI;
    scData.velocityECI = velocityECI;
    scData.eulerAngle = eulerAngle;
    scData.angularVelocity = angularVelocity;
    scData.attitQuat = attitQuat;

    /* convert ASCII UTC start time to real continuous seconds since
       12AM UTC 1/1/93, this is done so that the time offsets which are in
       seconds can be added to this value to get a unique numeric time for 
       each point */

    returnStatus = PGS_TD_UTCtoTAI(asciiUTC, &startTAI93);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&code,mnemonic,details);
	if (code != returnStatus)
	  PGS_SMF_GetMsgByCode(returnStatus,details);
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
	PGS_MEM_Free(sortedIndex);
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	PGS_MEM_Free(sortedIndex);
	return PGS_E_TOOLKIT;
    }

    /* the first time requested is the reference time plus the first offset,
       unless numValues is 0 in which case the first time is just the reference
       time itself--here the time is initialized to this reference time */

    /* check that all eph & att files are HDF or BINARY */
    if (hdfcheck == 0 && anum == 0)
      { 
	pgsStatus = PGS_PC_GetNumberOfFiles(PGSd_SC_EPHEM_DATA, &num_files_eph);
	
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
	    pgsStatus = 
	      PGS_PC_GetReference(PGSd_SC_EPHEM_DATA, &ithfile, File);
	    
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
	if(attFlag == PGS_TRUE)
	  {
	    /* check that all attitude file have the same format as 
	       ephemeris files */
	    pgsStatus = 
	      PGS_PC_GetNumberOfFiles(PGSd_SC_ATTIT_DATA, &num_files_eph);
	    
	    if (pgsStatus != PGS_S_SUCCESS)
	      {
		pgsStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		PGS_SMF_SetDynamicMsg(pgsStatus,
				      "error in accessing spacecraft attitude file(s)",
				      FUNCTION_NAME);
		return pgsStatus;
	  }
	
	    for(ifile = 1; ifile<= num_files_eph; ifile++)
	      {
		ithfile = ifile;
		pgsStatus = 
		  PGS_PC_GetReference(PGSd_SC_ATTIT_DATA, &ithfile, File);
		
		if (pgsStatus != PGS_S_SUCCESS)
		  {
		    PGS_SMF_SetDynamicMsg(pgsStatus,
					  "Error getting reference for att files",
					  FUNCTION_NAME);
		    return pgsStatus;
		  }
		
		hdfcheck_next = Hishdf(File);
		if(hdfcheck != hdfcheck_next)
		  {
		    PGS_SMF_SetDynamicMsg(PGSEPH_E_NO_SC_EPHEM_FILE,
					  "All ephemeris and attitude files must be either HDF or BINARY",
					  FUNCTION_NAME);
		    pgsStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		    return pgsStatus;
		  }
		anum = 1;
		hdfcheck = hdfcheck_next;
	      }
	  }
      }

    /* all file are either HDF or BINARY, Proceed ..... */

    if ( hdfcheck == 0) /* eph files are BINARY */
      { 
	if (numValues)
      secTAI93 = startTAI93 + offsets[sortedIndex[0]];
    else
      secTAI93 = startTAI93;

    /* Step through the requested times until a s/c record for one of the
       requested time is retrieved.  Note that "a s/c record" is really two
       records if a requested time is not coincident with a time recorded in 
       the s/c ephemeris file.  This is of course necessary in order to 
       interpolate the data to the requested time.  If no s/c record can 
       be found for any of the requested times exit with an error message. */

    cnt = 0;
    gotData = PGS_FALSE;

    if (secTAI93 < beginTAI93eph ||
	secTAI93 > endTAI93eph ||
	spacecraftTag != spacecraft)
    {
	do
	{
	    if (numValues)
	    {
		secTAI93 = startTAI93 + offsets[sortedIndex[cnt]];
	    }
	  
	    /* Fill the array ephemRecordArray with s/c ephemeris records. */

	    returnStatus1 = PGS_EPH_getEphemRecords(&scTagInfo,secTAI93,
						    BLOCK_SIZE,ephemRecordArray,
						    &totalEphemRecords, &lendcheck);
	    if(lendcheck == 1 || lendcheck == 0 )/* Endianness is the same for
						    all eph/att files */
	      {
		lendcheck_old = lendcheck;
	      }

#ifdef _PGS_THREADSAFE
            /* Reset globals */
            PGSg_TSF_EPHtotalEphemRecords[masterTSFIndex] = totalEphemRecords;
#endif
	    switch (returnStatus1)
	    {
	      case PGS_S_SUCCESS:
	      case PGSEPH_M_SHORT_ARRAY:
		gotData = PGS_TRUE;
		beginTAI93eph = ephemRecordArray[0].secTAI93;
		endTAI93eph = ephemRecordArray[totalEphemRecords-1].secTAI93;
#ifdef _PGS_THREADSAFE
                /* Reset globals */
                PGSg_TSF_EPHbeginTAI93eph[masterTSFIndex] = beginTAI93eph;
                PGSg_TSF_EPHendTAI93eph[masterTSFIndex] = endTAI93eph;   
#endif          
		break;
	      case PGSEPH_E_BAD_EPHEM_FILE_HDR:
	      case PGSEPH_E_NO_SC_EPHEM_FILE:
	      case PGS_E_TOOLKIT:

		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if (code != returnStatus1)
		  PGS_SMF_GetMsgByCode(returnStatus1,msg);
		if (returnStatus == PGS_S_SUCCESS)
		  strcpy(details,msg);
		numBadPoints++;
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif
		break;
	      default:
		PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
		returnStatus1 = PGS_E_TOOLKIT;
		numBadPoints++;
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif
		break;
	    }/* end switch on return from PGS_EPH_getEphemRecords() */
	} /* next line ends do, and increments cnt */
	while (++cnt < maxValues && gotData == PGS_FALSE);
    } /* END: if (secTAI93 < startTAI93eph ... except for "else" */
    else /* in this case time is in range; no need to get file */
    {
	gotData = PGS_TRUE;
	cnt++;
    }

    /* if gotData is still false here, all times were checked and no s/c
       ephemeris file was successfully opened, so exit with error message */

       if (gotData == PGS_FALSE)
    {
	PGS_MEM_Free(sortedIndex);

	PGS_SMF_SetDynamicMsg(returnStatus1,msg,FUNCTION_NAME);
	return returnStatus1;
    }

    /* now go on to obtaining attitude data */
    if (attFlag == PGS_TRUE)
    {
	attData = 1;
	count = cnt;
	cnt = 0;
	gotData = PGS_FALSE;
	
	if (secTAI93 < beginTAI93att ||
	    secTAI93 > endTAI93att ||
	    spacecraftTag != spacecraft)
	{
	    do
	    {
		if (numValues)
		{
		    secTAI93 = startTAI93 + offsets[sortedIndex[cnt]];
		}
	  
		/* Fill the array attitRecordArray with s/c attitude records.*/

		returnStatus1 = PGS_EPH_getAttitRecords(&scTagInfo,
							&lendcheck,
                                                        secTAI93,
							BLOCK_SIZE,
							attitRecordArray,
							&totalAttitRecords);
#ifdef _PGS_THREADSAFE
                /* Reset global */
                PGSg_TSF_EPHtotalAttitRecords[masterTSFIndex] = totalAttitRecords;
#endif
		switch (returnStatus1)
		{
		  case PGS_S_SUCCESS:
		  case PGSEPH_M_SHORT_ARRAY:
		    gotData = PGS_TRUE;
		    beginTAI93att = attitRecordArray[0].secTAI93;
		    endTAI93att =attitRecordArray[totalAttitRecords-1].secTAI93;

#ifdef _PGS_THREADSAFE
                    /* Reset globals */ 
                    PGSg_TSF_EPHbeginTAI93att[masterTSFIndex] = beginTAI93att;
                    PGSg_TSF_EPHendTAI93att[masterTSFIndex] = endTAI93att;
#endif          
		    break;
		  case PGSEPH_E_BAD_EPHEM_FILE_HDR:
		  case PGSEPH_E_NO_SC_EPHEM_FILE:
		  case PGS_E_TOOLKIT:

		    gotData = PGS_FALSE;
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if (code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    if (returnStatus == PGS_S_SUCCESS)
		      strcpy(details,msg);
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    break;
		  default:
		    gotData = PGS_FALSE;
		    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
		    returnStatus1 = PGS_E_TOOLKIT;
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    break;
		} /* end switch on return from PGS_EPH_getAttitRecords()*/
	    }
	    while (++cnt < maxValues && gotData == PGS_FALSE);
	}
	else /* time is already in the range */
	{
	    gotData = PGS_TRUE;
	    cnt++;
	} /* END: if (secTAI93 < startTAI93att ... */
	
	/* If "count" is greater than "cnt" then assign the value of "count" to
	   "cnt".  "cnt" is used to determine which input times have
	   corresponding s/c data.  This should therefore be set to the greater
	   of "count" (first time index for which valid ephemeris data was
	   found) and "cnt" (first time index for which valid attitude data was
	   found). */

	cnt = (count > cnt) ? count : cnt;
    }

    /* if gotData is still false here, all times were checked and no s/c
       (necessary) attitude file was successfully opened, so exit with error
       message */

       if (gotData == PGS_FALSE)
    {
	PGS_MEM_Free(sortedIndex);
	PGS_SMF_SetDynamicMsg(returnStatus1,msg,FUNCTION_NAME);
	return returnStatus1;
    }
    
    if (gotData == PGS_TRUE )
    {
	/* data is available for the current spacecraft; save the value of the
	   spacecraft tag so the data isn't reloaded unnecessarily in subsequent
	   calls to this function */

	spacecraft = spacecraftTag;

	/* Also save the Endianness of Binary files for the subsequent calls 
	   to this function with the same spacecraft name */

	lendcheck_old = lendcheck;

#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_EPHspacecraft[masterTSFIndex] = spacecraft;
#endif          

    } /* We loaded ephemeris data and, if requested, attitude data; now find
         the appropriate records and interpolate for final results */
    
    gotData = PGS_FALSE;

/* The algorithm for search is changed, so that we do not waste time stepping
   through all of the totalEphemRecords for each user-requested time */

    ephRecordCounter = -1; /* if it stays -1 search failed */
    endRecord = totalEphemRecords;  /* too big on purpose; will not be used
                                       as such */
    midRecord = (endRecord + ephRecordCounter)/2;

    while(endRecord > ephRecordCounter + 1)
    {
       midRecord = (endRecord + ephRecordCounter)/2;
       if(secTAI93 == ephemRecordArray[midRecord].secTAI93)
       {
          ephRecordCounter = midRecord;
          gotData = PGS_TRUE;
          break;
       }
       if(secTAI93 > ephemRecordArray[midRecord].secTAI93)
       {
          ephRecordCounter = midRecord;
       }
       else
       {
          endRecord   =  midRecord;
       }
      gotData = (ephRecordCounter >=0 &&
                  ephRecordCounter <= totalEphemRecords - 1);
    }  /* end while */
    if (gotData == PGS_TRUE)
    {
            memcpy((char *) &ephemRecord2,
                   (char *)&ephemRecordArray[ephRecordCounter],
                   sizeof(PGSt_ephemRecord));
	    if(ephRecordCounter > 0)
	      {
		memcpy((char *) &ephemRecord1,
		       (char *)&ephemRecordArray[ephRecordCounter-1],
		       sizeof(PGSt_ephemRecord));
	      }
    }

     if (gotData != PGS_TRUE)
    {
       returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
       sprintf(details,
       "search for data record within ephemeris file whose header"
                        " time range included this time has failed");
       PGS_SMF_SetDynamicMsg(returnStatus, details, FUNCTION_NAME);

       PGS_MEM_Free(sortedIndex);
       return returnStatus;
    }

    if (attFlag == PGS_TRUE)
    {

       gotData = PGS_FALSE;

/* The algorithm for search is changed, so that we do not waste time stepping
   through all of the totalAttitRecords for each user-requested time */

       attRecordCounter = -1;
       endRecord = totalAttitRecords;  /* too big on purpose; will not be used
                                          as such */
       midRecord = (endRecord + attRecordCounter)/2;
  
       while(endRecord > attRecordCounter + 1)
       {
          midRecord = (endRecord + attRecordCounter)/2;
          if(secTAI93 == attitRecordArray[midRecord].secTAI93)
          {
             attRecordCounter = midRecord;
             gotData = PGS_TRUE;
             break;
          }
          if(secTAI93 > attitRecordArray[midRecord].secTAI93)
          {
             attRecordCounter = midRecord;
          }
          else
          {
             endRecord   =  midRecord;
          }
          gotData = (attRecordCounter >=0 && 
		     attRecordCounter <= totalAttitRecords - 1);
       }  /* end while */

       if (gotData == PGS_TRUE)
       {
	 if(attRecordCounter >=0 )
	   {
	     memcpy((char *) &attitRecord2,
		    (char *) &attitRecordArray[attRecordCounter],
		    sizeof(PGSt_attitRecord));
	     if(attRecordCounter >0 )
	       {
		 memcpy((char *) &attitRecord1,
			(char *) &attitRecordArray[attRecordCounter-1],
			sizeof(PGSt_attitRecord));
	       }
	     gotData = PGS_TRUE;
	   }
       }

      if (gotData != PGS_TRUE)
      {
          returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
          sprintf(details,
          "search for data record within attitude file whose header"
                           " time range included this time has failed");
          PGS_SMF_SetDynamicMsg(returnStatus, details, FUNCTION_NAME);
  
           PGS_MEM_Free(sortedIndex);
           return returnStatus;
       }
    } /* END if attFlag TRUE */

    /* This ends the phase of loading records; now search for the right ones
       for interpolation */

    /* Step through the requested times retrieving s/c records.  Note that
       "a s/c record" is really two records if a requested time is not
       coincident with a time recorded in the s/c ephemeris file.  This is of
       course necessary in order to interpolate the data to the requested time.
       If no s/c record can be found for a requested time fill the record at
       that time with an "error value". */

    for (cnt--;cnt<maxValues;cnt++)
    {
	/* The current time is the reference time plus the current offset,
	   unless numValues is 0 in which case the current time is just the
	   reference time itself--to which it has been initialize above. */

	if (numValues)
	{
	    secTAI93 = startTAI93 + offsets[sortedIndex[cnt]];
	}

	/*If the current time falls within bounds of the most recently obtained
	   record go to the interpolation code... */

	if (!(secTAI93 >= ephemRecord1.secTAI93 && 
	      secTAI93 < ephemRecord2.secTAI93))
	{
	    /* ...otherwise get the next record. */

	    /*Search ephemRecordArray for the next record (starting at the last
	       place (time) a record was found in the array). */

	    gotData = PGS_FALSE;
	    for (;ephRecordCounter<totalEphemRecords;ephRecordCounter++)
	      if (secTAI93 < ephemRecordArray[ephRecordCounter].secTAI93)
	      {
		if(ephRecordCounter >=0 )
		  {
		    memcpy((char*)&ephemRecord2,
			   (char*)&ephemRecordArray[ephRecordCounter],
			   sizeof(PGSt_ephemRecord));
		    if(ephRecordCounter > 0 )
		      {
			memcpy((char*)&ephemRecord1,
			       (char*)&ephemRecordArray[ephRecordCounter-1],
			       sizeof(PGSt_ephemRecord));
		      }
		    gotData = PGS_TRUE;
		    break;
		  }
	      }	    

	    /* If no record was found in ephemRecordArray in the above search,
	       check the last record in the array (the above will miss the last
	       record) to see if the requested time exactly corresponds to the
	       last time in the array. */

	    if (gotData == PGS_FALSE)
	      if (secTAI93 == ephemRecordArray[totalEphemRecords-1].secTAI93)
	      {
		if(ephRecordCounter >=0 )
		  {
		    if(ephRecordCounter > 0 )
		      {
			memcpy((char*)&ephemRecord2,
			       (char*)&ephemRecordArray[ephRecordCounter-1],
			       sizeof(PGSt_ephemRecord));
			memcpy((char*)&ephemRecord1,
			       (char*)&ephemRecordArray[ephRecordCounter-1],
			       sizeof(PGSt_ephemRecord));
			gotData = PGS_TRUE;
		      }
		  }
	      }

	    /* If the appropriate record STILL has not been found in
	       ephemRecordArray, call PGS_EPH_getEphemRecords to read the next
	       block of spacecraft ephemeris data from disk. */

	    if (gotData == PGS_FALSE)
	    {
		returnStatus1 = PGS_EPH_getEphemRecords(&scTagInfo, secTAI93,
							BLOCK_SIZE,
							ephemRecordArray,
							&totalEphemRecords,
                                                        &lendcheck);
#ifdef _PGS_THREADSAFE
                /* Reset global */
                PGSg_TSF_EPHtotalEphemRecords[masterTSFIndex] = totalEphemRecords;
#endif
		switch (returnStatus1)
		{
		  case PGS_S_SUCCESS:
		  case PGSEPH_M_SHORT_ARRAY:
		    beginTAI93eph = ephemRecordArray[0].secTAI93;
		    endTAI93eph =ephemRecordArray[totalEphemRecords-1].secTAI93;
#ifdef _PGS_THREADSAFE
                    /* Reset globals */
                    PGSg_TSF_EPHbeginTAI93eph[masterTSFIndex] = beginTAI93eph;
                    PGSg_TSF_EPHendTAI93eph[masterTSFIndex] = endTAI93eph;   
#endif
		    for (ephRecordCounter=1;
			 ephRecordCounter<totalEphemRecords;
			 ephRecordCounter++)
		    {
			if (secTAI93 < 
			    ephemRecordArray[ephRecordCounter].secTAI93)
			{
			    memcpy((char*)&ephemRecord2,
				   (char*)&ephemRecordArray[ephRecordCounter],
				   sizeof(PGSt_ephemRecord));

			    memcpy((char*)&ephemRecord1,
				   (char*)&ephemRecordArray[ephRecordCounter-1],
				   sizeof(PGSt_ephemRecord));

			    gotData = PGS_TRUE;
			    break;
			}
		    }
		    
		    if (gotData == PGS_TRUE)
			break;
		    if (secTAI93 == 
			ephemRecordArray[totalEphemRecords-1].secTAI93)
		    {
		      if(ephRecordCounter > 0 )
			{
			  memcpy((char *) &ephemRecord2,
				 (char *) &ephemRecordArray[ephRecordCounter-1],
				 sizeof(PGSt_ephemRecord));
			  
			  memcpy((char *) &ephemRecord1,
				 (char *) &ephemRecordArray[ephRecordCounter-1],
				 sizeof(PGSt_ephemRecord));
			  
			  gotData = PGS_TRUE;
			  break;
			}
		    }
		    returnStatus1 = PGS_E_TOOLKIT;
		  case PGSEPH_E_BAD_EPHEM_FILE_HDR:
		  case PGSEPH_E_NO_SC_EPHEM_FILE:
		  case PGS_E_TOOLKIT:

		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if (code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    if (returnStatus == PGS_S_SUCCESS)
		      strcpy(details,msg);
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
		    /* We need to lock all shared memory management */
		    retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		    if (PGS_SMF_TestErrorLevel(retLock))
		      {
			return PGSTSF_E_GENERAL_FAILURE;
		      }
#endif

		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
		    /* unlock all shared memory management */
		    retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    continue;
		  default:
		    PGS_SMF_SetUnknownMsg(returnStatus1,
					  FUNCTION_NAME);
		    returnStatus1 = PGS_E_TOOLKIT;
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
		    /* We need to lock all shared memory management */
		    retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		    if (PGS_SMF_TestErrorLevel(retLock))
		      {
			return PGSTSF_E_GENERAL_FAILURE;
		      }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
		    /* unlock all shared memory management */
		    retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    continue;
		} /* end of switch statement */
	    } /* END of gotData == FALSE */
	} /* end the if! block */

	/* If the current time falls within bounds of the most recently obtained
	   record go to the interpolation code... */

	if ( attFlag == PGS_TRUE &&
	     (! (secTAI93 >= attitRecord1.secTAI93 && 
		secTAI93 < attitRecord2.secTAI93)))
	{
	    /* ...otherwise get the next record. */

	    /* Search attitRecordArray for the next record (starting at 
	       the last place (time) a record was found in the array). */

	    gotData = PGS_FALSE;
	    for (;attRecordCounter<totalAttitRecords;attRecordCounter++)
	      if (secTAI93 < attitRecordArray[attRecordCounter].secTAI93)
	      {
		if(attRecordCounter >=0 )
		  {
		    memcpy((char*)&attitRecord2,
			   (char*)&attitRecordArray[attRecordCounter],
			   sizeof(PGSt_attitRecord));
		    if(attRecordCounter >0 )
		      {
			memcpy((char*)&attitRecord1,
			       (char*)&attitRecordArray[attRecordCounter-1],
			       sizeof(PGSt_attitRecord));
		      }
		    gotData = PGS_TRUE;
		    break;
		  }
	      }

	    /* If no record was found in attitRecordArray in the above search,
	       check the last record in the array (the above will miss the last
	       record) to see if the requested time exactly corresponds to the
	       last time in the array. */

	    if (gotData == PGS_FALSE)
	      if (secTAI93 == attitRecordArray[totalAttitRecords-1].secTAI93)
	      {
		if(attRecordCounter >0 )
		  {
		    memcpy((char*)&attitRecord2,
			   (char*)&attitRecordArray[attRecordCounter-1],
			   sizeof(PGSt_attitRecord));
		    memcpy((char*)&attitRecord1,
			   (char*)&attitRecordArray[attRecordCounter-1],
			   sizeof(PGSt_attitRecord));
		    gotData = PGS_TRUE;
		  }
	      }

	    /* If the appropriate record STILL has not been found in
	       attitRecordArray, call PGS_EPH_getAttitRecords to read the next
	       block of spacecraft Attitude data from disk. */

	    if (gotData == PGS_FALSE)
	    {
		returnStatus1 = PGS_EPH_getAttitRecords(&scTagInfo, 
							&lendcheck, secTAI93,
							BLOCK_SIZE,
							attitRecordArray,
							&totalAttitRecords);
#ifdef _PGS_THREADSAFE
                /* Reset global */
                PGSg_TSF_EPHtotalAttitRecords[masterTSFIndex] = totalAttitRecords;
#endif
		switch (returnStatus1)
		{
		  case PGS_S_SUCCESS:
		  case PGSEPH_M_SHORT_ARRAY:
		    beginTAI93att = attitRecordArray[0].secTAI93;
		    endTAI93att = attitRecordArray[totalAttitRecords-1].secTAI93;

#ifdef _PGS_THREADSAFE
                    /* Reset globals */
                    PGSg_TSF_EPHbeginTAI93att[masterTSFIndex] = beginTAI93att;
                    PGSg_TSF_EPHendTAI93att[masterTSFIndex] = endTAI93att;
#endif
		    for (attRecordCounter=1;
			 attRecordCounter<totalAttitRecords;
			 attRecordCounter++)
		    {
			if (secTAI93 <
			    attitRecordArray[attRecordCounter].secTAI93)
			{
			    memcpy((char*)&attitRecord2,
				   (char*)&attitRecordArray[attRecordCounter],
				   sizeof(PGSt_attitRecord));
			    memcpy((char*)&attitRecord1,
				   (char*)&attitRecordArray[attRecordCounter-1],
				   sizeof(PGSt_attitRecord));
			    gotData = PGS_TRUE;
			    break;
			}
		    }
		    
		    if (gotData == PGS_TRUE)
			break;
		    if (secTAI93 == 
			attitRecordArray[totalAttitRecords-1].secTAI93)
		    {
		      if(attRecordCounter >0 )
			{
			  memcpy((char *) &attitRecord2,
				 (char *) &attitRecordArray[attRecordCounter-1],
				 sizeof(PGSt_attitRecord));
			  
			  memcpy((char *) &attitRecord1,
				 (char *) &attitRecordArray[attRecordCounter-1],
				 sizeof(PGSt_attitRecord));
			  
			  gotData = PGS_TRUE;
			  break;
			}
		    }
		    returnStatus1 = PGS_E_TOOLKIT;
		  case PGSEPH_E_BAD_EPHEM_FILE_HDR:
		  case PGSEPH_E_NO_SC_EPHEM_FILE:
		  case PGS_E_TOOLKIT:

		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if (code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    if (returnStatus == PGS_S_SUCCESS)
		      strcpy(details,msg);
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
		    /* We need to lock all shared memory management */
		    retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		    if (PGS_SMF_TestErrorLevel(retLock))
		      {
			return PGSTSF_E_GENERAL_FAILURE;
		      }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
		    /* unlock all shared memory management */
		    retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    continue;
		  default:
		    PGS_SMF_SetUnknownMsg(returnStatus1,
					  FUNCTION_NAME);
		    returnStatus1 = PGS_E_TOOLKIT;
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
		    /* We need to lock all shared memory management */
		    retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		    if (PGS_SMF_TestErrorLevel(retLock))
		      {
			return PGSTSF_E_GENERAL_FAILURE;
		      }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
		    /* unlock all shared memory management */
		    retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    continue;
		}
	    }
	}

	/* Get the current position and velocity--interpolating if necessary.
	   The pos. and vel. must be determined regardless of whether or not
	   the orbFlag has been set to PGS_TRUE since they are needed to
	   determine the attitQuat.  Therefore at this point the values are
	   being assigned to temporary variables. */

	if (secTAI93 == ephemRecord1.secTAI93)
	{
	    for (count=0;count<3;count++)
	    {
		tempPosition[count] = ephemRecord1.position[count];
		tempVelocity[count] = ephemRecord1.velocity[count];
		qualityFlags[sortedIndex[cnt]][0] = 
		  (PGSt_integer) ephemRecord1.qualityFlag;
	    }
	}
	else
	{
	    /* if the time interval between bounding ephemeris records is too
	       large, don't interpolate, consider this condition an error
	       (i.e. no data available at the requested time) */

	    if ((ephemRecord2.secTAI93 - ephemRecord1.secTAI93) >
		MAX_EPH_TIME_INTERVAL)
	    {
		if (returnStatus == PGS_S_SUCCESS)
		{
		    sprintf(details,"%s%s%.2f%s",
			    "interpolation error for some requested values, ",
			    "time between actual ephemeris records is too "
			    "large (> ", MAX_EPH_TIME_INTERVAL, " seconds).");
		}
		numBadPoints++;
#ifdef _PGS_THREADSAFE
		/* We need to lock all shared memory management */
		retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		if (PGS_SMF_TestErrorLevel(retLock))
		  {
		    return PGSTSF_E_GENERAL_FAILURE;
		  }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
		/* unlock all shared memory management */
		retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		continue;
	    }
	    
	    returnStatus1 = PGS_EPH_interpolatePosVel(ephemRecord1.secTAI93,
						      ephemRecord1.position,
						      ephemRecord1.velocity,
						      ephemRecord2.secTAI93,
						      ephemRecord2.position,
						      ephemRecord2.velocity,
						      secTAI93,
						      tempPosition,
						      tempVelocity);
	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
		if (returnStatus == PGS_S_SUCCESS)
		{
		    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
		    PGS_SMF_GetMsg(&code,mnemonic,details);
		}
		numBadPoints++;
#ifdef _PGS_THREADSAFE
		/* We need to lock all shared memory management */
		retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		if (PGS_SMF_TestErrorLevel(retLock))
		  {
		    return PGSTSF_E_GENERAL_FAILURE;
		  }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
		/* unlock all shared memory management */
		retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		continue;
	    }
	    qualityFlags[sortedIndex[cnt]][0]= (PGSt_integer)
	                                       (ephemRecord1.qualityFlag |
						ephemRecord2.qualityFlag |
						PGSd_INTERPOLATED_POINT);
	}

	/* If attitude data has been requested get the current Euler angles and
	   their respective rates--interpolating if necessary.  Then determine
	   the s/c to ECI quaternion. */

	if (attFlag == PGS_TRUE)
	{
	    if (secTAI93 == attitRecord1.secTAI93)
	    {
		for (count=0;count<3;count++)
		{
		    eulerAngle[sortedIndex[cnt]][count] = 
			attitRecord1.eulerAngle[count];
		    angularVelocity[sortedIndex[cnt]][count] = 
			attitRecord1.angularVelocity[count];
		}
		qualityFlags[sortedIndex[cnt]][1] =
		  (PGSt_integer) attitRecord1.qualityFlag;
	    }
	    else  /* interpolate Euler angles and angular rates */
	    {
		/* if the time interval between bounding attitude records is
		   too large, don't interpolate, consider this condition an
		   error (i.e. no data available at the requested time) */
		
		if ((attitRecord2.secTAI93 - attitRecord1.secTAI93) >
		    MAX_ATT_TIME_INTERVAL)
		{
		    if (returnStatus == PGS_S_SUCCESS)
		    {
		      sprintf(details,"%s%.2f%s",
			      "interpolation error for some requested values,"
			      " time between actual attitude records is too "
			      "large (> ", MAX_ATT_TIME_INTERVAL,
			      " seconds).");
		    }
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
		    /* We need to lock all shared memory management */
		    retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		    if (PGS_SMF_TestErrorLevel(retLock))
		      {
			return PGSTSF_E_GENERAL_FAILURE;
		      }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
		    /* unlock all shared memory management */
		    retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    continue;
		}
		
		returnStatus1 = 
		  PGS_EPH_interpolateAttitude(attitRecord1.secTAI93,
					      attitRecord1.eulerAngle,
					      attitRecord1.angularVelocity,
					      attitRecord2.secTAI93,
					      attitRecord2.eulerAngle,
					      attitRecord2.angularVelocity,
					      scTagInfo.eulerAngleOrder,
					      secTAI93,
					      eulerAngle[sortedIndex[cnt]],
					      angularVelocity[sortedIndex[cnt]]
					      );
		
		if (returnStatus1 != PGS_S_SUCCESS)
		{
		    if (returnStatus == PGS_S_SUCCESS)
		    {
			PGS_SMF_SetUnknownMsg(returnStatus1,
					      FUNCTION_NAME);
			PGS_SMF_GetMsg(&code,mnemonic,details);
		    }
		    numBadPoints++;
#ifdef _PGS_THREADSAFE
		    /* We need to lock all shared memory management */
		    retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		    if (PGS_SMF_TestErrorLevel(retLock))
		      {
			return PGSTSF_E_GENERAL_FAILURE;
		      }
#endif
		    PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					      &returnStatus);
#ifdef _PGS_THREADSAFE
		    /* unlock all shared memory management */
		    retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		    continue;
		}

		qualityFlags[sortedIndex[cnt]][1] = (PGSt_integer)
                                                    (attitRecord1.qualityFlag |
						     attitRecord2.qualityFlag |
						     PGSd_INTERPOLATED_POINT);
            }

	    /* Get the quaternion that gives the equivalent rotation as the s/c
	       Euler angles.  The Euler angle order depends on the s/c.  The
	       quaternion quatEuler can be used to rotate a vector from the s/c
	       reference frame to the orbital reference frame. */

	    returnStatus1 = PGS_CSC_EulerToQuat(eulerAngle[sortedIndex[cnt]],
						scTagInfo.eulerAngleOrder,
						quatEuler);
	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
		if (returnStatus == PGS_S_SUCCESS)
		{
		    PGS_SMF_SetUnknownMsg(returnStatus1,
					  FUNCTION_NAME);
		    PGS_SMF_GetMsg(&code,mnemonic,details);
		}
		numBadPoints++;
#ifdef _PGS_THREADSAFE
		/* We need to lock all shared memory management */
		retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		if (PGS_SMF_TestErrorLevel(retLock))
		  {
		    return PGSTSF_E_GENERAL_FAILURE;
		  }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
		/* unlock all shared memory management */
		retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		continue;
	    }

	    /* get the orbital to ECI reference frame quaternion */
	    
	    returnStatus1 = PGS_CSC_getORBtoECIquat(tempPosition,
						    tempVelocity,
						    quatORBtoECI);
	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
	      if (returnStatus == PGS_S_SUCCESS)
		{
		    PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
		    PGS_SMF_GetMsg(&code,mnemonic,details);
		}
		numBadPoints++;
#ifdef _PGS_THREADSAFE
		/* We need to lock all shared memory management */
		retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		if (PGS_SMF_TestErrorLevel(retLock))
		  {
		    return PGSTSF_E_GENERAL_FAILURE;
		  }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt],&scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
		/* unlock all shared memory management */
		retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		continue;
	    }

	    returnStatus1 = PGS_CSC_quatMultiply(quatORBtoECI, quatEuler,
						 attitQuat[sortedIndex[cnt]]);

	    /* Check the return status from the last operation any of the cases
	       above. */

	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
	      if (returnStatus == PGS_S_SUCCESS)
		{
		    PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME);
		    PGS_SMF_GetMsg(&code, mnemonic, details);
		}
		numBadPoints++;
#ifdef _PGS_THREADSAFE
		/* We need to lock all shared memory management */
		retLock = PGS_TSF_LockIt(PGSd_TSF_EPHLOCK);
		if (PGS_SMF_TestErrorLevel(retLock))
		  {
		    return PGSTSF_E_GENERAL_FAILURE;
		  }
#endif
		PGS_EPH_setBadEphemRecord(sortedIndex[cnt], &scData,
					  &returnStatus);
#ifdef _PGS_THREADSAFE
		/* unlock all shared memory management */
		retLock = PGS_TSF_UnlockIt(PGSd_TSF_EPHLOCK);
#endif          
		continue;
	    }
	} /* END: if (attitudeFlag == PGS_TRUE) */

	/* finally if the orbit flag is set to PGS_TRUE write the position and
	   velocity data into their respective arrays */

	if (orbFlag == PGS_TRUE)
	{
	    memcpy((char *) positionECI[sortedIndex[cnt]],(char *) tempPosition,
		   sizeof(PGSt_double)*3);
	    memcpy((char *) velocityECI[sortedIndex[cnt]],(char *) tempVelocity,
		   sizeof(PGSt_double)*3);
	}

    }

    PGS_MEM_Free(sortedIndex);
    
    if (numBadPoints >= maxValues)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
    }

    PGS_SMF_SetDynamicMsg(returnStatus,details,FUNCTION_NAME);
}
    /* To get ephemeris and/or attitude data from HDF files */
    if (hdfcheck == 1)
    {
        if (attFlag == PGS_TRUE)
         {
            attData = 1;
         }
	number = (numValues) ? numValues : 1;

	for ( j=0; j< number; j++)
	{
	   if (numValues == 0)
	   /*offsets[j]=0.0;*/
           secTAI93 = startTAI93;
            else 
            secTAI93 = startTAI93 + offsets[sortedIndex[j]]; 
           /*secTAI93 = startTAI93 + offsets[j];*/

	returnStatus = PGS_EPH_checkHDF(spacecraftTag,  secTAI93, attData,
		qua, pos, vel, eul, ang, atti);

	   if (returnStatus != PGS_S_SUCCESS)
	   {
              PGS_SMF_SetDynamicMsg(returnStatus,
                              " error in accessing spacecraft "
                              "file(s)", FUNCTION_NAME);
              return returnStatus;
	   }
	   else
	   {
        	if (orbFlag == PGS_TRUE)
        	{
		   for (count=0; count<3; count++)
		   {
			positionECI[j][count] = pos[count];
			velocityECI[j][count] = vel[count];
                   }
		   qualityFlags[j][0] = qua[0];
		}
                if (attFlag == PGS_TRUE)
                {
		   for (count=0; count<3; count++)
		   {
		      eulerAngle[j][count] = eul[count];
		      angularVelocity[j][count] = ang[count];     
		   }
		   qualityFlags[j][1] = qua[1];
		   for (count=0; count<4; count++)
	           {
		      attitQuat[j][count] = atti[count];
		   }
       	        }
	    }
	}
   }
   return returnStatus;
}

/* the following function sets the values of the elements of bad records
   (i.e. records that could not be determined) to PGSd_GEO_ERROR_VALUE */

static void
PGS_EPH_setBadEphemRecord(
    PGSt_integer    recordNum,
    PGSt_scData     *scData,
    PGSt_SMF_status *returnStatus)
{
    short           cntr;             /* loop counter */

    if (scData->orbFlag == PGS_TRUE)
    {	
	scData->qFlags[recordNum][0] = PGSd_NO_DATA | 0x1;
	for (cntr=0;cntr<3;cntr++)
	{
	    scData->positionECI[recordNum][cntr] = PGSd_GEO_ERROR_VALUE;
	    scData->velocityECI[recordNum][cntr] = PGSd_GEO_ERROR_VALUE;
	}
    }
    
    if (scData->attFlag == PGS_TRUE)
    {
	scData->qFlags[recordNum][1] = PGSd_NO_DATA | 0x1;	
	for (cntr=0;cntr<3;cntr++)
	{
	    scData->eulerAngle[recordNum][cntr] = PGSd_GEO_ERROR_VALUE;
	    scData->angularVelocity[recordNum][cntr] = PGSd_GEO_ERROR_VALUE;
	    scData->attitQuat[recordNum][cntr] = PGSd_GEO_ERROR_VALUE;
	}
	scData->attitQuat[recordNum][3] = PGSd_GEO_ERROR_VALUE;
    }
    if (*returnStatus == PGS_S_SUCCESS)
      *returnStatus = PGSEPH_W_BAD_EPHEM_VALUE;
    return;
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

void Tget_scTag(char *temp_sc_string, PGSt_tag *temp_spacecraftTag)
{

     if(!strcmp(temp_sc_string,"PGSd_EOS_AM"))
        *temp_spacecraftTag=PGSd_EOS_AM;
     else if(!strcmp(temp_sc_string,"EOSAM1"))
	*temp_spacecraftTag=PGSd_EOS_AM;
     else if(!strcmp(temp_sc_string,"EOSAURA"))
        *temp_spacecraftTag=PGSd_EOS_AURA;
     else if(!strcmp(temp_sc_string,"EOSPM1"))
	*temp_spacecraftTag=PGSd_EOS_PM_GIIS;
     else if(!strcmp(temp_sc_string,"PGSd_EOS_PM_GIIS"))
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

/* To get ephemeris and/or attitude data from HDF files */
static PGSt_SMF_status  
PGS_EPH_checkHDF(PGSt_tag spacecraftTag, 
		 PGSt_double secTAI93,  
		 PGSt_integer attData,
		 PGSt_integer qua[2], 
		 PGSt_double pos[3], 
		 PGSt_double vel[3], 
		 PGSt_double eul[3], 
		 PGSt_double ang[3], 
		 PGSt_double atti[4])
     
{
  const int DpCPrMaxScIdLen          = 24;
  const int DpCPrMaxTimeRangeLen     = 48;
  const int DpCPrMaxSourceLen        = 32;
  const int DpCPrMaxVersionLen       = 8;
  const int DpCPrMaxFrameLen         = 8;
  const int DpCPrMaxKepler           = 6;
  const int DpCPrMaxEulerAngleOrder  = 3;
  const int DpCPrMaxQaParameters     = 16;
  const int DpCPrMaxQaStatistics     = 4;
  const int DpCPrMaxUrLen            = 256;
  /*const int DpCPrMaxUrs              = 10;*/
  
  PGSt_integer 	         num_files, num, number;
  PGSt_integer	         file_num=1;
  PGSt_integer	         files_version_num=1;
  char 		         hdfFile[256], hdfFileAtt[256], 
                         hdfFile2[256], hdfFileAtt2[256];
  PGSt_SMF_status        pgsStatus;
  int32  		 hdfId = -1, hdfIdAtt = -1, 
                         hdfId2 = -1, hdfIdAtt2 = -1;

  int32  		 vDataRef, vDataRefAtt;
  int32  		 vDataId, vDataIdAtt;
  int32  		 interlace, vDataSize;
  int32  		 interlaceAtt, vDataSizeAtt;
  char   		 fields[512], fieldsAtt[512];
  char   		 vDataName[64], vDataNameAtt[64];
  unsigned char	         *hdfData=NULL, *hdfDataAtt=NULL;
  unsigned char	         *pntr;
  static char  		 hdfScId[24];
  static char  		 hdfTimeRange[48];
  static char  		 hdfSource[32];
  static char  		 hdfVersion[8];
  static double		 hdfStartTai;
  static double		 hdfEndTai;
  static float 		 hdfInterval;
  static int   		 hdfNUrs;
  static int 	         hdfNRecords;
  static int   		 hdfEulerAngleOrder[3];
  static int   		 hdfNOrbits;
  static int   		 hdfOrbitStart;
  static int   		 hdfOrbitEnd;
  static char  		 hdfRefFrame[8];
  static double		 keplerElements[6];
  static double		 keplerEpochTai;
  static double 	 hdfQaParams[16];
  static double 	 hdfQaStats[4];
  static PGSt_integer    NUM;
  double 		 hdfPeriod;
  PGSt_double 	         hdfDescProp;
  int    		 hdfFddReplace;
  char   		 hdfURs[10][256];
  PGSt_integer           iElm;
  PGSt_SMF_status        returnStatus = PGS_S_SUCCESS;
  PGSt_SMF_status        returnStatus1;
  PGSt_tag               hdfspacecraftTag;
  int32  	         ephNRecs=0, attNRecs=0;
  PGSt_integer           recordCount = 0;
  PGSt_double            tempPosVel[6];  
  PGSt_double            *tempPosition;
  PGSt_double            *tempVelocity;
  PGSt_integer           count;
  PGSt_boolean           gotData = PGS_FALSE;  
  PGSt_integer           ephRecordCounter=0; 
  PGSt_integer           attRecordCounter=0;
  PGSt_integer           midRecord =0;   
  PGSt_integer           endRecord=0; 
  PGSt_ephemRecord       ephemRecord1;   
  PGSt_ephemRecord       ephemRecord2; 
  PGSt_attitRecord       attitRecord1;
  PGSt_attitRecord       attitRecord2;
  PGSt_integer           counter;
  PGSt_integer           i,j;
  /*
    PGSt_double          position[3]; 
    PGSt_double          velocity[3];
    PGSt_double          eulerAngle[3];
  */
  PGSt_double            quatEuler[4]; 
  PGSt_double            quatORBtoECI[4]; 
  PGSt_integer           sizeint, sizechar, sizeflt, sizedbl, 
                         sizeuint, size_att_eph_struct;
  PGSt_integer           att_interp_err  = 0, eph_interp_err  = 0;
  char                   dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
  PGSt_integer           filecount, file_info_exist = 0;
  PGSt_integer           base;
  PGSt_integer           next;
  PGSt_integer           sort_buf_file_version_num;
  double                 sort_buf_startTAI93;
  double                 sort_buf_stopTAI93;


  static  PGSt_integer   totalRecords=0, totalRecordsatt=0, totalRecordseph=0;
  static  PGSt_integer   totaleph=0, totalatt=0, total=0;

  static  PGSt_ephemRecord hdfRecord[BLOCK_SIZE_HDF];
  static  PGSt_attitRecord hdfRecordAtt[BLOCK_SIZE_HDF];
  
  static PGSt_double     beginTAI93eph2=1.0e50;         /* first TAI time in
							    ephemRecordArray*/
  static PGSt_double     endTAI93eph2=-1.0e50;          /* last TAI time in
							    ephemRecordArray*/
  static PGSt_double     beginTAI93att2=1.0e50;         /* first TAI time in
							    attitRecordArray*/
  static PGSt_double     endTAI93att2=-1.0e50;          /* last TAI time in
							    attitRecordArray*/
  static PGSt_double     hdfEndTaieph = 0;
  static PGSt_double     hdfEndTaiatt= 0;
  static PGSt_tag        spacecraft2=0;                 /* spacecraftTag */

  PGSt_ephemfiles        eph_files_info;
  PGSt_attitfiles        att_files_info;


  ephemRecord1.secTAI93 = 1.0e50;   
  ephemRecord2.secTAI93 = -1.0e50; 
  attitRecord1.secTAI93 = 1.0e50;
  attitRecord2.secTAI93 = -1.0e50;

  eph_files_info.num_files = 0;
  att_files_info.num_files = 0;
  eph_files_info.spacecraftTag[0] = -1;
  att_files_info.spacecraftTag[0] = -1;

  sizeflt = sizeof(PGSt_real);
  sizedbl = sizeof(PGSt_double);
  sizechar = sizeof(char);
  sizeint = sizeof(int32);
  sizeuint = sizeof(PGSt_uinteger);
  size_att_eph_struct =   (7*sizedbl + sizeuint);


  tempPosition = tempPosVel;
  tempVelocity = tempPosVel+3;
  
  if ( secTAI93 < beginTAI93eph2 || 
       secTAI93 > endTAI93eph2 || 
       spacecraftTag != spacecraft2)
    {
      PGS_PC_GetNumberOfFiles(PGSd_SC_EPHHDF_DATA, &num_files);
      for ( num = 1; num <= num_files; num++)
	{
	  file_num = num;
	  files_version_num = num;
	  pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&file_num, hdfFile );
	  
	  if (pgsStatus != PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetDynamicMsg(pgsStatus,
				    "Error getting reference to HDF format files", 
				    FUNCTION_NAME2);
	      return pgsStatus;
	    }
	  
	  if(file_exists(hdfFile))
	    {
	      /* Open the HDF format file */
	      hdfId = Hopen( hdfFile, DFACC_RDONLY, 0 );
	      /* initialize the Vset interface */
	      Vstart( hdfId );
	      /* Get the reference number of the first Vdata in the file */
	      vDataRef = -1;
	      vDataRef = VSgetid( hdfId, vDataRef );
	      /* Attact to the first Vdata in read mode */
	      vDataId = VSattach( hdfId, vDataRef, "r" );
	      /* Get the list of field names */
	      VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
			 fields, (int32 *)&vDataSize, vDataName );
	      /* Name the fields to be read */
	      (void) VSsetfields( vDataId, fields );
	      
	      hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+100*DpCPrMaxUrLen*sizechar+2*sizedbl+sizeint);
	      if (hdfData == NULL)
		{
		  /*ERROR callocing*/
		  sprintf(dynamicMsg, "Error allocating memory for "
			  "hdfData");
		  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					FUNCTION_NAME2);
		  if(vDataId != -1){
		  VSdetach( vDataId );
		  vDataId = -1;
		  }
		  if(hdfId != -1){
		  Vend( hdfId );
		  Hclose( hdfId );
		  hdfId = -1;
		  }
		  return(PGSMEM_E_NO_MEMORY);
		}
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
	      memmove( &hdfNUrs, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfNRecords, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfNOrbits, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfOrbitStart, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfOrbitEnd, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfRefFrame, pntr, DpCPrMaxFrameLen*sizechar );
	      pntr += DpCPrMaxFrameLen*sizechar;
	      memmove( &keplerElements[0], pntr, 
		       DpCPrMaxKepler*sizedbl );
	      pntr += DpCPrMaxKepler*sizedbl;
	      memmove( &keplerEpochTai, pntr, sizedbl );
	      pntr += sizedbl;
	      memmove( &hdfQaParams[0], pntr, 
		       DpCPrMaxQaParameters*sizeflt );
	      pntr += DpCPrMaxQaParameters*sizeflt;
	      memmove( &hdfQaStats[0], pntr, 
		       DpCPrMaxQaStatistics*sizeflt );
	      pntr += DpCPrMaxQaStatistics*sizeflt;
	      memmove( &hdfPeriod, pntr, sizedbl );
	      pntr += sizedbl;
	      memmove( &hdfDescProp, pntr, sizedbl );
	      pntr += sizedbl;
	      memmove( &hdfFddReplace, pntr, sizeuint );
	      pntr += sizeuint;
	      counter = (hdfNUrs) ? hdfNUrs : 1;
	      for (iElm=0; iElm<counter; iElm++)
		{
		  memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
		  pntr += DpCPrMaxUrLen*sizechar;
		}
	      
	      /* Detach from the Vdata */
	      if(vDataId != -1){
		VSdetach( vDataId );
		vDataId = -1;
	      }
	      if (hdfData != NULL){
	      free(hdfData);
	      hdfData = NULL;
	      }
	      
	      Tget_scTag(hdfScId, &hdfspacecraftTag);
	      
	      /* before doing anything else put summry of this file in 
		 eph_files_info structure */
	      if( hdfspacecraftTag == spacecraftTag && eph_files_info.num_files >= 1)
		{
		  file_info_exist = 0;
		  for( filecount= 0; filecount < eph_files_info.num_files; filecount++)
		    {
		      if(eph_files_info.fileversion[filecount] == files_version_num)
			{
			  file_info_exist = 1;
			  break;
			}
		    }
		  if(file_info_exist != 1)
		    {
		      eph_files_info.num_files += 1;
		      eph_files_info.fileversion[eph_files_info.num_files-1] = files_version_num;
		      eph_files_info.spacecraftTag[eph_files_info.num_files-1] = hdfspacecraftTag;
		      eph_files_info.startTime[eph_files_info.num_files-1] = hdfStartTai;
		      eph_files_info.endTime[eph_files_info.num_files-1] = hdfEndTai;
		    }
		}
	      else if( hdfspacecraftTag == spacecraftTag)
		{
		  eph_files_info.num_files += 1;
		  eph_files_info.fileversion[eph_files_info.num_files-1] = files_version_num;
		  eph_files_info.spacecraftTag[eph_files_info.num_files-1] = hdfspacecraftTag;
		  eph_files_info.startTime[eph_files_info.num_files-1] = hdfStartTai;
		  eph_files_info.endTime[eph_files_info.num_files-1] = hdfEndTai;
		}

	      if ( hdfspacecraftTag == spacecraftTag && 
		   secTAI93 >= hdfStartTai && secTAI93 < hdfEndTai)
		{
		  /* Get the reference number of the Vdata */
		  vDataRef = VSgetid( hdfId, vDataRef );
		  /* Attach to the first Vdata in read mode */
		  vDataId = VSattach( hdfId, vDataRef, "r" );
		  /* Get the list of field names */
		  VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
			     fields, (int32 *)&vDataSize, vDataName );
		  /* Name the fields to be read */
		  (void) VSsetfields( vDataId, fields );
		  hdfData = (unsigned char *)malloc( sizeof(PGSt_ephemRecord)+1);
		  if (hdfData == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfData");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					    FUNCTION_NAME2);
		      if(vDataId != -1){
			VSdetach( vDataId );
			vDataId = -1;
		      }
		      if(hdfId != -1){
			Vend( hdfId );
			Hclose( hdfId );
			hdfId = -1;
		      }
		      return(PGSMEM_E_NO_MEMORY);
		    }

		  /* To avoid core dump or memory corruption we should 
		     limit ephNRecs to BLOCK_SIZE_HDF
		  */
		  if(ephNRecs > BLOCK_SIZE_HDF) ephNRecs = BLOCK_SIZE_HDF;

		  totalRecords += ephNRecs;
		  recordCount   =  -1;
		  
		  /* NOTE: for 24hr data files this while loop is the most 
		     time consuming part of the code. If user only wants to
		     extract data for a few offsest and if 2hr files are used
		     (instead of 24 hr files)  then we read much less ammount 
		     of data to the buffer and proccess much less amount of 
		     data.
		  */

		  while ( recordCount++ < ephNRecs )
		    {
		      /* Read a packed Vdata record */
		      (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
		      /* Unpack the packed Vdata record */

		      memmove( &hdfRecord[recordCount], hdfData,size_att_eph_struct);
		      /*
		      pntr = hdfData;
		      memmove( &hdfRecord[recordCount].secTAI93,    pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].position[0], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].position[1], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].position[2], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].velocity[0], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].velocity[1], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].velocity[2], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[recordCount].qualityFlag, pntr, sizeuint );
		      */
		    } 
		  /* Detach from the Vdata */
		  if(vDataId != -1){
		    VSdetach( vDataId );
		    vDataId = -1;
		  }
		  if (hdfData != NULL){
		    free(hdfData);
		    hdfData = NULL;
		  }
		  count=0;
		  totalRecords=ephNRecs;
		  total=ephNRecs;
		  totaleph =ephNRecs;
		  totalRecordseph = ephNRecs;
		  beginTAI93eph2 = hdfRecord[0].secTAI93;
		  endTAI93eph2 = hdfEndTai;
		  hdfEndTaieph = hdfEndTai;
		  spacecraft2 = hdfspacecraftTag;
		  NUM = num;
		  gotData = PGS_TRUE;
		  if(secTAI93 > hdfRecord[totalRecords-1].secTAI93 && 
		     secTAI93 < hdfEndTaieph && 
		     (totalRecords + GAP_COVER_BLKS) < BLOCK_SIZE_HDF)
		    {
		      number = num+count+1;
		      files_version_num = number;
		      pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&number, hdfFile2 );
		      
		      if (pgsStatus == PGS_S_SUCCESS)
			{
			  
			  if(file_exists(hdfFile2))
			    {
			      /* Open the HDF format file */
			      hdfId2 = Hopen( hdfFile2, DFACC_RDONLY, 0 );
			      /* initialize the Vset interface */
			      Vstart( hdfId2 );
			      /* Get the reference number of the first Vdata in the file */
			      vDataRef = -1;
			      vDataRef = VSgetid( hdfId2, vDataRef );
			      /* Attact to the first Vdata in read mode */
			      vDataId = VSattach( hdfId2, vDataRef, "r" );
			      /* Get the list of field names */
			      VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
					 fields, (int32 *)&vDataSize, vDataName );
			      /* Name the fields to be read */
			      (void) VSsetfields( vDataId, fields );
			      
			      hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+100*DpCPrMaxUrLen*sizechar+2*sizedbl+sizeint);
			      
			      if (hdfData == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg, "Error allocating memory for "
					  "hdfData");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							dynamicMsg, 
							FUNCTION_NAME2);
				  
				  if(vDataId != -1){
				    VSdetach( vDataId );
				    vDataId = -1;
				  }
				  if(hdfId2 != -1){
				    Vend( hdfId2 );
				    Hclose( hdfId2 );
				    hdfId2 = -1;
				  }
				  return(PGSMEM_E_NO_MEMORY);
				}
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
			      memmove( &hdfNUrs, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfNRecords, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfNOrbits, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfOrbitStart, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfOrbitEnd, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfRefFrame, pntr, DpCPrMaxFrameLen*sizechar );
			      pntr += DpCPrMaxFrameLen*sizechar;
			      memmove( &keplerElements[0], pntr, DpCPrMaxKepler*sizedbl );
			      pntr += DpCPrMaxKepler*sizedbl;
			      memmove( &keplerEpochTai, pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfQaParams[0], pntr, DpCPrMaxQaParameters*sizeflt );
			      pntr += DpCPrMaxQaParameters*sizeflt;
			      memmove( &hdfQaStats[0], pntr, DpCPrMaxQaStatistics*sizeflt );
			      pntr += DpCPrMaxQaStatistics*sizeflt;
			      memmove( &hdfPeriod, pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfDescProp, pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfFddReplace, pntr, sizeuint );
			      pntr += sizeuint;
			      counter = (hdfNUrs) ? hdfNUrs : 1;
			      for (iElm=0; iElm<counter; iElm++)
				{
				  memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
				  pntr += DpCPrMaxUrLen*sizechar;
				}
			      
			      /* Detach from the Vdata */
			      if(vDataId != -1){
				VSdetach( vDataId );
				vDataId = -1;
			      }
			      
			      if (hdfData != NULL){
				free(hdfData);
				hdfData = NULL;
			      }

			      Tget_scTag(hdfScId, &hdfspacecraftTag);
			      
			      /* before doing anything else put summry of this file in 
				 eph_files_info structure */
			      if( hdfspacecraftTag == spacecraftTag && eph_files_info.num_files >= 1)
				{
				  file_info_exist = 0;
				  for( filecount= 0; filecount < eph_files_info.num_files; filecount++)
				    {
				      if(eph_files_info.fileversion[filecount] == files_version_num)
					{
					  file_info_exist = 1;
					  break;
					}
				    }
				  if(file_info_exist != 1)
				    {
				      eph_files_info.num_files += 1;
				      eph_files_info.fileversion[eph_files_info.num_files-1] = files_version_num;
				      eph_files_info.spacecraftTag[eph_files_info.num_files-1] = hdfspacecraftTag;
				      eph_files_info.startTime[eph_files_info.num_files-1] = hdfStartTai;
				      eph_files_info.endTime[eph_files_info.num_files-1] = hdfEndTai;
				    }
				}
			      else if( hdfspacecraftTag == spacecraftTag)
				{
				  eph_files_info.num_files += 1;
				  eph_files_info.fileversion[eph_files_info.num_files-1] = files_version_num;
				  eph_files_info.spacecraftTag[eph_files_info.num_files-1] = hdfspacecraftTag;
				  eph_files_info.startTime[eph_files_info.num_files-1] = hdfStartTai;
				  eph_files_info.endTime[eph_files_info.num_files-1] = hdfEndTai;
				}
			      
			      if ( hdfspacecraftTag == spacecraftTag )
				{
				  ephNRecs = 0;
				  recordCount = 0;
				  /* Get the reference number of teh Vdata */
				  vDataRef = VSgetid( hdfId2, vDataRef );
				  /* Attach to the first Vdata in read mode */
				  vDataId = VSattach( hdfId2, vDataRef, "r" );
				  /* Get the list of field names */
				  VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
					     fields, (int32 *)&vDataSize, vDataName );
				  /* Name the fields to be read */
				  (void) VSsetfields( vDataId, fields );
				  hdfData = (unsigned char *)malloc( sizeof(PGSt_ephemRecord));
				  
				  if (hdfData == NULL)
				    {
				      /*ERROR callocing*/
				      sprintf(dynamicMsg, "Error allocating memory for "
					      "hdfData");
				      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
							    FUNCTION_NAME2);

				      if(vDataId != -1){
					VSdetach( vDataId );
					vDataId = -1;
				      }
				      if(hdfId2 != -1){
					Vend( hdfId2 );
					Hclose( hdfId2 );
					hdfId2 = -1;
				      }
				      return(PGSMEM_E_NO_MEMORY);
				    }
				  
				  recordCount = -1;
				  while ( recordCount++ < GAP_COVER_BLKS )
				    {
				      /* Read a packed Vdata record */
				      (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
				      /* Unpack the packed Vdata record */
				  
				      memmove( &hdfRecord[totalRecords+recordCount], hdfData,size_att_eph_struct);
				      /*
				      pntr = hdfData;
				      memmove( &hdfRecord[totalRecords+recordCount].secTAI93,    pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].position[0], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].position[1], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].position[2], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].velocity[0], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].velocity[1], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].velocity[2], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecord[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
				      */
				    }
				  total = totalRecords+recordCount-1;
				  totalRecords += GAP_COVER_BLKS;
				  
				  /* Detach from the Vdata */
				  if(vDataId != -1){
				  VSdetach( vDataId );
				  vDataId = -1;
				  }
				  if(hdfData != NULL){
				  free(hdfData);
				  hdfData = NULL;
				  }
				} /* end check spacecrafttag */
			      /* Close the Vset interface */
			      if(hdfId2 != -1){
			      Vend( hdfId2 );
			      /* Close the HDF format file */
			      Hclose( hdfId2 );
			      hdfId2 = -1;
			      }
			    } /* if(file_exists(hdfFile2)) */
			} /* end if (pgsStatus == SUCCESS) */
		      gotData = PGS_TRUE;
		      break;
		      /* beginTAI93eph2 =  hdfRecord[0].secTAI93;*/
		      /*  endTAI93eph2 = hdfEndTai;*/
		      /*  spacecraft2 = hdfspacecraftTag;*/ 
		    }/* end if totalRecords .. */ 
		}/* end check spacecraft Tag */
	      else if(hdfspacecraftTag == spacecraftTag &&
		      (hdfStartTai - secTAI93) > 0 && 
		      (hdfStartTai - secTAI93) < MAX_EPH_TIME_INTERVAL && 
		      (hdfStartTai - endTAI93eph2) < MAX_EPH_TIME_INTERVAL && 
		      (hdfRecord[0].secTAI93 < secTAI93 ) &&
		      (hdfRecord[totalRecords-1].secTAI93 < secTAI93 ) &&
		      ((secTAI93 - hdfRecord[totalRecords-1].secTAI93 ) < 
		       MAX_EPH_TIME_INTERVAL) &&
		      (totalRecords + GAP_COVER_BLKS) < BLOCK_SIZE_HDF)
		{
		  /* Get the reference number or the Vdata */
		  vDataRef = VSgetid( hdfId, vDataRef );
		  /* Attach to the first Vdata in read mode */
		  vDataId = VSattach( hdfId, vDataRef, "r" );
		  /* Get the list of field names */
		  VSinquire( vDataId, (int32 *)&ephNRecs,
			     (int32 *)&interlace,
			     fields, (int32 *)&vDataSize, vDataName );
		  /* Name the fields to be read */
		  (void) VSsetfields( vDataId, fields );
		  hdfData = (unsigned char *)malloc(sizeof(PGSt_ephemRecord)+1);
		  if (hdfData == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfData");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
					    FUNCTION_NAME2);
		      if(vDataId != -1){
		      VSdetach( vDataId );
		      vDataId = -1;
		      }
		      if(hdfId != -1){
		      Vend( hdfId );
		      Hclose( hdfId );
		      hdfId = -1;
		      }
		      
		      return(PGSMEM_E_NO_MEMORY);
		    }

		  recordCount         =  -1;
		  
		  while ( recordCount++ < GAP_COVER_BLKS )
		    {
		      /* Read a packed Vdata record */
		      (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
		      /* Unpack the packed Vdata record */
		      
		      memmove( &hdfRecord[totalRecords+recordCount], hdfData,size_att_eph_struct);
		      /*
		      pntr = hdfData;
		      memmove( &hdfRecord[totalRecords+recordCount].secTAI93,    pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].position[0], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].position[1], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].position[2], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].velocity[0], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].velocity[1], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].velocity[2], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
		      */
		    } /* end while */
		  /* Detach from the Vdata */
		  total = totalRecords+recordCount-1;
		  totalRecords +=  GAP_COVER_BLKS;
		  totaleph += GAP_COVER_BLKS;
		  totalRecordseph += GAP_COVER_BLKS;

		  if(vDataId != -1){
		    VSdetach( vDataId );
		    vDataId = -1;
		  }
		  if (hdfData != NULL){
		    free(hdfData);
		    hdfData = NULL;
		  }

		  count=0;
		  /*
		    beginTAI93eph2 = hdfRecord[0].secTAI93;
		    endTAI93eph2 = hdfEndTai;
		    hdfEndTaieph = hdfEndTai;
		    spacecraft2 = hdfspacecraftTag;
		  */
		  gotData = PGS_TRUE;
		}
	      else
		{
		  if((totalRecords + GAP_COVER_BLKS) > BLOCK_SIZE_HDF )
		    {
		      PGS_SMF_SetDynamicMsg(returnStatus,
					    " error: static memory allocated for "
					    "hdfRecord is not enough", 
					    FUNCTION_NAME2);
		      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		    }
		  
		  if(num == num_files)
		    {
		      PGS_SMF_SetDynamicMsg(returnStatus,
					    " error in accessing spacecraft "
					    "file(s)", FUNCTION_NAME2);
		      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		    }
		}
	      /* Close the Vset interface */
	      if(hdfId != -1){
		Vend( hdfId );
		/* Close the HDF format file */
		Hclose( hdfId );
		hdfId = -1;
	      }
	      if( gotData == PGS_TRUE)
		{
		  break;
		}
	    } /* if(file_exists(hdfFile)) */
	  else
	    {
	      if(num == num_files)
		{
		  PGS_SMF_SetDynamicMsg(returnStatus,
					" error in accessing spacecraft "
					"file(s)", FUNCTION_NAME2);
		  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		}
	    }
	} /* end for file_num=0 */
    } /* end if secTAI93< */
  if ( gotData== PGS_FALSE )
    {
      total = totaleph;
    }
  if ( secTAI93 > hdfRecord[total-1].secTAI93 && 
       secTAI93 < hdfEndTaieph && gotData == PGS_FALSE )
    {
      count = 0;
      number = NUM+count+1;
      files_version_num = number;
      pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&number, hdfFile2 );
      
      if (pgsStatus == PGS_S_SUCCESS)
	{
	  
	  if(file_exists(hdfFile2))
	    {
	      /* Open the HDF format file */
	      hdfId2 = Hopen( hdfFile2, DFACC_RDONLY, 0 );
	      /* initialize the Vset interface */
	      Vstart( hdfId2 );
	      /* Get the reference number of the first Vdata in the file */
	      vDataRef = -1;
	      vDataRef = VSgetid( hdfId2, vDataRef );
	      
	      /* Attact to the first Vdata in read mode */
	      vDataId = VSattach( hdfId2, vDataRef, "r" );
	      /* Get the list of field names */
	      VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
			 fields, (int32 *)&vDataSize, vDataName );
	      /* Name the fields to be read */
	      (void) VSsetfields( vDataId, fields );
	      
	      hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+100*DpCPrMaxUrLen*sizechar+2*sizedbl+sizeint);
	      
	      if (hdfData == NULL)
		{
		  /*ERROR callocing*/
		  sprintf(dynamicMsg, "Error allocating memory for "
			  "hdfData");
		  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
					dynamicMsg,
					FUNCTION_NAME2);
		  
		  if(vDataId != -1){
		    VSdetach( vDataId );
		    vDataId = -1;
		  }
		  if(hdfId2 != -1){
		    Vend( hdfId2 );
		    Hclose( hdfId2 );
		    hdfId2 = -1;
		  }
		  return(PGSMEM_E_NO_MEMORY);
		}
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
	      memmove( &hdfNUrs, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfNRecords, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfNOrbits, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfOrbitStart, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfOrbitEnd, pntr, sizeuint );
	      pntr += sizeuint;
	      memmove( &hdfRefFrame, pntr, DpCPrMaxFrameLen*sizechar );
	      pntr += DpCPrMaxFrameLen*sizechar;
	      memmove( &keplerElements[0], pntr, DpCPrMaxKepler*sizedbl );
	      pntr += DpCPrMaxKepler*sizedbl;
	      memmove( &keplerEpochTai, pntr, sizedbl );
	      pntr += sizedbl;
	      memmove( &hdfQaParams[0], pntr, DpCPrMaxQaParameters*sizeflt );
	      pntr += DpCPrMaxQaParameters*sizeflt;
	      memmove( &hdfQaStats[0], pntr, DpCPrMaxQaStatistics*sizeflt );
	      pntr += DpCPrMaxQaStatistics*sizeflt;
	      memmove( &hdfPeriod, pntr, sizedbl );
	      pntr += sizedbl;
	      memmove( &hdfDescProp, pntr, sizedbl );
	      pntr += sizedbl;
	      memmove( &hdfFddReplace, pntr, sizeuint );
	      pntr += sizeuint;
	      counter = (hdfNUrs) ? hdfNUrs : 1;
	      for (iElm=0; iElm<counter; iElm++)
		{
		  memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
		  pntr += DpCPrMaxUrLen*sizechar;
		}
	      /* Detach from the Vdata */
	      if(vDataId != -1){
		VSdetach( vDataId );
		vDataId = -1;
	      }
	      if (hdfData != NULL){
		free(hdfData);
		hdfData = NULL;
	      }

	      Tget_scTag(hdfScId, &hdfspacecraftTag);
	      
	      /* before doing anything else put summry of this file in 
		 eph_files_info structure */
	      if( hdfspacecraftTag == spacecraftTag && eph_files_info.num_files >= 1)
		{
		  file_info_exist = 0;
		  for( filecount= 0; filecount < eph_files_info.num_files; filecount++)
		    {
		      if(eph_files_info.fileversion[filecount] == files_version_num)
			{
			  file_info_exist = 1;
			  break;
			}
		    }
		  if(file_info_exist != 1)
		    {
		      eph_files_info.num_files += 1;
		      eph_files_info.fileversion[eph_files_info.num_files-1] = files_version_num;
		      eph_files_info.spacecraftTag[eph_files_info.num_files-1] = hdfspacecraftTag;
		      eph_files_info.startTime[eph_files_info.num_files-1] = hdfStartTai;
		      eph_files_info.endTime[eph_files_info.num_files-1] = hdfEndTai;
		    }
		}
	      else if( hdfspacecraftTag == spacecraftTag)
		{
		  eph_files_info.num_files += 1;
		  eph_files_info.fileversion[eph_files_info.num_files-1] = files_version_num;
		  eph_files_info.spacecraftTag[eph_files_info.num_files-1] = hdfspacecraftTag;
		  eph_files_info.startTime[eph_files_info.num_files-1] = hdfStartTai;
		  eph_files_info.endTime[eph_files_info.num_files-1] = hdfEndTai;
		}

	      if ( hdfspacecraftTag == spacecraftTag )
		{
		  ephNRecs = 0;
		  recordCount = 0;
		  /* Get the reference number of teh Vdata */
		  vDataRef = VSgetid( hdfId2, vDataRef );
		  /* Attach to the first Vdata in read mode */
		  vDataId = VSattach( hdfId2, vDataRef, "r" );
		  /* Get the list of field names */
		  VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
			     fields, (int32 *)&vDataSize, vDataName );
		  /* Name the fields to be read */
		  (void) VSsetfields( vDataId, fields );
		  hdfData = (unsigned char *)malloc( sizeof(PGSt_ephemRecord));
		  
		  if (hdfData == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfData");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, FUNCTION_NAME2);
		      if(vDataId != -1){
			VSdetach( vDataId );
			vDataId = -1;
		      }
		      if(hdfId2 != -1){
			Vend( hdfId2 );
			Hclose( hdfId2 );
			hdfId2 = -1;
		      }
		      return(PGSMEM_E_NO_MEMORY);
		    }
		  
		  recordCount = -1;
		  while ( recordCount++ < GAP_COVER_BLKS )
		    {
		      /* Read a packed Vdata record */
		      (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
		      /* Unpack the packed Vdata record */

		      memmove( &hdfRecord[totalRecords+recordCount], hdfData,size_att_eph_struct);
		      /*
		      pntr = hdfData;
		      memmove( &hdfRecord[totalRecords+recordCount].secTAI93,    pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].position[0], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].position[1], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].position[2], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].velocity[0], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].velocity[1], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].velocity[2], pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfRecord[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
		      */
		    }
		  total = totalRecords+recordCount-1;
		  totalRecords += GAP_COVER_BLKS;
		  
		  /* Detach from the Vdata */
		  if(vDataId != -1){
		    VSdetach( vDataId );
		    vDataId = -1;
		  }
		  if (hdfData != NULL){
		    free(hdfData);
		    hdfData = NULL;
		  }

		} /* end check spacecrafttag */
	      /* Close the Vset interface */
	      if(hdfId2 != -1){
		Vend( hdfId2 );
		Hclose( hdfId2 );
		hdfId2 = -1;
	      }
	    } /* if(file_exists(hdfFile2)) */
	} /*end if pgsStatus */
    }
  
  /*If we got here without getting gotData = PGS_TRUE, then the time may
    have fallen in the gaps between files, which can happen for real DPREP 
    data, where  there are gaps in recordes (i.e. missing records). If this
    happens look into the eph_files_info structure to see if the time is 
    between end time of one file and start time of a consecutive file(with 
    a time which is not 60 sec more than the end time of previous file). 
    If yes, open both and read them to buffer (from second file we just 
    need to get a few records) */ 
  
  if(gotData == PGS_FALSE)
    {
      /* First sort */
      for (base=0; base<eph_files_info.num_files; base++)
	{
	  /*  Get next earliest element */
	  for (i=base, next=base; i<eph_files_info.num_files; i++) 
	    {
	      if (eph_files_info.startTime[i] <
		  eph_files_info.startTime[next])
		{
		  next=i;
		}
	    }
	  
	  /* 
	   *    Swap into position, if needed
	   */
	  if (next != base)
	    {
	      sort_buf_file_version_num = eph_files_info.fileversion[base];
	      sort_buf_startTAI93 = eph_files_info.startTime[base];
	      sort_buf_stopTAI93 = eph_files_info.endTime[base];
	      
	      eph_files_info.fileversion[base] =  eph_files_info.fileversion[next];
	      eph_files_info.startTime[base] =  eph_files_info.startTime[next];
	      eph_files_info.endTime[base]  = eph_files_info.endTime[next];
	      
	      eph_files_info.fileversion[next] = sort_buf_file_version_num;
	      eph_files_info.startTime[next] = sort_buf_startTAI93;
	      eph_files_info.endTime[next] = sort_buf_stopTAI93;
	    }
	}

      if( (eph_files_info.num_files > 1) && 
	  (spacecraftTag == eph_files_info.spacecraftTag[0]))
	{
	  for(i=0; i< (eph_files_info.num_files-1); i++)
	    {
	      if( (secTAI93 < eph_files_info.startTime[i+1]) &&
		  (secTAI93 > eph_files_info.endTime[i]) &&
		  (eph_files_info.endTime[i] <= eph_files_info.startTime[i+1]) &&
		  ((eph_files_info.startTime[i+1] - eph_files_info.endTime[i]) < MAX_ATT_TIME_INTERVAL))
		{
		  /* secTAI93 is in the gap of two ephem files. read them */

		  file_num = eph_files_info.fileversion[i];
		  files_version_num = eph_files_info.fileversion[i];
		  pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&file_num, hdfFile );
		  
		  if (pgsStatus != PGS_S_SUCCESS)
		    {
		      PGS_SMF_SetDynamicMsg(pgsStatus,
					    "Error getting reference to HDF format files", 
					    FUNCTION_NAME2);
		      return pgsStatus;
		    }
		  
		  if(file_exists(hdfFile))
		    {
		      /* Open the HDF format file */
		      hdfId = Hopen( hdfFile, DFACC_RDONLY, 0 );
		      /* initialize the Vset interface */
		      Vstart( hdfId );
		      /* Get the reference number of the first Vdata in the file */
		      vDataRef = -1;
		      vDataRef = VSgetid( hdfId, vDataRef );
		      /* Attact to the first Vdata in read mode */
		      vDataId = VSattach( hdfId, vDataRef, "r" );
		      /* Get the list of field names */
		      VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
				 fields, (int32 *)&vDataSize, vDataName );
		      /* Name the fields to be read */
		      (void) VSsetfields( vDataId, fields );
		      
		      hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+100*DpCPrMaxUrLen*sizechar+2*sizedbl+sizeint);
		      if (hdfData == NULL)
			{
			  /*ERROR callocing*/
			  sprintf(dynamicMsg, "Error allocating memory for "
				  "hdfData");
			  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
						FUNCTION_NAME2);
			  
			  if(vDataId != -1){
			    VSdetach( vDataId );
			    vDataId = -1;
			  }
			  if(hdfId != -1){
			    Vend( hdfId );
			    Hclose( hdfId );
			    hdfId = -1;
			  }
			  return(PGSMEM_E_NO_MEMORY);
			}
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
		      memmove( &hdfNUrs, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfNRecords, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfNOrbits, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfOrbitStart, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfOrbitEnd, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfRefFrame, pntr, DpCPrMaxFrameLen*sizechar );
		      pntr += DpCPrMaxFrameLen*sizechar;
		      memmove( &keplerElements[0], pntr, 
			       DpCPrMaxKepler*sizedbl );
		      pntr += DpCPrMaxKepler*sizedbl;
		      memmove( &keplerEpochTai, pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfQaParams[0], pntr, 
			       DpCPrMaxQaParameters*sizeflt );
		      pntr += DpCPrMaxQaParameters*sizeflt;
		      memmove( &hdfQaStats[0], pntr, 
			       DpCPrMaxQaStatistics*sizeflt );
		      pntr += DpCPrMaxQaStatistics*sizeflt;
		      memmove( &hdfPeriod, pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfDescProp, pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfFddReplace, pntr, sizeuint );
		      pntr += sizeuint;
		      counter = (hdfNUrs) ? hdfNUrs : 1;
		      for (iElm=0; iElm<counter; iElm++)
			{
			  memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
			  pntr += DpCPrMaxUrLen*sizechar;
			}
		      
		      /* Detach from the Vdata */
		      if(vDataId != -1){
			VSdetach( vDataId );
			vDataId = -1;
		      }
		      if (hdfData != NULL){
			free(hdfData);
			hdfData = NULL;
		      }
		      Tget_scTag(hdfScId, &hdfspacecraftTag);
		      
		      /* Get the reference number of the Vdata */
		      vDataRef = VSgetid( hdfId, vDataRef );
		      /* Attach to the first Vdata in read mode */
		      vDataId = VSattach( hdfId, vDataRef, "r" );
		      /* Get the list of field names */
		      VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
				 fields, (int32 *)&vDataSize, vDataName );
		      /* Name the fields to be read */
		      (void) VSsetfields( vDataId, fields );
		      hdfData = (unsigned char *)malloc( sizeof(PGSt_ephemRecord)+1);
		      if (hdfData == NULL)
			{
			  /*ERROR callocing*/
			  sprintf(dynamicMsg, "Error allocating memory for "
				  "hdfData");
			  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
						FUNCTION_NAME2);
			  
			  if(vDataId != -1){
			    VSdetach( vDataId );
			    vDataId = -1;
			  }
			  if(hdfId != -1){
			    Vend( hdfId );
			    Hclose( hdfId );
			    hdfId = -1;
			  }
			  return(PGSMEM_E_NO_MEMORY);
			}

		      
		      /* To avoid core dump or memory corruption we should 
			 limit ephNRecs to BLOCK_SIZE_HDF
		      */
		      if(ephNRecs > BLOCK_SIZE_HDF) ephNRecs = BLOCK_SIZE_HDF;

		      totalRecords += ephNRecs;
		      recordCount   =  -1;
		      
		      while ( recordCount++ < ephNRecs )
			{
			  /* Read a packed Vdata record */
			  (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
			  /* Unpack the packed Vdata record */

			  memmove( &hdfRecord[recordCount], hdfData,size_att_eph_struct);
			  /*
         		  pntr = hdfData;
			  memmove( &hdfRecord[recordCount].secTAI93,    pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].position[0], pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].position[1], pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].position[2], pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].velocity[0], pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].velocity[1], pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].velocity[2], pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfRecord[recordCount].qualityFlag, pntr, sizeuint );
			  */
			} 
		      /* Detach from the Vdata */
		      if(vDataId != -1){
			VSdetach( vDataId );
			vDataId = -1;
		      }
		      if (hdfData != NULL){
			free(hdfData);
			hdfData = NULL;
		      }

		      count=0;
		      totalRecords=ephNRecs;
		      total=ephNRecs;
		      totaleph =ephNRecs;
		      totalRecordseph = ephNRecs;
		      beginTAI93eph2 = hdfRecord[0].secTAI93;
		      endTAI93eph2 = hdfEndTai;
		      hdfEndTaieph = hdfEndTai;
		      spacecraft2 = hdfspacecraftTag;
		      NUM = num;
		      if(hdfId != -1){
			Vend( hdfId );
			/* Close the HDF format file */
			Hclose( hdfId );
			hdfId = -1;
		      }
		    }   /* if(file_exists(hdfFile)) */
		  
		  /* open following file and read a few records from it */
		  
		  number = eph_files_info.fileversion[i+1];
		  files_version_num = number;
		  pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&number, hdfFile2 );
		  
		  if (pgsStatus == PGS_S_SUCCESS)
		    {
		      
		      if(file_exists(hdfFile2))
			{
			  /* Open the HDF format file */
			  hdfId2 = Hopen( hdfFile2, DFACC_RDONLY, 0 );
			  /* initialize the Vset interface */
			  Vstart( hdfId2 );
			  /* Get the reference number of the first Vdata in the file */
			  vDataRef = -1;
			  vDataRef = VSgetid( hdfId2, vDataRef );
			  /* Attact to the first Vdata in read mode */
			  vDataId = VSattach( hdfId2, vDataRef, "r" );
			  /* Get the list of field names */
			  VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
				     fields, (int32 *)&vDataSize, vDataName );
			  /* Name the fields to be read */
			  (void) VSsetfields( vDataId, fields );
			  
			  hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+100*DpCPrMaxUrLen*sizechar+2*sizedbl+sizeint);
			  
			  if (hdfData == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg, "Error allocating memory for "
				      "hdfData");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
						    dynamicMsg, 
						    FUNCTION_NAME2);
			      
			      if(vDataId != -1){
				VSdetach( vDataId );
				vDataId = -1;
			      }
			      if(hdfId2 != -1){
				Vend( hdfId2 );
				Hclose( hdfId2 );
				hdfId2 = -1;
			      }
			      return(PGSMEM_E_NO_MEMORY);
			    }
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
			  memmove( &hdfNUrs, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfNRecords, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfNOrbits, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfOrbitStart, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfOrbitEnd, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfRefFrame, pntr, DpCPrMaxFrameLen*sizechar );
			  pntr += DpCPrMaxFrameLen*sizechar;
			  memmove( &keplerElements[0], pntr, DpCPrMaxKepler*sizedbl );
			  pntr += DpCPrMaxKepler*sizedbl;
			  memmove( &keplerEpochTai, pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfQaParams[0], pntr, DpCPrMaxQaParameters*sizeflt );
			  pntr += DpCPrMaxQaParameters*sizeflt;
			  memmove( &hdfQaStats[0], pntr, DpCPrMaxQaStatistics*sizeflt );
			  pntr += DpCPrMaxQaStatistics*sizeflt;
			  memmove( &hdfPeriod, pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfDescProp, pntr, sizedbl );
			  pntr += sizedbl;
			  memmove( &hdfFddReplace, pntr, sizeuint );
			  pntr += sizeuint;
			  counter = (hdfNUrs) ? hdfNUrs : 1;
			  for (iElm=0; iElm<counter; iElm++)
			    {
			      memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
			      pntr += DpCPrMaxUrLen*sizechar;
			    }
			  
			  /* Detach from the Vdata */
			  if(vDataId != -1){
			    VSdetach( vDataId );
			    vDataId = -1;
			  }
			  if (hdfData != NULL){
			    free(hdfData);
			    hdfData = NULL;
			  }

			  Tget_scTag(hdfScId, &hdfspacecraftTag);
			  
			  ephNRecs = 0;
			  recordCount = 0;
			  /* Get the reference number of teh Vdata */
			  vDataRef = VSgetid( hdfId2, vDataRef );
			  /* Attach to the first Vdata in read mode */
			  vDataId = VSattach( hdfId2, vDataRef, "r" );
			  /* Get the list of field names */
			  VSinquire( vDataId, (int32 *)&ephNRecs, (int32 *)&interlace,
				     fields, (int32 *)&vDataSize, vDataName );
			  /* Name the fields to be read */
			  (void) VSsetfields( vDataId, fields );
			  hdfData = (unsigned char *)malloc( sizeof(PGSt_ephemRecord));
			  
			  if (hdfData == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg, "Error allocating memory for "
				      "hdfData");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
						    FUNCTION_NAME2);
			      
			      if(vDataId != -1){
				VSdetach( vDataId );
				vDataId = -1;
			      }
			      if(hdfId2 != -1){
				Vend( hdfId2 );
				Hclose( hdfId2 );
				hdfId2 = -1;
			      }
			      return(PGSMEM_E_NO_MEMORY);
			    }
			  
			  recordCount = -1;
			  while ( recordCount++ < GAP_COVER_BLKS )
			    {
			      /* Read a packed Vdata record */
			      (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
			      /* Unpack the packed Vdata record */

			      memmove( &hdfRecord[totalRecords+recordCount], hdfData, size_att_eph_struct);
			      /*
			      pntr = hdfData;
			      memmove( &hdfRecord[totalRecords+recordCount].secTAI93,    pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].position[0], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].position[1], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].position[2], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].velocity[0], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].velocity[1], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].velocity[2], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecord[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
			      */
			    }
			  total = totalRecords+recordCount-1;
			  totalRecords += GAP_COVER_BLKS;
			  
			  /* Detach from the Vdata */
			  if(vDataId != -1){
			    VSdetach( vDataId );
			    vDataId = -1;
			  }
			  if (hdfData != NULL){
			    free(hdfData);
			    hdfData = NULL;
			  }
			  if(hdfId2 != -1){
			    /* Close the Vset interface */
			    Vend( hdfId2 );
			    /* Close the HDF format file */
			    Hclose( hdfId2 );
			    hdfId2 = -1;
			  }
			} /* if(file_exists(hdfFile2)) */
		    }
		  gotData = PGS_TRUE;
		}
	      if(gotData == PGS_TRUE) break;
	    }
	}
    }
  
  gotData = PGS_FALSE;
  ephRecordCounter = -1; /* if it stays -1 search failed */
  endRecord = total;  /* too big on purpose; will not be used
			 as such */
  midRecord = (endRecord + ephRecordCounter)/2;
  
  while(endRecord > ephRecordCounter + 1)
    {
      midRecord = (endRecord + ephRecordCounter)/2;
      if(secTAI93 == hdfRecord[midRecord].secTAI93)
	{
	  ephRecordCounter = midRecord;
	  gotData = PGS_TRUE;
	  break;
	}
      if(secTAI93 > hdfRecord[midRecord].secTAI93)
	{
	  ephRecordCounter = midRecord;
	}
      else
	{
	  endRecord   =  midRecord;
	}
      gotData = (ephRecordCounter >=0 &&
		 ephRecordCounter <= total );
    }  /* end while */
  if (gotData == PGS_TRUE)
    {
      memmove((char *) &ephemRecord2,
	      (char *)&hdfRecord[ephRecordCounter],
	      sizeof(PGSt_ephemRecord));
      if(ephRecordCounter > 0 )
	{
	  memmove((char *) &ephemRecord1,
		  (char *)&hdfRecord[ephRecordCounter-1],
		  sizeof(PGSt_ephemRecord));
	}
    }
  else  /* something went wrong */
    {
      returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
      PGS_SMF_SetDynamicMsg(returnStatus, "search for data record within ephemeris file whose header" " time range included this time has failed ", FUNCTION_NAME2);
    }
  
  /* If the current time falls within bounds of the most recently obtained record go to the interpolation code... */
  
  if (!(secTAI93 >= ephemRecord1.secTAI93 &&
	secTAI93 < ephemRecord2.secTAI93))
    {
      
      gotData = PGS_FALSE;
      for (;ephRecordCounter<total;ephRecordCounter++)
	if (secTAI93 < hdfRecord[ephRecordCounter].secTAI93)
	  {
	    memmove((char*)&ephemRecord2,
		    (char*)&hdfRecord[ephRecordCounter],
		    sizeof(PGSt_ephemRecord));
	    if(ephRecordCounter > 0 )
	      {
		memmove((char*)&ephemRecord1,
			(char*)&hdfRecord[ephRecordCounter-1],
			sizeof(PGSt_ephemRecord));
	      }
	    gotData = PGS_TRUE;
	    break;
	  }
      
      
      if (gotData == PGS_FALSE)
	if (secTAI93 == hdfRecord[total-1].secTAI93)
	  {
	    if(ephRecordCounter > 0 )
	      {
		memmove((char*)&ephemRecord2,
			(char*)&hdfRecord[ephRecordCounter-1],
			sizeof(PGSt_ephemRecord));
		memmove((char*)&ephemRecord1,
			(char*)&hdfRecord[ephRecordCounter-1],
			sizeof(PGSt_ephemRecord));
		gotData = PGS_TRUE;
	      }
	  }
      
    }  /* end if (!(secTAI93....)).. */
  
  
  if (secTAI93 == ephemRecord1.secTAI93)
    {
      for (count=0;count<3;count++)
	{
	  tempPosition[count] = ephemRecord1.position[count];
	  tempVelocity[count] = ephemRecord1.velocity[count];
	  qua[0] =
	    (PGSt_integer) ephemRecord1.qualityFlag;
	}
      
    }
  else
    {
      if ((ephemRecord2.secTAI93 - ephemRecord1.secTAI93) >
	  MAX_EPH_TIME_INTERVAL)
	{
	  if (returnStatus == PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetDynamicMsg(returnStatus,"interpolation error for some requested values, " "time between actual ephemeris records is too " "large (> 60 ",FUNCTION_NAME2);
	    }
	}
      returnStatus1 = PGS_EPH_interpolatePosVel(ephemRecord1.secTAI93,
						ephemRecord1.position,
						ephemRecord1.velocity,
						ephemRecord2.secTAI93,
						ephemRecord2.position,
						ephemRecord2.velocity,
						secTAI93,
						tempPosition,
						tempVelocity);
      
      if (returnStatus1 != PGS_S_SUCCESS)
	{
	  if (returnStatus == PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME2);
	      eph_interp_err = -1;
	    }
	}
      if(eph_interp_err != -1)
	{
	  qua[0]= (PGSt_integer)
	    (ephemRecord1.qualityFlag |
	     ephemRecord2.qualityFlag |
	     PGSd_INTERPOLATED_POINT);
	}
      else
	{
	  qua[0]= PGSd_NO_DATA | 0x1;
	}
    } /* end else */
  
  if(eph_interp_err != -1)
    {
      for ( j=0; j< 3; j++)
	{
	  pos[j] = tempPosition[j];
	  vel[j] = tempVelocity[j];
	}
    }
  else
    {
      pos[j] = PGSd_GEO_ERROR_VALUE;
      vel[j] = PGSd_GEO_ERROR_VALUE;
    }
  if (gotData == PGS_TRUE)
    {
      returnStatus = PGS_S_SUCCESS;
      /* Close the Vset interface */
      if(vDataId != -1)
	{
	  VSdetach( vDataId );
	  vDataId = -1;
	}
      if(hdfId != -1)
	{
	  Vend( hdfId );
	  /* Close the HDF format file */
	  Hclose( hdfId );
	  hdfId = -1;
	}
    }
  
  if(vDataId != -1)
    {
      VSdetach( vDataId );
      vDataId = -1;
      Vend( hdfId );
      Hclose( hdfId );
      hdfId = -1;
    }
  if(hdfId != -1)
    {
      Vend( hdfId );
      Hclose( hdfId );
      hdfId = -1;
    }

  
  /* now go on to obtain attitude data */
  gotData = PGS_FALSE;
  if (attData == 1)
    {
      if (secTAI93 < beginTAI93att2 ||
          secTAI93 > endTAI93att2 ||
          spacecraftTag != spacecraft2)
	{
	  PGS_PC_GetNumberOfFiles(PGSd_SC_ATTHDF_DATA, &num_files);
	  
	  for ( num = 1; num <= num_files; num++)
	    {
	      file_num = num;
	      files_version_num = num;
	      pgsStatus = PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA,&file_num,
					      hdfFileAtt );
	      if (pgsStatus != PGS_S_SUCCESS)
		{
		  PGS_SMF_SetDynamicMsg(pgsStatus,
					"Error getting reference to HDF format files",
					FUNCTION_NAME2);
		  return pgsStatus;
		}
	      if(file_exists(hdfFileAtt))
		{
		  /* Open the HDF format file */
		  hdfIdAtt = Hopen( hdfFileAtt, DFACC_RDONLY, 0 );
		  /* initialize the Vset interface */
		  Vstart( hdfIdAtt );
		  /* Get the reference number of the first Vdata in the file */
		  vDataRefAtt = -1;
		  vDataRefAtt = VSgetid( hdfIdAtt, vDataRefAtt );
		  /* Attach to the first Vdata in read mode */
		  vDataIdAtt = VSattach( hdfIdAtt, vDataRefAtt, "r" );
		  /* Get the list of field names */
		  VSinquire( vDataIdAtt, (int32 *)&attNRecs,
			     (int32 *)&interlaceAtt,
			     fieldsAtt, (int32 *)&vDataSizeAtt, vDataNameAtt );
		  /* Name the fields to be read */
		  (void) VSsetfields( vDataIdAtt, fieldsAtt );
		  hdfDataAtt = (unsigned char *)malloc(sizeof(PGSt_attitHeader)+
						       100*DpCPrMaxUrLen*sizechar);
		  
		  if (hdfDataAtt == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfDataAtt");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
					    FUNCTION_NAME2);
		      if(vDataIdAtt != -1){
			VSdetach( vDataIdAtt );
			vDataIdAtt = -1;
		      }
		      if(hdfIdAtt != -1){
			Vend( hdfIdAtt );
			Hclose( hdfIdAtt );
			hdfIdAtt = -1;
		      }
		      return(PGSMEM_E_NO_MEMORY);
		    }
		  /* Read a packed Vdata record */
		  (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
		  /* Unpack the packed Vdata record */
		  pntr = hdfDataAtt;
		  
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
		  memmove( &hdfNUrs, pntr, sizeuint );
		  pntr += sizeuint;
		  memmove( &hdfNRecords, pntr, sizeuint );
		  pntr += sizeuint;
		  memmove( &hdfEulerAngleOrder[0], pntr, DpCPrMaxEulerAngleOrder*sizeuint );
		  pntr += DpCPrMaxEulerAngleOrder*sizeuint;
		  memmove( &hdfQaParams[0], pntr, DpCPrMaxQaParameters*sizeflt );
		  pntr += DpCPrMaxQaParameters*sizeflt;
		  memmove( &hdfQaStats[0], pntr, DpCPrMaxQaStatistics*sizeflt );
		  pntr += DpCPrMaxQaStatistics*sizeflt;
		  counter = (hdfNUrs) ? hdfNUrs : 1;
		  for (iElm=0; iElm<counter; iElm++)
		    {
		      memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
		      pntr += DpCPrMaxUrLen*sizechar;
		    }
		  /* Detach from the Vdata */
		  if(vDataIdAtt != -1){
		    VSdetach( vDataIdAtt );
		    vDataIdAtt = -1;
		  }
		  if (hdfDataAtt != NULL){
		    free(hdfDataAtt);
		    hdfDataAtt = NULL;
		  }

		  Tget_scTag(hdfScId, &hdfspacecraftTag);
		  
		  /* before doing anything else put summry of this file in 
		     att_files_info structure */
		  if(hdfspacecraftTag == spacecraftTag && att_files_info.num_files >= 1)
		    {
		      file_info_exist = 0;
		      for( filecount= 0; filecount < att_files_info.num_files; filecount++)
			{
			  if(att_files_info.fileversion[filecount] == files_version_num)
			    {
			      file_info_exist = 1;
			      break;
			    }
			}
		      if(file_info_exist != 1)
			{
			  att_files_info.num_files += 1;
			  att_files_info.fileversion[att_files_info.num_files-1] = files_version_num;
			  att_files_info.spacecraftTag[att_files_info.num_files-1] = hdfspacecraftTag;
			  att_files_info.startTime[att_files_info.num_files-1] = hdfStartTai;
			  att_files_info.endTime[att_files_info.num_files-1] = hdfEndTai;
			}
		    }
		  else if( hdfspacecraftTag == spacecraftTag)
		    {
		      att_files_info.num_files += 1;
		      att_files_info.fileversion[att_files_info.num_files-1] = files_version_num;
		      att_files_info.spacecraftTag[att_files_info.num_files-1] = hdfspacecraftTag;
		      att_files_info.startTime[att_files_info.num_files-1] = hdfStartTai;
		      att_files_info.endTime[att_files_info.num_files-1] = hdfEndTai;
		    }
		  
		  if ( hdfspacecraftTag == spacecraftTag &&
		       secTAI93 >= hdfStartTai && secTAI93 < hdfEndTai)
		    {
		      /* Get the reference number or the Vdata */
		      vDataRefAtt = VSgetid( hdfIdAtt, vDataRefAtt );
		      /* Attach to the first Vdata in read mode */
		      vDataIdAtt = VSattach( hdfIdAtt, vDataRefAtt, "r" );
		      /* Get the list of field names */
		      VSinquire( vDataIdAtt, (int32 *)&attNRecs,
				 (int32 *)&interlaceAtt,
				 fieldsAtt, (int32 *)&vDataSizeAtt, vDataNameAtt );
		      /* Name the fields to be read */
		      (void) VSsetfields( vDataIdAtt, fieldsAtt );
		      hdfDataAtt = (unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
		      if (hdfDataAtt == NULL)
			{
			  /*ERROR callocing*/
			  sprintf(dynamicMsg, "Error allocating memory for "
				  "hdfDataAtt");
			  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
						FUNCTION_NAME2);
			  if(vDataIdAtt != -1){
			    VSdetach( vDataIdAtt );
			    vDataIdAtt = -1;
			  }
			  if(hdfIdAtt != -1){
			    Vend( hdfIdAtt );
			    Hclose( hdfIdAtt );
			    hdfIdAtt = -1;
			  }
			  return(PGSMEM_E_NO_MEMORY);
			}

		      recordCount         =  -1;

		      /* To avoid core dump or memory corruption we should 
			 limit attNRecs to BLOCK_SIZE_HDF
		      */
		      if(attNRecs > BLOCK_SIZE_HDF) attNRecs = BLOCK_SIZE_HDF;

		      while ( recordCount++ < attNRecs )
			{
			  /* Read a packed Vdata record */
			  (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
			  /* Unpack the packed Vdata record */

			  memmove( &hdfRecordAtt[recordCount], hdfDataAtt, size_att_eph_struct);
			  /*
			    pntr = hdfDataAtt;
			    memmove( &hdfRecordAtt[recordCount].secTAI93,    pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].eulerAngle[0], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].eulerAngle[1], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].eulerAngle[2], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].angularVelocity[0], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].angularVelocity[1], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].angularVelocity[2], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[recordCount].qualityFlag, pntr, sizeuint );
			  */
			} /* end while */
		      /* Detach from the Vdata */
		      if(vDataIdAtt != -1){
			VSdetach( vDataIdAtt );
			vDataIdAtt = -1;
		      }
		      
		      if (hdfDataAtt != NULL){
			free(hdfDataAtt);
			hdfDataAtt = NULL;
		      }

		      count=0;
		      totalRecords=attNRecs;
		      total=attNRecs;
		      totalatt=attNRecs;
		      totalRecordsatt = attNRecs;
		      beginTAI93att2 = hdfRecordAtt[0].secTAI93;
		      endTAI93att2 = hdfEndTai;
		      hdfEndTaiatt = hdfEndTai;
		      spacecraft2 = hdfspacecraftTag;
		      gotData = PGS_TRUE;
		      if(secTAI93 > hdfRecordAtt[totalRecords -1].secTAI93 && 
			 secTAI93 < hdfEndTaiatt && 
			 (totalRecords + GAP_COVER_BLKS)  < BLOCK_SIZE_HDF)
			{
			  number = num+count+1;
			  files_version_num = number;
			  pgsStatus =
			    PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &number,
						hdfFileAtt2 );
			  if (pgsStatus == PGS_S_SUCCESS)
			    {
			      
			      if(file_exists(hdfFileAtt2))
				{
				  /* Open the HDF format file */
				  hdfIdAtt2 = Hopen( hdfFileAtt2, DFACC_RDONLY, 0 );
				  /* initialize the Vset interface */
				  Vstart( hdfIdAtt2 );
				  /* Get the reference number of the first Vdata
				     in the file */
				  vDataRefAtt = -1;
				  vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt );
				  /* Attach to the first Vdata in read mode */
				  vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt,"r");
				  /* Get the list of field names */
				  
				  VSinquire( vDataIdAtt, (int32 *)&attNRecs,
					     (int32 *)&interlaceAtt,
					     fieldsAtt, (int32 *)&vDataSizeAtt,
					     vDataNameAtt );
				  /* Name the fields to be read */
				  (void) VSsetfields( vDataIdAtt, fieldsAtt );
				  hdfDataAtt =
				    (unsigned char *)malloc(sizeof(PGSt_attitHeader)+
							    100*DpCPrMaxUrLen*sizechar);
				  if (hdfDataAtt == NULL)
				    {
				      /*ERROR callocing*/
				      sprintf(dynamicMsg,
					      "Error allocating memory for "
					      "hdfDataAtt");
				      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
							    dynamicMsg,
							    FUNCTION_NAME2);
				      if(vDataIdAtt != -1){
					VSdetach( vDataIdAtt );
					vDataIdAtt = -1;
				      }
				      if(hdfIdAtt2 != -1){
					Vend( hdfIdAtt2 );
					Hclose( hdfIdAtt2 );
					hdfIdAtt2 = -1;
				      }
				      return(PGSMEM_E_NO_MEMORY);
				    }
				  /* Read a packed Vdata record */
				  (void) VSread( vDataIdAtt, hdfDataAtt, 1,
						      FULL_INTERLACE );
				  /* Unpack the packed Vdata record */
				  pntr = hdfDataAtt;
				  memmove( &hdfScId, pntr,
					   DpCPrMaxScIdLen*sizechar );
				  pntr += DpCPrMaxScIdLen*sizechar;
				  memmove( &hdfTimeRange, pntr,
					   DpCPrMaxTimeRangeLen*sizechar );
				  pntr += DpCPrMaxTimeRangeLen*sizechar;
				  memmove( &hdfSource, pntr,
					   DpCPrMaxSourceLen*sizechar );
				  pntr += DpCPrMaxSourceLen*sizechar;
				  memmove( &hdfVersion, pntr,
					   DpCPrMaxVersionLen*sizechar );
				  pntr += DpCPrMaxVersionLen*sizechar;
				  memmove( &hdfStartTai, pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfEndTai, pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfInterval, pntr, sizeflt );
				  
				  pntr += sizeflt;
				  memmove( &hdfNUrs, pntr, sizeuint );
				  pntr += sizeuint;
				  memmove( &hdfNRecords, pntr, sizeuint );
				  pntr += sizeuint;
				  memmove( &hdfEulerAngleOrder[0], pntr,
					   DpCPrMaxEulerAngleOrder*sizeuint );
				  pntr += DpCPrMaxEulerAngleOrder*sizeuint;
				  memmove( &hdfQaParams[0], pntr,
					   DpCPrMaxQaParameters*sizeflt );
				  pntr += DpCPrMaxQaParameters*sizeflt;
				  memmove( &hdfQaStats[0], pntr,
					   DpCPrMaxQaStatistics*sizeflt );
				  pntr += DpCPrMaxQaStatistics*sizeflt;
				  counter = (hdfNUrs) ? hdfNUrs : 1;
				  for (iElm=0; iElm<counter; iElm++)
				    {
				      memmove( &hdfURs[iElm][0], pntr,
					       DpCPrMaxUrLen*sizechar );
				      pntr += DpCPrMaxUrLen*sizechar;
				    }
				  /* Detach from the Vdata */
				  if(vDataIdAtt != -1){
				    VSdetach( vDataIdAtt );
				    vDataIdAtt = -1;
				  }
				  if (hdfDataAtt != NULL){
				    free(hdfDataAtt);
				    hdfDataAtt = NULL;
				  }

				  Tget_scTag(hdfScId, &hdfspacecraftTag);
				  
				  /* before doing anything else put summry of this file in 
				     att_files_info structure */
				  if(hdfspacecraftTag == spacecraftTag && att_files_info.num_files >= 1)
				    {
				      file_info_exist = 0;
				      for( filecount= 0; filecount < att_files_info.num_files; filecount++)
					{
					  if(att_files_info.fileversion[filecount] ==  files_version_num)
					    {
					      file_info_exist = 1;
					      break;
					    }
					}
				      if(file_info_exist != 1)
					{
					  att_files_info.num_files += 1;
					  att_files_info.fileversion[att_files_info.num_files-1] = files_version_num;
					  att_files_info.spacecraftTag[att_files_info.num_files-1] = hdfspacecraftTag;
					  att_files_info.startTime[att_files_info.num_files-1] = hdfStartTai;
					  att_files_info.endTime[att_files_info.num_files-1] = hdfEndTai;
					}
				    }
				  else if( hdfspacecraftTag == spacecraftTag)
				    {
				      att_files_info.num_files += 1;
				      att_files_info.fileversion[att_files_info.num_files-1] = files_version_num;
				      att_files_info.spacecraftTag[att_files_info.num_files-1] = hdfspacecraftTag;
				      att_files_info.startTime[att_files_info.num_files-1] = hdfStartTai;
				      att_files_info.endTime[att_files_info.num_files-1] = hdfEndTai;
				    }
				  
				  
				  if ( hdfspacecraftTag == spacecraftTag )
				    {
				      attNRecs = 0;
				      recordCount = 0;
				      
				      /* Get the reference number or the Vdata */
				      vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt);
				      /* Attach to the first Vdata in read mode */
				      vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt,
							     "r" );
				      /* Get the list of field names */
				      VSinquire( vDataIdAtt, (int32 *)&attNRecs,
						 (int32 *)&interlaceAtt,
						 fieldsAtt, (int32 *)&vDataSizeAtt,
						 vDataNameAtt );
				      /* Name the fields to be read */
				      
				      (void) VSsetfields( vDataIdAtt,
							       fieldsAtt );
				      hdfDataAtt =
					(unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
				      if (hdfDataAtt == NULL)
					{
					  /*ERROR callocing*/
					  sprintf(dynamicMsg,
						  "Error allocating memory for "
						  "hdfDataAtt");
					  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
								dynamicMsg,
								FUNCTION_NAME2);
					  if(vDataIdAtt != -1){
					    VSdetach( vDataIdAtt );
					    vDataIdAtt = -1;
					  }
					  if(hdfIdAtt2 != -1){
					    Vend( hdfIdAtt2 );
					    Hclose( hdfIdAtt2 );
					    hdfIdAtt2 = -1;
					  }
					  return(PGSMEM_E_NO_MEMORY);
					}
				      
				      recordCount =  -1;
				      while ( recordCount++ < GAP_COVER_BLKS )
					{
					  /* Read a packed Vdata record */
					  (void) VSread( vDataIdAtt, hdfDataAtt,
							      1, FULL_INTERLACE );
					  /* Unpack the packed Vdata record */

					  memmove( &hdfRecordAtt[totalRecords+recordCount], hdfDataAtt, size_att_eph_struct);
					  /*
					    pntr = hdfDataAtt;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].secTAI93,
					    pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[0]
					    , pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[1]
					    , pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[2]
					    , pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[0], pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[1], pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[2], pntr, sizedbl );
					    pntr += sizedbl;
					    memmove( &hdfRecordAtt[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
					  */
					} /* end while */
				      total = totalRecords+recordCount-1;
				      totalRecords += GAP_COVER_BLKS;
				      totalatt += GAP_COVER_BLKS;
				      totalRecordsatt += GAP_COVER_BLKS;
				      /* Detach from the Vdata */
				      if(vDataIdAtt != -1){
					VSdetach( vDataIdAtt );
					vDataIdAtt = -1;
				      }
				      if (hdfDataAtt != NULL){
					free(hdfDataAtt);
					hdfDataAtt = NULL;
				      }
				      
				    } /* end check spacrcrafttag */
				  /* Close the Vset interface */
				  if(hdfIdAtt2 != -1){
				    Vend( hdfIdAtt2 );
				    /* Close the HDF format file */
				    Hclose( hdfIdAtt2 );
				    hdfIdAtt2 = -1;
				  }
				  
				} /* end if(file_exists(hdfFileAtt2)) */
			    } /* end if (pgsStatus == SUCCESS) */
			  gotData = PGS_TRUE;
			  break;
			}/* end if totalRecord ... */
		    } /* end check spacecrafttag */
		  else if(hdfspacecraftTag == spacecraftTag &&
			  (hdfStartTai - secTAI93) > 0 && 
			  (hdfStartTai - secTAI93) < MAX_ATT_TIME_INTERVAL && 
			  (hdfStartTai - endTAI93att2) < MAX_ATT_TIME_INTERVAL  && 
			  (hdfRecordAtt[0].secTAI93 < secTAI93 ) &&
			  (hdfRecordAtt[totalRecords-1].secTAI93 < secTAI93 ) &&
			  ((secTAI93 - hdfRecordAtt[totalRecords-1].secTAI93 ) < 
			   MAX_ATT_TIME_INTERVAL) &&
			  (totalRecords + GAP_COVER_BLKS) < BLOCK_SIZE_HDF)
		    {
		      /* Get the reference number or the Vdata */
		      vDataRefAtt = VSgetid( hdfIdAtt, vDataRefAtt );
		      /* Attach to the first Vdata in read mode */
		      vDataIdAtt = VSattach( hdfIdAtt, vDataRefAtt, "r" );
		      /* Get the list of field names */
		      VSinquire( vDataIdAtt, (int32 *)&attNRecs,
				 (int32 *)&interlaceAtt,
				 fieldsAtt, (int32 *)&vDataSizeAtt, vDataNameAtt );
		      /* Name the fields to be read */
		      (void) VSsetfields( vDataIdAtt, fieldsAtt );
		      hdfDataAtt = (unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
		      if (hdfDataAtt == NULL)
			{
			  /*ERROR callocing*/
			  sprintf(dynamicMsg, "Error allocating memory for "
				  "hdfDataAtt");
			  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
						FUNCTION_NAME2);
			  if(vDataIdAtt != -1){
			    VSdetach( vDataIdAtt );
			    vDataIdAtt = -1;
			  }
			  if(hdfIdAtt != -1){
			    Vend( hdfIdAtt );
			    Hclose( hdfIdAtt );
			    hdfIdAtt = -1;
			  }
			  return(PGSMEM_E_NO_MEMORY);
			}

		      recordCount         =  -1;
		      
		      while ( recordCount++ < GAP_COVER_BLKS )
			{
			  /* Read a packed Vdata record */
			  (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
			  /* Unpack the packed Vdata record */

			  memmove( &hdfRecordAtt[totalRecords+recordCount], hdfDataAtt, size_att_eph_struct);
			  /*
			    pntr = hdfDataAtt;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].secTAI93,    pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[0], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[1], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[2], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[0], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[1], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[2], pntr, sizedbl );
			    pntr += sizedbl;
			    memmove( &hdfRecordAtt[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
			  */
			} /* end while */
		      /* Detach from the Vdata */
		      total = totalRecords+recordCount-1;
		      totalRecords +=  GAP_COVER_BLKS;
		      totalatt += GAP_COVER_BLKS;
		      totalRecordsatt += GAP_COVER_BLKS;
		      if(vDataIdAtt != -1){
			VSdetach( vDataIdAtt );
			vDataIdAtt = -1;
		      }
		      if (hdfDataAtt != NULL){
			free(hdfDataAtt);
			hdfDataAtt = NULL;
		      }
		      
		      count=0;
		      
		      gotData = PGS_TRUE;
		    }
		  else
		    {
		      if((totalRecords + GAP_COVER_BLKS) > BLOCK_SIZE_HDF )
			{
			  PGS_SMF_SetDynamicMsg(returnStatus,
						" error: static memory allocated for "
						"hdfRecordAtt is not enough", 
						FUNCTION_NAME2);
			  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
			}
		      
		      if(num == num_files)
			{
			  PGS_SMF_SetDynamicMsg(returnStatus,
						" error in accessing spacecraft "
						"file(s)", FUNCTION_NAME2);
			  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
			}
		    }
		  /* Close the Vset interface */
		  if(hdfIdAtt != -1){
		    Vend( hdfIdAtt );
		    /* Close the HDF format file */
		    Hclose( hdfIdAtt );
		    hdfIdAtt = -1;
		  }
		  if ( gotData == PGS_TRUE)
		    {
		      break;
		    }
		} /* if(file_exists(hdfFileAtt)) */
	      else
		{
		  if(num == num_files)
		    {
		      PGS_SMF_SetDynamicMsg(returnStatus,
					    " error in accessing spacecraft "
					    "file(s)", FUNCTION_NAME2);
		      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		    }
		}
	      
	    }/* end for num_files */
	  
	}/* end if secTAI93 < */
      
      if (  gotData == PGS_FALSE )
	{
	  total = totalatt;
	  totalRecords = totalRecordsatt;
	}

      if(totalatt != 0) 
	{
	  if ( secTAI93 > hdfRecordAtt[totalatt-1].secTAI93 && 
	       secTAI93 < hdfEndTaiatt && gotData == PGS_FALSE )
	    {  
	      count =0; 
	      number = NUM+count+1;
	      files_version_num = number;
	      pgsStatus = PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &number,
					      hdfFileAtt2 );
	      if (pgsStatus == PGS_S_SUCCESS)
		{
		  if(file_exists(hdfFileAtt2))
		    {
		      /* Open the HDF format file */
		      hdfIdAtt2 = Hopen( hdfFileAtt2, DFACC_RDONLY, 0 );
		      /* initialize the Vset interface */
		      Vstart( hdfIdAtt2 );
		      /* Get the reference number of the first Vdata
			 in the file */
		      vDataRefAtt = -1;
		      vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt );
		      /* Attach to the first Vdata in read mode */
		      vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt,"r");
		      /* Get the list of field names */
		      
		      VSinquire( vDataIdAtt, (int32 *)&attNRecs,
				 (int32 *)&interlaceAtt,
				 fieldsAtt, (int32 *)&vDataSizeAtt,
				 vDataNameAtt );
		      /* Name the fields to be read */
		      (void) VSsetfields( vDataIdAtt, fieldsAtt );
		      hdfDataAtt =
			(unsigned char *)malloc(sizeof(PGSt_attitHeader)+
						100*DpCPrMaxUrLen*sizechar);
		      if (hdfDataAtt == NULL)
			{
			  /*ERROR callocing*/
			  sprintf(dynamicMsg,
				  "Error allocating memory for "
				  "hdfDataAtt");
			  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						dynamicMsg,
						FUNCTION_NAME2);
			  if(vDataIdAtt != -1){
			    VSdetach( vDataIdAtt );
			    vDataIdAtt = -1;
			  }
			  if(hdfIdAtt2 != -1){
			    Vend( hdfIdAtt2 );
			    Hclose( hdfIdAtt2 );
			    hdfIdAtt2 = -1;
			  }
			  return(PGSMEM_E_NO_MEMORY);
			}
		      /* Read a packed Vdata record */
		      (void) VSread( vDataIdAtt, hdfDataAtt, 1,
					  FULL_INTERLACE );
		      /* Unpack the packed Vdata record */
		      pntr = hdfDataAtt;
		      memmove( &hdfScId, pntr,
			       DpCPrMaxScIdLen*sizechar );
		      pntr += DpCPrMaxScIdLen*sizechar;
		      memmove( &hdfTimeRange, pntr,
			       DpCPrMaxTimeRangeLen*sizechar );
		      pntr += DpCPrMaxTimeRangeLen*sizechar;
		      memmove( &hdfSource, pntr,
			       DpCPrMaxSourceLen*sizechar );
		      pntr += DpCPrMaxSourceLen*sizechar;
		      memmove( &hdfVersion, pntr,
			       DpCPrMaxVersionLen*sizechar );
		      pntr += DpCPrMaxVersionLen*sizechar;
		      memmove( &hdfStartTai, pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfEndTai, pntr, sizedbl );
		      pntr += sizedbl;
		      memmove( &hdfInterval, pntr, sizeflt );
		      
		      pntr += sizeflt;
		      memmove( &hdfNUrs, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfNRecords, pntr, sizeuint );
		      pntr += sizeuint;
		      memmove( &hdfEulerAngleOrder[0], pntr,
			       DpCPrMaxEulerAngleOrder*sizeuint );
		      pntr += DpCPrMaxEulerAngleOrder*sizeuint;
		      memmove( &hdfQaParams[0], pntr,
			       DpCPrMaxQaParameters*sizeflt );
		      pntr += DpCPrMaxQaParameters*sizeflt;
		      memmove( &hdfQaStats[0], pntr,
			       DpCPrMaxQaStatistics*sizeflt );
		      pntr += DpCPrMaxQaStatistics*sizeflt;
		      counter = (hdfNUrs) ? hdfNUrs : 1;
		      for (iElm=0; iElm<counter; iElm++)
			{
			  memmove( &hdfURs[iElm][0], pntr,
				   DpCPrMaxUrLen*sizechar );
			  pntr += DpCPrMaxUrLen*sizechar;
			}
		      /* Detach from the Vdata */
		      if(vDataIdAtt != -1){
			VSdetach( vDataIdAtt );
			vDataIdAtt = -1;
		      }
		      
		      if (hdfDataAtt != NULL){
			free(hdfDataAtt);
			hdfDataAtt = NULL;
		      }

		      Tget_scTag(hdfScId, &hdfspacecraftTag);
		      
		      /* before doing anything else put summry of this file in 
			 att_files_info structure */
		      if(hdfspacecraftTag == spacecraftTag && att_files_info.num_files >= 1)
			{
			  file_info_exist = 0;
			  for( filecount= 0; filecount < att_files_info.num_files; filecount++)
			    {
			      if(att_files_info.fileversion[filecount] == files_version_num)
				{
				  file_info_exist = 1;
				  break;
				}
			    }
			  if(file_info_exist != 1)
			    {
			      att_files_info.num_files += 1;
			      att_files_info.fileversion[att_files_info.num_files-1] = files_version_num;
			      att_files_info.spacecraftTag[att_files_info.num_files-1] = hdfspacecraftTag;
			      att_files_info.startTime[att_files_info.num_files-1] = hdfStartTai;
			      att_files_info.endTime[att_files_info.num_files-1] = hdfEndTai;
			    }
			}
		      else if( hdfspacecraftTag == spacecraftTag)
			{
			  att_files_info.num_files += 1;
			  att_files_info.fileversion[att_files_info.num_files-1] = files_version_num;
			  att_files_info.spacecraftTag[att_files_info.num_files-1] = hdfspacecraftTag;
			  att_files_info.startTime[att_files_info.num_files-1] = hdfStartTai;
			  att_files_info.endTime[att_files_info.num_files-1] = hdfEndTai;
			}
		      
		      
		      if ( hdfspacecraftTag == spacecraftTag )
			{
			  attNRecs = 0;
			  recordCount = 0;
			  
			  /* Get the reference number or the Vdata */
			  vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt);
			  /* Attach to the first Vdata in read mode */
			  vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt,
						 "r" );
			  /* Get the list of field names */
			  VSinquire( vDataIdAtt, (int32 *)&attNRecs,
				     (int32 *)&interlaceAtt,
				     fieldsAtt, (int32 *)&vDataSizeAtt,
				     vDataNameAtt );
			  /* Name the fields to be read */
			  
			  (void) VSsetfields( vDataIdAtt,
						   fieldsAtt );
			  hdfDataAtt =
			    (unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
			  if (hdfDataAtt == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg,
				      "Error allocating memory for "
				      "hdfDataAtt");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
						    dynamicMsg,
						    FUNCTION_NAME2);
			      if(vDataIdAtt != -1){
				VSdetach( vDataIdAtt );
				vDataIdAtt = -1;
			      }
			      if(hdfIdAtt2 != -1){
				Vend( hdfIdAtt2 );
				Hclose( hdfIdAtt2 );
				hdfIdAtt2 = -1;
			      }
			      return(PGSMEM_E_NO_MEMORY);
			    }

			  recordCount =  -1;
			  while ( recordCount++ < GAP_COVER_BLKS )
			    {
			      /* Read a packed Vdata record */
			      (void) VSread( vDataIdAtt, hdfDataAtt,
						  1, FULL_INTERLACE );
			      /* Unpack the packed Vdata record */
			      
			      memmove( &hdfRecordAtt[totalRecords+recordCount], hdfDataAtt, size_att_eph_struct);
			      /*
				pntr = hdfDataAtt;
				memmove( &hdfRecordAtt[totalRecords+recordCount].secTAI93, pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[0], pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[1], pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[2], pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[0], pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[1], pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[2], pntr, sizedbl );
				pntr += sizedbl;
				memmove( &hdfRecordAtt[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
			      */
			    } /* end while */
			  total = totalRecords+recordCount-1;
			  totalRecords += GAP_COVER_BLKS;
			  
			  /* Detach from the Vdata */
			  if(vDataIdAtt != -1){
			    VSdetach( vDataIdAtt );
			    vDataIdAtt = -1;
			  }
			  if (hdfDataAtt != NULL){
			    free(hdfDataAtt);
			    hdfDataAtt = NULL;
			  }
			  
			} /* end check spacrcrafttag */
		      /* Close the Vset interface */
		      if(hdfIdAtt2 != -1){
			Vend( hdfIdAtt2 );
			/* Close the HDF format file */
			Hclose( hdfIdAtt2 );
			hdfIdAtt2 = -1;
		      }
		      
		    } /* end if(file_exists(hdfFileAtt2)) */
		} /* end if (pgsStatus == SUCCESS) */
	    }
	}
      /*If we got here without getting gotData =PGS_TRUE, then the time 
	may have fallen in the gaps between files, which can happen for 
	real DPREP data, where there are gaps in recordes (i.e. missing 
	records). If this happens look into the att_files_info structure 
	to see if the time is between end time of one file and start time 
	of a consecutive file(with a time which is not 60 sec more than 
	the end time of previous file). If yes, open both and read them to
	buffer (from second file we just need to get a few records) */ 
	
      if(gotData == PGS_FALSE)
	{
	  /* First sort */
	  for (base=0; base<att_files_info.num_files; base++)
	    {
	      /*  Get next earliest element */
	      for (i=base, next=base; i<att_files_info.num_files; i++) 
		{
		  if (att_files_info.startTime[i] <
		      att_files_info.startTime[next])
		    {
		      next=i;
		    }
		}
	      
	      /* 
	       *    Swap into position, if needed
	       */
	      if (next != base)
		{
		  sort_buf_file_version_num = att_files_info.fileversion[base];
		  sort_buf_startTAI93 = att_files_info.startTime[base];
		  sort_buf_stopTAI93 = att_files_info.endTime[base];
		  
		  att_files_info.fileversion[base] =  att_files_info.fileversion[next];
		  att_files_info.startTime[base] =  att_files_info.startTime[next];
		  att_files_info.endTime[base]  = att_files_info.endTime[next];
		  
		  att_files_info.fileversion[next] = sort_buf_file_version_num;
		  att_files_info.startTime[next] = sort_buf_startTAI93;
		  att_files_info.endTime[next] = sort_buf_stopTAI93;
		}
	    }
	  if( (att_files_info.num_files > 1) && 
	      (spacecraftTag == att_files_info.spacecraftTag[0]))
	    {
	      for(i=0; i< (att_files_info.num_files-1); i++)
		{
		  if( (secTAI93 < att_files_info.startTime[i+1]) &&
		      (secTAI93 > att_files_info.endTime[i]) &&
		      (att_files_info.endTime[i] <= att_files_info.startTime[i+1]) &&
		      ((att_files_info.startTime[i+1] - att_files_info.endTime[i]) < 
		       MAX_ATT_TIME_INTERVAL))
		    {
		      /* secTAI93 is in the gap of two attitude files. read them */
		      
		      file_num = att_files_info.fileversion[i];
		      files_version_num = att_files_info.fileversion[i];
		      pgsStatus = PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA,&file_num,
						      hdfFileAtt );
		      if (pgsStatus != PGS_S_SUCCESS)
			{
			  PGS_SMF_SetDynamicMsg(pgsStatus,
						"Error getting reference to HDF format files",
						FUNCTION_NAME2);
			  return pgsStatus;
			}

		      if(file_exists(hdfFileAtt))
			{
			  /* Open the HDF format file */
			  hdfIdAtt = Hopen( hdfFileAtt, DFACC_RDONLY, 0 );
			  /* initialize the Vset interface */
			  Vstart( hdfIdAtt );
			  /* Get the reference number of the first Vdata in the file */
			  vDataRefAtt = -1;
			  vDataRefAtt = VSgetid( hdfIdAtt, vDataRefAtt );
			  /* Attach to the first Vdata in read mode */
			  vDataIdAtt = VSattach( hdfIdAtt, vDataRefAtt, "r" );
			  /* Get the list of field names */
			  VSinquire( vDataIdAtt, (int32 *)&attNRecs,
				     (int32 *)&interlaceAtt,
				     fieldsAtt, (int32 *)&vDataSizeAtt, vDataNameAtt );
			  /* Name the fields to be read */
			  (void) VSsetfields( vDataIdAtt, fieldsAtt );
			  hdfDataAtt = (unsigned char *)malloc(sizeof(PGSt_attitHeader)+
							       100*DpCPrMaxUrLen*sizechar);
			  
			  if (hdfDataAtt == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg, "Error allocating memory for "
				      "hdfDataAtt");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
						    FUNCTION_NAME2);
			      if(vDataIdAtt != -1){
				VSdetach( vDataIdAtt );
				vDataIdAtt = -1;
			      }
			      if(hdfIdAtt != -1){
				Vend( hdfIdAtt );
				Hclose( hdfIdAtt );
				hdfIdAtt = -1;
			      }
			      return(PGSMEM_E_NO_MEMORY);
			    }
			  /* Read a packed Vdata record */
			  (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
			  /* Unpack the packed Vdata record */
			  pntr = hdfDataAtt;
			  
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
			  memmove( &hdfNUrs, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfNRecords, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfEulerAngleOrder[0], pntr, DpCPrMaxEulerAngleOrder*sizeuint );
			  pntr += DpCPrMaxEulerAngleOrder*sizeuint;
			  memmove( &hdfQaParams[0], pntr, DpCPrMaxQaParameters*sizeflt );
			  pntr += DpCPrMaxQaParameters*sizeflt;
			  memmove( &hdfQaStats[0], pntr, DpCPrMaxQaStatistics*sizeflt );
			  pntr += DpCPrMaxQaStatistics*sizeflt;
			  counter = (hdfNUrs) ? hdfNUrs : 1;
			  for (iElm=0; iElm<counter; iElm++)
			    {
			      memmove( &hdfURs[iElm][0], pntr, DpCPrMaxUrLen*sizechar );
			      pntr += DpCPrMaxUrLen*sizechar;
			    }
			  /* Detach from the Vdata */
			  if(vDataIdAtt != -1){
			    VSdetach( vDataIdAtt );
			    vDataIdAtt = -1;
			  }
			  if (hdfDataAtt != NULL){
			    free(hdfDataAtt);
			    hdfDataAtt = NULL;
			  }

			  Tget_scTag(hdfScId, &hdfspacecraftTag);
			  
			  
			  /* Get the reference number or the Vdata */
			  vDataRefAtt = VSgetid( hdfIdAtt, vDataRefAtt );
			  /* Attach to the first Vdata in read mode */
			  vDataIdAtt = VSattach( hdfIdAtt, vDataRefAtt, "r" );
			  /* Get the list of field names */
			  VSinquire( vDataIdAtt, (int32 *)&attNRecs,
				     (int32 *)&interlaceAtt,
				     fieldsAtt, (int32 *)&vDataSizeAtt, vDataNameAtt );
			  /* Name the fields to be read */
			  (void) VSsetfields( vDataIdAtt, fieldsAtt );
			  hdfDataAtt = (unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
			  if (hdfDataAtt == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg, "Error allocating memory for "
				      "hdfDataAtt");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg,
						    FUNCTION_NAME2);
			      if(vDataIdAtt != -1){
				VSdetach( vDataIdAtt );
				vDataIdAtt = -1;
			      }
			      if(hdfIdAtt != -1){
				Vend( hdfIdAtt );
				Hclose( hdfIdAtt );
				hdfIdAtt = -1;
			      }
			      return(PGSMEM_E_NO_MEMORY);
			    }

			  recordCount         =  -1;
			  
			  /* To avoid core dump or memory corruption we should 
			     limit attNRecs to BLOCK_SIZE_HDF
			  */
			  if(attNRecs > BLOCK_SIZE_HDF) attNRecs = BLOCK_SIZE_HDF;

			  while ( recordCount++ < attNRecs )
			    {
			      /* Read a packed Vdata record */
			      (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
			      /* Unpack the packed Vdata record */

			      memmove( &hdfRecordAtt[recordCount], hdfDataAtt, size_att_eph_struct);
			      /*
			      pntr = hdfDataAtt;
			      memmove( &hdfRecordAtt[recordCount].secTAI93,    pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].eulerAngle[0], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].eulerAngle[1], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].eulerAngle[2], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].angularVelocity[0], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].angularVelocity[1], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].angularVelocity[2], pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfRecordAtt[recordCount].qualityFlag, pntr, sizeuint );
			      */
			    } /* end while */
			  /* Detach from the Vdata */
			  if(vDataIdAtt != -1){
			    VSdetach( vDataIdAtt );
			    vDataIdAtt = -1;
			  }
			  if (hdfDataAtt != NULL){
			    free(hdfDataAtt);
			    hdfDataAtt = NULL;
			  }

			  count=0;
			  totalRecords=attNRecs;
			  total=attNRecs;
			  totalatt=attNRecs;
			  totalRecordsatt = attNRecs;
			  beginTAI93att2 = hdfRecordAtt[0].secTAI93;
			  endTAI93att2 = hdfEndTai;
			  hdfEndTaiatt = hdfEndTai;
			  spacecraft2 = hdfspacecraftTag;
			  if(hdfIdAtt != -1){
			    Vend( hdfIdAtt );
			    /* Close the HDF format file */
			    Hclose( hdfIdAtt );
			    hdfIdAtt = -1;
			  }
			}   /* if(file_exists(hdfFileAtt)) */
		      
		      
		      /* open following file and read a few records from it */
		      
		      number = att_files_info.fileversion[i+1];
		      files_version_num = number;
		      pgsStatus =
			PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &number,
					    hdfFileAtt2 );
		      if (pgsStatus == PGS_S_SUCCESS)
			{
			  
			  if(file_exists(hdfFileAtt2))
			    {
			      /* Open the HDF format file */
			      hdfIdAtt2 = Hopen( hdfFileAtt2, DFACC_RDONLY, 0 );
			      /* initialize the Vset interface */
			      Vstart( hdfIdAtt2 );
			      /* Get the reference number of the first Vdata
				 in the file */
			      vDataRefAtt = -1;
			      vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt );
			      /* Attach to the first Vdata in read mode */
			      vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt,"r");
			      /* Get the list of field names */
			      
			      VSinquire( vDataIdAtt, (int32 *)&attNRecs,
					 (int32 *)&interlaceAtt,
					 fieldsAtt, (int32 *)&vDataSizeAtt,
					 vDataNameAtt );
			      /* Name the fields to be read */
			      (void) VSsetfields( vDataIdAtt, fieldsAtt );
			      hdfDataAtt =
				(unsigned char *)malloc(sizeof(PGSt_attitHeader)+
							100*DpCPrMaxUrLen*sizechar);
			      if (hdfDataAtt == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg,
					  "Error allocating memory for "
					  "hdfDataAtt");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
							dynamicMsg,
							FUNCTION_NAME2);
				  if(vDataIdAtt != -1){
				    VSdetach( vDataIdAtt );
				    vDataIdAtt = -1;
				  }
				  if(hdfIdAtt2 != -1){
				    Vend( hdfIdAtt2 );
				    Hclose( hdfIdAtt2 );
				    hdfIdAtt2 = -1;
				  }
				  return(PGSMEM_E_NO_MEMORY);
				}
			      /* Read a packed Vdata record */
			      (void) VSread( vDataIdAtt, hdfDataAtt, 1,
						  FULL_INTERLACE );
			      /* Unpack the packed Vdata record */
			      pntr = hdfDataAtt;
			      memmove( &hdfScId, pntr,
				       DpCPrMaxScIdLen*sizechar );
			      pntr += DpCPrMaxScIdLen*sizechar;
			      memmove( &hdfTimeRange, pntr,
				       DpCPrMaxTimeRangeLen*sizechar );
			      pntr += DpCPrMaxTimeRangeLen*sizechar;
			      memmove( &hdfSource, pntr,
				       DpCPrMaxSourceLen*sizechar );
			      pntr += DpCPrMaxSourceLen*sizechar;
			      memmove( &hdfVersion, pntr,
				       DpCPrMaxVersionLen*sizechar );
			      pntr += DpCPrMaxVersionLen*sizechar;
			      memmove( &hdfStartTai, pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfEndTai, pntr, sizedbl );
			      pntr += sizedbl;
			      memmove( &hdfInterval, pntr, sizeflt );
			      
			      pntr += sizeflt;
			      memmove( &hdfNUrs, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfNRecords, pntr, sizeuint );
			      pntr += sizeuint;
			      memmove( &hdfEulerAngleOrder[0], pntr,
				       DpCPrMaxEulerAngleOrder*sizeuint );
			      pntr += DpCPrMaxEulerAngleOrder*sizeuint;
			      memmove( &hdfQaParams[0], pntr,
				       DpCPrMaxQaParameters*sizeflt );
			      pntr += DpCPrMaxQaParameters*sizeflt;
			      memmove( &hdfQaStats[0], pntr,
				       DpCPrMaxQaStatistics*sizeflt );
			      pntr += DpCPrMaxQaStatistics*sizeflt;
			      counter = (hdfNUrs) ? hdfNUrs : 1;
			      for (iElm=0; iElm<counter; iElm++)
				{
				  memmove( &hdfURs[iElm][0], pntr,
					   DpCPrMaxUrLen*sizechar );
				  pntr += DpCPrMaxUrLen*sizechar;
				}
			      /* Detach from the Vdata */
			      if(vDataIdAtt != -1){
				VSdetach( vDataIdAtt );
				vDataIdAtt = -1;
			      }
			      if (hdfDataAtt != NULL){
				free(hdfDataAtt);
				hdfDataAtt = NULL;
			      }
			      
			      Tget_scTag(hdfScId, &hdfspacecraftTag);
			      
			      attNRecs = 0;
			      recordCount = 0;
			      
			      /* Get the reference number or the Vdata */
			      vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt);
			      /* Attach to the first Vdata in read mode */
			      vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt,
						     "r" );
			      /* Get the list of field names */
			      VSinquire( vDataIdAtt, (int32 *)&attNRecs,
					 (int32 *)&interlaceAtt,
					 fieldsAtt, (int32 *)&vDataSizeAtt,
					 vDataNameAtt );
			      /* Name the fields to be read */
			      
			      (void) VSsetfields( vDataIdAtt,
						       fieldsAtt );
			      hdfDataAtt =
				(unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
			      if (hdfDataAtt == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg,
					  "Error allocating memory for "
					  "hdfDataAtt");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY,
							dynamicMsg,
							FUNCTION_NAME2);
				  if(vDataIdAtt != -1){
				    VSdetach( vDataIdAtt );
				    vDataIdAtt = -1;
				  }
				  if(hdfIdAtt2 != -1){
				    Vend( hdfIdAtt2 );
				    Hclose( hdfIdAtt2 );
				    hdfIdAtt2 = -1;
				  }
				  return(PGSMEM_E_NO_MEMORY);
				}
			      
			      recordCount =  -1;
			      while ( recordCount++ < GAP_COVER_BLKS )
				{
				  /* Read a packed Vdata record */
				  (void) VSread( vDataIdAtt, hdfDataAtt,
						      1, FULL_INTERLACE );
				  /* Unpack the packed Vdata record */

				  memmove( &hdfRecordAtt[totalRecords+recordCount], hdfDataAtt, size_att_eph_struct);
				  /*
				  pntr = hdfDataAtt;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].secTAI93,
					   pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[0]
					   , pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[1]
					   , pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[2]
					   , pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[0], pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[1], pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[2], pntr, sizedbl );
				  pntr += sizedbl;
				  memmove( &hdfRecordAtt[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
				  */
				} /* end while */
			      total = totalRecords+recordCount-1;
			      totalRecords += GAP_COVER_BLKS;
			      totalatt += GAP_COVER_BLKS;
			      totalRecordsatt += GAP_COVER_BLKS;
			      /* Detach from the Vdata */
			      if(vDataIdAtt != -1){
				VSdetach( vDataIdAtt );
				vDataIdAtt = -1;
			      }
			      if (hdfDataAtt != NULL){
				free(hdfDataAtt);
				hdfDataAtt = NULL;
			      }

			      /* Close the Vset interface */
			      if(hdfIdAtt2 != -1){
				Vend( hdfIdAtt2 );
				/* Close the HDF format file */
				Hclose( hdfIdAtt2 );
				hdfIdAtt2 = -1;
			      }
			      
			    } /* end if(file_exists(hdfFileAtt2)) */
			}
		      gotData = PGS_TRUE;
		    }
		  if(gotData == PGS_TRUE) break;
		}
	    }
	}
      
      gotData = PGS_FALSE;
      attRecordCounter = -1; /* if it stays -1 search failed */
      endRecord = total;     /* too big on purpose; will not be used
				as such */
      midRecord = (endRecord + attRecordCounter)/2;
      while(endRecord > attRecordCounter + 1)
	{
	  midRecord = (endRecord + attRecordCounter)/2;
	  if(secTAI93 == hdfRecordAtt[midRecord].secTAI93)
	    {
	      attRecordCounter = midRecord;
	      gotData = PGS_TRUE;
	      break;
	    }
	  if(secTAI93 > hdfRecordAtt[midRecord].secTAI93)
	    {
	      attRecordCounter = midRecord;
	    }
	  else
	    {
	      endRecord   =  midRecord;
	    }
	  gotData = (attRecordCounter >=0 &&
		     attRecordCounter <= total );
	}  /* end while */
      if (gotData == PGS_TRUE)
	{
	  if(attRecordCounter >=0 )
	    {
	      memmove((char *) &attitRecord2,
		      (char *)&hdfRecordAtt[attRecordCounter],
		      sizeof(PGSt_attitRecord));
	      if(attRecordCounter >0 )
		{
		  memmove((char *) &attitRecord1,
			  (char *)&hdfRecordAtt[attRecordCounter-1],
			  sizeof(PGSt_attitRecord));
		}
	    }
	}
      else  /* something went wrong */
	{
	  returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
	  PGS_SMF_SetDynamicMsg(returnStatus,
				"search for data record within attitude file whose header"
				" time range included this time has failed", 
				FUNCTION_NAME2);
	}
      
      /* If the current time falls within bounds of the most 
	 recently obtained record go to the interpolation code... */
      
      if (!(secTAI93 >= attitRecord1.secTAI93 &&
	    secTAI93 < attitRecord2.secTAI93))
	{
	  gotData = PGS_FALSE;
	  for (;attRecordCounter<total;attRecordCounter++)
	    if (secTAI93 < hdfRecordAtt[attRecordCounter].secTAI93)
	      {
		if(attRecordCounter >=0 )
		  {
		    memmove((char*)&attitRecord2,
			    (char*)&hdfRecordAtt[attRecordCounter],
			    sizeof(PGSt_attitRecord));
		    if(attRecordCounter >0 )
		      {
			memmove((char*)&attitRecord1,
				(char*)&hdfRecordAtt[attRecordCounter-1],
				sizeof(PGSt_attitRecord));
		      }
		    gotData = PGS_TRUE;
		    break;
		  }
	      }
	  
	  if (gotData == PGS_FALSE)
	    if (secTAI93 == hdfRecordAtt[total-1].secTAI93)
	      {
		if(attRecordCounter >0 )
		  {
		    memmove((char*)&attitRecord2,
			    (char*)&hdfRecordAtt[attRecordCounter-1],
			    sizeof(PGSt_attitRecord));
		    memmove((char*)&attitRecord1,
			    (char*)&hdfRecordAtt[attRecordCounter-1],
			    sizeof(PGSt_attitRecord));
		    gotData = PGS_TRUE;
		  }
	      }
	  
	}  /* end if ! ... */
      
      if (secTAI93 == attitRecord1.secTAI93)
	{
	  for (count=0;count<3;count++)
	    {
	      eul[count] = attitRecord1.eulerAngle[count];
	      ang[count] = attitRecord1.angularVelocity[count];
	      
	      qua[1] =
		(PGSt_integer) attitRecord1.qualityFlag;
	      
	    }
	}
      else
	{
	  if ((attitRecord2.secTAI93 - attitRecord1.secTAI93) >
	      MAX_ATT_TIME_INTERVAL)
	    {
	      if (returnStatus == PGS_S_SUCCESS)
		{
		  PGS_SMF_SetDynamicMsg(returnStatus,
					"interpolation error for some requested values, "
					"time between actual ephemeris records is too "
					"large (> 60 ",FUNCTION_NAME2);
		}
	    }
	  
	  returnStatus1 =
	    PGS_EPH_interpolateAttitude(attitRecord1.secTAI93,
					attitRecord1.eulerAngle,
					attitRecord1.angularVelocity,
					attitRecord2.secTAI93,
					attitRecord2.eulerAngle,
					attitRecord2.angularVelocity,
					hdfEulerAngleOrder,
					secTAI93,
					eul,
					ang
					);
	  
	  if (returnStatus1 != PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
	      att_interp_err = -1;
	    }
	  
	  if(att_interp_err != -1)
	    {
	      qua[1] = (PGSt_integer)
		(attitRecord1.qualityFlag |
		 attitRecord2.qualityFlag |
		 PGSd_INTERPOLATED_POINT);
	    }
	  else
	    {
	      qua[1]= PGSd_NO_DATA | 0x1;
	    }
	} /* end else */
      if(att_interp_err != -1)
	{
	  returnStatus1 = PGS_CSC_EulerToQuat(eul,
					      hdfEulerAngleOrder,
					      quatEuler);
	  if (returnStatus1 != PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
	    }
	  
	  returnStatus1 = PGS_CSC_getORBtoECIquat(tempPosition,
						  tempVelocity,
						  quatORBtoECI);
	  if (returnStatus1 != PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
	    }
	  
	  returnStatus1 = PGS_CSC_quatMultiply(quatORBtoECI, quatEuler,
					       atti);
	  
	  if (returnStatus1 != PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
	    }
	}
      else
	{
	  for (j=0; j<3; j++)
	    {
	      eul[j] = PGSd_GEO_ERROR_VALUE;
	      ang[j] = PGSd_GEO_ERROR_VALUE;
	      atti[j] = PGSd_GEO_ERROR_VALUE;
	    }
	  atti[3] = PGSd_GEO_ERROR_VALUE;
	}
      
      if (gotData == PGS_TRUE)
	{
	  returnStatus = PGS_S_SUCCESS;
	  /* Close the Vset interface */
	  if(vDataIdAtt != -1)
	    {
	      VSdetach( vDataIdAtt );
	      vDataIdAtt = -1;
	    }
	  if(hdfIdAtt != -1){
	    Vend( hdfIdAtt );
	    /* Close the HDF format file */
	    Hclose( hdfIdAtt );
	    hdfIdAtt = -1;
	  }
	}
    }/* end if attData = 1 */
  if(vDataIdAtt != -1)
    {
      VSdetach( vDataIdAtt );
      vDataIdAtt = -1;
      Vend( hdfIdAtt );
      Hclose( hdfIdAtt );
      hdfIdAtt = -1;
    }
  if(hdfIdAtt != -1)
    {
      Vend( hdfIdAtt );
      Hclose( hdfIdAtt );
      hdfIdAtt = -1;
    }
  
  return returnStatus;
  
}


