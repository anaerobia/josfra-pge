/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   orbsim.c

DESCRIPTION:
   Orbit and attitude simulator.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation
   Peter D. Noerdlinger / SM&A Inc.
   Abe Taaheri / SM&A Inc.
   Xin Wang / EIT Inc.
   Abe Taaheri / L3 Comm, EER inc.

HISTORY:
   17-Oct-1994  GTSK  Initial version
   31-Jan-1995  GTSK  Changed the way that record times are calculated.
   21-Jul-1995  GTSK  Changed printf and scanf statements so that they are only
                      using "int" and "double" types (or pointers) instead of
		      "PGSt_integer" or "PGSt_double" types which may be of the 
		      wrong type (i.e. they are NOT necessarily "int" and
		      "double" respectively).
   12-Feb-1999  PDN   Fixed nomenclature consistent with AM1 and later spacecraft
                      note that the function is also OK for TRMM, but in that case
                      please observe that angularVelocity is really (yaw rate,
                      pitch rate, and roll rate in that order).
   28-Feb-1999  PDN   Added CHEM spacecraft (unmerged code)
   13-May-1999  PDN   Reduced the starting orbit number for simulations before
                      launch date from 116880 to 30303. This allows tests with
                      data server.
   04-Aug-1999  PDN   Merged additions for CHEM spacecraft; also merged an
                      undocumented change in the function head (see comment).
                      Added a return PGS_TRUE line when user fails to enter
                      y or Y to continue the run.
   20-Dec-1999  AT    Modified PM spcaecraft tags to agree with other TOOLKIT
                      tools (such as L0, and TD) to distinguish GIIS from GIRD,
		      although both refer to the same PM spacecraft.
   05-Sep-2000  AT    Modified PM spcaecraft tags to use EOSPM1 for spacecraft
                      name as DPREP. 
   11-Apr-2001  AT    Modified CHEM to AURA
   19-Oct-2001  XW    Modified time to support noon-to-noon ephemeris.
   30-Oct-2001  XW    Parameterized upon inputting orbital elements.
   10-Dec-2001  XW    Produced orbsim data in time intervals other than 24 hours.
   10-Feb-2002  XW    Create ephemeris and attitude data for HDF files.
   13-May-2003  AT    Increased FileName array sizes to accomodate long file
                      names and paths. Modified orbMetadata.orbitDescendTime
                      and orbMetadata.orbitAscendTime to keep only 3 digits
                      after decimal point since these times are calculated
                      only with 3 digits of precision. Without this fix the 
                      simulated Ascending and Descending times have different 
                      values for the same orbit in two consecutive eph 
                      files(e.g. Ascending time values differ in last 3 digits
                      after decimal point in two consecutive files). Also 
                      changed precision from .001 to .0001 to avoid mis-match
                      because of round off.
   30-Jun-2003  AT    Modified for epoch time as input orbital element.
   22-Jan-2004  PN    Changed radians to degrees.
   07-May-2007  AT    Corrected the calculation of integer_descend_time

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Spacecraft Orbit and Attitude Simulator

NAME:
   orbsim

SYNOPSIS:
C:



DESCRIPTION:


INPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
 
OUTPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
 
          
RETURNS:
   

EXAMPLES:
C:

FORTRAN:
  
NOTES:
   None

REQUIREMENTS:
   PGSTK - 0720

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_EPH_attOrbSim()
   PGS_TD_UTCtoTAI()
   PGS_TD_timeCheck()
   PGS_TD_ASCIItime_BtoA()
   PGS_TD_julday()
   PGS_TD_calday()

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <PGS_math.h>
#include <PGS_PC.h>
#include <PGS_TD.h>
#include <PGS_CSC.h>
#include <PGS_EPH.h>
#include <PGS_SIM.h>
#include <PGS_SMF.h>
#include <PGS_MET.h>
#include <hdf.h>
#include <mfhdf.h>

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define  P    data[9]
#define  e    data[1]
#define  ma   data[5]
#define  ap   data[4]

#define  E(x) (acos((e + cos(x))/(1 + e*cos(x))))

#define  PGSd_TO_TIP -1
#define  PGSd_TO_ORB  1

#define  PGSd_CUSTOM -7             /* for consideration as possible value of
				       s/c tag, used to initiate special
				       processing in this program */

#define  START_BEFORE_LAUNCH 30303  /* hoked up number to prevent negatives */
#define  MAX_TRIES       5U         /* maximum number of times a user may
				       attempt to enter in a requested input
				       quantity before the program gives up on
				       said user */

typedef unsigned long int EcTULongInt;  /* unsigned long int */

static void quit(void);
static int file_exists(char *);
static PGSt_boolean get_scTag(char *, PGSt_tag *);
static void tipEuler(PGSt_double [3], PGSt_double [3], PGSt_integer [3],
		     PGSt_double [3], PGSt_integer);
static PGSt_double PGS_EPH_OrbitAscendTime(PGSt_tag, PGSt_double, PGSt_double, 
		PGSt_double, PGSt_double, PGSt_double, PGSt_double, PGSt_double,PGSt_double);
static PGSt_double PGS_EPH_OrbitDecendTime(PGSt_tag, PGSt_double, PGSt_double, 
		PGSt_double, PGSt_double, PGSt_double, PGSt_double, PGSt_double,PGSt_double);
static PGSt_boolean PGS_EPH_writeMetadata(int, PGSt_ephemHeader, EcTULongInt, EcTULongInt, char[10][256], PGSt_uinteger, PGSt_ephemMetadata );

typedef struct
{
  double       btai;
  double       bea[3];
  double       etai;
  double       eea[3];
  PGSt_boolean cont;
  PGSt_boolean lock;
} PGSt_customAttit;

