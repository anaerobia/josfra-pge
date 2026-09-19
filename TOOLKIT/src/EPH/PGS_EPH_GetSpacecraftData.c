/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
   PGS_EPH_GetSpacecraftData.c

DESCRIPTION:
   This file contains the function PGS_EPH_GetSpacecraftData().
   This function returns a structure containing a spacecraft tag and the
   corresponding name and Euler angle order for the spacecraft.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Peter D. Noerdlinger / SM&A Inc.
   Abe Taaheri / SM&A Inc.
  
HISTORY:
   30-Jan-1997 GTSK  Initial version
   28-Feb-1999 PDN   Added CHEM spacecraft
   09-July-1999  SZ  Updated for the thread-safe functionality  
   20-Dec-1999  AT   Modified PM spcaecraft tags to agree with other TOOLKIT
                     tools (such as L0, and TD) to distinguish GIIS from GIRD,
		     although both refer to the same PM spacecraft.
   06-Sep-2000  AT   Changed spacecraft names from EOSPMGIIS and EOSPMGIRD to
                     EOSPM1
   11-Apr-2001  AT   Modified CHEM to AURA

END_FILE_PROLOG
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Spacecraft Tag Information

NAME:
   PGS_EPH_GetSpacecraftData()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_GetSpacecraftData(
       PGSt_tag          scTag, 
       char*             scName,   
       PGSt_searchTag    searchType,
       PGSt_scTagInfo*   scData)   

FORTRAN:		  
   N/A

DESCRIPTION:
   This function returns a structure containing a spacecraft tag and the
   corresponding name and Euler angle order for the spacecraft.

INPUTS:
   Name         Description
   ----         -----------
   scTag        spacecraft tag (e.g. PGSd_TRMM, PGSd_EOS_AM1)

   scName       spacecraft name string (e.g. "TRMM", "EOSAM1")

   searchType   flag describing what type of search to do on the spacecraft
                tag data base (must be either PGSe_TAG_SEARCH to search by
		tag value (scTag above) or PGSe_NAME_SEARCH to search by name
		value (scName above))

OUTPUTS:
   Name         Description
   ----         -----------
   scData       a structure containing the spacecraft tag, spacecraft name and
                Euler angle order for the spacecraft
          
RETURNS:
   PGS_S_SUCCESS                 successful return 
   PGSTD_E_SC_TAG_UNKNOWN        unable to find spacecraft tag or name in
                                 database file (sc_tags.dat)
   PGS_E_UNIX                    a UNIX system error occured
   PGS_E_TOOLKIT                 an unexpected tookit error occured
   PGSTSF_E_GENERAL_FAILURE      problem in the thread-safe code   

EXAMPLES:
C:

FORTRAN:
   N/A

NOTES:
   This function searches the spacecraft tags database file for a match.  It can
   search on either an input tag or an input name (not both).  The search type is
   determined by the value of the input variable "searchType" (see above).  If
   searchType is PGSe_TAG_SEARCH, the search is done on the tags field of the
   spacecraft tags database using the value of the input variable "scTag".  In
   this case the value of the input variable "scName" is ignored.  If searchType
   is PGSe_NAME_SEARCH, the search is done on the names field of the spacecraft
   tags database using the value of the input variable "scName".  In this case
   the value of the input variable "scTag" is ignored.


REQUIREMENTS:
   PGSTK - ????, ????

DETAILS:
   None

GLOBALS:
   PGSg_TSF_EPHtableread
   PGSg_TSF_EPHnumArrayElements

FILES:
   None

FUNCTIONS_CALLED:
   PGS_IO_Gen_Open()
   PGS_EPH_GetToken()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_SetUnknownMsg()
   PGS_SMF_TestErrorLevel()
   PGS_TSF_GetTSFMaster()
   PGS_TSF_GetMasterIndex()

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <PGS_IO.h>
#include <PGS_EPH.h>
#include <PGS_TD.h>
#include <PGS_TSF.h>

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_GetSpacecraftData()"

