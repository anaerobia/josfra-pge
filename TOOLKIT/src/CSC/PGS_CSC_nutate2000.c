/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  PGS_CSC_nutate2000.c

DESCRIPTION:
  This file contains the function PGS_CSC_nutate2000().
  This function transforms a vector under nutation from Celestial Coordinates of
  date in Terrestrial Dynamical Time (TDT) to J2000 coordinates or from J2000
  coordinates to Celestial Coordinates of date.
      
AUTHOR:
  Peter D. Noerdlinger / Applied Research Corporation
  Guru Tej S Khalsa    / Applied Research Corporation
  Deborah Foch         / Applied Research Corporation
  Anubha Singhal       / Applied Research Corporation
  Curt Schafer         / Steven Myers & Associates

HISTORY:
      Dec-1993  PDN      Acquired from E. Myles Standish at JPL  
   31-Jan-1994  GTSK     Converted from FORTRAN to C                
                         Commented and tidied up                   
   14-Mar-1994  PDN      Rates changed to per SI second            
                         (were per day)
   14-Mar-1994  DF       Edited for improved style
   31-Mar-1994  UP       Modified prolog
   20-Oct-1994  AS       Fixed prologs and code to conform to latest PGS/ECS
                         standards
   23-Mar-1995  GTSK     Code rewritten.  Total overhaul.
   12-Jul-1999   CS      Updated for Threadsafe functionality

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:  
   Nutate State Vector Between J2000 and Ephemeris Time (ET)

NAME:  
   PGS_CSC_nutate2000
  
SYNOPSIS:
C:      
   #include PGS_CSC.h

   PGSt_SMF_status
   PGS_CSC_nutate2000(
        PGSt_integer threeOr6,
        PGSt_double  jedTDT[2],           
        PGSt_double  dvnut[4],                                      
        PGSt_boolean frwd,        
        PGSt_double  posVel[])           

FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_CSC_4.f'

      integer function pgs_csc_nutate2000(threeor6,jedtdt,dvnut,frwd,
     >                                    posvel)
      integer           threeor6
      double precision  jedtdt(2)
      double precision  dvnut(4)
      double precision  frwd
      double precision  posvel(*)

DESCRIPTION:
   This tool transforms a vector under nutation from Celestial Coordinates of 
   date in Terrestrial Dynamical Time (TDT) to J2000 coordinates or from 
   J2000 coordinates to Celestial Coordinates of date.
     
   Explanation and References:

   In going from J2000 to epoch, first precess, then nutate, as explained on
   pp. 150 - 151 of the Explanatory Supplement to the Astronomical Almanac
   (1991); also see the routine PGS_CSC_wahr2(), whose output is needed as input
   here; it is expressly designed to do the nutation in coordinates of date, not
   in J2000. Henceforth, we denote the Explanatory Supplement as mentioned above
   as "ES".
   
INPUTS:
   Name     Description                         Units     Min        Max
   ----     -----------                         -----     ---        ---
   threeOr6 chooses a 3 or 6 dimensional        N/A       N/A        N/A
            vector to nutate

   jedTDT   TDT (Terrestrial Dynamical Time)    days      ANY        ANY
            a Julian Date to or from which         
            the vector is to be nutated
	    (this variable is generally
	    referred to in Toolkit code as
	    jedTDT)

   dvnut    the two nutation angles and their   rad/s     -1.e-11    1.e-11
            rates, output from "PGS_CSC_wahr2"
	    (this variable is generally
	    referred to in Toolkit code as
	    dvnut)

   posVel   Vector (position and velocity) in
            starting reference frame:
            posVel[0-2]    position             m          ANY        ANY
            posVel[3-5]    velocity             m/s        ANY        ANY

   frwd     flag for sense of nutation:         T/F        N/A        N/A
            PGS_TRUE if nutating from J2000
	    to et PGS_FALSE if nutating
	    from et to J2000

OUTPUTS:
   Name     Description                         Units     Min        Max
   ----     -----------                         -----     ---        ---
   posVel   Vector (position and velocity)
            in final reference frame:
	    posVel[0-2]      position           m         ANY        ANY
	    posVel[3-5]      velocity           m/s       ANY        ANY

RETURNS:  
   PGS_S_SUCCESS                successful return
   PGSCSC_E_BAD_ARRAY_SIZE      not a 3 or 6 vector
   PGSCSC_E_BAD_DIRECTION_FLAG  the value of the direction flag is not either
                                PGS_TRUE or PGS_FALSE
   PGSTSF_E_GENERAL_FAILURE     Bad return from   PGS_TSF_GetMasterIndex()

