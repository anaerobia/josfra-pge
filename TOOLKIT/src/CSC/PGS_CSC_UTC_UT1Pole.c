/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
      PGS_CSC_UTC_UT1Pole.c

DESCRIPTION:
      This file contains the function PGS_CSC_UTC_UT1Pole()

AUTHOR:
      Deborah A. Foch / Applied Research Corporation
      Peter D. Noerdlinger / Applied Research Corporation
      Guru Tej S. Khalsa / Applied Research Corporation
      Anubha Singhal / Applied Research Corporation
      Curt Schafer   / Steven Myers & Associates

HISTORY:
      10-Feb-1994   DAF/PDN      Designed
      23-Feb-1994   DAF          Initial Version
      25-Feb-1994   DAF          Revised to read table only on first call to
                                 function
      28-Feb-1994   DAF          Revised to improve I/O and memory usage 
                                 - also changed name of function
      01-Mar-1994   PDN          Revised to simplify calling list and to return
                                 defaults of 0.0 when outside time range of
				 table
      02-Mar-1994   PDN/GTSK     Corrected first/last line usage
      15-Jul-1994   AS           Revised to use interpolation to extract the
                                 x,y pole offsets and UT1-UTC value
      08-Aug-1994   AS           Modified error reporting
      23-Sep-1994   GTSK         Modified to use PGS_IO_ routines to open and
                                 close data file.
      28-Apr-1994   GTSK         Changed types of some values read from the
                                 file utcpole.dat to be float rather than
				 PGSt_double to speed up processing and reduce
				 storage requirements.
     09-May-1994    GTSK         Altered table lookup algorithm to optimize for
                                 successive calls with similar times.
     01-Jun-1994    GTSK         Changed input Julian day from a single number
                                 to an array of two numbers.
     23-Jul-1996    PDN          Deleted obsolete return status 
                                    PGSCSC_W_INTERIM_UT1
     09-Jul-1999     CS          Updated for Threadsafe functionality

END_FILE_PROLOG:
*******************************************************************************/
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_PROLOG
 
TITLE: 
      This function gets  UT1 from UTC and gets the x,y  pole offsets

NAME: 
      PGS_CSC_UTC_UT1Pole()

SYNOPSIS: 
C:     
      #include <PGS_CSC.h>

      PGSt_SMF_Status
      PGS_CSC_UTC_UT1Pole(
            PGSt_double    jdUTC,
	    PGSt_double    *xpole,
            PGSt_double    *ypole,
            PGSt_double    *diffUT1UTC,
            PGSt_double    *jdtable)
  
DESCRIPTION:
      This tool accesses the file 'utcpole.dat' and extracts using 
      interpolation the x,y pole position in seconds of arc and the 
      difference of UT1 and UTC, given an input Julian day.
      
INPUTS:
      Name       Description               Units     Min     Max
      ----       -----------               -----     ---     ---
      jdUTC      input Julian day          days      2444054 245446.5

OUTPUTS:
      Name       Description               Units            Min        Max
      ----       -----------                -----            ---       ---
      xpole      x pole position           seconds of arc   -1.0       +1.0
      ypole      y pole position           seconds of arc   -1.0       +1.0
      diffUT1UTC difference of UT1 and UTC seconds of time  -1.0       +1.0
                 in seconds
      jdtable    Julian day of reported    days             2444054.5  2454466.5
                 position and times - the 
                 smaller of the 2 table 
                 entries between which the 
                 input day falls

RETURNS:
      PGS_S_SUCCESS                          
      PGSCSC_E_INACCURATE_UTCPOLE 
      PGSCSC_W_PREDICTED_UT1
      PGSCSC_W_JD_OUT_OF_RANGE
      PGSCSC_W_DATA_FILE_MISSING
      PGS_E_TOOLKIT
      PGSTSF_E_GENERAL_FAILURE       

EXAMPLES:
C:   
      PGSt_SMF_status   returnStatus;
      PGSt_double       jdUTC=2444056.5;
      PGSt_double       xpole;
      PGSt_double       ypole;
      PGSt_double       diffUT1UTC;
      PGSt_double       jdtable
      char              err[PGS_SMF_MAX_MNEMONIC_SIZE];
      char              msg[PGS_SMF_MAX_MSG_SIZE];

      returnStatus = PGS_CSC_UTC_UT1Pole(jdUTC,&xpole,&ypole,&diffUT1UTC,
                                         &jdtable)

      if(returnStatus != PGS_S_SUCCESS)
      {
          PGS_SMF_GetMsg(&returnStatus,err,msg);
          printf("\nERROR: %s",msg);
      }

