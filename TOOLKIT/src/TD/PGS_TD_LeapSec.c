/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME: 
      PGS_TD_LeapSec.c
 
DESCRIPTION:
      This file contains the tool PGS_TD_LeapSec(), which extracts the
      leap second value from file leapsec.dat and returns an error
      status. 

AUTHOR:
      Deborah Foch          / Applied Research Corporation
      Peter D. Noerdlinger  / Applied Research Corporation  
      Guru Tej S. Khalsa    / Applied Research Corporation
    
HISTORY:
      10-Feb-1994   DF/PDN  Designed
      24-Feb-1994   DF      Created
      25-Feb-1994   DF      Modified to correct errors             
      28-Feb-1994   DF      Modified to improve efficiency/logic   
      01-Mar-1994   PDN     Modified to streamline I/O list and
                            to warn on need to input UTC   
      02-Mar-1994   PDN     Modified to search table only when 
                            time is within its range      
      02-Mar-1994   PDN     Modified to utilize first and last rows
      09-Mar-1994   PDN     Modified to use more static variables
      01-Apr-1994   UP      Modified prolog
      22-Jul-1994   DF      Modified prolog
      25-Jul-1994   DF      Added functions to compute leapsecs 
                            between 1961 and 1968.  Added linear
                            function to compute leapsecs without
                            the table (rough estimate)
      26-Jul-1994   DF      Modified table read section to read
                            the constants to compute leap seconds
		      	    between 1961 and 1968.
      03-Aug-1994   DF      Corrected errors
      05-Aug-1994   DF      Changed table read algorithm for
                            improved efficiency
      17-Aug-1994   DF      Completed changes determined from code inspection
      19-Aug-1994   DF      Changed <= to < in line 349 to change Max UTC
      23-Sep-1994   GTSK    Modified to use PGS_IO_ routines to open and
                            close data file.
      23-Sep-1994   GTSK    Altered leap second search algorithm to optimize for
                            successive calls with similar times.
      28-Apr-1994   GTSK    Changed types of some values read from the file
                            leapsec.dat to be float rather than PGSt_double to
			    speed up processing and reduce storage requirements.
      01-Jun-1994   GTSK    Changed input Julian day from a single number to
                            an array of two numbers.
      30-Jun-1995   PDN     Fixed to skip new header line in data file
                            and fixed examples
      08-Jan-1997   PDN     Added a "break" statement for the cases of radical
                            failure of PGS_IO_Gen_Open - such as a missing file
      21-May-1997   PDN     Fixed Comments
      21-Oct-1999   PDN     Changed method of reading from fscanf to fgets followed
                            by sscanf.  Added more "sanity checks". New code avoids
                            going into a loop or core dumping with most plausible
                            kinds of corrupt leap seconds files. Also added better
                            messaging in case of failure
      27-Oct-1999   PDN     Changed MAX_HEADER from 600 to 110. This is fully ample
                            for normal length lines, which should not exceed 91 
                            characters, although obsolete files with PREDICTED
                            in the right hand column could reach 94 characters.

  
END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:
  
TITLE:
      Get Leap Second 
 
NAME:
      PGS_TD_LeapSec()
   
SYNOPSIS:
   
      #include <PGS_TD.h> 
   
      PGSt_SMF_status
      PGS_TD_LeapSec(
          PGSt_double   jdUTC[2],                         
          PGSt_double   *leapSec,
	  PGSt_double   *lastChangeJD,
	  PGSt_double   *nextChangeJD,
	  char          *leapStatus)                       

DESCRIPTION:	 
      This tool accesses the file 'leapsec.dat', extracts the leap second 
      value for an input Julian Day number, and returns an error status.

INPUTS:

      NAME            DESCRIPTION            UNITS        MIN         MAX
      ----            ----------------       -----        ---         ---
      jdUTC           input Julian Day       days         N/A         N/A
                      number (UTC)                          (see NOTES)
		                              