EXAMPLES:
C:      
    PGSt_SMF_status  returnStatus;
    PGSt_double      jedTDT[2]={2449720.5,0.25};
    PGSt_double      dvnut[4];
    PGSt_double      posVel[6]={6400000.0,-5000000.0,40000.0,
                                4000.0,7000.0,-6000.0};

    **  get the nutation angles and rates **

    PGS_CSC_wahr2(jedTDT,dvnut);

    ** nutate the vector **

    returnStatus = PGS_CSC_nutate2000(6,jedTDT,dvnut,PGS_TRUE,posVel);
    
    ** the input vector "posVel" has been overwritten with the nutated value ** 

FORTRAN:
      implicit none
      integer           pgs_csc_nutate2000
      integer           returnstatus
      integer           threeor6
      double precision  jedtdt(2)
      double precision  dvnut
      double precision  posvel(6)
     
      data jedtdt/2449720.5,0.25/
      data posvel/6400000.0,-5000000.0,40000.0,4000.0,7000.0,-6000.0/
      
      threeor6 = 6

!  get the nutation angles and rates

      returnstatus = pgs_csc_wahr2(jedtdt,dvnut)

!  nutate the vector

      returnstatus = pgs_csc_nutate2000(threeor6,jedtdt,dvnut,frwd,
     >                                  posvel)

!  the input vector "posvel" has been overwritten with the nutated value

NOTES:
   Purpose:  In the case of transforming from J2000, this function transforms 
   a vector (position and velocity) after precession from J2000 to the 
   correctly nutated coordinates -- i.e. the rotation (or Z) axis is along the
   Earth's angular velocity and the X axis is toward the equinox of date. 
   (Precession gives the mean equinox of date and the program rotates a vector 
   either to or from J2000, depending on the input flag.)

   In the opposite case, in going from arbitrary epoch to J2000, this function 
   nutates the vector to the "un-nutated" axis of date, after which it must be
   precessed to J2000 by the function PGS_CSC_precs2000().
  
   This code was modified so it now takes either a 3 or 6 dimensional vector.
   When 6 dimensions are used, they must be in the order (position, velocity)
   because the transformation of velocity is slightly different.  This function
   produces an output vector that overwrites the input vector.  The code was
   kept this way to preserve its heritage. The user is cautioned that her/his
   input vector will therefore be altered by this function.  The underlying 
   rotation functions do not have this property.

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

   Special Considerations:    

   This is not JPL certified code.
           
   Please do NOT change the names of any variables in this program!  Although
   this is not certified software, it is heritage software and any updates 
   that might be supplied will be impossible to install if names are changed.

REQUIREMENTS:
   PGSTK 0914, 0930, 1050

DETAILS:
   NONE

GLOBALS:
     PGSg_TSF_CSCNutateold_et
     PGSg_TSF_CSCNutateobm
     PGSg_TSF_CSCNutateobt
     PGSg_TSF_CSCNutatedpsi

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

#define JD2000        2451545.0             /* Julian Date of 1/1/2000 */
#define TCENT         36525.0               /* Julian century  */
#define ARCSECperRAD  206264.80624709636    /* arc-seconds per radian */
    
#define SCALE         650922224962336.809   /* ARCSECperRAD*TCENT*86400.0 */

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_nutate2000()"