PGSt_SMF_status
PGS_EPH_GetSpacecraftData(         /* look up s/c data by s/c tag or s/c name */
    PGSt_tag          scTag,       /* input s/c tag */
    char*             scName,      /* input s/c name */
    PGSt_searchTag    searchType,  /* search type (name or tag based look up) */
    PGSt_scTagInfo*   scData)      /* s/c data structure (output) */
{
    char*             charPtr;     /* character pointer used to point to string
				      "tokens" */
    char              buffer[104]; /* character buffer used to hold a line read
				      in from the disk file */
    size_t            numCheck;    /* used to check the return value of calls to
				      sscanf() */
    FILE*             filePtr;     /* pointer to opened s/c data file stream */

    int               index;       /* looping index */

    PGSt_SMF_status   returnStatus;/* return status of TK function calls */

    /* declaration and initialization of static variables */
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status  retVal;
    /* Create non-static variables and get globals for the thread-safe version */
    PGSt_boolean    tableread;
    PGSt_scTagInfo* scInfoArray;
    size_t          numArrayElements;
    int masterTSFIndex;
    extern PGSt_boolean PGSg_TSF_EPHtableread[];
    extern size_t PGSg_TSF_EPHnumArrayElements[];

    /* Set up global index, and TSF key keeper for the thread-safe */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(retVal) || masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart and TSD key for the thread-safe */
    tableread = PGSg_TSF_EPHtableread[masterTSFIndex];
    scInfoArray = (PGSt_scTagInfo *) pthread_getspecific(
                           masterTSF->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY]);
    numArrayElements = PGSg_TSF_EPHnumArrayElements[masterTSFIndex];
#else
    static PGSt_boolean    tableread=PGS_FALSE; /* true if the s/c data file has
						   already been read from disk
						   into memory, otherwise
						   false */
    static PGSt_scTagInfo* scInfoArray=NULL;    /* pointer to array of s/c data
						   structures (actual space for
						   array is allocated
						   dynamically in this
						   program) */
    static size_t          numArrayElements=0U; /* number of elements in the s/c
						   data array (see scInfoArray
						   above) */
#endif

    /***************************
     * BEGIN program execution *
     ***************************/

    /* If the disk file containing the spacecraft (s/c) tag information has
       already been read into memory then search the array of s/c tag
       information structures for a match (based on the input search criteria)
       */

    if(tableread == PGS_TRUE)
    {
	switch (searchType)
	{
	  case PGSe_TAG_SEARCH:

	    /* search based on scTag, search the array of s/c tag information
	       structures for a structure whose s/c tag member matches the input
	       s/c tag (scTag) */

	    for (index=0;index<numArrayElements;index++)
	    {
		/* if a matching tag is found, copy the appropriate s/c tag
		   information structure to be returned, otherwise continue
		   searching */

		if (scInfoArray[index].spacecraftTag == scTag)
		{
		    memcpy((void*) scData,
			   (void*) (scInfoArray + index),
			   sizeof(PGSt_scTagInfo));
		    return PGS_S_SUCCESS;
		}
	    }

	    /* no match was found for the input s/c tag */

	    PGS_SMF_SetStaticMsg(PGSTD_E_SC_TAG_UNKNOWN, FUNCTION_NAME);
	    return PGSTD_E_SC_TAG_UNKNOWN;

	  case PGSe_NAME_SEARCH:

	    /* search based on scName, search the array of s/c tag information
	       structures for a structure whose s/c name member matches the
	       input s/c name (scName) */

	    for (index=0;index<numArrayElements;index++)
	    {
		/* if a matching name is found, copy the appropriate s/c tag
		   information structure to be returned, otherwise continue
		   searching */

		if ( strcmp(scInfoArray[index].spacecraftName, scName) == 0 )
		{
		    memcpy((void*) scData,
			   (void*) (scInfoArray + index),
			   sizeof(PGSt_scTagInfo));
		    return PGS_S_SUCCESS;
		}
	    }

	    /* no match was found for the input s/c name */

	    PGS_SMF_SetDynamicMsg(PGSTD_E_SC_TAG_UNKNOWN,
				  "spacecraft name is unknown or not currently "
				  "supported",
				  FUNCTION_NAME);
	    return PGSTD_E_SC_TAG_UNKNOWN;

	  default:

	    /* invalid search criteria specified (invalid value of input
	       variable searchType) */

	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "unknown search type specified (neither "
				  "PGSe_TAG_SEARCH nor PGSe_NAME_SEARCH)",
				  FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
    }

    /* Open the spacecraft tag information file. */

    returnStatus = PGS_IO_Gen_Open(PGSd_SC_TAG_INFO_FILE, PGSd_IO_Gen_Read,
				   &filePtr, 1);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
	tableread = PGS_TRUE;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_EPHtableread[masterTSFIndex] = tableread;
#endif
	break;
	
      case PGSIO_E_GEN_FILE_NOEXIST:

#ifndef PGS_DAAC /* don't allow a default at the DAAC */

	/* no logical ID for the spacecraft tags file was found, in this case
	   use default values (to maintain backward compatibility for users who
	   don't upgrade their PCF (i.e. pretty much everybody)) */

        numArrayElements = 6;
	scInfoArray = (PGSt_scTagInfo*) calloc(numArrayElements,
					       sizeof(PGSt_scTagInfo));
#ifdef _PGS_THREADSAFE
        /* Reset global and TSD key */
        PGSg_TSF_EPHnumArrayElements[masterTSFIndex] = numArrayElements;
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY],
                                  scInfoArray);
