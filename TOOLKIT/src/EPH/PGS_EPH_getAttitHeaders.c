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
#include <sys/types.h>
#include <PGS_EPH.h>
#include <arpa/inet.h>

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_getAttitHeaders()"

/* prototype for function used to swap byte order */

extern int byteswap(char *, int);


PGSt_SMF_status
PGS_EPH_getAttitHeaders(
    PGSt_scTagInfo     *scTagInfo,
    PGSt_integer       *lendcheck,
    PGSt_hdrSummary    **header_array,
    PGSt_integer       *num_files)
{
    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;   /* return status */
    PGSt_integer     file_num;
    PGSt_integer     base;
    PGSt_integer     next;
    PGSt_integer     i;
    PGSt_integer     bogus_versions = 0;

    /* size_t           swap_size; */

    size_t           numCheck;

    FILE*            attitFilePtr;
    
    PGSt_attitHeader header;

    PGSt_hdrSummary  sort_buf;
    
    char             details[PGS_SMF_MAX_MSG_SIZE]; /*detailed error message */
    char             *ptr, *ptr1, *ptr2;
    int              ll, ll1, n;

    returnStatus = PGS_PC_GetNumberOfFiles(PGSd_SC_ATTIT_DATA, num_files);

    if (returnStatus != PGS_S_SUCCESS)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "error in accessing spacecraft attitude file(s)",
			      FUNCTION_NAME);
	return returnStatus;
    }

    /*
     *    Allocate the header array and pass the address back to
     *    the calling program.
     */

    if (*header_array != NULL)
    {
	free(*header_array);
    }
    
    *header_array = (PGSt_hdrSummary*) malloc(*num_files*
					      sizeof(PGSt_hdrSummary));

    if (*header_array == NULL)
    {
	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
			      "Failed to allocate sufficient dynamic memory "
			      "for array of s/c attitude file headers.",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    for (file_num=0;file_num<*num_files;file_num++)
    {
	/* 
	 *    Open the specified physical file
	 */
	
	returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA, 
				       PGSd_IO_Gen_Read,
				       &attitFilePtr,
				       file_num+1);

	if (returnStatus != PGS_S_SUCCESS) 
	{
	    PGS_SMF_SetDynamicMsg(PGSEPH_E_NO_SC_EPHEM_FILE,
				  "unable to open spacecraft attitude "
				  "file", FUNCTION_NAME);
	    (*header_array)[file_num].startTAI93 = 1.5E50;
	    bogus_versions++;
	    continue;
	}

	/* Read the file header info, checking to make sure that the read is
	   successfully accomplished. */
	
	numCheck = fread(&header, sizeof(PGSt_attitHeader), 1,
			 attitFilePtr);

	returnStatus = PGS_IO_Gen_Close(attitFilePtr);
	
	if (returnStatus != PGS_S_SUCCESS) 
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Unexpected failure closing attitude file.",
				  FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}

	if (numCheck != 1)
	{
	    PGS_SMF_SetStaticMsg(PGSEPH_E_BAD_EPHEM_FILE_HDR, FUNCTION_NAME);
	    (*header_array)[file_num].startTAI93 = 1.5E50;
	    bogus_versions++;
	    continue;
	}
	
       if(*lendcheck==1)
           {
	     ptr = (char *) (&header.startTime);
	     ll  = sizeof(PGSt_double);
	     byteswap(ptr, ll);
	     
	     ptr1 = (char *) (&header.endTime);
	     ll1   = sizeof(PGSt_double);
	     byteswap(ptr1, ll1);
	     
	     ptr2 = (char *) (&header.interval);
	     for(n=0, ptr2; n<26; n++, ptr2+=4) byteswap(ptr2, 4);
	   } 
	if (strcmp(scTagInfo->spacecraftName, header.spacecraftID) != 0)
	{
	    sprintf(details,"Encountered unexpected spacecraftID (%s) in "
		    "attitude file header (expected %s).", header.spacecraftID,
		    scTagInfo->spacecraftName);
	    PGS_SMF_SetDynamicMsg(PGSEPH_E_BAD_EPHEM_FILE_HDR, details,
				  FUNCTION_NAME);
	    (*header_array)[file_num].startTAI93 = 1.5E50;
	    bogus_versions++;
	    continue;
	}

	if ((scTagInfo->eulerAngleOrder[0] != header.eulerAngleOrder[0]) ||
	    (scTagInfo->eulerAngleOrder[1] != header.eulerAngleOrder[1]) ||
	    (scTagInfo->eulerAngleOrder[2] != header.eulerAngleOrder[2]))
	{
	    sprintf(details,
		    "Encountered unexpected Euler angle order (%1d-%1d-%1d) "
		    "in attitude file header (expected %1d-%1d-%1d).",
		    header.eulerAngleOrder[0],
		    header.eulerAngleOrder[1],
		    header.eulerAngleOrder[2],
		    scTagInfo->eulerAngleOrder[0],
		    scTagInfo->eulerAngleOrder[1],
		    scTagInfo->eulerAngleOrder[2]);
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

    for (base=0; base<*num_files; base++)
    {

        /* 
         *    Get next earliest element
         */
        for (i=base, next=base; i<*num_files; i++) 
        {
            if ((*header_array)[i].startTAI93 <
		(*header_array)[next].startTAI93)
            {
                next=i;
            }
        }

        /* 
         *    Swap into position, if needed
         */
        if (next != base)
        {
	    /* swap_size = sizeof(PGSt_hdrSummary); */

	    /* do things the hard way because memcpy decided not come to work
	       one day */

	    sort_buf.file_version_num = (*header_array)[base].file_version_num;
	    sort_buf.startTAI93 = (*header_array)[base].startTAI93;
	    sort_buf.stopTAI93 = (*header_array)[base].stopTAI93;

	    (*header_array)[base].file_version_num =
	      (*header_array)[next].file_version_num;
	    (*header_array)[base].startTAI93 =
	      (*header_array)[next].startTAI93;
	    (*header_array)[base].stopTAI93 =
	      (*header_array)[next].stopTAI93;
	    
	    (*header_array)[next].file_version_num = sort_buf.file_version_num;
	    (*header_array)[next].startTAI93 = sort_buf.startTAI93;
	    (*header_array)[next].stopTAI93 = sort_buf.stopTAI93;

/*  	    memcpy((char*) &sort_buf, (char*) (*header_array)+base, swap_size); */
/* 	    memcpy((char*) (*header_array)+base, (char*) (*header_array)+next, */
/* 		   swap_size); */
/* 	    memcpy((char*) (*header_array)+next, (char*) &sort_buf, */
/* 		   swap_size); */
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
			      "No valid attitude files found.",
			      FUNCTION_NAME);
    }
 return returnStatus;
  }
