/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  PGS_CSC_precs2000.c

DESCRIPTION:
  This file contains the function PGS_CSC_precs2000().
  This function precesses a vector from Celestial Coordinates of date in
  Terrestrial Dynamical Time (TDT) to J2000 coordinates or from J2000 
  coordinates to  Celestial Coordinates of date in Terrestrial Dynamical Time
  (TDT).
      
AUTHOR:
  Peter D. Noerdlinger / Applied Research Corporation
  Guru Tej S Khalsa    / Applied Research Corporation
  Curt Schafer         / Steven Myers & Associates

HISTORY:
   10-May-1995  PDN/GTSK  Initial version, based on FORTRAN code acquired from
                          E. Myles Standish at JPL
   12-Jul-1999    CS      Updated for Threadsafe functionality

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:    
   Precesses a vector between TDT Julian Date and J2000 Coordinates

NAME:     
   PGS_CSC_precs2000()

SYNOPSIS:
C:
   #include <PGS_CSC.h>

   PGSt_SMF_status
   PGS_CSC_precs2000(
       PGSt_integer threeOr6,  
       PGSt_double  jedTDT[2],
       PGSt_boolean frwd,              
       PGst_double  posVel[])

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_TD.4.f'

      integer function pgs_csc_precs2000(threeor6,jedtdt,frwd,posvel)

      integer          threeor6
      double precision jedtdt(2)
      integer          frwd
      double precision posvel(6)

DESCRIPTION:
   This tool precesses a vector from Celestial Coordinates of date in
   Terrestrial Dynamical Time (TDT) to J2000 coordinates or from J2000 
   coordinates to  Celestial Coordinates of date in Terrestrial Dynamical Time
   (TDT).

INPUTS:
   Name      Description                         Units      Min        Max
   ----      -----------                         -----      ---        ---
   threeOr6  chooses a 3 or 6 dimensional        N/A        N/A        N/A
             vector to precess

   jedTDT[2] TDT (Terrestrial Dynamical Time)    days       ANY        ANY
             as a Julian Date to or from which
             the vector is to be processed
            
   frwd      flag for sense of precession:       T/F        N/A        N/A
             PGS_TRUE if precessing from J2000
             to jedTDT PGS_FALSE if precessing 
             from jedTDT to J2000

   posVel    Vector (position and velocity) 
             in final reference frame:
             posvel[0-2]     position            m          ANY        ANY
             posvel[3-5]     velocity            m/s        ANY        ANY
            
OUTPUTS:
   Name      Description                         Units      Min        Max
   ----      -----------                         -----      ---        ---
   posVel    Vector (position and velocity) 
             in final reference frame:
             posvel[0-2]     position            m          ANY        ANY
             posvel[3-5]     velocity            m/s        ANY        ANY
   
RETURNS:      
   PGS_S_SUCCESS                successful return 
   PGSCSC_E_BAD_ARRAY_SIZE      not a 3 or 6 vector
   PGSCSC_E_BAD_DIRECTION_FLAG  the value of the direction flag is not either
                                PGS_TRUE or PGS_FALSE
   PGSTSF_E_GENERAL_FAILURE     Bad return from PGS_TSF_GetMasterIndex() 
        
EXAMPLES:
C:      
    PGSt_SMF_status  returnStatus;
    PGSt_double      jedTDT[2]={2449720.5,0.25};
    PGSt_double      posVel[6]={6400000.0,-5000000.0,40000.0,
                                4000.0,7000.0,-6000.0};

    ** precess the vector **

    returnStatus = PGS_CSC_precs2000(6,jedTDT,PGS_TRUE,posVel);
    
    ** the input vector "posVel" has been overwritten with the precessed 
       value ** 

FORTRAN:
      implicit none
      integer           pgs_csc_precs2000
      integer           returnstatus
      integer           threeor6
      double precision  jedtdt(2)
      double precision  posvel(6)
     
      data jedtdt/2449720.5,0.25/
      data posvel/6400000.0,-5000000.0,40000.0,4000.0,7000.0,-6000.0/
      
      threeor6 = 6

      returnstatus = pgs_csc_precs2000(threeor6,jedtdt,frwd,posvel)