int main(int argc,char *argv[])  /* function type and calling list altered
                                    by PDN Aug 4, 1999 pursuant to other
                                    changes in the version at main/relb/4 */
{

    const int           DpCPrMaxUrLen            = 256;
    const int           DpCPrMaxScIdLen          = 24;
    const int           DpCPrMaxFrameLen         = 8;
    const int           DpCPrMaxTimeRangeLen     = 48;
    const int           DpCPrMaxSourceLen        = 32;
    const int           DpCPrMaxVersionLen       = 8;
    const int           DpCPrMaxKepler           = 6;
    const int           DpCPrMaxQaParameters     = 16;
    const int           DpCPrMaxQaStatistics     = 4;
    const int           DpCPrMaxEulerAngleOrder  = 3;
    const char  	DpCPrAttitudeVdataName[]  = "HDF Vdata Attitude Data";
					/* Name of the attitude Vdata */
    const char  	DpCPrEphemerisVdataName[]  = "HDF Vdata Ephemeris Data";
					/* Name of the ephemeris Vdata */
    const char  	DpCPrMetadataVdataName[] = "HDF Vdata Ephemeris Orbit Metadata";
    const PGSt_integer  DpCPrPreLongGapFlag     = 00000001000;    /* Bit 9. */
    const PGSt_integer  DpCPrGoodQualityFlag    = 00000000000;    /* No bits. */
    const PGSt_integer  DpCPrPostLongGapFlag    = 00000000100;    /* Bit 6. */
    const PGSt_integer  DpCPrRedLowFlag         = 00000000004;    /* Bit 2. */
    const PGSt_integer  DpCPrYellowLowFlag      = 00000000010;    /* Bit 3. */
    const PGSt_integer  DpCPrYellowHighFlag     = 00000000020;    /* Bit 4. */
    const PGSt_integer  DpCPrRedHighFlag        = 00000000040;    /* Bit 5. */
    const PGSt_integer  DpCPrQaQueueConfigFlag  = 00000004000;    /* Bit 11. */
    const PGSt_integer  DpCPrSafeModeFlag       = 00000400000;    /* Bit 17. */
    const PGSt_uinteger DpCPrBoundsCheck   = DpCPrRedLowFlag +
                                             DpCPrYellowLowFlag +
                                             DpCPrYellowHighFlag +
                                             DpCPrRedHighFlag +
                                             DpCPrQaQueueConfigFlag +
                                             DpCPrSafeModeFlag;

    PGSt_uinteger    nURs=0; 
    char             nativeURs[10][256];
    char             attitudeVdataName[512];
    char             ephemerisVdataName[512];
    char             metadataVdataName[512];
    int              hdfId, hdfIdAtt;
    int32            hdfStatus;
    PGSt_integer     hdfStatus1;
    int              hdfSdId, hdfSdIdAtt;
    int              iElm;
  /*  int              iOrb; */
    int              nRecordsAfter;
    int              nRecordsBefore;
    int              vDataId, vDataIdAtt;
    double           granuleEndTime;
    double           granuleStartTime;
    EcTULongInt      qaLongGapCount=0;
    EcTULongInt      qaOutofBoundsDataCount=0;
    PGSt_uinteger    qaCheck;
    uchar8           *hdfRecord, *hdfRecordAtt;
    PGSt_byte        *hdfMeta;
    int32 	     vDataRef, vDataRefAtt; /* Vdata reference number */
    char 	     headerVdataName[] = {"HDF Vdata Ephemeris Header"}; 
				/* Name of the ephemeris header Vdata */
    char             headerVdataNameAtt[] = {"HDF Vdata Attitude Header"};
				/* Name of the attitude header Vdata */
    unsigned         char *hdfHeader;	/* pointer to HDF ephemeris file Header */
    unsigned         char *hdfHeaderAtt; /* pointer to HDF attitude file Header */
    unsigned         char *pntr, *pntrAtt;
    
    PGSt_ephemHeader ephemFileHeader;
    PGSt_attitHeader attitFileHeader;
    
    PGSt_ephemRecord ephemRecord;
    PGSt_attitRecord attitRecord;

    PGSt_ephemMetadata orbMetadata;
    
    PGSt_tag     spacecraftTag;
    
    double       yprNoise;
    double       yprRateNoise;

    PGSt_integer startOrbitNum;       /* orbit number of first orbit in file */

    PGSt_double  data[10];            /* keplerian orbital elements and other
					 stuff */
    PGSt_double  startTAI93;          /* file start time in seconds since 12AM
					 UTC 1-1-1993 */
    PGSt_double  stopTAI93;           /* file start time in seconds since 12AM
					 UTC 1-1-1993 */
    PGSt_double  defaultInterval;     /* default time interval between data sets
					 (spacecraft dependent) */
    PGSt_double  dataBytes;           /* number of bytes in a data record */
    PGSt_double  transform[3][3];     /* s/c to ECI transformation matrix */
    PGSt_double  numIntervals;        /* number of records in a file */

    PGSt_double  posvelECI[6];
    PGSt_double  posvelECR[6];
    PGSt_double  latitude;
    PGSt_double  longitude;
    PGSt_double  altitude;
    PGSt_double  time;
    PGSt_double  dt1;
    PGSt_double  dt2;
 /* PGSt_double  nu0; */
 /*   PGSt_double  nu; */
 /*   PGSt_double  k;  */
    PGSt_double  quatEuler[4];
    PGSt_double  quatORBtoECI[4];
    PGSt_double  quatSCtoECI[4];
    PGSt_double  quatECItoSC_0[4];
    PGSt_double  quatORBtoSC_0[4];
    PGSt_double  eulerAngle[3];
    PGSt_double  angularVelocity[3];
    PGSt_double  old_eea[3];
    PGSt_double  old_tai;

    PGSt_double  aIn;                 /* semi-major axis (m) */
    PGSt_double  eIn;                 /* eccentricity */
    PGSt_double  incIn;               /* inclination (degrees) */
    PGSt_double  anIn;                /* right ascension of ascending node (degrees) */
    PGSt_double  apIn;                /* argument of perigee (degrees) */ 
    PGSt_double  maIn;                /* mean anomaly at time (degrees) */ 

    double       tempDouble;          /* temporary "double" for use with scanf()
				       */
    unsigned int numTries;            /* number of times the user has attempted
					 to input something */
    unsigned int numTries_subloop;    /* number of times the user has attempted
					 to input something for a sub loop */
    int          numCheck;            /* used to verify calls to sscanf() and
					 fwrite() */
    int          year;                /* year */
    int          start_year;          /* start year */
    int          tmp_year;            /* temp year */
    int          month;               /* month */
    int          start_month;         /* start month */
    int          tmp_month;           /* temp month */
    int          day;                 /* day */
    int          start_day;           /* start day */
    int          tmp_day;             /* temp day */
    int          hour;                /* hour */ 
    int          start_hour;          /* start hour */
    int          stop_hour;           /* stop hour */
    int          hourInt;             /* hour interval */
    int          timeInt;             /* temporary int */
    int          numFiles;            /* number of files */
    int          numCount;            /* file number counter */
    int          leapYear;            /* 1 if year is leap year, otherwise 0 */

    PGSt_integer julianDay;           /* Julian day of file */
    PGSt_integer startJD;             /* Julian day of start day */
    PGSt_integer stopJD;              /* Julian day of stop day */
    PGSt_integer version=1;           /* version number for PC file */
    PGSt_integer count;               /* looping variable */
    PGSt_integer eao[3];
    PGSt_integer num_periods;
    PGSt_integer periodNum;
    
    PGSt_boolean gotData;             /* used to determine if an input value as
					 been successfully ingested */
    PGSt_boolean gotElem;            /* used to determine if an input value as
					 been successfully ingested */
    PGSt_boolean noon;                /* used to determine if start from noon */
    PGSt_boolean time_check;          /* used to determine if input time */
    PGSt_boolean timeInterval;        /* used to determine time interval */
    PGSt_boolean start_check;         /* used to determine start time */
    PGSt_boolean stop_check;          /* used to determine stop  time */
    PGSt_boolean hdf_file;	      /* used to determine if create HDF files */

    PGSt_boolean firstTime=PGS_TRUE;
    PGSt_boolean custom=PGS_FALSE;

    PGSt_customAttit ca[100];
    PGSt_customAttit tmp_ca;
   
  /*  size_t length; */

    char         *loc1;
    char         *loc2;
    char         *charPtr;
    char         attitudeNoise[12];   /* attitude noise string for simulator */
    char         inputBuffer[400];    /* input buffer */
    char         ephFilename[400];    /* name of s/c ephemeris file */
    char         attFilename[400];    /* name of s/c attitude file */
    char         ephHdfFilename[400]; /* name of HDF ephemeris file */
    char         attHdfFilename[400]; /* name of HDF attitude file */
    char         ephemDirectory[400]; /* name of directory for s/c ephemeris
					 files */
    char         scName[10];          /* spacecraft name */
    char         asciiUTC[28];        /* UTC time */
    char         startUTC[28];        /* beginning UTC time */
    char         stopUTC[28];         /* beginning UTC time */
    char         startUTC_print[28];  /* beginning UTC time (to stdout) */
    char         stopUTC_print[28];   /* beginning UTC time (to stdout) */
    char         startUTC_tmp[28];    /* temporary begin UTC time */ 
    char         stopUTC_tmp[28];     /* temporary stop UTC time */
    char         *newlinePtr;         /* pointer to newline character in the
					 input buffer */
    char         parentUR[PGSd_UR_FIELD_SIZE];

    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* status mnemonic
							 returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* status messsage
							 returned by call to
							 PGS_SMF_GetMsg() */
    
    FILE         *ephemFilePtr;       /* pointer to s/c ephemeris file */
    FILE         *attitFilePtr;       /* pointer to s/c attitude file */
    
    DIR          *directoryPtr;       /* pointer to directory for s/c ephemeris
					 files */
    
    PGSt_SMF_status returnStatus;     /* return status of calls to PGS toolkit
					 functions */
    PGSt_SMF_status code;             /* status code returned by
					 PGS_SMF_GetMsg() */
   
    double expectedDataInterval = 60.0; /* Expected data interval */
    double mySegmentDuration = 7200;	/* Data segment duration */
    double myClockEps = 0.002;		/* Clock epsilon */

    char XasciiUTC[28];			/* UTC time */
    char Xhour[26];			/* Hour */
    char XintervalStart[26];		/* Interval start time */
    char XstartUTC[26];			/* start UTC time */
    char XyearAndDayNumber[26];		/* year and day in UTC time */
    int  XstartHour;			/* Start hour */
    PGSt_double  XendReqTime;		/* Request end time */
    PGSt_double  XstartReqTime;		/* Request start time */
    PGSt_double  XendLimit;		/* End time */
    PGSt_double  XstartLimit;		/* Start time */
    long int              integer_descend_time;
    long int              integer_ascend_time;
    PGSt_double  epoch;
    char         epoch_tm[28];  /* epoch in CCSDS ASCII time code A */
    char *ptrpwd;

    float64 initF = 0.0;
    int32 initI = 0;
 
    /*******************
     * BEGIN EXECUTION *
     *******************/
    epoch = -1.; /* set default epoch time to a negative number. Real time
		  will be calculated from real default value for a given
		  spacecraft */
    scName[0] = '\0';

    system("clear\n");

    /* print out lovely picture because Tejmo was bored one day */

    printf("\n       ********************\n"
	   "       * ------O--------- *\n"
	   "       * ___/\\__/\\_______ *\n"
	   "       * __/  \\/  \\______ *\n"
	   "       *  /    \\___\\_____ *\n"
	   "       * /                *\n"
	   "       * ^^^^^^^^^^^^^^^^ *\n"
	   "       * ^^^^^^^^^^^^^^^^ *\n"
	   "       * ^^^^^^^^^^^^^^^^ *\n"
	   "       * =EOS=            *\n"
	   "       ********************\n\n");
    printf("  ECS SPACECRAFT ORBIT and ATTITUDE SIMULATOR\n\n");

    memset((void*)parentUR, (unsigned char)0, PGSd_UR_FIELD_SIZE);
    strcpy(parentUR, "SIMULATED DATA");
    
  BEGINNING:;
    memset((void*)&ephemFileHeader, (unsigned char)0, sizeof(PGSt_ephemHeader));
    memset((void*)&attitFileHeader, (unsigned char)0, sizeof(PGSt_attitHeader));
    memset((void*)&tmp_ca, (unsigned char)0, sizeof(PGSt_customAttit));
    num_periods = 0;

    printf("Enter <return> at a prompt to select the default\n"
	   "option (indicated by []).  Enter 'q' at any prompt\n"
	   "to quit.\n\n");
    
    gotData = PGS_FALSE;
    noon = PGS_FALSE;
    time_check = PGS_FALSE;
    timeInterval= PGS_FALSE;
    hdf_file = PGS_FALSE;

    numTries = 0;
 	
    /* determine spacecraft, set appropriate values to the spacecraft name
       string (scName) and default time interval (defaultInterval) */
   
    do
    {
	if (strlen(scName) == 0)
	  printf("enter spacecraft ID (TRMM, EOS_AM, EOS_PM, EOS_AURA):\n"
		 "-->");
	else
	  printf("enter spacecraft ID (TRMM, EOS_AM, EOS_PM, EOS_AURA) [%s]:\n"
		 "-->",scName);
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '\n' && strlen(scName) != 0)
	  strcpy(inputBuffer,scName);
	newlinePtr = strchr(inputBuffer,'\n');
	if (newlinePtr != NULL)
	  *newlinePtr = '\0';
	
	if (get_scTag(inputBuffer,&spacecraftTag) == PGS_FALSE)
	  continue;
	   
	gotData = PGS_TRUE;
	switch (spacecraftTag)
	{
	  case PGSd_TRMM:
	    strcpy(scName,"TRMM");
	    strcpy(epoch_tm,"1997-10-01T23:00:00Z");
	    returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
	    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		returnStatus != PGSTD_E_NO_LEAP_SECS)
		return PGS_E_TOOLKIT;
	    defaultInterval = 60.0;
	    eao[0] = 3;
	    eao[1] = 2;
	    eao[2] = 1;
	    aIn=6729390.;
	    eIn=.00053998;
	    incIn=35.;
	    anIn=0.0;
	    apIn=90.0;
	    maIn=270.0;
	    break;
	  case PGSd_EOS_AM:
	    strcpy(scName,"EOSAM1");
	    strcpy(epoch_tm,"1998-06-30T10:51:28.32Z");
	    returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
	    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		returnStatus != PGSTD_E_NO_LEAP_SECS)
		return PGS_E_TOOLKIT;
	    defaultInterval = 1.024;
	    eao[0] = 3;
	    eao[1] = 1;
	    eao[2] = 2;
	    aIn=7086930.;
	    eIn=.001281620;
	    incIn=98.199990;
	    anIn=255.355971130;
	    apIn=69.086962170;
	    maIn=290.912925280;
	    break;
	  case PGSd_EOS_PM:
	    strcpy(scName,"EOSPM1");
	    strcpy(epoch_tm,"2000-12-01T10:51:28.32Z");
	    returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
	    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		returnStatus != PGSTD_E_NO_LEAP_SECS)
		return PGS_E_TOOLKIT;
	    defaultInterval = 1.024;
	    eao[0] = 3;
	    eao[1] = 1;
	    eao[2] = 2;
	    aIn=7077589.2;
	    eIn=.0012;
	    incIn=98.145;
	    anIn=298.54;
	    apIn=90.;
	    maIn=270.;
	    break;
	  case PGSd_EOS_AURA: /* Probable order from Joe Guzek */
	    strcpy(scName,"EOSAURA");
	    strcpy(epoch_tm,"1998-06-20T16:58:46.466Z");
	    returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
	    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		returnStatus != PGSTD_E_NO_LEAP_SECS)
		return PGS_E_TOOLKIT;
	    defaultInterval = 1.024;
	    eao[0] = 3;
	    eao[1] = 1;
	    eao[2] = 2;
	    aIn=7078500.0;
	    eIn=.00116;
	    incIn=98.20;
	    anIn=115;
	    apIn=90.;
	    maIn=270.;
	    break;
	  case PGSd_CUSTOM:
	    custom = PGS_TRUE;
	    printf("\ncustom mode enabled...\n\n");
	    goto BEGINNING;
	  default:
	    gotData = PGS_FALSE;
	}	    
    }
    while (gotData == PGS_FALSE && numTries++ < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();

    /*  get answer to create HDF files */
    numTries = 0;
    gotData = PGS_FALSE;
    do
    {
        printf("\nDo you want to create HDF format files? (yes/no)[no]\n-->");
        fgets(inputBuffer,99,stdin);
        if (inputBuffer[0] == 'Y' || inputBuffer[0] == 'y')
        {
           hdf_file = PGS_TRUE;
        }
        gotData = PGS_TRUE;
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
        quit();

    numTries = 0;
    gotElem = PGS_FALSE;
    /* get orbital elements */
    do
    {
        printf("\nDo you want to change orbital elements? (yes/no)[no]\n-->");
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'Y' || inputBuffer[0] == 'y')
	  {
	    printf("\nDefault epoch time = %s. Click enter to use default. "
		   "Or input new value in CCSDS ASCII (format A or B). ", epoch_tm);
	    numTries_subloop = 0;
	    gotData = PGS_FALSE;
	    do
	      {
		printf("Enter epoch time:\n-->");
		fgets(inputBuffer,99,stdin);
		if (inputBuffer[0] == 'q')
		  quit();
		if (inputBuffer[0] == '\n')
		  {
		    gotData = PGS_TRUE;
		    break;
		  }
		newlinePtr = strchr(inputBuffer,'\n');
		if (newlinePtr != NULL)
		  *newlinePtr = '\0';
		returnStatus = PGS_TD_timeCheck(inputBuffer);
		switch (returnStatus)
		  {
		  case PGS_S_SUCCESS:
		    strcpy(epoch_tm, inputBuffer);
		    gotData = PGS_TRUE;
		    break;
		  case PGSTD_M_ASCII_TIME_FMT_B:
		    returnStatus = PGS_TD_ASCIItime_BtoA(inputBuffer,epoch_tm);
		    if (returnStatus != PGS_S_SUCCESS)
		      continue;
		    gotData = PGS_TRUE;
		    break;
		  default:
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    printf("\n%s\n\n",msg);
		    continue;
		  }
	      }
	    while (gotData == PGS_FALSE && ++numTries_subloop < MAX_TRIES);
	    if (gotData == PGS_FALSE)
	      quit();

	    returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
	    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		returnStatus != PGSTD_E_NO_LEAP_SECS)
	      return PGS_E_TOOLKIT;

	    printf("\nDefault semi-major axis (m) = %f. Click enter to use default. "
		   "Or input new value:\n-->", aIn);
	    fgets(inputBuffer,99,stdin);
            if (inputBuffer[0] == 'q')
		quit();
            if (inputBuffer[0] != '\n')
            {
		numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
		if (numCheck == 0)
	  	    continue;
		aIn = tempDouble;
            }

            printf("\nDefault eccentricity = %f. Click enter to use default. "
		   "Or input new value:\n-->", eIn);
            fgets(inputBuffer,99,stdin);
            if (inputBuffer[0] == 'q')
        	quit();
            if (inputBuffer[0] != '\n')
            {
        	numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
        	if (numCheck == 0)
          	    continue;
        	eIn = tempDouble;
      	    }

      	    printf("\nDefault inclination (degrees) = %f. Click enter to use default. "
		   "Or input new value:\n-->", incIn);
            fgets(inputBuffer,99,stdin);
            if (inputBuffer[0] == 'q')
        	quit();
            if (inputBuffer[0] != '\n')
            {
        	numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
        	if (numCheck == 0)
          	    continue;
        	incIn= tempDouble;
            }

      	    printf("\nDefault right ascension of ascending node (degrees) = %f. "
		   "Click enter to use default. Or input new value:\n-->", anIn);
      	    fgets(inputBuffer,99,stdin);
      	    if (inputBuffer[0] == 'q')
        	quit();
      	    if (inputBuffer[0] != '\n')
      	    {
        	numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
        	if (numCheck == 0)
          	    continue;
        	anIn= tempDouble;
            }

      	    printf("\nDefault argument of perigee (degrees) = %f. "
		   "Click enter to use default. Or input new value:\n-->", apIn);
      	    fgets(inputBuffer,99,stdin);
      	    if (inputBuffer[0] == 'q')
        	quit();
      	    if (inputBuffer[0] != '\n')
            {
        	numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
        	if (numCheck == 0)
          	    continue;
        	apIn= tempDouble;
            }

      	    printf("\nDefault mean anomaly at time (degrees) = %f. "
		   "Click enter to use default. Or input new value:\n-->", maIn);
      	    fgets(inputBuffer,99,stdin);
      	    if (inputBuffer[0] == 'q')
        	quit();
      	    if (inputBuffer[0] != '\n')
      	    {
        	numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
        	if (numCheck == 0)
          	    continue;
        	maIn= tempDouble;
            }
	    gotElem = PGS_TRUE;
        }
	else if (inputBuffer[0] == 'N' || inputBuffer[0] == 'n' || 
		 inputBuffer[0] == '\n')
	  {
	    gotElem = PGS_TRUE;
	  }
    }
    while (gotElem == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotElem == PGS_FALSE)
        quit();

    
    /* get start and stop days and time interval */
    do /* GET TIMES */
    {
	if (firstTime == PGS_TRUE)
	{
	    printf("\nenter start and stop dates in CCSDS ASCII "
		   "(format A or B)\n");
	    printf(" A) YYYY-MM-DD\n B) YYYY-DDD\n\n");
	    printf(" 1) 1995-10-20T00:00:00 and 1995-10-20 for starting from midnight.\n");
	    printf(" 2) 1995-10-20T12:00:00 for starting from noon.\n\n");
	  
	}
	else
	  printf("\n");
	    
	gotData = PGS_FALSE;
	numTries = 0;
	
	/* get start day */

	do
	{
	    printf("enter start date:\n-->");
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	      quit();
	    newlinePtr = strchr(inputBuffer,'\n');
	    if (newlinePtr != NULL)
	      *newlinePtr = '\0';
	    loc1=strstr( inputBuffer,"T12:00:00");
	    loc2=strstr( inputBuffer,"T00:00:00");
	    if ( loc1 != NULL)
		noon=PGS_TRUE;
	    if ( loc2 != NULL)
		time_check=PGS_TRUE;
	    returnStatus = PGS_TD_timeCheck(inputBuffer);
	    switch (returnStatus)
	    {
	      case PGS_S_SUCCESS:
	        if (noon == PGS_TRUE || time_check== PGS_TRUE )
	        {
	            strncpy(startUTC,inputBuffer,19);
  		    startUTC[19] = '\0'; 
	        }
	        else
	        {
  		    strncpy(startUTC,inputBuffer,10);
  		    startUTC[10] = '\0';
	        }
		strcpy(startUTC_print,startUTC);
		gotData = PGS_TRUE;
		break;
	      case PGSTD_M_ASCII_TIME_FMT_B:
		returnStatus = PGS_TD_ASCIItime_BtoA(inputBuffer,startUTC);
		if (returnStatus != PGS_S_SUCCESS)
		  continue;
		strcpy(startUTC_print,inputBuffer);
		if (noon == PGS_TRUE || time_check== PGS_TRUE )
		{
  		    startUTC[19] = '\0';
 		    startUTC_print[17] = '\0';
		}
		else
		{
  		    startUTC[10] = '\0';
  		    startUTC_print[8] = '\0';
		}
		gotData = PGS_TRUE;
		break;
	      default:
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		printf("\n%s\n\n",msg);
		continue;
	    }
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	  quit();

	gotData = PGS_FALSE;
	numTries = 0;
	
	/* get stop day, make sure stop day is not less than start day (the
	   default case is that the stop day is the same as the start day) */

	do
	{
	    printf("enter stop date [%.19s]:\n-->",startUTC_print);
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	      quit();
	    if (inputBuffer[0] == '\n')
	      strcpy(inputBuffer,startUTC_print);
	    newlinePtr = strchr(inputBuffer,'\n');
	    if (newlinePtr != NULL)
	      *newlinePtr = '\0';
	    returnStatus = PGS_TD_timeCheck(inputBuffer);
	    switch (returnStatus)
	    {
	      case PGS_S_SUCCESS:
		if (noon == PGS_TRUE || time_check== PGS_TRUE )
		{
  		    strncpy(stopUTC,inputBuffer,19);
  		    stopUTC[19] = '\0';
		}
		else
		{
  		    strncpy(stopUTC,inputBuffer,10);
  		    stopUTC[10] = '\0';
		}

		strcpy(stopUTC_print,stopUTC);
		gotData = PGS_TRUE;
		break;
	      case PGSTD_M_ASCII_TIME_FMT_B:
		returnStatus = PGS_TD_ASCIItime_BtoA(inputBuffer,stopUTC);
		if (returnStatus != PGS_S_SUCCESS)
		  continue;
		strcpy(stopUTC_print,inputBuffer);
		if (noon == PGS_TRUE || time_check== PGS_TRUE )
		{
  		    stopUTC[19] = '\0';
  		    stopUTC_print[17] = '\0';
		}
		else
		{
  		    stopUTC[10] = '\0';
  		    stopUTC_print[8] = '\0';
		}

		gotData = PGS_TRUE;
		break;
	      default:
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		printf("\n%s\n\n",msg);
		continue;
	    }

	    if (noon == PGS_TRUE || time_check== PGS_TRUE )
	    {
		if (strcmp(stopUTC,startUTC) == 0 )
  		{
      		    printf("\nstop time must be greater than start time\n\n");
      		    gotData = PGS_FALSE;
  		}
	    }
	    if (strcmp(stopUTC,startUTC) < 0)
	    {
		printf("\nstop time must be greater than or equal "
		       "to start time\n\n");
		gotData = PGS_FALSE;
	    }
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	  quit();

	gotData = PGS_FALSE;
	numTries = 0;
	
	/* get time interval */

	do
	{
	    printf("enter time interval in seconds [%f sec]:\n"
		   "-->",(double) defaultInterval);
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	      quit();
	    if (inputBuffer[0] != '\n')
	    {
		numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
		if (numCheck == 0)
		  continue;
		defaultInterval = tempDouble;
	    }
	    
	    gotData = PGS_TRUE;
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	  quit();


	gotData = PGS_FALSE;
	numTries = 0;

        do
        {
            printf("\nTK creates a whole day data file. Accept it? (yes/no)[yes]\n-->");
            fgets(inputBuffer,99,stdin);
            if (inputBuffer[0] == 'Y' || inputBuffer[0] == 'y' || inputBuffer[0] == '\n')
                timeInt=24;
            else
            {
                printf("\nHow many hours of each file? (1, 2, 3, 4, 6, 12, 24) \n--> ");
                fgets(inputBuffer,99,stdin);
                numCheck = sscanf(inputBuffer,"%d",&hourInt);
                if (numCheck == 0)
                    continue;
                switch (hourInt)
                {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 6:
                    case 12:
                    case 24:
                       timeInt = hourInt;
                       timeInterval=PGS_TRUE;
                       break;
                    default:
                       printf("\nPlease input the divisor of 24.(e.g., 1, 2, 3, 4, 6, 12, 24)\n");
                       quit();
                }
            }
            gotData = PGS_TRUE;
        }
        while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
        if (gotData == PGS_FALSE)
            quit();
	
	/* Calculate total size of data files to be generated.  The total size
	   of the data files generated will be approximately equal to the
	   (number of days requested)*(seconds per day)/(data time interval)*
	   (size in bytes of each record). */

	returnStatus = PGS_TD_UTCtoTAI(startUTC,&startTAI93);
	if (returnStatus == PGS_E_TOOLKIT)
	{
	    PGS_TD_UTCtoTAI(startUTC,&startTAI93);
	}
	
	PGS_TD_UTCtoTAI(stopUTC,&stopTAI93);
	if (noon == PGS_FALSE && time_check== PGS_FALSE)
	    stopTAI93 += 86400.0;
	dataBytes = (stopTAI93 - startTAI93 + 86400.)/
	            defaultInterval*sizeof(PGSt_ephemRecord); 
	dataBytes += (stopTAI93 - startTAI93 + 86400.)/
	             defaultInterval*sizeof(PGSt_attitRecord); 

	/* Print out the start and stop days, time interval and the
	   corresponding total size (in MB) of the files this will generate.
	   This is here because these files can be quite big. */

	printf("\nstart day:    %s\nstop day:     %s\ntime interval: %.4f"
	       " seconds\n\nsemi-major axis(m):            %.1f \n"
	       "eccentricity:                  %.9f \ninclination (degrees):"
	       "         %.6f \nright ascension of ascending node (degrees):"
	       "%.9f \nargument of perigee (degrees): %.9f \nmean anomaly at time "
	       "(degrees):%.9f\nepoch time:   %s \n\n",startUTC_print, stopUTC_print,(double) defaultInterval,
	       aIn, eIn, incIn, anIn, apIn, maIn, epoch_tm);
	printf("This will create approximately %.2f MB of data.\n",
	       (double) (dataBytes/1.e6));

	gotData = PGS_FALSE;
	numTries = 0;

	/* Verify start and stop time and time interval input.  This is to give
	   the user a chance to alter the input values if the size of the files
	   to be generated is larger than anticipated. */

	do
	{
	    printf("accept ([y]/n)?\n-->");
	    fgets(inputBuffer,99,stdin);
	    switch (inputBuffer[0])
	    {
	      case '\n':
	      case 'y':
	      case 'Y':
		gotData = PGS_TRUE;
		numTries = 2*MAX_TRIES;
		break;
	      case 'n':
	      case 'N':
		numTries = 2*MAX_TRIES;
		break;
	      case 'q':
		quit();
	    }
	}
	while (numTries++ < MAX_TRIES);

    } /* end do: GET TIMES */
    while (gotData == PGS_FALSE);

    /* get attitude noise */
    if (firstTime == PGS_TRUE)
      printf("\nYou may introduce random noise in the attitude and attitude\n"
	     "rates.  This noise will be deviations from the nominal values\n"
	     "of zero for these quantities (i.e. spacecraft reference frame\n"
	     "identical to orbital reference frame).  The number entered will\n"
	     "be the maximum deviation (+/-) from the nominal values.  The same"
	     "\nvalue will be used for all the components of a particular\n"
	     "quantity.  Enter 'N' at the first prompt for no noise at all.\n"
	     "Otherwise entering zero for a particular quantity will preclude\n"
	     "noise in that state.  The default case is no noise.\n"
	     "Enter noise level in arcseconds (0.00-999.99).\n\n");
    else
      printf("\nEnter noise level in arcseconds (0.00-999.99) or N for no"
	     " noise.\n\n");
    /* get attitude noise */

    numTries = 0;
    gotData = PGS_FALSE;
    do
    {
	printf("enter attitude noise level [N]:\n-->");
	fgets(inputBuffer,99,stdin);
	switch (inputBuffer[0])
	{
	  case '\n':
	  case 'n':
	  case 'N':
	    strcpy(attitudeNoise,"00000000000");
            yprNoise = 0.0;
	    gotData = PGS_TRUE;
	    break;
	  case 'q':
	    quit();
	  default:
	    attitudeNoise[0] = 'N';
	    numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
	    if (numCheck == 1)
		if ( (tempDouble >= 0.0) && (tempDouble <= 999.99) )
		{
		    gotData = PGS_TRUE;
		    yprNoise = (PGSt_double) tempDouble;
		}
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    /* get attitude rate noise */

    numTries = 0;
    gotData = PGS_FALSE;
    do
    {
	if (attitudeNoise[0] != 'N')
	{
	    gotData = PGS_TRUE;
	    yprRateNoise = 0.0;
	    continue;
	}
	printf("enter attitude rate noise level [%.2f]:\n-->",
	       (double) yprNoise);
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '\n')
	  sprintf(inputBuffer,"%f",(double) yprNoise);
	numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
	if (numCheck == 1)
	    if ( (tempDouble >= 0.0) && (tempDouble <= 999.99) )
	    {
		gotData = PGS_TRUE;
		yprRateNoise = (PGSt_double) tempDouble;
		sprintf(attitudeNoise+1,"%05d%05d",
			(int)(yprNoise*100.),
			(int)(yprRateNoise*100.));
	    }
    }
    while (gotData == PGS_FALSE && numTries++ < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    /* Get default directory from the process control file.  This file
       associates a physical file name with the logical identifier 
       PGSd_SC_EPHEM_DATA (in this case it is actually a directory name).  This
       is where the SDP (aka PGS) toolkit spacecraft ephemeris access tool 
       (PGS_EPH_EphemAttit()) expects to find s/c ephemeris files. */
    
    if (firstTime == PGS_TRUE)
    {
	version = 1;
	ephemDirectory[0] = '\0';
	returnStatus = PGS_PC_GetReference(10601,&version,
					   ephemDirectory);
	charPtr = strrchr(ephemDirectory, 'C');
	if (charPtr != NULL)
	{
	    *charPtr = '\0';
	    strcat(ephemDirectory, "EPH");
	}
	printf("\n");
	if (returnStatus != PGS_S_SUCCESS || charPtr == NULL)
	{
	    printf("Unable to determine default ephemeris file location.\n"
		   "Setting default output location to current directory.\n\n");
	    ptrpwd = (char *)getenv("PWD");
	    if (ptrpwd == NULL)
	      {
		printf(" failed to get environment variable PWD. Setting default output location to \".\". \n");
		strcpy(ephemDirectory,".");
	      }
	    else
	      {
		strcpy(ephemDirectory, ptrpwd);
	      }
	}
	else    
	  printf("By default this program will install the files it generates "
		 "in: %s.\n\n",ephemDirectory);
    }
    else 
      printf("\n");
    
    numTries = 0;
    gotData = PGS_FALSE;
    
    /* get install directory, make sure that the specified directory can be
       seen by the user */

    do
    {
	printf("install files in [%s]:\n-->",ephemDirectory);
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q' && strlen(inputBuffer) == 2)
	  quit();
	if (inputBuffer[0] != '\n')
	{
	    newlinePtr = strchr(inputBuffer,'\n');
	    if (newlinePtr != NULL)
	      *newlinePtr = '\0';
	}
	else
	  strcpy(inputBuffer,ephemDirectory);
	
	directoryPtr = opendir(inputBuffer);
	if (directoryPtr == NULL)
	  printf("\nunable to open directory: %s\n\n",inputBuffer);
	else
	{
	    closedir(directoryPtr);
	    strcpy(ephemDirectory,inputBuffer);
	    gotData = PGS_TRUE;
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);

    /* this section is executed only if we are doing attitude customization
       this feature is NOT an advertised one and therefore is likely only to be
       executed in house, at least until PDN tells everyone they can use it */

    if (custom == PGS_TRUE)
    {
	memset((void*)ca, (unsigned char)0, sizeof(PGSt_customAttit));
	printf("\nYou are now in the CUSTOM zone.  You may customize the "
	       "attitude of\nyour spacecraft here.  You will be prompted "
	       "for a start time, a stop\ntime, a starting set of euler "
	       "angles and an ending set of euler\nangles.  You will then "
	       "be asked if you would like to specify another\ncustom "
	       "period, at which point if you answer YES then the above "
	       "prompts\nwill be repeated.\n\nFor each period enter the "
	       "start time and the stop time of the period.\nThese times "
	       "need not fall within the time span of your current file\n"
	       "generation request but if there is no overlap at all then "
	       "you will get\nthe default file (i.e. you will have wasted a "
	       "lot of FLOPS and your\ntime).  Then at the first prompt for "
	       "euler angles enter the values of\nthe euler angles in the "
	       "order indicated (by the prompt) as three\nfloating point "
	       "numbers separated by spaces (on one line) (or\n'continue', "
	       "see below).  The simulator will force the the s/c attitude\n"
	       "to these values at the start time for this period.  At the "
	       "second\nprompt you may enter either another set of euler "
	       "angles OR the word\n'lock'.  If you enter 'lock', then the "
	       "s/c will enter an inertial lock\nmode and maintain its "
	       "current attitude relative to ECI.  If you enter\nanother "
	       "set of euler angles the spacecraft will rotate about a "
	       "fixed\naxis from the position defined by the first set of "
	       "euler angles at the\nstart time to the position defined by "
	       "the second set of euler angles\nat the end time.\n\nFor "
	       "each NEW period the s/c attitude will be set according to "
	       "the\nvalue of the starting Euler angles.  If you want to "
	       "ensure continuity,\nenter 'continue' at the first prompt.\n"
	       );

	count = 0;
	periodNum = -1;
	old_eea[0] = 0.0;
	old_eea[1] = 0.0;
	old_eea[2] = 0.0;

	do
	{
	    /* get start day */
	    
	    gotData = PGS_FALSE;

	    do
	    {
		printf("\nenter period start time (in CCSDS ASCII format):\n"
		       "-->");
		fgets(inputBuffer,99,stdin);
		if (inputBuffer[0] == 'q')
		  quit();
		newlinePtr = strchr(inputBuffer,'\n');
		if (newlinePtr != NULL)
		  *newlinePtr = '\0';
		returnStatus = PGS_TD_UTCtoTAI(inputBuffer, &ca[count].btai);
		switch (returnStatus)
		{
		  case PGS_S_SUCCESS:
		  case PGSTD_E_NO_LEAP_SECS:
		    if (ca[count].btai > stopTAI93)
		    {
			printf("\nInvalid period start time (start time is "
			       "AFTER ending time of this data set).\n");
			break;
		    }
		    gotData = PGS_TRUE;
		    break;
		  default:
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    printf("\n%s\n\n",msg);
		    continue;
		}
	    }
	    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	    if (gotData == PGS_FALSE)
	      quit();

	    gotData = PGS_FALSE;

	    do
	    {
		printf("\nenter period stop time (in CCSDS ASCII format):\n"
		       "-->");
		fgets(inputBuffer,99,stdin);
		if (inputBuffer[0] == 'q')
		  quit();
		newlinePtr = strchr(inputBuffer,'\n');
		if (newlinePtr != NULL)
		  *newlinePtr = '\0';
		returnStatus = PGS_TD_UTCtoTAI(inputBuffer, &ca[count].etai);
		switch (returnStatus)
		{
		  case PGS_S_SUCCESS:
		  case PGSTD_E_NO_LEAP_SECS:
		    if (ca[count].etai < startTAI93 ||
			ca[count].etai < ca[count].btai)
		    {
			printf("\nInvalid period stop time (stop time is "
			       "BEFORE start time of this\nperiod OR this "
			       "data set).\n");
			break;
		    }
		    gotData = PGS_TRUE;
		    break;
		  default:
		    PGS_SMF_GetMsg(&code,mnemonic,msg);
		    printf("\n%s\n\n",msg);
		    continue;
		}
	    }
	    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	    if (gotData == PGS_FALSE)
	      quit();
	    
	    gotData = PGS_FALSE;
	    
	    
	    printf("\nenter starting Euler angles (%1d %1d %1d) or "
		   "'continue':\n-->",
		   eao[0], eao[1], eao[2]);
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	      quit();
	    if (strncmp(inputBuffer, "continue", 8) == 0 ||
		strncmp(inputBuffer, "CONTINUE", 8) == 0 ||
		strncmp(inputBuffer, "Continue", 8) == 0)
	    {
		ca[count].cont = PGS_TRUE;
	    }
	    else
	    {
		ca[count].cont = PGS_FALSE;
		sscanf(inputBuffer, "%lf %lf %lf", &ca[count].bea[0],
		       &ca[count].bea[1],&ca[count].bea[2]); 
	    }
	    
	    printf("\nenter ending Euler angles (%1d %1d %1d) or 'lock':\n-->",
		   eao[0], eao[1], eao[2]);
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	      quit();
	    if (strncmp(inputBuffer, "lock", 4) == 0 ||
		strncmp(inputBuffer, "LOCK", 4) == 0 ||
		strncmp(inputBuffer, "Lock", 4) == 0 ||
		strncmp(inputBuffer, "inertial", 8) == 0 ||
		strncmp(inputBuffer, "INERTIAL", 8) == 0 ||
		strncmp(inputBuffer, "Inertial", 8) == 0)
	    {
		ca[count].lock = PGS_TRUE;
		ca[count].eea[0] = 0.0;
		ca[count].eea[1] = 0.0;
		ca[count].eea[2] = 0.0;
		
	    }
	    else
	    {
		ca[count].lock = PGS_FALSE;
		sscanf(inputBuffer, "%lf %lf %lf", &ca[count].eea[0],
		       &ca[count].eea[1],&ca[count].eea[2]);
	    }

	    count++;
	    
	    if (ca[0].etai >= startTAI93)
	    {
		memcpy((void*)&tmp_ca, (void*)&ca[0], sizeof(PGSt_customAttit));
		old_eea[0] = tmp_ca.eea[0];
		old_eea[1] = tmp_ca.eea[1];
		old_eea[2] = tmp_ca.eea[2];
		if (tmp_ca.cont == PGS_TRUE)
		{
		    tmp_ca.bea[0] = 0.0;
		    tmp_ca.bea[1] = 0.0;
		    tmp_ca.bea[2] = 0.0;
		}
		periodNum = 0;
	    }

	    printf("\nDefine another period? (y/[n]):\n-->");
	    fgets(inputBuffer,99,stdin);
	    switch (inputBuffer[0])
	    {
	      case 'Y':
	      case 'y':
		continue;

	      case 'q':
	      case 'Q':
		quit();
		
	      default:
		break;
	    }
	    break;
	} while (count < 100);
    }
    
    num_periods = count;
    
    printf("\n");

    /* convert the start and stop time input by the user to Julian days for
       looping purposes */
   
    numCheck = sscanf(startUTC,"%d-%d-%d",&year,&month,&day);
    startJD = PGS_TD_julday(year,month,day);
    numCheck = sscanf(stopUTC,"%d-%d-%d",&year,&month,&day);
    stopJD = PGS_TD_julday(year,month,day);
    if (noon == PGS_TRUE || time_check== PGS_TRUE)
	stopJD -= 1;

    /* initialize some constant values in the s/c ephem/attit structures */

    strcpy(ephemFileHeader.spacecraftID, scName);
    strcpy(ephemFileHeader.source, "simulation");
    strcpy(ephemFileHeader.version, "1");
    strcpy(ephemFileHeader.keplerRefFrame, "J2000");
    ephemFileHeader.nURs = 0;
    
    strcpy(attitFileHeader.spacecraftID, scName);
    strcpy(attitFileHeader.source, "simulation");
    strcpy(attitFileHeader.version, "1");
    attitFileHeader.nURs = 0;
    attitFileHeader.eulerAngleOrder[0] = (PGSt_uinteger) eao[0];
    attitFileHeader.eulerAngleOrder[1] = (PGSt_uinteger) eao[1];
    attitFileHeader.eulerAngleOrder[2] = (PGSt_uinteger) eao[2];

    ephemRecord.qualityFlag = 0;
    attitRecord.qualityFlag = 0;

    if ( ephemFileHeader.nURs == 0)
       nURs = 1;
    else
       nURs = ephemFileHeader.nURs;

    /* calculate number of files */
    numFiles = 24/timeInt;

    /* loop from start day to stop day (inclusive), writing out an entire day's
       worth of data for each day */
    if(time_check== PGS_TRUE && stopJD == (startJD -1)) stopJD=startJD;
    for(julianDay = startJD;julianDay<=stopJD;julianDay++)
    {
	/* create the CCSDS ASCII version of the day corresponding to the
	   current Julian day and the next Julian day */

	PGS_TD_calday(julianDay,&start_year,&start_month,&start_day);
	sprintf(startUTC,"%04d-%02d-%02d", start_year, start_month, start_day);
        sprintf(startUTC_tmp,"%04d-%02d-%02d", start_year, start_month, start_day);
        PGS_TD_calday(julianDay,&tmp_year,&tmp_month,&tmp_day);

        /* append time */
        if (noon == PGS_TRUE)
        {
            strcat(startUTC_tmp,"T12:00:00");
            strcpy(stopUTC_tmp,startUTC_tmp);
        }
        
        /* initialize some values */
        start_check = PGS_FALSE;
	stop_check = PGS_FALSE;
	hour = 0;
	start_hour = 0;
	stop_hour = 0;

	/* temporary begin UTC time */
        numCheck = sscanf(startUTC_tmp,"%04d-%02d-%02dT%02d",&start_year,&start_month,&start_day,&hour);

	/* loop for time interval */
        for (numCount=0; numCount<numFiles; numCount++)
        {
	   if (hdf_file == PGS_TRUE)
	   {
	       /* Create a file name of the form: <spacecraft>_<date>.eph.hdf.  Test to see
	       	  it this file already exists.  If it does do not overwrite the old data. */
  	       if (timeInterval == PGS_TRUE)
     	          sprintf(ephHdfFilename,"%s/%s_%s_%02dh_%02d.eph.hdf", ephemDirectory, scName, startUTC, timeInt, numCount+1);
  	       else
     	          sprintf(ephHdfFilename,"%s/%s_%s.eph.hdf", ephemDirectory, scName, startUTC);
  	       if (file_exists(ephHdfFilename))
  	       {
     		    printf("file: %s already exists ...skipping\n",ephHdfFilename);
     		    continue;
  	       }
               /* Create a file name of the form: <spacecraft>_<date>.att.hdf.  Test to see
                  it this file already exists.  If it does do not overwrite the old data. */
  	       if (timeInterval == PGS_TRUE)
     	           sprintf(attHdfFilename,"%s/%s_%s_%02dh_%02d.att.hdf", ephemDirectory, scName, startUTC, timeInt, numCount+1);
  	       else
     	           sprintf(attHdfFilename,"%s/%s_%s.att.hdf", ephemDirectory, scName, startUTC);
  	       if (file_exists(attHdfFilename))
  	       {
     		    printf("file: %s already exists ...skipping\n",attHdfFilename);
     		    continue;
               }
	   }

	   /* Create a file name of the form: <spacecraft>_<date>.eph.  Test to see
	   it this file already exists.  If it does do not overwrite the old
	   data. */

           if (timeInterval == PGS_TRUE)
	      sprintf(ephFilename,"%s/%s_%s_%02dh_%02d.eph", ephemDirectory, scName, startUTC, timeInt, numCount+1);
           else
              sprintf(ephFilename,"%s/%s_%s.eph", ephemDirectory, scName, startUTC);
	   if (file_exists(ephFilename))
	   {
	       printf("file: %s already exists ...skipping\n",ephFilename);
	       continue;
	   }

	   /* Create a file name of the form: <spacecraft>_<date>.att.  Test to see
	   it this file already exists.  If it does do not overwrite the old
	   data. */

           if (timeInterval == PGS_TRUE)
	      sprintf(attFilename,"%s/%s_%s_%02dh_%02d.att", ephemDirectory, scName, startUTC, timeInt, numCount+1);
           else
              sprintf(attFilename,"%s/%s_%s.att", ephemDirectory, scName, startUTC);
	   if (file_exists(attFilename))
	   {
	      printf("file: %s already exists ...skipping\n",attFilename);
	      continue;
	   }

	/* Attempt to create the file for writing.  If there is a problem
	   opening the file (probably no write permission in specified
	   directory) issue a message and wait for a response.  This gives the
	   user a chance to identify the problem and fix it.  The user may quit
	   at this point or continue.  If the user wishes to continue attempt to
	   open the file once again.  If it again fails skip it without issuing
	   another warning. */

	   if (hdf_file == PGS_TRUE)
	   {
	      /* Open the HDF format ephmeris file */
   	      hdfId = Hopen( ephHdfFilename, DFACC_CREATE, 0);
	      /* Open the HDF format ephmeris dataset for data access */
   	      hdfSdId = SDstart( ephHdfFilename, DFACC_RDWR );
	      /* Initialize the Vset interface */
   	      Vstart( hdfId );
	      /* Fetch the header Vdata reference number */
	      vDataRef = -1;
	      vDataRef = VSgetid( hdfId, vDataRef );
	      /* Create the header Vdata */
	      vDataId = VSattach( hdfId, vDataRef, "w" );
	      /* Set the name of the header Vdata */
	      VSsetname( vDataId, headerVdataName );
	      VSsetclass( vDataId, headerVdataName );
	      /* Define the fields in the header Vdata */
	      hdfStatus = VSfdefine( vDataId, DpDPrScIdField, DFNT_CHAR8,
                                       DpCPrMaxScIdLen );

              if ( hdfStatus == 0 )
              {
                    hdfStatus = VSfdefine( vDataId, DpDPrAsciiTimeField,
                                           DFNT_CHAR8, DpCPrMaxTimeRangeLen );
                    hdfStatus = VSfdefine( vDataId, DpDPrSourceField,
                                           DFNT_CHAR8, DpCPrMaxSourceLen );
                    hdfStatus = VSfdefine( vDataId, DpDPrVersionField,
                                           DFNT_CHAR8, DpCPrMaxVersionLen );
                    hdfStatus = VSfdefine( vDataId, DpDPrStartTimeField,
                                           DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrEndTimeField,
                                           DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrIntervalField,
                                           DFNT_FLOAT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrNURsField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrNRecordsField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrNOrbitsField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrOrbitStartField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrOrbitEndField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrKeplerFrameField,
                                           DFNT_CHAR8, DpCPrMaxFrameLen );
                    hdfStatus = VSfdefine( vDataId, DpDPrKeplerElementsField,
                                           DFNT_FLOAT64, DpCPrMaxKepler );
                    hdfStatus = VSfdefine( vDataId, DpDPrKeplerEpochField,
                                           DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrQaParamsField,
                                           DFNT_FLOAT32, DpCPrMaxQaParameters );
                    hdfStatus = VSfdefine( vDataId, DpDPrQaStatsField,
                                           DFNT_FLOAT32, DpCPrMaxQaStatistics );
                    hdfStatus = VSfdefine( vDataId, DpDPrOrbPeriodField,
                                           DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrDescPropField,
                                          DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrFddReplaceField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataId, DpDPrParentUrsField,
                                           DFNT_CHAR8,
                                           nURs*DpCPrMaxUrLen );
                    hdfStatus = VSsetfields( vDataId,
                                             DpDPrEphemerisHeaderFields );
              }
              if ( hdfStatus != 0 )
              {
                    printf("ERROR to set fileds for Ephemeris HDF files\n");
              }
	      /* Open the HDF format attitude file */
   	      hdfIdAtt = Hopen( attHdfFilename, DFACC_CREATE, 0);
	      /* Open the HDF format attitude dataset for data access */
   	      hdfSdIdAtt = SDstart( attHdfFilename, DFACC_RDWR );
	      /* Initialize the Vset interface */
   	      Vstart( hdfIdAtt );
	      /* Fetch the header Vdata reference number */
	      vDataRefAtt = -1;
	      vDataRefAtt = VSgetid( hdfIdAtt, vDataRefAtt );
	      /* Create the header Vdata */
	      vDataIdAtt = VSattach( hdfIdAtt, vDataRefAtt, "w" );
	      /* Set the name of the header Vdata */
	      VSsetname( vDataIdAtt, headerVdataNameAtt );
	      VSsetclass( vDataIdAtt, headerVdataNameAtt );
	      /* Define the fields in the header Vdata */
	      hdfStatus = VSfdefine( vDataIdAtt, DpDPrScIdField, DFNT_CHAR8,
                                       DpCPrMaxScIdLen );
              if ( hdfStatus == 0 )
              {
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrAsciiTimeField,
                                           DFNT_CHAR8, DpCPrMaxTimeRangeLen );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrSourceField,
                                           DFNT_CHAR8, DpCPrMaxSourceLen );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrVersionField,
                                           DFNT_CHAR8, DpCPrMaxVersionLen );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrStartTimeField,
                                           DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrEndTimeField,
                                           DFNT_FLOAT64, 1 );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrIntervalField,
                                           DFNT_FLOAT32, 1 );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrNURsField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrNRecordsField,
                                           DFNT_INT32, 1 );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrEulerAngleOrderField,
                                           DFNT_INT32, 3 );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrQaParamsField,
                                           DFNT_FLOAT32, DpCPrMaxQaParameters );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrQaStatsField,
                                           DFNT_FLOAT32, DpCPrMaxQaStatistics );
                    hdfStatus = VSfdefine( vDataIdAtt, DpDPrParentUrsField,
                                           DFNT_CHAR8,
                                           nURs*DpCPrMaxUrLen );
                    hdfStatus = VSsetfields( vDataIdAtt,
                                             DpDPrAttitudeHeaderFields );
                }
                if ( hdfStatus != 0 )
                {
                    printf("ERROR to set fileds for Attitude HDF files\n");
                }
	    }

	ephemFilePtr = fopen(ephFilename,"w");
	if (ephemFilePtr == NULL)
	{
	    printf("\nunable to create file: %s\n",ephFilename);
	    printf("make sure that you have write permission in directory: %s\n"
		   ,ephemDirectory);
	    printf("continue (y/n)?\n-->");
	    fgets(inputBuffer,99,stdin);
	    printf("\n");
	    switch (inputBuffer[0])
	    {
	      case 'n':
	      case 'N':
	      case 'q':
		quit();
	    }
	    ephemFilePtr = fopen(ephFilename,"w");
	    if (ephemFilePtr == NULL)
	      continue;
	}
	printf("creating file: %s\n",ephFilename);

	attitFilePtr = fopen(attFilename,"w");
	if (attitFilePtr == NULL)
	{
	    printf("\nunable to create file: %s\n",attFilename);
	    printf("make sure that you have write permission in directory: %s\n"
		   ,ephemDirectory);
	    printf("continue (y/n)?\n-->");
	    fgets(inputBuffer,99,stdin);
	    printf("\n");
	    switch (inputBuffer[0])
	    {
	      case 'n':
	      case 'N':
	      case 'q':
		quit();
	    }
	    attitFilePtr = fopen(attFilename,"w");
	    if (attitFilePtr == NULL)
	    {
		fclose(ephemFilePtr);
		continue;
	    }
	}
	printf("creating file: %s\n",attFilename);
	      
        /* calculate first record time and last record time */
	start_hour = hour + numCount*timeInt;
	stop_hour = start_hour + timeInt;

	/* 1 if year is leap year, otherwise 0 */
	leapYear = (year%4) ? 0 : 1;
	leapYear = (year%100) ? leapYear : 0;
	leapYear = (year%400) ? leapYear : 1;

	/* calculate stop time */
	if (stop_hour >= 24 && stop_check == PGS_FALSE)
	{
	   stop_check = PGS_TRUE;
	   stop_hour = stop_hour - 24;
  	   switch (tmp_month)
  	   {
    	       case 1:
               case 3:
               case 5:
               case 7:
               case 8:
               case 10:
      		  if (tmp_day == 31)
      		  {
        	      tmp_month = tmp_month+1;
        	      tmp_day = 1;
      		  }
      		  else
        	      tmp_day = tmp_day+1;
      		  break;
   	       case 12:
      		  if (tmp_day == 31)
      		  {
        	      tmp_year = tmp_year+1;
        	      tmp_month = 1;
        	      tmp_day = 1;
      		  }
      		  else
        	      tmp_day = tmp_day+1;
      		  break;
   	       case 4:
               case 6:
               case 9:
   	       case 11:
      		  if (tmp_day == 30)
      		  {
        	      tmp_month = tmp_month+1;
                      tmp_day = 1;
      		  }
      		  else
        	      tmp_day = tmp_day+1;
      		  break;
   	       case 2:
      		  if (tmp_day == (28 + leapYear))
      		  {
        	      tmp_month = tmp_month+1;
        	      tmp_day = 1;
      		  }
                  else
                      tmp_day = tmp_day+1;
                  break;
               default:
                  printf("**** ERROR **** \n\n");
                  quit();
           }
        }

        /* calculate start time */
	if (start_hour >=24 && start_check == PGS_FALSE )
	{
  	    start_check = PGS_TRUE;
  	    start_hour -= 24;
  	    switch (start_month)
  	    {
    	        case 1:
    		case 3:
    		case 5:
    		case 7:
    		case 8:
    		case 10:
      		    if (start_day == 31)
      		    {
		        start_month += 1;
        	        start_day = 1;
      		    }
      		    else
		        start_day += 1;
      		    break;
   		case 12:
      		    if (start_day == 31)
      		    { 
        		start_year += 1;
        		start_month = 1;
        		start_day = 1;
      		    } 
      		    else
        		start_day += 1;
      		    break;
   		case 4:
   		case 6:
   		case 9:
   		case 11:
      		    if (start_day == 30)
      		    { 
        		start_month += 1;
        		start_day = 1;
      		    } 
      		    else
        		start_day += 1;
      		    break;
   		case 2:
      		    if (start_day == (28 + leapYear))
      		    { 
        		start_month += 1;
        		start_day = 1;
      		    } 
      		    else
        	        start_day += 1;
      		    break;
   		default:
     		    printf("**** ERROR   **** \n\n");
                    quit();
   	    }
        }

        /* calculate time */
	if (start_hour >=24)
   	    start_hour -= 24;
	if (stop_hour >=24)
   	    stop_hour -= 24;

        /* calculate temporary time */
	sprintf(startUTC_tmp, "%04d-%02d-%02dT%02d:00:00", start_year, start_month, start_day,start_hour);
	sprintf(stopUTC_tmp, "%04d-%02d-%02dT%02d:00:00", tmp_year, tmp_month, tmp_day, stop_hour);

 
	PGS_TD_UTCtoTAI(startUTC_tmp,&startTAI93);
        PGS_TD_UTCtoTAI(stopUTC_tmp,&stopTAI93);

	ephemFileHeader.startTime = startTAI93;
	attitFileHeader.startTime = startTAI93;
	PGS_TD_TAItoUTC(startTAI93, startUTC_tmp);
	
	/* calculate number of records of ephemeris data */

	numIntervals = (stopTAI93 - startTAI93 - 1.e-6)/
                       defaultInterval;
	numIntervals -= fmod(numIntervals,1.0);
	
	ephemFileHeader.endTime = startTAI93 + defaultInterval*
	                                         (numIntervals + 1);
	PGS_TD_TAItoUTC(ephemFileHeader.endTime, stopUTC_tmp);
	sprintf(ephemFileHeader.asciiTimeRange,
		"%.22s - %.22s",startUTC_tmp, stopUTC_tmp);

	ephemFileHeader.nRecords = (PGSt_integer) numIntervals + 1;
	
	/* calculate number of records of attitude data */

	numIntervals = (stopTAI93 - startTAI93 - 1.e-6)/
                       defaultInterval;
	numIntervals = (int) numIntervals; /*-= fmod(numIntervals,1.0);*/
	attitFileHeader.endTime = startTAI93 + defaultInterval*
	                                         (numIntervals + 1);
	PGS_TD_TAItoUTC(attitFileHeader.endTime, stopUTC_tmp);
	sprintf(attitFileHeader.asciiTimeRange,
		"%.21sZ - %.21sZ",startUTC_tmp, stopUTC_tmp);

	attitFileHeader.nRecords = (PGSt_integer) numIntervals + 1;

	/* get the orbital elements (required for the ephemeris file header) */

	PGS_EPH_orbitalElements(startTAI93,spacecraftTag, aIn, eIn, incIn, anIn, apIn, maIn, data, &startOrbitNum, epoch);

	if (startOrbitNum < 0)
	{
	    /* the definition of orbit numbers in ephemeris file headers only
	       allows for positive orbit numbers, so make up some sillyness if
	       the orbit number comes back negative from the simulator call
	       above (which it will if the requested time is BEFORE launch) */

            startOrbitNum = START_BEFORE_LAUNCH;
	}

	ephemFileHeader.orbitNumberStart = startOrbitNum;
	ephemFileHeader.orbitNumberEnd = startOrbitNum-1;

