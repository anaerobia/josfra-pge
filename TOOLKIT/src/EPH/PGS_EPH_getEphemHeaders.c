/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <PGS_PC.h>
#include <PGS_IO.h>
#include <PGS_EPH.h>
#include <PGS_TSF.h>
#include <arpa/inet.h>

/* name of this function */
 
#define FUNCTION_NAME "PGS_EPH_getEphemHeaders()"

/* prototype for function used to swap byte order */
extern int byteswap(char *, int);

PGSt_SMF_status
PGS_EPH_getEphemHeaders(
    PGSt_scTagInfo     *scTagInfo,
    PGSt_hdrSummary    **header_array,
    PGSt_integer       *num_files,
    PGSt_integer       *lendcheck)
{
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;   /* return status */

    PGSt_integer     file_num;
    PGSt_integer     base;
    PGSt_integer     next;
    PGSt_integer     i;
    PGSt_integer     bogus_versions = 0;

    size_t           swap_size;
    size_t           numCheck;

    FILE*            ephemFilePtr;
    PGSt_ephemHeader header;

    PGSt_hdrSummary         sort_buf;
            
    char             details[PGS_SMF_MAX_MSG_SIZE]; /*detailed error message */
    char             *ptr, *ptr1, *ptr2, *ptr3, *ptr4;
    int              ll, ll1, n, m, q;

   
#ifdef _PGS_THREADSAFE
    /*    PGSt_integer     lendcheck_old;*/
    PGSt_SMF_status  retVal;
    /* Create non-static variables and get globals for the thread-safe version */
    PGSt_tag      headersoldSpacecraftTag;
    /*PGSt_integer  headersum_files;*/
    PGSt_hdrSummary *staticHdrArray;
    int masterTSFIndex;
    extern PGSt_tag PGSg_TSF_EPHheadersoldSpacecraftTag[];
    /* extern PGSt_tag PGSg_TSF_EPHlendcheck_old[];*/
    /* extern PGSt_integer  PGSt_tag PGSg_TSF_EPHheadersnum_files[];*/
    /* Set up global index, and TSF key keeper for the thread-safe */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);  
    masterTSFIndex = PGS_TSF_GetMasterIndex();  
    if (PGS_SMF_TestErrorLevel(retVal) || masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart and TSD key for the thread-safe */
    headersoldSpacecraftTag = 
      PGSg_TSF_EPHheadersoldSpacecraftTag[masterTSFIndex];
    staticHdrArray = (PGSt_hdrSummary *) pthread_getspecific(
                      masterTSF->keyArray[PGSd_TSF_KEYEPHSTATICHDRARRAY]);
#else
    static PGSt_tag         oldSpacecraftTag=0;
    
    static PGSt_hdrSummary  *staticHdrArray=NULL;
    /* static PGSt_integer     lendcheck_old = -1;*/
    /* static PGSt_integer     staticnum_files = -1;*/
#endif

#ifdef _PGS_THREADSAFE
    if (scTagInfo->spacecraftTag == headersoldSpacecraftTag)
    {
	if (staticHdrArray != NULL)
	{
	    *header_array = staticHdrArray;
	    /* *lendcheck = PGSg_TSF_EPHlendcheck_old[masterTSFIndex];*/
	    /* *num_files = PGSg_TSF_EPHheadersnum_files[masterTSFIndex];*/
	    return PGS_S_SUCCESS;
	}
    }
#else
    if (scTagInfo->spacecraftTag == oldSpacecraftTag)
    {
        if (staticHdrArray != NULL)
        {
            *header_array = staticHdrArray;
	   /* *lendcheck = lendcheck_old;*/
	   /* *num_files = staticnum_files;*/
            return PGS_S_SUCCESS;
        }
    }
#endif
    
    returnStatus = PGS_PC_GetNumberOfFiles(PGSd_SC_EPHEM_DATA, num_files);

    if (returnStatus != PGS_S_SUCCESS)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "error in accessing spacecraft ephemeris file(s)",
			      FUNCTION_NAME);
	return returnStatus;
    }

    /*
     *    Allocate the header array and pass the address back to
     *    the calling program.
     */

    /* the pointer *header_array is going to be assigned to staticHdrArray at
       exit from this function. To allocate the header array first we need to 
       free *header_array if it is not NULL. We need also to free 
       staticHdrArray if it is not the same as *header_array. 
       This will avoid memory leaks which have been observed
       running PGS_EPH_GetEphMet_Driver_c testdriver with purify---
       Abe Taaheri added following 9 lines to avoid memory leaks 5/24/99 */

    /* the following was commented out per NCR 8050296 on 01/14/2014. It seems that freeing
       staticHdrArray is not needed. It is a static pointer and its contentent 
       will be changed once a new header_array is malloced and a value found
       for it
    */
    /*
    if (staticHdrArray != NULL)
    {
	if(staticHdrArray != *header_array)
	{
	    free(staticHdrArray);
	    staticHdrArray=NULL;
	}
    }
    */

    if (*header_array != NULL)
    {
	free(*header_array);
	*header_array = NULL;
    }
    
    
    *header_array = (PGSt_hdrSummary*) malloc(*num_files*
					      sizeof(PGSt_hdrSummary));

    if (*header_array == NULL)
    {
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "Failed to allocate sufficient dynamic memory "
			      "for array of s/c ephemeris file headers.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    for (file_num=0;file_num<*num_files;file_num++)
    {
	/* 
	 *    Open the specified physical file
	 */
	
	returnStatus = PGS_IO_Gen_Open(PGSd_SC_EPHEM_DATA, 
				       PGSd_IO_Gen_Read,
				       &ephemFilePtr,
				       file_num+1);

	if (returnStatus != PGS_S_SUCCESS) 
	{
	    PGS_SMF_SetDynamicMsg(PGSEPH_E_NO_SC_EPHEM_FILE,
				  "unable to open spacecraft ephemeris "
				  "file", FUNCTION_NAME);
	    (*header_array)[file_num].startTAI93 = 1.5E50;
	    bogus_versions++;
	    continue;
	}

	/* Read the file header info, checking to make sure that the read is
	   successfully accomplished. */
	
	numCheck = fread(&header, sizeof(PGSt_ephemHeader), 1,
			 ephemFilePtr);

	returnStatus = PGS_IO_Gen_Close(ephemFilePtr);
	
	if (returnStatus != PGS_S_SUCCESS) 
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Unexpected failure closing ephemeris file.",
				  FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}

	if (numCheck != 1)
	{
	    PGS_SMF_SetDynamicMsg(PGSEPH_E_BAD_EPHEM_FILE_HDR,
				  "error attempting to read s/c ephemeris file "
				  "header",
				  FUNCTION_NAME);
	    (*header_array)[file_num].startTAI93 = 1.5E50;
	    bogus_versions++;
	    continue;
	}

	if(header.nOrbits > 100)
	  {   
	    *lendcheck = 1;
	  }
	else
	  {
	    *lendcheck = 0;
	  }

	if(*lendcheck == 1)
	  {
	    ptr = (char *) (&header.startTime);
	    ll  = sizeof(PGSt_double);
	    byteswap(ptr, ll);
	    
	    ptr1 = (char *) (&header.endTime);
	    ll1   = sizeof(PGSt_double);
	    byteswap(ptr1, ll1);
	    
            ptr2 = (char *) (&header.interval);
	    for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
	    
            ptr3 = (char *)(&header.keplerElements);
	    for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
	    
            ptr4 = (char *) (&header.qaParameters);
	    for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
	  }

	if (strcmp(scTagInfo->spacecraftName, header.spacecraftID) != 0)
	{
	    sprintf(details,"Encountered unexpected spacecraftID (%s) in "
		    "ephemeris file header (expected %s).", header.spacecraftID,
		    scTagInfo->spacecraftName);
	    PGS_SMF_SetDynamicMsg(PGSEPH_E_BAD_EPHEM_FILE_HDR, details,
				  FUNCTION_NAME);
	    (*header_array)[file_num].startTAI93 = 1.5E50;
	    bogus_versions++;
	    continue;
	}
	
	(*header_array)[file_num].file_version_num = file_num + 1;
	(*header_array)[file_num].startTAI93 = header.startTime;
	(*header_array)[file_num].stopTAI93 = header.endTime;
    }
    /*
     *    Sort the version array by start time, from earliest to latest
     */

    swap_size = sizeof(PGSt_hdrSummary); /* used for sorting (if necessary) */

    for (base=0; base<*num_files; base++)
    {

        /* 
         *    Get next earliest element
         */
        for (i=base, next=base; i<*num_files; i++) 
        {
            if ((*header_array)[i].startTAI93 < (*header_array)[next].startTAI93)
            {
                next=i;
            }
        }

        /* 
         *    Swap into position, if needed
         */
        if (next != base)
        {
	    memcpy((char*) &sort_buf, (char*) ((*header_array)+base), swap_size);
	    memcpy((char*) ((*header_array)+base), (char*) ((*header_array)+next),
		   swap_size);
	    memcpy((char*) ((*header_array)+next), (char*) &sort_buf,
		   swap_size);
	}
    }
    if (bogus_versions != 0)
    {
	*num_files -= bogus_versions;
	*header_array = (PGSt_hdrSummary*) realloc(*header_array, *num_files*
						   sizeof(PGSt_hdrSummary));
    }
    
    if (*num_files == 0)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "No valid ephemeris files found.",
			      FUNCTION_NAME);
    }

    /*Added per NCR 8049904 but commenting out so that function to work as before*/
    /*
    oldSpacecraftTag = scTagInfo->spacecraftTag;
#ifdef _PGS_THREADSA
    PGSg_TSF_EPHoldSpacecraftTag[masterTSFIndex] = oldSpacecraftTag;
    PGSg_TSF_EPHeadersnum_files[masterTSFIndex] = *num_files;
    PGSg_TSF_EPHlendcheck_old[masterTSFIndex] = *lendcheck;

#else
    staticnum_files = *num_files;
    lendcheck_old = *lendcheck;

#endif
    */
    staticHdrArray = *header_array;
#ifdef _PGS_THREADSAFE
        /* Reset TSD key */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHSTATICHDRARRAY],
                              staticHdrArray);
#endif
    
    return returnStatus;
}

