
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its Svendors, */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_EPH_EphAtt_unInterpolate.c 

DESCRIPTION:
   This file contains the function PGS_EPH_EphAtt_unInterpolate().
   This function gets uninterpolated actual ephemeris and/or attitude 
   data for the specified spacecraft at the specified time period. The latest
   version of this function is PGS_EPH_EphAttActualRecords(), that gets also
   attitude data if it is requested and the number of attitude records is
   different from the number of ephem record for the same period..

AUTHOR:
   Guru Tej S. Khalsa   / Applied Research Corporation
   Peter D. Noerdlinger / Applied Research Corporation
   Xin Wang             / Emergent Info. Tech.
   Adura Adekunjo       / L3 com. EER Systems Inc.
   Abe Taaheri          / L3 com. EER Systems Inc.

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
   01-Feb-1995  GTSK  Cast all pointers being passed in to the memcpy function
                      to type (char *). This is to prevent issuance of spurious
                      compiler messages about incompatible arguments from the
                      SunOS 4.x platforms .  It should have no effect on other 
                      platforms since the function memcpy takes void pointers 
                      and doesn't care what type of pointer is passed in.
   20-Mar-1995  GTSK  Replaced call to malloc() with call to PGS_MEM_Calloc().
                      Replaced string "PGS_EPH_EphemAttit()"with FUNCTION_NAME.
		      Renamed variable startUTC to asciiUTC.
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
                      called a new value of s/c tag had been passed in). This 
                      has been fixed.
   05-May-1996  GTSK  Implemented major changes to accommodate new
                      ephemeris/attitude file formats.
   27-May-1997  PDN   Deleted references to predicted leap seconds
   12-Feb-1999  PDN   Fixed nomenclature consistent with AM1 and later 
                      spacecraft note that the function is also OK for TRMM, 
                      but in that case please observe that angularVelocity is
                      really (yaw rate, pitch rate,and roll rate in that order)
   22-Feb-1999  PDN   Shortened the maximum interpolation interval from 121
                      seconds (useful in I&T) to 60 seconds, consistent with
                      DPREP capability for AM1 and probably later spacecraft.
   09-July-99   SZ    Updated for the thread-safe functionality
   24-Nov-1999  PDN   Altered the search through a block for the correct data
                      record(s) to use the (faster) method of bisection. Added 
                      comments. This is a rework of changes to Release 5A, July
                      1999 as the work was eclipsed by ThreadSafe changes
  06-Nov-2001   XW    Return actual data upon request.
  06-Sep-2002   AA    Modified to give outputs between a Start time  and a 
                      Stop time, and the number of values between the time 
                      interval.
  20-Nov-2002  AA     Added  the hdfcheck flag to distinguish between binary 
                      and hdf files (ECSed35587).
  18-Mar-2003  AT     Cleaned up, fixed memory leaks, etc.
  10-Jul-2003  AT     Modified to issue warning if user requests both eph 
                      and att data but the number of eph records is not 
                      the same as the number of att records in the same 
                      time period. In this case only the eph data will be 
                      returned to user.
