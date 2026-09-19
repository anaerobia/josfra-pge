/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
     
FILENAME:
     PGS_CBP_EphemRead.c

DESCRIPTION:
     This file contains following functions:
          PGS_CBP_EphemRead() which reads planetary ephemeris data provided by 
                              JPL for the given input time. 
          PGS_CBP_Pleph()     which returns position and velocity 
          PGS_CBP_State()     which reads and interpolates the ephemeris data
          PGS_CBP_Interp()    which differentiates and interpolates a set of
                              Chebyshev coefficients
          PGS_CBP_Split()     which breaks a double number into a double 
                              integer and double fractional parts.

AUTHOR:
     Snehavadan Macwan / Applied Research Corporation
     Guru Tej S. Khalsa / Applied Research Corporation
     Peter D. Noerdlinger / Applied Research Corporation
     Curt  Schafer        / Steven Myers & Associates

HISTORY:
     05-Jul-1994 PDN  Designed tool; obtained the ephemeris & documentation
     08-Jul-1994 SM   Initial Version
     28-Apr-1995 GTSK Altered PGS_CBP_EphemRead() to only open DE200 on first
                      call, and then just read the file header into a static
                      array and close the file again.  The file is now only
                      opened (and closed) in the function PGS_CBP_State().  
		      In PGS_CBP_State() a record is read into a static stucture
		      as well.  This record is saved in memory until a new
		      record is required (a single record is good for 30 days).
		      Also changed all global variables defined above to be
		      static (local to this file only).
     09-Apr-1997 PDN  Fixed comments to properly identify epochs and time trans-
                      formations.
     06-Jul-1999 CS   Updated code for Threadsafe version.

END_FILE_PROLOG:
*******************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <PGS_math.h>

#include <PGS_CBP.h>
#include <PGS_IO.h>
#include <PGS_TSF.h>

#define  MAXSIZE        200                     /* max number of ephemeris
						   constants */
#define  AU	        0.1495978706600000E+12	/* Astronomical unit in meter */

#define  RECSIZE        826                     /* size of ephemeris record */

#define  SECONDSperDAY  86400.0                 /* seconds per day */

/* logical identifier for JPL Celestial body ephemeris file (defined in the
   Process Control File) */
 
#define DE200 10601
 
static struct struct_type	       	 /* Structure for title record */
{
    PGSt_uinteger 	number;			/* Record number */
    char		ttl[3][84];		/* Title lines */
    char		cnam[MAXSIZE][8];	/* Names of the constants */
    PGSt_double		ss[3];			/* Start, end and record span */
    PGSt_integer	ncon;			/* Number of constants */
    PGSt_double		au;			/* Astronomical unit */
    PGSt_double		emrat;                  /* Earth Moon ratio */
    PGSt_integer	ipt[36];		/* Pointers needed by INTERP */
    PGSt_integer	numde;                  /* Planetary ephemeris number */
    PGSt_integer	lpt[3];			/* Pointers needed by INTERP */

}headerRec;
    
static struct cvalType	                /* Structure for constants record */
{
    PGSt_uinteger	number;			/* Record number */
    PGSt_double		cval[MAXSIZE];		/* Values of the ephemeris 
                                                   constants */

}cvalRec;

static struct	dataType	        /* Structure for ephemeris records */
{
    PGSt_uinteger	number;			/* Record number */
    PGSt_double		db[RECSIZE];		/* Ephemeris data */

}dataRec;


static struct	stateType		/* Structure for state function */
{
    PGSt_boolean	km;			/* logical flag defining 
                                                   physical units of the output;
                                                   TRUE - km and km/sec, 
						    FALSE - AU and AU/DAY */
    PGSt_boolean	barry;			/* Logical flag defining output
                                                   center; 
						   TRUE - center is solar-system
						   barycenter, 
						   FALSE - center is Sun */

    PGSt_double		pvSun[6];		/* Array containing the 
                                                   barycentric position and 
						   velocity of the Sun. */


}stateRec;


static  PGSt_integer	ipv;            /* interpolation flag for position and 
                                           velocity: 
                                           1 - interpolate position,
                                           2 - interpolate both position and 
                                               velocity */
                                           
static	PGSt_IO_Gen_FileHandle  *fp;    /* file pointer */

static	int		c_2 = 2;        /* needed for the interpolation */
static	int		c_3 = 3;        /* needed for the interpoaltion */

#ifdef _PGS_THREADSAFE
        PGSt_double     pvSunTSF[6];    /* needed for Threadsafe */
#endif


/** Function prototypes */
PGSt_SMF_status 
PGS_CBP_EphemRead(		
      PGSt_double [2],	
      PGSt_integer, 	
      PGSt_integer,      
      PGSt_double [],    	        
      PGSt_double [][3]);	
      
PGSt_SMF_status 
PGS_CBP_Split(		
	PGSt_double *, 		
        PGSt_double *);
        
PGSt_SMF_status        
PGS_CBP_Pleph(				
     PGSt_double [2],	
     PGSt_integer *,	 
     PGSt_integer *,	
     PGSt_double  *);		
	
PGSt_SMF_status 
PGS_CBP_Interp(			
      PGSt_integer,        
      PGSt_double  *,        
      PGSt_integer *,       
      PGSt_integer *,      
      PGSt_integer *,	      		
      PGSt_integer *,     
      PGSt_double  *);      

PGSt_SMF_status 
PGS_CBP_State(	
      PGSt_double *, 		
      PGSt_integer *, 		
      PGSt_double *, 		
      PGSt_double *);		
      	
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
    Reads planetary ephemeris data 

      
NAME:
    PGS_CBP_EphemRead()

SYNOPSIS:
    #include <PGS_CBP.h>
    
DESCRIPTION:
    This function reads planetary ephemeris data provided by JPL for the given
    input time.

INPUTS:

     Name	       Description           Units      Min           Max
     ----              -----------           -----      ---           ---
    jedTDB[2]          time tags             jdTDB      2436144.50   2466543.50
    cbId               Constatnt identifier   N/A       N/A           N/A
                       for celestial body 
                       ( see table below )
                       
    offsets    	       Array of offsets      seconds    (   see     NOTES)   
	               of each input 
	               UTC time 
    numValues	       number of             N/A	  0		any
		       required data	 
		       points
                       0 - only asciiUTC is
	               used
                       any - any time events are 
	                   used


         THE DESIGNATION OF THE ASTRONOMICAL BODIES BY 
         CELESTIAL BODY IDENTIFIER ( cbId ) IS:

                             cbId =

                1 = MERCURY           8 = NEPTUNE
                2 = VENUS             9 = PLUTO
                3 = EARTH            10 = MOON
                4 = MARS             11 = SUN
                5 = JUPITER          12 = SOLAR-SYSTEM BARYCENTER
                6 = SATURN           13 = EARTH-MOON BARYCENTER
                7 = URANUS            


OUTPUTS:
      
     Name                  Description            Units       Min       Max
     ----                  -----------            -----       ---       ---
   cbVectors[][3]          ECI unit vectors       meter    (  see   NOTES  )
                           from Earth to 
                           celestial body
                           for each specified 
                           time events

RETURNS:
      PGS_S_SUCCESS                Successful completion  
      PGSCBP_E_UNABLE_TO_OPEN_FILE Unable to open DE200 file
      PGSCBP_E_TIME_OUT_OF_RANGE   There is NO data for time specified.
      PGSCBP_E_NO_NUTATIONS        NO data for nutations
      PGSCBP_E_NO_LIBRATIONS       NO data for librations
      PGS_E_TOOLKIT                For unknown errors           
      PGSTSF_E_GENERAL_FAILURE     bad return from PGS_TSF_LockIt() 

EXAMPLES:
C:
	#define ARRAY_SIZE	3

	PGSt_SMF_status	returnStatus;
        PGSt_integer    cbId = 10;
	PGSt_integer	numValues;
        PGSt_double     jedTDB[2] = {2436173.0, 0.5};
        PGSt_double     offsets[ARRAY_SIZE] = {3600.0, 7200.0, 10800.0};
        PGSt_double     cbVectors[ARRAY_SIZE][3];

	char 		err[PGS_SMF_MAX_MNEMONIC_SIZE];
	char		msg[PGS_SMF_MAX_MSG_SIZE];

	numValues = ARRAY_SIZE;

        returnStatus = PGS_CBP_EphemRead
			(jedTDB, cbId, numValues, offsets, cbVectors)

	if (returnStatus != PGS_S_SUCCESS)
	{
		PGS_SMF_GetMsg(&returnStatus, err, msg);
		printf ("ERROR: %s\n", msg);
	}


NOTES:
     This file uses Planetary ephemeris data file de200.eos. This data file 
     should be pointed to by the PCF file. It is a machine-specific binary
     file created from de200.dat, an ASCII file in $PGSHOME/database/common.
     That file and the binary version have the following basis and range
     of time validity:

     JPL Planetary Ephemeris DE200/DE200
     Start Epoch: JED=  2433264.5 1949 DEC 14 00:00:00 
     Final Epoch: JED=  2466543.5 2041 JAN 24 00:00:00 
          
     Time is based on Barycentric Dynamical Time (TDB) expressed as Julian 
     ephemeris date ( jedTDB[2] ).

     To simplify the setup, the base time, from which the offsets are defined,
     is required to be within the range shown above.  This means, that near
     the end of the time span, (2021) the offsets must all be positive, while 
     near the beginning the offsets should be chosen as negative.
     
     The minimum and maximum values of the output cbVectors depend on the 
     target planet position with respect to the center planet.


   TIME ACRONYMS:
     
     TAI is:  International Atomic Time
     TDB is:  Barycentric Dynamical Time
     UTC is:  Coordinated Universal Time

   TIME BOUNDARIES:

     The minimum and maximum times that can successfully be processed by this
     function depend on the file leapsec.dat which relates leap second (TAI-UTC)
     values to UTC Julian dates.  The file leapsec.dat starts at Jan 1, 1961;
     therefore an error status message will be returned if this function is
     called with a time before that date.  The file contains dates of leap 
     second events. The file is updated when a new leap second is announced. 
     If an input date is past the last date in the file (or if the file cannot 
     be read) but the date is after Jan 1, 1961, the function will use a 
     calculated value of TAI-UTC based on a linear fit of the data known to be 
     in the table.  This value of TAI-UTC is relatively crude estimate and 
     may be off by as much as 1.0 or more seconds.  Thus, when the function 
     is used for dates in the future of the date and time of invocation, the 
     user ought to carefully check the return status.  The status level will 
     be 'E' if the TAI-UTC value is calculated (although processing will 
     continue in this case, using the calculated value of TAI-UTC).

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

   REFERENCES FOR TIME:

     CCSDS 301.0-B-2 (CCSDS => Consultative Committee for Space Data Systems) 
     Astronomical Almanac, Explanatory Supplement to the Astronomical Almanac