PGSt_SMF_status
PGS_CSC_nutate2000(  
    PGSt_integer threeOr6,   /* chooses a 3 or 6 dimensional vector to nutate */
    PGSt_double  et[2],      /* input TDT Julian date (aka jedTDT) */
    PGSt_double  put[4],     /* nutation angles and rates (aka dvnut)
				dvnut[0] - nutation in longitude (radians)
				dvnut[1] - nutation in obliquity (radians)
				dvnut[2] - nut. rate in longitude (radians/sec)
				dvnut[3] - nut. rate in obliquity (rad/sec) */
    PGSt_boolean frwd,       /* direction of nutation - to or from J2000 */
    PGSt_double  x[])        /* state vector (posVel in PROLOG above) */
{   
    /* In the following section please refer to pp. 114 - 115 of the Explan. 
       Suppl.  Equation numbers are from that section */

    PGSt_double        t;      /* Julian Ephemeris Centuries since J2000 */ 

    static PGSt_double obm[2]; /* Mean Obliquity of the Ecliptic Eq. 3.222.1 */
    static PGSt_double obt[2]; /* True Obliquity of the Ecliptic Eq. 3.222-2 */
    static PGSt_double dpsi[2];/* Nutation in Longitude and its rate (p. 114) */

    static PGSt_double old_et=-1.E50;
    
    /* pointer to appropriate rotation function - either PGS_CSC_rotat3() or
       PGS_CSC_rotat6() */

    PGSt_SMF_status (*pgs_rotate)(PGSt_double*, PGSt_double*,
				  PGSt_integer, PGSt_double*);
    
#ifdef _PGS_THREADSAFE

    /* Declare variables used for THREADSAFE version to replace statics
        The local names are appended with TSF and globals are preceded with
        directory and function name   */

    PGSt_double old_etTSF;
    PGSt_double obmTSF[2];
    PGSt_double obtTSF[2];
    PGSt_double dpsiTSF[2];
 
    /*  Globals     originals in PGS_TSF_SetupCSC.c */

    extern PGSt_double PGSg_TSF_CSCNutateold_et[];
    extern PGSt_double PGSg_TSF_CSCNutateobm[][2];
    extern PGSt_double PGSg_TSF_CSCNutateobt[][2];
    extern PGSt_double PGSg_TSF_CSCNutatedpsi[][2];
    int masterTSFIndex;
    int xcnt;        /* init loop index */

    /* Get index  and initialize keys  Then test for bad returns */

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ( masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* Initialize the variables used for the THREADSAFE version */

    old_etTSF = PGSg_TSF_CSCNutateold_et[masterTSFIndex];
    for(xcnt=0;xcnt<2;xcnt++)
    {
        obmTSF[xcnt] = PGSg_TSF_CSCNutateobm[masterTSFIndex][xcnt];
        obtTSF[xcnt] = PGSg_TSF_CSCNutateobt[masterTSFIndex][xcnt];
        dpsiTSF[xcnt] = PGSg_TSF_CSCNutatedpsi[masterTSFIndex][xcnt];
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
                         NO LOCKS  NO KEYS   4  GLOBALS  */

    /* Threadsafe Protect: old_et     Reassign its value for next use */

    if (old_etTSF != (et[0] + et[1]))
    {
        old_etTSF = et[0] + et[1];
	 PGSg_TSF_CSCNutateold_et[masterTSFIndex] = old_etTSF;
	/* Julian TDT Ephemeris Centuries since J2000 */

	t = ((et[0] - JD2000) + et[1])/TCENT;

	/******  IAU REVISED CONSTANTS  ******/
        /* Threadsafe Protect: obm     Reassign its value for next use */

	obmTSF[0]=(84381.448+t*(-46.815+t*(-0.00059+t*0.001813)))/ARCSECperRAD;  
				/* Eq. 3.222.1 */
          PGSg_TSF_CSCNutateobm[masterTSFIndex][0] = obmTSF[0];
	
	/* the constant 84381.448 may be recognized as the obliquity of the 
	   ecliptic, in seconds of arc, at J2000.0 - see p. 103, Explan. Suppl. 
	   The other terms are time variation.  See Eq. 3.222-1 of the ES. */
	
	/* The following (obm[1]) is the Time derivative of previous eq. */
        /* Threadsafe Protect: obm     Reassign its value for next use */
	
	obmTSF[1]=(-46.815-0.00118*t+0.005439*t*t)/SCALE;
          PGSg_TSF_CSCNutateobm[masterTSFIndex][1] = obmTSF[1];
	
	/* The constant 46.815 is an annual rate in seconds of arc described in
	   Table 3.211.1 of the ES.  The constant 0.00117 there giving the time
	   rate seems to be replaced with 0.00118 here, which comes from 2 times
	   0.00059 (the 2 is from differentiating x t squared). Programmer's
	   Note: The 86400 factor in the denominator was added by Peter
	   Noerdlinger 2/25/94 to implement SI units (angular velocity in
	   radians per second, not radians per day.  The rates coming from Wahr2
	   are assumed to be adjusted by the user to radians per second!) */
        /* Threadsafe Protect: dpsi, obt     Reassign its value for next use */
	
	dpsiTSF[0]=put[0];       /* put nutation from Wahr2 routine into dpsi
			         array */
	dpsiTSF[1]=put[2];       /* nutation rate  */
          PGSg_TSF_CSCNutatedpsi[masterTSFIndex][0] = dpsiTSF[0];
          PGSg_TSF_CSCNutatedpsi[masterTSFIndex][1] = dpsiTSF[1];
	obtTSF[0]=obmTSF[0]+put[1]; /* total obliquity epsilon0 + delta epsilon 
			         (p. 114) */
          PGSg_TSF_CSCNutateobt[masterTSFIndex][0] = obtTSF[0];
	obtTSF[1]=obmTSF[1]+put[3]; /* rate of change of the total obliquity */
          PGSg_TSF_CSCNutateobt[masterTSFIndex][1] = obtTSF[1];
    }

    /* The following rotations are described on p. 114 of the Expl. Suppl. 
       The rotation R1(epsilon), on the right in Eq. (3.222-3) must be
       applied first, then R3(-delta psi), then R1(- epsilon) */
    /* Threadsafe Protect: obm, dpsi, obt  while passing to pgs_rotate()  */
    
    switch (frwd)
    {
      case PGS_TRUE:  
	pgs_rotate(x,obmTSF,+1,x);
	pgs_rotate(x,dpsiTSF,-3,x);
	pgs_rotate(x,obtTSF,-1,x);
	break;
	
      case PGS_FALSE:
	
	/* to reverse transformation reverse signs and
	   order of operations */
	
	pgs_rotate(x,obtTSF,+1,x);
	pgs_rotate(x,dpsiTSF,+3,x);
	pgs_rotate(x,obmTSF,-1,x);
	break;
	

#else




    if (old_et != (et[0] + et[1]))
    {
        old_et = et[0] + et[1];
	
	/* Julian TDT Ephemeris Centuries since J2000 */

	t = ((et[0] - JD2000) + et[1])/TCENT;

	/******  IAU REVISED CONSTANTS  ******/
	
	obm[0]=(84381.448+t*(-46.815+t*(-0.00059+t*0.001813)))/ARCSECperRAD;  
				/* Eq. 3.222.1 */
	
	/* the constant 84381.448 may be recognized as the obliquity of the 
	   ecliptic, in seconds of arc, at J2000.0 - see p. 103, Explan. Suppl. 
	   The other terms are time variation.  See Eq. 3.222-1 of the ES. */
	
	/* The following (obm[1]) is the Time derivative of previous eq. */
	
	obm[1]=(-46.815-0.00118*t+0.005439*t*t)/SCALE;
	
	/* The constant 46.815 is an annual rate in seconds of arc described in
	   Table 3.211.1 of the ES.  The constant 0.00117 there giving the time
	   rate seems to be replaced with 0.00118 here, which comes from 2 times
	   0.00059 (the 2 is from differentiating x t squared). Programmer's
	   Note: The 86400 factor in the denominator was added by Peter
	   Noerdlinger 2/25/94 to implement SI units (angular velocity in
	   radians per second, not radians per day.  The rates coming from Wahr2
	   are assumed to be adjusted by the user to radians per second!) */
	
	dpsi[0]=put[0];       /* put nutation from Wahr2 routine into dpsi
			         array */
	dpsi[1]=put[2];       /* nutation rate  */
	obt[0]=obm[0]+put[1]; /* total obliquity epsilon0 + delta epsilon 
			         (p. 114) */
	obt[1]=obm[1]+put[3]; /* rate of change of the total obliquity */
    }

    /* The following rotations are described on p. 114 of the Expl. Suppl. 
       The rotation R1(epsilon), on the right in Eq. (3.222-3) must be
       applied first, then R3(-delta psi), then R1(- epsilon) */
    
    switch (frwd)
    {
      case PGS_TRUE:  
	pgs_rotate(x,obm,+1,x);
	pgs_rotate(x,dpsi,-3,x);
	pgs_rotate(x,obt,-1,x);
	break;
	
      case PGS_FALSE:
	
	/* to reverse transformation reverse signs and
	   order of operations */
	
	pgs_rotate(x,obt,+1,x);
	pgs_rotate(x,dpsi,+3,x);
	pgs_rotate(x,obm,-1,x);
	break;
	




#endif


      default:
	PGS_SMF_SetStaticMsg(PGSCSC_E_BAD_DIRECTION_FLAG,FUNCTION_NAME);
	return PGSCSC_E_BAD_DIRECTION_FLAG;
    }

    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);
    return PGS_S_SUCCESS;
}
