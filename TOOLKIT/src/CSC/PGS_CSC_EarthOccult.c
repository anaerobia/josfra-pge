/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_CSC_EarthOccult.c

DESCRIPTION:
   This file contains the function PGS_CSC_EarthOccult().
   
AUTHOR:
   Anubha Singhal    / Applied Research Corporation
   Peter Noerdlinger / Applied Research Corporation

HISTORY:
   04-Nov-1994 PDN Designed				
   29-Nov-1994 AS  Coded
   21-Dec-1994 AS  Changed to accept only one target point
   09-Jan-1995 AS  Made changes suggested in code inspection
   04-Jun-1995 PDN Altered returns to conform with TK standards

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE: 
   Test for Earth occultation of a celestial body.
   
NAME:   
   PGS_CSC_EarthOccult()

SYNOPSIS:
C:
   #include <PGS_CSC.h>
   
   PGSt_SMF_status
   PGS_CSC_EarthOccult(
       PGSt_double  inFOVvector[3],    
       PGSt_double  rMdotProduct,      
       PGSt_double  rAngCB,  
       PGSt_double  cb_SCvector[3],  
       PGSt_double  scToEcenter[3],  
       PGSt_double  extremeVec[3],
       PGSt_boolean *inFOVflag)       
   	      
FORTRAN:
      include 'PGS_CSC_4.f'
      include 'PGS_SMF.f'
      
      integer function pgs_csc_earthoccult(infovvector,rmdotproduct,
     >                                     rangcb,cb_scvector,
     >                                     sctoecenter,extremevec,
     >                                     infovflag)  
      
      double precision  infovvector(3)    
      double precision  rmdotproduct      
      double precision  rangcb  
      double precision  cb_scvector(3)  
      double precision  sctoecenter(3)  
      double precision  extremevec(3)
      integer           infovflag 
			         
DESCRIPTION:
   Test for earth occultation of a celestial body in the field of view. The 
   test is in three phases. The first phase does not depend on the CB at all -
   it is just a check if the Earth fills the field of view. The second test
   (exercised only if the first fails to find total occultation) determines if
   the celestial body is behind the Earth. If the second test fails, the
   vector in SC coordinates that points at the part of the CB most distant 
   from the Earth center is returned so that the calling function can 
   determine if the Earth's bulge (difference in radius over that of an 
   inscribed sphere) occults the CB.

INPUTS:
   Name              Description             Units       Min         Max
   ----              -----------             -----       ---         --- 
   inFOVvector       vector in FOV -         N/A         N/A         N/A
                     preferably near the
                     center in SC 
		     coordinates

   rMdotProduct      smallest dot product    N/A         0.0         1.0
                     between the inFOV
		     vector and any 
		     perimeter vector   

   angRadCB          angular radius of       radians     0           any
                     the celestial body 

   cb_SCvector       vector to target        meters      N/A         N/A  
                     point in SC coords;
		     must be normalized

   scToEcenter       spacecraft to earth     meters      N/A         N/A
                     center vector
		    
OUTPUTS:
   Name              Description             Units       Min         Max
   ----              -----------             ----        ---         ---
   extremeVec        vector in SC            meters      N/A         N/A
                     coordinates that 
                     points at the part of 
                     the CB most distant 
                     from Earth center 

   inFOVflag         PGS_TRUE if there is    N/A         N/A         N/A
                     no earth occultation
		     else PGS_FALSE
              
RETURNS:
   PGS_S_SUCCESS                 Successful return
   PGSCSC_W_DATA_FILE_MISSING    the data file earthfigure.dat is missing
   PGSCSC_M_EARTH_BLOCKS_CB      Earth blocks the celestial body 
   PGSCSC_M_EARTH_BLOCKS_FOV     Earth blocks the FOV
   PGSCSC_M_CHECK_EARTH_BULGE    check to see if the Earth's equatorial bulge
                                 occults the CB
                         
EXAMPLES:
C:
   
    PGSt_SMF_status    returnStatus;
    PGSt_double        inFOVvector[3];
    PGSt_boolean       inFOVflag;
    PGSt_double        cb_SCvector[3];
    PGSt_double        scToEcenter[3];
    PGSt_double        extremeVec[3];
    PGSt_double        angRadCB = 0.5;   
    PGSt_double        rMdotProduct = 0.7;

    returnStatus = PGS_CSC_EarthOccult(inFOVvector,rMdotProduct,rAngCB,
                                       cb_SCvector,scToEcenter,extremeVec,
				       &inFOVflag)     

    if(returnStatus != PGS_S_SUCCESS)
    {
                      :
     ** test errors, take appropriate action **
		      :
    }