REQUIREMENTS:
     PGSTK-0800
     
DETAILS:
     N/A

GLOBALS:
     This file relies on three global structures, namely "headerRec" which 
     contains header information, "cvalRec" which contains constants' values 
     and "dataRec" which contains actual ephemeris data. The file pointer fp 
     which is needed to read data from the ephemeris data file de200.eos is 
     declared global.
                    
FILES:  
     de200.eos                    ephemeris data file

FUNCTION CALLS:
     PGS_CBP_Pleph()             Read ephemeris data file 
     PGS_IO_Gen_Open()
     PGS_SMF_GetMsg()
     PGS_SMF_GetMsgByCode()
     PGS_SMF_SetDynamicMsg()
     PGS_SMF_SetUnknownMsg()
     PGS_SMF_TestErrorLevel()    test return status
     PGS_TSF_LockIt()            Locks section of code
     PGS_TSF_UnlockIt()

END_PROLOG:
*******************************************************************************/

PGSt_SMF_status 
PGS_CBP_EphemRead(			/* Read and interpolate JPL planetary 
					   ephemeris data */
    PGSt_double     startTDB[2], 	/* TDB Julian date */
    PGSt_integer    cbId, 		/* planet identifier */
    PGSt_integer    numValues,   	/* number of time events to be used */
    PGSt_double     offsets[],     	/* array with offsets of time 
		   			   events in seconds */        
    PGSt_double     cbVectors[][3])	/* returned planet position for each
		   			   time events */
{		   
    PGSt_integer    maxValues;
    int		    itimes;	    	/* counter of time events */
    		   
    PGSt_double     jedTDB[2]; 	        /* TDB Julian date */
    PGSt_double     cbVec[6];      	/* celestial position and velocity */
    PGSt_double     cbVectorMag;        /* magnitude of distance to CB vector */
    		   
    PGSt_SMF_status returnStatus;       /* return status indicating the success
		   			   or failure */
    PGSt_SMF_status returnStatus1;      /* return status indicating the success
		   			   or failure */
    PGSt_SMF_status code;	    	/* status code returned by
		   			   PGS_SMF_GetMsg() */
		   
    static PGSt_boolean	first=PGS_TRUE; /* this is true only on the first call
					   to this function, at which point the
					   DE200 file must be opened and the
					   header structure read into memory */
    
    char	    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg() */
    char	    msg[PGS_SMF_MAX_MSG_SIZE];		 /* message returned by
							    PGS_SMF_GetMsg() */

#ifdef _PGS_THREADSAFE

    /* This file contains 5 functions and 3 file global structures.  This LOCK 
       protects a global file pointer, several statics are not made Thread 
       Specific Data, the order of the calls to the 5 functions and the global 
       structures. */

    PGSt_boolean firstTSF;
    int masterTSFIndex;
    extern PGSt_boolean PGSg_TSF_CBPEphemfirst[];
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
    returnStatus = PGS_TSF_LockIt(PGSd_TSF_CBPLOCK);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
    /** Initialize the variables used for the THREADSAFE version **/

    firstTSF = PGSg_TSF_CBPEphemfirst[masterTSFIndex];
#endif
    	
    returnStatus = PGS_S_SUCCESS;

    /* Now open de200.eos planetary ephemeris data file (it this is the first
       time this function has been called). */

#ifdef _PGS_THREADSAFE 

    /* Threadsafe Protect: first   */

    if (firstTSF == PGS_TRUE)
#else
    if (first == PGS_TRUE)
#endif
    {
	returnStatus = PGS_IO_Gen_Open(DE200,PGSd_IO_Gen_Read,&fp,1);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGS_E_UNIX:
	  case PGSIO_E_GEN_OPENMODE:
	  case PGSIO_E_GEN_FILE_NOEXIST:
	  case PGSIO_E_GEN_REFERENCE_FAILURE:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code != returnStatus)
		PGS_SMF_GetMsgByCode(returnStatus,msg);
	    returnStatus = PGSCBP_E_UNABLE_TO_OPEN_FILE;
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_CBP_EphemRead()");


#ifdef _PGS_THREADSAFE

            /* Unlock CBPLOCK before the return so other Threads can use it */

            PGS_TSF_UnlockIt(PGSd_TSF_CBPLOCK);
#endif
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_CBP_EphemRead()");

#ifdef _PGS_THREADSAFE

           /* Unlock CBPLOCK before the return so other Threads can use it */

            returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_CBPLOCK);
#endif
	    return PGS_E_TOOLKIT;
	}
	
	/* Read the ephemeris file to let the user know what the constants 
	   are */
	
	fread (&headerRec, sizeof(struct struct_type), 1, fp);
	fread (&cvalRec, sizeof(struct cvalType), 1, fp);
	
	/* close de200.eos planetary ephemeris data file */
	
	returnStatus1 = PGS_IO_Gen_Close(fp);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus1,"PGS_CBP_EphemRead()");
	    returnStatus = PGS_E_TOOLKIT;
	}


#ifdef _PGS_THREADSAFE	

        /*  Threadsafe Protect:first  Reassign new value to global */

	firstTSF = PGS_FALSE;
        PGSg_TSF_CBPEphemfirst[masterTSFIndex] = firstTSF;
#else
	first = PGS_FALSE;
#endif
    }
    
    /** Check if time is in correct range **/

    if ((( startTDB[0]+startTDB[1]) < headerRec.ss[0]) || 
           ((startTDB[0]+startTDB[1]) > headerRec.ss[1]))
    {
	returnStatus = PGSCBP_E_TIME_OUT_OF_RANGE;
	    
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_EphemRead()");

#ifdef _PGS_THREADSAFE

        /* Unlock CBPLOCK before the return so other Threads can use it */

        PGS_TSF_UnlockIt(PGSd_TSF_CBPLOCK);
#endif
        return returnStatus;
    }

    /* If numValues = 0, call PGS_CBP_Pleph(). If numValues > 0, add offsets
       to jedTDB[1] and call PGS_CBP_Pleph() for each time events. */
  
    /* calculate for each time events */ 
    
    jedTDB[0] = startTDB[0];
    jedTDB[1] = startTDB[1];
    maxValues = (numValues > 0) ? numValues : 1;
    
    for (itimes = 0; itimes < maxValues; itimes++)
    {
	if (numValues != 0)
	  jedTDB[1] = startTDB[1] + offsets[itimes] / SECONDSperDAY;
	
	returnStatus1 = PGS_CBP_Pleph(jedTDB, &cbId, &c_3, cbVec);  
	
	switch (returnStatus1)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCBP_E_EPOCH_OUT_OF_SPAN:
	  case PGSCBP_E_NO_NUTATIONS:
	  case PGSCBP_E_NO_LIBRATIONS:
	  case PGSCBP_E_INVALID_FILE_POS:
	  case PGSCBP_E_READ_RECORD_ERROR:
	  case PGS_E_TOOLKIT:
	    if (returnStatus != PGSCBP_W_BAD_CB_VECTOR)
	    {
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		if (code != returnStatus1)
		  PGS_SMF_GetMsgByCode(returnStatus1,msg);
		returnStatus = PGSCBP_W_BAD_CB_VECTOR;
	    }
	    cbVectors[itimes][0] = PGSd_GEO_ERROR_VALUE;
	    cbVectors[itimes][1] = PGSd_GEO_ERROR_VALUE;
	    cbVectors[itimes][2] = PGSd_GEO_ERROR_VALUE;
	    break;
	  default:
	    if (returnStatus != PGSCBP_W_BAD_CB_VECTOR)
	    {
		PGS_SMF_SetUnknownMsg(returnStatus1,
				      "PGS_CBP_Earth_CB_Vector()");
		PGS_SMF_GetMsg(&code,mnemonic,msg);
		returnStatus = PGSCBP_W_BAD_CB_VECTOR;
	    }
	    cbVectors[itimes][0] = PGSd_GEO_ERROR_VALUE;
	    cbVectors[itimes][1] = PGSd_GEO_ERROR_VALUE;
	    cbVectors[itimes][2] = PGSd_GEO_ERROR_VALUE;
	}	/* end of switch */

	if (returnStatus == PGS_S_SUCCESS)
	{
	    cbVectors[itimes][0] = cbVec[0] * AU;
	    cbVectors[itimes][1] = cbVec[1] * AU;
	    cbVectors[itimes][2] = cbVec[2] * AU;
		
	    /* Correction for parallax, aberration and light travel time. */
		
	    cbVectorMag = sqrt(cbVectors[itimes][0]*cbVectors[itimes][0] +
			       cbVectors[itimes][1]*cbVectors[itimes][1] +
			       cbVectors[itimes][2]*cbVectors[itimes][2]);

	    cbVectors[itimes][0] = cbVectors[itimes][0] - 
	      cbVectorMag*(cbVec[3]*AU/SECONDSperDAY)/
		(cvalRec.cval[5]*1000.);

	    cbVectors[itimes][1] = cbVectors[itimes][1] -
	      cbVectorMag *
		(cbVec[4] * AU/SECONDSperDAY)/(cvalRec.cval[5]*1000.);

	    cbVectors[itimes][2] = cbVectors[itimes][2] -
	      cbVectorMag *
		(cbVec[5] * AU/SECONDSperDAY)/(cvalRec.cval[5]*1000.);
	} /* end of if */
	
	
    } /* end of for loop */
    
    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus,"PGS_CBP_EphemRead()");
    else 
      PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_CBP_EphemRead()");
#ifdef _PGS_THREADSAFE

    /* Unlock CBPLOCK before the return, so other Threads can use it */

    PGS_TSF_UnlockIt(PGSd_TSF_CBPLOCK);
#endif
    return returnStatus;
}


	    
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
    Break a double  number into a double integer and a double fractional part.
      
      
NAME:
    PGS_CBP_Split()

SYNOPSIS:


DESCRIPTION:
    This function breaks a double number into a double integer and a double 
    fractional part.
    

