/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:  
  PGS_CSC_wahr2.c
 
DESCRIPTION:
  This file contains the function PGS_CSC_wahr2().
      
AUTHOR:
  Peter D. Noerdlinger / Applied Research Corporation
  Guru Tej S Khalsa    / Applied Research Corporation
  Deborah Foch         / Applied Research Corporation
  Anubha Singhal       / Applied Research Corporation
  Urmila Prasad        / Applied Research Corporation
  Curt Schafer         / Steven Myers & Associates
 
HISTORY:
      Dec-1993  PDN      Acquired from E. Myles Standish at JPL  
   31-Jan-1994  GTSK     Converted from FORTRAN to C                
                         Commented and tidied up                   
   14-Mar-1994  DF       Edited for improved style
   31-Mar-1994  UP       Modified prolog
   20-Oct-1994  AS       Fixed prologs and code to conform to latest PGS/ECS
                         standards
   26-May-1995  GTSK     Split out from file PGS_CSC_EARTH_MOTION.c, input
                         changed from single PGSt_double to array of two
			 PGSt_doubles, added FORTRAN binding, CHANGED RATES TO
			 RADIANS/SEC FROM RADIAN/DAY!!!
   15-Jul-1999   CS      Updated for Threadsafe functionality
 
END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:   
   Calculate Nutation Angles

NAME:    
   PGS_CSC_wahr2()

SYNOPSIS:
C:
   #include PGS_CSC.h
  
   PGS_CSC_wahr2(
       PGSt_double ddjd[2],
       PGSt_double dvnut[4])

FORTRAN:
      include 'PGS_SMF.f'

      integer function pgs_csc_wahr2(ddjd,dvnut)
      double precision ddjd(2)
      double precision dvnut(4)

DESCRIPTION:
   Calculates nutation angles delta psi and delta epsilon, and their rates of 
   change, referred to the ecliptic of date, from the Wahr series.  
   
INPUTS:
   Name      Description                   Units        Min        Max
   ----      -----------                   -----        ---        ---  
   ddjd[2]   Barycentric Dynamical Time    days         ANY        ANY
             as a Julian Date

     ddjd[0] half-integral Julian day
     ddjd[1] Julian day fraction

OUTPUTS:
   Name      Description                   Units        Min        Max
   ----      -----------                   -----        ---        ---     
   dvnut[0]  nutation in longitude         radians      -0.01      0.01
   dvnut[1]  nutation in obliquity         radians      -0.001     0.001
   dvnut[2]  nutation rate in longitude    radians/sec  -1.16e-1   +1.16e-11
   dvnut[3]  nutation rate in obliquity    radians/sec  -1.16e-13  +1.16e-13
  
RETURNS:            
   PGS_S_SUCCESS       successful return
   PGSTSF_E_GENERAL_FAILURE     Bad return from   PGS_TSF_GetMasterIndex()

EXAMPLES:
C:
    PGSt_SMF_status  returnStatus;
    PGSt_double      jedTDT[2]={2449720.5,0.25};
    PGSt_double      dvnut[4];

    returnStatus = PGS_CSC_wahr2(jedTDT,dvnut);

    ** do something with shiny new nutation angles and rates **
                :
                :

FORTRAN:
     implicit none
     integer          pgs_csc_wahr2
     integer          returnstatus
     double precision jedtdb(2)
     double precision dvnut(4)

     data jedtdb/2449720.5,0.25/

     returnstatus = pgs_csc_wahr2(jedtdb,dvnut)

!  do something with shiny new nutation angles and rates
                :
                :

