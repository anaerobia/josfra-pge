/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_PointInFOVgeom.c

DESCRIPTION:
   This file contains the function PGS_CSC_PointInFOVgeom().
   
AUTHOR:
   Anubha Singhal    / Applied Research Corporation
   Peter Noerdlinger / Applied Research Corporation

HISTORY:
   25-Oct-1994 PDN Designed				
   26-Oct-1994 AS  Coded
   21-Dec-1994 AS  Changed to accept only one target point and made changes
                   suggested in code inspection
   25-May-1995 PDN fixed test of angleSign to bound totalAng away from 0 
   31-May-1995 PDN fixed code for case of candidate point on boundary 
   07-Nov-1996 PDN Fixed description of the OUTPUT

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   This function contains the geometry to determine if a point is in the field
   of view.
   
NAME:   
   PGS_CSC_PointInFOVgeom()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   #include <PGS_MEM.h>

   PGSt_SMF_status
   PGS_CSC_PointInFOVgeom(
	     PGSt_integer numFOVperimVec,     
	     PGSt_double  inFOVvector[3],       
	     PGSt_double  perimFOV_vectors[][3],
	     PGSt_double  scToTargetVec[3],   
	     PGSt_boolean *inFOVflag)          
					 
FORTRAN:
   include 'PGS_CSC_4.f'
   include 'PGS_MEM_4.f'
   include 'PGS_SMF.f'

   integer function
      pgs_csc_pointinfovgeom(numfovperimvec,infovvector,
                             perimfov_vecnorm,sctotargetvec,infovflag)  
			     
      integer           numfovperimvec     
      double precision  infovvector(3)       
      double precision  perimfov_vectors(3,*)
      double precision  sctotargetvec(3)   
      integer           infovflag
   
DESCRIPTION:
   For each input point, the function does the processing to determine if a
   point is in the field of view and returns a flag indicating whether the
   point is in the field of view.
   
INPUTS:
   Name             Description        Units    Min                 Max
   ----             -----------        -----    ---                 --- 
   numFOVperimVec   number of vectors  N/A      3                   any
                    defining FOV
                    perimeter
   inFOVvector      specimen vector in N/A      N/A                 N/A
                    in FOV in SC 
		    coordinates
   perimFOV_vectors vectors in SC      N/A      N/A                 N/A
                    coords defining
                    FOV's; MUST be
                    sequential around
                    FOV and must be
		    normalized
   scToTargetptVec  vector to target   N/A      -1                   1 
                    point in SCcoords;
		    must be
		    normalized
  	    		    
OUTPUTS:
   Name           Description                     Units       Min       Max
   ----           -----------                     ----        ---       ---
   inFOVflag      PGS_TRUE if target point is in  N/A         N/A       N/A
                  FOV, PGS_FALSE if the point is
		  outside 

RETURNS:
   PGS_S_SUCCESS                success
   PGSMEM_E_NO_MEMORY           no memory is available to allocate vectors
   PGSCSC_E_INVALID_FOV_DATA    invalid inFOVvector - on the FOV boundary

EXAMPLES:
C:

   #define   PERIMVEC_SIZE 4
   PGSt_SMF_status    returnStatus;
   PGSt_integer       numFOVperimVec;
   PGSt_double        inFOVvector[3] = {0.0,0.0,1.0};
   PGSt_double        perimFOV_vectors[PERIMVEC_SIZE][3]=
                                  { {0.0,1.0,0.0},
				    {-1.0,0.0,0.0},
				    {0.0,-1.0,0.0},
				    {1.0,0.0,0.0}
				  };  		      
   PGSt_boolean       inFOVflag;
   PGSt_double        sctoTargetVec[3];
  
   numFOVperimVec = PERIMVEC_SIZE;
   
   returnStatus =  PGS_CSC_PointInFOVgeom(numFOVperimVec,inFOVvector,
                              perimFOV_vectors,sctoTargetVec,&inFOVflag);

   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, 
         take appropriate
          action **
   }

