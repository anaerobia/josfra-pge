/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_attOrbSim.c

DESCRIPTION:
  This file contains the function PGS_EPH_attOrbSim().

AUTHOR:
  Guru Tej S. Khalsa
  Peter D. Noerdlinger / SM&A Inc.
  Xin Wang / EIT Inc.
  Abe Taaheri / L3 Coomm., EER Systems Inc.

HISTORY:
  08-Jul-1994  GTSK  Initial version
  22-Nov-1999  PDN   Fixed output angles to be in radians
  30-Oct-2001  XW    Parameterized upon inputting orbital elements
  30-Jun-2003  AT    Modified for epoch time as input orbital element.

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Simulate orbit position and attitude data for EOS spacecraft

NAME:
   PGS_EPH_attOrbSim()

SYNOPSIS:
   #include <PGS_EPH.h>
      
DESCRIPTION:
   Simulates orbit position and attitude data including random attitude and 
   attitude rate noise for EOS spacecraft.  Currently supported platforms are:
       TRMM
       EOS-AM
       EOS-PM

INPUTS:
   Name           Description                 Units       Min   Max
   ----           -----------                 -----       ---   ---
   asciiUTC       UTC time in CCSDS ASCII     ASCII       see NOTES
                  Time Code A (or B) format

   spacecraftTag  spacecraft identifier, currently recogined values are
                  TRMM, EOS_AM, EOS_PM (these are C constants defined in
		  PGS_TD.h).

   att_typ_ver    string specifying whether or not to include noise in the
                  attitude as well as the noise level.  The first character
		  of the string must be 'N' to indicate attitude noise should
		  be generated.  This should be followed by 10 digits indicating
		  the desired noise level as follows:
		    The first five digits indicate the amount of noise in
		    units of hundredths of arc-seconds introduced in the 
		    attitude (yaw, pitch, roll).
		    The second five digits indicate the amount of noise in
		    units of hundredths of arc-seconds introduced in the 
		    attitude rates (yaw rate, pitch rate, roll rate).

  a		- semi-major axis    	       m
  e		- eccentricity                 n/a
  inc		- inclination                  radians
  an		- right ascension of 
		  ascending node               radians
  ap		- argument of perigee          radians
  ma		- mean anomaly at time         radians

OUTPUTS:
   Name         Description                 Units        Min    Max
   ----         -----------                 -----        ---    ---
   pos          spacecraft ECI position     meters       see NOTES
		                                                  
   vel          spacecraft ECI velocity     meters/sec   see NOTES

   ypr          s/c yaw, pitch, roll        radians      see NOTES

   ypr_rate     s/c yaw, pitch, roll rates  radians/sec  see NOTES

   transform    spacecraft reference frame  N/A          N/A    N/A
                to ECI frame coordinate 
		transformation matrix
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSTD_E_TIME_VALUE_ERROR    error in input time value
   PGSTD_E_TIME_FMT_ERROR      error in input time format

NOTES:
   This routine calls PGS_EPH_orbSim which returns the spacecraft position
   information in ECI coordinates.  It then calculates the spacecraft reference
   frame to ECI frame coordinate transformation matrix from this data.  This
   is done by assuming that the yaw, pitch, and roll of the spacecraft are all
   zero.  The transformation matrix is then the 3X3 matrix whose columns consist
   of the X, Y, and Z unit vectors of the spacecraft reference frame rendered in
   ECI coordinates.  
   The spacecraft reference frame for zero yaw, pitch, and roll is coincident
   with the orbital reference frame.  Positive Z is defined as the anti-radius
   of the spacecraft, that is the unit vector along the radius vector pointing
   from the spacecraft to the center of the earth.  The X axis is defined to
   be in the plane defined by the spacecraft position and velocity vectors and
   perpendicular to the Z axis.  The Y axis is the cross-product of the Z and X
   axis unit vectors.
   If attitude noise has been requested this routine calls PGS_EPH_attitudeNoise
   which adds small random variations to the spacecraft yaw, pitch, and roll
   and/or yaw, pitch, and roll rates.  PGS_EPH_attitudeNoise also makes the
   proper adjustments to the transformation matrix.
   
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

   ypr      - for this routine the yaw, pitch and roll are all assumed to be
              zero.  If the attitude noise option has been selected then 
	      random noise is added to the yaw, pitch and roll.  The maximum
	      values of this noise are specified by the user (see INPUTS:
	      att_typ_ver).  The maximum magnitude of this noise is 999.99
	      arc-seconds.  The noise be positive or negative so the widest
	      range these values can have is -0.277775 to 0.277775 degrees.

   ypr_rate - this is handled the same way as ypr (see above) therefore the
              range these values can have is -0.277775 to 0.277775 deg/sec.

REQUIREMENTS:
   None

		  
GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_TD_UTCtoTAI()
   PGS_EPH_orbSim()
   PGS_EPH_attitudeNoise()

AUTHOR: 
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   08-Jul-1994  GTSK  Initial version

END_PROLOG:
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_SIM.h>

