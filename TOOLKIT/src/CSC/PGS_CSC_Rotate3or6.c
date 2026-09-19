/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_CSC_Rotate3or6.c

DESCRIPTION:
  This file contains the functions PGS_CSC_rotat3() and PGS_CSC_rotat6().

AUTHOR:
  Guru Tej S Khalsa / Applied Research Corporation
  Peter Noerdlinger / Applied Research Corporation
  Anubha Singhal    / Applied Research Corporation

HISTORY:
      Dec-1993    PDN    Acquired from E. Myles Standish at JPL  
   31-Jan-1994    GTSK   Converted from FORTRAN to C 
      Feb-1994    PDN    Explanatory Comments Added 
   04-Mar-1994    PDN    Indexing fixed to preserve sense 
   19-Oct-1994    AS     Fixed prologs and code to conform to latest PGS/ECS
                         standards
  
END_FILE_PROLOG:
******************************************************************************/
/******************************************************************************
BEGIN_PROLOG

TITLE:    
   This function rotates the position vector

NAME:     
   PGS_CSC_rotat3      
			  
SYNOPSIS:
C:  
   #include <PGS_CSC.h>

   PGSt_SMF_status
   PGS_CSC_rotat3(
        PGSt_double*  x,              
        PGSt_double*  a,                                       
        PGSt_integer  l,                   
	PGSt_double*  y)              

DESCRIPTION:
   This tool rotates a position vector through an input angle, about a 
   selected principal axis of rotation.  The vector needs to have 3 more 
   scratch components which represent velocity in sister routine 
   PGS_CSC_rotat6.  The present routine is designed to have the same 
   calling sequence but applies to cases where the velocity change would be 
   too large to be meaningful    

INPUTS:
   Name     Description        Units                     Min        Max
   ----     -----------        -----                     ---        ---
   x[6]     position vector    x[0],x[1],x[2] in any     N/A        N/A
                               length unit; x[3],x[4],
                               x[5] can be anything      
                               (not used)
   a[2]     angle and rate     rad and rad/time unit     N/A        N/A
                               (The rotation rate is discarded - it is present
                               only for consistency with sister routine 
                               PGS_CSC_rotat6).
   l        axis of rotation   N/A                       N/A        N/A
            1 for x, 2 for y, 3 for z (clockwise);
            -1, -2, -3 for the same axes (counterclockwise)
	    Here clockwise means that the rotation turns like the hands of a 
            clock as viewed along the axis looking outward from the origin.
    
OUTPUTS:
   Name     Description        Units                     Min        Max
   ----     -----------        -----                     ---        ---
   y        position vector    any length unit           N/A        N/A
 
RETURNS:  
   PGS_S_SUCCESS                    successful return
   PGSCSC_E_BAD_ROT_AXIS_INDEX      indicates bad rotation axis index
 
NOTES:
   This is personal and not JPL certified code.

REQUIREMENTS:
   PGSTK - 1050

DETAILS:
   NONE

GLOBALS:
   NONE

FILES:
   NONE

FUNCTION CALLS: 
   PGS_SMF_SetStaticMsg    set error/status message

END_PROLOG
******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <PGS_CSC.h>

PGSt_SMF_status
PGS_CSC_rotat3(                  /* rotates the position vector */
    PGSt_double* x,              /* state vector */
    PGSt_double* a,              /* angle and rate in rad and rad/time unit */ 
	                         /*********rate is ignored******************/
    PGSt_integer l,              /* axis of rotation 1 for x, 2 for y, 3 for z;
				    negatives for reverse sense */ 
    PGSt_double* y)              /* position vector */  
{
    PGSt_double  c;              /* cosine of the unsigned angle of rotation */
    PGSt_double  s;              /* sine of the unsigned angle of rotation */
    PGSt_double  t;              /* temporary storage to avoid over-writing */
    PGSt_double  sign;           /* sign of rotation - see "INPUTS" */
    PGSt_integer i;              /* i,j,k are C range indices for the 3 axes */
    PGSt_integer j;              /* i and j are in the rotation plane and k is
                                    + */
    PGSt_integer k;              /* the rotation axis; i,j,k are cyclic  */

    PGSt_SMF_status returnStatus;/* indicates success or error status */

    returnStatus = PGS_S_SUCCESS;
    
    if( (l < -3) || (l > 3) || (l == 0) )
    { 
	returnStatus = PGSCSC_E_BAD_ROT_AXIS_INDEX;
	PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_rotat3()");
	return returnStatus;
    }

    /* Get sense of rotation - clockwise or counter */

    sign = 1.0;
    if(l < 0) sign = -1.0;

    /* Here we must create a C-index in the range 0 .. 2 for the axis. */

    k = (l > 0) ? l - 1: -l -1;

    /* The following 2 lines induce a cyclic permutation mapping 1,2,3 into 
        k,j,i */

    i = (k + 1) % 3;
    j = 3-i-k;

    /* nonzero elements of the 2-d rotation matrix about axis k */
    /* This is a submatrix of the 3 dimensional matrices defined on
       p. B60 of the A.A.; the 3rd dimension is unaffected since the
       3 dimensional matrix partitions into a 2-d one and the identity */

    c = cos(a[0]);
    s = sin(a[0]*sign);

    /* temporary new i component of position while we fix up j component */

    t = x[i]*c+x[j]*s;

    /* fix up j component of position */

    y[j] = x[j]*c-x[i]*s;

    /* use our temporary to replace i component of position
       with result of rotation */
    y[i] = t;

    /*  copy over the unaffected component of position */

    y[k] = x[k];

    PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_rotat3()");
    return returnStatus;
}
/******************************************************************************
BEGIN_PROLOG

TITLE:    
   Rotate Position and Velocity Vectors

NAME:     
   PGS_CSC_rotat6()

SYNOPSIS:
C:   
   #include <PGS_CSC.h>

   PGSt_SMF_status
   PGS_CSC_rotat6(
        PGSt_double*  x,              
        PGSt_double*  a,                                       
        PGSt_integer  l,                   
	PGSt_double*  y) 

DESCRIPTION:
   This tool rotates a position and velocity about a specified axis by 
   specified angle a[0].  The change in position is by a rotation matrix. The
   transformation of velocity requires knowledge of the angular rate Omega 
   about that axis, which the user must supply in a[1]. The algorithm is 
   explained, for example, in "Classical Mechanics", by S. Goldstein 
   (Addison-Wesley, N.Y.1950) Eq. (4-102), p. 133.  It reads, thus, v' = v + 
   Omega X r where v' is the new velocity, v the old, and r the coordinates
   (first three components of x.)  The "X" in the foregoing equation is the 
   vector cross product.

INPUTS:
   Name   Description        Units                     Min        Max
   ----   -----------        -----                     ---        ---
   x[6]   state vector:                                N/A        N/A
          position and       Any consistent units can be used, i.e. x[0],x[1],
          velocity           x[2] in a length unit and velocity  as x[3], x[4]
                             , x[5] in that length unit per unit time 
   a[2]   angle and rate     rad and rad/time unit     N/A        N/A
                             (the time unit must be the same as used in 
                              defining the state vector).
   l      axis of rotation 
          1 for x, 2 for y, 3 for z (clockwise);
          -1, -2, -3 for the same axes (counterclockwise)
          Here clockwise means that the rotation turns like the hands of a 
          clock as viewed along the axis looking outward from the origin.

OUTPUTS:
   Name   Description        Units                     Min        Max
   ----   -----------        -----                     ---        ---
   y      state vector       any length unit           N/A        N/A         

RETURNS:
   PGS_S_SUCCESS                     success
   PGSCSC_E_BAD_ROT_AXIS_INDEX       indicates bad rotation axis index 

NOTES:
   This is derived from personal and not JPL certified code

REQUIREMENTS:
   PGSTK - 1050

DETAILS:
   NONE

GLOBALS:
   NONE

FILES:
   NONE 
	  
FUNCTION CALLS:
   PGS_SMF_SetStaticMsg       set error/status messages

END_PROLOG
******************************************************************************/
#include <PGS_math.h>
#include <PGS_CSC.h>