NOTES:
   from table 1,'proposal to the IAU working group on nutation', John M. Wahr 
   and Martin L. Smith (1979) subroutine to compute nutation angles and rates 
   from expressions given in Supplement to Astronomical Almanac 1984, S21-S26.
   Ref: P.K. Seidelmann, V.K. Abalakin, H. Kinoshita, J. Kovalevsky, C.A. 
   Murray, M.L. Smith, R.O. Vicente, J.G. Williams, Ya. S. Yatskiv: 1982, 
   "1980 IAU Theory of Nutation", Celestial Mechanics Journal, 
   vol 27., p. 79-105                
              
   Changes to code prior to acquisition for ECS project:

   Lieske 3/91.  NUTATION in the IAU J2000 system. Univac version obtained from
   Myles Standish, (subroutine WAHR) who had obtained it from USNO.  Re-ordered
   terms to match Astronomical Almanac 1984 table S23-S25 and corrected the rate
   for dPsi in the 0 0 2 -2 2 term. Eliminated the equivalences, common block
   and added necessary SAVEs. Corrected the fundamental angles (L, L', F, D,
   Node) to match Almanac.

   Acquired from E. Myles Standish, JPL, 12/93 by Peter Noerdlinger. This is
   personal and not JPL certified code. Please do not modify the names of the
   variables in this code. It is heritage code and we may receive updates. We
   may also receive other related code with the same names for variables.

   Users concerned with speed may wish to avoid repeated calls where possible.
   In this regard, the rates that are provided by Wahr2 can be used either for
   estimating the error of using nearby times, or for short term
   extrapolation. Note that in the original JPL code the rates issued by wahr2
   are in radians per day, this routine returns the rates as radians per second.

   The author is indebted to Guru Tej Khalsa of the Applied Research Corp.  for
   translating the code from FORTRAN to C and for valuable comments.
                                                                              
REQUIREMENTS:
   PGSTK - 0916, 0930, 1050

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   PGSg_TSF_CSCWahr2first

FILES:
   NONE
    
FUNCTIONS_CALLED:  
   PGS_TSF_GetMasterIndex   get the index for this thread
    
END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>
#include <PGS_TSF.h>

#define SECONDSperDAY    86400.0    /* number of seconds in a day */

#define NTERM 106

