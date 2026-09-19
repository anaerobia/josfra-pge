/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	PGS_AA_Map.c
 
DESCRIPTION:
         The file contains PGS_AA_Map, PGS_AA_Index, PGS_AA_PhysFile,
         PGS_AA_GetSupp and GetGeoSupp.  These functions are used 
         by each 2 and 3D tool for perform set up functions for each call
         and obtain specific support data.

AUTHOR:
  	Graham J Bland / EOSL

HISTORY:
  	07-July-94 	GJB 	Initial version
  	17-August-94    GJB     Update for coding standards
	31-August-94    GJB     Post review updates
	03-October-94   GJB     Remove printfs
	23-Jun-95       ANS     Changed val to static
        06-July-99      SZ      Updated for the thread-safe functionality
 
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h> 

#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_Tools.h>
#include <PGS_TSF.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  
       Map arguments
  	
 
NAME:
  	PGS_AA_Map

SYNOPSIS:
  	N/A

DESCRIPTION:
 Maps from parameter to the logical i.ds from format and support files using the 
 indexFile file.  Uses the PC tools to obtain the physical files names of the 
 format file (in lower level modules) and the data set itself (logical i.d. 
 supplied by the user). 

 [start]
 DO	 check nParms is not out of limits (1 and 4) and return error if so
 PERFORM PGS_AA_Index map into indexFile file
 PERFORM PGS_AA_PhysFile get physical file names
 [end]

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names         see notes
                        requested
        nParms          number of parms         none    1       #defined
        fileId          logical file number     none    variable variable
        version         version of dynamic file none    1       variable
        
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
     outputPhysFileFormat data set   	        none    N/A	N/A   
                         format file
        physFileName     physical file name     none	N/A	N/A 
        logSuppFile      support file i.d.      integer variable

RETURNS:
	PGSAA_E_INVALIDNOPARMS	(user + log)

EXAMPLES:
	N/A

NOTES:
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings. 

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_AA_Index
	PGS_AA_PhysFile
	PGS_SMF_SetStaticMsg 

END_PROLOG:
***************************************************************************/


/* receives the information from the tool as input */

PGSt_SMF_status
PGS_AA_Map(
		  char *parms[], 
		  PGSt_integer nParms,
		  PGSt_integer fileId,
		  PGSt_integer version,
		  char *outputPhysFileFormat, 
		  char *physFileName,
		  PGSt_integer logSuppFile[])

/* first thing is to perform check_call - get support file related to parms */
     
{
    short i;                  /* counter for nParms */
    PGSt_integer formatId;    /* i.d. of format file */
    PGSt_SMF_status val;

    /* first check no of parms is valid */

    if (nParms > PGSd_AA_MAXNOPARMS || nParms < 1 )      
    {
	PGS_SMF_SetStaticMsg 
	  (PGSAA_E_INVALIDNOPARMS, "PGS_CHECK_ARGS");
	return PGSAA_E_INVALIDNOPARMS;
    }

    val = PGS_AA_Index(parms, nParms, logSuppFile, &formatId);
    
    if (val != PGS_S_SUCCESS)
      
    {
	return(val);
    }
    
    /* check the arguments, and find the physical filename for static and dynamic 
     data sets */    
    
    for (i = 0; i < nParms; i++)
    {
	val = PGS_AA_PhysFile(parms, nParms, logSuppFile, formatId,
			      fileId, version, outputPhysFileFormat, physFileName);
	
	if (val != PGS_S_SUCCESS)
	  
	{
	    return(val);
	}
    }
    return PGS_S_SUCCESS;
    /* end */   
}



/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  Interogate index  file
  	
 
NAME:
  	PGS_AA_Index

SYNOPSIS:
  	N/A

DESCRIPTION:
 Open index file containing support /format file mapping 
 and extract data for requested parameters
 
 [start]
 PERFORM	PGS_IO_Gen_Open	for indexFile
 LOOP	FOR nParms
 	DO	input no of parameters from indexFile
 	LOOP	FOR no of parameters
 		DO	input parameter name, logical i.ds for format & support files
 		IF	parm matches parameter name	THEN
 			DO 	hold the value of logical i.ds and parameter
 		ENDIF
 	ENDLOOP
 	IF	more than 1 parameter being searched	THEN
 		DO	check formatId of found parameters is the same & return error if not
 	ENDIF
 	DO	check number of found parameters is nParms; return error if not
 ENDLOOP
 PERFORM	PGS_IO_Gen_CLOSE
 [end]
 	
     
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names        	see notes
                        requested
        nParms          number of parms         none     1      #defined
       
        
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        logSuppFile     support file i.d.       none    variable
        formatId        format file i.d.        none    variable 


