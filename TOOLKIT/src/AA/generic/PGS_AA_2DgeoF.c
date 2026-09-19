/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:  

FILENAME:   
  	PGS_AA_2DgeoF.c 
 
DESCRIPTION:
  	The interface to the calling algorithm which extracts gridded 
        data by geographic location.


AUTHOR:
  	Graham Bland / EOSL 

HISTORY:
  	07-July-94 	GJB 	Initial version
  	11-Aug -94 	GJB     Update for coding standards
	31-August-94    GJB     Post review updates 
	03-October-94   GJB     Remove F calls in favour of new version
	25-January-95	SGT	Added "undef _CFE" to get around SGI IRIX 5.2 compile error
	16-Feb-95       GJB     Add frees
	23-Jun-95	ANS	Removed cfortran inclusion and binding
				Changed val to static 
	11-July-95 	ANS	Improved fortran example in prolog
	19-July-95	ANS	Fixed ECSed00780
	25-July-95	ANS	Fixed ECSed01025
				
  
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h> 

#undef _CFE
#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_10.h>
#include <PGS_AA_Tools.h>

/*----   mnemonics -----*/

/*----   global variables  ----*/

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Extract data from gridded data sets by geographic location
  
NAME:  
	PGS_AA_2Dgeo

SYNOPSIS:
C:	
        #include <PGS_AA.h>
	
	PGSt_SMF_status
        PGS_AA_2Dgeo ( char  iparms[][100], 
	     PGSt_integer nParms,
	     PGSt_double latitude[],
	     PGSt_double longitude[],
	     PGSt_integer nPoints,
	     PGSt_integer fileId,
	     PGSt_integer version,
	     PGSt_integer operation, 
	     void *results);

FORTRAN:
         include 'PGS_AA_10.f'
	
     
         integer function pgs_aa_2dgeo( parms, nparms, latitude, longitude, 
	          fileId, version, operation, results ) 
         character*99 parms(*)
         integer      nParms
         real*8       latitude(*)
         real*8       longitude(*)
         integer      fileId
         integer      version
         integer      operation
         'user specified'   results (see Notes)
     
   
DESCRIPTION:
 The interface to the calling algorithm which extract gridded data by 
 geographic location.

 [start]
 PERFORM	PGS_AA_Map
 PERFORM	PGS_AA_GetSupp	to get support data 
 DO	allocate memory to parmBuffer using totalParmMemoryCache 
 PERFORM	PGS_AA_FF_Setup
 DO	set tool used to 2
 PERFORM	PGS_AA_GEOGrid
 [end]

INPUTS:
        Name		Description	       Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names        	see notes
                        requested
        nParms          number of parms         none    1       #defined
        latitude        latitude(s) of the      degrees -90.00  90.00
                        requested point
        longitude       longitude(s) of the     degrees -180.00 180.00
                        requested point
        nPoints         no. of points requested none    1       variable
        fileId          logical file number     none   variable variable
        version         version of dynamic file none    1       variable
        operation       defines user required   none    1       variable
        

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        results         results                 see notes 
	

