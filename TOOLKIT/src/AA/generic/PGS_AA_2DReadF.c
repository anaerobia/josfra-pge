/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
  	PGS_AA_2DRead.c
 
DESCRIPTION:
         This file contains the toolkit routines and support functions to give 
         generic access to gridded 2 dimensional data sets.  Freeform libary
	 functions are heavily used in certain functions for low level access.

AUTHOR: 
  	Graham J Bland / EOSL

HISTORY:
  	07-July-94 	GJB 	Initial version
  	17-August-94    GJB     Update for coding standards
	30-August-94    GJB     Update to fix coding standard review comments
	16-Feb-95       GJB     Add CFE fix and extra frees
	23-Jun-95       ANS     Removed cfortran inclusion and binding
                                Changed val to static
	11-July-95	ANS	Improved fortran example by putting IMPLICIT and function name 
				def's 
	19-July-95      ANS     Fixed ECSed00780
  
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
#include <PGS_AA_Tools.h>

/*----   mnemonics -----*/

/*----   global variables  ----*/


/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Extract data from gridded data sets by file structure; i.e. by specifying
        the start points and offsest in terms of numbers of cells from the start 
        point in the data set.
  
NAME:  
	PGS_AA_2DRead

SYNOPSIS:
C:
        #include <PGS_AA.h>
	
	PGSt_SMF_status
	PGS_AA_2DRead( 
	      char char iparms[][100], 
	      PGSt_integer nParms,
	      PGSt_integer xStart,
  	      PGSt_integer yStart,
	      PGSt_integer xDim,
 	      PGSt_integer yDim,
	      PGSt_integer fileId,
	      PGSt_integer version,
	      PGSt_integer operation, 
	      void *results)

FORTRAN:
          include 'PGS_AA_10.f'

	 integer function pgs_aa_2dread( parms, nparms, xStart, yStart, 
	 xDim, yDim, fileId, version, operation, results )
	      character*99 parms(*)
	      integer      nParms,
	      integer      xStart,
  	      integer      yStart,
	      integer      xDim,
 	      integer      yDim,
	      integer      fileId,
	      integer      version,
	      integer      operation, 
	      'user specified' results (see Notes)
	 
DESCRIPTION:
 The interface to the calling algorithm which accepts the arguments and calls 
 PGS_AA_Map, PGS_AA_GetSupp, PGS_AA_FF_Setup and PGS_AA_2DReadGrid.  
 The first 3 of these modules determine the validity of the call and 
 initialize support and load the identified data into memory.  
 PGS_AA_2DReadGrid performs the extraction requested from the 
 input arguments
 [start]
 PERFORM	PGS_AA_Map
 PERFORM	PGS_AA_GetSupp	to get support data 
 DO	allocate memory to parmBuffer using totalParmMemoryCache 
 PERFORM	PGS_AA_FF_Setup
 PERFORM	PGS_AA_2DReadGrid
 [end]

INPUTS:
        Name		Description	       Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names         see notes
                        requested
        nParms          number of parms         none    1       #defined
        xStart          the x start point       none    1       variable   
        yStart          the y start point       none    1       variable   
        xDim            the x dimension         none    1       variable   
        yDim            the y dimension         none    1       variable
        fileId          logical file number     none    variable variable
        version         version of dynamic file none    1       variable
        operation       defines user required   none    1       variable
        

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        results         results                 variable N/A    N/A
	

RETURNS:  (to user and log file - u/l)
         
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
	
	shortint results[2][200][50]
        char parm[][100] = {"OlsonWorldEcosystems1.3a"};
	longint	nParms = 1;
        longint	xStart = 4;
        longint	yStart = 7;
        longint	xDim = 20;
        longint	yDim = 50;					
	longint	fileId = 202;

	PGSt_SMF_status = PGS_AA_2DRead (parm, 			
	nParms, xStart, yStart, xDim, yDim, fileId, version, 1, 
        results);
        
      