RETURNS:
	PGSAA_E_CANT_PARSE_FILE	        (user + log)
	PGSAA_E_BADSUPPORTFILE	        (user + log)
	PGSAA_E_PARMSFROMANYFILES	(user + log)
	PGSAA_E_SUPPORTFILE		(user)
	PGSAA_E_PARMSNOTFOUND	        (log)

EXAMPLES:
	N/A

NOTES:
       The upper limit of the range of input variables is data set specific.
       The parms input variable is a parameter and data set specific 
       set of strings. 

RETURNS:

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	indexFile

FUNCTIONS_CALLED:
	
        PGS_IO_Gen_Open
	PGS_IO_Gen_CLOSE

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_Index(
		  char *parms[], 
		  PGSt_integer nParms,
		  PGSt_integer logSuppFile[PGSd_AA_MAXNOPARMS],
		  PGSt_integer *formatId)
     
{
    char parm[PGSd_AA_MAXNOCHARS];    /* holds parm from index file */
    PGSt_integer logId;               /* holds support file i.d. from indexFile */
    PGSt_integer oldFormatId;         /* holds format file i.d. */
    PGSt_integer id;                  /* holds format file i.d. from indexFile */
    PGSt_integer noSuppFiles;         /* no of support files in the indexFile file */
    short i, ii;                      /* loop counters */
    PGSt_IO_Gen_FileHandle *indexFileFile; /* pointer for file returned from IO tool */
    short noSuppFilesFound = 0;       /* number of parms/files found in indexFile */
    PGSt_integer version = 1;
    PGSt_SMF_status val;
    
    /* get support file containing support file logical to parm mapping */
    
   val = PGS_IO_Gen_Open(PGSd_AA_SUPPLIST,
			  PGSd_IO_Gen_Read,
			  &indexFileFile,
			  version); 

    if (val != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANT_PARSE_FILE, 
				       "PGS_IO_Gen_Open");
	PGS_SMF_SetStaticMsg (PGSAA_E_SUPPORTFILE, 
				       "PGS_IO_Gen_Open");
	return PGSAA_E_SUPPORTFILE;    
    }  
    
    /* search through to get the right support file logical i.d. */
    
    for ( ii=0; ii < nParms; ii++)
    {
	fscanf( indexFileFile, "%d", &noSuppFiles);     

	for (i = 0; i < noSuppFiles; i++)
	{    
	    val = fscanf(indexFileFile, "%s %d %d", parm, &logId, &id );	        
	    if (val != 3)
	    {
		PGS_SMF_SetStaticMsg (PGSAA_E_BADSUPPSUPPORT, 
					       "PGS_AA_Index");
		return PGSAA_E_BADSUPPSUPPORT;
	    }
	    val = strcmp ( parms[ii], parm);
	    
	    /* if a match is found then hold the value and rewind */
	    if ( val == 0)
	    { 
		logSuppFile[ii] = logId;
		*formatId = id;
		noSuppFilesFound++;	

		break;
	    } 
	}
	if ( ii > 0 )  /* test for same physical file */
	{
	    if (oldFormatId != *formatId)
	    {
		PGS_SMF_SetStaticMsg 
		  (PGSAA_E_PARMSFROMANYFILES, "PGS_AA_Index");
		PGS_SMF_SetStaticMsg 
		  (PGSAA_E_SUPPORTFILE, "PGS_AA_Index");
		return PGSAA_E_PARMSFROMANYFILES;
	    }
	} 
	oldFormatId  = *formatId; 
	rewind(indexFileFile);	
	
    }
    if ( noSuppFilesFound != nParms )
    {
	PGS_SMF_SetStaticMsg(PGSAA_E_PARMSNOTFOUND, 
				      "PGS_AA_Index");
	PGS_SMF_SetStaticMsg( PGSAA_E_SUPPORTFILE, 
				      "PGS_AA_Index");
	return PGSAA_E_SUPPORTFILE;
    }

    val = PGS_IO_Gen_Close(indexFileFile); 

    return (PGS_S_SUCCESS);
}

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Obtain physical file names of the format and data set
  	
 
NAME:
  	PGS_AA_PhysFile

SYNOPSIS:
  	N/A