RETURNS:   (to user and log file - u/l)
 u  PGSAA_E_GEOERROR		error in GEO extraction 
 l  PGSAA_E_AUTOOPERATION       error in executing autoOperation
 l  PGSAA_E_AUTOOPERATIONUNSET	no autOperation found in support file
 l  PGSAA_E_OPERATION		error in executing operation
 l  PGSAA_E_OPERATIONUNSET	operation not set by user
 ul PGSAA_E_GEOTOSTRUCT		failure in calculation of structure from lat/lon
 ul PGSAA_E_UNIDENTIFIEDTYPE	type cannot be identified, results failure             
 u  PGSAA_E_SUPPORTFILE	        support or format files inaccessible
 ul PGSAA_E_PARMSNOTFOUND       parameter(s) not found in the support support file
 l  PGSAA_E_DATARATEUNSET       dataRate attribute unset in support file
 ul PGSAA_E_PARMSFROMANYFILES   parameters requested from more than one physical file
 ul PGSAA_E_INVALIDNOPARMS      no of parms incorrect
 ul PGSAA_E_BADSUPPSUPPORT      tool support file is corrupted or incomplete
 ul PGSAA_E_CANTFINDFILE        format of input data file inaccessible
 u  PGSAA_E_FFERROR	        a freeform error has occured
 l  PGSAA_E_FFDBIN	        failure in Freeform make_dbin function
 l  PGSAA_E_FFDBSET	        failure in Freeform db_set function
 l  PGSAA_E_FFDBEVENTS	        failure in Freeform db_events function
 ul PGSAA_E_MALLOC	        failure to malloc 
 u  PGSAA_E_PEV_ERROR		an error has occured in the PeV tool
 l  PGSAA_E_PEV_XS_SUPPFILES	too many PeV files open, increase MAXFILES
 l  PGSAA_E_CANT_GET_VALUE	unable to extract value from dbin
 l  PGSAA_E_GETDBIN		errror in PeV tool obtaining dbin 
 u  PGSAA_E_GETSUPP		an error was detected while extracting support data
 l  PGSAA_E_POSITION_CALC_FAILURE  the position in the parmBuffer of the requested values was miscalculated
 u  PGSAA_E_TWOD_READ_ERROR	 function failure to read parameter values from buffer
 l  PGSAA_E_EXTRACTORESULTSERROR failure to transfer selected values from parmBuffer to results
 ul PGSAA_E_OUTOFRANGE		 input values out of data set range


EXAMPLES:
C:
     #include <PGS_AA.h>
     PGSt_SMF_status retStatus;
	
     char parms[][100] = { "etop05SeaLevelElevM" };
     long nParms = 1;
    
     PGSt_double latitude[] = {51.5, 51.23666, 50.973333} ;
     PGSt_double longitude[] = {0.1666666,0.3832,  0.5999};
                 
     PGSt_integer nPoints = 3;
    
     long fileId = 210;
 
     long version = 0;
    
     long operation = 1; 
    
     short results[3];  
  
     retStatus = PGS_AA_2Dgeo(parms, nParms, latitude, longitude, nPoints,
                  fileId, version, operation, results);
             
FORTRAN:
	IMPLICIT     NONE
	integer      pgs_aa_2dgeo
	character*99 parms(4)
        integer      nParms
        real*8       latitude(300)
        real*8       longitude(300)
        integer      fileId
        integer      nPoints
        integer      version
        integer      operation
        integer      results(300,2)
        parms(1)= "OlsonWorldEcosystems1.3a"
        nParms = 1       
        fileId = 202
        operation = 1
        version = 0
        nPoints = 300
        do 10 i = 1, 300
	.
	.
        latitude(i) = calculated_user_lat
        longitude(i) = calculated_user_lon
	.
	.
 10      continue

         call pgs_aa_2dgeo( parms, nparms,latitude, longitude, 
	          fileId, version, operation, results )
	     
NOTES:
	For further details of the background to this tools and 
        the available data sets, support files and the means by
        which new data sets can be introduced, see the Ancillary
        Data tools Primer.  The primer also includes details of
        the operations which can be set by the user and the 
        autoOperations associated with particular data sets.

	The FORTRAN result argument returned is not specified since
        it depends on the data set used; e.g. it could be real or 
	integer. 
	
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings. The parmBuffer input is a memory buffer holding 
	whatever data is extracted form the data set requested by the user.  
	The results buffer is similar although holds the final output sent 
	back to the user.  It can hold data of 4 types (long, short, float,double).

REQUIREMENTS:
        PGSTK-0931, 0840, PGSTK-0980, PGSTK-1030, PGSTK-1362

DETAILS:
	Data sets must be set up through the process control tables
        and properly referenced in the user code (fileId).  A format
        file and a support file must be present for each data set  
        being accessed.  
        The version number of dynamic files is obtained through the 
        use of the PGS_PC_GetAtttributes tool.  This tool does not 
        currently deal with versioned files.

GLOBALS:
	NONE

FILES:
	The Process control file pointed to by PGS_PC_INFO_FILE must
        be set befoore tool use.  
        The general support file containing mapping of parameters to 
        logical format and support files must be present (current suppSupport) 
        For each data file to be accessed through the tool, a format and 
        support file must exist.
        For further details, see the primer.

