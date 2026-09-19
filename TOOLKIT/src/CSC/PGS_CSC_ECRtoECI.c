/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_ECRtoECI.c

DESCRIPTION:
   This file contains the function PGS_CSC_ECRtoECI().
   This function rotates an array of 6-vectors from ECR (of date) coordinates to
   ECI (J2000) coordinates.
   
AUTHOR:
   Peter Noerdlinger  / Space Applications Corporation
   Anubha Singhal     / Applied Research Corporation
   Guru Tej S. Khalsa / Applied Research Corporation
   Curt Schafer       / Steven Myers & Associates

HISTORY:
   05-Mar-1994 PDN  Designed				
   09-Mar-1994 PDN  Created
   21-Sep-1994 AS   Updated to accept time offsets,updated calling sequences,
                    updated types, updated variable names, updated error 
                    messages 
   22-Sep-1994 AS   Updated prolog to conform to latest PGS/ECS standards
   27-Sep-1994 AS   Fixing up error handling
   08-Feb-1995 PDN  Fixed up error handling
   25-May-1995 GTSK Rewritten to improve efficiency and readability
   05-Jul-1995 GTSK Fixed bug that caused variation in error reporting when the
                    function was called twice in a row with the same time
   21-Jun-1996 PDN  Deleted check for obsolete "INTERIM_UT1" status
                    Added results of file latency study
   25-Jul-1997 PDN  Fixed "DETAILS" in the prologue for correct order of
                    rotations [must be opposite to PGS_CSC_ECItoECR()].
   25-Aug-1997 PDN  Changed Earth rotation rate to IERS value
   07-Jul-1999 CS   Updated for Threadsafe functionality

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:  
   Transform from ECR to ECI Coordinates

NAME:   
   PGS_CSC_ECRtoECI()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_ECRtoECI(   
       PGSt_integer numValues,
       char         asciiUTC[28], 
       PGSt_double  offsets[],  
       PGSt_double  posvelECR[][6],
       PGSt_double  posvelECI[][6])

FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_TD_3.f'
      include 'PGS_TD.f'
      include 'PGS_SMF.f'

      integer function pgs_csc_ecrtoeci(numvalues,asciiutc,offsets,
     >                                  posvelecr,posveleci)
       integer          numvalues
       character*27     asciiutc
       double precision offsets(*)
       double precision posvelecr(6,*)
       double precision posveleci(6,*)  
  

DESCRIPTION:
   This function rotates an array of 6-vectors from ECR (of date) coordinates to
   ECI (J2000) coordinates.
 
INPUTS:
   Name          Description         Units      Min        Max
   ----          -----------         -----      ---        --- 
   numValues     number of input     N/A        0          any
                 time offsets
   asciiUTC      UTC start time in   N/A        1979-06-30 ** see NOTES **
                 CCSDS ASCII Time
		 Code A or B format
   offsets       array of time       seconds    Max and Min such that
                 offsets                        asciiUTC+offset is 
		                                between asciiUTC Min
					        and Max values
   posvelECR[6]  Vector (position 
                 and velocity) in 
                 ECR
   posvelECR[0]..position            meters
   posvelECR[2]  
   posvelECR[3]..velocity            meters/
   posvelECR[5]                      second                     

OUTPUTS:

   posvelECI[6]  Vector after being 
                 transformed to 
                 J2000
   posvelECI[0]..position            meters 
   posvelECI[2]   
   posvelECI[3].. 
   posvelECI[5]  velocity            meters/
                                     second                      
RETURNS:
   PGS_S_SUCCESS                  successful return
   PGSCSC_W_BAD_TRANSFORM_VALUE   invalid ECRtoECI transformation
   PGSCSC_E_BAD_ARRAY_SIZE        incorrect array size
   PGSTD_E_NO_LEAP_SECS           no leap seconds correction available for
                                  input time
   PGSTD_W_PRED_LEAPS             a predicted leap second value was used for 
                                  at least one of the input offset times
   PGSTD_E_TIME_FMT_ERROR         format error in asciiUTC
   PGSTD_E_TIME_VALUE_ERROR       value error in asciiUTC
   PGSCSC_W_PREDICTED_UT1         status of UT1-UTC correction is predicted
   PGSTD_E_NO_UT1_VALUE           no UT1-UTC correction available
   PGS_E_TOOLKIT                  something unexpected happened, execution of
                                  function ended prematurely      
   PGSTSF_E_GENERAL_FAILURE       bad return from PGS_TSF_GetTSFMaster or 
                                  PGS_TSF_GetMasterIndex()

