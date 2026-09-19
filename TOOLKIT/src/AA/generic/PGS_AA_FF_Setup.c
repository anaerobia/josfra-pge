/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	PGS_AA_FF_Setup
 
DESCRIPTION:
         This file contains PGS_AA_FF_Setup and PGS_AA_CheckFile.
	 These functions are used by all 2 and 3D tools to establish
         data in buffers using the Freeform libraries.

AUTHOR:
  	Graham J Bland / EOSL

HISTORY:
  	07-July-94 	GJB 	Initial version
  	17-August-94    GJB     Update for coding standards
	31-August-94    GJB     Post code review update
	20-February-95  GJB     Fix buffer freeing for tk4 to enhance
	                        memory management.
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

TITLE: Set up Freeform buffer and ingest data.
  	
 
NAME:
  	PGS_AA_FF_Setup

SYNOPSIS:
  	N/A

DESCRIPTION:
 Uses FF libraries to load the physical file and extract parameter(s) from it.  
 
 [start]
 DO	declare all relevant buffers and output pointers
 PERFORM	PGS_AA_CheckFile with physFileName to check for pre-existing buffer
 IF	no pre-existing buffer filled	THEN
	PERFORM	MAKE_DBIN initialize new buffer with new buffer number
	PERFORM	DB_SET to initialize the buffer for file ingestion with 	
		fileMemoryCache 
	PERFORM	DB_EVENTS to fill fileMemoryCache using the 		
		outputPhysFileFormat
 ENDIF
 PERFORM	DB_SET with outputFormat for the file buffer
 PERFORM	DB_EVENTS to convert buffer contents to outputFormat in	parmBuffer
 [end]
 
     
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        parms           parameter names        	see notes
                        requested
        nParms          number of parms         none    1       #defined
        physFileName    physical file name      see notes  
        logSuppFile     support file i.d.       none    variable
        outputPhysFileFormat data set           see notes  
                         format file
        outputFormat     format of the output   see notes
                          buffer
      
        
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       parmBuffer       buffer containing     	see notes 
                        requested parameters
       


RETURNS:
	PGSAA_E_FFDBIN		(logfile)
	PGSAA_E_FFDBSET		(logfile)
	PGSAA_E_FFDBEVENTS	(logfile)
	PGSAA_E_MALLOC		(logfile + user)
	PGSAA_E_FFERROR		(user)
        PGSTSF_E_GENERAL_FAILURE   problem in the thread-safe code

EXAMPLES:
	N/A

NOTES:
	The upper limit of the range of input variables is data set specific.
	The parms input variable is a parameter and data set specific 
        set of strings.  The results buffer is a memory buffer holding 
	whatever data is extracted form the data set requested by the user.  
	It can hold data of 4 types (long, short, float,double).
	The file names are strings which are data set specific.

REQUIREMENTS:
	N/A

DETAILS:
	See Freeform documentation for further details

GLOBALS:
        NONE

FILES:
	format and data set files (various)

FUNCTIONS_CALLED:
	PGS_AA_CheckFile
	PGS_SMF_SetStaticMsg 
	MAKE_DBIN
	DB_SET 	
	DB_EVENTS
        PGS_TSF_LockIt
        PGS_TSF_UnlockIt
        PGS_SMF_TestErrorLevel
        PGS_TSF_GetTSFMaster

END_PROLOG:
***************************************************************************/

#ifdef _PGS_THREADSAFE
    /* Create non-static variable for the thread-safe version */
    DATA_BIN_PTR *physFileBin;
#else
    static DATA_BIN_PTR physFileBin[PGSd_AA_MAXNOCACHES];  /* retains file bins */  
#endif


PGSt_SMF_status PGS_AA_FF_Setup (  
			    char *parms[], 
			    PGSt_integer nParms, 
			    char *physFileName,
			    PGSt_integer logSuppFile[], 
			    char *parmBuffer,
			    char *outputFormat, 
			    char *outputPhysFileFormat)
     
{
    unsigned outputBytes = 0L;             /* no. of bytes allocated by Freeform */   
    short binCount = 0;                    /* id of file bin being used */     
    DATA_BIN_PTR physFileBinStat;          /* intermediate bin for passing */
    char  newOutputFormat[PGSd_AA_MAXNOCHARS + PGSd_AA_MAXNOCHARS];
    PGSt_SMF_status val;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retVal;
        PGSt_SMF_status retTSF;            /* lock and unlock return */
#endif

#ifdef _PGS_THREADSAFE
    /* Set up thread-safe key keeper and TSD key */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(retVal))
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    
    physFileBin = (DATA_BIN_PTR *) pthread_getspecific( 
                       masterTSF->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN]);