OUTPUTS:
      NAME            DESCRIPTION            UNITS        MIN         MAX
      ----            -----------            -----        ---         ---
      leapSec         leap second value      seconds       0          N/A
                      for day jdUTC,
                      read from table 
		      'leapsec.dat'                       
     
      lastChangeJD    Julian Day number      days         N/A         N/A
		      upon which that                       (see NOTES)
                      leap second value  
		      was effective                      

      nextChangeJD    Julian Day number      days         N/A         N/A
		      of the next ACTUAL                    (see NOTES)
                      or PREDICTED leap  
		      second
	                                                                  
      leapStatus      indicates whether      N/A          N/A         N/A
		      the leap second
                      value is ACTUAL, 
		      PREDICTED, a LINEARFIT,
		      or ZEROLEAPS (leap
		      second value is set to 
		      zero if the input time 
	              is before the start
                      of the table)


RETURNS:
      PGS_S_SUCCESS                successful execution
      PGSTD_W_JD_OUT_OF_RANGE      invalid input Julian Day number
      PGSTD_W_DATA_FILE_MISSING    leap second file not found
      PGSTD_E_NO_LEAP_SECS         defective leap second file


EXAMPLES:
      PGSt_double        jdUTC[2];
      PGSt_double        leapsecond;
      PGSt_double        lastChangeJD;
      PGSt_double        nextChangeJD;

      PGSt_SMF_status    returnStatus;

      char           leapStatus[10];

      jdUTC[0] = 2439999.5;
      jdUTC[1] = 0.5;
      returnStatus = PGS_TD_LeapSec(jdUTC,&leapsecond,&lastChangeJD,
                                    &nextChangeJD,leapStatus);

      if (returnStatus != PGS_S_SUCCESS)
      {
              **  handle errors  **
      }

