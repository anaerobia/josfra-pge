/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_attitudeNoise.c

DESCRIPTION:
   This file contains the function PGS_EPH_attitudeNoise.

AUTHOR:
  Guru Tej S. Khalsa
  Peter D. Noerdlinger / SM&A Inc.

HISTORY:
  08-Jul-1994  GTSK  Initial version
  22-Nov-1999  PDN Fixed output angles to be in radians

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:    Add random noise to spacecraft attitude and attitude rates.

NAME:     PGS_EPH_attitudeNoise()

GLOBALS_ACCESSED:  none

SYNOPSIS:
   #include <PGS_EPH.h>
      
DESCRIPTION:
   Adds random "noise" to spacecraft attitude and attitude rate; magnitude of
   noise specified by user.

INPUTS:
   Name         Description                            Units     Min      Max
   ----         -----------                            -----     ---      ---
   secTAI93     TAI time as seconds since              seconds   see NOTES
                12AM UTC 1/1/1993				 
								 
   ypr          spacecraft (s/c) yaw, pitch and roll   deg       see NOTES      
								 
   ypr_rate     s/c yaw, pitch and roll rates          deg	 see NOTES 

   transform    local vertical to ECI transfomation    N/A
                matrix				     
						     
   att_typ_ver  attitude and rates noise magnitudes    N/A	 see NOTES


OUTPUTS:
   Name         Description                            Units     Min      Max
   ----         -----------                            -----     ---      ---
   ypr          spacecraft yaw, pitch and roll         deg       see NOTES  
						     		 
   ypr_rate     spacecraft yaw, pitch and roll rates   deg/sec   see NOTES  
						     
   transform    s/c ref. frame to ECI transfomation    N/A
                matrix
          
RETURNS:
   PGS_S_SUCCESS               successful return 

NOTES:
   This adds the effects of random noise to the yaw, pitch and roll
   respectivley, as well as the yaw, pitch and roll rates.
                                                                
   It also calculates, based on the added values, the           
   transformation (rotation) matrix att_trn.  Given the components      
   of a vector in the spacecraft reference frame, the matrix transforms
   these to values as seen from the local vertical. 
                                                                
   The transformation matrix, transform, which transforms vector  
   components in the local vertical coordinates to the          
   ECI coordinates is input.                                    
                                                                
   att_trn is then multiplied with transform To generate the rotation 
   matrix from s/c ref. to ECI.  This resultant matrix is then restored
   in transform.                                                   

   att_typ_ver is a string containing the max. allowable values for the
   attitude and attitude rates noise.  The first character should be
   an 'N' or an 'n' if noise is wanted.  The next five characters MUST all be
   present and MUST be digits (leading zeros if necessary) indicating
   the max. number of hundredths of arc-seconds that the attitude noise may
   be.  The next five characters MUST either not exist or all be present
   and all be digits indicating the max. number of hundredths of arc-seconds
   that the attitude rates noise may be.  Note however that this function
   does not check for the inclusion of all the digits in either of the two
   noise fields, but the behavior may be strange if all five digits are not
   available.
 
  notes on boundaries of input/output values:
   
   secTAI93  -  this can really be any value, it is only used to seed the random
                number generator; its only real boundaries are the upper and
                lower limits of the PGSt_double type

   ypr       -  this can similarly be any value, incoming values are not check
                for any values and can be any real number allowed by the type
		PGSt_double, this routine will then add a number ranging from
		-.27777777 to .277777777 (inclusive) to the input value and
		return this sum; this of course is an angle (in degrees) so
		numbers with magnitude 360 and higher are redundant but can be
		used if desired

   ypr_rate  -  this can also have any value bounded only by the limits of the
                type PGSt_double, this function will then add a number ranging
		from -.27777777 to .277777777 (inclusive) to the input value and
		return this sum

FILE:
   None

FUNCTIONS_CALLED:
   PGS_EPH_matrixMultiply()

AUTHOR: 
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   08-Jul-1994  GTSK  Initial version

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_SIM.h>

/* constants */

#define DEGtoRAD .0174532925199433 /* degree to radian conversion */
    

