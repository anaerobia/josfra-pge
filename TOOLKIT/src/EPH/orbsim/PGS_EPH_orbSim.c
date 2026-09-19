/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_orbSim.c

DESCRIPTION:
   This file contains the function PGS_EPH_orbSim().
   This function simulates spacecraft position and velocity data.

AUTHOR:
  Guru Tej S. Khalsa
  Xin Wang / EIT Inc.
  Abe Taaheri / L3 Coomm., EER Systems Inc.
HISTORY:
  07-Jul-1994  GTSK  Initial version
  30-Oct-2001  XW    Parameterized upon inputting orbital elements
  30-Jun-2003  AT    Modified for epoch time as input orbital element.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Simulate spacecraft orbit and attitude data

NAME:
   PGS_EPH_orbSim()

SYNOPSIS:
   #include <PGS_TD.h>
      
DESCRIPTION:
   This function simulates spacecraft position and velocity data.

INPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   asciiUTC       UTC time in CCSDS ASCII     ASCII       see NOTES
                  Time Code A (or B) format

   spacecraftTag  spacecraft identifier, currently recogined values are
                  TRMM, EOS_AM, EOS_PM (these are C constants defined in
		  PGS_TD.h).

  a             - semi-major axis             m
  e             - eccentricity                n/a
  inc           - inclination                 radians
  an            - right ascension of
                  ascending node              radians
  ap            - argument of perigee         radians
  ma            - mean anomaly at time        radians

OUTPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   sat_pos        spacecraft ECI position     meters      see NOTES
                  (ECI is Earth Centered
		  Inertial)

   sat_vel	  spacecraft ECI velocity     meters/sec  see NOTES
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format

NOTES:
   notes on boundaries of input/output values:

   asciiUTC - This routine makes use the PGS toolkit internal time which is
              real continuous seconds since 12AM GMT 1/1/1993.  The conversion
	      from UTC time to this internal time requires the addition of
	      leap-seconds which is done via a lookup table which is must be
	      retrieved from the U.S. Naval Observatory (USNO) and is
	      periodically updated, therefore the limits of the input time will
	      most likely be determined by the UTC to TAI (internal time is TAI
	      based) conversion which in turn is dependent on the USNO
	      leapsecond file.  Also TAI time is not defined prior to 1958 and
	      therefore that is currently a limitation of this routine.

   pos      - the position of the spacecraft obviously depends on the orbital
              characteristics of the spacecraft.  Using the simple equations:
	         Rperiapsis = a(1 - e)
		 Rapoapsis = a(1 + e)
	      gives the following min. and max. values to be expected:
	      
	      TRMM      min: 6725756.2640 m      max: 6733023.7360 m
	      EOS-AM    min: 7077847.2488 m      max: 7096012.7512 m

   vel      - the velocity of the spacecraft similarly depends on the orbital
              characteristics of the spacecraft.  Solving the vis-viva eqn.
	      (-2/GM = (v^2)/2 - GM/r) for veloctiy yields the following min.
	      and max. values to be expected:

	      TRMM      min: 7692.129 m/s        max: 7700.440 m/s
	      EOS-AM    min: 7490.025 m/s        max: 7509.248 m/s


REQUIREMENTS:
   None