FUNCTIONS_CALLED:
                PGS_AA_Map
		PGS_AA_Map
		PGS_AA_FF_Setup
		PGS_AA_GeoGrid	
		PGS_SMF_SetStaticMsg 	

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_2DgeoF
( 
	     char iparms[][100], 
	     PGSt_integer nParms,
	     PGSt_double latitude[],
	     PGSt_double longitude[],
	     PGSt_integer nPoints,
	     PGSt_integer fileId,
	     PGSt_integer version,
	     PGSt_integer operation, 
	     void *results)
{
     
    char *parms[PGSd_AA_MAXNOPARMS];               /* parameter used to hold input */
    PGSt_integer logSuppFile[PGSd_AA_MAXNOPARMS];  /* support file ids */
    char *parmBuffer;                              /* buffer to hold parameter */
    char physFileName[PGSd_AA_MAXNOCHARS];         /* name of data set file */
    char  outputFormat[PGSd_AA_MAXNOCHARS];        /* Freeform format of output */  
    char outputPhysFileFormat[PGSd_AA_MAXNOCHARS]; /* Freeform format file */
    PGSt_integer totalParmMemoryCache=0;           /* size in bytes of memory for parameters requested */ 
    short toolUsed, i;                                /* specifies whether 2 or 3 D tool is to be uses */
    PGSt_integer height[1];                         /* holds height as input by user */
    PGSt_SMF_status val;

    for (i=0; i<nParms; i++)
    {
	parms[i] = (char *) malloc(100);
	strcpy(parms[i], iparms[i]);	
    }
     
    val = PGS_AA_Map(
		     parms, 
		     nParms, fileId, 
		     version, outputPhysFileFormat, 
		     physFileName, 
		     logSuppFile );
    
    
    if (val != PGS_S_SUCCESS)
    {
	for (i=0; i<nParms; i++)
        {
            if(parms[i] != NULL) free(parms[i]);
        }
	return val;
    } 
     /*   call in support data, get parm specific data first, then construct format */
    
    val = PGS_AA_GetSupp( parms, nParms, 
			 logSuppFile, outputFormat, 
			 &totalParmMemoryCache); 
    if (val != PGS_S_SUCCESS)
    {
	for (i=0; i<nParms; i++)
        {
            if(parms[i] != NULL) free(parms[i]);
        }
	return val; 
    }
    
    /* now allocate memory to the parm_buffer */    
    
    parmBuffer = (char *) malloc(totalParmMemoryCache);

    if (parmBuffer == NULL)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_2Dgeo");
	for (i=0; i<nParms; i++)
        {
            if(parms[i] != NULL) free(parms[i]);
        }
	return PGSAA_E_MALLOC;	
    }
    
    /* set up freeform buffers */
    val = PGS_AA_FF_Setup(parms, 
			  nParms, physFileName, logSuppFile, parmBuffer, 
			  outputFormat, 
			  outputPhysFileFormat);
    
    if (val != PGS_S_SUCCESS)
    {
	if(parmBuffer != NULL) free (parmBuffer);
	for (i=0; i<nParms; i++)
        {
            if(parms[i] != NULL) free(parms[i]);
        }
	return val;
    }

    /* set tool used to 2DGeo */
    toolUsed = TOOL2D;
    
    
    /* extract 2d parameters */
    val = PGS_AA_GEOGridF(parms, nParms, logSuppFile, operation, 
			 latitude, longitude, height, 
			 nPoints, parmBuffer, toolUsed, 
			 totalParmMemoryCache, results);   

    /* free up intermediate buffer containing unselected parms */    
    if(parmBuffer != NULL) free (parmBuffer);
    for (i=0; i<nParms; i++)
    {
 	if(parms[i] != NULL) free(parms[i]);
    }
    
    if (val != PGS_S_SUCCESS)
    {
	return val;
    }
    PGS_SMF_SetStaticMsg (PGS_S_SUCCESS,"PGS_AA_2Dgeo");
    return PGS_S_SUCCESS;   
}

