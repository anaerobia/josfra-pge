/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_getEphemRecords.c

DESCRIPTION:
  This file contains the function PGS_EPH_getEphemRecords()

AUTHORS:
  Guru Tej S. Khalsa   / Applied Reseach Corporation
  Peter D. Noerdlinger / SM&A Inc.
  Phuong T. Nguyen     / L3 Communication Corp.
  Adura Adekunjo       / L3 Communication Corp.
  Abe Taaheri          / L3 Communication Corp.

HISTORY:
  09-Dec-1994  GTSK  Initial version
  19-Apr-1996  GTSK  Rewritten for new file formats
  20-Aug-1999  PDN   Corrected calculation of records remaining to be read 
  09-July-1999  SZ   Updated for the thread-safe functionality
  21-Oct-2002  PTN   Convert the big-endian data read from the attitude and
                     ephemeris binary files into little-endian format.
  14-Feb-2003  AA    Included "lendcheck" flag so that data conversion can be
                     from big-endian to little-endian format and vice-versa.
  31-Mar-2003  AT    Fixed problem with not setting lemdcheck in a case where
                     a previous call to this function( from a separate test
                     sets oldSpacecraftTag but not lendcheck, causing problem
                     when eph file Endinness differes from the machin's 
                     Endianness. Also cleaned up and made a few other changes.
END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Array of Spacecraft Ephemeris Records

NAME:
   PGS_EPH_getEphemRecords()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_getEphemRecords(
       PGSt_scTagInfo   *scTagInfo,
       PGSt_double      secTAI93,
       PGSt_integer     maxArraySize,
       PGSt_ephemRecord *ephemRecordArray,
       PGSt_integer     *totalRecords,
       PGSt_integer     *lendcheck)
      
DESCRIPTION:
   This tool gets an array of spacecraft ephemeris records (time and number of
   records specified in calling function).

INPUTS:
   Name              Description               Units        Min         Max
   ----              -----------               -----        ---         ---
   scTagInfo         spacecraft tag            N/A
                     information structure

   secTAI93          time of first requested   sec
                     record (in seconds since
		     12 AM UTC 1-1-1993)

   maxArraySize      size of s/c record        N/A
                     structure array passed

OUTPUTS:
   Name              Description               Units        Min         Max
   ----              -----------               -----        ---         ---
   ephemRecordArray  array of s/c ephemeris    N/A
                     records

   totalRecords      number of records passed
                     back in ephemRecordArray
   
   lendcheck        Flag to check whether machine
                    is  big or little endian

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSEPH_M_SHORT_ARRAY        number of records returned is less than the max.
                               size allowed (i.e. the array size passed in)
   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c ephem files could be found for input times
   PGS_E_TOOLKIT               something really gonzo happened
   PGSTSF_E_GENERAL_FAILURE    problem in the thread-safe code

EXAMPLES:
C:
        #define ARRAY_SIZE 500

	PGSt_double      secTAI93 = 205124400.0;

	PGSt_scTagInfo   scTagInfo;

	PGSt_ephemRecord ephemRecordArray[ARRAY_SIZE];

	PGSt_integer     totalRecords;

	PGSt_SMF_status  returnStatus;


	returnStatus = PGS_EPH_GetSpacecraftData(PGSd_EOS_AM, NULL,
                                                 PGSe_TAG_SEARCH, &scTagInfo);

	if (returnStatus != PGS_S_SUCCESS)
	{
	            :
	 ** do some error handling ***
		    :
	}

        returnStatus = PGS_EPH_getEphemRecords(&scTagInfo,secTAI93,ARRAY_SIZE,
	                                       ephemRecordArray,&totalRecords);

	if (returnStatus != PGS_S_SUCCESS)
	{
	            :
	 ** do some error handling ***
		    :
	}
   
NOTES:
   None

REQUIREMENTS:
   PGSTK - 0720

DETAILS:
   None

GLOBALS:
   PGSg_TSF_EPHoldSpacecraftTag
   PGSg_TSF_EPHnextRecordPtr
   PGSg_TSF_EPHlastArraySize
   PGSg_TSF_EPHfile_num
   PGSg_TSF_EPHnum_files

FILES:
   This file accesses s/c attitude files (specified in the PCF).

FUNCTIONS_CALLED:
   PGS_EPH_getEphemHeaders()
   PGS_IO_Gen_Open()
   PGS_IO_Gen_Close()
   PGS_SMF_SetStaticMsg()    sets the message buffer
   PGS_SMF_SetDynamicMsg()   sets the message buffer
   PGS_SMF_TestErrorLevel()
   PGS_TSF_GetTSFMaster()
   PGS_TSF_GetMasterIndex()

END_PROLOG:
*******************************************************************************/

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <PGS_IO.h>
#include <PGS_EPH.h>
#include <PGS_TSF.h>
#include <arpa/inet.h>

int byteswap(char *, int l);

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_getEphemRecords()"
#define FUNCTION_NAME2 "PGS_EPH_getEphemRecords_UN()"
PGSt_SMF_status
PGS_EPH_getEphemRecords(
    PGSt_scTagInfo*   scTagInfo,
    PGSt_double       secTAI93,
    PGSt_integer      maxArraySize,
    PGSt_ephemRecord* ephemRecordArray,
    PGSt_integer*     totalRecords,
    PGSt_integer*     lendcheck)
{
    PGSt_ephemHeader  fileHeader;
    char              *ptr, *ptr1, *ptr2, *ptr3, *ptr4;
    int               ll, ll1, n, m, q;
    char              *c;
    int               jdata =0;
    int               ndata; 

    int               i;
    int               seekCheck;

    size_t            temp;
    size_t            numCheck;

    PGSt_integer      numRecords;
    
    FILE              *ephemFilePtr=NULL;
    
    PGSt_SMF_status   returnStatus;

    /* static variables */
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status   retVal;
    /* Create non-static variables and get globals for the 
       thread-safe version */
    PGSt_tag      oldSpacecraftTag;
    long          nextRecordPtr;
    PGSt_integer  lastArraySize;
    PGSt_integer  file_num;
    PGSt_integer  num_files;
    PGSt_hdrSummary *header_array;
    int masterTSFIndex;
    extern PGSt_tag PGSg_TSF_EPHoldSpacecraftTag[];
    extern long PGSg_TSF_EPHnextRecordPtr[];
    extern PGSt_integer PGSg_TSF_EPHlastArraySize[];
    extern PGSt_integer PGSg_TSF_EPHfile_num[];
    extern PGSt_integer PGSg_TSF_EPHnum_files[];

    /* Set up global index, and TSF key keeper for the thread-safe */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(retVal) || 
	masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart and TSD key for the thread-safe */
    oldSpacecraftTag = PGSg_TSF_EPHoldSpacecraftTag[masterTSFIndex];
    nextRecordPtr = PGSg_TSF_EPHnextRecordPtr[masterTSFIndex];
    lastArraySize = PGSg_TSF_EPHlastArraySize[masterTSFIndex];
    file_num = PGSg_TSF_EPHfile_num[masterTSFIndex];
    num_files = PGSg_TSF_EPHnum_files[masterTSFIndex];

    header_array = (PGSt_hdrSummary *) pthread_getspecific(
                        masterTSF->keyArray[PGSd_TSF_KEYEPHHEADER_ARRAY]);
#else
    static PGSt_tag      oldSpacecraftTag=0;

    static long          nextRecordPtr;

    static PGSt_integer  lastArraySize=-1;
    static PGSt_integer  file_num=-1;
    static PGSt_integer  num_files=-1;

    static PGSt_hdrSummary *header_array=NULL;
#endif

    if (scTagInfo->spacecraftTag == oldSpacecraftTag &&
        lastArraySize > 0 &&
	secTAI93 >= ephemRecordArray[0].secTAI93 &&
	secTAI93 <= ephemRecordArray[lastArraySize-1].secTAI93)
    {
	return PGS_S_SUCCESS;	
    }
    
    if ((scTagInfo->spacecraftTag != oldSpacecraftTag) || 
	(!( *lendcheck == 0 ||  *lendcheck == 1)))
    {
	returnStatus = PGS_EPH_getEphemHeaders(scTagInfo, &header_array,
					       &num_files, lendcheck);
#ifdef _PGS_THREADSAFE
        /* Reset global and TSD key */
        PGSg_TSF_EPHnum_files[masterTSFIndex] = num_files;
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHHEADER_ARRAY],
                            header_array);
#endif
	if (returnStatus != PGS_S_SUCCESS)
	{
	    return returnStatus;
	}
	oldSpacecraftTag = scTagInfo->spacecraftTag;
	lastArraySize = -1;
	file_num = -1;
	nextRecordPtr = 0;

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_EPHoldSpacecraftTag[masterTSFIndex] = oldSpacecraftTag;
        PGSg_TSF_EPHlastArraySize[masterTSFIndex] = lastArraySize;
        PGSg_TSF_EPHfile_num[masterTSFIndex] = file_num;
        PGSg_TSF_EPHnextRecordPtr[masterTSFIndex] = nextRecordPtr;
#endif  
    }

    if (secTAI93 < header_array[0].startTAI93 ||
	secTAI93 > header_array[num_files-1].stopTAI93)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "input time outside range of staged ephemeris "
			      "file(s)", FUNCTION_NAME);
	return returnStatus;	
    }
    
    /* If the currently open file has already been read from (true if this
       routine has been called with the same spacecraft ID and time as in a
       previous call) set the file pointer to next record to be read. Otherwise
       save the new time and/or s/c ID information. */

    /* although the following is already as clear as mud, perhaps it 
       could use abit more explanation */

    *totalRecords = 0;
    numRecords = maxArraySize;

    for (i=0; i<num_files; i++)
    {
	if (secTAI93 < header_array[i].stopTAI93)
	{
	    if (secTAI93 < header_array[i].startTAI93)
	    {
		/* this handles the special case of a time that falls inbetween
		   the last time of one physical file and the first time of the
		   (chronologically) next physical file */

		i--;
	    }
	    
	    if (i == file_num && secTAI93 >= ephemRecordArray[0].secTAI93)
	    {
		memcpy((char *) ephemRecordArray,
		       (char*) (ephemRecordArray+lastArraySize-1),
		       sizeof(PGSt_ephemRecord));
		*totalRecords = 1;
		numRecords -= 1;
		break;
	    }
	    else
	    {
		file_num = i;
		nextRecordPtr = 0;

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_EPHfile_num[masterTSFIndex] = file_num;
        PGSg_TSF_EPHnextRecordPtr[masterTSFIndex] = nextRecordPtr;
#endif
		break;
	    }
	}
    }
    
    /* open s/c ephemeris file at requested time */

    returnStatus = PGS_IO_Gen_Open(PGSd_SC_EPHEM_DATA, 
				   PGSd_IO_Gen_Read,
				   &ephemFilePtr,
				   header_array[file_num].file_version_num);
    
    if (returnStatus != PGS_S_SUCCESS) 
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "Unexpected failure opening ephemeris file.",
			      FUNCTION_NAME);
	return returnStatus;
    }

    numCheck = fread(&fileHeader, sizeof(PGSt_ephemHeader), 1, ephemFilePtr);
    if (numCheck != 1)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "ephemeris file header.", FUNCTION_NAME);
        PGS_IO_Gen_Close(ephemFilePtr);
	return returnStatus;
    }


   if(*lendcheck == 1)
     {
       /* Convert to local byte order, assume IEEE */

       ptr = (char *) (&fileHeader.startTime);
       ll  = sizeof(PGSt_double);
       byteswap(ptr, ll);
       
       ptr1 = (char *) (&fileHeader.endTime);
       ll1   = sizeof(PGSt_double);
       byteswap(ptr1, ll1);

       ptr2 = (char *) (&fileHeader.interval);
       for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
      
       ptr3 = (char *) (&fileHeader.keplerElements);
       for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
 
       ptr4 = (char *) (&fileHeader.qaParameters);
       for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
     }

    seekCheck = fseek(ephemFilePtr,(long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE),
		      SEEK_CUR);
    if (seekCheck != 0)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "ephemeris file header.", FUNCTION_NAME);
        PGS_IO_Gen_Close(ephemFilePtr);
	return returnStatus;
    }

    if (numRecords > fileHeader.nRecords)
    {
	numRecords = fileHeader.nRecords;
    }

    if (nextRecordPtr != 0)
    {
	seekCheck = fseek(ephemFilePtr,nextRecordPtr,SEEK_SET);
	if (seekCheck != 0)
	{
	    returnStatus = PGS_E_TOOLKIT;
	    PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
				  "ephemeris file.", FUNCTION_NAME);
            PGS_IO_Gen_Close(ephemFilePtr);
	    return returnStatus;
	}