NOTES:
   The minimum and maximum values of jdUTC are as of July 1994.

   TIME ACRONYMS:
     
     UTC is:  Coordinated Universal Time

   JULIAN DATES:

     Format:

       Toolkit Julian dates are kept as an array of two real (high precision)
       numbers (C: PGSt_double, FORTRAN: DOUBLE PRECISION).  The first element
       of the array should be the half integer Julian day (e.g. N.5 where N is a
       Julian day number).  The second element of the array should be a real
       number greater than or equal to zero AND less than one (1.0) representing
       the time of the current day (as a fraction of that (86400 second) day.
       This format allows relatively simple translation to calendar days (since
       the Julian days begin at noon of the corresponding calendar day).  Users
       of the Toolkit are encouraged to adhere to this format to maintain high
       accuracy (one number to track significant digits to the left of the
       decimal and one number to track significant digits to the right of the
       decimal).  Toolkit functions that do NOT require a Julian type date as an
       input and return a Julian date will return the Julian date in the above
       mentioned format.  Toolkit functions that require a Julian date as an
       input and do NOT return a Julian date will first convert the input date
       (internal) to the above format.  Toolkit functions that have a Julian
       date as both an input and an output will assume the input is in the above
       described format but will not check and the format of the output may not
       be what is expected if any other format is used for the input.

     Meaning:

       Toolkit "Julian dates" are all based on UTC.  A Julian date in any other
       "time" (e.g. TAI, TDT, UT1, etc.) is based on the difference between that
       "time" and the equivalent UTC time (differences range in magnitude from 0
       seconds to about a minute).

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
      PGSTK - 1170, 1220
 
DETAILS:
      NONE

GLOBALS:
      PGSg_TSF_CSCUTC_UT1Polexpoles
      PGSg_TSF_CSCUTC_UT1Poleypoles
      PGSg_TSF_CSCUTC_UT1Poleut1utcTab
      PGSg_TSF_CSCUTC_UT1Polejd
      PGSg_TSF_CSCUTC_UT1Polejd0
      PGSg_TSF_CSCUTC_UT1Polejdlast
      PGSg_TSF_CSCUTC_UT1Polecnt
      PGSg_TSF_CSCUTC_UT1Polenumrecs
      PGSg_TSF_CSCUTC_UT1Poletablerd
      PGSg_TSF_CSCUTC_UT1Polefilenotfound

FILES:
      utcpole.dat

FUNCTIONS CALLED:
      PGS_IO_Gen_Open()
      PGS_IO_Gen_Close()
      PGS_SMF_SetStaticMsg()
      PGS_SMF_SetDynamicMsg()
      PGS_TSF_GetTSFMaster()
      PGS_TSF_GetMasterIndex()
      PGS_TSF_GetTSFMaster()    
      PGS_TSF_GetMasterIndex()   

      
END_PROLOG	
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_IO.h>
#include <PGS_TSF.h>

#define  MAX_RECS     2400
#define  MAX_HEADER   139

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_UTC_UT1Pole()"

PGSt_SMF_status 
PGS_CSC_UTC_UT1Pole(         /* extracts using interpolation the x,y pole
				position in seconds of arc and the difference of
				UT1 and UTC, given an input Julian Day */
    PGSt_double jdUTC[2],    /* INPUT Julian day */
    PGSt_double *xpole,      /* x pole position, in seconds of arc */
    PGSt_double *ypole,      /* y pole position, in seconds of arc */
    PGSt_double *diffUT1UTC, /* UT1-UTC value (seconds) from table */
    PGSt_double *jdtable)    /* pointer to actual Julian Day value in table,
				which corresponds to xpole, ypole, and
				diffUT1UTC */
{
    PGSt_double        jdUTCadd;             /* input UTC Julian Date as a 
						single number:
						jdUTC[0] + jdUTC[1] */
    static float       xpoles[MAX_RECS];     /* x pole position in seconds of
                                                arc*/
    static float       ypoles[MAX_RECS];     /* y pole position in seconds of
                                                arc */
    static float       ut1utcTab[MAX_RECS];  /* UT1-UTC correction seconds of
                                                time */
    static PGSt_double jd[MAX_RECS];         /* converted Julian Date from MJD
                                                in table (add 2400000.5 to
						mjd) */
    static char        accuracy[MAX_RECS];   /* accuracy of data - 'f' for 
                                                final, or 'p' for predicted */
    static PGSt_double jd0 = 0;              /* jdutc start  of table */
    static PGSt_double jdlast = 0;           /* jdutc end of table    */

    double             mjd = 0;              /* Modified Julian Date (MJD)
                                                in table (eg.44054.0)
					        = Julian Date minus 2400000.5 */
    float              xpoleTemp;            /* for reading in xpole */
    float              ypoleTemp;            /* for reading in ypole */
    float              ut1utcTabTemp;        /* for reading UT1-UTC */
    char               accuracyTemp;         /* for reading accuracy */

    static int         cnt = 1;              /* array member counter for sort,
						initialized to 1 */

    FILE               *fp;                  /* pointer to current position in
                                                file */
    PGSt_SMF_status    returnStatus;         /* return returnStatus */
    PGSt_SMF_status    returnStatus1;        /* return status of PGS function
						calls */

    int                fileread;             /* file read error return 
                                                returnStatus */
    long               fileOffset;
    long               totalFileSize;
    
    long               numSkip;              /* number of records to skip when
						loading only part of the file to
						memory from disk */
    
    static PGSt_boolean tablerd = PGS_FALSE; /* Indicates whether table has
						been read into memory yet */

    static PGSt_boolean filenotfound = PGS_FALSE; /* indicates whether an 
                                                    attempt to read a file
                                                    which can't be found has
                                                    already been made */

    static int         numrecs = 0;          /* number of records read in from
                                                file (minus 1) */
    char               specifics[PGS_SMF_MAX_MSG_SIZE]; /* detailed error msg */

#ifdef _PGS_THREADSAFE

    /*  Declare variables used for THREADSAFE version to replace statics
        The local names are appended with TSF and globals are preceded with
        directory and function name */

    PGSt_TSF_MasterStruct *masterTSF;
    char *accuracyTSF;
    float xpolesTSF[MAX_RECS];
    float ypolesTSF[MAX_RECS];
    float ut1utcTabTSF[MAX_RECS];
    PGSt_double jdTSF[MAX_RECS];
    PGSt_double jd0TSF;
    PGSt_double jdlastTSF;
    int     cntTSF;
    int     numrecsTSF;
    PGSt_boolean tablerdTSF;
    PGSt_boolean filenotfoundTSF;

            /*  Globals     originals in PGS_TSF_SetupCSC.c */

   extern float        PGSg_TSF_CSCUTC_UT1Polexpoles[][MAX_RECS];
   extern float        PGSg_TSF_CSCUTC_UT1Poleypoles[][MAX_RECS];
   extern float        PGSg_TSF_CSCUTC_UT1Poleut1utcTab[][MAX_RECS];
   extern PGSt_double  PGSg_TSF_CSCUTC_UT1Polejd[][MAX_RECS];
   extern PGSt_double  PGSg_TSF_CSCUTC_UT1Polejd0[];
   extern PGSt_double  PGSg_TSF_CSCUTC_UT1Polejdlast[];
   extern int          PGSg_TSF_CSCUTC_UT1Polecnt[];
   extern int          PGSg_TSF_CSCUTC_UT1Polenumrecs[];
   extern PGSt_boolean PGSg_TSF_CSCUTC_UT1Poletablerd[];
   extern PGSt_boolean PGSg_TSF_CSCUTC_UT1Polefilenotfound[];
   int masterTSFIndex;
   int initx;                /* initialize index */

   /* Get index from PGS_TSF_GetMasterIndex()
           Initialize the Key used for the THREADSAFE version */

   returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
   masterTSFIndex = PGS_TSF_GetMasterIndex();

   /* Test for incorrect returns    IF BAD leave function   */

   if (PGS_SMF_TestErrorLevel(returnStatus) || masterTSFIndex ==
                          PGSd_TSF_BAD_MASTER_INDEX)
   {
       return PGSTSF_E_GENERAL_FAILURE;
   }

   /* Initialize the Variables used for the THREADSAFE version */

   accuracyTSF = (char *) pthread_getspecific(
               masterTSF->keyArray[PGSd_TSF_KEYCSCUTC_UT1POLEACCURACY]);
   for (initx=0; initx<MAX_RECS; initx++)
   {
       xpolesTSF[initx] = PGSg_TSF_CSCUTC_UT1Polexpoles[masterTSFIndex][initx];
       ypolesTSF[initx] = PGSg_TSF_CSCUTC_UT1Poleypoles[masterTSFIndex][initx];
       ut1utcTabTSF[initx] = PGSg_TSF_CSCUTC_UT1Poleut1utcTab[masterTSFIndex][initx];
       jdTSF[initx] = PGSg_TSF_CSCUTC_UT1Polejd[masterTSFIndex][initx];
    }
    jd0TSF = PGSg_TSF_CSCUTC_UT1Polejd0[masterTSFIndex];
    jdlastTSF = PGSg_TSF_CSCUTC_UT1Polejdlast[masterTSFIndex];
    cntTSF = PGSg_TSF_CSCUTC_UT1Polecnt[masterTSFIndex];
    numrecsTSF = PGSg_TSF_CSCUTC_UT1Polenumrecs[masterTSFIndex];
    tablerdTSF = PGSg_TSF_CSCUTC_UT1Poletablerd[masterTSFIndex];
    filenotfoundTSF = PGSg_TSF_CSCUTC_UT1Polefilenotfound[masterTSFIndex];
#endif

    /* set message to indicate success */

    returnStatus = PGS_S_SUCCESS;

    /* input is UTC Julian date as two numbers, convert to one number */

    jdUTCadd = jdUTC[0] + jdUTC[1];

    /* If the Julian day fraction is greater than 0.99, subtract 0.000001 to
       avoid computer round-off errors to the next Julian day */
    
    if (fmod(jdUTC[1],1.) > .99)
	jdUTCadd -= 1.0E-6;
    
    /* Initialize return values */

    *xpole   = 0.0;
    *ypole   = 0.0;
    *jdtable = 0.0;
    *diffUT1UTC  = 0.0;

    /* If utcpole file has already been read into memory, skip to the SORT
       section */

#ifdef _PGS_THREADSAFE

    /*  Threadsafe Protect: tablerd  and  filenotfound    */

    if(tablerdTSF == PGS_FALSE)
    {
        if (filenotfoundTSF == PGS_TRUE)
#else
    if(tablerd == PGS_FALSE)
    {
        if (filenotfound == PGS_TRUE)
#endif
        {
	    returnStatus = PGSCSC_W_DATA_FILE_MISSING;
	    sprintf(specifics,
		    "unable to open file 'utcpole.dat'(logical ID: %d),"
		    " UT1-UTC and polar motion not available",
		    PGSd_UTCPOLE);
            PGS_SMF_SetDynamicMsg(returnStatus, specifics, FUNCTION_NAME);
	    return returnStatus;
        }

	/* Open utcpole file */
        else 
        {
	    returnStatus = PGS_IO_Gen_Open(PGSd_UTCPOLE,PGSd_IO_Gen_Read,&fp,1);
	    switch (returnStatus)
	    {
	      case PGS_S_SUCCESS:
		break;

	      case PGS_E_UNIX:
	      case PGSIO_E_GEN_OPENMODE:
	      case PGSIO_E_GEN_FILE_NOEXIST:
	      case PGSIO_E_GEN_REFERENCE_FAILURE:
		returnStatus = PGSCSC_W_DATA_FILE_MISSING;
		sprintf(specifics,
			"unable to open file 'utcpole.dat'(logical ID: %d),"
			" UT1-UTC and polar motion not available",
			PGSd_UTCPOLE);
		PGS_SMF_SetDynamicMsg(returnStatus, specifics, FUNCTION_NAME);

#ifdef _PGS_THREADSAFE

                /* Threadsafe Protect:   filenotfound,   numrecs
                         reassign their values for next use */

                filenotfoundTSF = PGS_TRUE;
                PGSg_TSF_CSCUTC_UT1Polefilenotfound[masterTSFIndex] = 
                                                           filenotfoundTSF;
		return returnStatus;

	      default:
		PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
                filenotfoundTSF = PGS_TRUE;
                PGSg_TSF_CSCUTC_UT1Polefilenotfound[masterTSFIndex] = 
                                                         filenotfoundTSF;
                return PGS_E_TOOLKIT;
            }

            numrecsTSF = 0;           /* initialize record count */
            PGSg_TSF_CSCUTC_UT1Polenumrecs[masterTSFIndex] = numrecsTSF;
#else
                filenotfound = PGS_TRUE;
		return returnStatus;

	      default:
		PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
                filenotfound = PGS_TRUE;
                return PGS_E_TOOLKIT;
            }

            numrecs = 0;           /* initialize record count */
#endif
	    
	    /* Skip past the file header */

	    fseek(fp, PGSd_UTCPOLE_FIRST_TWO, SEEK_SET);
	    
	    /* Read utcpole and load x,y positions and difference of UT1 and
               UTC into arrays in memory until the end of file is encountered */
	    
            fileread = fscanf(fp,"%lf %f %*f %f %*f %f %*f %c",&mjd,&xpoleTemp,
			      &ypoleTemp,&ut1utcTabTemp,&accuracyTemp);
	    
	    /* determine first and last dates in the file */
	    
	    if (fileread != EOF)
	    {

#ifdef _PGS_THREADSAFE

                /*  Threadsafe Protect:   numrecs,   jd0  
                         reassign their values for next use  */

                jd0TSF = (PGSt_double) (mjd + 2400000.5);
                PGSg_TSF_CSCUTC_UT1Polejd0[masterTSFIndex] = jd0TSF;
		fileOffset = ftell(fp);
		fseek(fp, 0, SEEK_END);
		totalFileSize = ftell(fp);
		numrecsTSF = (totalFileSize - PGSd_UTCPOLE_FIRST_TWO)/
		                                      PGSd_UTCPOLE_RECORD;
                PGSg_TSF_CSCUTC_UT1Polenumrecs[masterTSFIndex] = numrecsTSF;
		fseek(fp,PGSd_UTCPOLE_FIRST_TWO, SEEK_SET);
		fseek(fp, (numrecsTSF - 1)*PGSd_UTCPOLE_RECORD, SEEK_CUR);
#else
                jd0 = (PGSt_double) (mjd + 2400000.5);
		fileOffset = ftell(fp);
		fseek(fp, 0, SEEK_END);
		totalFileSize = ftell(fp);
		numrecs = (totalFileSize - PGSd_UTCPOLE_FIRST_TWO)/
		           PGSd_UTCPOLE_RECORD;
		fseek(fp,PGSd_UTCPOLE_FIRST_TWO, SEEK_SET);
		fseek(fp, (numrecs - 1)*PGSd_UTCPOLE_RECORD, SEEK_CUR);
#endif
		fileread = fscanf(fp,"%lf %f %*f %f %*f %f %*f %c",&mjd,
				  &xpoleTemp,&ypoleTemp,&ut1utcTabTemp,
				  &accuracyTemp);
		fseek(fp, fileOffset, SEEK_SET);
		if (fileread == EOF || fileread != 5)
		{
		    PGS_IO_Gen_Close(fp);
		    sprintf(specifics,
			    "error parsing file 'utcpole.dat' (logical ID: %d),"
			    " UT1-UTC and polar motion not available",
			    PGSd_UTCPOLE);
		    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT, specifics,
					  FUNCTION_NAME);
		    return PGS_E_TOOLKIT;
		}
		
#ifdef _PGS_THREADSAFE

                /*  Threadsafe Protect:   jdlast, tablerd, numrecs, jd  
                         reassign their values for next use  */

		jdlastTSF = (PGSt_double) (mjd + 2400000.5);
                  PGSg_TSF_CSCUTC_UT1Polejdlast[masterTSFIndex] = jdlastTSF;

		/* temporarily set the following two variables */

		jdTSF[0] = PGSd_GEO_ERROR_VALUE;
                 PGSg_TSF_CSCUTC_UT1Polejd[masterTSFIndex][0] = PGSd_GEO_ERROR_VALUE;

		numrecsTSF = 1;
                PGSg_TSF_CSCUTC_UT1Polenumrecs[masterTSFIndex] = numrecsTSF;

		tablerdTSF = PGS_TRUE;
                PGSg_TSF_CSCUTC_UT1Poletablerd[masterTSFIndex] = tablerdTSF;
#else
		jdlast = (PGSt_double) (mjd + 2400000.5);

		/* temporarily set the following two variables */

		jd[0] = PGSd_GEO_ERROR_VALUE;
		numrecs = 1;

		tablerd = PGS_TRUE;
#endif
	    }
	    /* close file */
	
	    returnStatus1 = PGS_IO_Gen_Close(fp);
	    if (returnStatus1 != PGS_S_SUCCESS)
	    {
		sprintf(specifics,
			"error closing file 'utcpole.dat' (logical ID: %d),"
			" UT1-UTC and polar motion not available",
			PGSd_UTCPOLE);
		PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT, specifics,
				      FUNCTION_NAME);
		return PGS_E_TOOLKIT;
	    }
	}
    }

    /* If the input Julian day is not within the range of the table, return an
       error. */