EXAMPLES:
C:
   #define   ARRAY_SIZE   3

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numValues;
   char               asciiUTC[28];
   PGSt_double        offsets[ARRAY_SIZE] = {3600.0,7200.0,10800.0};
   PGSt_double        posvelECR[ARRAY_SIZE][6] = {
                                                   {0.5,0.75,0.90,0.3,0.2,0.8},
                                                   {0.65,1.2,3.65,0.1,3.2,1,7},
						   {0.98,2.6,4,78,0.2,1.5,0.9}
						  };
   PGSt_double        posvelECI[ARRAY_SIZE][6];				     
  
   numValues = ARRAY_SIZE;
   strcpy(asciiUTC,"1991-01-01T11:29:30.123211Z");
   returnStatus = PGS_CSC_ECRtoECI(numValues,asciiUTC,offsets,posvelECR,
                                   posvelECI)
                                                                      
   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
      implicit none
      integer		  pgs_csc_ecitoecr
      integer		  returnstatus
      integer		  numvalues
      character*27        asciiutc 
      double precision    offsets(3)
      double precision    posveleci(6,3)
      double precision    posvelecr(6,3)
      integer             cnt1
      integer             cnt2
      character*33 	  err
      character*241 	  msg
      
      data offsets/3600.0, 7200.0, 10800.0/
      
      do 10 cnt1 = 1,6
          do 10 cnt2 = 1,3
              posvelecr(cnt1,cnt2) = 100 * cnt1 * cnt2
  10  continue

      asciiutc = '1991-07-27T11:04:57.987654Z'
      numvalues = 3
      
      returnstatus = pgs_csc_ecitoecr(numvalues, asciiutc, offsets,
     >                                posvelecr, posveleci)

      if (returnstatus .ne. pgs_s_success) then
	  pgs_smf_getmsg(returnstatus, err, msg)
	  write(*,*) err, msg
      endif
   
NOTES:
   TIME ACRONYMS:
     
     GAST is: Greenwich Apparent Sidereal Time
     GMST is: Greenwich Mean Sidereal Time
     TAI is:  International Atomic Time
     TDB is:  Barycentric Dynamical Time
     TDT is:  Terrestrial Dynamical Time
     UT1 is:  Universal Time
     UTC is:  Coordinated Universal Time

    TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file utcpole.dat which relates UT1 - UTC values to
     UTC dates.  As of June 1996 utcpole.dat starts at June 30, 1979; therefore,
     the correction UT1 - UTC, which can be as large as (+-) 0.9 s, is not
     available until that date, and an error status message will be returned if
     this function is called with a time before that date.  The file
     utcpole.dat, which is maintained periodically, contains final (definitive),
     and predicted (approximate) values.  Thus, when the present function is used,
     the user ought to carefully check the return status.  A success status 
     message will be returned if all input times correspond to final values.
     A warning status message will be returned if any input times correspond to
     predicted values.  The predicted values are generally quite usable for
     most geolocation purposes provided that the data file was updated from
     the U.S. Naval Observatory source within the last few months.  Here follows
     the results of a latency study on this file:


   RESULTS OF THE LATENCY STUDY:

     We have compared two sources of error estimates: The IERS
     Explanatory Supplement to Bulletins A and B and the data
     tables themselves.  The results are essentially the same
     (as of March, 1996).
     
     Here are the results for the axial error; "Latency Time" means
     time since the last update.  A few error values for polar
     motion are included for comparison and for those most concerned
     with North-South error:
     
             Table 1 - Errors due to File Latency in equivalent meters
                       Earth motion
     
       File Latency  Time            UT1 Error               Polar Motion
             (days)                  (Equivalent E-W Meters  Error (Meters
                                     - one sigma)            One Sigma)
             1                        0.14                   -
             5                        0.45                   0.02
             10                       0.76                   0.09
             15                       1.03                   -
             20                       1.28                   -
             25                       1.51                   -
             30                       1.74                   0.30
             45                       2.35                   -
             60                       2.92                   -
             75                       3.45                   -
             90                       3.96                   0.50
             120                      4.91
             150                      5.82
             180                      6.66
             225                      7.87
             270                      9.03
             315                     10.14
             365                     11.32
             369                     11.42
     
      For further information see "Theoretical Basis of the SDP Toolkit 
      Geolocation Package for the ECS Project", Document 445-TP-002-002, 
      May 1995, by P. Noerdlinger.

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
     the toolkit as TAI.

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
      model will be used, and processing will continue normally, except that the
      return status will be have a status level of 'W' to alert the user that
      the default earth model was used and not the one specified by the user.
      The reporting of such general warnings takes precedence over the generic
      warning (see RETURNS above) that processing was not successful at some of
      the requested times.  Therefore in the case of any return status of level
      'W', the returned value of a high precision real variable generally should
      be examined for errors at each time offset, as specified above.

   REFERENCES:
 
     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac
 