FORTRAN:
      implicit none
      integer             pgs_csc_earthoccult
      integer		  returnstatus
      double precision    infovvector(3)
      integer             infovflag
      double precision    cb_scvector(3)
      double precision    sctoecenter(3)
      double precision    extremevec(3) 
      double precision    rangcb  
      double precision    rmdotproduct

      rangcb = 0.5
      rmdotproduct = 0.7   
      
      returnstatus = pgs_csc_earthoccult(infovvector,rmdotproduct,
     >                                   rangcb,cb_scvector,sctoecenter,
     >                                   extremevec,infovflag)
      
      if (returnstatus .ne. pgs_s_success) then
	  pgs_smf_getmsg(returnstatus, err, msg)
	  write(*,*) err, msg
      endif
        
NOTES:
   The vectors inFOVvector and cb_SCvector must be normalized before being
   passed to this function.
   
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
   PGS_CSC_GetEarthFigure()
   PGS_CSC_Norm()
   PGS_CSC_dotProduct()
   PGS_SMF_SetStaticMsg()

END_PROLOG
*******************************************************************************/
#include <stdio.h>
#include <PGS_math.h>
#include <stdlib.h>
#include <PGS_CSC.h>

/* name of this function */

#define FUNCTION_NAME "PGS_CSC_EarthOccult()"