DETAILS:
   The following interpolation method combines the position and velocity     
   for each vector component to perform cubic interpolation; this       
   ensures that the interpolated positions and velocities are           
   consistent.                                                          
                                                                        
   The method is as follows.  Each component of the orbit vector can    
   be viewed as an independent function in one dimension, with the      
   velocity as the first derivative.  A cubic polynomial can be fit     
   exactly to two successive position-velocity pairs, so that the       
   interpolated positions are evaluated directly from the cubic and     
   the velocities are computed using the derivative (a quadratic).  If  
   the time coordinates of the two end points are arbitrarily assigned  
   values of 0 and 1, then the polynomial coefficients a0, a1, a2, and  
   a3 are computed as follows:                                          
                                                                        
                       a0  =  P1                                    (1) 
                                                                        
                       a1  =  V1                                    (2) 
                                                                        
                       a2  =  3P2  -  3P1  -  2V1dT  - V2dT         (3) 
                                                                        
                       a3  =  2P1  -  2P2  +  V1dT  + V2dT          (4) 
                                                                        
   where P1, P2, V1 and V2 are the successive values of the position and
   velocity, respectively, and dT = (T2  -  T1) is the time difference  
   in seconds.  Note that the velocities need to be multiplied by the   
   time difference to account for the arbitrary time scale.  To         
   interpolate to any intermediate point, the desired sample time Ts    
   is converted to a relative value between 0 and 1:                    
                                                                        
                       T  =  (Ts  -  T1) / dT                       (5) 
                                                                        
   The position and velocity are then computed from the cubic and its   
   derivative:                                                          
                                                                        
                P  =  a0  +  a1T  +  a2T^2  +  a3T^3                (6) 
                                                                        
                V  =  (a1  +  2a2T  +  3a3T2) / dT                  (7) 
                                                                        
                                                                        
   The pos/vel vectors are thus interpolated to the sample times by    
   first finding the pos/vel vector pairs which bracket the sample time;   
   then computing the polynomial coefficients for each of the three     
   vector components using equations 1 through 4; and finally           
   computing the interpolated values of the position and velocity       
   using equations 5 through 7.  In practice, several consecutive       
   sample times are typically bracketed by each pair of pos/vel vectors,   
   so the coefficients need not be recomputed for each sample time.        
		  
GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_EPH_UTCtoTAI()
   PGS_EPH_orbitalElements()

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <PGS_SIM.h>