REQUIREMENTS:  
   PGSTK - 1050

DETAILS:
   After some preliminary tests, this program will rotate a vector x from J2000
   to the epoch jed.  The rotation is in four parts:

       Adjustment of the Earth geographic axis in relation to the Earth 
            Rotation axis
       diurnal Earth Rotation about the axis corrected for polar motion
       nutation
       precession

    The sequence of operations for ECR -> ECI is:

        1. get TAI from UTC and leap seconds
	2. get precession and nutation and TDB from TAI
	3. get UT1-UTC and polar motion from a utility that reads tabulated
	   values
	4. get Greenwich mean sidereal time from UT1
	5. get Greenwich apparent sidereal time from Greenwich mean sidereal 
	   time and nutation  (equation of the equinoxes)
	6. adjust the vector from the geographic pole to the North rotation
	   pole         
	7. rotate about the N pole by the angle gast in radians to bring 
	   the vector to celestial coordinates of date
	8. nutate and precess the vector

GLOBALS:
    PGSg_TSF_CSCECRtoECIlastTAI93
    PGSg_TSF_CSCECRtoECIjedTDB
    PGSg_TSF_CSCECRtoECIjedTDT
    PGSg_TSF_CSCECRtoECIdvnut
    PGSg_TSF_CSCECRtoECIearthrot
    PGSg_TSF_CSCECRtoECIxpol
    PGSg_TSF_CSCECRtoECIypol
    PGSg_TSF_CSCECRtoECIsetupStatus

FILES:
   This tool accesses files leapsec.dat and utcpole.dat  
      
FUNCTIONS CALLED:
   PGS_TD_UTCtoTAI          convert UTC to TAI
   PGS_TD_TAItoUT1pole      get UT1 and pole data
   PGS_CSC_quickWahr        obtain nutation angles and rates
   PGS_CSC_dotProduct       find dot product of two vectors
   PGS_TD_gmst              get GMST
   PGS_TD_gast              get GAST
   PGS_CSC_precs2000        precess vector from J2000 to TDT
   PGS_CSC_nutate2000       nutate vector to pole of date
   PGS_CSC_rotat6           rotate position and velocity vectors
   PGS_CSC_rotat3           rotate position vector only
   PGS_SMF_SetStaticMsg     set error/status message  
   PGS_SMF_SetDynamicMsg    set dynamic message
   PGS_SMF_SetUnknownMsg    set unknown message 
   PGS_SMF_GetMsg           get the mnemonic and the message
   PGS_SMF_GetMsgByCode     get the message by code
   PGS_TSF_GetTSFMaster     setup threads version
   PGS_TSF_GetMasterIndex   get the index for this thread

END_PROLOG
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <string.h>
#include <PGS_TD.h>
#include <PGS_CSC.h>
#include <PGS_TSF.h>

   /* constant needed for velocity transformation */

#define   EARTH_ROTATE_RATE 0.000072921151 /* IERS Value as of 1997 */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_ECRtoECI()"