!  the input vector "posvel" has been overwritten with the precessed value

NOTES:
   This function is a simplified version of its precursor: PGS_CSC_precs3or6().
   This function is specific to the case of precessing to or from the epoch of
   J2000.  The various coefficients used are the constants that result for this
   epoch.

   This function produces an output vector that overwrites the input vector.
   The code was kept this way to preserve its heritage. The user is cautioned 
   that her/his input vector will be therefore be altered by this function. 
   The underlying rotation functions do not have this property.

   TIME ACRONYMS:

     TDT is:  Terrestrial Dynamical Time
     
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
   PGSTK -  0912, 0930, 1050

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
     PGSg_TSF_CSCPrecsold_jedTDT
     PGSg_TSF_CSCPrecsz
     PGSg_TSF_CSCPrecsze
     PGSg_TSF_CSCPrecsth

FILES:
   NONE
      
FUNCTIONS_CALLED:
   PGS_CSC_rotat3         - rotate position only; zero the velocity
   PGS_CSC_rotat6         - rotate position and velocity
   PGS_SMF_SetStaticMsg   - set error/status message
   PGS_TSF_GetMasterIndex   get the index for this thread
                                
END_PROLOG:
*******************************************************************************/

#include <PGS_CSC.h>
#include <PGS_TSF.h>


#define JD2000       2451545.0           /* Julian Date of 1/1/2000 */
#define TCENT        36525.0             /* Julian century  */
#define ARCSECperRAD 206264.80624709636  /* seconds per radian */
    
/* The following coefficients are the constant values obtained when the
   epoch time is J2000 */

/* N.B. P1 and Q1 have the same value, because the leading terms in zA and
   zeta have the same value; however, to keep consistency with Table 3.211.1
   both names are used */

#define P1 2306.2181   /* first term in zetaA, the last (leftmost) Z
			  rotation in Eq. (3.21-7) and Table 3.211.1 */
#define P2 0.30188     /* coefficient of t squared term in zetaA in
			  Table 3.211.1 */ 
#define P3 0.017998    /* coefficient of t cubed term in zetaA in
			  Table 3.211.1 */
#define Q1 2306.2181   /* first term in zA, the Z rotation in Eq. 
			  (3.21-7) and Table 3.211.1 */ 
#define Q2 1.09468     /* coefficient for second term in zA, the Z 
			  rotation in Eq. (3.21-7) and Table 3.211.1 */
#define Q3 0.018203    /* coefficient for third last term in zA, the Z 
			  rotation in Eq. (3.21-7) and Table 3.211.1 */
#define R1 2004.3109   /* first term in thetaA, Table 3.211.1  */
#define R2 -0.42665    /* coefficient of t squared in thetaA,
			  Table 3.211.1 */
#define R3 -0.041833   /* coefficient of t cubed in thetaA, Table 
			  3.211.1 */

/* scale for time rate of precession, altered by Peter D. Noerdlinger
   3/14/94 by inclusion of the factor 86,400 seconds per day */
    
#define SCALE     650922224962336.809 /* ARCSECperRAD*TCENT*86400.0 */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_precs2000()"