NOTES:
   TIME ACRONYMS:
     
      UTC: Coordinated Universal Time
      TAI: International Atomic Time


   TIME BOUNDARIES for TRANSFORMATIONS BETWEEN UTC and TAI:

     The minimum and maximum times that can successfully be processed by 
     this function depend on the file "leapsec.dat" which relates leap 
     second (TAI-UTC) values to UTC Julian dates. The file "leapsec.dat" 
     contains dates of new leap seconds and the total leap seconds for 
     times on and after Jan 1, 1972.  For times between Jan 1, 1961 and 
     Jan 1, 1972 it contains coefficients for an approximation supplied 
     by the International Earth Rotation Service (IERS) and the United 
     States Naval Observatory (USNO).  The Toolkit then uses these 
     coefficients in an algorithm consistent with IERS/USNO usage. For 
     times after Jan 1, 1961, but before the last date in the file, the
     Toolkit sets TAI-UTC equal to the total number of leap seconds to 
     date, (or to the USNO/IERS approximation, for dates before Jan 1,
     1972). If an input date is before Jan 1, 1961 the Toolkit sets the
     leap seconds value to 0.0.  This is consistent with the fact that,
     for civil timekeeping since 1972, UTC replaces Greenwich Mean Solar 
     Time (GMT), which had no leap seconds. Thus for times before Jan 1, 
     1961, the user can, for most Toolkit-related purposes, encode 
     Greenwich Mean Solar Time as if it were UTC.  If an input date
     is after the last date in the file, or after Jan 1, 1961, but the 
     file cannot be read, the function will use a calculated value of 
     TAI-UTC based on a linear fit of the data known to be in the table
     as of early 1997.  This value is a crude estimate and may be off by 
     as much as 1.0 or more seconds.  If the data file, "leapsec.dat", 
     cannot be opened, or the time is outside the range from Jan 1, 1961 
     to the last date in the file, the return status level will be 'E'.  
     Even when the status level is 'E', processing will continue, using 
     the default value of TAI-UTC (0.0 for times before Jan 1, 1961, or 
     the linear fit for later times). Thus, the user should always 
     carefully check the return status.  

     The file "leapsec.dat" is updated when a new leap second is 
     announced by the IERS, which has been, historically, about once 
     every year or two. 


   JULIAN DATES:

     The Julian date is a day count originating at noon of Jan. 1st, 4713 B.C.

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
       input and do NOT return a Julian date will first convert (internally) the
       input date to the above format if necessary.  Toolkit functions that have
       a Julian date as both an input and an output will assume the input is in
       the above described format but will not check and the format of the
       output may not be what is expected if any other format is used for the
       input.

     Meaning:

       Toolkit "Julian dates" are all derived from UTC Julian Dates.  A Julian
       date in any other time stream (e.g. TAI, TDT, UT1, etc.) is the UTC
       Julian date plus the known difference of the other stream from UTC
       (differences range in magnitude from 0 seconds to about a minute).

       Examples:

         In the following examples, all Julian Dates are expressed in Toolkit
         standard form as two double precision numbers. For display here, the
         two members of the array are enclosed in braces {} and separated by a
         comma.

         A) UTC to TAI Julian dates conversion

         The Toolkit UTC Julian date for 1994-02-01T12:00:00 is: 
         {2449384.50, 0.5}.
         TAI-UTC at 1994-02-01T12:00:00 is 28 seconds (.00032407407407 days). 
         The Toolkit TAI Julian date for 1994-02-01T12:00:00 is:
         {2449384.50, 0.5 + .00032407407407} = {2449384.50, 0.50032407407407}

         Note that the Julian day numbers in UTC and the target time stream may
         be different by + or - 1 for times near midnight:

         B) UTC to UT1 Julian dates conversion

         The Toolkit UTC Julian date for 1994-04-10T00:00:00 is: 
         {2449452.50, 0.0}.
         UT1-UTC at 1994-04-10T00:00:00 is -.04296 seconds 
         (-0.00000049722221 days).  The Toolkit UT1 Julian date for
         1994-04-10T00:00:00 is:
         {2449452.50, 0.0 - 0.0000004972222} = 
         {2449452.50, -0.0000004972222} =
         {2449451.50, 0.9999995027778}

   MODIFIED JULIAN DATES:

     Modified Julian dates follow the same conventions as those for Julian
     dates (above) EXCEPT that the modified Julian day number is integral (NOT
     half-integral).  The modified Julian date in any time stream has a day 
     number that is 2400000.5 days less than the Julian date day number and an
     identical day fraction (i.e. the modified Julian date is a day count
     originating at 1858-11-17T00:00:00).


REQUIREMENTS:
      PGSTK - 1050, 0930 

DETAILS:
     Input times which are not included in the leap second file, are handled by
     setting the leap second value equal to 0 if the input time is before the
     start of the leap second file, and by a linear fit of the whole number leap
     seconds in the table as of July 1994 for input times after the end date of
     the table.  If the leap second file is not found, any input time will be
     accepted, and the leap second value estimated based on the appropriate
     linear fit which is determined by whether the input time is before or after
     Jan 1, 1972.

     The equation used for the linear fit of the table values between 1972 and
     2007 (and beyond) is the linear fit of the steps of leap second value
     changes (between 1972 and 2007) minus 0.5 (to put the line through the
     average values for each time interval between leap second increases).  The
     equation was updated last on July 27, 1994.

GLOBALS:
      NONE

FILES:
      This tool accesses the file leapsec.dat.  If this file is not
      available a computed value is returned (see NOTES).
     
FUNCTIONS_CALLED:
      PGS_SMF_SetDynamicMsg()      set error/status message
        

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_TD.h>
#include <PGS_EPH.h>
#include <PGS_IO.h>