DESCRIPTION:
 Obtain the physical file names of the format and data set files.
 
 [start]
 PERFORM	PGS_PC_GetReference with formatId	
 PERFORM	PGS_AA_PeV_string with <support> for dataType
 IF	dataType = dynamic	THEN
 	PERFORM	PGS_PC_GetReference with fileId and version number
 ELSE	dataType = static	THEN
 	PERFORM	PGS_PC_GetReference with fileId
 ENDIF
 [end]
      
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names         see notes
                        requested
        nParms          number of parms         none    1       #defined
        logSuppFile     support file i.d.       none    variable
        formatId        format file i.d.        none    variable 
        fileId          logical file number     none    variable variable
        version         version of dynamic file none    1       variable
      
        
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       outputPhysFileFormat data set   	        N/A	N/A	N/A  
                         format file
        physFileName     physical file name     N/A	N/A	N/A 
    


RETURNS:
	PGSAA_E_DATARATEUNSET	        (log)
 	PGSAA_E_SUPPORTFILE		(user)
 	PGSAA_E_CANTFINDFILE		(user + log)

EXAMPLES:
	N/A

NOTES:
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings.

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	format and data set files (various)

FUNCTIONS_CALLED:
	
        PGS_PC_GetReference 
 	PGS_AA_PeV_string
	PGS_SMF_SetStaticMsg

END_PROLOG:
***************************************************************************/

PGSt_SMF_status

PGS_AA_PhysFile( 
		     char *parms[], 
		     PGSt_integer nParms,
		     PGSt_integer logSuppFile[], 
		     PGSt_integer formatId,
		     PGSt_integer fileId,
		     PGSt_integer version,
		     char *outputPhysFileFormat, 
		     char *physFileName) 
     
{    
    PGSt_integer v = 1;     /* dummy value */
    PGSt_SMF_status val;    

    /* get real physical name for physFileFormat logical ID */
    
    val = PGS_PC_GetPCSData(
			    PGSd_PC_SUPPORT_IN_NAME,
			    formatId, 
			    outputPhysFileFormat,
			    &v);
    if (val!= PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANTFINDFILE, "PGS_PC_GetPCSData");
	return PGSAA_E_CANTFINDFILE;    
    } 

    /* now get file name of data set file - both static and dynamic use the same 
       call */

    val = PGS_PC_GetPCSData(
			    PGSd_PC_INPUT_FILE_NAME,
			    fileId, 
			    physFileName,
			    &version);
    if (val != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANTFINDFILE, "PGS_PC_GetPCSData");
	return PGSAA_E_CANTFINDFILE;    
    }
  
    return PGS_S_SUCCESS;        	   
}

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Construct output format ans get support data and place in global.
  	
 
NAME:
  	PGS_AA_GetSupp

SYNOPSIS:
  	N/A

DESCRIPTION:
 Get support data and output format.
 [start]
 PERFORM PGS_AA_PeV_integer with <support> to get cacheFormat, 
     cacheFormatBytes, parmMemoryCache, fileMemoryCache,  
     xCells, yCells, zCells, dataType
 DO	sum totalParmMemoryCache from individual parmMemoryCache
 DO	construct an outputFormat containing requested parms using parms cacheFormat, 
      cacheFormatBytes.  
 [end]

   

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        parms           parameter names        	see notes
                        requested
        nParms          number of parms         none    1      #defined
    
        logSuppFile     support file i.d.       none variable
          
             
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        outputFormat    format of the output    none    N/A     N/A
                          buffer
   totalParmMemoryCache bytes required for      none    1       variable
                        parameters
   

RETURNS:
	PGSAA_E_GETSUPP 
        PGSTSF_E_GENERAL_FAILURE      problem in the thread-safe code

EXAMPLES:
	N/A

NOTES:
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings. 

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:  as in PGS_AA_Global.h. 


FILES:
	supportFiles (various)

FUNCTIONS_CALLED:
	PGS_AA_PeV_integer 
	PGS_AA_PeV_string 
	PGS_SMF_SetStaticMsg 	
        PGS_TSF_LockIt
        PGS_TSF_UnlockIt
        PGS_SMF_TestErrorLevel

END_PROLOG:
***************************************************************************/
     
PGSt_SMF_status PGS_AA_GetSupp( char *parms[], 
			       PGSt_integer nParms,
			       PGSt_integer logSuppFile[PGSd_AA_MAXNOPARMS], 
			       char *outputFormat, PGSt_integer *totalParmMemoryCache)