PGSt_SMF_status
PGS_EPH_orbSim(                 /* simulates s/c orbit ephemeris data */
    PGSt_double  aIn,           /* semi-major axis (m) */
    PGSt_double  eIn,           /* eccentricity */
    PGSt_double  incIn,         /* inclination (radians) */
    PGSt_double  anIn,          /* right ascension of ascending node (radians) */ 
    PGSt_double  apIn,          /* argument of perigee (radians) */
    PGSt_double  maIn,          /* mean anomaly at time (radians) */
    PGSt_double  secTAI93,      /* time in internal time format */
    PGSt_tag     spacecraftTag, /* unique s/c identification tag */
    PGSt_double  *sat_pos,      /* s/c position in ECI coordinates */
    PGSt_double  *sat_vel,      /* s/c velocity in ECI coordinates */
    PGSt_double  epoch)         /* epoch in secs since 12AM UTC 1/1/1993 */
{
    PGSt_double  tempTime;      /* temp time variable */
    PGSt_double  dt;            /* difference between most recent integer minute
				   and input time (in seconds) */
    PGSt_double  elt[10];       /* keplerian orbital elements and other stuff */
    PGSt_double  th;            /* true anomaly (rad) */
    PGSt_double  r;             /* orbit radius (meters) */
    PGSt_double  thdot;         /* rate of change of true anomaly (rad/sec) */
    PGSt_double  rdot;          /* rate of change of radius (m/s) */
    PGSt_double  thpdot;        /* rate of change of true anomaly + rate of
				   change of argument of perigee (i.e. rate of
				   change of argument of latitude) */
    PGSt_double  ci;            /* cosine of inclination */
    PGSt_double  si;            /* sine of inclination */
    PGSt_double  can;           /* cos of right ascension of ascending node */
    PGSt_double  san;           /* sine of right ascension of ascending node */
    PGSt_double  cthp;          /* cosine of argument of latitude */
    PGSt_double  sthp;          /* sine of argument of latitude */

    static PGSt_double  oldTime; /* secTAI93 - dt from last call to function */
    static PGSt_double  p[2][3]; /* position vectors */
    static PGSt_double  v[2][3]; /* velocity vectors */
    static PGSt_double  cp[3][4];/* interpolation coefficients */

    PGSt_integer orbitNumber;    /* jes like it says */
    
    short        j;              /* loop counter */

    /* return status of this function intialized to indicate success */

    PGSt_SMF_status returnStatus=PGS_S_SUCCESS;
    
    static PGSt_boolean firstTime=PGS_TRUE; /* true on first call to this
					       function, otherwise false */
    
    /* this code was converted from fortran, the following is a quick and dirty
       means of dealing with a fortran equivalence statement */

#define a     elt[0]  /* semi-major axis (m) */      
#define e     elt[1]  /* eccentricity */    
#define inc   elt[2]  /* inclination (radians) */
#define an    elt[3]  /* right ascension of ascending node (radians) */
#define ap    elt[4]  /* argument of perigee (radians) */
#define ma    elt[5]  /* mean anomaly at time (radians) */
#define andot elt[6]  /* rate of change of RA of ascending node (rad/sec) */
#define apdot elt[7]  /* rate of change of argument of perigee (rad/sec) */
#define madot elt[8]  /* mean motion (rad/sec) */

    /*  convert ASCII time to internal time */

/*     returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93); */
/*     if (returnStatus != PGS_S_SUCCESS) */
/*       return returnStatus; */

    /* if new minute, compute new point(s) and polynomial */
    
    tempTime = secTAI93 - fmod(secTAI93,60.);
    if (((tempTime-oldTime) >= 60.0) || (firstTime) || (tempTime < oldTime) )
    {
        firstTime=PGS_FALSE;
        
	/* update elements based on time since epoch */

        oldTime = tempTime;
        for (j=0;j<2;j++)
        {
            tempTime += j*60;   
            returnStatus = PGS_EPH_orbitalElements(tempTime,spacecraftTag,
				aIn, eIn, incIn, anIn, apIn, maIn, elt,
					     &orbitNumber, epoch);
	    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
		returnStatus != PGSTD_E_NO_LEAP_SECS)
              return returnStatus;
            
            /* approximate Theta (true anomaly), Radius and corresponding
	       rates (e << 1) */
            
            th=ma+2.0*e*sin(ma)+1.25*e*e*sin(2.0*ma);
            r=a*(1.0 - e*cos(ma) - 0.5*e*e*(cos(2.0*ma) - 1.0));
            thdot=madot*(1.0 + 2.0*e*cos(ma) + 2.50*e*e*cos(2.0*ma));
            rdot=a*madot*(e*sin(ma) + e*e*sin(2.0*ma) );
            thpdot=thdot+apdot;
            
            /* rotate orbit plane and corresponding rates to ECI using right
               ascension of Ascending Node, Argument of Perigee, and 
               Inclination */
            
            ci=cos(inc);
            si=sin(inc);
            can=cos(an);
            san=sin(an);
            cthp=cos(th+ap);
            sthp=sin(th+ap);
            
            p[j][0] = r*(cthp*can - ci*sthp*san);
            p[j][1] = r*(cthp*san + ci*sthp*can);
            p[j][2] = r*sthp*si;
            
            v[j][0] = rdot*(cthp*can-ci*sthp*san) +
                      r*(-thpdot*sthp*can - cthp*andot*san -
                         ci*(thpdot*cthp*san+sthp*andot*can));
            v[j][1] = rdot*(cthp*san+ci*sthp*can) +
                      r*(-thpdot*sthp*san + cthp*andot*can +
                         ci*(thpdot*cthp*can - sthp*andot*san));
            v[j][2] = rdot*sthp*si + r*thpdot*cthp*si;
        }

        /* calculate coefficients of cubic polynomial through points */
        
        for (j=0;j<3;j++)
        {
            cp[j][0]=p[0][j];
            cp[j][1]=60.0*v[0][j];
            cp[j][2]=3.0*( p[1][j] - p[0][j]) - 60.0*(v[1][j] + 2.0*v[0][j]);
            cp[j][3]=2.0*(-p[1][j] + p[0][j]) + 60.0*(v[1][j] +  v[0][j]);
        }
    }
    
    /* evaluate polynomial at input time for position and velocity */
    
    dt=fmod(secTAI93,60.)/60.;
    for (j=0;j<3;j++)
    {
        sat_pos[j]=dt*(dt*(dt*cp[j][3] + cp[j][2]) + cp[j][1]) + cp[j][0];
        sat_vel[j]=(dt*(dt*3.0*cp[j][3] + 2.0*cp[j][2]) + cp[j][1])/60.0;
    }
    return returnStatus;
}