END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get uninterpolated Ephemeris and Attitude records
NAME:
   PGS_EPH_EphAtt_unInterpolate()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_EphAtt_unInterpolate(
        PGSt_tag        spacecraftTag,   
	char            *asciiUTC_start,  
        char            *asciiUTC_stop,
	PGSt_boolean    orbFlag,          
	PGSt_boolean    attFlag,
        PGSt_integer    qualityFlags[][2],
	PGSt_integer    numValues,        
        char            asciiUTC_EphAtt[28],    
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

      integer function pgs_eph_uninterpolate(spacecrafttag,numvalues,
     >                                    asciiutcstart,asciiutcstop,
     >                                    orbflag,attflag,
     >                                    qualityflags,numvalues,
     >                                    asciiutcephatt,
     > 				          positioneci,velocityeci,
     >                                    eulerangles,xyzrotrates,
     > 				          attitquat)
      integer           spacecrafttag
      integer           numvalues
      character*27      asciiutcstart
      character*27      asciiutcstop
      character         asciiutcephatt(27,*)
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
   at the specified time period. User should pass output array sizes to this 
   function usimg numValues to avoid violation of array bounds writing.
   If user requests for both eph and att records, This function will return
   records for both eph and att if the number of records for them are the 
   same in the requested time interval. Otherwise this routine will issue 
   warning and return only eph records to the user. In that case user should 
   call this function twice to get eph and att records separately with each 
   call(by setting orbFlag or attFlag to PGS_TRUE), or call the function
   PGS_EPH_EphAttActualRecords() once to get both data. 

   For the cases where PGS_EPH_EphAtt_unInterpolate() retrieves both EPH and
   ATT records, the retrieved ASCCI time for the records will be those for 
   attitude records only. This may cast doubt on the validity of these 
   retrieved record times for the retrieved EPH data. To avoid confusion,
   user must call this function twice as mentioned above or use 
  PGS_EPH_EphAttActualRecords instead.

INPUTS:
   Name               Description               Units  Min        Max
   ----               -----------               -----  ---        ---
   spacecraftTag      spacecraft identifier     N/A


   asciiUTC_start     UTC time reference start  ASCII  1961-01-01 see NOTES
                      time in CCSDS ASCII time
	              code A format

   asciiUTC_stop      UTC time reference stop   ASCII  1961-01-01 see NOTES
                      time in CCSDS ASCII time
                      code A format

   orbFlag            set to true to get        T/F
                      ephemeris data

   attFlag            set to true to get        T/F
                      attitude data

   numValues         As input this is the max 
                     number of values for EPH
                     or ATT data that is 
                     expecteted to be
                     retrieved                  N/A

OUTPUTS:
   Name              Description                Units        Min         Max
   ----              -----------                -----        ---         ---
   qualityFlags      quality flags for                 ** see NOTES **
                     position and attitude
 
   asciiUTC_EphAtt   UTC time reference        ASCII  1961-01-01 see NOTES
                     Uninterpolated eph/att
                     time in CCSDS ASCII time
                     code A format		     
 
   numValues         number of values for
                     eph and/or att records     N/A   

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
   PGSEPH_W_EPHATT_NUMRECS_DIFFER attitude and ephemeris record sizes 
                                  don't match for the same time period

EXAMPLES:
C:
        #define ARRAY_SIZE 7500
	PGSt_double     positionECI[ARRAY_SIZE][3];
	PGSt_double     velocityECI[ARRAY_SIZE][3];
	PGSt_double     eulerAngles[ARRAY_SIZE][3];
	PGSt_double     xyzRotRates[ARRAY_SIZE][3];
	PGSt_double     attitQuat[ARRAY_SIZE][4];
        PGSt_integer    numValues;
	char            asciiUTC_start[28];
        char            asciiUTC_stop[28];
        char            asciiUTC_EphAtt[ARRAY_SIZE][28];
	PGSt_integer    qualityFlags[ARRAY_SIZE][2];

	
	PGSt_SMF_status returnStatus;


	strcpy(asciiUTC_start,"1998-02-03T19:23:45.123");
        strcpy(asciiUTC_stop,"1998-02-03T20:23:45.123");
        numValues = ARRAY_SIZE;
        returnStatus = PGS_EPH_EphAtt_unInterpolate(PGSd_EOS_AM,
                                          asciiUTC_start,asciiUTC_stop,
	                                  PGS_TRUE,PGS_TRUE,qualityFlags,
                                          numValues, ,asciiUTC_EphAtt,
					  positionECI,velocityECI,
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

      integer           pgs_eph_uninterpolate

      integer           numvalues/10/
      integer           returnstatus
      integer           qualityflags(2,*)
     
      character*27      asciiutcstart 
      character*27      asciiutcstop
      
      character         asciiutcephatt(27,*)
      double precision  positioneci(3,*)
      double precision  velocityeci(3,*)
      double precision  eulerangles(3,*)
      double precision  xyzrotrates(3,*)
      double precision  attitquat(4,*)

      asciiutcstart = '1998-02-03T19:23:45.123'
      asciiutcstop = '1998-02-03T20:23:45.123'

      returnstatus =  pgs_eph_uninterpolate
     >                         (pgsd_eos_am,asciiutcstart,asciiutcstop,
     >                          pgs_true,pgs_true,qualityflags,numvalues,
     >                          asciiutcephatt,positioneci,
     >          		velocityeci,eulerangles,
     >                          xyzrotrates,attitquat)

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
     function depend on the file leapsec.dat which relates leap second 
     (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file, which is updated when a 
     new leap second event is announced, contains dates of leap second events.
     If an input date is outside of the range of dates in the file (or if the 
     file cannot be read) the function PGS_TD_LeapSec() which reads the file
     will return a calculated value of TAI-UTC based on a linear fit of the 
     data in the table as of late 1997.  This value of TAI-UTC is relatively 
     crude estimate and may be off by as much as 1.0 or more seconds. The
     warning return status from PGS_TD_LeapSec() is promoted to an error 
     level by any Toolkit function receiving it, which will terminate the 
     present function.

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
   PGS_TD_TAItoUTC()             returns UTC time in CCSDS ASCII Time Code A 
                                 format
   PGS_EPH_getEphemRecords_UN()  returns an array of s/c ephemeris records
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

#define  BLOCK_SIZE            15000  /* size of ephemDataBlock array,
					 good enough for 2hr worth of data */

#define  MAX_EPH_TIME_INTERVAL 60.0   /* maximum time interval (in seconds)
					  over which ephemeris interpolation
					  will be done */

#define  MAX_ATT_TIME_INTERVAL 60.0   /* maximum time interval (in seconds)
					  over which attitude interpolation
					  will be done */

/* prototype for function used in getting spacecraft Tag */
void Tget_scTag(char *temp_sc_string, PGSt_tag *temp_spacecraftTag);

/* prototype for function used to get ephemeris and or attitude data 
   from HDF files */
static PGSt_SMF_status 
PGS_EPH_checkHDF1(PGSt_tag, PGSt_double, PGSt_double, 
		  PGSt_integer, PGSt_integer, 
		  char [][28], char [][28], 
		  PGSt_integer * , PGSt_integer * ,
		  PGSt_integer [][2], 
		  PGSt_double [][3], PGSt_double [][3], PGSt_double [][3], 
		  PGSt_double [][3], PGSt_double [][4]);

/* name of the functions */
#define  FUNCTION_NAME "PGS_EPH_EphAtt_unInterpolate()"
#define  FUNCTION_NAME2 "PGS_EPH_checkHDF1()"

PGSt_SMF_status
PGS_EPH_EphAtt_unInterpolate(            /* get ephemeris and attitude data */
    PGSt_tag        spacecraftTag,       /* spacecraft identifier */
    char            *asciiUTC_start,     /* reference start time */
    char            *asciiUTC_stop,      /* reference stop time */
    PGSt_boolean    orbFlag,             /* get/don't get orbit data */
    PGSt_boolean    attFlag,             /* get/don't get attitude data */
    void            *qualityFlagsPtr,    /* quality flags */
    PGSt_integer    *numValues,          /* number of values requested */
    char            asciiUTC_EphAtt[][28],/* Record times */
    PGSt_double     positionECI[][3],    /* ECI position */
    PGSt_double     velocityECI[][3],    /* ECI velocity */
    PGSt_double     eulerAngle[][3],     /* s/c attitude Euler angles */
    PGSt_double     angularVelocity[][3],/* angular rates about s/c body axes*/
    PGSt_double     attitQuat[][4])      /* s/c to ECI rotation quaternion */
{

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
    static PGSt_ephemRecord ephemRecordArray[BLOCK_SIZE];/* s/c ephemeris file 
							    records */
    static PGSt_attitRecord attitRecordArray[BLOCK_SIZE];/* s/c ephemeris file 
							    records */
    static PGSt_integer     totalEphemRecords=0;         /* number of records
							    in scRecordArray */
    static PGSt_integer     totalAttitRecords=0;         /* number of records
							    in scRecordArray */
    static PGSt_double      beginTAI93eph=1.0e50;        /* first TAI time in
							    ephemRecordArray */
    static PGSt_double      endTAI93eph=-1.0e50;         /* last TAI time in
							    ephemRecordArray */
    static PGSt_double      beginTAI93att=1.0e50;        /* first TAI time in
							    attitRecordArray */
    static PGSt_double      endTAI93att=-1.0e50;         /* last TAI time in
							    attitRecordArray */
    static PGSt_tag         spacecraft=0;                /* spacecraftTag value
							     from last call to
							     this function */
    static PGSt_integer    lendcheck_old= -1;/* big/little endian m/c check */
                                             /* set to -1 here so that when its
					     setting by PGS_EPH_getAttitHeaders
					     is required, we check it to see
					     whether it is set to 0 or 1 */
    static int       hdfcheck=0;      /* checks if HDF/Binary file is needed */
    static int       anum = 0;
#endif

    PGSt_SMF_status  pgsStatus;
    char             File[256];
    PGSt_integer     endRecord=0;     /* end of search sub-block
                                         for binary search */
    PGSt_integer     midRecord =0;    /* recursively, 1/2, 1/4,..
                                         of the  records in scRecordArray */

    PGSt_boolean    gotData;          /* indicates outcome of data search */

    PGSt_integer    Numvalue;	      /* number of Ephem records beteewn time 
					 interval */
    PGSt_integer    numval;           /* number of attitude records between 
					  time interval */
    PGSt_integer    count;            /* loop counter */
    PGSt_integer    j;		      /* loop counter */
    PGSt_integer    ephRecordCounter=0; /* loop counter */
    PGSt_integer    attRecordCounter=0; /* loop counter */
    PGSt_integer    ephstartIndex;	/* eph start count index*/
    PGSt_integer    ephstopIndex;       /* eph stop count index*/
    PGSt_integer    attstartIndex;      /* att start count index*/
    PGSt_integer    attstopIndex;       /* att stop count index*/
    PGSt_integer    (*qualityFlags)[2]; /* pointer to quality flag data */

    PGSt_integer    lendcheck= -1;      /* little/big endian flag */
                                        /* set to -1 here so that when its
					   setting by PGS_EPH_getAttitHeaders
					   is required, we check it to see
					   whether it is set to 0 or 1 */
    PGSt_double     secTAI93_start;     /* start time of requested s/c data */
    PGSt_double     secTAI93_stop;      /*  stop time of requested s/c data */
    PGSt_double     secTAI93_start_eph; /* start time of requested s/c eph 
					   data */
    PGSt_double     secTAI93_stop_eph;  /*  stop time of requested s/c eph 
					    ddata */
    PGSt_double     secTAI93_start_att; /* start time of requested s/c att 
					   data */
    PGSt_double     secTAI93_stop_att;  /*  stop time of requested s/c att 
					    data */
    PGSt_double     startTAI93;         /* TAI equivalent of asciiUTC */
    PGSt_double     stopTAI93;          /* TAI equivalent of asciiUTC */
    PGSt_double     quatORBtoECI[4];    /* orbital ref. frame to ECI 
					   quaternion */
    PGSt_double     quatEuler[4];       /* quaternion equivalent of s/c 
					   attitude as defined by the s/c 
					   Euler angles */

    PGSt_scTagInfo  scTagInfo;          /* s/c tag info. structure */

    char            mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* error mnemonic */
    char            msg[PGS_SMF_MAX_MSG_SIZE];           /* error message */
    char            details[PGS_SMF_MAX_MSG_SIZE];       /* detailed error 
							    message */

    PGSt_SMF_status returnStatus1;       /* return value of PGS function 
					    calls */
    PGSt_SMF_status code;                /* returned from PGS_SMF_GetMsg() */
    PGSt_SMF_status returnStatus;        /* return value of this function */ 
    PGSt_integer    numv_Eph; 
    PGSt_integer    numv_Att; 

    PGSt_integer    num_files_eph = 0;
    PGSt_integer    ifile, ithfile;
    PGSt_integer    hdfcheck_next=0;
    PGSt_integer    DidNotGetEPH=0;
    char            asciiUTC_Eph[BLOCK_SIZE][28];
    char            asciiUTC_Att[BLOCK_SIZE][28];
#ifdef _PGS_THREADSAFE
    /* Set up TSF globals, global index, and TSF key keeper for the 
       thread-safe */
    int                 masterTSFIndex;
    extern PGSt_integer PGSg_TSF_EPHtotalEphemRecords[];
    extern PGSt_integer PGSg_TSF_EPHtotalAttitRecords[];
    extern PGSt_double  PGSg_TSF_EPHbeginTAI93eph[];
    extern PGSt_double  PGSg_TSF_EPHendTAI93eph[];
    extern PGSt_double  PGSg_TSF_EPHbeginTAI93att[];
    extern PGSt_double  PGSg_TSF_EPHendTAI93att[];
    extern PGSt_tag     PGSg_TSF_EPHspacecraft[];
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
    
    /* initialize return value to indicate success */
    
    returnStatus = PGS_S_SUCCESS;
    returnStatus1 = PGS_S_SUCCESS;
    
    /* initialize position and velocity pointers to point to the appropriate
       places is the array tmpPosVel */
    
    /* The incoming pointer to the quality flags array must be cast to type:
       pointer to array of two integers.  This is what is expected.  A void 
       type is being used for now to handle some backward compatibility issues,
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

    if(spacecraftTag == spacecraft)
      {
	lendcheck = lendcheck_old;
      }
    
    /* convert ASCII UTC start time to real continuous seconds since
       12AM UTC 1/1/93, this is done so that the time offsets which are in
       seconds can be added to this value to get a unique numeric time for 
       each point */
    
    returnStatus = PGS_TD_UTCtoTAI(asciiUTC_start, &startTAI93);
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
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
	return PGS_E_TOOLKIT;
      }
    
    returnStatus = PGS_TD_UTCtoTAI(asciiUTC_stop, &stopTAI93);
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
        return returnStatus;
      default:
        PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
        return PGS_E_TOOLKIT;
      }
    
    if ( startTAI93 > stopTAI93 )
      { 
	PGS_SMF_SetStaticMsg(PGSEPH_E_BAD_TIME_ORDER,FUNCTION_NAME);
	return PGSEPH_E_BAD_TIME_ORDER;
      }


    /* check that all eph & att files are HDF or BINARY */
    if (hdfcheck == 0 && anum == 0)
      { 
	pgsStatus = 
	  PGS_PC_GetNumberOfFiles(PGSd_SC_EPHEM_DATA, &num_files_eph);
	
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
	    pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHEM_DATA, &ithfile, 
					    File);
	    
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
	    pgsStatus = PGS_PC_GetNumberOfFiles(PGSd_SC_ATTIT_DATA, 
						&num_files_eph);
	    
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
		pgsStatus = PGS_PC_GetReference(PGSd_SC_ATTIT_DATA, 
						&ithfile, File);
		
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

    if ( hdfcheck == 0)  /* eph/att files are binary files */
      {	
	secTAI93_start_eph = startTAI93;
	secTAI93_stop_eph  = stopTAI93;
	
	gotData = PGS_FALSE;
	
	if (secTAI93_start_eph < beginTAI93eph ||
	    secTAI93_start_eph > endTAI93eph ||
	    spacecraftTag != spacecraft)
	  {
	    secTAI93_start_eph = startTAI93;
	    
	    /* Fill the array ephemRecordArray with s/c ephemeris records. */
	    
	    returnStatus1 = 
	      PGS_EPH_getEphemRecords_UN(&scTagInfo,secTAI93_start_eph,
					 BLOCK_SIZE,ephemRecordArray,
					 &totalEphemRecords, &lendcheck);
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
		break;
	      default:
		PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
		returnStatus1 = PGS_E_TOOLKIT;
		break;
	      }/* end switch on return from PGS_EPH_getEphemRecords_UN() */
	  } /* END: if (secTAI93_start_eph < beginTAI93eph ... except 
	       for "else" */
	else /* in this case time is in range; no need to get file */
	  {
	    gotData = PGS_TRUE;
	  }
	
	/* if gotData is still false here, all times were checked and no s/c
	   ephemeris file was successfully opened, so exit with error message
	   if only eph data requested; Otherwise get the attutiude data 
	   without attitude quaternions calculations and give warning to user
	*/
	
	if (gotData == PGS_FALSE )
	  {
	    if(orbFlag == PGS_TRUE)
	      {
		strcpy(msg,"Unable to get ephem records for the requested time period");
		PGS_SMF_SetDynamicMsg(returnStatus1,msg,FUNCTION_NAME);
		return returnStatus1;
	      }
	    else
	      {
		DidNotGetEPH = 1;
		strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
		PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME);
	      }
	  }
	/* now go on to obtaining attitude data if gotData = PGS_TRUE */
	
	if (attFlag == PGS_TRUE)
	  {
	    secTAI93_start_att = startTAI93;
	    secTAI93_stop_att  = stopTAI93;
	    
	    gotData = PGS_FALSE;
	    
	    if (secTAI93_start_att < beginTAI93att ||
		secTAI93_start_att > endTAI93att ||
		spacecraftTag != spacecraft)
	      {
		secTAI93_start_att = startTAI93 ;
		
		
		/* Fill the array attitRecordArray with s/c attitude records */
		
		returnStatus1 = PGS_EPH_getAttitRecords_UN(&scTagInfo,
							   &lendcheck,
							   secTAI93_start_att,
							   BLOCK_SIZE,
							   attitRecordArray,
							   &totalAttitRecords);
#ifdef _PGS_THREADSAFE
                /* Reset global */
                PGSg_TSF_EPHtotalAttitRecords[masterTSFIndex] = 
		  totalAttitRecords;
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
		    break;
		  default:
		    gotData = PGS_FALSE;
		    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
		    returnStatus1 = PGS_E_TOOLKIT;
		    break;
		  } /* end switch on return from PGS_EPH_getAttitRecords_UN()*/
	      }
	    else /* time is already in the range */
	      {
		gotData = PGS_TRUE;
	      } /* END: if (secTAI93 < startTAI93att ... */
	    
	    /* if gotData is still false here, all times were checked 
	       and no s/c (necessary)  attitude file was successfully 
	       opened, so exit with error message */
	    
	    if (gotData == PGS_FALSE )
	      {
		strcpy(msg,"Unable to get attitude records for the requested time oeriod");
		PGS_SMF_SetDynamicMsg(returnStatus1,msg,FUNCTION_NAME);
		return returnStatus1;
	      }
	  }
	if (gotData == PGS_TRUE) 
	  {
	    /* data is available for the current spacecraft; save the 
	       value of the spacecraft tag so the data isn't reloaded 
	       unnecessarily in subsequent calls to this function */
	    
	    spacecraft = spacecraftTag;
	    
	    /* Also save the Endianness of Binary files for the 
	       subsequent calls 
	       to this function with the same spacecraft name */
	    
	    lendcheck_old = lendcheck;
	    
	    
#ifdef _PGS_THREADSAFE
	    /* Reset global */
	    PGSg_TSF_EPHspacecraft[masterTSFIndex] = spacecraft;
#endif          
	    
	  } /* We loaded ephemeris data and, if requested, attitude data; 
	       now find the appropriate records in the time range */
	
	if (DidNotGetEPH == 0)
	  {	    
	    gotData = PGS_FALSE;
	    
	    /* The algorithm for search is changed, so that we do not waste 
	       time stepping through all of the totalEphemRecords for each 
	       user-requested time */
	    
	    ephRecordCounter = -1; /* if it stays -1 search failed */
	    endRecord = totalEphemRecords;  /* too big on purpose; 
					       will not be used as such */
	    midRecord = (endRecord + ephRecordCounter)/2;
	    
	    while(endRecord > ephRecordCounter + 1)
	      {
		midRecord = (endRecord + ephRecordCounter)/2;
		if(secTAI93_start_eph == ephemRecordArray[midRecord].secTAI93)
		  {
		    ephRecordCounter = midRecord;
		    gotData = PGS_TRUE;
		    break;
		  }
		if(secTAI93_start_eph > ephemRecordArray[midRecord].secTAI93)
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
		if(ephemRecordArray[ephRecordCounter].secTAI93 < 
		   secTAI93_start_eph)
		  {
		    ephstartIndex = ephRecordCounter+1;
		  }
		
		if(secTAI93_start_eph <= 
		   ephemRecordArray[ephRecordCounter].secTAI93) 
		  { 
                    ephstartIndex = ephRecordCounter;
		    
		  }
		
	      }
	    else
	      {
		if(orbFlag == PGS_TRUE)
		  {
		    returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
		    sprintf(details,
		    "search for data record within ephemeris file whose header"
		    " time range included this time has failed");
		    PGS_SMF_SetDynamicMsg(returnStatus, details, FUNCTION_NAME);
		    return returnStatus;
		  }
		else
		  {
		    DidNotGetEPH = 1;
		    strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
		    PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME);
		  }
	      }
	    
	    /* Do the same for the Ephemeris stop time */ 
	    if(DidNotGetEPH == 0)
	      {
		gotData = PGS_FALSE;  
		
		ephRecordCounter = -1; /* if it stays -1 search failed */
		endRecord = totalEphemRecords;  /* too big on purpose; 
						   will not be used as such */
		midRecord = (endRecord + ephRecordCounter)/2;
		
		while(endRecord > ephRecordCounter + 1)
		  {
		    midRecord = (endRecord + ephRecordCounter)/2;
		    
		    if(secTAI93_stop_eph == ephemRecordArray[midRecord].secTAI93)
		      {
			ephRecordCounter = midRecord;
			gotData = PGS_TRUE;
			break;
		      }
		    
		    if(secTAI93_stop_eph > ephemRecordArray[midRecord].secTAI93)
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
		
		
		if(gotData == PGS_TRUE)
		  {
		    if(ephemRecordArray[ephRecordCounter].secTAI93 <= 
		       secTAI93_stop_eph)
		      {
			ephstopIndex = ephRecordCounter;
		      }	
		    if (secTAI93_stop_eph < 
			ephemRecordArray[ephRecordCounter].secTAI93)
		      {    ephstopIndex = ephRecordCounter-1;
		      
		      }
		  }
		
		else  /* something went wrong */
		  {
		    if(orbFlag == PGS_TRUE)
		      {
			returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
			sprintf(details,
			"search for data record within ephemeris file whose header"
			" time range included this time has failed");
			PGS_SMF_SetDynamicMsg(returnStatus, details, FUNCTION_NAME);
			return returnStatus;
		      }
		    else
		      {
			DidNotGetEPH = 1;
			strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
			PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME);
		      }
		  }
	      }
	    
	    if(DidNotGetEPH == 0)
	      {
		Numvalue = ( ephstopIndex - ephstartIndex );
		*numValues = Numvalue;
		for( j = 0; j < *numValues; j++ )
		  {
		    returnStatus = 
		      PGS_TD_TAItoUTC(ephemRecordArray[ j+ephstartIndex].secTAI93, 
				      asciiUTC_EphAtt[j]);
		    
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
			if(orbFlag == PGS_TRUE)
			  {
			    return returnStatus;
			  }
			else
			  {
			    DidNotGetEPH = 1;
			    strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
			    PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME);
			  }
		      default:
			if(orbFlag == PGS_TRUE)
			  {
			    PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
			    return PGS_E_TOOLKIT;
			  }
			else
			  {
			    DidNotGetEPH = 1;
			    strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
			    PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME);
			  }
		      }
		    if(DidNotGetEPH == 0)
		      {
			qualityFlags[j][0] = 
			  ephemRecordArray[ j + ephstartIndex].qualityFlag;
			
			for (count = 0; count < 3; count++ )
			  {
			    positionECI[j][count] = 
			      ephemRecordArray[ j + ephstartIndex].position[count];
			    velocityECI[j][count] = 
			      ephemRecordArray[ j + ephstartIndex].velocity[count];
			  }
		      }
		  }

		/* we got what we wanted, if user requests only eph data, 
		   leave */
		if(orbFlag == PGS_TRUE && attFlag == PGS_FALSE )
		  {
		    return returnStatus;
		  }
		
	      }
	  }

	if (attFlag == PGS_TRUE)
	  {	 
	    gotData = PGS_FALSE;
	    
	    /* The algorithm for search is changed, so that we do not 
	       waste time stepping through all of the totalAttitRecords 
	       for each user-requested time */
	    
	    attRecordCounter = -1;
	    endRecord = totalAttitRecords;  /* too big on purpose; 
					       will not be used as such */
	    midRecord = (endRecord + attRecordCounter)/2;
	    
	    while(endRecord > attRecordCounter + 1)
	      {
		midRecord = (endRecord + attRecordCounter)/2;
		if(secTAI93_start_att == attitRecordArray[midRecord].secTAI93)
		  {
		    attRecordCounter = midRecord;
		    gotData = PGS_TRUE;
		    break;
		  }
		if(secTAI93_start_att > attitRecordArray[midRecord].secTAI93)
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
		if (attitRecordArray[attRecordCounter].secTAI93 < 
		    secTAI93_start_att)
		  {
		    attstartIndex = attRecordCounter+1;
		  }
		
		if(secTAI93_start_att <= 
		   attitRecordArray[attRecordCounter].secTAI93)
		  {        
		    attstartIndex = attRecordCounter;
		  }
	      }
	    else /* something went wrong */ 
	      {
		returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
		sprintf(details,
		"search for data record within attitude file whose header"
	     	" time range included this time has failed");
		PGS_SMF_SetDynamicMsg(returnStatus, details, 
				      FUNCTION_NAME);
		
		return returnStatus;
	      }
	    
	    
	    /* Repeat for attitude stop time */
	    gotData = PGS_FALSE;
	    
	    attRecordCounter = -1;
	    endRecord = totalAttitRecords;  /* too big on purpose; 
					       will not be used as such */
	    midRecord = (endRecord + attRecordCounter)/2;
	    
	    while(endRecord > attRecordCounter + 1)
	      {
		midRecord = (endRecord + attRecordCounter)/2;
		if(secTAI93_stop_att == attitRecordArray[midRecord].secTAI93)
		  {
		    attRecordCounter = midRecord;
		    gotData = PGS_TRUE;
		    break;
		  }
		if(secTAI93_stop_att > attitRecordArray[midRecord].secTAI93)
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
	    
	    if(gotData == PGS_TRUE)
	      {
		
		if(attitRecordArray[attRecordCounter].secTAI93 <= 
		   secTAI93_stop_att)
		  {
		    attstopIndex = attRecordCounter;
		  }
		
		if(secTAI93_stop_att < 
		   attitRecordArray[attRecordCounter].secTAI93)
		  {
		    attstopIndex = attRecordCounter;
		  }       
		
	      }
	    
	    else
	      {
		returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
		sprintf(details,
		"search for data record within attitude file whose header"
	       	" time range included this time has failed");
		PGS_SMF_SetDynamicMsg(returnStatus, details, 
				      FUNCTION_NAME);
		
		return returnStatus;
	      }
	    
	    numval = (attstopIndex - attstartIndex);
	  }
	
	
	
	if((orbFlag == PGS_TRUE && 
	    attFlag == PGS_TRUE && DidNotGetEPH == 0) ||
	   (orbFlag == PGS_FALSE && 
	    attFlag == PGS_TRUE && DidNotGetEPH == 0))
	  {
	    for( j = 0; j < numval; j++ )
	      {
		
		/* if orbFlag == PGS_FALSE ascii time should be
		   the one for attitude records */
		if(orbFlag == PGS_FALSE)
		  {
		    returnStatus = 
		      PGS_TD_TAItoUTC(attitRecordArray[ j+attstartIndex].secTAI93, 
				      asciiUTC_EphAtt[j]);
		    
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
			return returnStatus;
		      default:
			PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
			return PGS_E_TOOLKIT;
		      }
		  }
		qualityFlags[j][1] = 
		  attitRecordArray[ j + attstartIndex].qualityFlag;
		for (count = 0; count < 3; count++ )
		  {  
		    angularVelocity[j][count] =
		      attitRecordArray[ j + attstartIndex].angularVelocity[count];
		    eulerAngle[j][count] = 
		      attitRecordArray[ j + attstartIndex].eulerAngle[count];
		  }
		
	      } /* END for j  */
	    if (numval == Numvalue)
	      {
		
		*numValues = numval;
	      }
	    else
	      {
		if(orbFlag == PGS_FALSE)
		  {
		    *numValues = numval;
		  }
		if(orbFlag == PGS_TRUE)
		  {
		    returnStatus = PGSEPH_W_EPHATT_NUMRECS_DIFFER;
		    sprintf(details," attitude and ephemeris record sizes don't match (nrec_eph=%d , nrec_att=%d), only eph records will be returned", Numvalue, numval);
		    PGS_SMF_SetDynamicMsg(returnStatus,details,FUNCTION_NAME);
		    for( j = 0; j < numval; j++ )
		      {
			qualityFlags[j][1] = (PGSt_integer)0.0;
			
			for (count = 0; count < 3; count++ )
			  {  
			    angularVelocity[j][count] = 0.0;
			    eulerAngle[j][count] = 0.0;
			  }
		      }
		    return returnStatus;
		  }
		else if(orbFlag == PGS_FALSE)
		  {
		    returnStatus = PGSEPH_W_EPHATT_NUMRECS_DIFFER;
		    sprintf(details," attitude and ephemeris record sizes don't match (nrec_eph=%d , nrec_att=%d), uninterpolated attitude quaternions cannot be calculated", Numvalue, numval);
		    PGS_SMF_SetDynamicMsg(returnStatus,details,FUNCTION_NAME);
		    return returnStatus;
		  }
	      }
	    
	    for( j = 0; j < *numValues; j++ )
	      {
		returnStatus1 = 
		  PGS_CSC_EulerToQuat(eulerAngle[j],
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
		    continue;
		  }
		/* get the orbital to ECI reference frame quaternion */
		
		returnStatus1 = PGS_CSC_getORBtoECIquat(positionECI[j],
							velocityECI[j],
							quatORBtoECI);
		if (returnStatus1 != PGS_S_SUCCESS)
		  {
		    if (returnStatus == PGS_S_SUCCESS)
		      {
			PGS_SMF_SetUnknownMsg(returnStatus1, 
					      FUNCTION_NAME);
			PGS_SMF_GetMsg(&code,mnemonic,details);
		      }
		    
		    continue;
		  }
		
		returnStatus1 = PGS_CSC_quatMultiply(quatORBtoECI, 
						     quatEuler,
						     attitQuat[j]);
		if (returnStatus1 != PGS_S_SUCCESS)
		  {
		    if (returnStatus == PGS_S_SUCCESS)
		      {
			PGS_SMF_SetUnknownMsg(returnStatus1, 
					      FUNCTION_NAME);
			PGS_SMF_GetMsg(&code, mnemonic, details);
		      }
		    continue;
		  }
	      }
	    return returnStatus;
	  }
	if(DidNotGetEPH == 1)
	  {
	    if(orbFlag == PGS_TRUE )/* we wanted eph data but we couldn't,
				       this is error */
	      {
		returnStatus = PGS_E_TOOLKIT;
		sprintf(details," Problem getting requested eph data");
		PGS_SMF_SetDynamicMsg(returnStatus,details,FUNCTION_NAME);
	      }
	    else if (orbFlag == PGS_FALSE && attFlag == PGS_TRUE)
	      {
		*numValues = numval;
		
		for( j = 0; j < *numValues; j++ )
		  {
		    returnStatus = 
		      PGS_TD_TAItoUTC(attitRecordArray[ j+attstartIndex].secTAI93, 
				      asciiUTC_EphAtt[j]);
		    
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
			return returnStatus;
		      default:
			PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
			return PGS_E_TOOLKIT;
		      }
		    
		    qualityFlags[j][1] = 
		      attitRecordArray[ j + attstartIndex].qualityFlag;
		    for (count = 0; count < 3; count++ )
		      {  
			angularVelocity[j][count] =
			  attitRecordArray[ j + attstartIndex].angularVelocity[count];
			eulerAngle[j][count] = 
			  attitRecordArray[ j + attstartIndex].eulerAngle[count];
		      }
		    
		  } /* END for j  */
	      }
	    return returnStatus;
	  }
      }
    
    /********************************************************/ 
    /* To get ephemeris and/or attitude data from HDF files */
    /********************************************************/ 
    
    if (hdfcheck == 1)
      {
	numv_Eph = *numValues;
	numv_Att = *numValues;
	
	secTAI93_start = startTAI93;
	secTAI93_stop =  stopTAI93;
	
	returnStatus = PGS_EPH_checkHDF1(spacecraftTag, secTAI93_start, 
					 secTAI93_stop, orbFlag, attFlag, 
					 asciiUTC_Eph, asciiUTC_Att, 
					 &numv_Eph, &numv_Att,
					 qualityFlags, 
					 positionECI, velocityECI, eulerAngle,
					 angularVelocity, attitQuat);
	
	/* if we were expecting everything and we got all */
	/* OR */
	/* if we were expecting everything but eph and att record number
	   were different in the requested time range */
	if(orbFlag == PGS_TRUE &&  
	   attFlag == PGS_TRUE && 
	   (returnStatus == PGS_S_SUCCESS || 
	    returnStatus == PGSEPH_W_EPHATT_NUMRECS_DIFFER))
	  {
	    *numValues = numv_Eph;
	    for(j=0; j<*numValues; j++)
	      {
		strcpy(asciiUTC_EphAtt[j], asciiUTC_Eph[j]);
	      }
	    /* we do not pass any attitude data if return status is 
	       PGSEPH_W_EPHATT_NUMRECS_DIFFER */
	    if(returnStatus == PGSEPH_W_EPHATT_NUMRECS_DIFFER)
	      {
		for(j=0; j<numv_Att; j++)
		  {
		    qualityFlags[j][1] = (PGSt_integer)0.0;
		    for (count = 0; count < 3; count++ )
		      {
			eulerAngle[j][count] = 0.0;
			angularVelocity[j][count] = 0.0;
		      }
		  }
	      }
	    return returnStatus;
	  }
	/* if we were expecting att records only but we were not able to
           get eph and, therefor, quaternion too */
	else if(orbFlag == PGS_FALSE &&  
		attFlag == PGS_TRUE &&
		(returnStatus == PGS_S_SUCCESS || 
		 returnStatus == PGSEPH_W_EPHATT_NUMRECS_DIFFER))
	  {
	    *numValues = numv_Att;
	    for(j=0; j<*numValues; j++)
	      {
		strcpy(asciiUTC_EphAtt[j], asciiUTC_Att[j]);
	      }
	    /* we do not pass any eph data if return status is 
	       PGSEPH_W_EPHATT_NUMRECS_DIFFER */
	    if(returnStatus == PGSEPH_W_EPHATT_NUMRECS_DIFFER)
	      {
		for(j=0; j<numv_Eph; j++)
		  {
		    qualityFlags[j][0] = 0;
		    for (count = 0; count < 3; count++ )
		      {
			positionECI[j][count] = 0.0;
			velocityECI[j][count] = 0.0;
		      }
		  }
	      }
	    return returnStatus;
	  }
	else if( orbFlag == PGS_TRUE && attFlag == PGS_FALSE)
	  {
	    *numValues = numv_Eph;
	    for(j=0; j<*numValues; j++)
	      {
		strcpy(asciiUTC_EphAtt[j], asciiUTC_Eph[j]);
	      }
	  }
	else
	  {
	    PGS_SMF_GetMsgByCode(returnStatus,details);
	    PGS_SMF_SetDynamicMsg(returnStatus, details, 
				  FUNCTION_NAME);
	    return returnStatus;
	  }	
      }
 
    return returnStatus;
}
/**************************************************************************/
/*
 * file_split_path()
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
PGS_EPH_checkHDF1(PGSt_tag spacecraftTag,
		  PGSt_double secTAI93_start,
		  PGSt_double secTAI93_stop,
		  PGSt_boolean    orbFlag,
		  PGSt_boolean    attFlag, 
		  char  asciiUTC_Eph[][28],
		  char  asciiUTC_Att[][28],
		  PGSt_integer *numvalues_Eph,
		  PGSt_integer *numvalues_Att,
		  PGSt_integer qualityFlags[][2],
		  PGSt_double positionECI[][3],
		  PGSt_double velocityECI[][3],
		  PGSt_double eulerAngle[][3],
		  PGSt_double angularVelocity[][3],
		  PGSt_double attitQuat[][4])
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
  /*  const int DpCPrMaxUrs              = 10;*/
  
  PGSt_integer     num_files, file_numb, num, number;
  PGSt_integer     file_num=1;
  char             hdfFile[256], hdfFile2[256], hdfFile3[256], hdfFile4[256];
  char             hdfFileAtt[256], hdfFileAtt2[256], hdfFileAtt3[256];
  char             hdfFileAtt4[256];
  PGSt_SMF_status  pgsStatus;
  int32            hdfId, hdfId2, hdfId3, hdfId4;
  int32            hdfIdAtt, hdfIdAtt2, hdfIdAtt3, hdfIdAtt4;
  int32            vDataRef, vDataRefAtt;
  int32            vDataId, vDataIdAtt;
  int32            hdrNRecs, interlace, vDataSize;
  int32            hdrNRecsAtt, interlaceAtt, vDataSizeAtt;
  char             fields[512], fieldsAtt[512];
  char             vDataName[64], vDataNameAtt[64];
  unsigned char    *hdfData, *hdfDataAtt;
  unsigned char    *pntr;
  char             hdfScId[24];
  char             hdfTimeRange[48];
  char             hdfSource[32];
  char             hdfVersion[8];
  double           hdfStartTai;
  double           hdfEndTai;
  
  float            hdfInterval;
  int              hdfNUrs;
  int              hdfNRecords;
  int              hdfEulerAngleOrder[3];
  int              hdfNOrbits;
  int              hdfOrbitStart;
  int              hdfOrbitEnd;
  char             hdfRefFrame[8];
  double           keplerElements[6];
  double           keplerEpochTai;
  double           hdfQaParams[16];
  double           hdfQaStats[4];
  double           hdfPeriod;
  PGSt_double      hdfDescProp;
  int              hdfFddReplace;
  char             hdfURs[10][256];
  char             mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* error mnemonic */
  char             details[PGS_SMF_MAX_MSG_SIZE]; /* detailed error message */

  PGSt_integer     iElm;
  PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;
  PGSt_SMF_status  code;             /* returned from PGS_SMF_GetMsg() */
  PGSt_SMF_status  returnStatus1;
  PGSt_tag         hdfspacecraftTag;
  int32            ephNRecs=0, attNRecs=0;
  PGSt_integer     recordCount = 0;
  PGSt_ephemRecord *hdfRecord = NULL;
  PGSt_attitRecord *hdfRecordAtt = NULL;
  PGSt_integer     count;
  PGSt_boolean     gotData = PGS_FALSE;
  PGSt_integer     ephRecordCounter=0;
  PGSt_integer     attRecordCounter=0;
  PGSt_integer     ephstartIndex;
  PGSt_integer     ephstopIndex;
  PGSt_integer     attstartIndex;
  PGSt_integer     attstopIndex;
  PGSt_integer     midRecord =0;
  PGSt_integer     endRecord=0;
  PGSt_integer     NUM;
  PGSt_integer     NUMA;
  PGSt_integer     counter;
  PGSt_integer     j;
  PGSt_double      quatEuler[4];
  PGSt_double      quatORBtoECI[4];
  PGSt_integer     totalRecords=0, total=0;
  PGSt_integer     sizeint, sizechar, sizeflt, sizedbl, sizeuint;
  char             dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
  PGSt_integer     input_numvalues_Eph;
  PGSt_integer     input_numvalues_Att;
  PGSt_integer     DidNotGetEPH=0;
  char             msg[PGS_SMF_MAX_MSG_SIZE];           /* error message */

  sizeflt = sizeof(PGSt_real);
  sizedbl = sizeof(PGSt_double);
  sizechar = sizeof(char);
  sizeint = sizeof(int32); 
  sizeuint = sizeof(PGSt_uinteger);
  /* make sure that input numvalues is a reasonable number
     This number cannot exceed the dimmension of user passed in arrays,
     otherwise it may result in core dump */

  if(*numvalues_Eph <= 0 || *numvalues_Att <= 0)
    {
      returnStatus = PGSEPH_E_BAD_ARRAY_SIZE;
      sprintf(dynamicMsg,"The 6th argument passed to %s cannot be zero or negative number. Max value is the dimmension of array arguments that user shall provide.",FUNCTION_NAME);
      PGS_SMF_SetDynamicMsg(returnStatus, dynamicMsg, FUNCTION_NAME2);
      return(returnStatus);
    }
  else
    {
      input_numvalues_Eph = *numvalues_Eph;
      input_numvalues_Att = *numvalues_Att;
    }

  PGS_PC_GetNumberOfFiles(PGSd_SC_EPHHDF_DATA, &num_files);
  for ( num = 1; num <= num_files; num++)
    {
      file_num = num;
      pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&file_num, hdfFile );
      
      if (pgsStatus != PGS_S_SUCCESS)
	{
	  if(orbFlag == PGS_TRUE)
	    {
	      PGS_SMF_SetDynamicMsg(pgsStatus,
				    "Error getting reference to HDF format files",
				    FUNCTION_NAME2);
	      return pgsStatus;
	    }
	  else
	    {
	      DidNotGetEPH = 1;
	      strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
	      PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME2);
	      goto GET_ATT;
	    }
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
	  VSinquire( vDataId, (int32 *)&hdrNRecs, (int32 *)&interlace,
		     fields, (int32 *)&vDataSize, vDataName );
	  /* Name the fields to be read */
	  (void) VSsetfields( vDataId, fields );
	  
	  hdfData = 
	    (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+
				     100*DpCPrMaxUrLen*sizechar+
				     2*sizedbl+sizeint);
	  if (hdfData == NULL)
	    {
	      /*ERROR callocing*/
	      sprintf(dynamicMsg, "Error allocating memory for "
		      "hdfData");
	      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
				    FUNCTION_NAME2);
	      VSdetach( vDataId );
	      vDataId = -1;
	      Vend( hdfId );
	      Hclose( hdfId );
	      hdfId = -1;
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
	  VSdetach( vDataId );
	  free(hdfData);
	  Tget_scTag(hdfScId, &hdfspacecraftTag);
	  
	  if ( hdfspacecraftTag == spacecraftTag && 
	       secTAI93_start >= hdfStartTai && secTAI93_start < hdfEndTai)
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
		  VSdetach( vDataId );
		  vDataId = -1;
		  Vend( hdfId );
		  Hclose( hdfId );
		  hdfId = -1;
		  return(PGSMEM_E_NO_MEMORY);
		}

	      totalRecords += ephNRecs;
	      recordCount   =  -1;
	      hdfRecord = 
		(PGSt_ephemRecord *) malloc(sizeof(PGSt_ephemRecord) * 
					    (ephNRecs+1) );
	      if (hdfRecord == NULL)
		{
		  /*ERROR callocing*/
		  sprintf(dynamicMsg, "Error allocating memory for "
			  "hdfRecord");
		  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					FUNCTION_NAME2);
		  VSdetach( vDataId );
		  vDataId = -1;
		  Vend( hdfId );
		  Hclose( hdfId );
		  hdfId = -1;
		  free(hdfData);
		  return(PGSMEM_E_NO_MEMORY);
		}
	      while ( recordCount++ < ephNRecs )
		{
		  /* Read a packed Vdata record */
		  (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
		  /* Unpack the packed Vdata record */
		  pntr = hdfData;
		  
		  memmove( &hdfRecord[recordCount].secTAI93,   pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].position[0],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].position[1],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].position[2],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].velocity[0],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].velocity[1],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].velocity[2],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].qualityFlag,pntr, sizeuint);
		}
	      /* Detach from the Vdata */
	      VSdetach( vDataId );
	      free(hdfData);
	      count=0;
	      totalRecords=ephNRecs;
	      total=ephNRecs;
	      if(totalRecords<BLOCK_SIZE)
		{
		  number = num+count+1;
		  pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&number,
						  hdfFile2 );
		  
		  if (pgsStatus == PGS_S_SUCCESS)
		    {     
		      if(file_exists(hdfFile2))
			{
			  /* Open the HDF format file */
			  hdfId2 = Hopen( hdfFile2, DFACC_RDONLY, 0 );
			  /* initialize the Vset interface */
			  Vstart( hdfId2 );
			  /* Get the reference number of the first Vdata 
			     in the file */
			  vDataRef = -1;
			  vDataRef = VSgetid( hdfId2, vDataRef );
			  /* Attact to the first Vdata in read mode */
			  vDataId = VSattach( hdfId2, vDataRef, "r" );
			  /* Get the list of field names */
			  VSinquire( vDataId, (int32 *)&hdrNRecs, 
				     (int32 *)&interlace,
				     fields, (int32 *)&vDataSize, vDataName );
			  /* Name the fields to be read */
			  (void) VSsetfields( vDataId, fields );
			  hdfData = 
			    (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+
						  100*DpCPrMaxUrLen*sizechar+
						  2*sizedbl+sizeint);
			  if (hdfData == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg, 
				      "Error allocating memory for "
				      "hdfData");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
						    dynamicMsg, 
						    FUNCTION_NAME2);
			      VSdetach( vDataId );
			      vDataId = -1;
			      Vend( hdfId );
			      Hclose( hdfId );
			      hdfId = -1;
			      if(hdfRecord != NULL)
				{
				  free(hdfRecord);
				  hdfRecord = NULL;
				}
			      return(PGSMEM_E_NO_MEMORY);
			    }
			  /* Read a packed Vdata record */
			  (void) VSread( vDataId, hdfData, 1, 
					      FULL_INTERLACE );
			  /* Unpack the packed Vdata record */
			  pntr = hdfData;
			  
			  memmove( &hdfScId, pntr, DpCPrMaxScIdLen*sizechar );
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
			  memmove( &hdfNOrbits, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfOrbitStart, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfOrbitEnd, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfRefFrame, pntr, 
				  DpCPrMaxFrameLen*sizechar );
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
			      memmove( &hdfURs[iElm][0], pntr, 
				       DpCPrMaxUrLen*sizechar );
			      pntr += DpCPrMaxUrLen*sizechar;
			    }
			  
			  /* Detach from the Vdata */
			  VSdetach( vDataId );
			  free(hdfData);
			  Tget_scTag(hdfScId, &hdfspacecraftTag);
			  
			  if ( hdfspacecraftTag == spacecraftTag )
			    {
			      ephNRecs = 0;
			      recordCount = 0;
			      /* Get the reference number of teh Vdata */
			      vDataRef = VSgetid( hdfId2, vDataRef );
			      /* Attach to the first Vdata in read mode */
			      vDataId = VSattach( hdfId2, vDataRef, "r" );
			      /* Get the list of field names */
			      VSinquire( vDataId, (int32 *)&ephNRecs, 
					 (int32 *)&interlace,
					 fields, (int32 *)&vDataSize, 
					 vDataName );
			      /* Name the fields to be read */
			      (void) VSsetfields( vDataId, fields );
			      hdfData = 
				(unsigned char *)malloc(sizeof(PGSt_ephemRecord)+1);
			      if (hdfData == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg, "Error allocating memory for "
					  "hdfData");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							dynamicMsg, 
							FUNCTION_NAME2);
				  VSdetach( vDataId );
				  vDataId = -1;
				  Vend( hdfId );
				  Hclose( hdfId );
				  hdfId = -1;
				  if(hdfRecord != NULL)
				    {
				      free(hdfRecord);
				      hdfRecord = NULL;
				    }
				  return(PGSMEM_E_NO_MEMORY);
				}

			    /*reallocate memory for hdfRecord */
			      
			      hdfRecord = 
				(PGSt_ephemRecord *)realloc((void*)hdfRecord, 
				     sizeof(PGSt_ephemRecord) * 
					 (totalRecords+ephNRecs+1) );
			      if (hdfRecord == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg, 
					  "Error allocating memory for "
					  "hdfRecord");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							dynamicMsg, 
							FUNCTION_NAME2);
				  VSdetach( vDataId );
				  vDataId = -1;
				  Vend( hdfId );
				  Hclose( hdfId );
				  hdfId = -1;
				  free(hdfData);
				  return(PGSMEM_E_NO_MEMORY);
				}

			      recordCount = -1;
			      while ( recordCount++ < ephNRecs )
				{
				  /* Read a packed Vdata record */
				  (void) VSread( vDataId, hdfData, 1, 
						 FULL_INTERLACE );
				  /* Unpack the packed Vdata record */
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
				  memmove( &hdfRecord[totalRecords+recordCount].qualityFlag, pntr, sizeuint )
				    ;
				} /* end while */
			      
			      total = totalRecords+recordCount-1;
			      totalRecords += ephNRecs;
			      
			      /* Detach from the Vdata */
			      VSdetach( vDataId );
			      free(hdfData);
			    } /* end check spacrcrafttag */
			  /* Close the Vset interface */
			  Vend( hdfId2 );
			  /* Close the HDF format file */
			  Hclose( hdfId2 );
			  
			} /* if(file_exists(hdfFile2)) */
		      count++;
		    } /* end if (pgsStatus == SUCCESS) */
		}
	      
	      if ( !(secTAI93_start < hdfRecord[0].secTAI93 || 
		     secTAI93_start > hdfRecord[total-1].secTAI93))
		{
		  gotData = PGS_TRUE;
		}
	      
	      gotData = PGS_FALSE;
	      ephRecordCounter = -1; /* if it stays -1 search failed */
	      endRecord = total;     /* too big on purpose; will not be used
					as such */
	      midRecord = (endRecord + ephRecordCounter)/2;
	      
	      while(endRecord > ephRecordCounter + 1)
		{
		  midRecord = (endRecord + ephRecordCounter)/2;
		  if(secTAI93_start == hdfRecord[midRecord].secTAI93)
		    {
		      ephRecordCounter = midRecord;
		      gotData = PGS_TRUE;
		      break;
		    }
		  if(secTAI93_start > hdfRecord[midRecord].secTAI93)
		    {
		      ephRecordCounter = midRecord;
		    }
		  else
		    {
		      endRecord   =  midRecord;
		    }
		  gotData = (ephRecordCounter >=0 &&
			     ephRecordCounter <= total - 1);
		}  /* end while */
	      if (gotData == PGS_TRUE)
		{
		  if (hdfRecord[ephRecordCounter].secTAI93 < secTAI93_start)
		    { 
		      ephstartIndex = ephRecordCounter+1; 
		    }
		  
		  if(secTAI93_start <= hdfRecord[ephRecordCounter].secTAI93)
		    {
		      ephstartIndex = ephRecordCounter;
		    }
		} 
	      else  /* something went wrong */
		{
		  returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
		  PGS_SMF_SetDynamicMsg(returnStatus,
		   "search for data record within ephemeris file whose header"
		   " time range included this time has failed", 
					FUNCTION_NAME2);
		}
	    } /*end of spacecraftTag check for start time */
	  else
	    {
	      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	    }
	  /* Close the Vset interface */
	  Vend( hdfId );
	  /* Close the HDF format file */
	  Hclose( hdfId );
	  
	} /* if(file_exists(hdfFile)) */
      else
	{
	  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	}
      
      /* free hdfRecord memory */
      if(hdfRecord != NULL)
	{
	  free(hdfRecord);
	  hdfRecord = NULL;
	}

      /***********Repeat for Stop Time***************************/
      
      file_numb = num;
      pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA,&file_numb, 
				      hdfFile3 );
      
      if (pgsStatus != PGS_S_SUCCESS)
	{
	  if(orbFlag == PGS_TRUE)
	    {
	      PGS_SMF_SetDynamicMsg(pgsStatus,
			       "Error getting reference to HDF format files", 
			       FUNCTION_NAME2);
	      return pgsStatus;
	    }
	  else
	    {
	      DidNotGetEPH = 1;
	      strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
	      PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME2);
	      goto GET_ATT;
	    }
	}
      if(file_exists(hdfFile3))
	{
	  /* Open the HDF format file */
	  hdfId3 = Hopen( hdfFile3, DFACC_RDONLY, 0 );
	  /* initialize the Vset interface */
	  Vstart( hdfId3 );
	  /* Get the reference number of the first Vdata in the file */
	  vDataRef = -1;
	  vDataRef = VSgetid( hdfId3, vDataRef );
	  /* Attact to the first Vdata in read mode */
	  vDataId = VSattach( hdfId3, vDataRef, "r" );
	  /* Get the list of field names */
	  VSinquire( vDataId, (int32 *)&hdrNRecs, (int32 *)&interlace,
		     fields, (int32 *)&vDataSize, vDataName );
	  /* Name the fields to be read */
	  (void) VSsetfields( vDataId, fields );
	  
	  hdfData = (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+
					     100*DpCPrMaxUrLen*sizechar+
					     2*sizedbl+sizeint);
	  if (hdfData == NULL)
	    {
	      /*ERROR callocing*/
	      sprintf(dynamicMsg, "Error allocating memory for "
		      "hdfData");
	      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
				    FUNCTION_NAME2);
	      VSdetach( vDataId );
	      vDataId = -1;
	      Vend( hdfId );
	      Hclose( hdfId );
	      hdfId = -1;
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
	  VSdetach( vDataId );
	  free(hdfData);
	  Tget_scTag(hdfScId, &hdfspacecraftTag);
	  
	  if ( hdfspacecraftTag == spacecraftTag && 
	       secTAI93_stop > hdfStartTai && secTAI93_stop < hdfEndTai)
	    {
	      /* Get the reference number of the Vdata */
	      vDataRef = VSgetid( hdfId3, vDataRef );
	      /* Attach to the first Vdata in read mode */
	      vDataId = VSattach( hdfId3, vDataRef, "r" );
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
		  VSdetach( vDataId );
		  vDataId = -1;
		  Vend( hdfId );
		  Hclose( hdfId );
		  hdfId = -1;
		  return(PGSMEM_E_NO_MEMORY);
		}

	      totalRecords += ephNRecs;
	      recordCount   =  -1;
	      hdfRecord = 
		(PGSt_ephemRecord *) malloc(sizeof(PGSt_ephemRecord) * 
					    (ephNRecs+1) );
	      if (hdfRecord == NULL)
		{
		  /*ERROR callocing*/
		  sprintf(dynamicMsg, "Error allocating memory for "
			  "hdfRecord");
		  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					FUNCTION_NAME2);
		  VSdetach( vDataId );
		  vDataId = -1;
		  Vend( hdfId );
		  Hclose( hdfId );
		  hdfId = -1;
		  free(hdfData);
		  return(PGSMEM_E_NO_MEMORY);
		}
	      while ( recordCount++ < ephNRecs )
		{
		  /* Read a packed Vdata record */
		  (void) VSread( vDataId, hdfData, 1, FULL_INTERLACE );
		  /* Unpack the packed Vdata record */
		  pntr = hdfData;
		  
		  memmove( &hdfRecord[recordCount].secTAI93,   pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].position[0],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].position[1],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].position[2],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].velocity[0],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].velocity[1],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].velocity[2],pntr, sizedbl );
		  pntr += sizedbl;
		  memmove( &hdfRecord[recordCount].qualityFlag,pntr, sizeuint);
		}
	      /* Detach from the Vdata */
	      VSdetach( vDataId );
	      free(hdfData);
	      count=0;
	      totalRecords=ephNRecs;
	      total=ephNRecs;
	      if(totalRecords<BLOCK_SIZE)
		{
		  number = num+count+1;
		  pgsStatus = PGS_PC_GetReference(PGSd_SC_EPHHDF_DATA, &number,
						  hdfFile4 );
		  
		  if (pgsStatus == PGS_S_SUCCESS)
		    {
		      
		      if(file_exists(hdfFile4))
			{
			  /* Open the HDF format file */
			  hdfId4 = Hopen( hdfFile4, DFACC_RDONLY, 0 );
			  /* initialize the Vset interface */
			  Vstart( hdfId2 );
			  /* Get the reference number of the first Vdata in the file */
			  vDataRef = -1;
			  vDataRef = VSgetid( hdfId4, vDataRef );
			  /* Attact to the first Vdata in read mode */
			  vDataId = VSattach( hdfId4, vDataRef, "r" );
			  /* Get the list of field names */
			  VSinquire( vDataId, (int32 *)&hdrNRecs, 
				     (int32 *)&interlace,
				     fields, (int32 *)&vDataSize, vDataName );
			  /* Name the fields to be read */
			  (void) VSsetfields( vDataId, fields );
			  hdfData = 
			    (unsigned char *)malloc (sizeof(PGSt_ephemHeader)+
						     100*DpCPrMaxUrLen*
						     sizechar+2*sizedbl+
						     sizeint);
			  if (hdfData == NULL)
			    {
			      /*ERROR callocing*/
			      sprintf(dynamicMsg, 
				      "Error allocating memory for "
				      "hdfData");
			      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
						    dynamicMsg, 
						    FUNCTION_NAME2);
			      VSdetach( vDataId );
			      vDataId = -1;
			      Vend( hdfId );
			      Hclose( hdfId );
			      hdfId = -1;
			      if(hdfRecord != NULL)
				{
				  free(hdfRecord);
				  hdfRecord = NULL;
				}
			      return(PGSMEM_E_NO_MEMORY);
			    }
			  /* Read a packed Vdata record */
			  (void) VSread( vDataId, hdfData, 1, 
					      FULL_INTERLACE );
			  /* Unpack the packed Vdata record */
			  pntr = hdfData;
			  memmove( &hdfScId, pntr, DpCPrMaxScIdLen*sizechar );
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
			  memmove( &hdfNOrbits, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfOrbitStart, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfOrbitEnd, pntr, sizeuint );
			  pntr += sizeuint;
			  memmove( &hdfRefFrame, pntr, 
				  DpCPrMaxFrameLen*sizechar );
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
			    for (iElm=0; iElm<counter; iElm++)
			      {
				memmove( &hdfURs[iElm][0], pntr, 
					 DpCPrMaxUrLen*sizechar );
				pntr += DpCPrMaxUrLen*sizechar;
			      }
			  
			  /* Detach from the Vdata */
			  VSdetach( vDataId );
			  free(hdfData);
			  Tget_scTag(hdfScId, &hdfspacecraftTag);
			  
			  if ( hdfspacecraftTag == spacecraftTag )
			    {
			      ephNRecs = 0;
			      recordCount = 0;
			      /* Get the reference number of the Vdata */
			      vDataRef = VSgetid( hdfId4, vDataRef );
			      /* Attach to the first Vdata in read mode */
			      vDataId = VSattach( hdfId4, vDataRef, "r" );
			      /* Get the list of field names */
			      VSinquire( vDataId, (int32 *)&ephNRecs, 
					 (int32 *)&interlace,
					 fields, (int32 *)&vDataSize, 
					 vDataName );
			      /* Name the fields to be read */
			      (void) VSsetfields( vDataId, fields );
			      hdfData = 
				(unsigned char *)malloc( sizeof(PGSt_ephemRecord)+1);

			      if (hdfData == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg, "Error allocating memory for "
					  "hdfData");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							dynamicMsg, 
							FUNCTION_NAME2);
				  VSdetach( vDataId );
				  vDataId = -1;
				  Vend( hdfId );
				  Hclose( hdfId );
				  hdfId = -1;
				  if(hdfRecord != NULL)
				    {
				      free(hdfRecord);
				      hdfRecord = NULL;
				    }
				  return(PGSMEM_E_NO_MEMORY);
				}

			    /*reallocate memory for hdfRecord */
			      
			      hdfRecord = 
				(PGSt_ephemRecord *)realloc((void*)hdfRecord, 
					      sizeof(PGSt_ephemRecord) * 
						  (totalRecords+ephNRecs+1) );
			      if (hdfRecord == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg, 
					  "Error allocating memory for "
					  "hdfRecord");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
							FUNCTION_NAME2);
				  VSdetach( vDataId );
				  vDataId = -1;
				  Vend( hdfId );
				  Hclose( hdfId );
				  hdfId = -1;
				  free(hdfData);
				  return(PGSMEM_E_NO_MEMORY);
				}

			      recordCount = -1;
			      while ( recordCount++ < ephNRecs )
				{
				  /* Read a packed Vdata record */
				  (void) VSread( vDataId, hdfData, 1, 
						      FULL_INTERLACE );
				  /* Unpack the packed Vdata record */
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
				  memmove( &hdfRecord[totalRecords+recordCount].qualityFlag, pntr, sizeuint )
				    ;
				} /* end while */
			      
			      total = totalRecords+recordCount-1;
			      totalRecords += ephNRecs;
			      
			      /* Detach from the Vdata */
			      VSdetach( vDataId );
			      free(hdfData);
			    } /* end check spacrcrafttag */
			  /* Close the Vset interface */
			  Vend( hdfId4 );
			  /* Close the HDF format file */
			  Hclose( hdfId4 );
			  
			} /* if(file_exists(hdfFile4)) */
		      count++;
		    } /* end if (pgsStatus == SUCCESS) */
		}
	      if ( !(secTAI93_stop < hdfRecord[0].secTAI93 || 
		     secTAI93_stop > hdfRecord[total-1].secTAI93))
		{
		  gotData = PGS_TRUE;
		}
	      
	      gotData = PGS_FALSE;
	      ephRecordCounter = -1; /* if it stays -1 search failed */
	      endRecord = total;     /* too big on purpose; will not be used
					as such */
	      midRecord = (endRecord + ephRecordCounter)/2;
	      
	      while(endRecord > ephRecordCounter + 1)
		{
		  midRecord = (endRecord + ephRecordCounter)/2;
		  if(secTAI93_stop == hdfRecord[midRecord].secTAI93)
		    {
		      ephRecordCounter = midRecord;
		      gotData = PGS_TRUE;
		      break;
		    }
		  if(secTAI93_stop > hdfRecord[midRecord].secTAI93)
		    {
		      ephRecordCounter = midRecord;
		    }
		  else
		    {
		      endRecord   =  midRecord;
		    }
		  gotData = (ephRecordCounter >=0 &&
			     ephRecordCounter <= total - 1);
		}  /* end while */
	      if (gotData == PGS_TRUE)
		{
		  if (hdfRecord[ephRecordCounter].secTAI93 <= secTAI93_stop)
		    {
		      ephstopIndex = ephRecordCounter;
		    }
		  
		  if(secTAI93_stop < hdfRecord[ephRecordCounter].secTAI93)
		    {
		      ephstopIndex = ephRecordCounter-1;
		    }
		}
	      else  /* something went wrong */
		{
		  returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
		  PGS_SMF_SetDynamicMsg(returnStatus,
		   "search for data record within ephemeris file whose header"
		   " time range included this time has failed", 
					FUNCTION_NAME2);
		}
	      NUM = (ephstopIndex - ephstartIndex);
	      /* do not let *numvalues_Eph exceed max of user 
		 provided *numvalues_Eph and NUM */
	      
	      if(NUM < input_numvalues_Eph)
		{
		  *numvalues_Eph = NUM;
		}
	      else
		{
		  *numvalues_Eph = input_numvalues_Eph;
		}
	      
	      /* if we got to this point, and *numvalues_Eph is <= 0, the 
		 user should get one value */
	      
	      if(*numvalues_Eph <= 0) *numvalues_Eph = 1;
	      
	      for(j=0; j < *numvalues_Eph; j++)
		{
		  returnStatus = 
		    PGS_TD_TAItoUTC(hdfRecord[j + ephstartIndex].secTAI93, 
				    asciiUTC_Eph[j]);  
		  
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
		      
		      /* free hdfRecord memory */
		      if(hdfRecord != NULL)
			{
			  free(hdfRecord);
			  hdfRecord = NULL;
			}
		      if(orbFlag == PGS_TRUE)
			{
			  return returnStatus;
			}
		      else
			{
			  DidNotGetEPH = 1;
			  strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
			  PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME2);
			  goto GET_ATT;
			}
		    default:
		      if(orbFlag == PGS_TRUE)
			{
			  PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME2);
			  
			  /* free hdfRecord memory */
			  if(hdfRecord != NULL)
			    {
			      free(hdfRecord);
			      hdfRecord = NULL;
			    }
			  
			  return PGS_E_TOOLKIT;
			}
		      else
			{
			  DidNotGetEPH = 1;
			  strcpy(msg,"Unable to get ephem records for the requested time period, uninterpolated attitude quaternions cannot be calculated");
			  PGS_SMF_SetDynamicMsg(PGSEPH_W_BAD_EPHEM_VALUE,msg,FUNCTION_NAME2);
			  goto GET_ATT;
			}
		    }
		  
		  qualityFlags[j][0] = hdfRecord[j + ephstartIndex].qualityFlag;
		  for (count = 0; count < 3; count++ )
		    {
		      positionECI[j][count] = 
			hdfRecord[j + ephstartIndex].position[count];
		      velocityECI[j][count] = 
			hdfRecord[j + ephstartIndex].velocity[count];
		    }
		}
	    } /*end of spacecraftTag check for stop time */
	  else
	    {
	      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	    }
	  /* Close the Vset interface */
	  Vend( hdfId3 );
	  /* Close the HDF format file */
	  Hclose( hdfId3 );
	  
	} /* end of if(file_exists(hdfFile3)) */
      else
	{
	  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	}
      if(gotData == PGS_TRUE)
	{ 
	  returnStatus = PGS_S_SUCCESS;
	}

      /* free hdfRecord memory */
      if(hdfRecord != NULL)
	{
	  free(hdfRecord);
	  hdfRecord = NULL;
	}
      if(gotData == PGS_TRUE) break;
    } /* end for(file_num=0 */
  
  
 GET_ATT:  /* skip to here if we have any problem, other than memory 
	      allocation in retrieving eph data */
  
  
  if (attFlag == PGS_TRUE )
    {
      
      PGS_PC_GetNumberOfFiles(PGSd_SC_ATTHDF_DATA, &num_files);
      
      for ( num = 1; num <= num_files; num++)
	{	  
	  file_num = num;
	  pgsStatus = PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &file_num,
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
	      VSinquire( vDataIdAtt, (int32 *)&hdrNRecsAtt, 
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
		  VSdetach( vDataIdAtt );
		  vDataIdAtt = -1;
		  Vend( hdfIdAtt );
		  Hclose( hdfIdAtt );
		  hdfIdAtt = -1;
		  if(hdfRecord != NULL)
		    {
		      free(hdfRecord);
		      hdfRecord = NULL;
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
	      memmove( &hdfEulerAngleOrder[0], pntr, 
		      DpCPrMaxEulerAngleOrder*sizeuint );
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
	      VSdetach( vDataIdAtt );
	      free(hdfDataAtt);
	      Tget_scTag(hdfScId, &hdfspacecraftTag);
	      
	      if ( hdfspacecraftTag == spacecraftTag && 
		   secTAI93_start >= hdfStartTai && secTAI93_start < hdfEndTai)
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
		  hdfDataAtt = 
		    (unsigned char *)malloc(sizeof(PGSt_attitRecord)+1);
		  if (hdfDataAtt == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfDataAtt");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					    FUNCTION_NAME2);
		      VSdetach( vDataIdAtt );
		      vDataIdAtt = -1;
		      Vend( hdfIdAtt );
		      Hclose( hdfIdAtt );
		      hdfIdAtt = -1;
		      if(hdfRecord != NULL)
			{
			  free(hdfRecord);
			  hdfRecord = NULL;
			}
		      return(PGSMEM_E_NO_MEMORY);
		    }

		  recordCount         =  -1;
		  hdfRecordAtt = 
		    (PGSt_attitRecord *) malloc(sizeof(PGSt_attitRecord) * 
						(attNRecs+1) );
		  if (hdfRecordAtt == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfRecordAtt");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					    FUNCTION_NAME2);
		      VSdetach( vDataIdAtt );
		      vDataIdAtt = -1;
		      Vend( hdfIdAtt );
		      Hclose( hdfIdAtt );
		      hdfIdAtt = -1;
		      free(hdfDataAtt);
		      if(hdfRecord != NULL)
			{
			  free(hdfRecord);
			  hdfRecord = NULL;
			}
		      return(PGSMEM_E_NO_MEMORY);
		    }
		  while ( recordCount++ < attNRecs )
		    {
		      /* Read a packed Vdata record */
		      (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
		      /* Unpack the packed Vdata record */
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
		    } /* end while */
		  /* Detach from the Vdata */
		  VSdetach( vDataIdAtt );
		  free(hdfDataAtt);
		  count=0;
		  totalRecords=attNRecs;
		  total=attNRecs;
		  if(totalRecords<BLOCK_SIZE)
		    {
		      number = num+count+1;
		      pgsStatus = 
			PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &number,
					    hdfFileAtt2 );
		      if (pgsStatus == PGS_S_SUCCESS)
			{
			  
			  if(file_exists(hdfFileAtt2))
			    {
			      /* Open the HDF format file */
			      hdfIdAtt2 = Hopen( hdfFileAtt2, DFACC_RDONLY, 0);
			      /* initialize the Vset interface */
			      Vstart( hdfIdAtt2 );
			      /* Get the reference number of the first 
				 Vdata in the file */
			      vDataRefAtt = -1;
			      vDataRefAtt = VSgetid( hdfIdAtt2, vDataRefAtt );
			      /* Attach to the first Vdata in read mode */
			      vDataIdAtt = VSattach( hdfIdAtt2, vDataRefAtt, 
						     "r" );
			      /* Get the list of field names */
			      VSinquire( vDataIdAtt, (int32 *)&hdrNRecsAtt,
					 (int32 *)&interlaceAtt,
					 fieldsAtt, (int32 *)&vDataSizeAtt, 
					 vDataNameAtt );
			      /* Name the fields to be read */
			      (void) VSsetfields( vDataIdAtt, fieldsAtt );
			      hdfDataAtt = 
			      (unsigned char *)malloc(sizeof(PGSt_attitHeader)+
						     100*DpCPrMaxUrLen*sizechar
								   );
			      if (hdfDataAtt == NULL)
				{
				  /*ERROR callocing*/
				  sprintf(dynamicMsg, 
					  "Error allocating memory for "
					  "hdfDataAtt");
				  PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							dynamicMsg, 
							FUNCTION_NAME2);
				  VSdetach( vDataIdAtt );
				  vDataIdAtt = -1;
				  Vend( hdfIdAtt );
				  Hclose( hdfIdAtt );
				  hdfIdAtt = -1;
				  if(hdfRecord != NULL)
				    {
				      free(hdfRecord);
				      hdfRecord = NULL;
				    }
				  if(hdfRecordAtt != NULL)
				    {
				      free(hdfRecordAtt);
				      hdfRecordAtt = NULL;
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
			      VSdetach( vDataIdAtt );
			      free(hdfDataAtt);
			      Tget_scTag(hdfScId, &hdfspacecraftTag);
			      
			      if ( hdfspacecraftTag == spacecraftTag )
				{
				  attNRecs = 0;
				  recordCount = 0;
				  
				  /* Get the reference number or the Vdata */
				  vDataRefAtt = VSgetid( hdfIdAtt2, 
							 vDataRefAtt );
				  /* Attach to the first Vdata in read mode */
				  vDataIdAtt = VSattach( hdfIdAtt2,
							 vDataRefAtt, "r" );
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
				      VSdetach( vDataIdAtt );
				      vDataIdAtt = -1;
				      Vend( hdfIdAtt );
				      Hclose( hdfIdAtt );
				      hdfIdAtt = -1;
				      if(hdfRecord != NULL)
					{
					  free(hdfRecord);
					  hdfRecord = NULL;
					}
				      if(hdfRecordAtt != NULL)
					{
					  free(hdfRecordAtt);
					  hdfRecordAtt = NULL;
					}
				      return(PGSMEM_E_NO_MEMORY);
				    }
				  
				  /*reallocate memory for hdfRecordAtt */
				  
				  hdfRecordAtt = 
				    (PGSt_attitRecord *)realloc((void*)hdfRecordAtt, 
						 sizeof(PGSt_attitRecord) * 
						  (totalRecords+attNRecs+1) );
				  if (hdfRecordAtt == NULL)
				    {
				      /*ERROR callocing*/
				      sprintf(dynamicMsg, 
					      "Error allocating memory for "
					      "hdfRecordAtt");
				      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							    dynamicMsg, 
							    FUNCTION_NAME2);
				      VSdetach( vDataId );
				      vDataId = -1;
				      Vend( hdfId );
				      Hclose( hdfId );
				      hdfId = -1;
				      free(hdfData);
				      return(PGSMEM_E_NO_MEMORY);
				    }

				  recordCount =  -1;
				  while ( recordCount++ < attNRecs )
				    {
				      /* Read a packed Vdata record */
				      (void) VSread( vDataIdAtt, 
							  hdfDataAtt, 1, 
							  FULL_INTERLACE );
				      /* Unpack the packed Vdata record */
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
				    } /* end while */
				  total = totalRecords+recordCount-1;
				  totalRecords += attNRecs;
				  
				  /* Detach from the Vdata */
				  VSdetach( vDataIdAtt );
				  free(hdfDataAtt);
				} /* end check spacrcrafttag */
			      /* Close the Vset interface */
			      Vend( hdfIdAtt2 );
			      /* Close the HDF format file */
			      Hclose( hdfIdAtt2 );
			    } /* if(file_exists(hdfFileAtt2)) */
			  count++;
			} /* end if (pgsStatus == SUCCESS) */
		    }
		  
		  gotData = PGS_FALSE;
		  if ( !(secTAI93_start < hdfRecordAtt[0].secTAI93 || 
			 secTAI93_start > hdfRecordAtt[total-1].secTAI93))
		    {
		      gotData = PGS_TRUE;
		    }
		  gotData = PGS_FALSE;
		  attRecordCounter = -1; /* if it stays -1 search failed */
		  endRecord = total;  /* too big on purpose; will not be used
					 as such */
		  midRecord = (endRecord + attRecordCounter)/2;
		  while(endRecord > attRecordCounter + 1)
		    {
		      midRecord = (endRecord + attRecordCounter)/2;
		      if(secTAI93_start == hdfRecordAtt[midRecord].secTAI93)
			{
			  attRecordCounter = midRecord;
			  gotData = PGS_TRUE;
			  break;
			}
		      if(secTAI93_start > hdfRecordAtt[midRecord].secTAI93)
			{
			  attRecordCounter = midRecord;
			}
		      else
			{
			  endRecord   =  midRecord;
			}
		      gotData = (attRecordCounter >=0 &&
				 attRecordCounter <= total - 1);
		    }  /* end while */

		  if (gotData == PGS_TRUE)
		    {
		      if (hdfRecordAtt[attRecordCounter].secTAI93 < 
			  secTAI93_start)
			{
			  attstartIndex = attRecordCounter+1;
			}
		      if(secTAI93_start <=
			 hdfRecordAtt[attRecordCounter].secTAI93)
			{
			  attstartIndex = attRecordCounter;
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
		}/*end of spacecraft check for attitude start time */
	      else
		{
		  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		}
	      /*close the Vset interface */
	      Vend( hdfIdAtt);
	      /* close the HDF file */
	      Hclose(hdfIdAtt);
	    } /* end if(file_exits(hdfFileAtt) */
	  
	  /* free hdfRecord memory */
	  if(hdfRecord != NULL)
	    {
	      free(hdfRecord);
	      hdfRecord = NULL;
	    }
	  if(hdfRecordAtt != NULL)
	    {
	      free(hdfRecordAtt);
	      hdfRecordAtt = NULL;
	    }
	  /*****Repeat for att stop time ************************/   
	  
	  file_numb = num;
	  pgsStatus = PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &file_numb, 
					  hdfFileAtt3 );
	  
	  if (pgsStatus != PGS_S_SUCCESS)
	    {
	      PGS_SMF_SetDynamicMsg(pgsStatus,
				"Error getting reference to HDF format files", 
			        FUNCTION_NAME2);
	      return pgsStatus;
	    }
	  
	  if(file_exists(hdfFileAtt3))
	    {
	      /* Open the HDF format file */
	      hdfIdAtt3 = Hopen( hdfFileAtt3, DFACC_RDONLY, 0 );
	      /* initialize the Vset interface */
	      Vstart( hdfIdAtt3 );
	      /* Get the reference number of the first Vdata in the file */
	      vDataRefAtt = -1;
	      vDataRefAtt = VSgetid( hdfIdAtt3, vDataRefAtt );
	      /* Attach to the first Vdata in read mode */
	      vDataIdAtt = VSattach( hdfIdAtt3, vDataRefAtt, "r" );
	      /* Get the list of field names */
	      VSinquire( vDataIdAtt, (int32 *)&hdrNRecsAtt, 
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
		  VSdetach( vDataIdAtt );
		  vDataIdAtt = -1;
		  Vend( hdfIdAtt );
		  Hclose( hdfIdAtt );
		  hdfIdAtt = -1;
		  if(hdfRecord != NULL)
		    {
		      free(hdfRecord);
		      hdfRecord = NULL;
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
	      memmove( &hdfEulerAngleOrder[0], pntr, 
		      DpCPrMaxEulerAngleOrder*sizeuint );
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
	      VSdetach( vDataIdAtt );
	      free(hdfDataAtt);
	      Tget_scTag(hdfScId, &hdfspacecraftTag);
	      
	      if ( hdfspacecraftTag == spacecraftTag && 
		   secTAI93_stop >= hdfStartTai && secTAI93_stop < hdfEndTai)
		{
		  /* Get the reference number or the Vdata */
		  vDataRefAtt = VSgetid( hdfIdAtt3, vDataRefAtt );
		  /* Attach to the first Vdata in read mode */
		  vDataIdAtt = VSattach( hdfIdAtt3, vDataRefAtt, "r" );
		  /* Get the list of field names */
		  VSinquire( vDataIdAtt, (int32 *)&attNRecs, 
			     (int32 *)&interlaceAtt,
			     fieldsAtt, (int32 *)&vDataSizeAtt, vDataNameAtt );
		  /* Name the fields to be read */
		  (void) VSsetfields( vDataIdAtt, fieldsAtt );
		  hdfDataAtt = 
		    (unsigned char *)malloc(sizeof(PGSt_attitRecord));
		  if (hdfDataAtt == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfDataAtt");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					    FUNCTION_NAME2);
		      VSdetach( vDataIdAtt );
		      vDataIdAtt = -1;
		      Vend( hdfIdAtt );
		      Hclose( hdfIdAtt );
		      hdfIdAtt = -1;
		      if(hdfRecord != NULL)
			{
			  free(hdfRecord);
			  hdfRecord = NULL;
			}
		      return(PGSMEM_E_NO_MEMORY);
		    }

		  recordCount         =  -1;

		  /*reallocate memory for hdfRecordAtt */
		  
		  hdfRecordAtt = (PGSt_attitRecord *)realloc((void*)hdfRecordAtt, sizeof(PGSt_attitRecord) * (totalRecords+attNRecs+1) );
		  if (hdfRecordAtt == NULL)
		    {
		      /*ERROR callocing*/
		      sprintf(dynamicMsg, "Error allocating memory for "
			      "hdfRecordAtt");
		      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, dynamicMsg, 
					    FUNCTION_NAME2);
		      VSdetach( vDataIdAtt );
		      vDataIdAtt = -1;
		      Vend( hdfIdAtt );
		      Hclose( hdfIdAtt );
		      hdfIdAtt = -1;
		      free(hdfDataAtt);
		      if(hdfRecord != NULL)
			{
			  free(hdfRecord);
			  hdfRecord = NULL;
			}
		      return(PGSMEM_E_NO_MEMORY);
		    }
		  while ( recordCount++ < attNRecs )
		    {
		      /* Read a packed Vdata record */
		      (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
		      /* Unpack the packed Vdata record */
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
		    } /* end while */
		  /* Detach from the Vdata */
		  VSdetach( vDataIdAtt );
		  free(hdfDataAtt);
		  count=0;
		  totalRecords=attNRecs;
		  total=attNRecs;
		  if(totalRecords<BLOCK_SIZE)
		    {
		      number = num+count+1;
		      pgsStatus = 
			PGS_PC_GetReference(PGSd_SC_ATTHDF_DATA, &number,
					    hdfFileAtt4 );
		      if (pgsStatus == PGS_S_SUCCESS)
			{
			  if(file_exists(hdfFileAtt4))
			    {
			      hdfIdAtt4 = Hopen( hdfFileAtt4, DFACC_RDONLY, 0);
			      /* initialize the Vset interface */
			      Vstart( hdfIdAtt4 );
			      /* Get the reference number of the first 
				 Vdata in the file */
			      vDataRefAtt = -1;
			      vDataRefAtt = VSgetid( hdfIdAtt4, vDataRefAtt );
			      /* Attach to the first Vdata in read mode */
			      vDataIdAtt = VSattach( hdfIdAtt4, vDataRefAtt, 
						     "r" );
			      /* Get the list of field names */
			      VSinquire( vDataIdAtt, (int32 *)&hdrNRecsAtt, 
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
				  VSdetach( vDataIdAtt );
				  vDataIdAtt = -1;
				  Vend( hdfIdAtt );
				  Hclose( hdfIdAtt );
				  hdfIdAtt = -1;
				  if(hdfRecord != NULL)
				    {
				      free(hdfRecord);
				      hdfRecord = NULL;
				    }
				  if(hdfRecordAtt != NULL)
				    {
				      free(hdfRecordAtt);
				      hdfRecordAtt = NULL;
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
			      VSdetach( vDataIdAtt );
			      free(hdfDataAtt);
			      Tget_scTag(hdfScId, &hdfspacecraftTag);
			      
			      if ( hdfspacecraftTag == spacecraftTag )
				{
				  attNRecs = 0;
				  recordCount = 0;
				  /* Get the reference number or the Vdata */
				  vDataRefAtt = VSgetid( hdfIdAtt4, vDataRefAtt);
				  /* Attach to the first Vdata in read mode */
				  vDataIdAtt = VSattach( hdfIdAtt4, vDataRefAtt,
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
				      VSdetach( vDataIdAtt );
				      vDataIdAtt = -1;
				      Vend( hdfIdAtt );
				      Hclose( hdfIdAtt );
				      hdfIdAtt = -1;
				      if(hdfRecord != NULL)
					{
					  free(hdfRecord);
					  hdfRecord = NULL;
					}
				      if(hdfRecordAtt != NULL)
					{
					  free(hdfRecordAtt);
					  hdfRecordAtt = NULL;
					}
				      return(PGSMEM_E_NO_MEMORY);
				    }
				  
				  /*reallocate memory for hdfRecordAtt */
				  
				  hdfRecordAtt = (PGSt_attitRecord *)realloc((void*)hdfRecordAtt, sizeof(PGSt_attitRecord) * (totalRecords+attNRecs+1) );
				  if (hdfRecordAtt == NULL)
				    {
				      /*ERROR callocing*/
				      sprintf(dynamicMsg, 
					      "Error allocating memory for "
					      "hdfRecordAtt");
				      PGS_SMF_SetDynamicMsg(PGSMEM_E_NO_MEMORY, 
							    dynamicMsg, 
							    FUNCTION_NAME2);
				      VSdetach( vDataId );
				      vDataId = -1;
				      Vend( hdfId );
				      Hclose( hdfId );
				      hdfId = -1;
				      free(hdfData);
				      return(PGSMEM_E_NO_MEMORY);
				    }
				  
				  recordCount =  -1;
				  while ( recordCount++ < attNRecs )
				    {
				      /* Read a packed Vdata record */
				      (void) VSread( vDataIdAtt, hdfDataAtt, 1, FULL_INTERLACE );
				      /* Unpack the packed Vdata record */
				      pntr = hdfDataAtt;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].secTAI93,    pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[0], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[1], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].eulerAngle[2], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[0], pntr,sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[1], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].angularVelocity[2], pntr, sizedbl );
				      pntr += sizedbl;
				      memmove( &hdfRecordAtt[totalRecords+recordCount].qualityFlag, pntr, sizeuint );
				    } /* end while */
				  total = totalRecords+recordCount-1;
				  totalRecords += attNRecs;
				  
				  /* Detach from the Vdata */
				  VSdetach( vDataIdAtt );
				  free(hdfDataAtt);
				}  /* end if check spacecrafttag */
			      /* Close the Vset interface */
			      Vend( hdfIdAtt4 );
			      /* Close the HDF format file */
			      Hclose( hdfIdAtt4 );
			    } /* if(file_exists(hdfFileAtt4)) */
			  count++;
			} /* end if (pgsStatus_SUCCESS) */
		    }
		  gotData = PGS_FALSE;
		  if ( !(secTAI93_stop < hdfRecordAtt[0].secTAI93 || 
			 secTAI93_stop > hdfRecordAtt[total-1].secTAI93))
		    {
		      gotData = PGS_TRUE;
		    }
		  gotData = PGS_FALSE;
		  attRecordCounter = -1; /* if it stays -1 search failed */
		  endRecord = total;  /* too big on purpose; will not be used
					 as such */
		  
		  midRecord = (endRecord + attRecordCounter)/2;
		  while(endRecord > attRecordCounter + 1)
		    {
		      
		      midRecord = (endRecord + attRecordCounter)/2;
		      if(secTAI93_stop == hdfRecordAtt[midRecord].secTAI93)
			{
			  attRecordCounter = midRecord;
			  gotData = PGS_TRUE;
			  break;
			}
		      if(secTAI93_stop > hdfRecordAtt[midRecord].secTAI93)
			{
			  attRecordCounter = midRecord;
			}
		      else
			{
			  endRecord   =  midRecord;
			}
		      gotData = (attRecordCounter >=0 &&
				 attRecordCounter <= total - 1);
		    }  /* end while */

		  if (gotData == PGS_TRUE)
		    {
		      if (hdfRecordAtt[attRecordCounter].secTAI93 <= 
			  secTAI93_stop)
			{
			  attstopIndex = attRecordCounter;
			}
		      if(secTAI93_stop < 
			 hdfRecordAtt[attRecordCounter].secTAI93)
			{
			  attstopIndex = attRecordCounter-1;
			}
		    }
		  else /* something went wrong */
		    {
		      returnStatus = PGSEPH_E_BAD_EPHEM_FILE_HDR;
		      return returnStatus; 
		    }
		  NUMA = ( attstopIndex - attstartIndex );
		  if(NUMA < input_numvalues_Att)
		    {
		      *numvalues_Att = NUMA;
		    }
		  else
		    {
		      *numvalues_Att = input_numvalues_Att;
		    }
		  /* if we got to this point, and *numvalues is <= 0, the 
		     user should get one record */
		  
		  if(*numvalues_Att <= 0) *numvalues_Att = 1;
		  
		  
		  if((orbFlag == PGS_TRUE && 
		      attFlag == PGS_TRUE && DidNotGetEPH == 0) ||
		     (orbFlag == PGS_FALSE && 
		      attFlag == PGS_TRUE && DidNotGetEPH == 0))
		    {
		      for(j=0; j < *numvalues_Att; j++)
			{
			  returnStatus = 
			    PGS_TD_TAItoUTC(hdfRecordAtt[j + attstartIndex].secTAI93, 
					    asciiUTC_Att[j]);  
			  
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
			      
			      /* free hdfRecord memory */
			      if(hdfRecordAtt != NULL)
				{
				  free(hdfRecordAtt);
				  hdfRecordAtt = NULL;
				}
			      return returnStatus;
			    default:
			      PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME2);
			      
			      /* free hdfRecord memory */
			      if(hdfRecordAtt != NULL)
				{
				  free(hdfRecordAtt);
				  hdfRecordAtt = NULL;
				}
			      
			      return PGS_E_TOOLKIT;
			    }
			  
			  qualityFlags[j][1] = 
			    hdfRecordAtt[ j +  attstartIndex].qualityFlag;
			  for (count = 0; count < 3; count++ )
			    {
			      eulerAngle[j][count] = 
				hdfRecordAtt[j + attstartIndex].eulerAngle[count];
			      angularVelocity[j][count] = 
				hdfRecordAtt[j + attstartIndex].angularVelocity[count];
			    }
			}/*End for j */
		      if (NUMA == NUM)
			{
			  /* do nothing */
			}
		      else
			{
			  if(attFlag == PGS_TRUE && orbFlag == PGS_TRUE)
			    {
			      returnStatus = PGSEPH_W_EPHATT_NUMRECS_DIFFER;
			      sprintf(details," attitude and ephemeris record sizes don't match (nrec_eph=%d , nrec_att=%d), only eph records will be returned", *numvalues_Eph, *numvalues_Att);
			      PGS_SMF_SetDynamicMsg(returnStatus,details,FUNCTION_NAME2);
			    }
			  else if(attFlag == PGS_TRUE && orbFlag == PGS_FALSE)
			    {
			      returnStatus = PGSEPH_W_EPHATT_NUMRECS_DIFFER;
			      sprintf(details," attitude and ephemeris record sizes don't match (nrec_eph=%d , nrec_att=%d), uninterpolated attitude quaternions cannot be calculated", *numvalues_Eph, *numvalues_Att);
			      PGS_SMF_SetDynamicMsg(returnStatus,details,FUNCTION_NAME2);
			    }
			  if(hdfRecord != NULL)
			    {
			      free(hdfRecord);
			      hdfRecord = NULL;
			    }
			  if(hdfRecordAtt != NULL)
			    {
			      free(hdfRecordAtt);
			      hdfRecordAtt = NULL;
			    }
			  /* Close the Vset interface */
			  Vend( hdfIdAtt3 );
			  /* Close the HDF format file */
			  Hclose( hdfIdAtt3 );
			  return(returnStatus);
			}
		      

		      
		      for(j=0; j < *numvalues_Eph; j++)
			{
			  returnStatus1 = PGS_CSC_EulerToQuat(eulerAngle[j], 
							      hdfEulerAngleOrder, 
							      quatEuler);
			  if (returnStatus1 != PGS_S_SUCCESS)
			    {
			      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
			      
			    }
			  /*Get the orbital to ECI reference frame quaternion */
			  
			  returnStatus1 = PGS_CSC_getORBtoECIquat(positionECI[j], 
								  velocityECI[j], 
								  quatORBtoECI);
			  
			  if (returnStatus1 != PGS_S_SUCCESS)
			    {
			      
			      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
			      
			    }
			  
			  returnStatus1 = PGS_CSC_quatMultiply(quatORBtoECI, 
							       quatEuler, 
							       attitQuat[j]);
			  
			  if (returnStatus1 != PGS_S_SUCCESS)
			    {
			      PGS_SMF_SetUnknownMsg(returnStatus1, FUNCTION_NAME2);
			    }
			}
		      gotData = PGS_TRUE;
		    }
		  
		  
		  if((orbFlag == PGS_FALSE && 
		      attFlag == PGS_TRUE && DidNotGetEPH == 1) ||
		     (orbFlag == PGS_TRUE && 
		      attFlag == PGS_TRUE && DidNotGetEPH == 1))
		    {
		      for(j=0; j < *numvalues_Att; j++)
			{
			  returnStatus = 
			    PGS_TD_TAItoUTC(hdfRecordAtt[j + attstartIndex].secTAI93, 
					    asciiUTC_Att[j]);  
			  
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
			      
			      /* free hdfRecord memory */
			      if(hdfRecordAtt != NULL)
				{
				  free(hdfRecordAtt);
				  hdfRecordAtt = NULL;
				}
			      return returnStatus;
			    default:
			      PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME2);
			      
			      /* free hdfRecord memory */
			      if(hdfRecordAtt != NULL)
				{
				  free(hdfRecordAtt);
				  hdfRecordAtt = NULL;
				}
			      
			      return PGS_E_TOOLKIT;
			    }
			  
			  qualityFlags[j][1] = 
			    hdfRecordAtt[ j +  attstartIndex].qualityFlag;
			  for (count = 0; count < 3; count++ )
			    {
			      eulerAngle[j][count] = 
				hdfRecordAtt[j + attstartIndex].eulerAngle[count];
			      angularVelocity[j][count] = 
				hdfRecordAtt[j + attstartIndex].angularVelocity[count];
			    }
			}/*End for j */
		      gotData = PGS_TRUE;
		    }
		  if (gotData == PGS_TRUE)
		    {
		      if(DidNotGetEPH == 1)
			{
			  returnStatus = PGSEPH_W_EPHATT_NUMRECS_DIFFER;
			}
		      else
			{
			  returnStatus = PGS_S_SUCCESS;
			}
		      /* Close the Vset interface */
		      Vend( hdfIdAtt3 );
		      /* Close the HDF format file */
		      Hclose( hdfIdAtt3 );
		      break;
		    }
		}/*end of check spacecraft tag for stop time */
	      else
		{
		  returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
		}
	      /* Close the Vset interface */
	      Vend( hdfIdAtt3 );
	      /* Close the HDF format file */
	      Hclose( hdfIdAtt3 );
	      
	    } /* if(file_exists(hdfFileAtt3)) */
	  else
	    {
	      returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	    }
	  /* free hdfRecord memory */
	  if(hdfRecordAtt != NULL)
	    {
	      free(hdfRecordAtt);
	      hdfRecordAtt = NULL;
	    }
	} /* end for num_files */
    } 
  /* if still allocated mem left, free them. */
  
  if(hdfRecord != NULL)
    {
      free(hdfRecord);
      hdfRecord = NULL;
    }
  if(hdfRecordAtt != NULL)
    {
      free(hdfRecordAtt);
      hdfRecordAtt = NULL;
    }
  return returnStatus;
}


