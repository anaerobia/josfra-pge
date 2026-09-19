/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_FOVconicalHull.c

DESCRIPTION:
   This file contains the function PGS_CSC_FOVconicalHull().
   
AUTHOR:
   Anubha Singhal    / Applied Research Corporation
   Peter Noerdlinger / Applied Research Corporation

HISTORY:
   04-Nov-1994 PDN Designed				
   08-Nov-1994 AS  Coded
   21-Dec-1994 AS  Changed to accept only one target point and made changes
                   suggested in code inspection
   26-May-1995 PDN Improved error messaging

END_FILE_PROLOG:
******************************************************************************/
/******************************************************************************
BEGIN_PROLOG/

TITLE: 
   This function draws a circular cone around the FOV and checks if a point is
   inside it.
   
NAME:   
   PGS_CSC_FOVconicalHull()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_FOVconicalHull(
            PGSt_integer numFOVperimVec,   
            PGSt_double  inFOVvecNorm[3],  
            PGSt_double  perimFOV_vectors[][3],
            PGSt_double  angularRadius,    
	    PGSt_double  sctoTargetVec[3],
	    PGSt_double  perimFOV_vecNorm[][3],
            PGSt_boolean *inFOVflag,      
            PGSt_double  *rMdotProduct)    
   	      
FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_SMF.f'
      
      integer function pgs_csc_fovconicalhull(numfovperimvec,
     >                                        infovvecnorm,
     >                                        perimfov_vectors,
     >                                        angularradius,
     >                                        sctotargetvec,
     >                                        perimfov_vecnorm,
     >                                        infovflag,rmdotproduct)
      
      integer           numvalues        
      integer           numfovperimvec   
      double precision  infovvecnorm(3)  
      double precision  perimfov_vectors(*,3)
      double precision  angularradius    
      double precision  sctotargetvec(3)
      double precision  perimfov_vecnorm(*,3)
      integer           infovflag     
      double precision  *rmdotproduct     
			         
DESCRIPTION:
   A circular cone is drawn around the FOV and a check is made as to whether
   the candidate point is inside it before going any further. The function has
   two purposes:

   a) it will speed up tasks by obviating complicated algorithms for points
      well away from the FOV
   b) it will enable detection and rejection of FOV specifications outside our
      present algorithmic limits. [Present software does not reliably handle
      fields of view more than 180 degrees across.]

INPUTS:
   Name              Description            Units       Min          Max
   ----              -----------            -----       ---          --- 
   numFOVperimVec    number of vectors      N/A         3            any
                     defining FOV				    
                     perimeter		    			    

   inFOVvecNorm      vector in FOV -        N/A         N/A          N/A
                     perfectly near the				    
                     center in SC 				    
		     coordinates					    

   perimFOV_vectors  vectors in SC          N/A         N/A          N/A
                     coords defining				    
                     FOV's; MUST be				    
                     sequential around				    
                     FOV						    

   angularRadius     angular radius of      radians     0            any
                     a celestial body -  			    
                     in the case of an 				    
                     earth point or 				    
		     star use 0.0				    

   scToTargetptVec   vector to target       N/A         N/A          N/A  
                     point in SC coords;
		     must be
		     normalized
		    
OUTPUTS:
   Name              Description            Units       Min          Max
   ----              -----------            ----        ---          ---
   perimFOV_vecNorm  vectors in SC          N/A         N/A          N/A
                     coords defining
                     FOV's normalized

   inFOVflag         PGS_TRUE if target     N/A         N/A          N/A
                     point is in conical
		     FOV else PGS_FALSE

   rMdotProduct      smallest dot product   N/A         0.0          1.0
                     between the inFOV
		     vector and any 
		     perimeter vector              

RETURNS:
   PGS_S_SUCCESS                 Successful return
   PGSCSC_E_INVALID_FOV_DATA     FOV perimeter vectors or inFOVvector invalid
   PGSCSC_E_FOV_TOO_LARGE        FOV specification outside algorithmic limits
                         
EXAMPLES:
C:
   #define PERIMVEC_SIZE 4

   PGSt_SMF_status    returnStatus;
   PGSt_integer       numFOVperimVec;
   PGSt_double        inFOVvecNorm[3] = {0.0,0.0,1.0};
   PGSt_double        perimFOV_vectors[PERIMVEC_SIZE][3]=
                                  { {100.0,100.0,100.0},
				    {-100.0,100.0,100.0},
				    {-100.0,-100.0,100.0},
				    {100.0,-100.0,100.0}
				  };  		      
   PGSt_boolean       inFOVflag;
   PGSt_double        sctoTargetVec[3];
   PGSt_double        angularRadius = 0.0;    
   PGSt_double        perimFOV_vecNorm[PERIMVEC_SIZE][3];
   PGSt_double        rMdotProduct;
  
   numFOVperimVec = PERIMVEC_SIZE;
   
   returnStatus =  PGS_CSC_FOVconicalHull(numFOVperimVec,inFOVvecNorm,
                                          perimFOV_vectors,angularRadius,
					  sctoTargetVec,perimFOV_vecNorm,
					  &inFOVflag,&rMdotProduct);

   if(returnStatus != PGS_S_SUCCESS)
   {
     ** test errors, take appropriate action **
   }

FORTRAN:
      implicit none
      integer       	     pgs_csc_fovconicalhull  
      integer                returnstatus
      integer                numfovperimvec
      double precision       infovvecnorm(3)
      double precision       perimfov_vectors(4,3)
      integer                infovflag
      double precision       sctotargetvec(3)
      double precision       angularradius   
      double precision       perimfov_vecnorm(4,3)
      double precision       rmdotproduct
      
      perimfov_vectors(1,1) = 100.0
      perimfov_vectors(2,1) = 100.0
      perimfov_vectors(3,1) = 100.0
      
      perimfov_vectors(1,2) = -100.0
      perimfov_vectors(2,2) = 100.0
      perimfov_vectors(3,2) = 100.0
      
      perimfov_vectors(1,3) = -100.0
      perimfov_vectors(2,3) = -100.0
      perimfov_vectors(3,3) = 100.0
      
      perimfov_vectors(1,4) = 100.0
      perimfov_vectors(2,4) = -100.0
      perimfov_vectors(3,4) = 100.0
      
      infovvecnorm(1) = 0.0
      infovvecnorm(2) = 0.0
      infovvecnorm(3) = 0.0
      
      angularradius = 0.0  
      
      returnstatus = pgs_csc_fovconicalhull(numfovperimvec,
     >                                      infovvecnorm,
     >                                      perimfov_vectors,
     >                                      perimfov_vecnorm,
     >                                      angularradius,sctotargetvec,
     >                                      infovflag,rmdotproduct)
      
      if (returnstatus .ne. pgs_s_success) then
          pgs_smf_getmsg(returnstatus, err, msg)
          write(*,*) err, msg
      endif
        
NOTES:
   The inFOV vector must be normalized before passing it to this function.
   
REQUIREMENTS:
   PGSTK - 1090 

DETAILS:
      See "Theoretical Basis of the SDP Toolkit Geolocation Package for the
      ECS Project", Document 445-TP-002-002, May 1995, by P. Noerdlinger, 
      for more information on the algorithm.

GLOBALS:
   NONE

FILES:
   NONE

FUNCTIONS CALLED:
   PGS_CSC_dotProduct()
   PGS_CSC_Norm() 
   PGS_SMF_SetStaticMsg()

END_PROLOG
******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <stdlib.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_FOVconicalHull()"

PGSt_SMF_status
PGS_CSC_FOVconicalHull(            /* determine if a point is in the conical 
				      hull of a field of view */ 
    PGSt_integer numFOVperimVec,   /* number of perimeter vectors defining the
				      field of view */
    PGSt_double  inFOVvecNorm[3],  /* vector in the field of view normalized */
    PGSt_double  perimFOV_vectors[][3],/* FOV perimeter vectors */
    PGSt_double  angularRadius,    /* angular radius of a celestial body - in 
				      the case of an earth point or star use
                                      zero */
    PGSt_double  sctoTargetVec[3], /* vector to target point in SC 
 				      coordinates */
    PGSt_double  perimFOV_vecNorm[][3],/* normalized FOV perimeter vectors */
    PGSt_boolean *inFOVflag,       /* set to PGS_TRUE if target point is in the
				      field of view else PGS_FALSE */
    PGSt_double  *rMdotProduct)    /* smallest dot product between the inFOV
				      vector and any perimeter vector - does 
				      not include the calculation for the 
				      entire disk in the case of a celestial
				      body */
{
    PGSt_double	 norm;	            /* norm of any  vector */
    PGSt_double  dotProduct;        /* dot product of vectors */
    PGSt_integer counter;           /* loop counter */
    short        cnt2;              /* loop counter */
    PGSt_double  rMdotProductPlus;  /* smallest dot product between the inFOV
				       vector and any perimeter vector - 
				       includes the calculation for the 
				       entire disk in the case of a celestial
				       body */
    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
    
    /* initialize return value to indicate success */
    
    returnStatus = PGS_S_SUCCESS; 

    /* normalize the FOV perimeter vectors  */
        
    for (counter = 0; counter < numFOVperimVec; counter++)
    {
		
	norm = PGS_CSC_Norm(perimFOV_vectors[counter]);
		
	if (fabs(norm) < EPS_64)
        {  
            returnStatus = PGSCSC_E_INVALID_FOV_DATA;
	    PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );  
	    return returnStatus;
        }
        else
          for (cnt2 = 0; cnt2 < 3; cnt2++)
	    perimFOV_vecNorm[counter][cnt2] = perimFOV_vectors[counter][cnt2] 
	                                      / norm;  
    }
    
    *rMdotProduct = 1.1;
    
    /* find the largest angular distance from the inFOVvector to any of the
       perimeter vectors. The method is to take the angle rM which is the arc
       cosine of the smallest dot product between the center vector and any
       other. In the case of determining if a point is in the FOV then its
       sufficient to find the cosine of rM which is the smallest of the dot
       products */

    for (counter = 0; counter < numFOVperimVec; counter++)
    {
	dotProduct = PGS_CSC_dotProduct(perimFOV_vecNorm[counter],inFOVvecNorm,
					3);

	if (dotProduct < 0.0)
	{
	    returnStatus = PGSCSC_E_FOV_TOO_LARGE;
	    PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );
	    return returnStatus;
	}
	
	if (dotProduct < *rMdotProduct)
	  *rMdotProduct = dotProduct;
    }

    /* if the smallest dot product equals or nearly equals 1.0 then all the
       perimeter vectors are nearly the same as each other and the infov vector 
       so return with PGSCSC_E_INVALID_FOV_DATA.  The number used for the test,
       0.99999999999, amounts to defining an angle of ~0.9 arc sec between
       the perimeter vertex and the inFOVvector. A distance smaller than
       that could lead to erratic performance of the algorithm due to
       truncation error.  */
 
    if (*rMdotProduct > .99999999999)
    {
        returnStatus = PGSCSC_E_INVALID_FOV_DATA;
        return returnStatus;
    }


    /* in the case of angularRadius being greater than zero, a check must be 
       made to see if the entire disk of the CB lies outside the FOV 
       circumscribed cone - the calculation to include this is as follows */
    
    rMdotProductPlus = (angularRadius > 0.0) ? 
                       cos(acos(*rMdotProduct) + angularRadius) : *rMdotProduct;
    
    /* see if the point is inside the conical field of view */
    
    dotProduct = PGS_CSC_dotProduct(sctoTargetVec,inFOVvecNorm,3);
    
    if (dotProduct >= rMdotProductPlus)
	*inFOVflag = PGS_TRUE;
    else
	*inFOVflag = PGS_FALSE;
    
    return returnStatus;
}