/* Prior to Aug 20, 1999 code read:

	temp = nextRecordPtr - sizeof(PGSt_ephemHeader);
  
        In the case that nURs is != 0, this calculates too large a value 
	for "temp", which was obviously intended to mark the record pointer 
	relative to true start-of-data, after the header and the URs 
	(univeral record locators, giving the pedigree of the data.) But, 
	instead, "temp" was marking the record pointer relative to the end 
	of the header, which is too early, with the strange result that, 
	being too large a number, it caused too few records to be read 
	thereafter! Indeed, the function next attempts to calculate the 
        remaining records waiting to be read, which was coming out too small,
	because temp, which really stands for the number of bytes worth of 
	records already read, was coming out too large.  The next executable 
	line corrects that problem.  PDN  20 August, 1999.  */

        temp = nextRecordPtr - sizeof(PGSt_ephemHeader) - 
	  fileHeader.nURs * PGSd_UR_FIELD_SIZE;

	if (temp%sizeof(PGSt_ephemRecord) != 0)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Corrupt ephemeris file pointer.",
				  FUNCTION_NAME);
            PGS_IO_Gen_Close(ephemFilePtr);
	    return PGS_E_TOOLKIT;
	}
	temp = temp/sizeof(PGSt_ephemRecord);
	if (numRecords > (fileHeader.nRecords - temp))
	{
	    numRecords = fileHeader.nRecords - (PGSt_integer) temp;
	}
    }
    
    do
    {

       /* *totalRecords += fread(ephemRecordArray+(*totalRecords), */
       ndata = fread(ephemRecordArray+(*totalRecords),
                               sizeof(PGSt_ephemRecord),numRecords,
                               ephemFilePtr);

       if(*lendcheck == 1)
	 {
	   for (jdata = 0; jdata < ndata; jdata++) {
	     c  = (char *)(ephemRecordArray+(*totalRecords)+ jdata);
	     for(n=0, c; n<7; n++, c+=8) byteswap(c, 8);
	     byteswap(c, 4);
	   }
	 }
       
       *totalRecords += ndata;

#ifndef _PGS_THREADSAFE
	lastArraySize = *totalRecords;
	nextRecordPtr = ftell(ephemFilePtr);
#endif

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_EPHlastArraySize[masterTSFIndex] = lastArraySize;
        PGSg_TSF_EPHnextRecordPtr[masterTSFIndex] = nextRecordPtr;
#endif  
	returnStatus = PGS_IO_Gen_Close(ephemFilePtr);
	
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Unexpected failure closing ephemeris "
				  "file.", FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	
	if (*totalRecords < maxArraySize)
	{
	    numRecords = maxArraySize - *totalRecords;
	    file_num++;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_EPHfile_num[masterTSFIndex] = file_num;
#endif  
	    if (file_num >= num_files)
	    {
                return PGSEPH_M_SHORT_ARRAY;		
	    }

	    returnStatus = PGS_IO_Gen_Open(PGSd_SC_EPHEM_DATA, 
					   PGSd_IO_Gen_Read,
					   &ephemFilePtr,
					   header_array[file_num].
					   file_version_num);
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,
				      "Unexpected failure opening ephemeris "
				      "file.", FUNCTION_NAME);
		return returnStatus;
	    }
	    
	    numCheck = fread(&fileHeader, sizeof(PGSt_ephemHeader), 1, 
			     ephemFilePtr);
	    if (numCheck != 1)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,"Unexpected failure reading "
				      "ephemeris file header.", FUNCTION_NAME);
                PGS_IO_Gen_Close(ephemFilePtr);
		return returnStatus;
	    }

	    if(*lendcheck ==1)
	      {
		ptr = (char *) (&fileHeader.startTime);
		ll  = sizeof(PGSt_double);
		byteswap(ptr, ll);
		
		ptr1 = (char *) (&fileHeader.endTime);
		ll1   = sizeof(PGSt_double);
		byteswap(ptr1, ll1);
		
		ptr2 = (char *) (&fileHeader.interval);
		for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
		
		ptr3 = (char *) (&fileHeader.keplerElements);
		for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
		
		ptr4 = (char *) (&fileHeader.qaParameters);
		for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
	      }

            seekCheck = fseek(ephemFilePtr,
			      (long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE),
			      SEEK_CUR);
	    if (seekCheck != 0)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading"
				      " ephemeris file header.", FUNCTION_NAME);
                PGS_IO_Gen_Close(ephemFilePtr);
		return returnStatus;
	    }
	    
	    if (numRecords > fileHeader.nRecords)
	    {
		numRecords = fileHeader.nRecords;
	    }
	}
    }
    while (*totalRecords < maxArraySize);

    if (ephemRecordArray[*totalRecords-1].secTAI93 < secTAI93)
    {
	returnStatus = PGS_EPH_getEphemRecords(scTagInfo,secTAI93,
					       maxArraySize,ephemRecordArray,
					       totalRecords, lendcheck);
    }

    return returnStatus;
}