#endif
	if (scInfoArray == NULL)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_UNIX,
				  "error attempting to allocate dynamic memory",
				  FUNCTION_NAME);
            PGS_IO_Gen_Close(filePtr);
	    return PGS_E_UNIX;
	}

	/* set defaults for TRMM */

	scInfoArray[0].spacecraftTag = PGSd_TRMM;
	strcpy(scInfoArray[0].spacecraftName, "TRMM");
	scInfoArray[0].eulerAngleOrder[0] = 3;
	scInfoArray[0].eulerAngleOrder[1] = 2;
	scInfoArray[0].eulerAngleOrder[2] = 1;

	/* set defaults for EOS-AM-1 */

	scInfoArray[1].spacecraftTag = PGSd_EOS_AM1;
	strcpy(scInfoArray[1].spacecraftName, "EOSAM1");
	scInfoArray[1].eulerAngleOrder[0] = 3;
	scInfoArray[1].eulerAngleOrder[1] = 1;
	scInfoArray[1].eulerAngleOrder[2] = 2;

	/* set defaults for EOS-PM-1 */
	/* Although for PM two new tags were defined to distinguish GIIS from
	   GIRD, still we kept PGSd_EOS_PM1 in case if some PGEs using it */ 

	scInfoArray[2].spacecraftTag = PGSd_EOS_PM1;
	strcpy(scInfoArray[2].spacecraftName, "EOSPM1");
	scInfoArray[2].eulerAngleOrder[0] = 3;
	scInfoArray[2].eulerAngleOrder[1] = 1;
	scInfoArray[2].eulerAngleOrder[2] = 2;

       /* set defaults for EOS-AURA */

        scInfoArray[3].spacecraftTag = PGSd_EOS_AURA;
        strcpy(scInfoArray[3].spacecraftName, "EOSAURA");
        scInfoArray[3].eulerAngleOrder[0] = 3;
        scInfoArray[3].eulerAngleOrder[1] = 1;
        scInfoArray[3].eulerAngleOrder[2] = 2;

	/* set defaults for EOS-PM-1, GIIS */

	scInfoArray[4].spacecraftTag = PGSd_EOS_PM_GIIS;
	strcpy(scInfoArray[4].spacecraftName, "EOSPM1");
	scInfoArray[4].eulerAngleOrder[0] = 3;
	scInfoArray[4].eulerAngleOrder[1] = 1;
	scInfoArray[4].eulerAngleOrder[2] = 2;

	/* set defaults for EOS-PM-1, GIRD */

	scInfoArray[5].spacecraftTag = PGSd_EOS_PM_GIRD;
	strcpy(scInfoArray[5].spacecraftName, "EOSPM1");
	scInfoArray[5].eulerAngleOrder[0] = 3;
	scInfoArray[5].eulerAngleOrder[1] = 1;
	scInfoArray[5].eulerAngleOrder[2] = 2;
 
	tableread = PGS_TRUE;