#define MAX_HEADER  110
#define ARRAYSIZE   200
#define PREDCHAR    10
#define MONTHCHAR   4
#define JD1961JAN1  2437300.5
#define JD1972JAN1  2441317.5
#define MAX_PERIOD  83.0       /* maximum number of days after the last UPDATE
				  of the leap seconds file that the last leap
				  second entry in the file will be assumed to be
				  valid for (after which an error will be
				  returned indicating that the leap second file
				  is out of date), the number 83 corresponds to
				  roughly 3 months less 7 days */

/* logical identifier for leap seconds file leapsec.dat (defined in Process
   Control File) */
 
#define LEAPSEC 10301
 
/* name of this function */

#define FUNCTION_NAME "PGS_TD_LeapSec()"

PGSt_SMF_status 
PGS_TD_LeapSec(                                /* Get Leap Second */
    PGSt_double jdUTC[2],                      /* UTC Julian Date */
    PGSt_double *leapSec,                      /* requested leap second value */
    PGSt_double *lastChangeJD,                 /* Julian Day number on which
						  output leap second was
						  effective */
    PGSt_double *nextChangeJD,                 /* Julian Day number on which
						  next leap second change
						  occurred (or is predicted to
						  occur) */
    char        leapStatus[PREDCHAR])          /* indicates whether the leap sec
						  value is actual, predicted or
						  computed using a linear fit */
{	   
    PGSt_double        jdUTCadd;               /* input UTC Julian Date as a 
						  single number:
						  jdUTC[0] + jdUTC[1] */
    PGSt_double        updateJD[2];            /* Julian date of last update
						  time of leap seconds file */
    static  double     jd[ARRAYSIZE];          /* array of Julian Day numbers in
						  the leap second table */
    static  float      leapseconds[ARRAYSIZE]; /* array of leap second values in
						  leap second table */
    static  float      slope[ARRAYSIZE];       /* array of slopes used to
						  compute leap seconds values
						  for 1961 to 1971 */
    static  float      baseDay[ARRAYSIZE];     /* array of constants to subtract
						  from MJD in equation used to
						  compute leap seconds */  
    static PGSt_double jdfirst;                /* first Julian Day number in
						  table */
    static PGSt_double jdlast;                 /* last Julian Day number for
						  which a valid leap second
						  value is defined */
    static char predict[ARRAYSIZE][PREDCHAR];  /* character array indicating
						  whether the leap second values
						  are PREDICTED, ACTUAL,  set to
						  ZEROLEAPS for a 0 leap second 
						  value, or computed with a
						  LINEARFIT equation*/ 
    static PGSt_uinteger numrecs;              /* leap second file/table record
						  number or array member counter
						  (minus one) */
    static PGSt_boolean tableread = PGS_FALSE; /* indicates whether table of
						  leap second values has been
						  read into memory */
    static PGSt_boolean filenotfound = PGS_FALSE; /* indicates whether an
						     attempt to read a file
						     which cannot be found has
						     already been made */
    
    static PGSt_integer recordread;            /* sscanf return value */ 
                      
    static PGSt_uinteger cntr = 1;             /* array member counter,
						  initialized to 0 */    
    FILE *fp;                                  /* pointer to current position in
						  file */
    PGSt_SMF_status returnStatus;              /* return status of this
						  function */
    PGSt_SMF_status returnStatus1;             /* return status of PGS function
						  calls */
    PGSt_SMF_status code;	    	       /* status code returned by
						  PGS_SMF_GetMsg() */

    char*           asciiUTC;                  /* UTC in CCSDS ASCII format */

    char            headerl[MAX_HEADER];   /* used for reading in header lines */
 
    char	    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg() */
    char	    msg[PGS_SMF_MAX_MSG_SIZE];		 /* message returned by
							    PGS_SMF_GetMsg() */
    char	    specifics[PGS_SMF_MAX_MSG_SIZE];	 /* message returned by
							    PGS_SMF_GetMsg() */
    static char	    noFileMsg[PGS_SMF_MAX_MSG_SIZE];	 /* message returned by
							    PGS_IO_Gen_Open() */
    static char	    bareFileMsg[PGS_SMF_MAX_MSG_SIZE];	 /* message returned by
							    PGS_IO_Gen_Open() */
    /* begin executable program */

    /* initialize returnStatus to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* input is UTC Julian date as two numbers, convert to one number */

    jdUTCadd = jdUTC[0] + jdUTC[1];

    /* If the Julian day fraction is greater than 0.99, subtract 0.000001 to
       avoid computer round-off errors to the next Julian day */
    
    if (fmod(jdUTC[1],1.) > .99)
	jdUTCadd -= 1.0E-6;
    
    *leapSec = 0;
    *lastChangeJD = jdUTCadd;
    *nextChangeJD = jdUTCadd;
    leapStatus[0] = '\0';
    
    
    /* If leap second file containing the table of values has already been read 
       into memory, skip to SEARCH section */
    
    if(tableread == PGS_FALSE)
    {
	if(filenotfound == PGS_FALSE)
	{
	    /* Open Leap Second file */
	    
	    returnStatus = PGS_IO_Gen_Open(LEAPSEC,PGSd_IO_Gen_Read,&fp,1);
	    switch (returnStatus)
	    {
	      case PGS_S_SUCCESS:
		break;
	      case PGS_E_UNIX:
	      case PGSIO_E_GEN_OPENMODE:
	      case PGSIO_E_GEN_FILE_NOEXIST:
	      case PGSIO_E_GEN_REFERENCE_FAILURE:
	      case PGSIO_E_GEN_BAD_ENVIRONMENT:
		PGS_SMF_GetMsg(&code,mnemonic,bareFileMsg);
		if (code != returnStatus)
		  PGS_SMF_GetMsgByCode(returnStatus,bareFileMsg);
		filenotfound = PGS_TRUE;
                break;
	      default:
		PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
		return PGS_E_TOOLKIT;
	    }
	}
	    
	if(filenotfound == PGS_TRUE) /** FILE MISSING ** IF START */
	{
	    returnStatus = PGSTD_W_DATA_FILE_MISSING;
            sprintf(specifics,"\nFor UTC Julian Date %f "
                              ,jdUTC[0]+jdUTC[1]);
            strcpy(noFileMsg,bareFileMsg);
            strcat(noFileMsg,specifics);
	    PGS_SMF_SetDynamicMsg(returnStatus,noFileMsg,FUNCTION_NAME);
	    
	    /* leap second value defaults to initialized value of zero if input
	       date is before 1961 -- otherwise, the appropriate linear
	       approximation is used depending on whether the jdUTCadd is
	       between Jan 1, 1961 and 12/31/71, or after 12/31/71.*/
	    
	    /* do linear fits */
	    
	    if(jdUTCadd >= JD1961JAN1)  /** LINEAR FIT ** IF START */
	    {
		if( jdUTCadd < JD1972JAN1)
		  *leapSec = (jdUTCadd - JD1961JAN1)*10.0/(JD1972JAN1 -
							   JD1961JAN1);
		else
		  *leapSec = (2.21908779093e-3)*jdUTCadd - 5406.04201391 - 0.5;
		
		strncpy(leapStatus,"LINEARFIT",PREDCHAR);
		return returnStatus;
	    }  
	    else /* otherwise, set the leap seconds value to zero */
	    {
		*leapSec = 0; 
		strncpy(leapStatus,"ZEROLEAPS",PREDCHAR);
		return returnStatus;
		
	    }                        /** LINEAR FIT ** IF END*/
	}                            /** FILE MISSING ** IF END */
	
	/* Read leap second file and load into arrays in memory until the end of
	   file is encountered, skipping unwanted characters */
	
	/* initialize record counter to zero */
	
	numrecs = 0; 
	
	/*  Read header line - ignore (recycle the msg variable which is no
	    longer needed at this point) */

        fgets(headerl,MAX_HEADER,fp);

	asciiUTC = PGS_EPH_getToken(headerl, ":");
	asciiUTC = PGS_EPH_getToken(NULL, " ");
	if (asciiUTC == NULL)
	{
	    /* if asciiUTC could not be successfully determined, set jdlast to
	       indicate an error condition */

	    jdlast = PGSd_GEO_ERROR_VALUE;
	}

	returnStatus = PGS_TD_UTCtoUTCjd(asciiUTC, updateJD);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_M_LEAP_SEC_IGNORED:

	    /* set jdlast to MAX_PERIOD days greater then the time of the
	       last update of the leap seconds file, the idea is that the
	       leap seconds file should be checked (and updated if necessary)
	       with a minimum frequency of MAX_PERIOD days) */

	    jdlast = updateJD[0] + MAX_PERIOD;
	    break;
	    
	  default:

	    /* if asciiUTC could not be successfully determined, set jdlast to
	       indicate an error condition */
	    
	    jdlast = PGSd_GEO_ERROR_VALUE;
	}
	
       /*  Read first data line */

       fgets(headerl,MAX_HEADER,fp);
 
        if(strlen(headerl) < 91)
        {
           returnStatus = PGSTD_E_NO_LEAP_SECS;
           sprintf(specifics,"short 1st data line in LeapSeconds file \n");
           PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
           PGS_IO_Gen_Close(fp);
           return returnStatus;
        }
       recordread = sscanf(headerl, "%*d %*s %*d %*s %lf %*s %f %*s %*s %*s %*s"
                           "%f%*s %*s %f %*s %9s", &jd[numrecs],
                           &leapseconds[numrecs],&baseDay[numrecs],
                           &slope[numrecs],predict[numrecs]);
        if(recordread < 5)
        {
           returnStatus = PGSTD_E_NO_LEAP_SECS;
           sprintf(specifics,"insufficient fields in LeapSeconds file at 1st data line\n");
           PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
           PGS_IO_Gen_Close(fp);
           return returnStatus;
        }

	    /*  Read remaining data lines */

	    /* The following && clause is to handle blank records at end 
	       of leapsec.dat - if any (\n's) */
	    while (fgets(headerl,MAX_HEADER,fp) != NULL &&
		  (int) strlen(headerl) > 20 )   
	    {
		numrecs++;
                if(strlen(headerl) < 91)
                {
                   returnStatus = PGSTD_E_NO_LEAP_SECS;
                   sprintf(specifics,"short LeapSeconds file data line No. %d \n",numrecs+1);
                   PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
                   PGS_IO_Gen_Close(fp);
                   return returnStatus;
                }
		if (numrecs == ARRAYSIZE)
		{
		    returnStatus = PGS_E_TOOLKIT;
		    sprintf(specifics,"%s %d %s","leap seconds file exceeds ",
				    ARRAYSIZE, "records ");
		    PGS_SMF_SetDynamicMsg(returnStatus,
					       specifics,
					       FUNCTION_NAME);
                    PGS_IO_Gen_Close(fp);
		    return returnStatus;
		}

		recordread = sscanf(headerl, 
                                    "%*d %*s %*d %*s %lf %*s %f %*s %*s %*s"
				    "%*s %f%*s %*s %f %*s %9s", &jd[numrecs],
				     &leapseconds[numrecs], &baseDay[numrecs],
				     &slope[numrecs],predict[numrecs]);
       if(recordread < 5)
       {
          returnStatus = PGSTD_E_NO_LEAP_SECS;
          sprintf(specifics,"insufficient fields in LeapSeconds file at data line %d\n",
                  numrecs+1);
          PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);
          PGS_IO_Gen_Close(fp);
          return returnStatus;
       }
    }  /* end while */ 
