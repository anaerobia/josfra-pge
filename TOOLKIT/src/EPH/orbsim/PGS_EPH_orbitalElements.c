/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_EPH_orbitalElements.c

DESCRIPTION:
   This file contains the function PGS_EPH_orbitalElements().
   Given input time and epoch time as well as initial keplerian orbital
   elements (i.e. at epoch) for the spacecraft, this routine calculates 
   spacecraft orbital elements at the input time.

AUTHOR:
   Guru Tej S. Khalsa  / Applied Research Corp
   Peter D. Noerdlinger / SM&A Inc.
   Abe Taaheri / SM&A Inc.
   Xin Wang / EIT Inc.

HISTORY:
   13-Jul-1994  GTSK  Initial version.  Adapted from UARS FORTRAN code.
   16-Mar-1995  GTSK  Add effect of J2 into calculation of mean angular motion.
   16-Mar-1995  GTSK  Truncate some calculated values (data[5], data[4],
                      data[3]) so that the precision of the result does not
		      exceed the accuracy of the calculation.  This was done
		      to ensure uniform results across different computing
		      platforms (otherwise, who cares?).
   26-Mar-1998  GTSK  Changed orbit number determination so that orbit numbers
                      start at 0 (they were previously starting at 1).
   28-Feb-1999   PDN  Added CHEM spacecraft
   20-Dec-1999    AT  Modified PM spcaecraft tags to agree with other TOOLKIT
                      tools (such as L0, and TD) to distinguish GIIS from GIRD,
		      although both refer to the same PM spacecraft.
   05-Sep-2000    AT  Modified PM spcaecraft tags to use EPSPM1 for spacecraft
                      name as DPREP. 
   11-Apr-2001    AT  Modified CHEM to AURA
   30-Oct-2001    XW  Parameterized upon inputting orbital elements.
   30-Jun-2003    AT  Modified for epoch time as input orbital element.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Calculate Spacecraft Orbital Elements

NAME:
   PGS_EPH_orbitalElements()

SYNOPSIS:
   #include <PGS_EPH.h>
      
DESCRIPTION:
   Given input time and epoch time as well as initial keplerian orbital
   elements (i.e. at epoch) for the spacecraft, this routine calculates 
   spacecraft orbital elements at the input time.

INPUTS:
   Name           Description                              Units   Min    Max
   ----           -----------                              -----   ---    ---
   secTAI93       time in seconds since 12AM UTC 1/1/1993  sec

   spacecraftTag  unique spacecraft identification tag     n/a

   a              - semi-major axis             	   m
   e              - eccentricity                 	   n/a
   inc            - inclination                  	   radians
   an             - right ascension of ascending node      radians
   ap             - argument of perigee          	   radians
   ma             - mean anomaly at time  	           radians

OUTPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   data           keplerian orbital elements 
                  and other stuff
     data[0]      - semi-major axis           m      
     data[1]  	  - eccentricity              n/a    
     data[2]  	  - inclination               radians
     data[3] 	  - right ascension of               
                    ascending node            radians
     data[4]  	  - argument of perigee       radians
     data[5]  	  - mean anomaly at time      radians
     data[6]  	  - rate of change of RA of          
                    ascending node            rad/sec
     data[7]  	  - rate of change of                
                    argument of perigee       rad/sec
     data[8]  	  - mean motion               rad/sec
     data[9] 	  - period                    seconds

   orbitNum       orbit number since epoch    n/a

          
RETURNS:
   PGS_S_SUCCESS               successful return
   PGSTD_E_SC_TAG_UNKNOWN      unknown/unsupported spacecraft tag
   PGS_E_TOOLKIT               something bizarre and unexpected has occurred

NOTES:
   This function defines the orbit of a spacecraft via various orbital elements
   and their rates at epoch.  Then for a user specified time the time difference
   between the input time and epoch is calculated and the current orbital
   parameters are determined by simply iterating the orbit detlaT/orbitPeriod
   times.

FILES:
   None

GLOBALS:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTAI()

END_PROLOG:
*******************************************************************************/

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_SIM.h>

/*   gravitational constants for Earth (IAU 1976) */

#define  GE         3.986005e14          /* gravitational parameter */
#define  J2         0.00108263           /* oblateness term */
#define  EARTH_RAD  6378137.0            /* WGS84 Earth equatorial radius (m) */
#define  DEGtoRAD   .0174532925199433    /* degree to radian conversion */
#define  PGS_PI     3.141592653589793    /* pi */