#ifdef _PGS_THREADSAFE
        /* Reset TSD key and global */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY],
                            scInfoArray);     
        PGSg_TSF_EPHtableread[masterTSFIndex] = tableread;
#endif  

        PGS_IO_Gen_Close(filePtr);
	return PGS_EPH_GetSpacecraftData(scTag, scName, searchType, scData);

#endif

      case PGSIO_E_GEN_OPENMODE:
      case PGSIO_E_GEN_REFERENCE_FAILURE:
      case PGSIO_E_GEN_BAD_ENVIRONMENT:
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "an unexpected toolkit error occurred while "
			      "attempting to access the spacecraft tags "
			      "database file",
			      FUNCTION_NAME);
        PGS_IO_Gen_Close(filePtr);
	return PGS_E_TOOLKIT;
	
      case PGS_E_UNIX:
        PGS_IO_Gen_Close(filePtr);
	return returnStatus;
	
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,FUNCTION_NAME);
        PGS_IO_Gen_Close(filePtr);
	return PGS_E_TOOLKIT;
    }

    /* Read the first line of the spacecraft tag information file. */

    charPtr = fgets(buffer, 100, filePtr);

    /* Read the spacecraft tag information from the file.  Keep reading lines
       from the file and adding them to the scTagInfo array until an EOF is
       encountered. */

    while (charPtr != NULL)
    {
	/* Re-allocate memory for scInfoArray to accommodate the new entry. */

	scInfoArray = (PGSt_scTagInfo*) realloc((void*) scInfoArray,
						(numArrayElements+1)*
						(sizeof(PGSt_scTagInfo)));
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY],
                                  scInfoArray);
#endif

	/* Ignore any part of the buffer that follows the first occurrence of a
	   '#' character (ignore the '#' character as well). */

	charPtr = strchr(buffer, '#');
	if (charPtr != NULL)
	{
	    *charPtr = '\0';
	}
	
	/* Parse the line from s/c tag information file... */
	       
	/* Get the numeric s/c tag. */

	charPtr = PGS_EPH_getToken(buffer, ",");
	if (charPtr == NULL)
	{
	    goto GET_NEXT_LINE;
	}
	scInfoArray[numArrayElements].spacecraftTag = atoi(charPtr);
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY],
                                  scInfoArray);
#endif  
	if ( scInfoArray[numArrayElements].spacecraftTag == 0 )
	{
	    goto GET_NEXT_LINE;
	}

	/* Get the s/c name string. */

	charPtr = PGS_EPH_getToken(NULL, ",");
	if (charPtr == NULL)
	{
	    goto GET_NEXT_LINE;
	}
	strcpy(scInfoArray[numArrayElements].spacecraftName, charPtr);
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHSCINFOARRAY],
                                  scInfoArray);
#endif  

	/* Get the Euler angle order values. */

	charPtr = PGS_EPH_getToken(NULL, ",");
	if (charPtr == NULL)
	{
	    goto GET_NEXT_LINE;
	}
	numCheck = sscanf(charPtr, "%1d%1d%1d",
			  &scInfoArray[numArrayElements].eulerAngleOrder[0],
			  &scInfoArray[numArrayElements].eulerAngleOrder[1],  
			  &scInfoArray[numArrayElements].eulerAngleOrder[2]);
	if (numCheck != 3U)
	{
	    goto GET_NEXT_LINE;
	}

	/* Increment the number of array elements in our array.
	   
	   NOTE:
	   If any of the necessary members of the elements of the s/c tag
	   information structure could not be successfully determined above,
	   then this function goes directly to the GET_NEXT_LINE label skipping
	   this statement, effectively ignoring the current record from the s/c
	   tag information file. */

	numArrayElements++;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_EPHnumArrayElements[masterTSFIndex] = numArrayElements;
#endif  
	
    GET_NEXT_LINE:

	/* read the next line of the s/c tag information file */

	charPtr = fgets(buffer, 100, filePtr);
    }

    returnStatus = PGS_IO_Gen_Close(filePtr);
    if (returnStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "An unexpected error occurred while attempting "
			      "to close the spacecraft tags database file.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    return PGS_EPH_GetSpacecraftData(scTag, scName, searchType, scData);
}