/****
	nu = ma+2.0*e*sin(ma)+1.25*e*e*sin(2.0*ma);
	if ( nu < 0.0 )
	{
	    nu = 2.0*M_PI + nu;
	}
	
	if ( ap > 0.0 )
	{
	    nu0 = 2.0*M_PI - ap;
	}
	else
	{
	    nu0 = -ap;
	}
	
	if ( nu0 > nu )
	{
	    k = 1.0;
	}
	else
	{
	    k = 0.0;
	}
	dt1 = P/(2.0*M_PI)*(2.0*k*M_PI + 
			   (E(nu) - e*sin(E(nu))) - 
			   (E(nu0) - e*sin(E(nu0))));
****/
        dt1 = startTAI93 - PGS_EPH_OrbitAscendTime(spacecraftTag, startTAI93, aIn, eIn, incIn, anIn, apIn, maIn, epoch);
        dt2 = startTAI93 - PGS_EPH_OrbitDecendTime(spacecraftTag, startTAI93, aIn, eIn, incIn, anIn, apIn, maIn, epoch);
	time = startTAI93 - dt1;
	while ( time < stopTAI93 )
	{
	    ephemFileHeader.orbitNumberEnd += 1;
	    time += 2.0*(M_PI/data[8]);
	}

	ephemFileHeader.keplerElements[0] = data[0];
	ephemFileHeader.keplerElements[1] = data[1];
	ephemFileHeader.keplerElements[2] = data[2];
	ephemFileHeader.keplerElements[3] = data[3];
	ephemFileHeader.keplerElements[4] = data[4];
	ephemFileHeader.keplerElements[5] = data[5];

	/* just some bogus junk (i.e. filling out the number of orbits field in
	   the ephemeris header with bogus values, just so there are at LEAST
	   some values in these fields so that they can accessed in a test
	   situation) */

	ephemFileHeader.nOrbits = 1 + ephemFileHeader.orbitNumberEnd - 
	  ephemFileHeader.orbitNumberStart;
	
	/* write ephemeris file header */

	ephemFileHeader.interval = (PGSt_real) defaultInterval;
	numCheck = (int) fwrite(&ephemFileHeader,sizeof(PGSt_ephemHeader),1,
				ephemFilePtr);
	if (numCheck != 1)
	{
	    printf("error writing to file: %s ...aborting\n",ephFilename);
	    fclose(ephemFilePtr);
	    fclose(attitFilePtr);
	    break;
	}

	numCheck = (int) fwrite(parentUR, PGSd_UR_FIELD_SIZE,
				ephemFileHeader.nURs, ephemFilePtr);
	if (numCheck != ephemFileHeader.nURs)
	{
	    printf("error writing to file: %s ...aborting\n",ephFilename);
	    fclose(ephemFilePtr);
	    fclose(attitFilePtr);
	    break;
	}


	if (hdf_file == PGS_TRUE)
	{
	   /* Create a packed Vdata header record */
           if ((hdfHeader=(unsigned char *)malloc(sizeof(PGSt_ephemHeader)+nURs*DpCPrMaxUrLen*sizeof(char)+2*sizeof(float64)+sizeof(int32))) == NULL)
	       printf (" Allocation ERROR\n");
	   pntr = hdfHeader;

	   memcpy( pntr, ephemFileHeader.spacecraftID,DpCPrMaxScIdLen*sizeof(char) );
	   pntr += DpCPrMaxScIdLen*sizeof(char);

	   memcpy( pntr, ephemFileHeader.asciiTimeRange, DpCPrMaxTimeRangeLen*sizeof(char) );
	   pntr += DpCPrMaxTimeRangeLen*sizeof(char);

	   memcpy( pntr, ephemFileHeader.source, DpCPrMaxSourceLen*sizeof(char) );
	   pntr += DpCPrMaxSourceLen*sizeof(char);

	   memcpy( pntr, ephemFileHeader.version,DpCPrMaxVersionLen*sizeof(char) );
	   pntr += DpCPrMaxVersionLen*sizeof(char);

	   memcpy( pntr, &ephemFileHeader.startTime, sizeof(float64) );
	   pntr += sizeof(float64);

	   memcpy( pntr, &ephemFileHeader.endTime, sizeof(float64) );
	   pntr += sizeof(float64);

	   memcpy( pntr, &ephemFileHeader.interval, sizeof(float32) );
	   pntr += sizeof(float32);

	   memcpy( pntr, &ephemFileHeader.nURs, sizeof(int32) );
	   pntr += sizeof(int32);

	   memcpy( pntr, &ephemFileHeader.nRecords, sizeof(int32) );
	   pntr += sizeof(int32);

	   memcpy( pntr, &ephemFileHeader.nOrbits, sizeof(int32) );
	   pntr += sizeof(int32);

	   memcpy( pntr, &ephemFileHeader.orbitNumberStart, sizeof(int32) );
	   pntr += sizeof(int32);

	   memcpy( pntr, &ephemFileHeader.orbitNumberEnd, sizeof(int32) );
	   pntr += sizeof(int32);

	   memcpy( pntr, ephemFileHeader.keplerRefFrame, DpCPrMaxFrameLen*sizeof(char) );
	   pntr += DpCPrMaxFrameLen*sizeof(char);

	   memcpy( pntr, &ephemFileHeader.keplerElements[0], DpCPrMaxKepler*sizeof(float64) );
	   pntr += DpCPrMaxKepler*sizeof(float64);

	   memcpy( pntr, &ephemFileHeader.keplerEpochTAI, sizeof(float64) );
	   pntr += sizeof(float64);

	   memcpy( pntr, &ephemFileHeader.qaParameters[0], DpCPrMaxQaParameters*sizeof(float32) );
	   pntr += DpCPrMaxQaParameters*sizeof(float32);

	   memcpy( pntr, &ephemFileHeader.qaStatistics[0], DpCPrMaxQaStatistics*sizeof(float32) );
	   pntr += DpCPrMaxQaStatistics*sizeof(float32);

	   memcpy( pntr, &initF, sizeof(float64));
	   pntr += sizeof(float64);

	   memcpy( pntr, &initF, sizeof(float64) );
	   pntr += sizeof(float64);

	   memcpy( pntr, &initI, sizeof(int32) );
	   pntr += sizeof(int32);


  	   for (iElm=0; iElm<nURs; iElm++)
  	   {

    		memcpy( pntr, &nativeURs[iElm][0], DpCPrMaxUrLen );
    		pntr += DpCPrMaxUrLen*sizeof(char);
  	   }

	   /* Write the packed Vdata header record */
	   hdfStatus = VSwrite( vDataId, hdfHeader, 1, FULL_INTERLACE );
	   /* Detach from the Vdata */
	   VSdetach( vDataId );

	   /* To coordinate the time management operations for DPREP */

	   /* get the UTC start time of the datset */
	   hdfStatus1 = PGS_TD_TAItoUTC(ephemFileHeader.startTime, XasciiUTC);
	   if( hdfStatus1 != PGS_S_SUCCESS )
   		printf (" PGS_TD_TAItoUTC ERROR\n");
	   hdfStatus1 = PGS_TD_ASCIItime_AtoB(XasciiUTC, XstartUTC);
	   if( hdfStatus1 != PGS_S_SUCCESS )
   		printf (" PGS_TD_ASCIItime_AtoB ERROR\n");

	   /* check if the start hour is even or odd */
	   strncpy( Xhour, &XstartUTC[9], 2 );
	   Xhour[2] = '\0';
	   XstartHour = atoi( Xhour );
	   if ( (XstartHour % 2) != 0 )
		XstartHour--;
	   /* Fix the start time estimate to the nearset even hour */
	   strncpy( XyearAndDayNumber, XstartUTC, 9 );
	   XyearAndDayNumber[9] = '\0';
	   sprintf( XintervalStart, "%s%2.2i:00:00.000000Z", XyearAndDayNumber, XstartHour );
	   XintervalStart[25] = '\0';
	   hdfStatus1 = PGS_TD_UTCtoTAI(XintervalStart, &XstartLimit);
	   if( hdfStatus1 != PGS_S_SUCCESS )
   		printf (" DpPrGranuleTimes PGS_TD_UTCtoTAI ERROR\n");
	   /* Estimate the request start time */
	   XstartReqTime = ephemFileHeader.startTime;
	   while ( XstartReqTime >= XstartLimit )
		XstartReqTime = XstartReqTime - expectedDataInterval;
	   granuleStartTime = XstartReqTime + expectedDataInterval;
	   /* Estimate the request end time */
	   if ( ephemFileHeader.endTime  != 0 )
	   {
		XendLimit = XstartLimit + mySegmentDuration - myClockEps;
		XendReqTime = ephemFileHeader.endTime;
		while ( XendReqTime < XendLimit )
		   XendReqTime = XendReqTime + expectedDataInterval;
		granuleEndTime = XendReqTime - expectedDataInterval;
	   }

        }


	/* write attitude file header */

	attitFileHeader.interval = (PGSt_real) defaultInterval;
	numCheck = (int) fwrite(&attitFileHeader,sizeof(PGSt_attitHeader),1,
				attitFilePtr);
	if (numCheck != 1)
	{
	    printf("error writing to file: %s ...aborting\n",attFilename);
	    fclose(ephemFilePtr);
	    fclose(attitFilePtr);
	    break;
	}

	numCheck = (int) fwrite(parentUR, PGSd_UR_FIELD_SIZE,
				attitFileHeader.nURs, attitFilePtr);
	if (numCheck != attitFileHeader.nURs)
	{
	    printf("error writing to file: %s ...aborting\n",attFilename);
	    fclose(ephemFilePtr);
	    fclose(attitFilePtr);
	    break;
	}


	if (hdf_file == PGS_TRUE)
	{
	   /* Create a packed Vdata header record */
	   hdfHeaderAtt =(unsigned char *)malloc(sizeof(PGSt_attitHeader)+nURs*DpCPrMaxUrLen*sizeof(char));
	   pntrAtt = hdfHeaderAtt;

	   memcpy( pntrAtt, attitFileHeader.spacecraftID,DpCPrMaxScIdLen*sizeof(char) );
	   pntrAtt += DpCPrMaxScIdLen*sizeof(char);
	   memcpy( pntrAtt, attitFileHeader.asciiTimeRange, DpCPrMaxTimeRangeLen*sizeof(char) );
	   pntrAtt += DpCPrMaxTimeRangeLen*sizeof(char);
	   memcpy( pntrAtt, attitFileHeader.source, DpCPrMaxSourceLen*sizeof(char) );
	   pntrAtt += DpCPrMaxSourceLen*sizeof(char);
	   memcpy( pntrAtt, attitFileHeader.version,DpCPrMaxVersionLen*sizeof(char) );
	   pntrAtt += DpCPrMaxVersionLen*sizeof(char);
	   memcpy( pntrAtt, &attitFileHeader.startTime, sizeof(float64) );
	   pntrAtt += sizeof(float64);
	   memcpy( pntrAtt, &attitFileHeader.endTime, sizeof(float64) );
	   pntrAtt += sizeof(float64);
	   memcpy( pntrAtt, &attitFileHeader.interval, sizeof(float32) );
	   pntrAtt += sizeof(float32);
	   memcpy( pntrAtt, &attitFileHeader.nURs, sizeof(int32) );
	   pntrAtt += sizeof(int32);
	   memcpy( pntrAtt, &attitFileHeader.nRecords, sizeof(int32) );
	   pntrAtt += sizeof(int32);
	   memcpy( pntrAtt, &attitFileHeader.eulerAngleOrder[0], DpCPrMaxEulerAngleOrder*sizeof(int32) );
	   pntrAtt += DpCPrMaxEulerAngleOrder*sizeof(int32);
	   memcpy( pntrAtt, &attitFileHeader.qaParameters[0], DpCPrMaxQaParameters*sizeof(float32) );
	   pntrAtt += DpCPrMaxQaParameters*sizeof(float32);
	   memcpy( pntrAtt, &attitFileHeader.qaStatistics[0], DpCPrMaxQaStatistics*sizeof(float32) );
	   pntrAtt += DpCPrMaxQaStatistics*sizeof(float32);

  	   for (iElm=0; iElm<nURs; iElm++)
  	   {
    		memcpy( pntrAtt, &nativeURs[iElm][0], DpCPrMaxUrLen );
    		pntrAtt += DpCPrMaxUrLen*sizeof(char);
  	   }

	   /* Write the packed Vdata header record */
	   hdfStatus = VSwrite( vDataIdAtt, hdfHeaderAtt, 1, FULL_INTERLACE );
	   /* Detach from the Vdata */
	   VSdetach( vDataIdAtt );
        }
			  
	/* write ephemeris data */

	ephemRecord.secTAI93=ephemFileHeader.startTime;
	attitRecord.secTAI93=attitFileHeader.startTime;

	if (hdf_file == PGS_TRUE)
	{
	   /* Initialize the Vdata for an Ephemeris Record */

	   /* Create the first Vdata without data */
	   vDataId = VSattach( hdfId, -1, "w" );
	   if ( vDataId == FAIL )
   		printf("Fail EPH\n");
	   /* Set the name of the Vdata */
	   strcpy( ephemerisVdataName, DpCPrEphemerisVdataName );
	   VSsetname( vDataId, ephemerisVdataName );
	   VSsetclass( vDataId, ephemerisVdataName );
	   /* Define the fields in the Vdata */
	   hdfStatus = VSfdefine( vDataId, DpDPrTimeField, DFNT_FLOAT64,1 );
	   if ( hdfStatus == 0 )
	   {
		hdfStatus = VSfdefine( vDataId, DpDPrXPosField,
                                           DFNT_FLOAT64, 1 );
		hdfStatus = VSfdefine( vDataId, DpDPrYPosField,
                                           DFNT_FLOAT64, 1 );
		hdfStatus = VSfdefine( vDataId, DpDPrZPosField,
                                           DFNT_FLOAT64, 1 );
		hdfStatus = VSfdefine( vDataId, DpDPrXVelField,
                                           DFNT_FLOAT64, 1 );
		hdfStatus = VSfdefine( vDataId, DpDPrYVelField,
                                           DFNT_FLOAT64, 1 );
		hdfStatus = VSfdefine( vDataId, DpDPrZVelField,
                                           DFNT_FLOAT64, 1 );
		hdfStatus = VSfdefine( vDataId, DpDPrQualityFlagField,
                                           DFNT_INT32, 1 );
		hdfStatus = VSsetfields( vDataId,
                                             DpDPrEphemerisDataFields );
	   }
	   if ( hdfStatus != 0 )
	   {
		printf("******** ERROR   **************\n");
	   }

	   /* Create the first Vdata without data */
	   vDataIdAtt = VSattach( hdfIdAtt, -1, "w" );
	   if ( vDataIdAtt == FAIL )
   		printf("Fail ATT\n");
	   /* Set the name of the Vdata */
	   strcpy( attitudeVdataName, DpCPrAttitudeVdataName );
	   VSsetname( vDataIdAtt, attitudeVdataName );
	   VSsetclass( vDataIdAtt, attitudeVdataName );
	   /* Define the fields in the Vdata */
	   hdfStatus = VSfdefine( vDataIdAtt, DpDPrTimeField, DFNT_FLOAT64,1 );
	   if ( hdfStatus != 0 )
		printf("******** ERROR   **************\n");

	   if ( hdfStatus == 0 )
	   {
	        hdfStatus = VSfdefine( vDataIdAtt, DpDPrFirstEulerAngleField,
                                           DFNT_FLOAT64, 1 );

		hdfStatus = VSfdefine( vDataIdAtt, DpDPrSecondEulerAngleField,
                                           DFNT_FLOAT64, 1 );

		hdfStatus = VSfdefine( vDataIdAtt, DpDPrThirdEulerAngleField,
                                           DFNT_FLOAT64, 1 );

		hdfStatus = VSfdefine( vDataIdAtt, DpDPrXRateField,
                                           DFNT_FLOAT64, 1 );

		hdfStatus = VSfdefine( vDataIdAtt, DpDPrYRateField,
                                           DFNT_FLOAT64, 1 );

		hdfStatus = VSfdefine( vDataIdAtt, DpDPrZRateField,
                                           DFNT_FLOAT64, 1 );

		hdfStatus = VSfdefine( vDataIdAtt, DpDPrAttQualityFlagField,
                                           DFNT_INT32, 1 );

		hdfStatus = VSsetfields( vDataIdAtt,DpDPrAttitudeDataFields );

	   }
	   if ( hdfStatus != 0 )
	   {
		printf("******** ERROR Att  **************\n");
	   }
	}

	for (count=1;ephemRecord.secTAI93<stopTAI93;count++)
	{
	    /* call spacecraft attitude and orbit simulator */

	    returnStatus = PGS_EPH_attOrbSim(aIn, eIn, incIn, anIn, apIn, maIn,
                                             ephemRecord.secTAI93,
					     spacecraftTag,
					     attitudeNoise,
					     ephemRecord.position,
					     ephemRecord.velocity,
					     attitRecord.eulerAngle,
					     attitRecord.angularVelocity,
					     transform, epoch);

	    /************************
	     * BEGIN CUSTOM SECTION *
	     ************************/

	    if (custom == PGS_TRUE)
	    {
		if (attitRecord.secTAI93 > tmp_ca.etai)
		{
		    if (periodNum < num_periods-1)
		    {
			if (ca[periodNum+1].btai > attitRecord.secTAI93)
			{
			    if (tmp_ca.lock == PGS_TRUE)
			    {
				memcpy((void*)tmp_ca.bea, (void*)old_eea,
				       sizeof(old_eea));
				memcpy((void*)tmp_ca.eea, (void*)old_eea,
				       sizeof(old_eea));
			    }
			    else
			    {
				memcpy((void*)tmp_ca.bea, (void*)tmp_ca.eea,
				       sizeof(old_eea));
			    }
			    
			    tmp_ca.btai = tmp_ca.etai;
			    tmp_ca.etai = ca[periodNum+1].btai - 1.0E-6;
			    tmp_ca.lock = PGS_FALSE;
			    tmp_ca.cont = PGS_FALSE;
			}
			else
			{
			    periodNum++;
			    memcpy((void*)old_eea, (void*)tmp_ca.eea,
				   sizeof(old_eea));
			    memcpy((void*)&tmp_ca, (void*)&ca[periodNum], 
				   sizeof(PGSt_customAttit));
			    if (tmp_ca.cont == PGS_TRUE)
			    {
				memcpy((void*)tmp_ca.bea, (void*)old_eea,
				       sizeof(old_eea));
				tmp_ca.btai = old_tai;
			    }
			}
		    }
		    else
		    {
			tmp_ca.bea[0] = tmp_ca.eea[0];
			tmp_ca.bea[1] = tmp_ca.eea[1];
			tmp_ca.bea[2] = tmp_ca.eea[2];
			tmp_ca.btai = tmp_ca.etai;
			tmp_ca.etai = stopTAI93;
			tmp_ca.cont = PGS_FALSE;
			tmp_ca.lock = PGS_FALSE;
			custom = PGS_FALSE;
		    }
		    if (tmp_ca.lock == PGS_TRUE)
		    {
			
			PGS_CSC_EulerToQuat(tmp_ca.bea,
					    eao,quatEuler);
			PGS_CSC_getORBtoECIquat(ephemRecord.position,
						ephemRecord.velocity,
						quatORBtoECI);
			PGS_CSC_quatMultiply(quatORBtoECI, quatEuler,
					     quatSCtoECI);
			quatECItoSC_0[0] = quatSCtoECI[0];
			quatECItoSC_0[1] = -quatSCtoECI[1];
			quatECItoSC_0[2] = -quatSCtoECI[2];
			quatECItoSC_0[3] = -quatSCtoECI[3];
		    }
		    
		}
		if (tmp_ca.lock != PGS_TRUE)
		{
		    PGS_EPH_interpolateAttitude(tmp_ca.btai, tmp_ca.bea,
						attitRecord.angularVelocity,
						tmp_ca.etai, tmp_ca.eea,
						attitRecord.angularVelocity,
						eao, ephemRecord.secTAI93,
						eulerAngle, angularVelocity);
		    old_tai = ephemRecord.secTAI93;
		    old_eea[0] = eulerAngle[0];
		    old_eea[1] = eulerAngle[1];
		    old_eea[2] = eulerAngle[2];
		}
		else
		{
		    PGS_CSC_getORBtoECIquat(ephemRecord.position,
					    ephemRecord.velocity,
					    quatORBtoECI);
		    PGS_CSC_quatMultiply(quatECItoSC_0, quatORBtoECI,
					 quatORBtoSC_0);
		    quatEuler[0] = quatORBtoSC_0[0];
		    quatEuler[1] = -quatORBtoSC_0[1];
		    quatEuler[2] = -quatORBtoSC_0[2];
		    quatEuler[3] = -quatORBtoSC_0[3];
		    PGS_CSC_QuatToEuler(quatEuler, eao, eulerAngle);

		    old_tai = ephemRecord.secTAI93;
		    old_eea[0] = eulerAngle[0];
		    old_eea[1] = eulerAngle[1];
		    old_eea[2] = eulerAngle[2];
		    if (spacecraftTag == PGSd_TRMM)
		    {
			tipEuler(ephemRecord.position, ephemRecord.velocity,
				 eao, old_eea, PGSd_TO_TIP);
		    }
		}
		attitRecord.eulerAngle[0] += eulerAngle[0];
		attitRecord.eulerAngle[1] += eulerAngle[1];
		attitRecord.eulerAngle[2] += eulerAngle[2];
	    }

	    /**********************
	     * END CUSTOM SECTION *
	     **********************/

	    /* If the spacecraft is TRMM assume simulator ephemeris output is
	       TOD and simulator attitude output is geodetic referenced.  These
	       must be transformed to J2000 and geocentric referenced,
	       respectively. */

	    if (spacecraftTag == PGSd_TRMM)
	    {
	    
		if (custom == PGS_FALSE || tmp_ca.lock != PGS_TRUE)
		{
		    tipEuler(ephemRecord.position, ephemRecord.velocity, eao,
			     attitRecord.eulerAngle, PGSd_TO_ORB);
		}
		
	        /* Convert the True of Date position and velocity to the J2000
		   reference frame (this function--PGS_EPH_EphemAttit()--returns
		   ephemeris data in J2000 coordinates).  The "6" in the
		   function call below indicates that ephemRecord.position is a
		   6-vector.  This is a questionable trick, taking advantage of
		   the fact that ephemRecord.velocity is a three vector in the
		   structure ephemRecord immediately following the three vector
		   position (this is not an ideal programming technique, but it
		   should work). */

		PGS_CSC_TODtoJ2000(6,ephemRecord.secTAI93,
				   ephemRecord.position,
				   ephemRecord.position);
	    }
		
	    returnStatus = PGS_EPH_TransformBodyRates(
		                              PGSe_ORBtoECI,
					      ephemRecord.position,
					      ephemRecord.velocity,
					      attitRecord.eulerAngle,
					      (PGSt_integer*)attitFileHeader.eulerAngleOrder,
					      attitRecord.angularVelocity,
					      attitRecord.angularVelocity);

	    /* write ephemeris data to file, abort if any error occurs and print
	       warning message */

	    numCheck = (int) fwrite(&ephemRecord,sizeof(PGSt_ephemRecord),1,
				    ephemFilePtr);
	    if (numCheck != 1)
	    {
		printf("error writing to file: %s ...aborting\n",ephFilename);
		break;
	    }


  	    /* Write the att data records. */

	   if (hdf_file == PGS_TRUE)
	   {
	      /* Write the attitude data records */
              hdfRecordAtt =(uchar8 *)malloc(sizeof(PGSt_attitRecord));
	      pntrAtt = hdfRecordAtt;

	      memcpy( pntrAtt, &attitRecord.secTAI93,    sizeof(float64) );
	      pntrAtt += sizeof(float64);
	      memcpy( pntrAtt, &attitRecord.eulerAngle[0], sizeof(float64) );
	      pntrAtt += sizeof(float64);
	      memcpy( pntrAtt, &attitRecord.eulerAngle[1], sizeof(float64) );
              pntrAtt += sizeof(float64);
              memcpy( pntrAtt, &attitRecord.eulerAngle[2], sizeof(float64) );
              pntrAtt += sizeof(float64);
              memcpy( pntrAtt, &attitRecord.angularVelocity[0], sizeof(float64) );
              pntrAtt += sizeof(float64);
              memcpy( pntrAtt, &attitRecord.angularVelocity[1], sizeof(float64) );
              pntrAtt += sizeof(float64);
              memcpy( pntrAtt, &attitRecord.angularVelocity[2], sizeof(float64) );
              pntrAtt += sizeof(float64);
              memcpy( pntrAtt, &attitRecord.qualityFlag, sizeof(int32) );
	      /* Write the packed Vdata record */
              hdfStatus = VSwrite( vDataIdAtt, hdfRecordAtt, 1, FULL_INTERLACE );

              /* Write the ephemeris data records. */

              qaLongGapCount         = 0;
              qaOutofBoundsDataCount = 0;

	      hdfRecord =(uchar8 *)malloc(sizeof(PGSt_ephemRecord));
              pntr = hdfRecord;

              memcpy( pntr, &ephemRecord.secTAI93,    sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.position[0], sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.position[1], sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.position[2], sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.velocity[0], sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.velocity[1], sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.velocity[2], sizeof(float64) );
              pntr += sizeof(float64);
              memcpy( pntr, &ephemRecord.qualityFlag, sizeof(int32) );
	      /* Write the packed Vdata record */
              hdfStatus = VSwrite( vDataId, hdfRecord, 1, FULL_INTERLACE );
    	      /* Count long data gaps and out-of-bound data */
	      qaCheck = ephemRecord.qualityFlag & DpCPrPreLongGapFlag;

              if ( qaCheck != DpCPrGoodQualityFlag )
              {
            	nRecordsBefore = (int)(( ephemRecord.secTAI93 - granuleStartTime ) /
                                 ( expectedDataInterval + myClockEps ));
            	nRecordsAfter  = (int)(( granuleEndTime - ephemRecord.secTAI93 ) /
                                 ( expectedDataInterval + myClockEps ));

            	if ( nRecordsBefore != 0 && nRecordsAfter != 0 )
            	{
                   qaLongGapCount++;
            	}
              }

              qaCheck = ephemRecord.qualityFlag & DpCPrPostLongGapFlag;

              if ( qaCheck != DpCPrGoodQualityFlag )
              {
            	nRecordsBefore = (int)(( ephemRecord.secTAI93 - granuleStartTime ) /
                                 ( expectedDataInterval + myClockEps ));
             	nRecordsAfter  = (int)(( granuleEndTime - ephemRecord.secTAI93 ) /
                                 ( expectedDataInterval + myClockEps ));

            	if ( nRecordsBefore != 0 && nRecordsAfter != 0 )
            	{
                   qaLongGapCount++;
            	}
              }

              qaCheck = ephemRecord.qualityFlag & DpCPrBoundsCheck;

              if ( qaCheck != DpCPrGoodQualityFlag )
              {
            	qaOutofBoundsDataCount++;
              }

            }
	    
	    /* write attitude data to file, abort if any error occurs and print
	       warning message */

	    numCheck = (int) fwrite(&attitRecord,sizeof(PGSt_attitRecord),1,
				    attitFilePtr);
	    if (numCheck != 1)
	    {
		printf("error writing to file: %s ...aborting\n",attFilename);
		break;
	    }
	    
	    /* increment the time
	       NOTE: the time is calculated as (startTime)+(record #)*(interval)
	             because calculating it as (previous record time)+(interval)
		     causes errors, presumably due to machine precision
		     limitations */

	    ephemRecord.secTAI93 = startTAI93 + (defaultInterval*count);
	    attitRecord.secTAI93 = startTAI93 + (defaultInterval*count);
	}


	if (hdf_file == PGS_TRUE)
	{
	   /* Detach from the Vdata */
	   VSdetach( vDataId );
           VSdetach( vDataIdAtt);
	   /* Create the matadata Vdata without data */
           vDataId = VSattach( hdfId, -1, "w" );
           if ( vDataId == FAIL )
	      printf(" FAIL \n");
	   /* set the name of the Vdata */
           strcpy( metadataVdataName, DpCPrMetadataVdataName );
           VSsetname( vDataId, metadataVdataName );
           VSsetclass( vDataId, metadataVdataName );
	   /* Define the fields in the Vdata */
           hdfStatus = VSfdefine( vDataId, DpDPrOrbitNumberField, DFNT_INT32,  1 );

           if ( hdfStatus == 0 )
           {
              hdfStatus = VSfdefine( vDataId, DpDPrAscendTimeField,
                                           DFNT_FLOAT64, 1 );
              hdfStatus = VSfdefine( vDataId, DpDPrDescendTimeField,
                                           DFNT_FLOAT64, 1 );
              hdfStatus = VSfdefine( vDataId, DpDPrDescendLongitudeField,
                                           DFNT_FLOAT64, 1 );
              hdfStatus = VSsetfields( vDataId, DpDPrEphemerisOrbitFields );
           }
           if ( hdfStatus != 0 )
	   {
		printf("******** ERROR   **************\n");
	   }
	}

       
	for (count=0;count<ephemFileHeader.nOrbits;count++)
	{
	    orbMetadata.orbitNumber = startOrbitNum + count;
	    
	    time = startTAI93 - dt1 + count*P;
            PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, time, spacecraftTag, posvelECI, posvelECI+3, epoch);
	    while ( posvelECI[2] < 0.0 && count > 0 )
	    {
		time -= 300.0;
                PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, time, spacecraftTag, posvelECI, posvelECI+3, epoch);
	    }
            time = PGS_EPH_OrbitAscendTime( spacecraftTag, time, aIn, eIn, incIn, anIn, apIn, maIn, epoch);
	    orbMetadata.orbitAscendTime = time;

	    /* keep only 3 digits after decimal points. Rest is garbage. 
	       If it is not cleaned up, two same orbits in two consecutive 
	       files may have different Ascend times */

	    integer_ascend_time = (long int)(((orbMetadata.orbitAscendTime)-(long int)(orbMetadata.orbitAscendTime))*1000.0);
	    orbMetadata.orbitAscendTime = (long int)(orbMetadata.orbitAscendTime)+integer_ascend_time/1000.0;

	    time = startTAI93 - dt2 + count*P;
            time = PGS_EPH_OrbitDecendTime( spacecraftTag, time , aIn, eIn, incIn, anIn, apIn, maIn, epoch);
	    orbMetadata.orbitDescendTime = time;

	    /* keep only 3 digits after decimal points. Rest is garbage. 
	       If it is not cleaned up, two same orbits in two consecutive 
	       files may have different Descend times */

	    integer_descend_time = (long int)(((orbMetadata.orbitDescendTime)-(long int)(orbMetadata.orbitDescendTime))*1000.0);
	    orbMetadata.orbitDescendTime = (long int)(orbMetadata.orbitDescendTime)+integer_descend_time/1000.0;

	    PGS_TD_TAItoUTC(orbMetadata.orbitDescendTime, asciiUTC);

            PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn,
			    orbMetadata.orbitDescendTime,
			    spacecraftTag,
			    posvelECI,
			    posvelECI+3, epoch);
	    PGS_CSC_ECItoECR(0, asciiUTC, NULL,
			     (PGSt_double(*)[6]) posvelECI, 
			     (PGSt_double(*)[6]) posvelECR);
	    PGS_CSC_ECRtoGEO(posvelECR, "WGS84", &longitude, &latitude,
			     &altitude);
	    orbMetadata.orbitDescendLongitude = longitude;
	    numCheck = (int) fwrite(&orbMetadata,sizeof(PGSt_ephemMetadata),1,
				    ephemFilePtr);
	    if (numCheck != 1)
	    {
		printf("error writing to file: %s ...aborting\n",ephFilename);
		break;
	    }
	    
	    if (hdf_file == PGS_TRUE)
	    {
	       /* Write the orbit data */
	       hdfMeta = (PGSt_byte *)malloc(sizeof(PGSt_ephemMetadata));
               pntr = hdfMeta;

               memcpy( pntr, &orbMetadata.orbitNumber,sizeof(int32));
               pntr += sizeof(int32);
               memcpy( pntr, &orbMetadata.orbitAscendTime,sizeof(float64));
               pntr += sizeof(float64);
               memcpy( pntr, &orbMetadata.orbitDescendTime,sizeof(float64));
               pntr += sizeof(float64);
               memcpy( pntr, &orbMetadata.orbitDescendLongitude,sizeof(float64));
	       /* Write the packed Vdata record */
               hdfStatus = VSwrite( vDataId, hdfMeta, 1, FULL_INTERLACE );
            }
	}
	if (hdf_file == PGS_TRUE)
        {
	    /* Detach from the Vdata */
	    VSdetach( vDataId );
	    
	    /* Write the metadata */
	    PGS_EPH_writeMetadata( hdfSdId,
		     		   ephemFileHeader,
				   qaLongGapCount,
				   qaOutofBoundsDataCount,
				   nativeURs,
				   nURs,
				   orbMetadata );
	    
	    /* close the Vset interface */	    
	    Vend( hdfId );
	    Vend( hdfIdAtt );
	    /* Close the HDF fomat files */
	    hdfStatus = Hclose( hdfId );
	    hdfStatus = Hclose( hdfIdAtt );
	    SDend( hdfSdId );
	    SDend( hdfSdIdAtt );
	    Vend( hdfId);
	    Vend( hdfIdAtt );
	    Hclose( hdfId );
	    Hclose( hdfIdAtt );

	    /* free allocated memory */
	    free(hdfHeader);
	    free(hdfHeaderAtt);
	    free(hdfRecordAtt);
	    free(hdfRecord);
	    free(hdfMeta);
	  }

	fclose(ephemFilePtr);
	fclose(attitFilePtr);
        } /* END: for (numCount... */
    } /* END: for (julianDay... */
    printf("\n");
    printf("Done.  Generate another set of ephemeris files (y/[n]):\n-->");
    fgets(inputBuffer,99,stdin);
    switch (inputBuffer[0])
    {
      case 'y':
      case 'Y':
	printf("\n\n\n");
	firstTime = PGS_FALSE;
	custom = PGS_FALSE;
	goto BEGINNING;
    }
    printf("\n");
    return PGS_TRUE;
}

