
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/****************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_EPH_UnInterpEphAtt.c

DESCRIPTION:
   This file contains the function PGS_EPH_UnInterpEphAtt().
   This function gets uninterpolated actual ephemeris and/or attitude 
   data for the specified spacecraft at a given time period. User should
   pass output array sizes to this function usimg numValues_Eph and 
   numValues_Att, to avoid violation of array bounds writing.
   This function is similar to PGS_EPH_EphAtt_unInterpolate(), but it
   has two more argument than PGS_EPH_EphAtt_unInterpolate() to return 
   records for both eph and att records if the number of eph records is 
   not the same as the number of att records in the same time period.
   If one uses PGS_EPH_EphAtt_unInterpolate() to request both uninterpolated 
   eph and att records, one will get both only if the number of eph 
   records is the same as the number of att records in the same time period,
   otherwise one will get a warning and function will return only ephem
   records.


AUTHOR:
   Abe Taaheri          / L3 com. GSI

HISTORY:
   10-Jul-2003  AT  Wrapped this function around PGS_EPH_EphAtt_unInterpolate()
                    functions to accept one more argument, so that one can
                    get records for both eph and att even if the number of 
                    eph records is not the same as the number of att 
                    records in the same time period.
 
END_FILE_PROLOG:
****************************************************************************/