INPUTS:
     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---  
     tt                jedTDB time           jedTDB   2436144.50    2466543.50
     

OUTPUTS:
     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---
     fr                double jedTDB        jedTDB     2436144.50    2466543.50
                       array
RETURNS:
      PGS_S_SUCCESS                Successful completion  
                                            
      PGS_E_TOOLKIT                For unknown errors

      
EXAMPLES:
C:
     To get vector to Moon
     
     
     PGSt_SMF_status        returnStatus;
     
     PGSt_double            *tt;
     PGSt_double            *fr;
     char 		err[PGS_SMF_MAX_MNEMONIC_SIZE];
     char		msg[PGS_SMF_MAX_MSG_SIZE];
     
  returnStatus = PGS_CBP_Split(&tt,fr);
     
     if (returnStatus != PGS_S_SUCCESS)
     {
         PGS_SMF_GetMsg(&returnStatus, err, msg);
         
         printf ("\ERROR: %s\n", msg);
     ]
     

NOTES:
     This file uses Planetary ephemeris data file de200.eos. This data file 
     should be pointed to by the environment variable PGSDAT.
          
     Time is based on Barycentric Dynamical Time (TDB) expressed as Julian 
     date ( jdTDB )
     
      ******************** JPL PROLOG **************************************

     This subroutine breaks a DOUBLE PRECISION number into a D.P. integer
     and a DOUBLE PRECISION fractional part.

     Calling sequence parameters:

       TT = DOUBLE PRECISION input number

       FR = DOUBLE PRECISION 2-word output array.
            FR(1) contains integer part
            FR(2) contains fractional part

            For negative input numbers, FR(1) contains the next
            more negative integer; FR(2) contains a positive fraction.
     
       **************** END JPL PROLOG ************************************
       
REQUIREMENTS:
     PGSTK-0800

GLOBALS:
     This file relies on three global structures, namely "headerRec" which 
     contains header information, "cvalRec" which contains constants' values 
     and "dataRec" which contains actual ephemeris data. The file pointer fp 
     which is needed to read data from the ephemeris data file de200.eos is 
     declared global.
                    
FILES:  
     de200.eos                    ephemeris data file

FUNCTION CALLS:
     PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/
PGSt_SMF_status 
PGS_CBP_Split(				/* breaks a double number into a double
					   integer and double fractional part */
	PGSt_double 	*tt, 		/* double input number */
        PGSt_double 	*fr)		/* pointer to double which contains
					   double integer and double 
					   fractional part */

{

    PGSt_SMF_status	returnStatus = PGS_S_SUCCESS;

    /* Parameter adjustments */

    --fr;
    
    fr[1] = (PGSt_double) ((PGSt_integer) *tt);
    
    fr[2] = *tt - fr[1];
    
    

    if ((*tt < 0.0) && (fabs(fr[2]) > EPS_64 ))
    {
	/** make adjustments for negative input number **/

	fr[1] += -1.0;
	fr[2] += 1.0;
    }
    
    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Split()");

    return returnStatus;
}




/*******************************************************************************
BEGIN_PROLOG:

TITLE:
    Returns the position and velocity of the point 'TARG' with respect to 
    'CENT'.
      
      
NAME:
    PGS_CBP_Pleph()

SYNOPSIS:
    #include <PGS_CBP.h>

DESCRIPTION:
    The function PGS_CBP_Pleph reads the JPL planetary ephemeris and returns 
    the position and velocity of the point 'TARG' with respect to 'CENT'.


INPUTS:

     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---
    jedTDB[2]          time tags             jdTDB      2436144.50   2466543.50
    targ               target point           N/A       N/A           N/A
    cent               center point           N/A       N/A           N/A
    
	THE NUMBERING CONVENTION FOR 'TARG' AND 'CENT' IS:

                1 = MERCURY           8 = NEPTUNE
                2 = VENUS             9 = PLUTO
                3 = EARTH            10 = MOON
                4 = MARS             11 = SUN
                5 = JUPITER          12 = SOLAR-SYSTEM BARYCENTER
                6 = SATURN           13 = EARTH-MOON BARYCENTER
                7 = URANUS           14 = NUTATIONS (LONGITUDE AND OBLIQ)
                            15 = LIBRATIONS, IF ON EPH FILE

             (IF NUTATIONS ARE WANTED, SET TARG = 14. FOR LIBRATIONS,
              SET TARG = 15. 'CENT' WILL BE IGNORED ON EITHER CALL.)
 

OUTPUTS:
      
     Name                  Description            Units       Min       Max
     ----                  -----------            -----       ---       ---
     rrd                   position and           AU and      ( see NOTES )
                           velocity of point      AU/DAy
                           targ relative to
                           cent       

RETURNS:
      PGS_S_SUCCESS                Successful completion  
      PGSCBP_E_UNABLE_TO_OPEN_FILE Unable to open DE200 file
      PGS_E_TOOLKIT                For unknown errors
      PGSCBP_E_NO_NUTATIONS        NO nutation data
      PGSCBP_E_NO_LIBRATIONS       NO libration data
      PGSCBP_E_EP0CH_OUT_OF_SPAN   Epoch out of span
      PGSTSF_E_GENERAL_FAILURE     Bad return from PGS_TSF_GetMasterIndex() 

      
EXAMPLES:
C:
	PGSt_SMF_status	returnStatus;
        PGSt_integer    targ = 10;
        PGSt_integer    cent = 3;
        PGSt_double     jedTDB[2] = {2436173.0, 0.5};
        PGSt_double     rrd[6];

	char 		err[PGS_SMF_MAX_MNEMONIC_SIZE];
	char		msg[PGS_SMF_MAX_MSG_SIZE];

        returnStatus = PGS_CBP_Pleph
			(jedTDB, &targ, &cent, rrd)

	if (returnStatus != PGS_S_SUCCESS)
	{
		PGS_SMF_GetMsg(&returnStatus, err, msg);
		printf ("ERROR: %s\n", msg);
	}


NOTES:
     This file uses Planetary ephemeris data file de200.eos. This data file 
     should be pointed to by the environment variable PGSDAT.
          
     Time is based on Barycentric Dynamical Time (TDB) expressed as Julian 
     date ( jdTDB ).

     The minimum and maximum values of the output rrd  depends on the 
     target planet with respect to the center planet. The maximum value of 
     any component of the cbVectors for the target with respect to
     the center will be the distance between target and center planets.

   **************************** JPL PROLOG *********************************

  The function PGS_CBP_Pleph reads the JPL planetary ephemeris and returns the
  position and velocity of the point 'TARG' with respect to 'CENT'.

  Calling Sequence parameters:

  	 jd = D.P. JULIAN EPHEMERIS DATE AT WHICH INTERPOLATION
	 	IS WANTED.

	targ = INTEGER NUMBER OF 'TARGET' POINT.

	cent = INTEGER NUMBER OF 'CENTER' POINT.


	THE NUMBERING CONVENTION FOR 'TARG' AND 'CENT' IS:

                1 = MERCURY           8 = NEPTUNE
                2 = VENUS             9 = PLUTO
                3 = EARTH            10 = MOON
                4 = MARS             11 = SUN
                5 = JUPITER          12 = SOLAR-SYSTEM BARYCENTER
                6 = SATURN           13 = EARTH-MOON BARYCENTER
                7 = URANUS           14 = NUTATIONS (LONGITUDE AND OBLIQ)
                            15 = LIBRATIONS, IF ON EPH FILE

             (IF NUTATIONS ARE WANTED, SET TARG = 14. FOR LIBRATIONS,
              SET TARG = 15. 'CENT' WILL BE IGNORED ON EITHER CALL.)
 
	rrd = OUTPUT 6-WORD D.P. ARRAY CONTAINING POSITION AND VELOCITY
            OF POINT 'TARG' RELATIVE TO 'CENT'. THE UNITS ARE AU AND
            AU/DAY. FOR LIBRATIONS THE UNITS ARE RADIANS AND RADIANS
            PER DAY. IN THE CASE OF NUTATIONS THE FIRST FOUR WORDS OF
            RRD WILL BE SET TO NUTATIONS AND RATES, HAVING UNITS OF
            RADIANS AND RADIANS/DAY.
 	
	INSIDE is TRUE if the input Julian Ephemeris Date (JD) is within
            the ephemeris time span.  If not, INSIDE is set to FALSE.

    **************************** END OF JPL PROLOG ************************
REQUIREMENTS:
     PGSTK-0800

DETAILS:
     N/A

GLOBALS:
     This file relies on three global structures, namely "headerRec" which 
     contains header information, "cvalRec" which contains constants' values 
     and "dataRec" which contains actual ephemeris data. The file pointer fp 
     which is needed to read data from the ephemeris data file de200.eos is 
     declared global.
     PGSg_TSF_CBPPlephpv
     PGSg_TSF_CBPPlephpvSun
     PGSg_TSF_CBPPlephnemb  
                    
FILES:  
     de200.eos                    ephemeris data file

FUNCTION CALLS:
     PGS_CBP_State()             
     PGS_SMF_SetDynamicMsg()
     PGS_SMF_SetStaticMsg()
     PGS_SMF_SetUnknownMsg()
     PGS_TSF_GetMasterIndex()

END_PROLOG:
*******************************************************************************/