/* 
   This will be called from PGS_EPH_EphAtt_unInterpolate() function. It is
   added to avoid core dump in linux machine when 
   PGS_EPH_EphAtt_unInterpolate() is called after PGS_EPH_EphemAttit().
   Before this fix core dump occured (in linux only) while freeing memory 
   for header_array when entering first time in PGS_EPH_getEphemHeaders().
*/

PGSt_SMF_status
PGS_EPH_getEphemRecords_UN(
    PGSt_scTagInfo*   scTagInfo,
    PGSt_double       secTAI93,
    PGSt_integer      maxArraySize,
    PGSt_ephemRecord* ephemRecordArray,
    PGSt_integer*     totalRecords,
    PGSt_integer*     lendcheck)
{
    PGSt_ephemHeader  fileHeader;
    char              *ptr, *ptr1, *ptr2, *ptr3, *ptr4;
    int               ll, ll1, n, m, q;
    char              *c;
    int               jdata =0;
    int               ndata; 

    int               i;
    int               seekCheck;

    size_t            temp;
    size_t            numCheck;

    PGSt_integer      numRecords;
    
    FILE              *ephemFilePtr=NULL;
    
    PGSt_SMF_status   returnStatus;

    /* static variables */
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status   retVal;
    /* Create non-static variables and get globals for the 
       thread-safe version */
    PGSt_tag      oldSpacecraftTag;
    long          nextRecordPtr;
    PGSt_integer  lastArraySize;
    PGSt_integer  file_num;
    PGSt_integer  num_files;
    PGSt_hdrSummary *header_array;
    int masterTSFIndex;
    extern PGSt_tag PGSg_TSF_EPHoldSpacecraftTag[];
    extern long PGSg_TSF_EPHnextRecordPtr[];
    extern PGSt_integer PGSg_TSF_EPHlastArraySize[];
    extern PGSt_integer PGSg_TSF_EPHfile_num[];
    extern PGSt_integer PGSg_TSF_EPHnum_files[];

    /* Set up global index, and TSF key keeper for the thread-safe */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(retVal) || 
	masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart and TSD key for the thread-safe */
    oldSpacecraftTag = PGSg_TSF_EPHoldSpacecraftTag[masterTSFIndex];
    nextRecordPtr = PGSg_TSF_EPHnextRecordPtr[masterTSFIndex];
    lastArraySize = PGSg_TSF_EPHlastArraySize[masterTSFIndex];
    file_num = PGSg_TSF_EPHfile_num[masterTSFIndex];
    num_files = PGSg_TSF_EPHnum_files[masterTSFIndex];

    header_array = (PGSt_hdrSummary *) pthread_getspecific(
                        masterTSF->keyArray[PGSd_TSF_KEYEPHHEADER_ARRAY]);
#else
    static PGSt_tag      oldSpacecraftTag=0;

    static long          nextRecordPtr;

    static PGSt_integer  lastArraySize=-1;
    static PGSt_integer  file_num=-1;
    static PGSt_integer  num_files=-1;

    static PGSt_hdrSummary *header_array=NULL;
#endif

    if (scTagInfo->spacecraftTag == oldSpacecraftTag &&
        lastArraySize > 0 &&
	secTAI93 >= ephemRecordArray[0].secTAI93 &&
	secTAI93 <= ephemRecordArray[lastArraySize-1].secTAI93)
    {
	return PGS_S_SUCCESS;	
    }
    
    if ((scTagInfo->spacecraftTag != oldSpacecraftTag) || 
	(!( *lendcheck == 0 ||  *lendcheck == 1)))
    {
	returnStatus = PGS_EPH_getEphemHeaders(scTagInfo, &header_array,
					       &num_files, lendcheck);
#ifdef _PGS_THREADSAFE
        /* Reset global and TSD key */
        PGSg_TSF_EPHnum_files[masterTSFIndex] = num_files;
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHHEADER_ARRAY],
                            header_array);