#ifdef _PGS_THREADSAFE

    /* Threadsafe Protects:  jdlast, jd0, jd, numrecs, 
               reassigns numrecs value for next use  */
                  
    if ( (jdUTCadd > jdlastTSF) || (jdUTCadd < jd0TSF) )
    {
	PGS_SMF_SetDynamicMsg(PGSCSC_W_JD_OUT_OF_RANGE,
			      "no UT1-UTC or polar motion available for input "
			      "UTC time, assuming UT1-UTC and polar motion "
			      "values are 0", FUNCTION_NAME);
	return PGSCSC_W_JD_OUT_OF_RANGE;
    }
    
    if (jdUTCadd < jdTSF[0] || jdUTCadd > jdTSF[numrecsTSF-1])
    {
	/* we don't necessarily want to read the entire file (it may be too
	   big); determine which part of the file to read into memory */
	
	numrecsTSF = 0;
        PGSg_TSF_CSCUTC_UT1Polenumrecs[masterTSFIndex] = numrecsTSF;

	numSkip = 0;

	if (jdlastTSF < (jdUTCadd + 2*(MAX_RECS/3)))
	{
	    numSkip = jdlastTSF - (jdUTCadd + 2*(MAX_RECS/3));
	}

	numSkip = numSkip + (int) (jdUTCadd - jd0TSF) - MAX_RECS/3;
#else
    if ( (jdUTCadd > jdlast) || (jdUTCadd < jd0) )
    {
	PGS_SMF_SetDynamicMsg(PGSCSC_W_JD_OUT_OF_RANGE,
			      "no UT1-UTC or polar motion available for input "
			      "UTC time, assuming UT1-UTC and polar motion "
			      "values are 0", FUNCTION_NAME);
	return PGSCSC_W_JD_OUT_OF_RANGE;
    }
    
    if (jdUTCadd < jd[0] || jdUTCadd > jd[numrecs-1])
    {
	/* we don't necessarily want to read the entire file (it may be too
	   big); determine which part of the file to read into memory */
	
	numrecs = 0;
	numSkip = 0;

	if (jdlast < (jdUTCadd + 2*(MAX_RECS/3)))
	{
	    numSkip = (long)(jdlast - (jdUTCadd + 2*(MAX_RECS/3)));
	}

	numSkip = numSkip + (int) (jdUTCadd - jd0) - MAX_RECS/3;
#endif
	if (numSkip < 0)
	{
	    numSkip = 0;
	}

	fileOffset = PGSd_UTCPOLE_FIRST_TWO + (numSkip*PGSd_UTCPOLE_RECORD);
	
	returnStatus = PGS_IO_Gen_Open(PGSd_UTCPOLE,PGSd_IO_Gen_Read,&fp,1);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	
	fseek(fp, fileOffset, SEEK_SET);

	fileread = fscanf(fp,"%lf %f %*f %f %*f %f %*f %c",&mjd,&xpoleTemp,
			  &ypoleTemp,&ut1utcTabTemp,&accuracyTemp);
	
#ifdef _PGS_THREADSAFE

        /*Threadsafe Protect: numrecs, jd, xpoles, ypoles, ut1utcTab, accuracy
                                 reassign their values for next use */

	while (fileread != EOF && numrecsTSF < MAX_RECS)   /* WHILE START */
	{	
	    
	    /* convert modified Julian day to Julian day */
	    
	    jdTSF[numrecsTSF]= (PGSt_double) (mjd + 2400000.5); 
            PGSg_TSF_CSCUTC_UT1Polejd[masterTSFIndex][numrecsTSF] = 
                                                          jdTSF[numrecsTSF];

	    /* assign the values read from the utcpole file to the table */
	    
	    xpolesTSF[numrecsTSF]    = xpoleTemp;
	    ypolesTSF[numrecsTSF]    = ypoleTemp;
	    ut1utcTabTSF[numrecsTSF] = ut1utcTabTemp;
            for (initx=0; initx<MAX_RECS; initx++)
            {
                PGSg_TSF_CSCUTC_UT1Polexpoles[masterTSFIndex][initx] = 
                                                            xpolesTSF[initx];
                PGSg_TSF_CSCUTC_UT1Poleypoles[masterTSFIndex][initx] = 
                                                            ypolesTSF[initx];
                PGSg_TSF_CSCUTC_UT1Poleut1utcTab[masterTSFIndex][initx] = 
                                                         ut1utcTabTSF[initx];
            }

	    accuracyTSF[numrecsTSF]  = accuracyTemp;	
            pthread_setspecific(
                     masterTSF->keyArray[PGSd_TSF_KEYCSCUTC_UT1POLEACCURACY],
                     (void *) accuracyTSF);
	    
	    numrecsTSF++;
            PGSg_TSF_CSCUTC_UT1Polenumrecs[masterTSFIndex] = numrecsTSF;
#else    
	while (fileread != EOF && numrecs < MAX_RECS)   /* WHILE START */
	{	
	    
	    /* convert modified Julian day to Julian day */
	    
	    jd[numrecs]= (PGSt_double) (mjd + 2400000.5); 
	    
	    /* assign the values read from the utcpole file to the table */
	    
	    xpoles[numrecs]    = xpoleTemp;
	    ypoles[numrecs]    = ypoleTemp;
	    ut1utcTab[numrecs] = ut1utcTabTemp;
	    accuracy[numrecs]  = accuracyTemp;	
	    
	    numrecs++;
#endif	    
	    fileread = fscanf(fp, "%lf %f %*f %f %*f %f %*f %c", &mjd,
			      &xpoleTemp, &ypoleTemp, &ut1utcTabTemp,
			      &accuracyTemp);
	}   /* WHILE END */
	
	returnStatus1 = PGS_IO_Gen_Close(fp);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus, FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
    }
    
    /****** SORT through arrays to find desired Julian day and pole
            positions/difference of UT1 and UTC ********/

    /* Main part of program */

    /* Search through the table and find the appropriate time.  The variable cnt
       is static, search for the time using the most recent value of cnt.  This
       optimizes for the case that the times input (in successive calls to this
       function) are near one another (seems pretty likely). */

#ifdef _PGS_THREADSAFE

    /*  Threadsafe Protect:  jd, cnt, numrecs,  accuracy.
                                         reassigns cnt values for next use */

    if (jdUTCadd < jdTSF[cntTSF-1])
	{
	while ( jdUTCadd < jdTSF[cntTSF-1] )  cntTSF--;
           PGSg_TSF_CSCUTC_UT1Polecnt[masterTSFIndex] = cntTSF;
	}
    else
	{
	while ( (cntTSF < numrecsTSF) && (jdUTCadd >= jdTSF[cntTSF]) ) cntTSF++;
           PGSg_TSF_CSCUTC_UT1Polecnt[masterTSFIndex] = cntTSF;
	}

    /* if the time is exactly at the last point then do not interpolate 
       as it would attempt to access data out of the table range or 
       if the input julian date is equal to one of the dates in the file
       use the values at that date and do not interpolate */

    if ((cntTSF == numrecsTSF) || (jdUTCadd == jdTSF[cntTSF-1]) )
    { 
	switch (accuracyTSF[cntTSF-1])
#else
    if (jdUTCadd < jd[cnt-1])
	while ( jdUTCadd < jd[cnt-1] ) cnt--; 
    else
	while ( (cnt < numrecs) && (jdUTCadd >= jd[cnt]) ) cnt++; 

    /* if the time is exactly at the last point then do not interpolate 
       as it would attempt to access data out of the table range or 
       if the input julian date is equal to one of the dates in the file
       use the values at that date and do not interpolate */

    if ((cnt == numrecs) || (jdUTCadd == jd[cnt-1]) )
    { 
	switch (accuracy[cnt-1])
#endif
	{
	  case 'p': 
	    returnStatus = PGSCSC_W_PREDICTED_UT1;
	    break;
	  case 'f': break;
	  /* if the accuracy of the data is not one of final, or predicted 
	       then issue an error message and return */
	  default:  
	    returnStatus = PGSCSC_E_INACCURATE_UTCPOLE;
	    sprintf(specifics, "%s%d%s", 
                    "inaccurate UT1 data for modified Julian date ",
		    (int) (jdUTCadd - 2400000.5),
		    " - should be one of 'f' for final,"
		    "  or 'p' for predicted"); 
	    PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	    return returnStatus; 	
	}

#ifdef _PGS_THREADSAFE

        /* Threadsafe Protect: xpoles, ypoles, cnt, ut1utcTab, jd, accuracy */

	*xpole = xpolesTSF[cntTSF-1];
	*ypole = ypolesTSF[cntTSF-1];
	*diffUT1UTC = ut1utcTabTSF[cntTSF-1];
	*jdtable = jdTSF[cntTSF-1];	
    }
    else    /* interpolate linearly between the values at cnt and cnt-1 */
    {
	/* if the data at cnt-1 is not one of final, or predicted 
	   then issue an error message and return */
	switch (accuracyTSF[cntTSF-1])
#else
	*xpole = xpoles[cnt-1];
	*ypole = ypoles[cnt-1];
	*diffUT1UTC = ut1utcTab[cnt-1];
	*jdtable = jd[cnt-1];	
    }
    else    /* interpolate linearly between the values at cnt and cnt-1 */
    {
	/* if the data at cnt-1 is not one of final, or predicted 
	   then issue an error message and return */
	switch (accuracy[cnt-1])
#endif
	{
	  case 'p': 
	  case 'f': break;
	  default: 
	    returnStatus = PGSCSC_E_INACCURATE_UTCPOLE;
	    sprintf(specifics, "%s%d%s", 
                    "inaccurate UT1 data for modified Julian date ",
		    (int) (jdUTCadd - 2400000.5),
		    " - should be one of 'f' for final,"
		    "  or 'p' for predicted"); 
	    PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	    return returnStatus; 	
	}

#ifdef _PGS_THREADSAFE

        /*  Threadsafe Protect:  cnt, accuracy  */

	switch (accuracyTSF[cntTSF])
#else
	switch (accuracy[cnt])
#endif
	{
	  case 'p': 
	    returnStatus = PGSCSC_W_PREDICTED_UT1;
	    break;

	  case 'f': break;
	    /* if the data at i is not one of final,predicted or interim
	       then issue an error message and return */
	  default: 
	    returnStatus = PGSCSC_E_INACCURATE_UTCPOLE;
	    sprintf(specifics, "%s%d%s", 
                    "inaccurate UT1 data for modified Julian date ",
		    (int) (jdUTCadd - 2400000.5),
		    " - should be one of 'f' for final,"
		    "  or 'p' for predicted"); 
	    PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
	    return returnStatus; 	
	}

	/* interpolate to find the x pole and y pole position */  

#ifdef _PGS_THREADSAFE

        /*  Threadsafe Protect:  xpoles, ypoles, cnt, ut1utcTab, jd */

	*xpole = xpolesTSF[cntTSF-1] + ((jdUTCadd - jdTSF[cntTSF-1])/
                                    (jdTSF[cntTSF] - jdTSF[cntTSF-1])*
				  (xpolesTSF[cntTSF] - xpolesTSF[cntTSF-1]));

	*ypole = ypolesTSF[cntTSF-1] + ((jdUTCadd - jdTSF[cntTSF-1])/
                                      (jdTSF[cntTSF] - jdTSF[cntTSF-1])* 
				  (ypolesTSF[cntTSF] - ypolesTSF[cntTSF-1]));

	/* if the change to UT1 - UTC exceeds 0.65 then the next leap
	   second has occurred and UTC has been set back. Since the actual
	   UTC time has not reached that place, the change to the 
	   difference of UT1 and UTC by one second must be temporarily
	   reversed */ 

	if ( (ut1utcTabTSF[cntTSF] - ut1utcTabTSF[cntTSF-1]) >  0.65F )
	    *diffUT1UTC = ut1utcTabTSF[cntTSF-1] + 
                ((jdUTCadd - jdTSF[cntTSF-1])/(jdTSF[cntTSF] - jdTSF[cntTSF-1])*
	                 (ut1utcTabTSF[cntTSF] - 1 - ut1utcTabTSF[cntTSF-1]));
	else 
	    *diffUT1UTC = ut1utcTabTSF[cntTSF-1] + 
                ((jdUTCadd - jdTSF[cntTSF-1])/(jdTSF[cntTSF] - jdTSF[cntTSF-1])* 
	                  (ut1utcTabTSF[cntTSF] - ut1utcTabTSF[cntTSF-1])); 
	*jdtable = jdTSF[cntTSF-1];
#else
	*xpole = xpoles[cnt-1] + ((jdUTCadd - jd[cnt-1])/(jd[cnt] - jd[cnt-1])*
				  (xpoles[cnt] - xpoles[cnt-1]));

	*ypole = ypoles[cnt-1] + ((jdUTCadd - jd[cnt-1])/(jd[cnt] - jd[cnt-1])*
				  (ypoles[cnt] - ypoles[cnt-1]));

	/* if the change to UT1 - UTC exceeds 0.65 then the next leap
	   second has occurred and UTC has been set back. Since the actual
	   UTC time has not reached that place, the change to the 
	   difference of UT1 and UTC by one second must be temporarily
	   reversed */ 

	if ( (ut1utcTab[cnt] - ut1utcTab[cnt-1]) >  0.65F )
	    *diffUT1UTC = ut1utcTab[cnt-1] + 
                          ((jdUTCadd - jd[cnt-1])/(jd[cnt] - jd[cnt-1])*
	                  (ut1utcTab[cnt] - 1 - ut1utcTab[cnt-1]));
	else 
	    *diffUT1UTC = ut1utcTab[cnt-1] + 
                          ((jdUTCadd - jd[cnt-1])/(jd[cnt] - jd[cnt-1])* 
	                   (ut1utcTab[cnt] - ut1utcTab[cnt-1])); 
	*jdtable = jd[cnt-1];
#endif
    }   /* END: if ((cnt == numrecs) || (jdUTCadd == jd[cnt-1]) ) */

    PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME); 
    return returnStatus;
}