PGSt_SMF_status
PGS_CSC_wahr2(                /* calculates nutation angles */
    PGSt_double ddjd[2],      /* julian date (TDT) */
    PGSt_double dvnut[4])     /* nutation angles and rates
				 dvnut[0] - nutation in longitude (radians)
				 dvnut[1] - nutation in obliquity (radians)
				 dvnut[2] - nut. rate in longitude (radians/sec)
				 dvnut[3] - nut. rate in obliquity (rad/sec) */
{
    PGSt_double dangle[5];    /* Angles defined in Table 3.222.2 on p. 114 of 
                                 the Explan. Supplement to the Astronomical 
                                 Almanac (1992). */
    PGSt_double dangrt[5];    /* Rates of change of dangle[5]   */
    PGSt_double dj;           /* Julian Ephemeris (TDT) days from J2000  */ 
    PGSt_double dd;           /* one ten thousandth of dj   */
    PGSt_double t;            /* Julian Ephemeris (TDT) centuries from J2000 */
                              /* N.b. : a Julian year is 365.25 days of 
                                 86400.0 SI seconds each exactly */
    PGSt_double cl;           /* cumulant for the quantity S(i) entering the 
                                 sum for delta psi (nutation in longitude) in 
                                 the equation just above Eq.(3.222-6) on 
                                 p. 115 of the Explan.Suppl. */
    PGSt_double ce;           /* cumulant for the quantity S(i) entering the 
                                 sum for delta epsilon (the nutation in 
                                 obliquity) in the equation just above 
                                 Eq.(3.222-6) on p. 115 of the Explan. Suppl. */
    PGSt_double cosang;       /* cos A(i) in the equation for delta epsilon 
                                 just above eq. (3.222-6) on p. 115 of the 
                                 Exp. Suppl. */
    PGSt_double sinang;       /* sin A(i) in the equation for delta psi just 
                                 above eq. (3.222-6)  on p. 115 of the Exp. 
                                 Suppl. */
    PGSt_double darg;         /* the argument of cosang and sinang 
                                 (see above)   */
    PGSt_double dargrt;       /* rate of change of darg (see above)   */
    PGSt_double dpi;          /* pi=3.141592653589793238.. as a PGSt_double */
    PGSt_double rasec;        /* seconds of arc per radian */
    PGSt_double dzero = 0.0;  /* zero as a PGSt_double */
    PGSt_double done  = 1.0;  /* unity  as a PGSt_double*/
 
    register int i;           /* dummy index for sums  */
    register int j;           /* dummy index for sums  */

    /* making them static saves: first, dtwopi, radian, factr between calls */
    /* The statics factr, dtwopi, radian are Thread Specific data because their
       values are assigned as soon as the function starts and are not
       reassigned */

    static PGSt_double factr; /*  10**4 times number of seconds per radian  */
    static PGSt_boolean first=PGS_TRUE; /* flag to save setup time in subsequent
					   passes  */
    static PGSt_double dtwopi;/*  two pi = 2*3.141592653589793...  */
    static PGSt_double radian;/*  degrees per radian (approximately 57.3  */

    /* matrix[][1...9] contains coefficients: 
       	  L, Lprime, F, D, Node, dPsi, dPsiDot, dObl, dOblDot
       where dPsi units of 0.0001 arcsec, dPsiDot 10-5 arcsec/cent */
 
    /* Periods: 6798.4, 3399.2, 1305.5, 1095.2, 1615.7, 3232.9, 6786.3,
                943.2, 182.6, 365.3, 121.7, 365.2, 177.8, 205.9, 173.3,
                182.6, 386.0, 91.3, 346.6 */

    static int matrix[NTERM][9]=
    {
 	{0,     0,     0,     0,     1, -171996, -1742, 92025,    89},
 	{0,     0,     0,     0,     2,    2062,     2,  -895,     5},
       {-2,     0,     2,     0,     1,      46,     0,   -24,     0},
 	{2,     0,    -2,     0,     0,      11,     0,     0,     0},
       {-2,     0,     2,     0,     2,      -3,     0,     1,     0},
 	{1,    -1,     0,    -1,     0,      -3,     0,     0,     0},
 	{0,    -2,     2,    -2,     1,      -2,     0,     1,     0},
 	{2,     0,    -2,     0,     1,       1,     0,     0,     0},
 	{0,     0,     2,    -2,     2,  -13187,   -16,  5736,   -31},
 	{0,     1,     0,     0,     0,    1426,   -34,    54,    -1},
 	{0,     1,     2,    -2,     2,    -517,    12,   224,    -6},
 	{0,    -1,     2,    -2,     2,     217,    -5,   -95,     3},
 	{0,     0,     2,    -2,     1,     129,     1,   -70,     0},
 	{2,     0,     0,    -2,     0,      48,     0,     1,     0},
 	{0,     0,     2,    -2,     0,     -22,     0,     0,     0},
 	{0,     2,     0,     0,     0,      17,    -1,     0,     0},
 	{0,     1,     0,     0,     1,     -15,     0,     9,     0},
 	{0,     2,     2,    -2,     2,     -16,     1,     7,     0},
 	{0,    -1,     0,     0,     1,     -12,     0,     6,     0},
	
	/* Periods: 199.8, 346.6, 212.3, 119.6, 411.8, 131.7, 169.0, 329.8,
 	            409.2, 388.3, 117.5, 13.7, 27.6, 13.6, 9.1, 31.8, 27.1,
		    14.8,  27.7 */
	
       {-2,     0,     0,     2,     1,      -6,     0,     3,     0},
 	{0,    -1,     2,    -2,     1,      -5,     0,     3,     0},
 	{2,     0,     0,    -2,     1,       4,     0,    -2,     0},
 	{0,     1,     2,    -2,     1,       4,     0,    -2,     0},
 	{1,     0,     0,    -1,     0,      -4,     0,     0,     0},
 	{2,     1,     0,    -2,     0,       1,     0,     0,     0},
 	{0,     0,    -2,     2,     1,       1,     0,     0,     0},
 	{0,     1,    -2,     2,     0,      -1,     0,     0,     0},
 	{0,     1,     0,     0,     2,       1,     0,     0,     0},
       {-1,     0,     0,     1,     1,       1,     0,     0,     0},
 	{0,     1,     2,    -2,     0,      -1,     0,     0,     0},
 	{0,     0,     2,     0,     2,   -2274,    -2,   977,    -5},
 	{1,     0,     0,     0,     0,     712,     1,    -7,     0},
 	{0,     0,     2,     0,     1,    -386,    -4,   200,     0},
 	{1,     0,     2,     0,     2,    -301,     0,   129,    -1},
 	{1,     0,     0,    -2,     0,    -158,     0,    -1,     0},
       {-1,     0,     2,     0,     2,     123,     0,   -53,     0},
 	{0,     0,     0,     2,     0,      63,     0,    -2,     0},
 	{1,     0,     0,     0,     1,      63,     1,   -33,     0},
	
	/* Periods: 27.4, 9.6, 9.1, 7.1, 13.8, 23.9, 6.9, 13.6, 27.0, 32.0,
 	            {31.7, 9.5, 34.8, 13.2, 14.2, 5.6, 9.6, 12.8}, 14.8 */
	
       {-1,     0,     0,     0,     1,     -58,    -1,    32,     0},
       {-1,     0,     2,     2,     2,     -59,     0,    26,     0},
 	{1,     0,     2,     0,     1,     -51,     0,    27,     0},
 	{0,     0,     2,     2,     2,     -38,     0,    16,     0},
 	{2,     0,     0,     0,     0,      29,     0,    -1,     0},
 	{1,     0,     2,    -2,     2,      29,     0,   -12,     0},
 	{2,     0,     2,     0,     2,     -31,     0,    13,     0},
 	{0,     0,     2,     0,     0,      26,     0,    -1,     0},
       {-1,     0,     2,     0,     1,      21,     0,   -10,     0},
       {-1,     0,     0,     2,     1,      16,     0,    -8,     0},
 	{1,     0,     0,    -2,     1,     -13,     0,     7,     0},
       {-1,     0,     2,     2,     1,     -10,     0,     5,     0},
 	{1,     1,     0,    -2,     0,      -7,     0,     0,     0},
 	{0,     1,     2,     0,     2,       7,     0,    -3,     0},
 	{0,    -1,     2,     0,     2,      -7,     0,     3,     0},
 	{1,     0,     2,     2,     2,      -8,     0,     3,     0},
 	{1,     0,     0,     2,     0,       6,     0,     0,     0},
 	{2,     0,     2,    -2,     2,       6,     0,    -3,     0},
 	{0,     0,     0,     2,     1,      -6,     0,     3,     0},
	
	/* Periods: 7.1, 23.9, 14.7, 29.8, 6.9, 15.4, 26.9, 29.5, 25.6, 9.1,
	            9.4, 9.8, 13.7, 5.5, 7.2, 8.9, 32.6, 13.8, 27.8 */
	
 	{0,     0,     2,     2,     1,      -7,     0,     3,     0},
 	{1,     0,     2,    -2,     1,       6,     0,    -3,     0},
 	{0,     0,     0,    -2,     1,      -5,     0,     3,     0},
 	{1,    -1,     0,     0,     0,       5,     0,     0,     0},
 	{2,     0,     2,     0,     1,      -5,     0,     3,     0},
 	{0,     1,     0,    -2,     0,      -4,     0,     0,     0},
 	{1,     0,    -2,     0,     0,       4,     0,     0,     0},
 	{0,     0,     0,     1,     0,      -4,     0,     0,     0},
 	{1,     1,     0,     0,     0,      -3,     0,     0,     0},
 	{1,     0,     2,     0,     0,       3,     0,     0,     0},
 	{1,    -1,     2,     0,     2,      -3,     0,     1,     0},
       {-1,    -1,     2,     2,     2,      -3,     0,     1,     0},
       {-2,     0,     0,     0,     1,      -2,     0,     1,     0},
 	{3,     0,     2,     0,     2,      -3,     0,     1,     0},
 	{0,    -1,     2,     2,     2,      -3,     0,     1,     0},
 	{1,     1,     2,     0,     2,       2,     0,    -1,     0},
       {-1,     0,     2,    -2,     1,      -2,     0,     1,     0},
 	{2,     0,     0,     0,     1,       2,     0,    -1,     0},
 	{1,     0,     0,     0,     2,      -2,     0,     1,     0},
	
	/* Periods: 9.2, 9.3, 27.3, 10.1, 14.6, 5.8, 15.9, 22.5, 5.6, 7.3, 9.1,
	            29.3, 12.8, 4.7, 9.6, 12.7, 8.7, 23.8, 13.1 */
	
 	{3,     0,     0,     0,     0,       2,     0,     0,     0},
 	{0,     0,     2,     1,     2,       2,     0,    -1,     0},
       {-1,     0,     0,     0,     2,       1,     0,    -1,     0},
 	{1,     0,     0,    -4,     0,      -1,     0,     0,     0},
       {-2,     0,     2,     2,     2,       1,     0,    -1,     0},
       {-1,     0,     2,     4,     2,      -2,     0,     1,     0},
 	{2,     0,     0,    -4,     0,      -1,     0,     0,     0},
 	{1,     1,     2,    -2,     2,       1,     0,    -1,     0},
 	{1,     0,     2,     2,     1,      -1,     0,     1,     0},
       {-2,     0,     2,     4,     2,      -1,     0,     1,     0},
       {-1,     0,     4,     0,     2,       1,     0,     0,     0},
 	{1,    -1,     0,    -2,     0,       1,     0,     0,     0},
 	{2,     0,     2,    -2,     1,       1,     0,    -1,     0},
 	{2,     0,     2,     2,     2,      -1,     0,     0,     0},
 	{1,     0,     0,     2,     1,      -1,     0,     0,     0},
 	{0,     0,     4,    -2,     2,       1,     0,     0,     0},
 	{3,     0,     2,    -2,     2,       1,     0,     0,     0},
 	{1,     0,     2,    -2,     0,      -1,     0,     0,     0},
 	{0,     1,     2,     0,     1,       1,     0,     0,     0},
	
	/* Periods: 35.0, 13.6, 25.4, 14.2, 9.5, 14.2, 34.7, 32.8, 7.1, 4.8, 
	            27.3 */
	
       {-1,    -1,     0,     2,     1,       1,     0,     0,     0},
 	{0,     0,    -2,     0,     1,      -1,     0,     0,     0},
 	{0,     0,     2,    -1,     2,      -1,     0,     0,     0},
 	{0,     1,     0,     2,     0,      -1,     0,     0,     0},
 	{1,     0,    -2,    -2,     0,      -1,     0,     0,     0},
 	{0,    -1,     2,     0,     1,      -1,     0,     0,     0},
 	{1,     1,     0,    -2,     1,      -1,     0,     0,     0},
 	{1,     0,    -2,     2,     0,      -1,     0,     0,     0},
 	{2,     0,     0,     2,     0,       1,     0,     0,     0},
 	{0,     0,     2,     4,     2,      -1,     0,     0,     0},
 	{0,     1,     0,     1,     0,       1,     0,     0,     0}
    };
    
#ifdef _PGS_THREADSAFE
    PGSt_boolean firstTSF; 
    int masterTSFIndex;
    
    /* 1 GLOBAL  */
   
    extern PGSt_boolean PGSg_TSF_CSCWahr2first[];


    /* Get index    Then test for bad return */

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ( masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* Initialize the variables used for the THREADSAFE version */

    firstTSF = PGSg_TSF_CSCWahr2first[masterTSFIndex];
    
    /* Threadsafe Protect: first    Reassign value to global */

    if (firstTSF)
    {
        firstTSF = PGS_FALSE;
        PGSg_TSF_CSCWahr2first[masterTSFIndex] = firstTSF;
#else
    if (first)
    {
        first = PGS_FALSE;
#endif
        dpi = 4.0*atan(done);
        dtwopi = 2.0*dpi;
        radian = 180.0/dpi;
        rasec = 3600.0*radian;
        factr = 1.e4*rasec;
    }

    /* End setup; Save first, dtwopi, radian, factr */
    
    dj = (ddjd[0] - 2451545.0) + ddjd[1];
    dd = dj/1.e4;
    t = dj/36525.0;
     
    dangle[0] = 134.9629814 + 13.0649929472*dj + dd*dd*(.0006519+.000000365*dd);
    dangle[1] = 357.527723 + .9856002831*dj - dd*dd*(.0000120 +.000000068*dd);
    dangle[2] = 93.271910 + 13.2293502406*dj + dd*dd*(-.0002760 +.000000063*dd);
    dangle[3] = 297.850363 + 12.1907491165*dj + dd*dd*(-.0001435 + 
                .000000108*dd);
    dangle[4] = 125.044522 - .0529537648*dj + dd*dd*(.0001552 + .000000046*dd);
    /*            ^    */
    /*            |    */
    /* NOTE:  This number is incorrectly listed as 135 degrees, 2 minutes, 
       40.28 seconds arc in the last line of Table 3.222.2 on p. 114 of the 
       Explan. Supplement to the Astronomical Almanac (1992). The number as 
       given herein to EOSDIS by E. Myles Standish checks against the 1994 
       A.A. itself, p. D2, where Omega, the longitude of the mean ascending 
       node of on the ecliptic, as measured from the mean equinox of date, is
       241.145659 degrees on 1994 Jan 0 at 0 hours TDT.  The difference
       125.044522, (NOT 135.044522) is from the terms in dj and dd  */
    
    dangrt[0] = 13.0649929472 + dd*( 0.0013039e-4 + 0.000001095e-4*dd);
    dangrt[1] =   .9856002831 - dd*( 0.0000240e-4 + 0.000000205e-4*dd);
    dangrt[2] = 13.2293502406 + dd*(-0.0005521e-4 + 0.000000188e-4*dd);
    dangrt[3] = 12.1907491165 + dd*(-0.0002880e-4 + 0.000000325e-4*dd);
    dangrt[4] =  -.0529537648 + dd*( 0.0003104e-4 + 0.000000137e-4*dd);
    
    for(j=0;j<5;j++)
    {
	dangle[j] = fmod(dangle[j], 360.);
	dangle[j] = dangle[j]/radian;
	dangrt[j] = fmod(dangrt[j], 360.);
	dangrt[j] = dangrt[j]/radian;
    }

    for(j=0;j<4;j++)
    {
	dvnut[j] = dzero;
    }
       
    for(i=0;i<NTERM;i++)
    {
	darg = 0.;
	dargrt = 0.;
	
	for(j=0;j<5;j++)
	{
	    if (matrix[i][j] != 0)
	    {
		darg = darg + matrix[i][j]*dangle[j];
		dargrt = dargrt + matrix[i][j]*dangrt[j];
		darg = fmod(darg, dtwopi);
	    }
	}
	cl = matrix[i][5];
	if (matrix[i][6] != 0) 
	  cl = cl + matrix[i][6]*t/10.;
	ce = matrix[i][7];
	if (matrix[i][8] != 0) 
	  ce = ce + matrix[i][8]*t/10.;
	cosang = cos(darg);
	sinang = sin(darg);
	dvnut[0] += cl*sinang/factr;
	dvnut[1] += ce*cosang/factr;
	dvnut[2] += cl*cosang*dargrt/factr;
	dvnut[3] -= ce*sinang*dargrt/factr;
    }

    /* convert rates from radians/day to radians/sec */

    dvnut[2] = dvnut[2]/SECONDSperDAY;
    dvnut[3] = dvnut[3]/SECONDSperDAY;

    return PGS_S_SUCCESS;
}