/****************************************************************************
BEGIN_PROLOG:

TITLE:
   Get uninterpolated Ephemeris and Attitude records
NAME:
   PGS_EPH_UnInterpEphAtt()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_UnInterpEphAtt(
        PGSt_tag        spacecraftTag,
        char            *asciiUTC_start,
        char            *asciiUTC_stop,
        PGSt_boolean    orbFlag,
        PGSt_boolean    attFlag,
        PGSt_integer    qualityFlags[][2],
        PGSt_integer    *numValuesEph,
        PGSt_integer    *numValuesAtt,
        char            asciiUTC_Att[][28],
        char            asciiUTC_Eph[][28],
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

      integer function pgs_eph_uninterpephAtt
     >                                   (spacecrafttag,asciiutcstart,
     >                                    asciiutcstop,orbflag,
     >                                    attflag,qualityflags,
     >                                    numvalueseph,numvaluesatt,
     >                                    asciiutceph,asciiutcatt
     > 				          positioneci,velocityeci,
     >                                    eulerangles,xyzrotrates,
     > 				          attitquat)
      integer           spacecrafttag
      integer           numvalueseph
      integer           numvaluesatt
      character*27      asciiutcstart
      character*27      asciiutcstop
      integer           orbflag
      integer           attflag
      character         asciiutcatt(27,*)
      character         asciiutceph(27,*)
      integer           qualityflags(2,*)
      double precision  positioneci(3,*)
      double precision  velocityeci(3,*)
      double precision  eulerangles(3,*)
      double precision  xyzrotrates(3,*)
      double precision  attitquat(4,*)

DESCRIPTION:
   This tool gets uninterpo;ated ephemeris and/or attitude data for the 
   specified spacecraft at the specified time period.

INPUTS:
   Name               Description               Units  Min        Max
   ----               -----------               -----  ---        ---
   spacecraftTag      spacecraft identifier     N/A


   asciiUTC_start     UTC time reference start  ASCII  1961-01-01 see NOTES
                      time in CCSDS ASCII time
	              code A format

   asciiUTC_stop      UTC time reference stop    ASCII  1961-01-01 see NOTES
                      time in CCSDS ASCII time
                      code A format

   orbFlag            set to true to get        T/F
                      ephemeris data

   attFlag            set to true to get        T/F
                      attitude data

   numValuesEph      As input this is the max 
                     number of values for EPH 
                     data that is expecteted
                     to be retrieved              N/A   

   numValuesAtt      As input this is the max 
                     number of values for ATT 
                     data that is expecteted
                     to be retrieved              N/A  
 
OUTPUTS:
   Name              Description                Units        Min         Max
   ----              -----------                -----        ---         ---
   qualityFlags      quality flags for                 ** see NOTES **
                     position and attitude
 
   asciiUTC_Att      UTC time reference        ASCII  1961-01-01 see NOTES
                     Uninterpolated attitude
                     time in CCSDS ASCII time
                     code A format
		     
   asciiUTC_Eph      UTC time reference        ASCII  1961-01-01 see NOTES
                     Uninterpolated ephemeris
                     time in CCSDS ASCII time
                     code A format
		     
   numValuesEph      As output this is 
                     Actual num. of values 
                     for EPH data retrieved      N/A   

   numValuesAtt      As output this is 
                     Actual num. of values 
                     for ATT data retrieved      N/A   

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
   PGSMEM_E_NO_MEMORY          unable to allocate required dynamic memory 
                               space
   PGSEPH_E_BAD_EPHEM_FILE_HDR no s/c ephemeris and/or attitude files had
                               readable headers
   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c ephemeris and/or attitude files could
                                be
                               found for input times
   PGSEPH_E_NO_DATA_REQUESTED  both orb and att flags are set to false
   PGSTD_E_SC_TAG_UNKNOWN      unrecognized/unsupported spacecraft tag
   PGSEPH_E_BAD_ARRAY_SIZE     array size specified is less than 0
   PGSTD_E_TIME_FMT_ERROR      format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR    value error in asciiUTC
   PGSTD_E_NO_LEAP_SECS        no leap seconds correction available for
                               initial
                               time (asciiUTC)
   PGS_E_TOOLKIT               an unexpected error occured
   PGSTSF_E_GENERAL_FAILURE    problem in the thread-safe code

EXAMPLES:
C:
        #define ARRAY_SIZE_EPH 7500
        #define ARRAY_SIZE_ATT 9500
        PGSt_integer    returnStatus;
	PGSt_double     positionECI[ARRAY_SIZE_EPH][3];
	PGSt_double     velocityECI[ARRAY_SIZE_EPH][3];
	PGSt_double     eulerAngles[ARRAY_SIZE_ATT][3];
	PGSt_double     xyzRotRates[ARRAY_SIZE_ATT][3];
	PGSt_double     attitQuat[ARRAY_SIZE_ATT][4];
        PGSt_integer    numValues_Att;
        PGSt_integer    numValues_Eph;
	char            asciiUTC_start[28];
        char            asciiUTC_stop[28];
        char            asciiUTC_Att[ARRAY_SIZE_ATT][28];
        char            asciiUTC_Eph[ARRAY_SIZE_EPH][28];
	PGSt_integer    qualityFlags[ARRAY_SIZE_ATT][2];

	
	PGSt_SMF_status returnStatus;


	strcpy(asciiUTC_start,"1998-02-03T19:23:45.123");
        strcpy(asciiUTC_stop,"1998-02-03T20:23:45.123");
        numValues_Eph = ARRAY_SIZE_EPH;
        numValues_Att = ARRAY_SIZE_ATT;
        returnStatus = PGS_EPH_UnInterpEphAtt(PGSd_EOS_AM,
                                          asciiUTC_start,asciiUTC_stop,
	                                  PGS_TRUE,PGS_TRUE,qualityFlags,
                                          numValues_Eph,numValues_Att,
                                          asciiUTC_Eph,asciiUTC_Att,
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

      integer           pgs_eph_uninterpephAtt

      integer           numvaluesatt/2000/
      integer           numvalueseph/1000/
      integer           returnstatus
      integer           qualityflags(2,*)
     
      character*27      asciiutcstart 
      character*27      asciiutcstop
      
      character         asciiutcatt(27,*)
      character         asciiutceph(27,*)
      double precision  positioneci(3,*)
      double precision  velocityeci(3,*)
      double precision  eulerangles(3,*)
      double precision  xyzrotrates(3,*)
      double precision  attitquat(4,*)

      asciiutcstart = '1998-02-03T19:23:45.123'
      asciiutcstop = '1998-02-03T20:23:45.123'

      returnstatus =  pgs_eph_uninterpephAtt
     >                         (pgsd_eos_am,asciiutcstart,asciiutcstop,
     >                          pgs_true,pgs_true,qualityflags,
     >                          numvalueseph,numvaluesatt,
     >                          asciiutceph,asciiutcatt,positioneci,
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
   PGS_SMF_SetStaticMsg()        sets the message buffer
   PGS_SMF_SetDynamicMsg()       sets the message buffer
   PGS_EPH_UnInterpEphAtt() 
   PGS_SMF_TestStatusLevel()

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

/* name of the functions */
#define  FUNCTION_NAME "PGS_EPH_UnInterpEphAtt()"