{  
    short parmCount;       /* count for each parameters requested */
    PGSt_SMF_status val;  

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;      /* lock and unlock return */
#endif

#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    /* loop round to get parm specific support data */
    
    for ( parmCount=0; parmCount < nParms; parmCount++)
    {
	val = PGS_AA_PeV_string(logSuppFile[parmCount], "cacheFormat1",
				cacheFormat1[parmCount]);
	
	if (val != PGS_S_SUCCESS)
	{
	    
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP;
	}
	
	val = PGS_AA_PeV_integer(logSuppFile[parmCount], "cacheFormat2",
				&cacheFormat2[parmCount]);
	if (val != PGS_S_SUCCESS) 
	{
	    
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP;
	}
	
	val = PGS_AA_PeV_integer(logSuppFile[parmCount], "cacheFormatBytes",
				&cacheFormatBytes[parmCount]);
	if (val != PGS_S_SUCCESS )  
	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     
	
	return PGSAA_E_GETSUPP;
	}	
	val = PGS_AA_PeV_integer(logSuppFile[parmCount], "parmMemoryCache",
				 &parmMemoryCache[parmCount]); 
	if (val != PGS_S_SUCCESS )  
	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP; 
	}
	*totalParmMemoryCache = *totalParmMemoryCache + parmMemoryCache[parmCount];
	  
    }
    /* get logical file data relating to each parm */

    val = PGS_AA_PeV_integer(logSuppFile[0], "xCells",
			     &xCells);
    if (val != PGS_S_SUCCESS ) 
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP;
    } 
    val = PGS_AA_PeV_integer(logSuppFile[0], "yCells",
			     &yCells);
    if (val != PGS_S_SUCCESS )  
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP;
    }
    val = PGS_AA_PeV_integer(logSuppFile[0], "zCells",
			     &zCells);
    if (val != PGS_S_SUCCESS ) 
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP; 
    }
    val = PGS_AA_PeV_integer(logSuppFile[0], "fileMemoryCache",
			     &fileMemoryCache);
    if (val != PGS_S_SUCCESS )  
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP;
     }
   val = PGS_AA_PeV_string(logSuppFile[0], "dataType",
			     dataType[0]);
    if (val != PGS_S_SUCCESS )  
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

	return PGSAA_E_GETSUPP;
    }
    /* construct outputFormat for the parameter extraction  */
    
    switch (nParms)
    {
      case 1:
	sprintf(outputFormat, "%s 1 %d %s %d\n",   
		parms[0], cacheFormatBytes[0], cacheFormat1[0], cacheFormat2[0]); 
	break;
	
      case 2:
	
	sprintf(outputFormat, "%s 1 %d %s %d\n%s %d %d %s %d\n", 
		parms[0], cacheFormatBytes[0], cacheFormat1[0], cacheFormat2[0], 
		parms[1], cacheFormatBytes[0]+1,(cacheFormatBytes[0] + cacheFormatBytes[1]), 
		cacheFormat1[1], cacheFormat2[1]);
	break;
	
      case 3:
	
	sprintf(outputFormat, "%s 1 %d %s %d\n%s %d %d %s %d\n%s %d %d %s %d\n", 
		parms[0], cacheFormatBytes[0], cacheFormat1[0], cacheFormat2[0], 
		parms[1], (cacheFormatBytes[0] + 1),(cacheFormatBytes[0] + cacheFormatBytes[1]),
		cacheFormat1[1], cacheFormat2[1], 
		parms[2], (cacheFormatBytes[0] + cacheFormatBytes[1] + 1),
		(cacheFormatBytes[0] + cacheFormatBytes[1] + cacheFormatBytes[2]), 
		cacheFormat1[2], cacheFormat2[2]);
	break;
	
      case 4:
	sprintf(outputFormat, "%s 1 %d %s %d\n%s %d %d %s %d\n%s %d %d %s %d\n%s %d %d %s %d\n", 
		parms[0], cacheFormatBytes[0], cacheFormat1[0], cacheFormat2[0], 
		parms[1], (cacheFormatBytes[0] + 1),(cacheFormatBytes[0] + cacheFormatBytes[1]),
		cacheFormat1[1], cacheFormat2[1], 
		parms[2], (cacheFormatBytes[0] + cacheFormatBytes[1] + 1),
		(cacheFormatBytes[0] + cacheFormatBytes[1] + cacheFormatBytes[2]), 
		cacheFormat1[2], cacheFormat2[2],
		parms[3], (cacheFormatBytes[0] + cacheFormatBytes[1] + cacheFormatBytes[2] + 1),
		(cacheFormatBytes[0] + cacheFormatBytes[1] + cacheFormatBytes[2] + cacheFormatBytes[3]), 
		cacheFormat1[3], cacheFormat2[3]);
	break;
    }
    /* end */

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif     

   return PGS_S_SUCCESS;
}

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Get support data for coordinate operations
  	
 
NAME:
  	PGS_AA_GetSuppGeo