PGSt_SMF_status 
PGS_CSC_ECRtoECI(                  /* transforms from ECI reference frame to 
                                      ECR ref. frame */
    PGSt_integer numValues,        /* number of time offsets */
    char         asciiUTC[28],     /* UTC in CCSDS ASCII format (A or B) */
    PGSt_double  offsets[],        /* time offsets relative to startTime */
    PGSt_double  posvelECR[][6],   /* input vector: position (m) and velocity 
				      (m/s) in ECR */
    PGSt_double  posvelECI[][6])   /* the answer - vector rotated to ECI */ 
{    
    PGSt_integer       cnt1;       /* loop counter */
    PGSt_integer       cnt2;       /* loop counter */
    PGSt_integer       maxValues;  /* number of vectors to transform */	
    PGSt_integer       threeOr6;   /* 3 to transform only position, 6 for
				      position and velocity */
    PGSt_integer       numBad=0;   /* tracks number of unusable input times */

    PGSt_double        diffUT1UTC; /* UT1 - UTC in seconds of TIME (not of
				       arc!) */
    PGSt_double        jdtable;    /* nearest tabulated JD to the input */
        
    PGSt_double        gmst;       /* Greenwich Mean sidereal time */
    PGSt_double        startTAI93; /* continuous seconds since 0 hours
				      UTC on 01-01-93 (of asciiUTC) */
    PGSt_double        secTAI93;   /* continuous seconds since 0 hours
				      UTC on 01-01-93 with offsets */
    PGSt_double        jdUT1[2];   /* UT1 expressed in Julian days as two 
                                      PGSt_doubles */
    PGSt_double        xpole;      /* x offset of polar wander in seconds
				      of arc (used in call to TAItoUT1pole which
				      will return a value of zero for xpole if
				      the call fails, this would reset xpol
				      (defined below) which is bad, so we use
				      this variable as an interim variable) */
    PGSt_double        ypole;      /* y offset of polar wander in seconds
				      of arc (used in call to TAItoUT1pole which
				      will return a value of zero for ypole if
				      the call fails, this would reset ypol
				      (defined below) which is bad, so we use
				      this variable as an interim variable) */

    static PGSt_double jedTDT[2];  /* TDT expressed in Julian days */

    static PGSt_double jedTDB[2];  /* TDB expressed in Julian days (TDT with
                                      additional periodic correction) see
				      A. A. p. B5 for this correction.
				      Note: The correction in the Astronomical
				      Almanac is based on an old and somewhat
				      too large value of the eccentricity of
				      the Earth's orbit, but it is good to 1
				      microsecond.  P. Noerdlinger is in touch
				      with the Naval Observatory in regards
				      to correcting the equation.  Herein it is 
				      used as published.  Errors in TBD pro-
				      pagate only to the slowly varying
				      precession and nutation and do not
				      affect gmst or gast */
    
    static PGSt_double earthrot[2];/* Earth rotation angle and rate about 
				      true N pole of date; first component 
				      is GAST, the second is 
				      0.000072921151467, according to the 
				      Expl. Sup. p. 51, Eq. (2.24-3).(changed
                                      to IERS value Aug 1997   */
    static PGSt_double dvnut[4];   /* the two nutation angles and their 
				      rates */
    
    /* Note: the rotation routines PGS_CSC_rotat3 and PGS_CSC_rotat6 require an
       input of angular displacement and velocity.  Polar Motion is so small,
       and its velocity so poorly known, that we must replace the velocity by
       zero for our purposes.  The following two definitions establish vectors
       that can be used for this purpose; the first component is the
       displacement; the second will be equated to zero */
    
    static PGSt_double xpol[2]={0.,0.};   /* x offset of polar wander in 
					     radians */
    static PGSt_double ypol[2]={0.,0.};   /* y offset of polar wander in 
					     radians */
    static PGSt_double lastTAI93=1.e50;   /* previous time value to avoid 
					     repeated setup */
    PGSt_SMF_status    returnStatus;      /* return status of this function */
    PGSt_SMF_status    returnStatus1;     /* return status of function calls */
    PGSt_SMF_status    code;              /* status code returned by 
					     PGS_SMF_GetMsg() */

    char  mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE];    /* mnemonic returned by
						     PGS_SMF_GetMsg() */
    char  msg[PGS_SMF_MAX_MSG_SIZE];              /* message returned by
						     PGS_SMF_GetMsg() */

    static PGSt_SMF_status setupStatus;   /* return status of function calls */

    static char  setup_msg[PGS_SMF_MAX_MSG_SIZE]; /* message returned by
						     PGS_SMF_GetMsg() */

    /* pointer to appropriate rotation function - either PGS_CSC_rotat3() or
       PGS_CSC_rotat6() */

    PGSt_SMF_status (*pgs_rotate)(PGSt_double*, PGSt_double*,
				  PGSt_integer, PGSt_double*);
    