PGSt_SMF_status
PGS_EPH_attOrbSim(                /* spacecraft orbit and attitude simulator */
    PGSt_double  aIn,             /* semi-major axis (m) */ 
    PGSt_double  eIn,             /* eccentricity */
    PGSt_double  incIn,           /* inclination (radians) */ 
    PGSt_double  anIn,            /* right ascension of ascending node (radians) */
    PGSt_double  apIn,            /* argument of perigee (radians) */ 
    PGSt_double  maIn,            /* mean anomaly at time (radians) */
    PGSt_double  secTAI93,        /* internal TAI time */
    PGSt_tag     spacecraftTag,   /* spacecraft ID tag */
    char         *att_typ_ver,    /* attitude flag and noise level */
    PGSt_double  pos[3],          /* spacecraft ECI position */
    PGSt_double  vel[3],          /* spacecraft ECI velocity */
    PGSt_double  ypr[3],          /* spacecraft yaw/pitch/roll */
    PGSt_double  ypr_rate[3],     /* spacecraft yaw/pitch/roll rates */
    PGSt_double  transform[3][3], /* spacecraft ref. frame to ECI
				     transformation matrix */
    PGSt_double  epoch)           /* epoch in TAI format */
{
/*    PGSt_double secTAI93;  internal TAI time */
    PGSt_double rdotr;    /* dot product of position vector with itself */
    PGSt_double vdotr;    /* dot product of velocity and position vectors */
    PGSt_double r;        /* magnitude of position vector (ECI) */
    PGSt_double vcg;      /* normalizing factor for 1st col. of trans. matrix */

    int lenchr;           /* length of att_typ_ver string */
    
    PGSt_SMF_status returnStatus; /* return value of calls to PGS functions */
    
    short j;              /* loop counter */
    short iop;            /* numeric value indicating if noise was requested */
    
    /* intialize status to indicate success */

    returnStatus = PGS_S_SUCCESS;
    
    /* convert ascii time to seconds since 12AM GMT 1/1/1993 */
   
/*     returnStatus = PGS_TD_UTCtoTAI(asciiUTC,&secTAI93); */
/*     if (returnStatus != PGS_S_SUCCESS) */
/*       return returnStatus; */
      
    /* get orbital position and velocity */
     
    returnStatus = PGS_EPH_orbSim(aIn, eIn, incIn, anIn, apIn, maIn, secTAI93,spacecraftTag,pos,vel, epoch);
    if (PGS_SMF_TestStatusLevel(returnStatus) == PGS_SMF_MASK_LEV_E &&
	returnStatus != PGSTD_E_NO_LEAP_SECS)
      return returnStatus;
    
    /* to determine the transformation matrix the unit vectors of the X-Y-Z
       components of the spacecraft in the spacecraft reference frame are
       rendered in their ECI components.  The following first cut at the
       the transformation matrix is valid for zero yaw, pitch and roll of
       the spacecraft */

    rdotr=pos[0]*pos[0]+pos[1]*pos[1]+pos[2]*pos[2];    
    vdotr=vel[0]*pos[0]+vel[1]*pos[1]+vel[2]*pos[2];    
    r=sqrt(rdotr);    
    
    /* Z-axis is unit vector toward nadir (anti-radius) */
    
    for (j=0;j<3;j++)
    {
	transform[j][2]=-pos[j]/r;                 /* Z-axis */
	transform[j][0]=vel[j]-vdotr*pos[j]/rdotr; /* X-axis */
    }
     
    /* X-axis is close to velocity, but normal to R in R-V plane.
       The X-axis components were calculate in the above loop, here
       they are being normalized. */
      
    vcg=sqrt(transform[0][0]*transform[0][0] + transform[1][0]*transform[1][0] +
	     transform[2][0]*transform[2][0]);
    for (j=0;j<3;j++)
      transform[j][0]=transform[j][0]/vcg;
    
      
    /* Y-axis is Z cross X (already unitized) 
       i.e. Z-axis is column 3 of transformation matrix, and X-axis is
            column 1 of transformation matrix, therefore Y-axis is the
	    cross-product of column 3 and column 1.  Note of course that
	    the cross-product of two orthogonal unit vectors is another
	    unit vector so no normalization is necessary */
    
    transform[0][1] = transform[1][2]*transform[2][0] -
                      transform[2][2]*transform[1][0];
    transform[1][1] = transform[2][2]*transform[0][0] -
                      transform[0][2]*transform[2][0];
    transform[2][1] = transform[0][2]*transform[1][0] -
                      transform[1][2]*transform[0][0];      
    
    /* return nominal yaw, pitch, and roll (i.e. assume all three quantities
       are zero).
       
       Note: if the yaw, pitch and roll are not set to zero here then the
             transformation matrix "transform" will not be correct */
    
    for (j=0;j<3;j++)
    {
	ypr[j]=0.0;
	ypr_rate[j]=0.0;
    }

    /* check to see if noise option selected, this is indicated by the string
       att_typ_ver the second character of which should be an N if noise is
       desired ("noise" means random attitude noise) */
    
    iop = 0;
    lenchr = strlen(att_typ_ver);
    if(lenchr > 1)
    {
	/* iop = 1 for noise otherwise iop = 0 */
	if(att_typ_ver[0] != 'N' && att_typ_ver[0] != 'n') 
	  iop = 0;
	else
	  iop =1;
    }
    
    /* adjust noise if option selected, if noise was selected small random
       variations in yaw, pitch and roll (from their nominal values) are
       introduced and the transform matrix is adjusted accordingly */

    if (iop == 1)
      PGS_EPH_attitudeNoise(secTAI93, ypr, ypr_rate, transform, att_typ_ver);

    /* return */

    return returnStatus;
}