SYNOPSIS:
  	N/A

DESCRIPTION:
 Get from the support file those data necessary to calculate geographic 
 access.  Also get data specific to certain autoOperations.  

 Output: maxLat, minLat, maxLong, minLong, autoOperation, (all in global)
 	for autoOperation AOP_W3FB06, lowerLeftLat, lowerLeftLong, meshLength, 
 gridOrientation.

 [start]
 PERFORM PGS_AA_PeV_xxxx to obtain Output values from logSuppFile
 IF	autoOperation includes polar stereographic transform	THEN
 	PERFORM	PGS_AA_PeV_xxxx to obtain autoOperation specific Output 
           values from logSuppFile
 ENDIF
 [end]
 
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        logSuppFile     support file i.d.       integer variable
                            
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
   

RETURNS:
	PGSAA_E_GETSUPP		            (user)
 	plus errors from PGS_AA_PeV where detected  (log)
        PGSTSF_E_GENERAL_FAILURE     problem in the thread-safe code

EXAMPLES:
	N/A

NOTES:
 The output from this function is in the form of support data loaded 
 from an external disc file into the global area in PGS_AA_Tools.h

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	as in PGS_AA_Global.h

FILES:
	NONE

FUNCTIONS_CALLED:
        PGS_AA_PeV_real
 	PGS_AA_PeV_integer
        PGS_TSF_LockIt
        PGS_TSF_UnlockIt
        PGS_SMF_TestErrorLevel

END_PROLOG:
***************************************************************************/
      
PGSt_SMF_status   PGS_AA_GetSuppGeo(PGSt_integer logSuppFile[])
     
{
    PGSt_SMF_status val;    

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;      /* lock and unlock return */
#endif

    /* get data from support file using PeV tool */
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_AALOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    val = PGS_AA_PeV_real(logSuppFile[0], "maxLat",
			  &maxLat);
    if (val != PGS_S_SUCCESS ) 
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	return PGSAA_E_GETSUPP;
    }
    val = PGS_AA_PeV_real(logSuppFile[0], "minLat",
			  &minLat);
    if (val != PGS_S_SUCCESS )  
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	return PGSAA_E_GETSUPP;
    }
    val = PGS_AA_PeV_real(logSuppFile[0], "maxLong",
			  &maxLong);
    if (val != PGS_S_SUCCESS ) 
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	return PGSAA_E_GETSUPP;
    }
    val = PGS_AA_PeV_real(logSuppFile[0], "minLong",
			  &minLong);
    if (val != PGS_S_SUCCESS )  
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	return PGSAA_E_GETSUPP;
    }
    val = PGS_AA_PeV_integer(logSuppFile[0], "autoOperation",
			     &autoOperation);
    if (val != PGS_S_SUCCESS )  
    {
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

	return PGSAA_E_GETSUPP;
    }
    /*  get support data for specific  autoOperations  */
    
    if ((autoOperation & PGSd_AA_AOP_POLARSTEREO) == PGSd_AA_AOP_POLARSTEREO)
    {
	/* get additional support data for polar stereographic conversion */
	
	val = PGS_AA_PeV_real(logSuppFile[0], "lowerLeftLat",
			      &lowerLeftLat);
	if (val != PGS_S_SUCCESS ) 
	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GETSUPP;
	}
	val = PGS_AA_PeV_real(logSuppFile[0], "lowerLeftLong",
			      &lowerLeftLong);
	if (val != PGS_S_SUCCESS ) 
	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GETSUPP;
	}
	val = PGS_AA_PeV_real(logSuppFile[0], "meshLength",
			      &meshLength);
	if (val != PGS_S_SUCCESS ) 
	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GETSUPP;
	}
	val = PGS_AA_PeV_real(logSuppFile[0], "gridOrientation",
			      &gridOrientation);
	if (val != PGS_S_SUCCESS ) 
	{
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif

		return PGSAA_E_GETSUPP;	
 	}
    }
    
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_AALOCK);
#endif


	return PGS_S_SUCCESS;
    /* end */
}