/******************************************************************************
BEGIN_PROLOG:

NAME:
  PGS_EPH_EphAtt_unInterpolateAux.c

DESCRIPTION:
  This function is a wrapper around the function PGS_EPH_EphAtt_unInterpolate()
  This function (which has one more argument than  PGS_EPH_EphAtt_unInterpolate), 
  is used to make the FORTRAN binding work properly, where the size of 
  asciiUTC_Eph  array are required in the FORTRAN binder, 
  but not present in the argument list of PGS_EPH_EphAtt_unInterpolate.
  The size of asciiUTC_Eph arrays passed to this 
  function will be maxnumrecs.
  See PGS_EPH_EphAtt_unInterpolateF.f for more information

AUTHOR:
  Abe Taaheri / L3 Communication, EER Systems Inc.

HISTORY:
  01-Aug-2003  AT  Initial version

END_PROLOG:
******************************************************************************/

PGSt_SMF_status
PGS_EPH_EphAtt_unInterpolateAux(                   
    PGSt_tag        spacecraftTag,       /* spacecraft identifier */
    char            *asciiUTC_start,     /* reference start time */
    char            *asciiUTC_stop,      /* reference stop time */
    PGSt_boolean    orbFlag,             /* get/don't get orbit data */
    PGSt_boolean    attFlag,             /* get/don't get attitude data */
    void            *qualityFlagsPtr,    /* quality flags */
    PGSt_integer    *numValues_Eph,      /* number of EPH record values 
					    estimated/requested */
    PGSt_integer    maxnumrecs,
    char            asciiUTC_Eph[][28],  /* Eph record times retrieved */    
    PGSt_double     positionECI[][3],    /* ECI position */
    PGSt_double     velocityECI[][3],    /* ECI velocity */
    PGSt_double     eulerAngles[][3],    /* s/c attitude Euler angles */
    PGSt_double     angularVelocity[][3],/* angular rates about s/c 
					    body axes*/
    PGSt_double     attitQuat[][4])      /* s/c to ECI rotation quaternion */
{

    PGSt_SMF_status    returnStatus;     /* status of TK func. calls */
    
    returnStatus =  PGS_EPH_EphAtt_unInterpolate(spacecraftTag, 
					   asciiUTC_start, asciiUTC_stop, 
					   orbFlag, attFlag, 
					   qualityFlagsPtr, 
					   numValues_Eph,
					   asciiUTC_Eph,  
					   positionECI, velocityECI,
					   eulerAngles, angularVelocity, 
					   attitQuat);
    return returnStatus;  
}