static void quit(void)
{
    printf("\nquitting\nno data generated\n\n");
    exit(0);
}


static PGSt_boolean get_scTag(char *scNameStr, PGSt_tag *spacecraftTag)
{
    char  *spacecraftNameString;
    
    spacecraftNameString = scNameStr;
    *spacecraftTag = 0;
    
    /* allow for use of new PGSd_ prefix */

    if (strncmp(spacecraftNameString,"PGSd_",5) == 0)
	spacecraftNameString += 5;
    else if (strncmp(spacecraftNameString,"pgsd_",5) == 0)
	spacecraftNameString += 5;
    
    if (!strcmp(spacecraftNameString,"TRMM"))
    {
	*spacecraftTag = PGSd_TRMM;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_AM"))
    {
	*spacecraftTag = PGSd_EOS_AM;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_PM"))
    {
	*spacecraftTag = PGSd_EOS_PM;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_AURA"))
    {
	*spacecraftTag = PGSd_EOS_AURA;
	return PGS_TRUE;
    }
    
    /* if input is not actually supported test every case before returning since
       I don't feel like putting a test after each case */

    if (!strcmp(spacecraftNameString,"trmm"))
      *spacecraftTag = PGSd_TRMM;
    if (!strcmp(spacecraftNameString,"eos_am"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"EOS AM"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"EOS-AM"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eos am"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eos-am"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"EOSAM"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eosam"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eos_pm"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"EOS PM"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"EOS-PM"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"eos pm"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"eos-pm"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"EOSPM"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"eospm"))
      *spacecraftTag = PGSd_EOS_PM;
    if (!strcmp(spacecraftNameString,"eos_aura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"EOS-AURA"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"eos aura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"eos-aura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"EOS AURA"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"EOSAURA"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"eosaura"))
      *spacecraftTag = PGSd_EOS_AURA;

    if (!strcmp(spacecraftNameString,"custom"))
      *spacecraftTag = PGSd_CUSTOM;
    if (!strcmp(spacecraftNameString,"CUSTOM"))
      *spacecraftTag = PGSd_CUSTOM;

    if (*spacecraftTag)
      return PGS_TRUE;
    else
      return PGS_FALSE;
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

static void
tipEuler(
    PGSt_double   position[3],
    PGSt_double   velocity[3],
    PGSt_integer  eulerAngleOrder[3],
    PGSt_double   eulerAngles[3],
    PGSt_integer  direction)
{
    PGSt_double   quatEuler[4];
    PGSt_double   quatTIPtoORB[4];
    PGSt_double   *quatORBtoTIP;
    PGSt_double   *quatSCtoORB;
    PGSt_double   *quatSCtoTIP;
    
    quatORBtoTIP = quatTIPtoORB;
    quatSCtoORB = quatEuler;
    quatSCtoTIP = quatEuler;
    
    /* Get the quaternion that gives the equivalent rotation as the s/c Euler
       angles. */
    
    PGS_CSC_EulerToQuat(eulerAngles,
			eulerAngleOrder, quatEuler);
    
    /* The numbers 6378137.0 and 6356752.314245 below are the Earth's equatorial
       and polar radii, respectively.  Note that PGS_CSC_TiltYaw() MUST be
       called with True of Date position and velocity.  TRMM ephemeris elements
       are True of Date. */
    
    PGS_CSC_TiltYaw(position, velocity, 6378137.0, 6356752.314245,
		    quatTIPtoORB);
    
    if (direction == PGSd_TO_TIP)
    {
	/* The quaternion defining the rotation from the TIP to SC reference
	   frame is determined by the successive rotations: SC to ORB => ORB to
	   TIP. */
	
	/*  The ORB to TIP quaternion is the conjugate of the TIP to ORB
	    quaternion. */

	quatORBtoTIP[1] = -quatTIPtoORB[1];
	quatORBtoTIP[2] = -quatTIPtoORB[2];
	quatORBtoTIP[3] = -quatTIPtoORB[3];

	/* quatEuler is now quatSCtoTIP */

	PGS_CSC_quatMultiply(quatORBtoTIP, quatSCtoORB, quatEuler);
    }
    else
    {
	/* The quaternion defining the rotation from the SC to ORB reference
	   frame is determined by the successive rotations: SC to TIP => TIP to
	   ORB. */
	
	/* quatEuler is now quatSCtoORB */

	PGS_CSC_quatMultiply(quatTIPtoORB, quatSCtoTIP, quatEuler);
    }
    
    /* finally convert the SC to ORB (or SC to TIP) quaternion back to a set of
       Euler angles */
    
    PGS_CSC_QuatToEuler(quatEuler, eulerAngleOrder, eulerAngles);

}

static PGSt_double
PGS_EPH_OrbitAscendTime(
    PGSt_tag              spacecraftTag,
    PGSt_double           startTAI93,
    PGSt_double 	  aIn,
    PGSt_double 	  eIn, 
    PGSt_double 	  incIn, 
    PGSt_double 	  anIn, 
    PGSt_double 	  apIn, 
    PGSt_double 	  maIn,
    PGSt_double           epoch)           /* epoch in TAI format */
{
    PGSt_double           posECI[3];
    PGSt_double           velECI[3];
    PGSt_double*          zPos;
    PGSt_double           deltaT;
    PGSt_double           t1;
    PGSt_double           t2;
    PGSt_double           ascend_time;

    zPos = posECI+2;
    t1 = startTAI93;
    deltaT = 300.0;
    
    PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, t1, spacecraftTag, posECI, velECI , epoch);
    while ( *zPos < 0.0 )
    {
	t1 -= deltaT;
        PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, t1, spacecraftTag, posECI, velECI, epoch );
    }
    
    t2 = t1 - deltaT;

    while ( deltaT > 0.0001 )
    {
        PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, t2, spacecraftTag, posECI, velECI , epoch);
	if ( *zPos < 0.0 )
	{
	    deltaT = deltaT/2.0;
	    t2 = t1 - deltaT;
	}
	else
	{
	    t1 = t2;
	    t2 = t2 - deltaT;
	}
    }

    ascend_time = t1 + deltaT;

    return (ascend_time);
}

static PGSt_double
PGS_EPH_OrbitDecendTime(
    PGSt_tag              spacecraftTag,
    PGSt_double           startTAI93,
    PGSt_double 	  aIn,
    PGSt_double 	  eIn, 
    PGSt_double 	  incIn, 
    PGSt_double 	  anIn, 
    PGSt_double 	  apIn, 
    PGSt_double 	  maIn,
    PGSt_double           epoch)           /* epoch in TAI format */
{
    PGSt_double           posECI[3];
    PGSt_double           velECI[3];
    PGSt_double*          zPos;
    PGSt_double           deltaT;
    PGSt_double           t1;
    PGSt_double           t2;
    PGSt_double           descend_time;

    zPos = posECI+2;
    t1 = startTAI93;
    deltaT = 300.0;
    
    PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, t1, spacecraftTag, posECI, velECI , epoch);
    while ( *zPos < 0.0 )
    {
	t1 -= deltaT;
        PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, t1, spacecraftTag, posECI, velECI , epoch);
    }
    
    t2 = t1 + deltaT;

    while ( deltaT > 0.0001 )
    {
        PGS_EPH_orbSim( aIn, eIn, incIn, anIn, apIn, maIn, t2, spacecraftTag, posECI, velECI , epoch);
	if ( *zPos < 0.0 )
	{
	    deltaT = deltaT/2.0;
	    t2 = t1 + deltaT;
	}
	else
	{
	    t1 = t2;
	    t2 = t2 + deltaT;
	}
    }

    descend_time = t1 + deltaT;

    return (descend_time);
}

/* To write the metadata to the .met file and appends the science data to the .hdf file */
PGSt_boolean PGS_EPH_writeMetadata(int                HdfSdId,
                                   PGSt_ephemHeader   ephemFileHeader,
                                   EcTULongInt        qaLongGapCount,
                                   EcTULongInt        qaOutofBoundsDataCount,
                                   char               parentURs[10][256],
                                   PGSt_uinteger      nURs,
                                   PGSt_ephemMetadata orbMetadata)
{
    const int   DpCPrInventoryMetaData   = 1;
    const PGSt_PC_Logical  DpCPrMcfHdfTemplateEphemeris = 1400; 

    char*  autoQaFlag = "Passed";
    char*  autoQaFlagExp = "Passed all QA analysis criteria.";
    char *convertFirstDateTime=NULL;
    char *convertFirstDate=NULL;
    char *convertFirstTime=NULL;
    char *convertLastDateTime=NULL;
    char *convertLastDate=NULL;
    char *convertLastTime=NULL;
/*  PGSt_integer       attrValueInt[5]; */
    char               *urList[10];
    char               attribute[64];
    char               fileMessage[241];
    char               message[241];
    char               mnemonic[32];
    char               pgsMessage[241];
    int		       a;
    int                iOrb;
    int                iElm;
    int                qaPercentInterpolatedData = 0;
    int                qaPercentOutofBoundsData = 0;
    int                qaPercentMissingData = 0;
    char               *orbitalModelName = "Ptolomaic";
    char               *parameterName = "Ephemeris";
    double             ip;
    PGSt_boolean       returnCode;
    PGSt_SMF_status    pgsStatus;
    PGSt_MET_all_handles  mdHandles;

    double  startTime;
    double  endTime;
    int  orbitNumberStart;
    int  orbitNumberEnd;
    int  orbitCount;

    /* Save the pertinent fields form the ephemeris header */
    orbitCount = ephemFileHeader.nOrbits;
    startTime = ephemFileHeader.startTime;
    endTime =   ephemFileHeader.endTime;
    orbitNumberStart = orbMetadata.orbitNumber;
    orbitNumberEnd   = orbMetadata.orbitNumber;

    qaPercentInterpolatedData =  (int) ephemFileHeader.qaStatistics[0] ;

    if ( modf( (double) ephemFileHeader.qaStatistics[0], &ip ) >= 0.5 )
    {
        qaPercentInterpolatedData++;
    }

    if (qaLongGapCount < 1)
    {
        qaPercentMissingData = 0;
    }
    else
    {
        qaPercentMissingData =  (int) ephemFileHeader.qaStatistics[1] ;
        if ( modf( (double) ephemFileHeader.qaStatistics[1], &ip ) >= 0.5 )
        {
            qaPercentMissingData++;
        }
        if ( ephemFileHeader.qaStatistics[1] > 0 && qaPercentMissingData == 0 )
        {
            qaPercentMissingData = 1;
        }
    }

    if (qaOutofBoundsDataCount < 1)
    {
        qaPercentOutofBoundsData = 0;
    }
    else
    {
        qaPercentOutofBoundsData =  (int) ephemFileHeader.qaStatistics[2] ;
        if ( modf( (double) ephemFileHeader.qaStatistics[2], &ip ) >= 0.5 )
        {
            qaPercentOutofBoundsData++;
        }
        if ( ephemFileHeader.qaStatistics[2] > 0 && qaPercentOutofBoundsData == 0 )
        {
            qaPercentOutofBoundsData = 1;
        }
    }

    for (iElm=0; iElm<nURs; iElm++)
    {
        urList[iElm] = &parentURs[iElm][0];
    }
    urList[nURs] = NULL;
    returnCode = PGS_TRUE;

    /* Initialize ephemeris dataset timerange processing */
    convertFirstDateTime = (char *)malloc ( 29 );
    convertFirstDate = (char *)malloc ( 11 );
    convertFirstTime = (char *)malloc ( 16 );
    convertLastDateTime = (char *)malloc ( 29 );
    convertLastDate = (char *)malloc ( 11 );
    convertLastTime = (char *)malloc ( 16 );

    /* Build UTC dataset timerange */
    if ( returnCode == PGS_TRUE )
    {
        pgsStatus = PGS_TD_TAItoUTC( startTime, convertFirstDateTime );
        PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );

        if ( PGS_SMF_TestStatusLevel(pgsStatus) <= PGS_SMF_MASK_LEV_W )
        {
            pgsStatus = PGS_TD_TAItoUTC( endTime, convertLastDateTime );
            PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );
        }

        if ( PGS_SMF_TestStatusLevel(pgsStatus) > PGS_SMF_MASK_LEV_W )
        {
            PGS_SMF_GetMsgByCode( PGSMET_E_SD_SETATTR, message );
            sprintf( fileMessage,
                     message,
                     "PGS_TD_TAItoUTC/PGS_TD_ASCIItime_AtoB",
                     pgsMessage );
            PGS_SMF_SetDynamicMsg( PGSMET_E_SD_SETATTR,
                                   fileMessage,
                                   "PGS_EPH_writeMetadata()" );

            returnCode  = PGS_FALSE;
        }
    }


    /* Initialize MET processing */
    if ( returnCode == PGS_TRUE )
    {
        pgsStatus = PGS_MET_Init( DpCPrMcfHdfTemplateEphemeris, mdHandles );

        if ( pgsStatus != PGS_S_SUCCESS )
        {

            PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );

            PGS_SMF_GetMsgByCode( PGSMET_E_SD_SETATTR, message );
            sprintf( fileMessage, message, "PGS_MET_Init", pgsMessage );
            PGS_SMF_SetDynamicMsg( PGSMET_E_SD_SETATTR,
                                   fileMessage,
                                   "PGS_EPH_writeMetadata()" );

            returnCode  = PGS_FALSE;
        }

    }


    if ( returnCode == PGS_TRUE )
    {
        if ( pgsStatus == PGS_S_SUCCESS )
        {
	   for (a=0; a<5; a++)
   /*		attrValueInt[a]    = 1; */
	    /* Set attribut inputpointer */
            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "InputPointer",
                                         urList );
        }

	/* Set attribut RangeBeginningDate */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            strncpy( &convertFirstDate[0], &convertFirstDateTime[0], 10 );
            convertFirstDate[10] = '\0';
            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "RangeBeginningDate",
                                         &convertFirstDate );
        }

	/* Set attribut parameterName */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "parameterName.1",
                                         &parameterName );
        }

	/* Set attribut RangeBeginningTime */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            strncpy( &convertFirstTime[0], &convertFirstDateTime[11], 15 );
            convertFirstTime[15] = '\0';

            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "RangeBeginningTime",
                                         &convertFirstTime );
        }

	/* Set attribut RangeEndingDate */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            strncpy( &convertLastDate[0], &convertLastDateTime[0], 10 );
            convertLastDate[10] = '\0';

            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "RangeEndingDate",
                                         &convertLastDate );
        }

	/* Set attribut RangeEndingTime */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            strncpy( &convertLastTime[0], &convertLastDateTime[11], 15 );
            convertLastTime[15] = '\0';

            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "RangeEndingTime",
                                         &convertLastTime );
        }

	/* Set attribut qaPercentInterpolatedData */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "qaPercentInterpolatedData.1",
                                         &qaPercentInterpolatedData );
        }

	/* Set attribut qaPercentOutofBoundsData */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "qaPercentOutofBoundsData.1",
                                         &qaPercentOutofBoundsData );
        }

	/* Set attribut qaPercentMissingData */
        if ( pgsStatus == PGS_S_SUCCESS )
        {
            pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                         "qaPercentMissingData.1",
                                         &qaPercentMissingData );
        }

	/* Check the set attribute status */
        if ( pgsStatus != PGS_S_SUCCESS )
        {
            PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );

            PGS_SMF_GetMsgByCode( PGSMET_E_SD_SETATTR, message );
            sprintf( fileMessage, message, "PGS_MET_SetAttr", pgsMessage );
            PGS_SMF_SetDynamicMsg( PGSMET_E_SD_SETATTR,
                                   fileMessage,
                                   "PGS_EPH_writeMetadata()" );

            returnCode  = PGS_FALSE;
        }

    }

    /* Set orbit specific attributes */
    if ( returnCode == PGS_TRUE )
    {
        iOrb = 0;

        while ( iOrb < orbitCount && returnCode == PGS_TRUE )
        {
	    /* Build UTC AscendingNodeCrossingDataTime */
            pgsStatus = PGS_TD_TAItoUTC( orbMetadata.orbitAscendTime,
                                         convertFirstDateTime );
            PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );

	    /* Build UTC EquatorCrossingDataTime */
            if ( PGS_SMF_TestStatusLevel(pgsStatus) <= PGS_SMF_MASK_LEV_W )
            {
                pgsStatus = PGS_TD_TAItoUTC( orbMetadata.orbitDescendTime,
                                             convertLastDateTime );
                PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );
            }

	    /* Check the time conversion status */
            if ( PGS_SMF_TestStatusLevel(pgsStatus) > PGS_SMF_MASK_LEV_W )
            {
                PGS_SMF_GetMsgByCode( PGSMET_E_SD_SETATTR, message );
                sprintf( fileMessage, message, "PGS_TD_TAItoUTC", pgsMessage );
                PGS_SMF_SetDynamicMsg( PGSMET_E_SD_SETATTR,
                                       fileMessage,
                                       "PGS_EPH_writeMetadata()" );

                returnCode  = PGS_FALSE;
            }

	    /* Set orbit metadata attributes */
            if ( returnCode == PGS_TRUE )
            {
		/* Set attribute OrbitalModelName */
                if ( pgsStatus == PGS_S_SUCCESS )
                {
                    sprintf(attribute, "%s.%i", "OrbitalModelName", iOrb);
                    pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                                 attribute,
                                                 &orbitalModelName);
                }

		/* Set attribute EquatorCrossingDate */
                if ( pgsStatus == PGS_S_SUCCESS )
                {
                    sprintf( attribute,
                             "%s.%i",
                             "EquatorCrossingDate",
                             iOrb );

                    strncpy( &convertLastDate[0], &convertLastDateTime[0], 10 );
                    convertLastDate[10] = '\0';

                    pgsStatus =
                              PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                               attribute,
                                               &convertLastDate );
                }

		/* Set attribute EquatorCrossingTime */
                if ( pgsStatus == PGS_S_SUCCESS )
                {
                    sprintf( attribute,
                             "%s.%i",
                             "EquatorCrossingTime",
                             iOrb );

                    strncpy( &convertLastTime[0], &convertLastDateTime[11], 15 );
                    convertLastTime[15] = '\0';

                    pgsStatus =
                              PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                               attribute,
                                               &convertLastTime );
                }

		/* Set attribute EquatorCrossingLongitude */
                if ( pgsStatus == PGS_S_SUCCESS )
                {
                    sprintf( attribute, "%s.%i", "EquatorCrossingLongitude",
                                                                         iOrb );

                    pgsStatus =
                              PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                               attribute,
                                               &orbMetadata.orbitDescendLongitude );
                }

		/* Set attribute StartOrbitNumber */
                if ( pgsStatus == PGS_S_SUCCESS )
                {
                    sprintf( attribute, "%s.%i", "StartOrbitNumber", iOrb );

                    pgsStatus =
                              PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                               attribute,
                                               &orbitNumberStart );
                }

		/* Set attribute StopOrbitNumber */
                if ( pgsStatus == PGS_S_SUCCESS )
                {
                    sprintf( attribute, "%s.%i", "StopOrbitNumber", iOrb );

                    pgsStatus =
                              PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                               attribute,
                                               &orbitNumberEnd );
                }

		/* Check the set attribute status */
                if ( pgsStatus != PGS_S_SUCCESS )
                {
                    PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );

                    PGS_SMF_GetMsgByCode( PGSMET_E_SD_SETATTR, message );
                    sprintf( fileMessage,
                             message,
                             "PGS_MET_SetAttr",
                             pgsMessage );
                    PGS_SMF_SetDynamicMsg( PGSMET_E_SD_SETATTR,
                                           fileMessage,
                                           "PGS_EPH_writeMetadata()" );

                    returnCode  = PGS_FALSE;
                }
            }


            iOrb++;
        }

        if (pgsStatus != PGS_S_SUCCESS )
        {
            PGS_SMF_SetStaticMsg( PGSMET_E_SD_SETATTR,
                                  "PGS_EPH_writeMetadata()" );
            pgsStatus = PGSMET_E_SD_SETATTR;
        }


        if (qaLongGapCount >= 1)
        {
            autoQaFlag = "Failed";
            autoQaFlagExp = "Long gap exists in ephemeris time-line.";
        }

        if (qaOutofBoundsDataCount >= 1)
        {
            autoQaFlag = "Failed";
            autoQaFlagExp = "Range violation in ephemeris time-line.";
        }

        if (qaLongGapCount >= 1 &&
            qaOutofBoundsDataCount >= 1)
        {
            autoQaFlag = "Failed";
            autoQaFlagExp = "Long gap/range violation in ephemeris time-line.";
        }
        pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                     "AutomaticQualityFlag.1",
                                     &autoQaFlag );

        if (pgsStatus != PGS_S_SUCCESS )
        {
            PGS_SMF_SetStaticMsg( PGSMET_E_SD_SETATTR,
                                  "PGS_EPH_writeMetadata()" );
        }

        pgsStatus = PGS_MET_SetAttr( mdHandles[DpCPrInventoryMetaData],
                                     "AutomaticQualityFlagExplanation.1",
                                     &autoQaFlagExp );

        if (pgsStatus != PGS_S_SUCCESS )
        {
            PGS_SMF_SetStaticMsg( PGSMET_E_SD_SETATTR,
                                  "PGS_EPH_writeMetadata()" );
        }

    }

    /* Write the metadata to the HDF file */
    if ( returnCode == PGS_TRUE )
    {
        pgsStatus = PGS_MET_Write( mdHandles[DpCPrInventoryMetaData],
                                   "coremetadata",
                                   HdfSdId );

        if ( pgsStatus != PGS_S_SUCCESS )
        {
            PGS_SMF_GetMsg( &pgsStatus, mnemonic, pgsMessage );

            PGS_SMF_GetMsgByCode( PGSMET_E_SD_SETATTR, message );
            sprintf( fileMessage, message, "PGS_MET_Write", pgsMessage );
            PGS_SMF_SetDynamicMsg( PGSMET_E_SD_SETATTR,
                                   fileMessage,
                                   "PGS_EPH_writeMetadata()" );

            returnCode  = PGS_FALSE;
        }
    }

    /* Clean-up from MET processing */
    PGS_MET_Remove();

    if(convertFirstDateTime != NULL)
    {
	free(convertFirstDateTime);
	convertFirstDateTime=NULL;
    }

    if(convertFirstDate != NULL)
    {
	free(convertFirstDate);
	convertFirstDate=NULL;
    }

    if(convertFirstTime != NULL)
    {
	free(convertFirstTime);
	convertFirstTime=NULL;
    }

    if(convertLastDateTime != NULL)
    {
	free(convertLastDateTime);
	convertLastDateTime=NULL;
    }

    if(convertLastDate != NULL)
    {
	free(convertLastDate);
	convertLastDate=NULL;
    }

    if(convertLastTime != NULL)
    {
	free(convertLastTime);
	convertLastTime=NULL;
    }

    return( returnCode );

}