#endif
	if (returnStatus != PGS_S_SUCCESS)
	{
	    return returnStatus;
	}
	oldSpacecraftTag = scTagInfo->spacecraftTag;
	lastArraySize = -1;
	file_num = -1;
	nextRecordPtr = 0;

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_EPHoldSpacecraftTag[masterTSFIndex] = oldSpacecraftTag;
        PGSg_TSF_EPHlastArraySize[masterTSFIndex] = lastArraySize;
        PGSg_TSF_EPHfile_num[masterTSFIndex] = file_num;
        PGSg_TSF_EPHnextRecordPtr[masterTSFIndex] = nextRecordPtr;
#endif  
    }

    if (secTAI93 < header_array[0].startTAI93 ||
	secTAI93 > header_array[num_files-1].stopTAI93)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "input time outside range of staged ephemeris "
			      "file(s)", FUNCTION_NAME2);
	return returnStatus;	
    }
    
    /* If the currently open file has already been read from (true if this
       routine has been called with the same spacecraft ID and time as in a
       previous call) set the file pointer to next record to be read. Otherwise
       save the new time and/or s/c ID information. */

    /* although the following is already as clear as mud, perhaps it 
       could use abit more explanation */

    *totalRecords = 0;
    numRecords = maxArraySize;

    for (i=0; i<num_files; i++)
    {
	if (secTAI93 < header_array[i].stopTAI93)
	{
	    if (secTAI93 < header_array[i].startTAI93)
	    {
		/* this handles the special case of a time that falls inbetween
		   the last time of one physical file and the first time of the
		   (chronologically) next physical file */

		i--;
	    }
	    
	    if (i == file_num && secTAI93 >= ephemRecordArray[0].secTAI93)
	    {
		memcpy((char *) ephemRecordArray,
		       (char*) (ephemRecordArray+lastArraySize-1),
		       sizeof(PGSt_ephemRecord));
		*totalRecords = 1;
		numRecords -= 1;
		break;
	    }
	    else
	    {
		file_num = i;
		nextRecordPtr = 0;

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_EPHfile_num[masterTSFIndex] = file_num;
        PGSg_TSF_EPHnextRecordPtr[masterTSFIndex] = nextRecordPtr;
#endif
		break;
	    }
	}
    }
    
    /* open s/c ephemeris file at requested time */

    returnStatus = PGS_IO_Gen_Open(PGSd_SC_EPHEM_DATA, 
				   PGSd_IO_Gen_Read,
				   &ephemFilePtr,
				   header_array[file_num].file_version_num);
    
    if (returnStatus != PGS_S_SUCCESS) 
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "Unexpected failure opening ephemeris file.",
			      FUNCTION_NAME2);
	return returnStatus;
    }

    numCheck = fread(&fileHeader, sizeof(PGSt_ephemHeader), 1, ephemFilePtr);
    if (numCheck != 1)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "ephemeris file header.", FUNCTION_NAME2);
        PGS_IO_Gen_Close(ephemFilePtr);
	return returnStatus;
    }


   if(*lendcheck == 1)
     {
       /* Convert to local byte order, assume IEEE */

       ptr = (char *) (&fileHeader.startTime);
       ll  = sizeof(PGSt_double);
       byteswap(ptr, ll);
       
       ptr1 = (char *) (&fileHeader.endTime);
       ll1   = sizeof(PGSt_double);
       byteswap(ptr1, ll1);

       ptr2 = (char *) (&fileHeader.interval);
       for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
      
       ptr3 = (char *) (&fileHeader.keplerElements);
       for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
 
       ptr4 = (char *) (&fileHeader.qaParameters);
       for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
     }

    seekCheck = fseek(ephemFilePtr,(long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE),
		      SEEK_CUR);
    if (seekCheck != 0)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "ephemeris file header.", FUNCTION_NAME2);
        PGS_IO_Gen_Close(ephemFilePtr);
	return returnStatus;
    }

    if (numRecords > fileHeader.nRecords)
    {
	numRecords = fileHeader.nRecords;
    }

    if (nextRecordPtr != 0)
    {
	seekCheck = fseek(ephemFilePtr,nextRecordPtr,SEEK_SET);
	if (seekCheck != 0)
	{
	    returnStatus = PGS_E_TOOLKIT;
	    PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
				  "ephemeris file.", FUNCTION_NAME2);
            PGS_IO_Gen_Close(ephemFilePtr);
	    return returnStatus;
	}