#endif

    /* check to see if this file already held in memory */

    sprintf(newOutputFormat, "binary_output_data \"data format\"\n%s", outputFormat);
    val = PGS_AA_CheckFile(physFileName, &binCount);
    if (val == PGSAA_E_MALLOC )
    {
	return val;
    }

    /* if a new file is needed then open it and read it in */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEFORM) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEFORM);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    if (val == PGSAA_S_NEWFILE)
    {
	/* This program uses one multiple data bin */

	physFileBin[binCount] = NULL;

#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN],
                            physFileBin);
#endif
	
	/* name the databin with a label physFileBin by physicalFileId */

	physFileBinStat = db_make(physFileName);  
	physFileBin[binCount] = physFileBinStat; 
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN],
                            physFileBin);     
#endif
	
	if(physFileBin[binCount] == NULL)
	{
	    PGS_SMF_SetStaticMsg (PGSAA_E_FFDBIN, physFileName);
	    
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif   
	    return PGSAA_E_FFERROR;	    
	}
	
	/* read in the file and extract the parameters required 

	printf (" db_set %s bincount %d fileMemoryCache %d physFileName %s \n \n",
		physFileBin[binCount], binCount, fileMemoryCache, 
		physFileName  );*/
	
	
	val = db_set(physFileBin[binCount],
		     DBIN_BUFFER_SIZE,   DEFAULT_BUFFER_SIZE,
		     DBIN_FILE_NAME,     (char*)physFileName,
		     DBIN_FILE_HANDLE,
		     INPUT_FORMAT, (char*)outputPhysFileFormat, (void*)NULL,  
		     MAKE_NAME_TABLE,   (void*)NULL,
		     CHECK_FILE_SIZE,
		     DBIN_CACHE_SIZE,   fileMemoryCache,
		     END_ARGS);

	if  ( val != 0 )
	{
	    PGS_SMF_SetStaticMsg(PGSAA_E_FFDBSET, "db_set");
	    PGS_SMF_SetStaticMsg(PGSAA_E_FFERROR, "db_set");
	  
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif   
            return PGSAA_E_FFERROR;
	}
	
	/* having established a buffer for the file and set the output format, 
	   now fill the buffer.  
	   Use PROCESS_FORMAT_LIST to fill cache and fill headers */
	
	val = db_events(physFileBin[binCount], PROCESS_FORMAT_LIST, FFF_ALL_TYPES, END_ARGS);
	if (val != 0)
	{
	    PGS_SMF_SetStaticMsg (PGSAA_E_FFDBEVENTS, physFileName);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif   
	    return PGSAA_E_FFERROR;   
	}	
    }   
    
    /* this is the end of the jump around setting up new physFileBin 
    
    Now establish an output format for the file buffer */
    
    val = db_set(physFileBin[binCount],OUTPUT_FORMAT,(void *)NULL, newOutputFormat, END_ARGS);
    if (val != 0)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_FFDBSET, "DB_SET");

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif   
	return PGSAA_E_FFERROR;
    }   
    
    /* Convert the cache into the output_buffer */

    physFileBinStat = physFileBin[binCount];
    
   
    val = db_events(physFileBinStat,
		    DBIN_DATA_TO_NATIVE,(char*)NULL,(char*)NULL,(FORMAT_PTR)NULL,
		    DBIN_CONVERT_CACHE, parmBuffer,(void *)NULL, &outputBytes, END_ARGS);
    if (val != 0)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_FFDBEVENTS, physFileName);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif   
	return PGSAA_E_FFERROR; 
    } 
	
     /* end */
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif   
    return PGS_S_SUCCESS;
}

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE: Check for previously filled buffers
  	
 
NAME:
  	PGS_AA_CheckFile

SYNOPSIS:
  	N/A

DESCRIPTION:
 Takes physical file name and check to see if a buffer containing the file  
 contents has already been loaded into memory.  If so it return the buffer number.  
 If not then a new buffer number is allocated.  The max number of buffers is 
 tested and buffer are overwritten is the max is been reached.  PGS_A_SUCCESS is
 returned if a new file is not needed and an error valueis returned if a new file 
 must be loaded.

 [start]
 LOOP	FOR number of buffers currently held in memory
	IF	existing buffer name is physFileName	THEN
		set buffer number 
	ENDIF
 ENDLOOP
 IF	no existing buffer for the physFileName	THEN
 	IF	max number buffer reached	THEN
 		reset buffer number to overwrite existing buffers
 	ELSE
 		set buffer number to next available buffer slot
 	ENDIF
 ENDIF
 [end]
  
INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---     
        physFileName    physical file name      see notes
    
             
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        binCount        buffer i.d.             none    1       configured 

RETURNS:
	PGSAA_E_FFSETUP
        PGSTSF_E_GENERAL_FAILURE      problem in the thread-safe code

EXAMPLES:
	N/A