PGSt_SMF_status PGS_CSC_rotat6( /* rotates the position vector */
    PGSt_double* x,             /* state vector */
    PGSt_double* a,             /* angle and rate in rad and rad/time unit */ 
    PGSt_integer l,             /* axis of rotation */ 
    PGSt_double* y)             /* state vector */     
{
    PGSt_double  c;             /* cosine of the unsigned angle of rotation */
    PGSt_double  s;             /* sine of the unsigned angle of rotation */
    PGSt_double  t;             /* temporary storage to avoid overwriting */
    PGSt_double  sign;          /* sign of rotation  -- see "INPUTS" in 
                                   PROLOGUE */
    register int i;             /* i, j, and k are C range indices for the 3 
                                   axes; */
    register int j;             /* i and j are in the rotation plane and k is */
    register int k;             /* the rotation axis; i, j, and k are cyclic */

    PGSt_SMF_status returnStatus; /* indicates success or error status */

    returnStatus = PGS_S_SUCCESS;

    /* Check axis selection is in range */

    if( (l < -3) || (l > 3) || (l == 0) )
    {
        returnStatus = PGSCSC_E_BAD_ROT_AXIS_INDEX;
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_rotat6()");
        return returnStatus;
    }

    /*  Get sense of rotation - clockwise or counter */

    sign=1.0;
    if(l < 0) sign = -1.0;

    /* Here we must create a C-index in the range 0 .. 2 for the axis. */

    k = (l > 0) ? l - 1: -l -1;

    /* The following 2 lines induce a cyclic permutation mapping 1,2,3 into 
       k,j,i */

    i= (k + 1) % 3;
    j=3-i-k;

    /* nonzero elements of the 2-d rotation matrix about axis k */
    /* This is a submatrix of the 3 dimensional matrices defined on
       p. B60 of the A.A.; the 3rd dimension is unaffected since the
       3 dimensional matrix partitions into a 2-d one and the identity */

    c=cos(a[0]);
    s=sin(a[0]*sign);

    /* temporary new velocity component */

    t=x[i+3]*c+x[j+3]*s+sign*a[1]*(x[j]*c-x[i]*s); 

    /*  fix up the velocity in the (i,j) plane */

    y[j+3]=x[j+3]*c-x[i+3]*s-sign*a[1]*(x[i]*c+x[j]*s);
    y[i+3]=t;

    /* copy over the unaffected component of velocity */

    y[k+3]=x[k+3];

    /* temporary new i component of position while we fix up j component */

    t=x[i]*c+x[j]*s;

    /* fix up j component of position */

    y[j]=x[j]*c-x[i]*s;

    /* use our temporary to replace i component of position with result of 
       rotation */

    y[i]=t;

    /*  copy over the unaffected component of position */

    y[k]=x[k];

    PGS_SMF_SetStaticMsg(returnStatus,"PGS_CSC_rotat6()");
    return returnStatus;
}