/* Prior to Aug 20, 1999 code read:

	temp = nextRecordPtr - sizeof(PGSt_ephemHeader);
  
        In the case that nURs is != 0, this calculates too large a value 
	for "temp", which was obviously intended to mark the record pointer 
	relative to true start-of-data, after the header and the URs 
	(univeral record locators, giving the pedigree of the data.) But, 
	instead, "temp" was marking the record pointer relative to the end 
	of the header, which is too early, with the strange result that, 
	being too large a number, it caused too few records to be read 
	thereafter! Indeed, the function next attempts to calculate the 
        remaining records waiting to be read, which was coming out too small,
	because temp, which really stands for the number of bytes worth of 
	records already read, was coming out too large.  The next executable 
	line corrects that problem.  PDN  20 August, 1999.  */

        temp = nextRecordPtr - sizeof(PGSt_ephemHeader) - 
	  fileHeader.nURs * PGSd_UR_FIELD_SIZE;

	if (temp%sizeof(PGSt_ephemRecord) != 0)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Corrupt ephemeris file pointer.",
				  FUNCTION_NAME2);
            PGS_IO_Gen_Close(ephemFilePtr);
	    return PGS_E_TOOLKIT;
	}
	temp = temp/sizeof(PGSt_ephemRecord);
	if (numRecords > (fileHeader.nRecords - temp))
	{
	    numRecords = fileHeader.nRecords - (PGSt_integer) temp;
	}
    }
    
    do
    {

       /* *totalRecords += fread(ephemRecordArray+(*totalRecords), */
       ndata = fread(ephemRecordArray+(*totalRecords),
                               sizeof(PGSt_ephemRecord),numRecords,
                               ephemFilePtr);

       if(*lendcheck == 1)
	 {
	   for (jdata = 0; jdata < ndata; jdata++) {
	     c  = (char *)(ephemRecordArray+(*totalRecords)+ jdata);
	     for(n=0, c; n<7; n++, c+=8) byteswap(c, 8);
	     byteswap(c, 4);
	   }
	 }
       
       *totalRecords += ndata;

#ifndef _PGS_THREADSAFE
	lastArraySize = *totalRecords;
	nextRecordPtr = ftell(ephemFilePtr);
#endif

#ifdef _PGS_THREADSAFE
        /* Reset globals */
        PGSg_TSF_EPHlastArraySize[masterTSFIndex] = lastArraySize;
        PGSg_TSF_EPHnextRecordPtr[masterTSFIndex] = nextRecordPtr;
#endif  
	returnStatus = PGS_IO_Gen_Close(ephemFilePtr);
	
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Unexpected failure closing ephemeris "
				  "file.", FUNCTION_NAME2);
	    return PGS_E_TOOLKIT;
	}
	
	if (*totalRecords < maxArraySize)
	{
	    numRecords = maxArraySize - *totalRecords;
	    file_num++;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_EPHfile_num[masterTSFIndex] = file_num;
#endif  
	    if (file_num >= num_files)
	    {
                return PGSEPH_M_SHORT_ARRAY;		
	    }

	    returnStatus = PGS_IO_Gen_Open(PGSd_SC_EPHEM_DATA, 
					   PGSd_IO_Gen_Read,
					   &ephemFilePtr,
					   header_array[file_num].
					   file_version_num);
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,
				      "Unexpected failure opening ephemeris "
				      "file.", FUNCTION_NAME2);
		return returnStatus;
	    }
	    
	    numCheck = fread(&fileHeader, sizeof(PGSt_ephemHeader), 1, 
			     ephemFilePtr);
	    if (numCheck != 1)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,"Unexpected failure reading "
				      "ephemeris file header.", FUNCTION_NAME2);
                PGS_IO_Gen_Close(ephemFilePtr);
		return returnStatus;
	    }

	    if(*lendcheck ==1)
	      {
		ptr = (char *) (&fileHeader.startTime);
		ll  = sizeof(PGSt_double);
		byteswap(ptr, ll);
		
		ptr1 = (char *) (&fileHeader.endTime);
		ll1   = sizeof(PGSt_double);
		byteswap(ptr1, ll1);
		
		ptr2 = (char *) (&fileHeader.interval);
		for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
		
		ptr3 = (char *) (&fileHeader.keplerElements);
		for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
		
		ptr4 = (char *) (&fileHeader.qaParameters);
		for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
	      }

            seekCheck = fseek(ephemFilePtr,
			      (long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE),
			      SEEK_CUR);
	    if (seekCheck != 0)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading"
				      " ephemeris file header.", FUNCTION_NAME2);
                PGS_IO_Gen_Close(ephemFilePtr);
		return returnStatus;
	    }
	    
	    if (numRecords > fileHeader.nRecords)
	    {
		numRecords = fileHeader.nRecords;
	    }
	}
    }
    while (*totalRecords < maxArraySize);

    if (ephemRecordArray[*totalRecords-1].secTAI93 < secTAI93)
    {
	returnStatus = PGS_EPH_getEphemRecords(scTagInfo,secTAI93,
					       maxArraySize,ephemRecordArray,
					       totalRecords, lendcheck);
    }

    return returnStatus;
}



/**********************************************************************/
/*
 * function: byteswap()
 * author:   James Johnson
 * purpose:  Performs byte swapping for 2 and 4 byte integers,
 *           as well as 4 and 8 byte floats.
 * date:     Febuary 6, 1995
*/
 
int byteswap(char *d, int l)
{
  char tmp;
  
  switch(l)
    {
    default:
      break;
    case 2:
      tmp=*d; *d=*(d+1);*(d+1)=tmp;
      break;
    case 4:
      tmp=*d; *d=*(d+3);*(d+3)=tmp;
      tmp=*(d+1); *(d+1)=*(d+2);*(d+2)=tmp;
      break;
    case 8:
      tmp=*d; *d=*(d+7);*(d+7)=tmp;
      tmp=*(d+1); *(d+1)=*(d+6);*(d+6)=tmp;
      tmp=*(d+2); *(d+2)=*(d+5);*(d+5)=tmp;
      tmp=*(d+3); *(d+3)=*(d+4);*(d+4)=tmp;
      break;
    }
  return(0);
}