NOTES:
	NONE

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
        PGSg_TSF_AAnoOfIds
        PGSg_TSF_AAnoOfNewIds

FILES:
	NONE

FUNCTIONS_CALLED:
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
	PGS_SMF_TestErrorLevel()
        PGS_TSF_GetTSFMaster()
        PGS_TSF_GetMasterIndex()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status PGS_AA_CheckFile(char *physFileName, short *binCount)

{
#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

#ifdef _PGS_THREADSAFE
    /* Create non-static variables and get globals for the thread-safe version */
    char **oldFiles;
    short noOfIds;
    short noOfNewIds;
    int masterTSFIndex;
    extern short PGSg_TSF_AAnoOfIds[];
    extern short PGSg_TSF_AAnoOfNewIds[];
#else
    static char *oldFiles[PGSd_AA_MAXNOCACHES]; 
                                 /* retain copy of the file name ingested */
    static short noOfIds = 0;      /* number of files held in buffers */
    static short noOfNewIds = 0;      /* number of files held in overwritten buffers */ 
#endif

    short count = 0;                    /* i.d. of buffer to use */
    PGSt_SMF_status val;

#ifdef _PGS_THREADSAFE 
    PGSt_SMF_status retVal;
    /* Set up global index, and TSF key keeper for the thread-safe */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(retVal) || masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart and TSD key for the thread-safe */
    noOfIds = PGSg_TSF_AAnoOfIds[masterTSFIndex];
    noOfNewIds = PGSg_TSF_AAnoOfNewIds[masterTSFIndex];
    oldFiles = (char **) pthread_getspecific(
                           masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES]);
    physFileBin = (DATA_BIN_PTR *) pthread_getspecific(
                       masterTSF->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN]);
#endif

    val = PGSAA_S_NEWFILE;                /* set value to a new file setting */
    

/* loop through existing files and check to see if already in memory */

for ( count = 0; count < noOfIds; count++)
{
    /*check if file already used */

    if (strcmp(oldFiles[count], physFileName)==0)
    {
	*binCount = count;
	val = PGS_S_SUCCESS;
    }
}

if ( val == PGSAA_S_NEWFILE )
{
    /* file not found, check that max number not exceeded  */
    
    if ( count == PGSd_AA_MAXNOCACHES )
    {
	/* all caches filled, start reusing caches from no. 0 */
	if (noOfNewIds == noOfIds )
	{
	    noOfNewIds = 0;

#ifdef _PGS_THREADSAFE
            /* Reset global */
            PGSg_TSF_AAnoOfNewIds[masterTSFIndex] = noOfNewIds;
#endif  
	}
	/* set i.d. to a new number and copy file name */

	*binCount = noOfNewIds;

	/* now free up everything from the old buffer by direct freeing
	   of buffer content */

	if(oldFiles[*binCount] != NULL)
	{
		free (oldFiles[*binCount]);
		oldFiles[*binCount] = NULL;
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES],oldFiles);
#endif   
	}
/*
	if(physFileBin[*binCount]->buffer != NULL)
	{
		free(physFileBin[*binCount]->buffer);
		physFileBin[*binCount]->buffer
	
	if(physFileBin[*binCount]->cache != NULL) free(physFileBin[*binCount]->cache);
*/
	if(physFileBin[*binCount] != NULL)
	{
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEFORM) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEFORM);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
		db_free(physFileBin[*binCount]);
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif          
		physFileBin[*binCount] = NULL;

#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAPHYSFILEBIN],
                                     physFileBin);
#endif
	}

	oldFiles[*binCount] = (char *)malloc(PGSd_AA_MAXNOCHARS);

#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES],oldFiles);
#endif

	/* check malloc */
	if (oldFiles[*binCount] == NULL)
	{    
	    PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_CheckFile");
	    return PGSAA_E_MALLOC;	
	}
	
	strcpy(oldFiles[*binCount], physFileName);
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES],oldFiles);
#endif
	noOfNewIds++;    
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_AAnoOfNewIds[masterTSFIndex] = noOfNewIds;
#endif
    }
    else
    {
	/* remember the new file and set the binCount */

	oldFiles[count] = (char *)malloc(PGSd_AA_MAXNOCHARS);
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES],oldFiles);
#endif   
	/* check malloc */
	if (oldFiles[count] == NULL)
	{    
	    PGS_SMF_SetStaticMsg (PGSAA_E_MALLOC,"PGS_AA_CheckFile");
	    return PGSAA_E_MALLOC;	
	}	

	strcpy(oldFiles[count], physFileName);
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYAAOLDFILES],oldFiles);
#endif
	*binCount = count;
	noOfIds++; 
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_AAnoOfIds[masterTSFIndex] = noOfIds;
#endif

    }   
}
/* end and return success or not of getting i.d. */

return val;
}