PGSt_SMF_status
PGS_CSC_precs2000(
    PGSt_integer threeOr6,     /* chooses a 3 or 6 dimensional vector to 
				  precess */
    PGSt_double  jedTDT[2],    /* TDT Julian Date to or from which the vector
				  is to be precessed */
    PGSt_boolean frwd,         /* PGS_TRUE if taking x from J2000 to jedTDT, 
				  PGS_FALSE if from jedTDT to J2000 */
    PGSt_double  posVel[])     /* vector to be transformed   */
{
    static PGSt_double  ze[2]; /* the angle zeta (precession about the celestial
				  pole of epoch) in Eq. 3.21-7, p. 103, Suppl.
				  to Astr. Almanac and its rate.  Rate converted
				  herein from arc seconds/day to arc sec/second 
				  by Peter Noerdlinger, March 14, 1994 */
    static PGSt_double  th[2]; /* the angle theta  (from the pole of epoch to 
				  that of date) in Eq. 3.21-7, p. 103, Suppl. to
				  Astron. Almanac and its rate. Rate converted 
				  herein from arc seconds/day to arc sec/second
				  by Peter Noerdlinger, March 14, 1994 */
    static PGSt_double  z[2];  /* the angle zA (precession about the celestial 
				  pole of date) in Eq. 3.21-7, p. 103, Suppl. to
				  Astron. Almanac and its rate. Rate converted 
				  herein from arc seconds/day to arc sec/second 
				  by Peter Noerdlinger, March 14, 1994 */
    PGSt_double         deltaT;/* jedTDT - J2000 in Julian Centuries */

    /* save time from previous calls */

    static PGSt_double  old_jedTDT=-1.E50;

    /* pointer to appropriate rotation function - either PGS_CSC_rotat3() or
       PGS_CSC_rotat6() */

    PGSt_SMF_status (*pgs_rotate)(PGSt_double*, PGSt_double*,
				  PGSt_integer, PGSt_double*);
    
#ifdef _PGS_THREADSAFE

    /* Declare variables used for THREADSAFE version to replace statics
        The local names are appended with TSF and globals are preceded with
        directory and function name        */

    PGSt_double old_jedTDTTSF;
    PGSt_double zeTSF[2];
    PGSt_double zTSF[2];
    PGSt_double thTSF[2];

    /* Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_double PGSg_TSF_CSCPrecsold_jedTDT[];
    extern PGSt_double PGSg_TSF_CSCPrecsz[][2];
    extern PGSt_double PGSg_TSF_CSCPrecsze[][2];
    extern PGSt_double PGSg_TSF_CSCPrecsth[][2];
    int masterTSFIndex;
    int xcnt;               /* initialization loop index */

    /* Get index    Then test for bad returns */

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ( masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX )
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* Initialize the variables used for the THREADSAFE version */

    old_jedTDTTSF = PGSg_TSF_CSCPrecsold_jedTDT[masterTSFIndex];
    for(xcnt=0;xcnt<2;xcnt++)
    {
        zeTSF[xcnt] = PGSg_TSF_CSCPrecsze[masterTSFIndex][xcnt];
        zTSF[xcnt] = PGSg_TSF_CSCPrecsz[masterTSFIndex][xcnt];
        thTSF[xcnt] = PGSg_TSF_CSCPrecsth[masterTSFIndex][xcnt];
    }

#endif

    switch ( threeOr6 )
    {
      case 3:
	pgs_rotate = PGS_CSC_rotat3;
	break;
      case 6:
	pgs_rotate = PGS_CSC_rotat6;
	break;
      default:
	PGS_SMF_SetStaticMsg(PGSCSC_E_BAD_ARRAY_SIZE,FUNCTION_NAME);  
	return PGSCSC_E_BAD_ARRAY_SIZE;
    }

#ifdef _PGS_THREADSAFE

    /*  Almost entire function is duplicated for the THREADSAFE version to
        protect statics  When a value is reassigned the global is updated
                         NO LOCKS  NO KEYS   4 GLOBALS  */


    /* Threadsafe protect: old_jedTDT    reassign its value for next use */

    if ((jedTDT[0] + jedTDT[1]) != old_jedTDTTSF)
    {
	old_jedTDTTSF = jedTDT[0] + jedTDT[1];
        PGSg_TSF_CSCPrecsold_jedTDT[masterTSFIndex] = old_jedTDTTSF;
	
	deltaT = ((jedTDT[0] - JD2000) + jedTDT[1])/TCENT;

	/* precession about the celestial pole of epoch */	

        /* Threadsafe protect: ze,   reassign its values for next use */

	zeTSF[0] = deltaT*(P1 + deltaT*(P2 + deltaT*P3))/ARCSECperRAD;
	zeTSF[1] = (P1 + deltaT*(2.0*P2 + deltaT*3.0*P3))/SCALE;
        PGSg_TSF_CSCPrecsze[masterTSFIndex][0] = zeTSF[0];
        PGSg_TSF_CSCPrecsze[masterTSFIndex][1] = zeTSF[1];

	/* precession about the celestial pole of date */

        /* Threadsafe protect: z,   reassign its values for next use */
	
	zTSF[0]  = deltaT*(Q1 + deltaT*(Q2 + deltaT*Q3))/ARCSECperRAD;
	zTSF[1]  = (Q1+deltaT*(2.0*Q2+deltaT*3.0*Q3))/SCALE;
        PGSg_TSF_CSCPrecsz[masterTSFIndex][0] = zTSF[0];
        PGSg_TSF_CSCPrecsz[masterTSFIndex][1] = zTSF[1];
	
	/* angle from the pole of epoch to that of date */

        /* Threadsafe protect: th,   reassign its values for next use */
	
	thTSF[0] = deltaT*(R1 + deltaT*(R2 + deltaT*R3))/ARCSECperRAD;  
	thTSF[1] = (R1 + deltaT*(2.0*R2 + deltaT*3.0*R3))/SCALE;
        PGSg_TSF_CSCPrecsth[masterTSFIndex][0] = thTSF[0];
        PGSg_TSF_CSCPrecsth[masterTSFIndex][1] = thTSF[1];
    }
    
    switch (frwd)
    {
      case PGS_TRUE:
	
	/* here for precessing from J2000 to jedTDT - see p. 103 of the
	   Explanatory Supplement to the Astronomica Almanac, 1994,
	   Eq. 3.21-7 */

        /* Threadsafe protect: th, ze, z   */

	pgs_rotate(posVel,zeTSF,-3,posVel);
	pgs_rotate(posVel,thTSF,+2,posVel);
	pgs_rotate(posVel,zTSF ,-3,posVel);
	break;

      case PGS_FALSE:
	
	/* here for precessing from jedTDT to J2000 (get inverse of previous 
	   transformation - reverse rotation angles and order) */

        /* Threadsafe protect: th, ze, z   */
        
	pgs_rotate(posVel,zTSF ,+3,posVel);
	pgs_rotate(posVel,thTSF,-2,posVel);
	pgs_rotate(posVel,zeTSF,+3,posVel);
	break;

#else

    if ((jedTDT[0] + jedTDT[1]) != old_jedTDT)
    {
	old_jedTDT = jedTDT[0] + jedTDT[1];
	
	deltaT = ((jedTDT[0] - JD2000) + jedTDT[1])/TCENT;

	/* precession about the celestial pole of epoch */	

	ze[0] = deltaT*(P1 + deltaT*(P2 + deltaT*P3))/ARCSECperRAD;
	ze[1] = (P1 + deltaT*(2.0*P2 + deltaT*3.0*P3))/SCALE;

	/* precession about the celestial pole of date */
	
	z[0]  = deltaT*(Q1 + deltaT*(Q2 + deltaT*Q3))/ARCSECperRAD;
	z[1]  = (Q1+deltaT*(2.0*Q2+deltaT*3.0*Q3))/SCALE;
	
	/* angle from the pole of epoch to that of date */
	
	th[0] = deltaT*(R1 + deltaT*(R2 + deltaT*R3))/ARCSECperRAD;  
	th[1] = (R1 + deltaT*(2.0*R2 + deltaT*3.0*R3))/SCALE;
    }
    
    switch (frwd)
    {
      case PGS_TRUE:
	
	/* here for precessing from J2000 to jedTDT - see p. 103 of the
	   Explanatory Supplement to the Astronomica Almanac, 1994,
	   Eq. 3.21-7 */

	pgs_rotate(posVel,ze,-3,posVel);
	pgs_rotate(posVel,th,+2,posVel);
	pgs_rotate(posVel,z ,-3,posVel);
	break;

      case PGS_FALSE:
	
	/* here for precessing from jedTDT to J2000 (get inverse of previous 
	   transformation - reverse rotation angles and order) */
        
	pgs_rotate(posVel,z ,+3,posVel);
	pgs_rotate(posVel,th,-2,posVel);
	pgs_rotate(posVel,ze,+3,posVel);
	break;

#endif

      default:
	PGS_SMF_SetStaticMsg(PGSCSC_E_BAD_DIRECTION_FLAG,FUNCTION_NAME);

	return PGSCSC_E_BAD_DIRECTION_FLAG;
    }

    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);

    return PGS_S_SUCCESS;
}