/* define a macro to calculate the mean angular motion (rate of change of the
   mean anomaly), accounting for the perturbing affect of J2 (i.e. Earth's
   oblateness) */

#define  MADOT(a,e,i) (1.5*J2*(1.-1.5*pow(sin(i),2))*pow((EARTH_RAD/(a)),2)/  \
		       pow((1.0-(e)*(e)),1.5) + 1.)*sqrt(GE/pow(a,3))

/* define a macro to calculate the rate of change of the right ascension of the
   ascending node, accounting for the perturbing affect of J2 (i.e. Earth's
   oblateness) */

#define  ANDOT(dm,a,e,i) -1.50*(dm)*cos(i)*J2* \
                         pow((EARTH_RAD/(a)/(1.0-(e)*(e))),2)

PGSt_SMF_status      
PGS_EPH_orbitalElements(        /* calculate orbital elements for input s/c */
    PGSt_double  secTAI93,      /* time in seconds since 12AM UTC 1/1/1993 */
    PGSt_tag     spacecraftTag, /* unique spacecraft identification tag */
    PGSt_double  aIn,           /* semi-major axis (m) */
    PGSt_double  eIn,           /* eccentricity */
    PGSt_double  incIn,         /* inclination (radians) */
    PGSt_double  anIn,          /* right ascension of ascending node (radians) */
    PGSt_double  apIn,          /* argument of perigee (radians) */
    PGSt_double  maIn,          /* mean anomaly at time (radians) */
    PGSt_double  data[10],      /* keplerian orbital elements and other stuff */
    PGSt_integer *orbitNumber,  /* orbit number since epoch */
    PGSt_double  epoch_in)      /* epoch in CCSDS ASCII time code A */
{
    PGSt_double  t;             /* time in seconds since epoch */
    char         epoch_tm[28];  /* epoch in CCSDS ASCII time code A */
    static PGSt_double  inc;    /* inclination (rad) */
    static PGSt_double  ma0;    /* mean anomaly at epoch (rad) */
    static PGSt_double  madot;  /* mean motion (rad/sec) */
    static PGSt_double  epoch;  /* epoch in secs since 12AM UTC 1/1/1993 */
    static PGSt_double  a;      /* semi-major axis */
    static PGSt_double  e;      /* eccentricity */
    static PGSt_double  an0;    /* right ascension of ascending node at epoch*/
    static PGSt_double  ap0;    /* argument of perigee at epoch */
    static PGSt_double  apdot;  /* rate of change of perigee at epoch */
    static PGSt_double  andot;  /* rate of change of right ascension of
				   ascending node at epoch */

    static PGSt_tag     spacecraft=0;         /* s/c tag from previous call - 
						 initially set to nonsense */
    /* return value of PGSTK function called by this function */

    static PGSt_SMF_status returnStatus=PGS_E_TOOLKIT;

    /*   START EXECUTION: 
	 initialize orbital elements and compute element rates on first call
	 or if a new spacecraft tag has been specified (i.e. different from the
	 previous call) */

    a = aIn;
    e = eIn;
    inc = incIn*DEGtoRAD;
    an0 = anIn*DEGtoRAD;
    ap0 = apIn*DEGtoRAD;
    ma0 = maIn*DEGtoRAD;
    epoch = epoch_in;

    if (spacecraft != spacecraftTag)
	switch (spacecraftTag)
	{
	  case PGSd_TRMM:

	    /*   simulated TRMM epoch and orbital elements */
	    if(epoch_in < 0.0)
	      {
		strcpy(epoch_tm,"1997-10-01T23:00:00Z");
		returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
		if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		    returnStatus != PGSTD_E_NO_LEAP_SECS)
		  return PGS_E_TOOLKIT;
	      }
	    else
	      {
		returnStatus = PGS_S_SUCCESS;
	      }
            apdot=0.0*DEGtoRAD;             /* Is this really 0.0 for TRMM? */
	    madot=MADOT(a,e,inc);
	    andot=ANDOT(madot,a,e,inc);
	    spacecraft = spacecraftTag;
	    break;

	  case PGSd_EOS_AM:

	    /*   simulated EOS-AM epoch and orbital elements */
	    if(epoch_in < 0.0)
	      {
		strcpy(epoch_tm,"1998-06-30T10:51:28.32Z");
		returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
		if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		    returnStatus != PGSTD_E_NO_LEAP_SECS)
		  return PGS_E_TOOLKIT;
	      }
	    else
	      {
		returnStatus = PGS_S_SUCCESS;
	      }
            apdot=0.0*DEGtoRAD;             /* apdot=0.0 => frozen orbit */
	    madot=MADOT(a,e,inc);
	    andot=ANDOT(madot,a,e,inc);
	    spacecraft = spacecraftTag;
	    break;

	  case PGSd_EOS_PM:

	    /*   simulated EOS-PM epoch and orbital elements */
	    if(epoch_in < 0.0)
	      {
		strcpy(epoch_tm,"2000-12-01T10:51:28.32Z");
		returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
		if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		    returnStatus != PGSTD_E_NO_LEAP_SECS)
		  return PGS_E_TOOLKIT;
	      }
	    else
	      {
		returnStatus = PGS_S_SUCCESS;
	      }
            apdot=0.0*DEGtoRAD;             /* apdot=0.0 => frozen orbit */
	    madot=MADOT(a,e,inc);
	    andot=ANDOT(madot,a,e,inc);
	    spacecraft = spacecraftTag;
	    break;
	    
        case PGSd_EOS_AURA:
	  
	  /*   simulated EOS-PM epoch and orbital elements */
	  if(epoch_in < 0.0)
	    {
	      strcpy(epoch_tm,"1998-06-20T16:58:46.466Z");
	      returnStatus=PGS_TD_UTCtoTAI(epoch_tm,&epoch);
	      if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		  returnStatus != PGSTD_E_NO_LEAP_SECS)
		return PGS_E_TOOLKIT;
	    }
	  else
	    {
	      returnStatus = PGS_S_SUCCESS;
	    }
	  apdot=0.0*DEGtoRAD;             /* apdot=0.0 => frozen orbit */
	  madot=MADOT(a,e,inc);
	  andot=ANDOT(madot,a,e,inc);
	  spacecraft = spacecraftTag;
	  break;