#ifdef _PGS_THREADSAFE

    /* Declare variables used for THREADSAFE version to replace statics
        The local names are appended with TSF and globals are preceded with
        directory and function name */

    PGSt_TSF_MasterStruct *masterTSF;
    PGSt_double jedTDBTSF[2];
    PGSt_double jedTDTTSF[2];
    PGSt_double dvnutTSF[4];
    PGSt_double earthrotTSF[2];
    PGSt_double xpolTSF[2];
    PGSt_double ypolTSF[2];
    PGSt_double lastTAI93TSF;
    PGSt_SMF_status setupStatusTSF;
    char  *setup_msgTSF;      /* TSD key     does Not use global */

       /* Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_double     PGSg_TSF_CSCECRtoECIlastTAI93[];
    extern PGSt_double     PGSg_TSF_CSCECRtoECIjedTDB[][2];
    extern PGSt_double     PGSg_TSF_CSCECRtoECIjedTDT[][2];
    extern PGSt_double     PGSg_TSF_CSCECRtoECIdvnut[][4];
    extern PGSt_double     PGSg_TSF_CSCECRtoECIearthrot[][2];
    extern PGSt_double     PGSg_TSF_CSCECRtoECIxpol[][2];
    extern PGSt_double     PGSg_TSF_CSCECRtoECIypol[][2];
    extern PGSt_SMF_status PGSg_TSF_CSCECRtoECIsetupStatus[];

    int masterTSFIndex;
    int x;                /* loop index for initialization  */

 
   /* Get index from PGS_TSF_GetMasterIndex()
           Initialize the variables used for the THREADSAFE version */

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(returnStatus) || masterTSFIndex ==
                          PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    setupStatusTSF = PGSg_TSF_CSCECRtoECIsetupStatus[masterTSFIndex];
    lastTAI93TSF = PGSg_TSF_CSCECRtoECIlastTAI93[masterTSFIndex];
    for (x=0; x<2; x++)
    {
        jedTDBTSF[x] =   PGSg_TSF_CSCECRtoECIjedTDB[masterTSFIndex][x];
        jedTDTTSF[x] =   PGSg_TSF_CSCECRtoECIjedTDT[masterTSFIndex][x];
        earthrotTSF[x] = PGSg_TSF_CSCECRtoECIearthrot[masterTSFIndex][x];
        xpolTSF[x] =     PGSg_TSF_CSCECRtoECIxpol[masterTSFIndex][x];
        ypolTSF[x] =     PGSg_TSF_CSCECRtoECIypol[masterTSFIndex][x];
    }
    for (x=0; x<4; x++)
    {
        dvnutTSF[x] = PGSg_TSF_CSCECRtoECIdvnut[masterTSFIndex][x];
    }
    setup_msgTSF = (char *) pthread_getspecific(
                  masterTSF->keyArray[PGSd_TSF_KEYCSCECRTOECISETUP_MSG]);
 