FORTRAN:
	IMPLICIT     NONE
	integer      pgs_aa_2dread
	character*99 parms(4)
        integer      nParms
        integer      xStart
        integer      yStart
        integer      xDim
        integer      yDim
        integer      fileId
        integer      version
        integer      operation

        integer      results(20,14)
        parms(1)= "OlsonWorldEcosystems1.3a"
        nParms = 1
        yStart = 102  
        xStart = 205   
        yDim = 20
        xDim = 14  
        fileId = 202
        operation = 1
        version = 0
   
         call pgs_aa_2dread( parms, nparms, xStart, yStart, 
	 xDim, yDim, fileId, version, operation, results )
	     
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
        set of strings.  The results buffer is a memory buffer holding 
	whatever data is extracted form the data set requested by the user.  
	It can hold data of 4 types (long, short, float,double).

REQUIREMENTS:
        PGSTK-0931, 1360, 1362, 0850, 0980, 1000, 10823,


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
        be set before tool use.  
        The general support file containing mapping of parameters to 
        logical format and support files must be present (current indexFile) 
        For each data file to be accessed through the tool, a format and 
        support file must exist.
        For further details, see the primer.

FUNCTIONS_CALLED:
                PGS_AA_Map
		PGS_AA_GetSupp
		PGS_AA_FF_Setup
		PGS_AA_2DReadGrid	
		PGS_SMF_SetStaticMsg

END_PROLOG:
***************************************************************************/

PGSt_SMF_status  
PGS_AA_2DReadF( 
	      char iparms[][100], 
	      PGSt_integer nParms,
	      PGSt_integer xStart,
  	      PGSt_integer yStart,
	      PGSt_integer xDim,
 	      PGSt_integer yDim,
	      PGSt_integer fileId,
	      PGSt_integer version,
	      PGSt_integer operation, 
	      void *results)
{ 
    char *parms[PGSd_AA_MAXNOPARMS];               /* parameter used to hold input */
    PGSt_integer logSuppFile[PGSd_AA_MAXNOPARMS];  /* support file ids */
    PGSt_integer i;                                /* format file id */
    char *parmBuffer;                              /* buffer to hold parameter */
    char physFileName[PGSd_AA_MAXNOCHARS];         /* name of data set file */
    char  outputFormat[PGSd_AA_MAXNOCHARS];        /* Freeform format of output */  
    char outputPhysFileFormat[PGSd_AA_MAXNOCHARS];  /* Freeform format file */
    PGSt_integer totalParmMemoryCache=0;            /* size in bytes of memory for parameters requested */
    PGSt_SMF_status val;

    for (i=0; i<nParms; i++)
    {
	parms[i] = iparms[i];	
    }
    
    val = PGS_AA_Map (
		      parms, 
		      nParms, fileId, 
		      version, outputPhysFileFormat, 
		      physFileName, 
		      logSuppFile);
    
    if (val != PGS_S_SUCCESS) 
    { 
	return val;
    } 

    /*   call in support data, get parm specific data first, then construct format */
    
    val = PGS_AA_GetSupp( parms, nParms,
			 logSuppFile, outputFormat, 
			 &totalParmMemoryCache);

 
    if (val != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg(val, "PGS_AA_2DRead");	
	return val; 
    }
    
    /* now allocate memory to the parmBuffer */    
    
    parmBuffer = (char *) malloc(totalParmMemoryCache);

    if (parmBuffer == NULL)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_2DRead");
	return PGSAA_E_MALLOC;	
    }

    /* set up freeform buffers */
    val = PGS_AA_FF_Setup( parms, 
			  nParms, physFileName, logSuppFile, parmBuffer, 
			  outputFormat, 
			  outputPhysFileFormat);
    
    if (val != PGS_S_SUCCESS)
    {
	if(parmBuffer != NULL) free (parmBuffer);
	return val;
    }

    /* extract 2d parameters */
    val = PGS_AA_2DReadGridF(parms, 
			    nParms, xStart, yStart, 
			    xDim, yDim, parmBuffer, totalParmMemoryCache,
			    results);   
   
    if (val != PGS_S_SUCCESS)
    {
	if(parmBuffer != NULL) free (parmBuffer);
	return val;
    }
    
    /* free up intermediate buffer containing unselected parms */    
    if(parmBuffer != NULL) free (parmBuffer);
    
    /* set success and return */
    PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, "PGS_AA_2DRead");	
    return PGS_S_SUCCESS;   
}