PGSt_SMF_status
PGS_EPH_UnInterpEphAtt(                 /* get ephemeris and attitude data */
    PGSt_tag        spacecraftTag,       /* spacecraft identifier */
    char            *asciiUTC_start,     /* reference start time */
    char            *asciiUTC_stop,      /* reference stop time */
    PGSt_boolean    orbFlag,             /* get/don't get orbit data */
    PGSt_boolean    attFlag,             /* get/don't get attitude data */
    void            *qualityFlagsPtr,    /* quality flags */
    PGSt_integer    *numValues_Eph,      /* number of EPH record values 
					    estimated/requested */
    PGSt_integer    *numValues_Att,      /* number of ATT record values 
					    estimated/requested */
    char            asciiUTC_Eph[][28],  /* Eph record times retrieved */
    char            asciiUTC_Att[][28],  /* Att record times retrieved */

    PGSt_double     positionECI[][3],    /* ECI position */
    PGSt_double     velocityECI[][3],    /* ECI velocity */
    PGSt_double     eulerAngles[][3],     /* s/c attitude Euler angles */
    PGSt_double     angularVelocity[][3],/* angular rates about s/c 
					    body axes*/
    PGSt_double     attitQuat[][4])      /* s/c to ECI rotation quaternion */
{
    char            details[PGS_SMF_MAX_MSG_SIZE];       /* detailed error 
							    message */
    PGSt_SMF_status returnStatus;        /* return value of this function */ 
    PGSt_SMF_status returnStatus1;       /* return value of this function */ 
    PGSt_SMF_status whoCalled;           /* user that called this function */



/***************************************************************************
*    Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();
    
    if(orbFlag ==  PGS_TRUE && attFlag != PGS_TRUE )
      {
	returnStatus = 
	  PGS_EPH_EphAtt_unInterpolate(spacecraftTag,
				       asciiUTC_start,asciiUTC_stop,
				       PGS_TRUE, PGS_FALSE, 
				       qualityFlagsPtr,
				       numValues_Eph, 
				       asciiUTC_Eph,
				       positionECI,velocityECI,
				       eulerAngles,angularVelocity,attitQuat);
	
	if(returnStatus != PGS_S_SUCCESS)
	  {
	    PGS_SMF_GetMsgByCode(returnStatus,details);
	    PGS_SMF_SetDynamicMsg(returnStatus, details,FUNCTION_NAME);
	    return returnStatus;
	  }
      }
    else if(orbFlag !=  PGS_TRUE && attFlag == PGS_TRUE )
      {
	
	returnStatus = 
	  PGS_EPH_EphAtt_unInterpolate(spacecraftTag,
				       asciiUTC_start,asciiUTC_stop,
				       PGS_FALSE, PGS_TRUE, 
				       qualityFlagsPtr,
				       numValues_Att, 
				       asciiUTC_Att,
				       positionECI,velocityECI,
				       eulerAngles,angularVelocity,attitQuat);
	if(returnStatus != PGS_S_SUCCESS)
	  {
	    PGS_SMF_GetMsgByCode(returnStatus,details);
	    PGS_SMF_SetDynamicMsg(returnStatus, details,FUNCTION_NAME);
	    return returnStatus;
	  }
      }
    else if(orbFlag ==  PGS_TRUE && attFlag == PGS_TRUE )
      {
	
	returnStatus1 = 
	  PGS_EPH_EphAtt_unInterpolate(spacecraftTag,
				       asciiUTC_start,asciiUTC_stop,
				       PGS_FALSE, PGS_TRUE, 
				       qualityFlagsPtr,
				       numValues_Att, 
				       asciiUTC_Att,
				       positionECI,velocityECI,
				       eulerAngles,angularVelocity,attitQuat);
	
	if(returnStatus1 != PGS_S_SUCCESS)
	  {
	    switch(PGS_SMF_TestStatusLevel(returnStatus1))
	      {
	      case PGS_SMF_MASK_LEV_E:
		PGS_SMF_GetMsgByCode(returnStatus1,details);
		PGS_SMF_SetDynamicMsg(returnStatus1, details,FUNCTION_NAME);
		return returnStatus1;
		break;
	      case PGS_SMF_MASK_LEV_S:
		break;
	      case PGS_SMF_MASK_LEV_W:
		break;
	      default:
		if (whoCalled != PGSd_CALLERID_SMF)
		  {
		    PGS_SMF_SetStaticMsg(returnStatus1,FUNCTION_NAME); 
		  }
		break;
	      }
	  }
	returnStatus = 
	  PGS_EPH_EphAtt_unInterpolate(spacecraftTag,
				       asciiUTC_start,asciiUTC_stop,
				       PGS_TRUE, PGS_FALSE, 
				       qualityFlagsPtr,
				       numValues_Eph, 
				       asciiUTC_Eph,
				       positionECI,velocityECI,
				       NULL, NULL, NULL);
	
	if(returnStatus != PGS_S_SUCCESS)
	  {
	    switch(PGS_SMF_TestStatusLevel(returnStatus))
	      {
	      case PGS_SMF_MASK_LEV_E:
		PGS_SMF_GetMsgByCode(returnStatus,details);
		PGS_SMF_SetDynamicMsg(returnStatus, details,FUNCTION_NAME);
		return returnStatus;
		break;
	      case PGS_SMF_MASK_LEV_S:
		break;
	      case PGS_SMF_MASK_LEV_W:
		PGS_SMF_GetMsgByCode(returnStatus,details);
		PGS_SMF_SetDynamicMsg(returnStatus, details,FUNCTION_NAME);
		break;
	      default:
		if (whoCalled != PGSd_CALLERID_SMF)
		  {
		    PGS_SMF_SetStaticMsg(returnStatus,FUNCTION_NAME); 
		  }
		break;
	      }
	  }
	else
	  {
	    PGS_SMF_GetMsgByCode(returnStatus1,details);
	    PGS_SMF_SetDynamicMsg(returnStatus1, details,FUNCTION_NAME);
	    return returnStatus1;
	  }
      }
    else if(orbFlag !=  PGS_TRUE && attFlag != PGS_TRUE )
      {
	PGS_SMF_SetStaticMsg(PGSEPH_E_NO_DATA_REQUESTED,FUNCTION_NAME);
	return PGSEPH_E_NO_DATA_REQUESTED;
      }
    return returnStatus;
}

/******************************************************************************
BEGIN_PROLOG:

NAME:
  PGS_EPH_UnInterpEphAttAux.c

DESCRIPTION:
  This function is a wrapper around the function PGS_EPH_UnInterpEphAtt()
  This function (which has one more argument than  PGS_EPH_UnInterpEphAttt), 
  is used to make the FORTRAN binding work properly, where the size of 
  asciiUTC_Eph, and asciiUTC_Att arrays are required in the FORTRAN binder, 
  but not present in the argument list of PGS_EPH_UnInterpEphAtt.
  The size of asciiUTC_Eph, and asciiUTC_Att arrays passed to this 
  function will be maxnumrecs.
  See PGS_EPH_UnInterpEphAttF.f for more information

AUTHOR:
  Abe Taaheri / L3 Communication, EER Systems Inc.

HISTORY:
  01-Aug-2003  AT  Initial version

END_PROLOG:
******************************************************************************/

PGSt_SMF_status
PGS_EPH_UnInterpEphAttAux(                   
    PGSt_tag        spacecraftTag,       /* spacecraft identifier */
    char            *asciiUTC_start,     /* reference start time */
    char            *asciiUTC_stop,      /* reference stop time */
    PGSt_boolean    orbattFlags[2],             /* get/don't get orbit & att data */
    void            *qualityFlagsPtr,    /* quality flags */
    PGSt_integer    numValues_EphAtt[2], /* number of EPH & ATT  record values 
					    estimated/requested */
    PGSt_integer    maxnumrecs,
    char            asciiUTC_Eph[][28],  /* Eph record times retrieved */
    char            asciiUTC_Att[][28],  /* Att record times retrieved */
    
    PGSt_double     positionECI[][3],    /* ECI position */
    PGSt_double     velocityECI[][3],    /* ECI velocity */
    PGSt_double     eulerAngles[][3],    /* s/c attitude Euler angles */
    PGSt_double     angularVelocity[][3],/* angular rates about s/c 
					    body axes*/
    PGSt_double     attitQuat[][4])      /* s/c to ECI rotation quaternion */
{

    PGSt_SMF_status    returnStatus;     /* status of TK func. calls */
    PGSt_boolean       orbFlag, attFlag;
    PGSt_integer       numValues_Eph, numValues_Att;

    orbFlag = orbattFlags[0];
    attFlag = orbattFlags[1];
    /* initially assign an estimated number of values for numValues_Eph
       and numValues_Att */
    numValues_Eph = numValues_EphAtt[0];
    numValues_Att = numValues_EphAtt[1];

    returnStatus =  PGS_EPH_UnInterpEphAtt(spacecraftTag, 
					   asciiUTC_start, asciiUTC_stop, 
					   orbFlag, attFlag, 
					   qualityFlagsPtr, 
					   &numValues_Eph, &numValues_Att, 
					   asciiUTC_Eph, asciiUTC_Att,  
					   positionECI, velocityECI,
					   eulerAngles, angularVelocity, 
					   attitQuat);
    numValues_EphAtt[0] = numValues_Eph;
    numValues_EphAtt[1] = numValues_Att;
    return returnStatus;  
}