#endif

    /* NO negative array size is allowed.  Return an error if numValues < 0. */

    if ( numValues < 0 )
    {
	PGS_SMF_SetStaticMsg(PGSCSC_E_BAD_ARRAY_SIZE, FUNCTION_NAME);
	return PGSCSC_E_BAD_ARRAY_SIZE;
    }

    /* convert input start time to TAI (check errors) */
    
    returnStatus = PGS_TD_UTCtoTAI(asciiUTC, &startTAI93);
    switch(returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
	PGS_SMF_GetMsg(&code,mnemonic,msg);
	if(code != returnStatus)
	    PGS_SMF_GetMsgByCode(returnStatus,msg);
	break;
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGSTD_E_TIME_FMT_ERROR:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    /* set maximum value of counter for looping to include start UTC time */

    maxValues = (numValues == 0) ? 1 : numValues;
    
    /* loop through for each offset */

    for (cnt1=0;cnt1<maxValues;cnt1++)
    {
	/* add in the offsets */

        if (numValues != 0)
	    secTAI93 = startTAI93 + offsets[cnt1];
        else
	    secTAI93 = startTAI93;

	/* Check if we have just used this function with the same input 
           date and time. If so, omit all setup items and proceed to the 
           rotations! */

#ifdef _PGS_THREADSAFE

        /*  Threadsafe protect: lastTAI93  */

	if (lastTAI93TSF != secTAI93) /* BEGIN setup */
#else
	if (lastTAI93 != secTAI93) /* BEGIN setup */
#endif
        {
	    /* get UT1 and pole information */

	    returnStatus1 = PGS_TD_TAItoUT1pole(secTAI93, jdUT1, &xpole,
                                                &ypole, &diffUT1UTC, &jdtable);

            switch (returnStatus1)
	    {
	      case PGS_S_SUCCESS:
		break;
              case PGSTD_W_PRED_LEAPS:
		if (returnStatus == PGS_S_SUCCESS)
                {   
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if(code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    returnStatus = returnStatus1;
		}
		break;
	      case PGSTD_E_NO_LEAP_SECS:
		if (returnStatus == PGS_S_SUCCESS)
                {   
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if(code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    
		    /* Even if the return status was NO_LEAP_SECS, set the
		       return status to PRED_LEAPS.  This is in case at least
		       some of the input times were available, and basically
		       the predicted leap seconds aren't much better than the
		       leapseconds calculated by the toolkit if no leap seconds
		       file is found.  If NO useful times are found then
		       PGSTD_E_NO_LEAP_SECS will be returned. */

		    returnStatus = PGSTD_W_PRED_LEAPS;
		}
		if (++numBad == maxValues)
		    return returnStatus1;
		break; 
	      case PGSCSC_W_PREDICTED_UT1:      
		if (returnStatus != PGSTD_E_NO_LEAP_SECS)
                {   
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if(code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    returnStatus = returnStatus1;
		}
	   	break;
	      case PGSTD_E_NO_UT1_VALUE:
		if (returnStatus == PGS_S_SUCCESS)
                {   
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if(code != returnStatus1)
			PGS_SMF_GetMsgByCode(returnStatus1,msg);
		    returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;    
		}
	   	for (cnt2=0; cnt2 < 6; cnt2++)
		    posvelECR[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;

		/* If NO useful times are found then return
		   PGSTD_E_NO_UT1_VALUE. */

		if (++numBad == maxValues)
		{
		    PGS_SMF_SetStaticMsg(returnStatus1,FUNCTION_NAME);
		    return returnStatus1;
		}
		continue;  
	      case PGSCSC_W_DATA_FILE_MISSING:
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if(code != returnStatus1)
		    PGS_SMF_GetMsgByCode(returnStatus1,msg);
		PGS_SMF_SetDynamicMsg(PGSTD_E_NO_UT1_VALUE,msg,FUNCTION_NAME);
		return PGSTD_E_NO_UT1_VALUE;
	      default:
	        if (returnStatus == PGS_S_SUCCESS)
                {
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    if(code != returnStatus1)
		      PGS_SMF_GetMsgByCode(returnStatus1,msg);   
		    returnStatus = PGSCSC_W_BAD_TRANSFORM_VALUE;    
		}
		for (cnt2=0; cnt2 < 6; cnt2++)
		    posvelECR[cnt1][cnt2] = PGSd_GEO_ERROR_VALUE;

		/* If NO useful times are found then return an error */

		if (++numBad == maxValues)
		{
		    PGS_SMF_SetUnknownMsg(returnStatus1,FUNCTION_NAME);
		    return PGS_E_TOOLKIT;
		}
		continue;  
	    }

	    /* Save the return status from the above call to 
	       PGS_TD_TAItoUT1pole().  If it is not PGS_S_SUCCESS save the error
	       message as well.  This is done since if this function is called
	       again with the same time as the current time this setup section
	       is skipped and any relevant error messages that ought to be
	       returned from this function due to the above call will not be
	       generated.  Therefore the information is saved here and if this
	       section is skipped in future calls then this information will be
	       checked to see if a relevant status message should be
	       reported. */

#ifdef _PGS_THREADSAFE

            /* Threadsafe Protect: setupStatus, setup_msg, xpol,ypol, 
               jedTDB and dvnut,     reassign their values for next use */

	    if ((setupStatusTSF=returnStatus) != PGS_S_SUCCESS)
            {
		strcpy(setup_msgTSF,msg);
	        pthread_setspecific(
                       masterTSF->keyArray[PGSd_TSF_KEYCSCECRTOECISETUP_MSG],
                       (void *) setup_msgTSF);
            }
            PGSg_TSF_CSCECRtoECIsetupStatus[masterTSFIndex] = setupStatusTSF;

	    
	    /* Change the Polar Motion units from seconds of arc to 
	       radians  */
	
	    xpolTSF[0] = xpole/206264.806;
            PGSg_TSF_CSCECRtoECIxpol[masterTSFIndex][0] = xpolTSF[0];
	    ypolTSF[0] = ypole/206264.806;
            PGSg_TSF_CSCECRtoECIypol[masterTSFIndex][0] = ypolTSF[0];
	
	    /* get the nutation angles and their rates and jedTDB */
	    
	    PGS_CSC_quickWahr(secTAI93, jedTDBTSF, dvnutTSF);
            for (x=0; x<4; x++)
            {
                PGSg_TSF_CSCECRtoECIdvnut[masterTSFIndex][x] = dvnutTSF[x];
            }
            for (x=0; x<2; x++)
            {
                PGSg_TSF_CSCECRtoECIjedTDB[masterTSFIndex][x] = jedTDBTSF[x];
            }
#else
	    if ((setupStatus=returnStatus) != PGS_S_SUCCESS)
		strcpy(setup_msg,msg);
	    
	    /* Change the Polar Motion units from seconds of arc to 
	       radians  */
	
	    xpol[0] = xpole/206264.806;
	    ypol[0] = ypole/206264.806;
	
	    /* get the nutation angles and their rates and jedTDB */
	    
	    PGS_CSC_quickWahr(secTAI93, jedTDB, dvnut);
#endif	    
	    /* Get the Greenwich Mean Sidereal time  GMST */
	
	    gmst = PGS_TD_gmst(jdUT1);
	
	    /* Get GAST (Greenwich Apparent Sidereal Time) from GMST.  Use the
	       obliquity of J2000 + the nutation in obliquity (see PROLOGUE).
	       Note: all these values are in radians.  One Sidereal Day = 2 pi
	       radians */
	
	    /* The following equation, or the expression beginning with "dvnut",
	       the nutation in longitude, is called "The Equation of the
	       Equinoxes." See Tables on on pp. B8 to B15 of the 1994
	       A.A. (Values there are in seconds of time.  Thus, to compare to
	       the analysis here, one must convert to radians.  To do that,
	       multiply seconds of time by 15 to get seconds of arc and divide
	       by 206264.806 to get radians) */
	
	    /* determine Earth rotation angle and rate about true N pole of
	       date; first component is GAST, the second is 0.000072921151467,
	       according to the Expl. Sup. p. 51, Eq. (2.24-3).  */
    
#ifdef _PGS_THREADSAFE

            /* Threadsafe protect: earthrot, dvnut,jedTDB,jedTDT, lastTAI93,
                 setupStatus, setup_msg    Reassign values for next use    */

	    earthrotTSF[0] = PGS_TD_gast(gmst, dvnutTSF[0], jedTDBTSF);
	    earthrotTSF[1] = EARTH_ROTATE_RATE;
            for (x = 0; x<2; x++)
            {
               PGSg_TSF_CSCECRtoECIearthrot[masterTSFIndex][x] = earthrotTSF[x];
            }
	    /* get TDT from TAI */

	    PGS_TD_TAItoTAIjd(secTAI93, jedTDTTSF);
            for (x=0; x<2; x++)
            {
                PGSg_TSF_CSCECRtoECIjedTDT[masterTSFIndex][x] = jedTDTTSF[x];
            }
	    PGS_TD_TAIjdtoTDTjed(jedTDTTSF, jedTDTTSF);
            for (x=0; x<2; x++)
            {
                PGSg_TSF_CSCECRtoECIjedTDT[masterTSFIndex][x] = jedTDTTSF[x];
            }
    
	    /* set lastTAI93 to secTAI93 to avoid doing the above setup if
	       we are reprocessing for the same time in the next iteration of
	       the loop or the next time this function is called */

	    lastTAI93TSF = secTAI93;    
            PGSg_TSF_CSCECRtoECIlastTAI93[masterTSFIndex] = lastTAI93TSF;
        
	}  /* END  Setup  */
	else if (setupStatusTSF != PGS_S_SUCCESS)
	{
	    /* If the current time is the same as some previous time make (and
	       the above setup section has therefore been skipped) report any
	       relevant status message. */

	    returnStatus = setupStatusTSF;
	    strcpy(msg,setup_msgTSF);
	}

#else
	    earthrot[0] = PGS_TD_gast(gmst, dvnut[0], jedTDB);
	    earthrot[1] = EARTH_ROTATE_RATE;

	    /* get TDT from TAI */

	    PGS_TD_TAItoTAIjd(secTAI93, jedTDT);
	    PGS_TD_TAIjdtoTDTjed(jedTDT, jedTDT);
    
	    /* set lastTAI93 to secTAI93 to avoid doing the above setup if
	       we are reprocessing for the same time in the next iteration of
	       the loop or the next time this function is called */

	    lastTAI93 = secTAI93;    
        
	}  /* END  Setup  */
	else if (setupStatus != PGS_S_SUCCESS)
	{
	    /* If the current time is the same as some previous time make (and
	       the above setup section has therefore been skipped) report any
	       relevant status message. */

	    returnStatus = setupStatus;
	    strcpy(msg,setup_msg);
	}
#endif

	/* WARNING: In principle we handle 6 - vectors here: position and
	   velocity.  But for distant objects (such as the Sun, or Mars) it
	   makes no sense to obtain their velocities as measured in the rotating
	   reference frame of the Earth.  We know the Sun and Saturn aren't
	   going on huge circles around us once a day.  (Nicholas Copernicus,
	   "De Revolutionibus Orbium Coelestium", 1540.)  Therefore, before we
	   rotate the state vector, we'll check its space components for length.
	   If the norm is less than 500,000,000 m (a little beyond the Moon)
	   we'll set to zero the velocity components of the vector we rotate.
	   No warning will be issued, because the tool may be called often where
	   the Sun or a star is in ECR, and too many messages would result.
	   We'll call PGS_CSC_rotate3 instead of PGS_CSC_rotat6 in order to
	   avoid calculating a meaningless velocity. */

	/* actually check dot product of vector with itself (rather than norm)
	   to avoid taking a square root, test the dot product against
	   500,000,000 (see comment above) squared (2.5E17) */

	if(PGS_CSC_dotProduct(posvelECR[cnt1],posvelECR[cnt1],3) < 2.5E17 )
	{
	    /* transform velocity */

	    threeOr6 = 6;
	    pgs_rotate = PGS_CSC_rotat6;
	}
	else
	{
	    /* do not transform velocity */

	    threeOr6 = 3;
	    pgs_rotate = PGS_CSC_rotat3;

	    /* set velocity components of transformed vector to zero (0.0) */

	    for (cnt2=3;cnt2<6;cnt2++)
		posvelECI[cnt1][cnt2] = 0.0;
	}

	/* The input vector still has not been touched.  Now transform it from
	   geographic coordinates to those based on the true pole and meridian
	   (i.e. compensate polar motion).  In fact, we'll take out the gmat
	   rotation about the true rotation axis just after that. */
    
	/*  Rotate to compensate polar motion in x (see A.A. p. B60, 
	    R2(+x) term)  */

#ifdef _PGS_THREADSAFE

       /* Threadsafe protect: xpol, ypol, earthrot, dvnut,jedTDT
           there is no change in calls so there is no reassignment*/

	pgs_rotate(posvelECR[cnt1], xpolTSF, 2, posvelECI[cnt1]);
    
	/*  Rotate to compensate polar motion in y (see A.A. p. B60, 
	    R1(+y) term)  */
	
	pgs_rotate(posvelECI[cnt1], ypolTSF, 1, posvelECI[cnt1]);

	/* The vector is now in ECI with equator and pole of date.  All
	   that remains is to rotate it from Celestial coordinates into
	   ECR based on the current Earth pole and Greenwich Apparent
	   Sidereal Time, gast. See A.A. p. B60 . */
    
	pgs_rotate(posvelECI[cnt1], earthrotTSF, -3, posvelECI[cnt1]);

	/* The vector is now in ECI with equator and pole of date.  All
	   that remains is to rotate it from Celestial coordinates of date
	   into ECI based on the epoch J2000 */
	
	/* nutate the vector to pole of J2000 */
	
	PGS_CSC_nutate2000(threeOr6, jedTDTTSF, dvnutTSF, PGS_FALSE, posvelECI[cnt1]);
	
	/* Precess the vector from jedTDT to J2000 */
	
	PGS_CSC_precs2000(threeOr6, jedTDTTSF, PGS_FALSE, posvelECI[cnt1]);
#else
	pgs_rotate(posvelECR[cnt1], xpol, 2, posvelECI[cnt1]);
    
	/*  Rotate to compensate polar motion in y (see A.A. p. B60, 
	    R1(+y) term)  */
	
	pgs_rotate(posvelECI[cnt1], ypol, 1, posvelECI[cnt1]);

	/* The vector is now in ECI with equator and pole of date.  All
	   that remains is to rotate it from Celestial coordinates into
	   ECR based on the current Earth pole and Greenwich Apparent
	   Sidereal Time, gast. See A.A. p. B60 . */
    
	pgs_rotate(posvelECI[cnt1], earthrot, -3, posvelECI[cnt1]);

	/* The vector is now in ECI with equator and pole of date.  All
	   that remains is to rotate it from Celestial coordinates of date
	   into ECI based on the epoch J2000 */
	
	/* nutate the vector to pole of J2000 */
	
	PGS_CSC_nutate2000(threeOr6, jedTDT, dvnut, PGS_FALSE, posvelECI[cnt1]);
	
	/* Precess the vector from jedTDT to J2000 */
	
	PGS_CSC_precs2000(threeOr6, jedTDT, PGS_FALSE, posvelECI[cnt1]);
#endif
    }
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
    else
        PGS_SMF_SetDynamicMsg(returnStatus, msg, FUNCTION_NAME);
    
    return returnStatus;
}