FORTRAN:
   implicit none
   integer		  pgs_csc_pointinfovgeom
   integer		  returnstatus
   integer                numfovperimvec
   double precision       infovvector(3)
   double precision       perimfov_vectors(4,3)
   integer                infovflag
   double precision       sctotargetvec(3)
   
   perimfov_vectors(1,1) = 0.0
   perimfov_vectors(2,1) = 1.0
   perimfov_vectors(3,1) = 0.0

   perimfov_vectors(1,2) = -1.0
   perimfov_vectors(2,2) = 0.0
   perimfov_vectors(3,2) = 0.0

   perimfov_vectors(1,3) = 0.0
   perimfov_vectors(2,3) = -1.0
   perimfov_vectors(3,3) = 0.0

   perimfov_vectors(1,4) = 1.0
   perimfov_vectors(2,4) = 0.0
   perimfov_vectors(3,4) = 0.0

   infovvector(1) = 0.0
   infovvector(2) = 0.0
   infovvector(3) = 1.0

   returnstatus = pgs_csc_pointinfovgeom(numfovperimvec,infovvector,
                             perimfov_vectors,sctotargetvec,infovflag)

   if (returnstatus .ne. pgs_s_success) then
	 pgs_smf_getmsg(returnstatus, err, msg)
	 write(*,*) err, msg
   endif
                                                          
NOTES:
   The inFOVvector, perimFOV_vectors and scToTargetptVec must all be
   normalized.

REQUIREMENTS:
   PGSTK - 1090 

DETAILS:
   NONE

GLOBALS:
   NONE

FILES:
   NONE

FUNCTIONS CALLED:
   PGS_MEM_Free()
   PGS_MEM_Malloc()
   PGS_CSC_dotProduct()
   PGS_CSC_crossProduct()
   PGS_CSC_Norm()
   PGS_SMF_SetStaticMsg()

END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <PGS_math.h>
#include <stdlib.h>
#include <PGS_CSC.h>
#include <PGS_MEM.h>

#define POINT99TWOPI 6.2203534541078 /* (0.99)*(2.0)*(PI) */

PGSt_double CheckPointInFOV(PGSt_double[3],
                            PGSt_integer,
			    PGSt_double[][3],
			    PGSt_double[][3]);

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_PointInFOVgeom()"

PGSt_SMF_status
PGS_CSC_PointInFOVgeom(                /* geometrical determination if a point
					  is in the field of view */ 
    PGSt_integer numFOVperimVec,       /* number of perimeter vectors defining
					  the field of view */
    PGSt_double  inFOVvector[3],       /* specimen vector in the field of 
					  view */
    PGSt_double  perimFOV_vectors[][3],/* FOV perimeter vectors */
    PGSt_double  scToTargetVec[3],     /* target vectors in spacecraft 
				          coordinates */
    PGSt_boolean *inFOVflag)           /* set to PGS_TRUE if target point is in
					  the field of view */
{
    PGSt_double  (*targetToPerimVec)[3]=NULL; /* vectors from tip of target
                                                 vector to perimeter vector;
                                                 orthogonal to the latter */

    PGSt_double  totalAng;             /* total cumulated angle between line 
					  segments from the tip of target 
					  vectors to perimeter vector */
    PGSt_double  angleSign;            /* sign of the total sum of the angles
				          between line segments from the in 
					  field of view vector to the 
					  perimeter vector*/
    PGSt_SMF_status returnStatus;      /* error returnStatus of function call */
    /* initialize return value to indicate success */

    returnStatus = PGS_S_SUCCESS; 

    /* initialize inFOVflag to PGS_FALSE */

    *inFOVflag = PGS_FALSE;
    
    /* allocate space for target to perimeter vector */

    returnStatus = PGS_MEM_Malloc((void**)&targetToPerimVec,
				   (sizeof(PGSt_double)*3*numFOVperimVec));

    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	break;
      case PGSMEM_E_NO_MEMORY:
	return returnStatus;
      default:
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );      
        return returnStatus;
    }
    
    /* determine if the inFOVvector vector is in the field of view */
      
    totalAng = CheckPointInFOV(inFOVvector,numFOVperimVec,perimFOV_vectors,
			       targetToPerimVec);
    
    /* save the sign of the total angle */
    
    if( totalAng > POINT99TWOPI ) 
    {
       angleSign =  1.0;
    }
    else if( totalAng < - POINT99TWOPI ) 
    {
       angleSign = - 1.0;
    }
    else
    {
       /* Notice - the following return is issued only with
          the inFOVvector as "target", as is correct. Other-
          wise a totalAng not in a good range indicates that 
          the CB is on the FOV boundary!  */

       returnStatus = PGSCSC_E_INVALID_FOV_DATA;
       PGS_MEM_Free(targetToPerimVec);
       return returnStatus;
    }
    
    /* determine if the target vector is in the field of view */

    totalAng = CheckPointInFOV(scToTargetVec,numFOVperimVec,perimFOV_vectors,
			       targetToPerimVec);

    /* if the total angle is greater than 1.0  then check if the
       sign is the same as the inFOV vector to determine if the vector
       is in the field of view.  The value 1.0 is somewhat arbitrary -
       normally the value always turns out to be within platform-
       limited truncation accuracy of 2 pi or - 2 pi, unless the
       scToTargetVec is on the boundary of the FOV. In that case you
       can get almost any value - we include most such cases by testing
       against 1.0 since you usually pick up that much angle from the
       remaining boundary arcs of the FOV */

    if (fabs(totalAng) > 1.0  )
    {    
	if ((totalAng < 0.0) == (angleSign < 0.0))
	  *inFOVflag = PGS_TRUE;
    }

    /* free the memory allocated */

    PGS_MEM_Free(targetToPerimVec);

    return returnStatus;
}