PGSt_SMF_status
PGS_CSC_EarthOccult(                /* test for earth occultation of a
				       celestial body */ 
    PGSt_double  inFOVvector[3],    /* vector in the field of view normalized */
    PGSt_double  rMdotProduct,      /* smallest dot product between the inFOV
				       vector and any perimeter vector */
    PGSt_double  rAngCB,            /* angular radius of the Celestial Body */
    PGSt_double  cb_SCvector[3],    /* vector to target point in SC 
				       coordinates */
    PGSt_double  scToEcenter[3],    /* position vectors */
    PGSt_double  extremeVec[3],     /* vector in SC coordinates that points at
				       the part of the CB most distant from 
				       Earth center */
    PGSt_boolean *inFOVflag)        /* set to PGS_TRUE if target point is in 
				       the field of view else PGS_FALSE */
{
    PGSt_double	 norm;	            /* norm of any vector */
    PGSt_double  scToEC;            /* norm of spacecraft to Earth vector */
    PGSt_double  tempVar;           /* cosine of the sum of rMdotProduct and 
				       the angle between Earth center and the 
				       inFOVvector*/
    PGSt_double  unitEarthVec[3];   /* unit vector from spacecraft to Earth */
    PGSt_double  earthDotInFOV;     /* dot product of unit vector from
				       spacecraft to Earth with inFOVvector */
    PGSt_double  eta[3];            /* unit vector from spacecraft to 
				       Celestial body  */
    PGSt_double  scCBdotSCearth;    /* dot product of unit vector 
				       from spacecraft to celestial body with
				       unit vector from spacecraft to Earth */
    PGSt_double  CBtoExtremePtLen;  /* length from celestial body to extreme
				       point */
    PGSt_double  earthToCBvec[3];   /* vector from earth to celestial body */
    short        cnt2;              /* loop counter */
    PGSt_double  angRadEsin;        /* the angular radius of a disk on the sky
				       inscribed in the oblate earth */
    PGSt_double  bigAngRadEsin;     /* the angular radius of a disk on the sky
				       inscribed in the earth */
    PGSt_double  equatRad_A;        /* equatorial radius */
    PGSt_double  polarRad_C;        /* polar radius */
    PGSt_SMF_status returnStatus;   /* error returnStatus of function call */
       
    /* initialize return value to indicate success */
    
    returnStatus = PGS_S_SUCCESS; 

    /* get equatorial and polar radius */ 

    returnStatus = PGS_CSC_GetEarthFigure("WGS84",&equatRad_A,&polarRad_C);

    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
        break;
      case PGSCSC_W_DATA_FILE_MISSING:
        return returnStatus;
      default:
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );	
        return returnStatus;
    }

    /* initialize inFOVflag to PGS_TRUE */

    *inFOVflag = PGS_TRUE;
    
    /* find the norm of the earth center to spacecraft coordinates vector */
    
    scToEC = PGS_CSC_Norm(scToEcenter);
    
    /* find the unit vector from the spacecraft to the earth */
    
    for (cnt2 = 0; cnt2 < 3; cnt2++)
      unitEarthVec[cnt2] = scToEcenter[cnt2] / scToEC;
    
    /* case 1 - determine if there is total FOV blockage */
    
    /* find the sin of the angular radius of a disk on the sky inscribed in 
       the oblate earth */
    
    angRadEsin = polarRad_C / scToEC;
    bigAngRadEsin = equatRad_A / scToEC;
    
    /* test if the angular distance from the inFOVvector to the Earth 
       Center is less than the angular Earth radius (angRadEsin) minus the
       half cone angle (rMax) */
    
    earthDotInFOV = PGS_CSC_dotProduct(unitEarthVec,inFOVvector,3);
    
    /* determine if the earth occults the FOV */
    
    /* identities 4.4.33 and 4.4.35 on p. 80 of the U.S. Dept of Commerce
       "Handbook of Mathematical Functions" have been used to eliminate 
       angles */

    tempVar = (earthDotInFOV * rMdotProduct) - 
      sqrt((1.0 - earthDotInFOV * earthDotInFOV) * 
	   (1.0 - rMdotProduct * rMdotProduct));
    
    if ((angRadEsin * tempVar) > 
	sqrt((1.0 - angRadEsin * angRadEsin) * (1.0 - tempVar * tempVar)))
    {
		
	*inFOVflag = PGS_FALSE;
		
	returnStatus = PGSCSC_M_EARTH_BLOCKS_FOV; 
	
	PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );	
	return returnStatus; 
    }
    
    /* case 2 - determine if the celestial body is behind the earth */
    
    norm = PGS_CSC_Norm(cb_SCvector);
    
    for (cnt2 = 0; cnt2 < 3; cnt2++)
      eta[cnt2] = cb_SCvector[cnt2] / norm;
    
    scCBdotSCearth = PGS_CSC_dotProduct(unitEarthVec,eta,3);
    
    /* identities 4.4.33 and 4.4.35 on p. 80 of the U.S. Dept of Commerce 
       "Handbook of Mathematical Functions" have been used to eliminate 
       angles */

    if ( (angRadEsin*scCBdotSCearth -
	  sqrt((1.0 - angRadEsin*angRadEsin)*
	       (1.0 - scCBdotSCearth*scCBdotSCearth)))
	>
        sin(rAngCB) )
    {
	*inFOVflag = PGS_FALSE;
		
	returnStatus = PGSCSC_M_EARTH_BLOCKS_CB; 
		
	PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );
	return returnStatus;
		
    }
    else
    {   
	if ((bigAngRadEsin * scCBdotSCearth - 
	     sqrt((1.0 - bigAngRadEsin * bigAngRadEsin) *
		  (1.0 - scCBdotSCearth * scCBdotSCearth))) <= sin(rAngCB))
	  return returnStatus;	    
    }
    
    /* case 3 - construct a vector in SC coordinates that points at the part of
       the CB most distant from the Earth center(extreme vector). If the calling
       function finds the CB to lie in the FOV then it will use the extreme
       vector to determine cases where the Earth's bulge occults the CB */
    
    /* find the length from the celestial body center to the extreme point */
        
    if (fabs(rAngCB) > EPS_64)
    {
	
	CBtoExtremePtLen = sin(rAngCB) * (1.0 / sin(rAngCB + 
					 (acos(scCBdotSCearth) / 2.0)));
    
	/* find the vector from the earth center to the celestial body center
	   which will be used in constructing the spacecraft to extreme point
	   vector */
    
	for (cnt2 = 0; cnt2 < 3; cnt2++)
	  earthToCBvec[cnt2] = cb_SCvector[cnt2] - scToEcenter[cnt2];
    
	/* construct the extreme point vector - this is found by adding the
	   spacecraft to celestial body vector to the celestial body to extreme
	   point vector */
    
	for (cnt2 = 0; cnt2 < 3; cnt2++)
	  extremeVec[cnt2] = cb_SCvector[cnt2] +
	    (CBtoExtremePtLen * earthToCBvec[cnt2]);
    
	/* normalize the extreme point vector */
    
	norm = PGS_CSC_Norm(extremeVec);
    
	for (cnt2 = 0; cnt2 < 3; cnt2++)
	  extremeVec[cnt2] = extremeVec[cnt2] / norm; 
    }
    else
      for (cnt2 = 0; cnt2 < 3; cnt2++)
	extremeVec[cnt2] = cb_SCvector[cnt2]; 

    returnStatus = PGSCSC_M_CHECK_EARTH_BULGE;
    PGS_SMF_SetStaticMsg( returnStatus, FUNCTION_NAME );
    return returnStatus;
}