PGSt_SMF_status 
PGS_CBP_Pleph(				/* read the JPL planetary ephemeris file
					   and returns position and velocity of 
					   the 'targ' with respect to 'cent' */
     PGSt_double 	jed[2],		/* Julian date (jedTDB) at which 
					   interpoaltion is wanted */ 
     PGSt_integer	*targ,		/* integer number for target */ 
     PGSt_integer 	*cent, 		/* integer number for center */
     PGSt_double 	*rrd)		/* output aray containing position and 
					   velocity of the target */
{

    static PGSt_double	jdtot;
    static PGSt_double	pv[78];		/* position and velocity array */
    
    static PGSt_double	embf[2] = { -1.0, 1.0 };
    
    
    static PGSt_double	ve[2];
    static PGSt_double	fac;
  
    
     PGSt_integer		list[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
     static PGSt_integer	l[2];
     static PGSt_integer	tc[2];
     static PGSt_integer	llst[13] = { 1, 2, 10, 4, 5, 6, 7, 8, 9, 10, 11,
					    11, 3};
     static PGSt_integer	nemb = 1;
     static PGSt_integer 	ncmp;
     PGSt_integer 		lme = 0;

     PGSt_integer		counter;

    PGSt_SMF_status 	returnStatus = PGS_S_SUCCESS;
    
     static PGSt_boolean	first = PGS_TRUE;
    
     static PGSt_boolean	bsave = PGS_FALSE;

#ifdef _PGS_THREADSAFE

    /** Declare variables used for the THREADSAFE version **/

    PGSt_boolean firstTSF;
    PGSt_double pvTSF[78];
    PGSt_integer nembTSF;
    int loopVar;
    int masterTSFIndex;
    extern PGSt_double PGSg_TSF_CBPPlephpv[][78];
    extern PGSt_double PGSg_TSF_CBPPlephpvSun[][6];
    extern PGSt_integer PGSg_TSF_CBPPlephnemb[];
    extern PGSt_boolean PGSg_TSF_CBPPlephfirst[];

    /** Get index   Then test for bad returns **/

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /** Initialize the variables used for the THREADSAFE version **/

    for (loopVar = 0; loopVar < 78; loopVar++)
    {
        pvTSF[loopVar]  = PGSg_TSF_CBPPlephpv[masterTSFIndex][loopVar];
    }

    for (loopVar = 0; loopVar < 6; loopVar++)
    {
        pvSunTSF[loopVar]  = PGSg_TSF_CBPPlephpvSun[masterTSFIndex][loopVar];
    }
    nembTSF = PGSg_TSF_CBPPlephnemb[masterTSFIndex];
    firstTSF = PGSg_TSF_CBPPlephfirst[masterTSFIndex];
    
#endif
    
    stateRec.km = PGS_FALSE;		

    stateRec.barry = PGS_FALSE;	

    /** Parameter adjustments **/

    --rrd;

    /** First time in, be sure ephemeris is initialized **/

#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect:first  Reassign new value to global **/

    if (firstTSF == PGS_TRUE)
    {
	ipv = 2;
	firstTSF = PGS_FALSE;
        PGSg_TSF_CBPPlephfirst[masterTSFIndex] = firstTSF;
#else
    if (first == PGS_TRUE)
    {
	ipv = 2;
	first = PGS_FALSE;
#endif	
    	ve[0] = 1.0/(1.0 + headerRec.emrat);
	ve[1] = headerRec.emrat * ve[0];
    }
    

    /** Initialize JED for 'STATE' and set up component count **/


    jdtot = jed[0] + jed[1];
    
   
    if ((jdtot < headerRec.ss[0]) || (jdtot > headerRec.ss[1]))
    {
	returnStatus = PGSCBP_E_EPOCH_OUT_OF_SPAN;
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");
	return returnStatus;
    }
  
    ncmp = 3 * ipv;
	

    /** Check for Nutation call **/

    if (*targ == 14)
    {
       if (headerRec.ipt[34] > 0)
       {
	   list[10] = ipv;

#ifdef _PGS_THREADSAFE

           /** Threadsafe Protect: pv     Reassign new values to global 
               since call to State alters its values **/
                                                            
	   returnStatus = PGS_CBP_State(jed, list, pvTSF, &rrd[1]);
           for (loopVar = 0; loopVar < 78; loopVar++)
           {
               PGSg_TSF_CBPPlephpv[masterTSFIndex][loopVar] = pvTSF[loopVar];
           } 
#else
	   returnStatus = PGS_CBP_State(jed, list, pv, &rrd[1]);
#endif
	   
	   switch (returnStatus)
	   {
	        case PGS_S_SUCCESS:
	            break; 
		case PGSCBP_E_UNABLE_TO_OPEN_FILE:
		    return returnStatus;
	        case PGSCBP_E_INVALID_FILE_POS:
	            PGS_SMF_SetDynamicMsg(returnStatus,
	                           "Data file can't be positioned",
	                           "PGS_CBP_Pleph()");
	            return returnStatus;
	        case PGSCBP_E_READ_RECORD_ERROR:
	            PGS_SMF_SetDynamicMsg(returnStatus,
	                           "Data record read error",
	                           "PGS_CBP_Pleph()");
	            return returnStatus;
		case PGSCBP_E_INVALID_CB_ID:
		case PGSCBP_E_INVALID_INT_FLAG:
		    returnStatus = PGS_E_TOOLKIT;
		    PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");
		    return returnStatus;
		    
	        default:
	            PGS_SMF_SetUnknownMsg(returnStatus, 
	                            "PGS_CBP_Pleph()");
	            returnStatus = PGS_E_TOOLKIT;
	            return returnStatus;
	    }
	            
	   list[10] = 0;
	   PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");
	   return returnStatus;
       }
       else
       {
	   returnStatus = PGSCBP_E_NO_NUTATIONS;
	   PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");	    
	   return returnStatus;
        }
      }
	
      /** Check for Librations **/

      if (*targ == 15)
      {
	 if (headerRec.lpt[1] > 0)
	 {
	     list[11] = ipv;

#ifdef _PGS_THREADSAFE

             /** Threadsafe Protect: pv     Reassign new values to global 
                 since call to State alters its values **/

             returnStatus = PGS_CBP_State(jed, list, pvTSF, &rrd[1]);
             for (loopVar = 0; loopVar < 78; loopVar++)
             {
                 PGSg_TSF_CBPPlephpv[masterTSFIndex][loopVar] = pvTSF[loopVar];
             }
#else
	     returnStatus = PGS_CBP_State(jed, list, pv, &rrd[1]);
#endif
	     switch (returnStatus)
	     {
	          case PGS_S_SUCCESS:
	              break; 
	          case PGSCBP_E_UNABLE_TO_OPEN_FILE:
		      return returnStatus;
	          case PGSCBP_E_INVALID_FILE_POS:
	              PGS_SMF_SetDynamicMsg(returnStatus,
	                           "Data file can't be positioned",
	                           "PGS_CBP_Pleph()");
	              return returnStatus;
	          case PGSCBP_E_READ_RECORD_ERROR:
	              PGS_SMF_SetDynamicMsg(returnStatus,
	                           "Data record read error",
	                           "PGS_CBP_Pleph()");
	              return returnStatus;
		  case PGSCBP_E_INVALID_CB_ID:
		  case PGSCBP_E_INVALID_INT_FLAG:
		      returnStatus = PGS_E_TOOLKIT;
		      PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");
		      return returnStatus;
	            
	          default:
	              PGS_SMF_SetUnknownMsg(returnStatus, 
	                            "PGS_CBP_Pleph()");
	              returnStatus = PGS_E_TOOLKIT;
	              return returnStatus;
	    }	     
	    list[11] = 0;
	    for (counter = 1; counter <= ncmp; ++counter)
            {

#ifdef _PGS_THREADSAFE

                /** Threadsafe Protect: pv **/

	        rrd[counter] = pvTSF[counter+59];
#else
		rrd[counter] = pv[counter+59];
#endif
            }
	    PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");
	    return returnStatus;
	  }
	  else
	  {
	     returnStatus = PGSCBP_E_NO_LIBRATIONS;
	     PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");	     
	     return returnStatus;
	  }
       }
	
	
	/** Force barycentric output by PGS_CBP_STATE **/

	bsave = stateRec.barry;
	stateRec.barry = PGS_TRUE;
	
	/** Set up proper entries in 'LIST' array for PGS_CBP_STATE call **/

	tc[0] = *targ;
	tc[1] = *cent;
	lme = 0;
	   
	
	for (counter = 1; counter <= 2; ++counter)
	{
	    l[counter-1] = llst[tc[counter-1]-1];
	    
	    
	    if (l[counter-1] < 11)
	      list[l[counter-1]-1] = ipv;
	    
	    
	    
	    if (tc[counter-1] == 3)
	    {
		lme = 3;
		fac = -ve[0];
	    }
	    else if (tc[counter-1] == 10)
	    {
		lme = 10;
		fac = ve[1];
	    }
	    else if (tc[counter-1] == 13)
            {

#ifdef _PGS_THREADSAFE

                /** Threadsafe Protect nemb **/

		nembTSF = counter;
                PGSg_TSF_CBPPlephnemb[masterTSFIndex] = nembTSF;
#else
		nemb = counter;
#endif
            }
	}
	
	
	if ((list[9] == ipv) && (l[0] != l[1]))
	  list[2] = ipv - list[2];
	
	
	/** Make call to PGS_CBP_State **/

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: pv     Reassign new values to global 
            since call to State alters its values **/

	returnStatus = PGS_CBP_State(jed, list, pvTSF, &rrd[1]);
        for (loopVar = 0; loopVar < 78; loopVar++)
        {
            PGSg_TSF_CBPPlephpv[masterTSFIndex][loopVar] = pvTSF[loopVar];
        }
#else
	returnStatus = PGS_CBP_State(jed, list, pv, &rrd[1]);
#endif
	switch (returnStatus)
	{
	     case PGS_S_SUCCESS:
	    		break; 
	     case PGSCBP_E_UNABLE_TO_OPEN_FILE:
		        return returnStatus;
	     case PGSCBP_E_INVALID_FILE_POS:
			PGS_SMF_SetDynamicMsg(returnStatus,
			       "Data file can't be positioned",
			       "PGS_CBP_Pleph()");
			return returnStatus;
	     case PGSCBP_E_READ_RECORD_ERROR:
			PGS_SMF_SetDynamicMsg(returnStatus,
	                       "Data record read error",
	                       "PGS_CBP_Pleph()");
			return returnStatus;
	     case PGSCBP_E_INVALID_CB_ID:
	     case PGSCBP_E_INVALID_INT_FLAG:
			returnStatus = PGS_E_TOOLKIT;
			PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");
			return returnStatus;
	            
	      default:
			PGS_SMF_SetUnknownMsg(returnStatus, 
	                       "PGS_CBP_Pleph()");
			returnStatus = PGS_E_TOOLKIT;
			return returnStatus;
	 }	     
	
	

	/** Case: Earth-to-Moon **/

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: pv, nemb, pvSun 
            Reassign pv's new values to global **/ 

	if ((*targ == 10) && (*cent == 3))
	    for (counter = 1; counter <= ncmp; ++counter)
	      rrd[counter] = pvTSF[counter+53];

	/** Case: Moon-to-Earth **/
	
	else if ((*targ == 3) && (*cent == 10))
	  for (counter = 1; counter <= ncmp; ++counter)
	    rrd[counter] = -pvTSF[counter+53];
	
	/** Case: Embary to Moon Or Earth **/
	
	else if (((*targ == 13) || (*cent == 13)) && (list[9] == ipv))
	  for (counter = 1; counter <= ncmp; ++counter)
	    rrd[counter] = pvTSF[counter+53] * fac * embf[nembTSF-1];
	
	/** Otherwise, get Earth or Moon vector and then get output vector **/

	else
	{
	    for (counter = 1; counter <= ncmp; ++counter)
	    {
		pvTSF[counter+59] = pvSunTSF[counter-1];
		pvTSF[counter+71] = pvTSF[counter+11];
                PGSg_TSF_CBPPlephpv[masterTSFIndex][counter+59] = 
                                                    pvTSF[counter+59];
                PGSg_TSF_CBPPlephpv[masterTSFIndex][counter+71] = 
                                                    pvTSF[counter+71];
		if (lme > 0)
                {
		  pvTSF[counter+lme*6-7] = pvTSF[counter+11] + fac *
                                                       pvTSF[counter+53];
                  PGSg_TSF_CBPPlephpv[masterTSFIndex][counter+lme*6-7] = 
                                                  pvTSF[counter+lme*6-7];
                }

		rrd[counter] = pvTSF[counter + *targ * 6 - 7] - 
					pvTSF[counter + *cent * 6 - 7];
		
	    }
	}
#else
	if ((*targ == 10) && (*cent == 3))
	    for (counter = 1; counter <= ncmp; ++counter)
	      rrd[counter] = pv[counter+53];

	/** Case: Moon-to-Earth **/
	
	else if ((*targ == 3) && (*cent == 10))
	  for (counter = 1; counter <= ncmp; ++counter)
	    rrd[counter] = -pv[counter+53];
	
	/** Case: Embary to Moon Or Earth **/
	
	else if (((*targ == 13) || (*cent == 13)) && (list[9] == ipv))
	  for (counter = 1; counter <= ncmp; ++counter)
	    rrd[counter] = pv[counter+53] * fac * embf[nemb-1];
	
	/** Otherwise, get Earth or Moon vector and then get output vector **/

	else
	{
	    for (counter = 1; counter <= ncmp; ++counter)
	    {
		pv[counter+59] = stateRec.pvSun[counter-1];
		pv[counter+71] = pv[counter+11];
		if (lme > 0)
		  pv[counter+lme*6-7] = pv[counter+11] + fac * pv[counter+53];

		rrd[counter] = pv[counter + *targ * 6 - 7] - 
					pv[counter + *cent * 6 - 7];
		
	    }
	}
#endif
	
	/** Clear PGS_CBP_State body array and restore barycenter flag **/

	list[2] = 0;
	list[l[0]-1] = 0;
	list[l[1]-1] = 0;
	stateRec.barry = bsave;
	     
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Pleph()");	       
	return returnStatus;
 

}

	    
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
    Differentiate and interpolate a set of Chebyshev coefficients 
      
NAME:
    PGS_CBP_Interp()

SYNOPSIS:
    #include <PGS_CBP.h>

DESCRIPTION:
    This function differentiates and interpolates set of Chebyshev coefficients 
    to give position and velocity. 

INPUTS:
     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---
     buf               First location of      N/A       N/A           N/A
                       array of double
                       Chebyshev coefficients
                       of position
                       
     t[2]              time interval           jedTDB   2436144.50    2466543.50
     ncf               # of coefficients       N/A       N/A          N/A
                       per components
     ncm               # of components per     N/A       N/A          N/A
                       set of coefficients 
     na                # of sets of coefficients N/A     N/A          N/A
                       in full array
     fl                integer flag;             N/A      0            1
                       0 = for position
                       1 = for position and
                           velocity

OUTPUTS:
     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---
     pv                interpolated position   AU       ( see  NOTES )
                       and velocity of point   AU/DAy

RETURNS:
      PGS_S_SUCCESS                Successful completion  
                                            
      PGSCBP_E_INVALID_INT_FLAG    Invalid flag for fl
      PGS_E_TOOLKIT                For unknown errors     
      PGSTSF_E_GENERAL_FAILURE     Bad return from PGS_TSF_GetTSFMaster or 
                                   PGS_TSF_GetMasterIndex()

      
EXAMPLES:
C:
     PGSt_SMF_status        returnStatus;
     
     PGSt_integer           buf = 3;
     PGSt_double            *t;
     PGSt_integer           *ncf;
     PGSt_integer           *ncm;
     PGSt_integer           *na;
     PGSt_integer           *fl;
     PGSt_double            *pv;
     char 		err[PGS_SMF_MAX_MNEMONIC_SIZE];
     char		msg[PGS_SMF_MAX_MSG_SIZE];
     
     returnStatus = PGS_CBP_Interp(buf,t,&ncf,&ncm,&na,&fl,pv);
     
     if (returnStatus != PGS_S_SUCCESS)
     {
         PGS_SMF_GetMsg(&returnStatus, err, msg);
         
         printf ("\ERROR: %s\n", msg);
     }
     

NOTES:
     This file uses Planetary ephemeris data file de200.eos. This data file 
     should be pointed to by the environment variable PGSDAT.
          
     Time is based on Barycentric Dynamical Time (TDB) expressed as Julian 
     date ( jdTDB ).

     The minimum and maximum values of the output pv  depends on the 
     target planet with respect to the center planet. If the target = center, 
     then minimum value will for the target planet will be 0.0. If traget !=
     center, the maximum value of the cbVectors for the taget with respect to
     the center will be the distance between target and center planets.
     
      ******************** JPL PROLOG **************************************
      
     This subroutine differentiates and interpolates a set of 
     Chebyshev coefficients to give position and velocity.

     Calling sequence parameters:

       Input:

         BUF   1st location of array of PGSt_double precision Chebyshev 
               coefficients of position

           T   T(1) IS DOUBLE PRECISION  FRACTIONAL TIME IN INTERVAL COVERED 
               BY COEFFICIENTS AT WHICH INTERPOLATION IS WANTED 
               (0 .LE. T(1) .LE. 1).  T(2) IS DOUBLE PRECISION LENGTH OF 
               WHOLE INTERVAL IN INPUT TIME UNITS.

         NCF   # OF COEFFICIENTS PER COMPONENT

         NCM   # OF COMPONENTS PER SET OF COEFFICIENTS

          NA   # OF SETS OF COEFFICIENTS IN FULL ARRAY
               (I.E., # OF SUB-INTERVALS IN FULL INTERVAL)

          FL   INTEGER FLAG: =1 FOR POSITIONS ONLY
                             =2 FOR POS AND VEL


       OUTPUT:

         PV   INTERPOLATED QUANTITIES REQUESTED.  DIMENSION
              EXPECTED IS PV(NCM,FL), DOUBLE PRECISION.
             
       **************** END JPL PROLOG ************************************
       
REQUIREMENTS:
     PGSTK-0800

DETAILS:
     N/A

GLOBALS:
     This file relies on three global structures, namely "headerRec" which 
     contains header information, "cvalRec" which contains constants' values 
     and "dataRec" which contains actual ephemeris data. The file pointer fp 
     which is needed to read data from the ephemeris data file de200.eos is 
     declared global.
     PGSg_TSF_CBPInterppc
     PGSg_TSF_CBPInterpvc
     PGSg_TSF_CBPInterpnp
     PGSg_TSF_CBPInterpnv
     PGSg_TSF_CBPInterptwot
                    
FILES:  
     de200.eos                    ephemeris data file

FUNCTION CALLS:
     PGS_TSF_GetMasterIndex()
     PGS_SMF_SetStaticMsg()

END_PROLOG:
*******************************************************************************/
	
PGSt_SMF_status 
PGS_CBP_Interp(			/* differentiates and interpolates set of 
				   Chebyshev coefficients */
      PGSt_integer buf,         /* First location of array of double Chebyshev 
                                   coefficients of position */
      PGSt_double  *t,          /*time interval */
      PGSt_integer *ncf,        /* # of coefficients per component */
      PGSt_integer *ncm,        /* # of components per set of coefficients */
      PGSt_integer *na,	        /* # of sets of coefficients in full array */			
      PGSt_integer *fl,         /* integer flag */
      PGSt_double  *pv)         /* interpolated position and velocity */


{
      static PGSt_double	dna;
      static PGSt_double 	dt1;
      static PGSt_double	temp;
      static PGSt_double	tc;
    
      static PGSt_integer 	np = 2;
      static PGSt_integer 	nv = 3;

      static PGSt_double 	twot;
    
      static PGSt_double 	pc[18];
      static PGSt_double 	vc[18];
      static PGSt_double	vfac;

      static PGSt_integer	lIndx;
    

    PGSt_integer		counter;
    PGSt_integer		count;

    PGSt_integer		bufDim1;
    PGSt_integer		bufDim2;
    PGSt_integer		bufOffset;

    PGSt_integer		pvDim1;
    PGSt_integer		pvOffset;

    PGSt_SMF_status             returnStatus = PGS_S_SUCCESS;
    
    

#ifdef _PGS_THREADSAFE
    PGSt_double pcTSF[18];
    PGSt_double vcTSF[18];
    PGSt_integer npTSF;
    PGSt_integer nvTSF;
    PGSt_double twotTSF;
    int masterTSFIndex;
    int loopVar;
    extern PGSt_double PGSg_TSF_CBPInterppc[][18];
    extern PGSt_double PGSg_TSF_CBPInterpvc[][18];
    extern PGSt_integer PGSg_TSF_CBPInterpnp[];
    extern PGSt_integer PGSg_TSF_CBPInterpnv[];
    extern PGSt_double PGSg_TSF_CBPInterptwot[];

    /** Get index   Then test for bad return **/

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /** Initialize the variables used for the THREADSAFE version **/

    for (loopVar = 0; loopVar < 18; loopVar++)
    {
        pcTSF[loopVar] = PGSg_TSF_CBPInterppc[masterTSFIndex][loopVar];
        vcTSF[loopVar] = PGSg_TSF_CBPInterpvc[masterTSFIndex][loopVar];
    }

    npTSF = PGSg_TSF_CBPInterpnp[masterTSFIndex];
    nvTSF = PGSg_TSF_CBPInterpnv[masterTSFIndex];
    twotTSF = PGSg_TSF_CBPInterptwot[masterTSFIndex];
    
    /** Threadsafe Protect: pc, vc    Reassign new values to globals **/

    pcTSF[0] = 1.0;
    vcTSF[1] = 1.0;
    PGSg_TSF_CBPInterppc[masterTSFIndex][0] = pcTSF[0];
    PGSg_TSF_CBPInterpvc[masterTSFIndex][1] = vcTSF[1];
#else
    pc[0] = 1.0;
    vc[1] = 1.0;
#endif
     
   
    if (*fl <= 1)
    {
	returnStatus = PGSCBP_E_INVALID_INT_FLAG;
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Interp()");
	return returnStatus;
    }

    /* Parameter adjustments */

    pvDim1 = *ncm;
    pvOffset = pvDim1 + 1;
    pv -= pvOffset;
    --t;
    bufDim1 = *ncf;
    bufDim2 = *ncm;
    bufOffset = bufDim1 * (bufDim2 + 1) + 1;
    buf -= bufOffset;
 
    /** Entry point. Get correct sub-interval number for this set of 
	coefficients and then get normalized Chebyshev time within that 
	subinterval. **/

    dna = (PGSt_double) *na;

    dt1 = (PGSt_double)((PGSt_integer) t[1]);

    temp = dna * t[1];

    lIndx = (PGSt_integer) ((PGSt_double)(temp - dt1)) + 1;
    

    
    /** tc is the normalized Chebyshev time (-1 <= tc <= 1) **/

    tc = 2.0 * ((temp - (PGSt_integer) temp) + dt1) - 1.0;
    
    

    /** Check to see whether Chebyshev time has changed, and compute new 
	polynomial values if it has. The element pc[1] is the value of t1[tc] 
	and hence contains the value of tc on the previous call. **/

#ifdef _PGS_THREADSAFE
    
    /** Threadsafe Protect: np, nv, pc, twot   
                       Reassign new values to globals **/
    
    if (tc != pcTSF[1])
    {
	npTSF = 2;
        PGSg_TSF_CBPInterpnp[masterTSFIndex] = npTSF;
	nvTSF = 3;
        PGSg_TSF_CBPInterpnv[masterTSFIndex] = nvTSF;
	pcTSF[1] = tc;
        PGSg_TSF_CBPInterppc[masterTSFIndex][1] = pcTSF[1];
	twotTSF = tc + tc;
        PGSg_TSF_CBPInterptwot[masterTSFIndex] = twotTSF;
    }
      
    
    /** Be sure that at least 'ncf' polynomials have been evaluated and are 
	stored in the array 'pc'. **/
    
    if (npTSF < *ncf)
    {
    
	for (counter = npTSF+1; counter <= *ncf; ++counter)
	   pcTSF[counter-1] = twotTSF*pcTSF[counter-2] - pcTSF[counter-3];

	for (loopVar = 0; loopVar < 18; loopVar++)
        {
            PGSg_TSF_CBPInterppc[masterTSFIndex][loopVar] = pcTSF[loopVar];
        }

	npTSF = *ncf;
        PGSg_TSF_CBPInterpnp[masterTSFIndex] = npTSF;
    }
    
    /** Interpolate to get position for each component **/

    for (counter = 1; counter <= *ncm; ++counter)
    {
	pv[counter + pvDim1] = 0.0;
	
	
	for (count = *ncf; count >= 1; --count)
	    pv[counter + pvDim1] += pcTSF[count-1] * dataRec.db[buf + count +
				(counter + lIndx * bufDim2) * bufDim1];

    }
   
    /** If velocity interpolation is wanted, be sure enough derivative 
	polynomials have been generated and stored. **/
    /** Threadsafe Protect vc, twot, nv  reassign vc and nv values **/

    vfac = (dna + dna)/t[2];
    
    vcTSF[2] = twotTSF + twotTSF;
    PGSg_TSF_CBPInterpvc[masterTSFIndex][2] = vcTSF[2];
    
    if (nvTSF < *ncf)
    {
	for (counter = nvTSF+1; counter <= *ncf; ++counter)
	   vcTSF[counter-1] = twotTSF * vcTSF[counter-2] + pcTSF[counter-2] +
                                pcTSF[counter-2] - vcTSF[counter-3];

	for (loopVar = 18; loopVar < 18; loopVar++)
        {
            PGSg_TSF_CBPInterpvc[masterTSFIndex][loopVar] = vcTSF[loopVar];
        }
	
	nvTSF = *ncf;
        PGSg_TSF_CBPInterpnv[masterTSFIndex] = nvTSF;
    }
   
#else
    if (tc != pc[1])
    {
	np = 2;
	nv = 3;
	pc[1] = tc;
	twot = tc + tc;
    }
      
    
    /** Be sure that at least 'ncf' polynomials have been evaluated and are 
	stored in the array 'pc'. **/
    
    if (np < *ncf)
    {
    
	for (counter = np+1; counter <= *ncf; ++counter)
	   pc[counter-1] = twot*pc[counter-2] - pc[counter-3];

	np = *ncf;
    }
    
    /** Interpolate to get position for each component **/

    for (counter = 1; counter <= *ncm; ++counter)
    {
	pv[counter + pvDim1] = 0.0;
	
	
	for (count = *ncf; count >= 1; --count)
	    pv[counter + pvDim1] += pc[count-1] * dataRec.db[buf + count +
				(counter + lIndx * bufDim2) * bufDim1];

    }
   
    /** If velocity interpolation is wanted, be sure enough derivative 
	polynomials have been generated and stored. **/
        

    vfac = (dna + dna)/t[2];
    
    vc[2] = twot + twot;
    
    
    if (nv < *ncf)
    {
	for (counter = nv+1; counter <= *ncf; ++counter)
	   vc[counter-1] = twot * vc[counter-2] + pc[counter-2] + pc[counter-2] 
				- vc[counter-3];
	
	nv = *ncf;
    }
#endif


    /** Interpolate to get velocity for each component. **/

    for (counter = 1; counter <= *ncm; ++counter)
    {
        
	pv[counter + (pvDim1 << 1)] = 0.0;

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: vc **/

	for (count = *ncf; count >= 2; --count)
	   pv[counter + (pvDim1 << 1)] += vcTSF[count-1] * 
            dataRec.db[buf + count + (counter + lIndx * bufDim2) * bufDim1];
#else
	for (count = *ncf; count >= 2; --count)
	   pv[counter + (pvDim1 << 1)] += vc[count-1] * dataRec.db[buf + 
                          count + (counter + lIndx * bufDim2) * bufDim1];
#endif

	pv[counter + (pvDim1 << 1)] *= vfac;
   }

    if (returnStatus == PGS_S_SUCCESS)
      PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_Interp()");

    return returnStatus;
}


    
    
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
    Read and interpolate the JPL planetary ephemeris file.
      
NAME:
    PGS_CBP_State()

SYNOPSIS:


DESCRIPTION:
    The function PGS_CBP_State reads and interpolates the JPL planetary 
    ephemeris file.

INPUTS:
     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---
     jedTDB            time tags             jdTDB      2436144.50    2466543.50
     list              integer array	     N/A	0	      2
		       specifying what
	               interpolation is
		       wanted  

		       0 - NO interpolation
		       1 - POSITION only
		       2 - POSITION and VELOCITY
				          

OUTPUTS:
     Name              Description           Units      Min           Max
     ----              -----------           -----      ---           ---
     pv		       array containing	     AU and	(   see	  NOTES)
		       requested 	     AU/DAY
		       interpolated
		       quantities	
     nut	       array containing	    ( see 	JPL 	PROLOG	)	
		       nutations
			
RETURNS:
      PGS_S_SUCCESS                Successful completion  
      PGSCBP_E_UNABLE_TO_OPEN_FILE Unable to open DE200 file
      PGS_E_TOOLKIT                For unknown errors
      PGSCBP_E_TARG_EQUAL_CENT     Target equal to Center flag
      PGSTSF_E_GENERAL_FAILURE     Bad return from PGS_TSF_GetTSFMaster or 
                                   PGS_TSF_GetMasterIndex()  
      
EXAMPLES:
C:
      PGSt_SMF_status	returnStatus;
      PGSt_double     *jedTDB;
      PGSt_integer    *list; 
      PGSt_double     *pv;     
      PGSt_double     *nut;

      char 		err[PGS_SMF_MAX_MNEMONIC_SIZE];
      char		msg[PGS_SMF_MAX_MSG_SIZE];

        returnStatus = PGS_CBP_State
			(jedTDB, list, pv, nut)

	if (returnStatus != PGS_S_SUCCESS)
	{
		PGS_SMF_GetMsg(&returnStatus, err, msg);
		printf ("ERROR: %s\n", msg);
	}

     

NOTES:
     This file uses Planetary ephemeris data file de200.eos. This data file 
     should be pointed to by the environment variable PGSDAT.
          
     Time is based on Barycentric Dynamical Time (TDB) expressed as Julian 
     date ( jdTDB ).

     The minimum and maximum values of the output pv  depends on the 
     target planet with respect to the center planet. 
     
  *********************** JPL PROLOG ************************************
  
  This function reads and interpolates the JPL planetary ephemeris file.


     CALLING SEQUENCE PARAMETERS:

     INPUT:

         JED   DP 2-WORD JULIAN EPHEMERIS EPOCH AT WHICH INTERPOLATION
               IS WANTED.  ANY COMBINATION OF JED(1)+JED(2) WHICH FALLS
               WITHIN THE TIME SPAN ON THE FILE IS A PERMISSIBLE EPOCH.

                A. FOR EASE IN PROGRAMMING, THE USER MAY PUT THE
                   ENTIRE EPOCH IN JED(1) AND SET JED(2)=0.

                B. FOR MAXIMUM INTERPOLATION ACCURACY, SET JED(1) =
                   THE MOST RECENT MIDNIGHT AT OR BEFORE INTERPOLATION
                   EPOCH AND SET JED(2) = FRACTIONAL PART OF A DAY
                   ELAPSED BETWEEN JED(1) AND EPOCH.

                C. AS AN ALTERNATIVE, IT MAY PROVE CONVENIENT TO SET
                   JED(1) = SOME FIXED EPOCH, SUCH AS START OF INTEGRATION,
                   AND JED(2) = ELAPSED INTERVAL BETWEEN THEN AND EPOCH.

        LIST   12-WORD INTEGER ARRAY SPECIFYING WHAT INTERPOLATION
               IS WANTED FOR EACH OF THE BODIES ON THE FILE.

                         LIST(I)=0, NO INTERPOLATION FOR BODY I
                                =1, POSITION ONLY
                                =2, POSITION AND VELOCITY

               THE DESIGNATION OF THE ASTRONOMICAL BODIES BY I IS:

                         I = 1: MERCURY
                           = 2: VENUS
                           = 3: EARTH-MOON BARYCENTER
                           = 4: MARS
                           = 5: JUPITER
                           = 6: SATURN
                           = 7: URANUS
                           = 8: NEPTUNE
                           = 9: PLUTO
                           =10: GEOCENTRIC MOON
                           =11: NUTATIONS IN LONGITUDE AND OBLIQUITY
                           =12: LUNAR LIBRATIONS (IF ON FILE)


     OUTPUT:

          PV   DP 6 X 11 ARRAY THAT WILL CONTAIN REQUESTED INTERPOLATED
               QUANTITIES.  THE BODY SPECIFIED BY LIST(I) WILL HAVE ITS
               STATE IN THE ARRAY STARTING AT PV(1,I).  (ON ANY GIVEN
               CALL, ONLY THOSE WORDS IN 'PV' WHICH ARE AFFECTED BY THE
               FIRST 10 'LIST' ENTRIES (AND BY LIST(12) IF LIBRATIONS ARE
               ON THE FILE) ARE SET.  THE REST OF THE 'PV' ARRAY
               IS UNTOUCHED.)  THE ORDER OF COMPONENTS STARTING IN
               PV(1,I) IS: X,Y,Z,DX,DY,DZ.

               ALL OUTPUT VECTORS ARE REFERENCED TO THE EARTH MEAN
               EQUATOR AND EQUINOX OF EPOCH. THE MOON STATE IS ALWAYS
               GEOCENTRIC; THE OTHER NINE STATES ARE EITHER HELIOCENTRIC
               OR SOLAR-SYSTEM BARYCENTRIC, DEPENDING ON THE SETTING OF
               COMMON FLAGS (SEE BELOW).

               LUNAR LIBRATIONS, IF ON 12, ARE PUT INTO PV(K,11) IF
               LIST(12) IS 1 OR 2.

         NUT   DP 4-WORD ARRAY THAT WILL CONTAIN NUTATIONS AND RATES,
               DEPENDING ON THE SETTING OF LIST(11).  THE ORDER OF
               QUANTITIES IN NUT IS:

                        D PSI  (NUTATION IN LONGITUDE)
                        D EPSILON (NUTATION IN OBLIQUITY)
                        D PSI DOT
                        D EPSILON DOT

     PARAMETERS IN COMMON:

     COMMON AREA EPUNIT:

        FILE   INTEGER OF THE UNIT CONTAINING THE EPHEMERIS. DEFAULT = 12

     COMMON AREA STCOMM:

          KM   LOGICAL FLAG DEFINING PHYSICAL UNITS OF THE OUTPUT
               STATES. KM = .TRUE., KM AND KM/SEC
                          = .FALSE., AU AND AU/DAY
               DEFAULT VALUE = .FALSE.  (KM DETERMINES TIME UNIT
               FOR NUTATIONS AND LIBRATIONS.  ANGLE UNIT IS ALWAYS RADIANS.)

        BARY   LOGICAL FLAG DEFINING OUTPUT CENTER.
               ONLY THE 9 PLANETS ARE AFFECTED.
                        BARY = .TRUE. =\ CENTER IS SOLAR-SYSTEM BARYCENTER
                             = .FALSE. =\ CENTER IS SUN
               DEFAULT VALUE = .FALSE.

       PVSUN   DP 6-WORD ARRAY CONTAINING THE BARYCENTRIC POSITION AND
               VELOCITY OF THE SUN.
               
     ************************** END OF JPL PROLOG *******************************

REQUIREMENTS:
     PGSTK-0800

DETAILS:
     N/A

GLOBALS:
     This file relies on three global structures, namely "headerRec" which 
     contains header information, "cvalRec" which contains constants' values 
     and "dataRec" which contains actual ephemeris data. The file pointer fp 
     which is needed to read data from the ephemeris data file de200.eos is 
     declared global.
     PGSg_TSF_CBPPlephpvSun
     PGSg_TSF_CBPStatet
     PGSg_TSF_CBPStatejd
     PGSg_TSF_CBPStateauFac
                    
FILES:  
     de200.eos                    ephemeris data file

FUNCTION CALLS:
     PGS_CBP_Split()             
     PGS_CBP_Interp()
     PGS_TSF_GetMasterIndex()

END_PROLOG:
*******************************************************************************/

PGSt_SMF_status 
PGS_CBP_State(				/* reads and interpolates the JPL 
					   planetary ephemeris file */
    PGSt_double 	*jed, 		/* Julian date (jedTDB) at which 
					   interpolation is wanted */
    PGSt_integer 	*list, 		/* pointer to integer specifying what 
					   interpolation is wanted for each of 
					   the celestial body */
    PGSt_double 	*pv, 		/* pointer to double that contains 
					   requested interpolation quantities */
    PGSt_double 	*nut)		/* pointer to double that contains
					   nutations and rates */
{

    static PGSt_double	t[2];
    static PGSt_double 	auFac = 1.0;
    static PGSt_double 	jd[4];
    static PGSt_double 	s;
    
    static PGSt_integer nrl;
    static PGSt_integer	nr;
    
    PGSt_integer	counter;
    PGSt_integer	count;

    PGSt_uinteger       position;
  
    static PGSt_boolean first=PGS_TRUE;

    PGSt_SMF_status     returnStatus = PGS_S_SUCCESS;
    PGSt_SMF_status     code;                    /* status code returned
						    by PGS_SMF_GetMsg */
    char	    mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* mnemonic returned by
							    PGS_SMF_GetMsg() */
    char	    msg[PGS_SMF_MAX_MSG_SIZE];		 /* message returned by
							    PGS_SMF_GetMsg() */
#ifdef _PGS_THREADSAFE
    PGSt_boolean firstTSF;
    PGSt_double tTSF[2];
    PGSt_double jdTSF[4];
    PGSt_double auFacTSF;
    int masterTSFIndex;
    int loopVar;
    extern PGSt_double PGSg_TSF_CBPPlephpvSun[][6];
    extern PGSt_double PGSg_TSF_CBPStatet[][2];
    extern PGSt_double PGSg_TSF_CBPStatejd[][4];
    extern PGSt_double PGSg_TSF_CBPStateauFac[];
    extern PGSt_boolean PGSg_TSF_CBPStatefirst[];

    /** Get index   Then test for bad return **/

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /**   Initialize the variables used for the THREADSAFE version **/

    for (loopVar = 0; loopVar < 6; loopVar++)
    {
        pvSunTSF[loopVar]  = PGSg_TSF_CBPPlephpvSun[masterTSFIndex][loopVar];
    }
    for (loopVar = 0; loopVar < 2; loopVar++)
    {
        tTSF[loopVar]  = PGSg_TSF_CBPStatet[masterTSFIndex][loopVar];
    }
    for (loopVar = 0; loopVar < 4; loopVar++)
    {
        jdTSF[loopVar]  = PGSg_TSF_CBPStatejd[masterTSFIndex][loopVar];
    }
    auFacTSF = PGSg_TSF_CBPStateauFac[masterTSFIndex];
    firstTSF = PGSg_TSF_CBPStatefirst[masterTSFIndex];
#endif
    

    /** Parameter adjustments **/

    --nut;
    pv -= 7;
    --list;
    --jed;

    /** 1st time in, get pointer data etc., from ephemeris file **/
    
#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect:first,t,auFac  Reassign new values to globals **/

    if (firstTSF == PGS_TRUE)
    {
	firstTSF = PGS_FALSE;
        PGSg_TSF_CBPStatefirst[masterTSFIndex] = firstTSF;
	if (stateRec.km == PGS_TRUE)
	    tTSF[1] = headerRec.ss[2]*SECONDSperDAY;
	else 
	{
	    tTSF[1] = headerRec.ss[2];
	    auFacTSF = 1.0/headerRec.au;
            PGSg_TSF_CBPStateauFac[masterTSFIndex] = auFacTSF;
	}
        PGSg_TSF_CBPStatet[masterTSFIndex][1] = tTSF[1];
#else
    if (first == PGS_TRUE)
    {
	first = PGS_FALSE;
	if (stateRec.km == PGS_TRUE)
	    t[1] = headerRec.ss[2]*SECONDSperDAY;
	else 
	{
	    t[1] = headerRec.ss[2];
	    auFac = 1.0/headerRec.au;
	}
#endif
    }
    
    
    
    /** main entry point -- check epoch and read right record **/
    
    s = jed[1] - 0.5;
    
#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect: jd    Reassign new values to global since 
        call to Split alters its values **/

    returnStatus = PGS_CBP_Split(&s, jdTSF);
    for (loopVar = 0; loopVar < 4; loopVar++)
    {
        PGSg_TSF_CBPStatejd[masterTSFIndex][loopVar] = jdTSF[loopVar];
    }
#else
    returnStatus = PGS_CBP_Split(&s, jd);
#endif
    if (returnStatus != PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(returnStatus, 
			     "PGS_CBP_State()");
        returnStatus = PGS_E_TOOLKIT;
        return returnStatus;
    }
    
#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect: jd  Reassign new values to global since 
        call to Split alters its values **/

    returnStatus = PGS_CBP_Split(&jed[2], &jdTSF[2]);
    PGSg_TSF_CBPStatejd[masterTSFIndex][2] = jdTSF[2];
#else
    returnStatus = PGS_CBP_Split(&jed[2], &jd[2]);
#endif
    if (returnStatus != PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(returnStatus, 
			     "PGS_CBP_State()");
        returnStatus = PGS_E_TOOLKIT;
        return returnStatus;
    }
    
#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect: jd  Reassign new values to global since 
        call to Split alters its values **/

    jdTSF[0] = jdTSF[0] + jdTSF[2] + 0.5;
    jdTSF[1] += jdTSF[3];
    returnStatus = PGS_CBP_Split(&jdTSF[1], &jdTSF[2]);
    PGSg_TSF_CBPStatejd[masterTSFIndex][0] = jdTSF[0];
    PGSg_TSF_CBPStatejd[masterTSFIndex][1] = jdTSF[1];
    PGSg_TSF_CBPStatejd[masterTSFIndex][2] = jdTSF[2];
#else
    jd[0] = jd[0] + jd[2] + 0.5;
    jd[1] += jd[3];
    returnStatus = PGS_CBP_Split(&jd[1], &jd[2]);
#endif
    if (returnStatus != PGS_S_SUCCESS)
    {
        PGS_SMF_SetStaticMsg(returnStatus, 
			     "PGS_CBP_State()");
        returnStatus = PGS_E_TOOLKIT;
        return returnStatus;
    }
    
#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect: jd,     Reassign new values to globals **/

    jdTSF[0] += jdTSF[2];
    PGSg_TSF_CBPStatejd[masterTSFIndex][0] = jdTSF[0];
     

    /** Error return of epoch out of range **/
    
    if ((jdTSF[0] < headerRec.ss[0]) || ((jdTSF[0] + jdTSF[3]) > 
                                                    headerRec.ss[1]))
    {
	returnStatus = PGSCBP_E_EPOCH_OUT_OF_SPAN;
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_State()");
	return returnStatus;
    }
    /** Calculate record # and relative time in interval **/
    
    nr = (PGSt_integer) ((jdTSF[0] - headerRec.ss[0])/headerRec.ss[2]) + 3;
    
    if (jdTSF[0] == headerRec.ss[1])
	--nr;

    /** Threadsafe Protect: jd, t    Reassign new t values to globals **/

    tTSF[0] = (jdTSF[0] - ((PGSt_double)(nr - 3) * headerRec.ss[2] + 
            headerRec.ss[0]) + jdTSF[3])/ headerRec.ss[2];
    PGSg_TSF_CBPStatet[masterTSFIndex][0] = tTSF[0];
#else
    jd[0] += jd[2];
     

    /** Error return of epoch out of range **/
    
    if ((jd[0] < headerRec.ss[0]) || ((jd[0] + jd[3]) > headerRec.ss[1]))
    {
	returnStatus = PGSCBP_E_EPOCH_OUT_OF_SPAN;
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_State()");
	return returnStatus;
    }
    /** Calculate record # and relative time in interval **/
    
    nr = (PGSt_integer) ((jd[0] - headerRec.ss[0])/headerRec.ss[2]) + 3;
    
    if (jd[0] == headerRec.ss[1])
	--nr;
    t[0] = (jd[0] - ((PGSt_double)(nr - 3) * headerRec.ss[2] + headerRec.ss[0]) 
	    + jd[3])/ headerRec.ss[2];
#endif

    /** Read correct record if not in core **/
    
    if (nr != nrl)
    {
	returnStatus = PGS_IO_Gen_Open(DE200,PGSd_IO_Gen_Read,&fp,1);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGS_E_UNIX:
	  case PGSIO_E_GEN_OPENMODE:
	  case PGSIO_E_GEN_FILE_NOEXIST:
	  case PGSIO_E_GEN_REFERENCE_FAILURE:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code != returnStatus)
		PGS_SMF_GetMsgByCode(returnStatus,msg);
	    returnStatus = PGSCBP_E_UNABLE_TO_OPEN_FILE;
	    PGS_SMF_SetDynamicMsg(returnStatus,msg,"PGS_CBP_State()");
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_CBP_State()");
	    return PGS_E_TOOLKIT;
	}

	nrl = nr;

	position = sizeof(struct struct_type) + sizeof(struct cvalType) + 
                  ((nr - 3) * sizeof(struct dataType));
	    
	if ((fseek(fp, position, SEEK_SET)) != 0)
	{
	    returnStatus = PGSCBP_E_INVALID_FILE_POS;
	    PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_State()");
	    PGS_IO_Gen_Close(fp);
	    return returnStatus;
	} 
       
	if ((fread(&dataRec, sizeof(struct dataType), 1, fp)) != 1)
	{
	    returnStatus = PGSCBP_E_READ_RECORD_ERROR;
	    PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_State()");	    
	    PGS_IO_Gen_Close(fp);
	    return returnStatus;
        }
      
	returnStatus = PGS_IO_Gen_Close(fp);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_CBP_State()");
	    returnStatus = PGS_E_TOOLKIT;
	}
    }
    

    /** Interpolate SSBARY Sun **/

#ifdef _PGS_THREADSAFE

    /** Threadsafe Protect: pvSun, t     Reassign new values to globals **/

    returnStatus=PGS_CBP_Interp(headerRec.ipt[30]-1, tTSF, &headerRec.ipt[31],
				&c_3, &headerRec.ipt[32], &c_2, pvSunTSF);
    for (loopVar = 0; loopVar < 6; loopVar++)
    {
        PGSg_TSF_CBPPlephpvSun[masterTSFIndex][loopVar] = pvSunTSF[loopVar];
    }

    for (loopVar = 0; loopVar < 2; loopVar++)
    {
        PGSg_TSF_CBPStatet[masterTSFIndex][loopVar] = tTSF[loopVar];
    }
#else
    returnStatus=PGS_CBP_Interp(headerRec.ipt[30]-1, t, &headerRec.ipt[31],
				&c_3, &headerRec.ipt[32], &c_2, stateRec.pvSun);
#endif
  
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCBP_E_INVALID_INT_FLAG:
	PGS_SMF_SetDynamicMsg(returnStatus, "Integer flag is invalid",
			      "PGS_CBP_State()");
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, "PGS_CBP_State()");
	returnStatus = PGS_E_TOOLKIT;
	return returnStatus;
    }
    
    for (counter = 1; counter <= 6; ++counter)
    {

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: pvSun, auFac  
                  Reassign pvSun new values to globals **/

	pvSunTSF[counter - 1] *= auFacTSF;
        PGSg_TSF_CBPPlephpvSun[masterTSFIndex][counter - 1] = 
                                          pvSunTSF[counter - 1];
#else
	stateRec.pvSun[counter - 1] *= auFac;
#endif
    }
				     
         
    

    /** Check and interpolate whichever bodies are requested **/

    for (counter = 1; counter <= 10; ++counter)
    {
	if (list[counter] <= 0)
	    continue;
	
	if (headerRec.ipt[counter * 3 - 2] <= 0)
	{
	    returnStatus = PGSCBP_E_INVALID_CB_ID;
	    PGS_SMF_SetStaticMsg(returnStatus, 
				 "PGS_CBP_State()");
	    return returnStatus;
	}

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: t,  Reassign new values to globals **/
	
        returnStatus = PGS_CBP_Interp(headerRec.ipt[counter*3 - 3] - 1, tTSF,
				      &headerRec.ipt[counter*3 - 2], &c_3, 
				      &headerRec.ipt[counter*3 - 1], 
				      &list[counter], &pv[counter*6 + 1]);   
        for (loopVar = 0; loopVar < 2; loopVar++)
        {
            PGSg_TSF_CBPStatet[masterTSFIndex][loopVar] = tTSF[loopVar];
        }
#else					  
        returnStatus = PGS_CBP_Interp(headerRec.ipt[counter*3 - 3] - 1, t,
				      &headerRec.ipt[counter*3 - 2], &c_3, 
				      &headerRec.ipt[counter*3 - 1], 
				      &list[counter], &pv[counter*6 + 1]);   
#endif
					  
	for (count = 1; count <= list[counter]*3; ++count)
	{
	    if (( counter <= 9) && (!(stateRec.barry)))
            {

#ifdef _PGS_THREADSAFE

                /** Threadsafe Protect: pvSun, auFac **/

		pv[count + counter * 6] = pv[count + counter * 6] * auFacTSF - 
		                          pvSunTSF[count-1];
#else
		pv[count + counter * 6] = pv[count + counter * 6] * auFac - 
		                          stateRec.pvSun[count-1];
#endif
            }
	    else
             {

#ifdef _PGS_THREADSAFE

                /** Threadsafe Protect: auFac **/

		pv[count + counter * 6] *= auFacTSF;
#else
		pv[count + counter * 6] *= auFac;
#endif
             }
	}
    }

    /** Do nutations if requested (and if on file) **/

    if ((list[11] > 0) && (headerRec.ipt[34] > 0))
    {

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: t,    Reassign new values to globals **/

	returnStatus = PGS_CBP_Interp(headerRec.ipt[33]-1,tTSF,
                                      &headerRec.ipt[34],
				      &c_2,&headerRec.ipt[35],&list[11],
				      &nut[1]);
        for (loopVar = 0; loopVar < 2; loopVar++)
        {
            PGSg_TSF_CBPStatet[masterTSFIndex][loopVar] = tTSF[loopVar];
        }
#else
	returnStatus = PGS_CBP_Interp(headerRec.ipt[33]-1,t,&headerRec.ipt[34],
				      &c_2,&headerRec.ipt[35],&list[11],
				      &nut[1]);
#endif
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGSCBP_E_INVALID_INT_FLAG:
	    PGS_SMF_SetDynamicMsg(returnStatus,"Integer flag is invalid",
				  "PGS_CBP_State()");
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_CBP_State()");
	    returnStatus = PGS_E_TOOLKIT;
	    return returnStatus;
	}
    }

    /** Get librations if requested (and if on file) **/

    if ((headerRec.lpt[1] > 0) && (list[11] > 0))
    {

#ifdef _PGS_THREADSAFE

        /** Threadsafe Protect: t,    Reassign new values to globals **/

	returnStatus = PGS_CBP_Interp(headerRec.lpt[0]-1, tTSF, 
                                      &headerRec.lpt[1],
				      &c_3, &headerRec.lpt[2], &list[12], 
				      &pv[67]);
        for (loopVar = 0; loopVar < 2; loopVar++)
        {
            PGSg_TSF_CBPStatet[masterTSFIndex][loopVar] = tTSF[loopVar];
        }
#else
	returnStatus = PGS_CBP_Interp(headerRec.lpt[0]-1, t, &headerRec.lpt[1],
				      &c_3, &headerRec.lpt[2], &list[12], 
				      &pv[67]);
#endif
    }
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSCBP_E_INVALID_INT_FLAG:
	PGS_SMF_SetDynamicMsg(returnStatus, "Integer flag is invalid",
			      "PGS_CBP_State()");
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus, "PGS_CBP_State()");
	returnStatus = PGS_E_TOOLKIT;
	return returnStatus;
    }
    
    
    if (returnStatus == PGS_S_SUCCESS)
	PGS_SMF_SetStaticMsg(returnStatus, "PGS_CBP_State()");
    
    return returnStatus;
}