PGSt_double CheckPointInFOV(PGSt_double  scToTargetVec[3],
                            PGSt_integer numFOVperimVec,
			    PGSt_double  perimFOV_vectors[][3],
			    PGSt_double  targetToPerimVec[][3])
{
    PGSt_double  norm;            /* norm of vector */
    PGSt_double  dotProduct;      /* dot product of vectors */
    PGSt_double  crossProduct[3]; /* cross product of vectors */
    PGSt_integer counter2;        /* loop counter */
    PGSt_integer cnt3;            /* loop counter */
    PGSt_double  tripScale;       /* triple scalar product of vectors */
    PGSt_double  totalAng;        /* total cumulated angle between line 
				     segments from the tip of target 
				     vectors to perimeter vector */
    
    /* construct a line segment from the perimeter vector orthogonal to the
       target vector */

    for (counter2 = 0; counter2 < numFOVperimVec; counter2++)
    {

	dotProduct = PGS_CSC_dotProduct(scToTargetVec,
					perimFOV_vectors[counter2],3);

	for (cnt3 = 0; cnt3 < 3; cnt3++)
	  targetToPerimVec[counter2][cnt3] = 
	    perimFOV_vectors[counter2][cnt3] 
	      - scToTargetVec[cnt3] * dotProduct;

	/* normalize target to perimeter vector */

	norm = PGS_CSC_Norm(targetToPerimVec[counter2]);
	if (fabs(norm) > EPS_64)
	  for (cnt3 =0 ; cnt3 < 3; cnt3++)
	    targetToPerimVec[counter2][cnt3] = 
	      targetToPerimVec[counter2][cnt3] / norm;

    }    
 
    /* calculate the total angle from the tip of the target to the tip of each
       perimeter vector */
    
    totalAng = 0.0;

    for (counter2 = 0; counter2 < numFOVperimVec; counter2++)
    { 
	cnt3 = (counter2 + 1) % numFOVperimVec;
	PGS_CSC_crossProduct(targetToPerimVec[counter2],
			     targetToPerimVec[cnt3],
			     crossProduct);
	
	/* find the dot product of the crossProduct calculated in the last
	   step with the target vector */
            
	tripScale = PGS_CSC_dotProduct(crossProduct,scToTargetVec,3);

        /* normal case - the two perimeter vectors and the vector
           to the target span a volume (are not coplanar).  Find 
           the angle about which a vector orthogonal to the axis 
           defined by the target vector and pointing first at the 
           line of one vertex, then the line of the next, swings
           around the target vector axis in moving.  */

	if( fabs(tripScale) > EPS_64)
        {
	   dotProduct = PGS_CSC_dotProduct(targetToPerimVec[counter2],
					   targetToPerimVec[cnt3],3);

           totalAng += atan2(tripScale,dotProduct);  
        }
        /*  In case tripScale = 0.0, , the target vector lies on the plane 
            defined by the two perimeter vectors.  It is impossible to say 
            if it is in or outside the FOV.  We shall not increment the 
            angle by which the line from the target vector to the seq-
            uence of perimeter vectors moves in this case.  Thus, the
            answer could come out almost anything. */
    }

    return totalAng;	    
}