PGSt_SMF_status
PGS_EPH_attitudeNoise(         /* introduce random noise to yaw, pitch, roll and
				  their respective rates */
PGSt_double  secTAI93,         /* TAI time as seconds since 12AM GMT 1/1/1993 */
PGSt_double  ypr[3],           /* spacecraft yaw, pitch and roll */
PGSt_double  ypr_rate[3],      /* spacecraft yaw, pitch and roll rates */
PGSt_double  transform[3][3],  /* s/c ref. frame to ECI transfomation matrix */
char         *att_typ_ver)     /* attitude and rates noise magnitudes */
{
    PGSt_double att_tmp[3][3]; /* temporary transformation matrix */
    PGSt_double att_trn[3][3]; /* temporary transformation matrix */
    PGSt_double cpsi;          /* cosine of pitch angle (psi) */
    PGSt_double spsi;          /* sine of pitch angle (psi) */
    PGSt_double cth;           /* cosine of roll angle (theta) */
    PGSt_double sth;           /* sine of roll angle (theta) */
    PGSt_double cphi;          /* cosine of yaw angle (phi) */
    PGSt_double sphi;          /* sine of yaw angle  (phi) */
    PGSt_double y_rad;         /* yaw angle in radians */
    PGSt_double p_rad;         /* pitch angle in radians */
    PGSt_double r_rad;         /* roll angle in radians */
    PGSt_double std1;          /* max. value of yaw, pitch and roll noise */
    PGSt_double std2;          /* max. value y, p and r rates noise */

    int          itmp1=0;      /* integer value of noise (=100*std1) */
    int          itmp2=0;      /* integer value of noise (=100*std2) */

    unsigned seed1;            /* seed value for random number generator */
    unsigned seed2;            /* seed value for random number generator */
    unsigned seed3;            /* seed value for random number generator */
    
    short it;                  /* loop counter */
    short is;                  /* loop counter */

    PGSt_SMF_status returnStatus; /* return status of this function */

    /* intialize status to return PGS_S_SUCCESS, which as it happens is
       currently the only thing this function returns */

    returnStatus = PGS_S_SUCCESS;
    
    /* att_typ_ver is a string containing the max. allowable values for the
       attitude and attitude rates noise.  The first character should be
       an 'N' or an 'n' if noise is wanted.  The next five characters MUST all
       be present and MUST be digits (leading zeros if necessary) indicating
       the max. number of hundredths of arc-seconds that the attitude noise may
       be.  The next five characters MUST either not exist or all be present
       and all be digits indicating the max. number of hundredths of arc-seconds
       that the attitude rates noise may be.  The integer number of hundredths
       of arc-seconds are read into the integer variables itmp1 and itmp2 which
       are then divided by the real number 100.0 and stored in std1 and std2
       as real numbers (probable double, maybe long double: see PGS_TYPES.h). */

    if (att_typ_ver[0] == 'N' || att_typ_ver[0] == 'n')
      sscanf(att_typ_ver,"%*c%5d%5d",&itmp1,&itmp2);
    std1 = itmp1/100.0;
    std2 = itmp2/100.0;
    
    /* Guru Tej S. Khalsa's harebrained scheme for tying a set noise to a 
       particular time (pat. pend.).  The idea is to use the input time (or some
       variation thereof) to seed the random number generator before each
       call to the random number generator.  This way the noise assigned
       at a given time will be constant for a given noise magnitude specified
       no matter when the time is called.  i.e. whether the noise routine
       is called every 15 seconds, every 30 seconds, every minute, every two
       minutes, etc., etc. the noise at a given clock time will always be
       exactly the same.  The intention is to further the simulation by 
       producing consistent data any time the routine is called, as opposed to
       e.g. using system time to seed the generator so that data today will not
       match the same data generated tomorrow.  So what is below is code to
       generate three hopefully different seed values (one each for roll, pitch
       and yaw) based on input time, and seed the random number generator 
       intentionally before each call. */

    if (fabs(secTAI93) < pow(2.,32.))
    {
	seed1 = (unsigned) fabs(secTAI93);
	seed2 = (unsigned) fabs(secTAI93)%100000 + 100;
	seed3 = (unsigned) fabs(secTAI93)%1000000 + 1000;
    }
    else
    {
 	seed1 = (unsigned) (fmod(fabs(secTAI93),pow(2.,32)));
 	seed2 = (unsigned) (fmod(fabs(secTAI93),pow(2.,32)))%100000 + 100;
	seed3 = (unsigned) (fmod(fabs(secTAI93),pow(2.,32)))%1000000 + 1000;
    }

    /* seed the random number generator (srand/rand are used here since they
       are ANSI/POSIX compliant functions although not the best random number
       generation system) and then call rand and render the results as a real
       number in the range [-1,1].  Multiply this by the max noise level as
       specified in att_typ_ver.  This result is then converted to degrees 
       or degrees/second (in the case of rates) and added to the existing value
       of the appropriate quantity. NOTE: rand returns integers in the range
       [0,2^15 - 1] so the result of rand is divided by 2^15 - 1 to get a real
       number in the range [0,1] and then multiplied by 2 to get [0,2].  Finally
       1 is subtracted to give [-1,1] (in case you were curious) */

    /* Note: The (pow(2.,15) - 1) was replaced with RAND_MAX. 
       it looks like that the value of (pow(2.,15) - 1) is the 
       same as RAND_MAX in all unix machines, but different in linux */
    /*
    srand(seed1);
    ypr[0] = ypr[0] + (rand()/(pow(2.,15) - 1)*2-1)*((std1/3600.0)*DEGtoRAD);
    srand(seed2);
    ypr[1] = ypr[1] + (rand()/(pow(2.,15) - 1)*2-1)*((std1/3600.0)*DEGtoRAD);
    srand(seed3);
    ypr[2] = ypr[2] + (rand()/(pow(2.,15) - 1)*2-1)*((std1/3600.0)*DEGtoRAD);
    
    srand(seed1/10);
    ypr_rate[0] = ypr_rate[0] + (rand()/(pow(2.,15) - 1)*2-1)*((std2/3600.0)*DEGtoRAD);
    srand(seed2/10);
    ypr_rate[1] = ypr_rate[1] + (rand()/(pow(2.,15) - 1)*2-1)*((std2/3600.0)*DEGtoRAD);
    srand(seed3/10);
    ypr_rate[2] = ypr_rate[2] + (rand()/(pow(2.,15) - 1)*2-1)*((std2/3600.0)*DEGtoRAD);
    */

    srand(seed1);
    ypr[0] = ypr[0] + ((rand()/(double)RAND_MAX)*2-1)*((std1/3600.0)*DEGtoRAD);
    srand(seed2);
    ypr[1] = ypr[1] + ((rand()/(double)RAND_MAX)*2-1)*((std1/3600.0)*DEGtoRAD);
    srand(seed3);
    ypr[2] = ypr[2] + ((rand()/(double)RAND_MAX)*2-1)*((std1/3600.0)*DEGtoRAD);
    
    srand(seed1/10);
    ypr_rate[0] = ypr_rate[0] + ((rand()/(double)RAND_MAX)*2-1)*((std2/3600.0)*DEGtoRAD);
    srand(seed2/10);
    ypr_rate[1] = ypr_rate[1] + ((rand()/(double)RAND_MAX)*2-1)*((std2/3600.0)*DEGtoRAD);
    srand(seed3/10);
    ypr_rate[2] = ypr_rate[2] + ((rand()/(double)RAND_MAX)*2-1)*((std2/3600.0)*DEGtoRAD);

    /* calculate transformation matrix */

    /* The transformation is from s/c ref. to local vertical orbital.
       The rotation from local vertical to s/c ref. is positive.
       Therefore, we use inverse of existing formulas, which
       is also the transpose.

       The sequence of rotation is about the
           yaw axis 
           roll axis
           pitch axis
       respectively.

      Note however, that ypr is in yaw, pitch, roll order. */

    /* Change to radians for trig functions is already done */

    y_rad = ypr[0];
    p_rad = ypr[1];
    r_rad = ypr[2];
    
    /* load cos and sin of the various quantities into memory to avoid
       costly repeated calls to cos and sin functions */

    cpsi = cos(p_rad);
    spsi = sin(p_rad);
    cth  = cos(r_rad);
    sth  = sin(r_rad);
    cphi = cos(y_rad);
    sphi = sin(y_rad);
    
    /* calculate spacecraft reference frame to local vertical orbital reference
       frame transformation matrix */
    
    att_trn[0][0] = cpsi*cphi - sth*spsi*sphi;
    att_trn[0][1] = -cth*sphi;
    att_trn[0][2] = spsi*cphi + sth*cpsi*sphi;
    att_trn[1][0] = cpsi*sphi + sth*spsi*cphi;
    att_trn[1][1] = cth*cphi;
    att_trn[1][2] = spsi*sphi - sth*cpsi*cphi;
    att_trn[2][0] = -cth*spsi;
    att_trn[2][1] = sth;
    att_trn[2][2] = cth*cpsi;
    
    /* update transformation matrix to account for the spacecraft attitude */

    PGS_EPH_matrixMultiply(transform,att_trn,att_tmp);

    /* att_tmp is the result of [transform] X [att_trn] and is actually the new
       transformation matrix that is desired so copy att_tmp to transform */

    for (it=0;it<3;it++)
      for (is=0;is<3;is++)
	transform[is][it] = att_tmp[is][it];
    
    return returnStatus;
}