/*
From FDD Osvaldo Cuevas:
June 20, 1998 at 165846.466 
SMA= 7078.5km 
Eccentricity = .00116 
Inclination = 98.2 
Asc Node = 111.195 degrees 
Arg of Perigee = 90 degrees 
Mean Anomaly = 270 
*/
	default:
	  return PGSTD_E_SC_TAG_UNKNOWN;
	} /* END: switch (spacecraftTag) */
    
    /* END: if (spacecraft != spacecraftTag) */

    /* return nodal period and element rates */

    data[9]=2.0*PGS_PI/(madot+apdot);  /* nodal period (seconds) */
    data[8]=madot;                     /* mean motion (rad/sec) */
    data[7]=apdot;                     /* rate of change of argument of perigee
					  (rad/sec) */
    data[6]=andot;                     /* rate of change of right ascension of
					  ascending node (rad/sec) */

    /* update elements based on time (seconds) since epoch */

    t=secTAI93-epoch;

    /* the following equations add the time since epoch (t) multiplied by the
       rate of change of the appropriate element (xxdot) to the initial
       condition of the element (xx0) to get the new condition of the element,
       and then--since these are angles--gets the remainder of the division by
       2PI to give an angle in the range [0,2PI).   NOTE: empirical evidence
       suggests that only about 8 decimal places of the output from fmod() in
       the cases where the remainders of the division of large numbers by 2*PI
       are being calculated are accurate.  Therefore a second fmod() is done to
       throw away the remaining precision in the original answers (since it is
       essentially meaningless). */

    data[5]=fmod((ma0 + t*madot),(2.0*PGS_PI)); /* mean anomaly (radians) */
    data[5] -= fmod(data[5],1.0E-8);
    data[4]=fmod((ap0 + t*apdot),(2.0*PGS_PI)); /* argument of perigee (rad) */
    data[4] -= fmod(data[4],1.0E-8);
    data[3]=fmod((an0 + t*andot),(2.0*PGS_PI)); /* right ascension of ascending
						   node (radians) */
    data[3] -= fmod(data[3],1.0E-8);
    data[2]=inc;                                /* inclination (radians) */
    data[1]=e;                                  /* eccentricity */
    data[0]=a;                                  /* semi-major axis (meters) */

    /* get orbit number based on mean motion and perigee precession */

    *orbitNumber = (PGSt_integer) (t*(madot + apdot)/(2.0*PGS_PI));

    return returnStatus;
}