/* here we increment numrecs one more time to show that the last "recordread = sscanf"
   was fulfilled */
		numrecs++;
	
	jdfirst = jd[0];         /* first Julian Day number in the table */

	if ( (jdlast == PGSd_GEO_ERROR_VALUE)  ||
	     (updateJD[0] < jd[numrecs-1]) )
	{
	    /* if the last time in the leap seconds file is > than the time of
	       the last file update (which will be the case when a new leap
	       second occurrence has been announced but the date has not yet
	       passed) OR if an error occurred determining jdlast (see above)
	       then base jdlast on the time last in the leap seconds file */

	    jdlast = jd[numrecs-1] + MAX_PERIOD;
	}
	
	/* close file */
	
	returnStatus1 = PGS_IO_Gen_Close(fp);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code != returnStatus1)
		PGS_SMF_GetMsgByCode(returnStatus1,msg);
	    returnStatus = PGS_E_TOOLKIT;
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,FUNCTION_NAME);
	    return returnStatus;
	}
	
	/* Set table read flag to indicate table has been read into memory */
	
	tableread = PGS_TRUE;
    } /* END: if (tablereadd == PGS_FALSE) */
    
    /* SEARCH through arrays to find desired Julian Day number and leap
       seconds value */
    
    /* If the input Julian day is within the range of the table -- output it. */
    
    if ( (jdUTCadd < jdlast ) && (jdUTCadd >= jdfirst) )
	
    {                                    /** TABLE SEARCH ** IF START */

	/* Search through the table and find the appropriate time.  The variable
	   cntr is static, search for the time using the most recent value of
	   cntr.  This optimizes for the case that the times input (in
	   successive calls to this function) are near one another (seems pretty
	   likely). */

	if (jdUTCadd < (PGSt_double) jd[cntr-1])
	    while ( jdUTCadd < (PGSt_double) jd[cntr-1] ) cntr--; 
	else
	    while ( (cntr < numrecs) && (jdUTCadd >= (PGSt_double) jd[cntr]) )
		cntr++; 

	if (slope[cntr-1] == 0.0F)
	    *leapSec = (PGSt_double) leapseconds[cntr-1];
	else
	    *leapSec = (PGSt_double) (leapseconds[cntr-1] + slope[cntr-1]*
				      (jdUTCadd - 2400000.5 - baseDay[cntr-1]));
	*lastChangeJD = (PGSt_double) jd[cntr-1];
	if (cntr < numrecs)
	    *nextChangeJD = (PGSt_double) jd[cntr];
	else
	    *nextChangeJD = jdlast;
	
	strncpy(leapStatus,predict[cntr-1],PREDCHAR);
    }                                    /** TABLE SEARCH **  IF END  */
    else
    {                            /** LEAP SECONDS ESTIMATE ** IF START */
	
	/* If input Julian Day number is not within the  table, estimate the
	   value of the leap seconds using a linear fit equation */
	
	if( jdUTCadd < jdfirst)                     
	{
	    *leapSec = 0;
	    strncpy(leapStatus,"ZEROLEAPS",PREDCHAR);
	    
	    returnStatus = PGSTD_W_JD_OUT_OF_RANGE;
            sprintf(specifics,"input time (at UTC Julian Date %f) is before"
                              " start of table --\n a leap seconds value"
                              " of 0 is being returned",jdUTC[0]+jdUTC[1]);
            PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);

	}                                       
	else      
	{
	    *leapSec = (2.21908779093e-3)*jdUTCadd - 5406.04201391 - 0.5;
	    strncpy(leapStatus,"LINEARFIT",PREDCHAR);
	    
	    returnStatus = PGSTD_W_JD_OUT_OF_RANGE;
            sprintf(specifics,"input time (at UTC Julian Date %f) is past"
                              " end of table --\n a linear approximation was"
                              " used to determine the leap seconds value",
                                jdUTC[0]+jdUTC[1]);
            PGS_SMF_SetDynamicMsg(returnStatus,specifics,FUNCTION_NAME);

	}
    }                           /** LEAP SECONDS ESTIMATE **  IF END  */
    
    return returnStatus;
} 
