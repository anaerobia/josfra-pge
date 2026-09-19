/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_getAttitRecords.c

DESCRIPTION:
  This file contains the function PGS_EPH_getAttitRecords()

AUTHORS:
  Guru Tej S. Khalsa   / Applied Reseach Corporation
  Peter D. Noerdlinger / SM&A Inc.
  Phuong T. Nguyen     / L3 Communication Corp.
  Adura  Adekunjo      / L3 Communication Corp.
  Abe Taaheri          / L3 Communication Corp.

HISTORY:
  19-Apr-1996  GTSK  Initial version
  20-Aug-1999  PDN   Corrected calculation of records remaining to be read

  09-July-1999 SZ    Updated for the thread-safe functionality
  21-Oct-2002  PTN   Convert the big-endian data read from the attitude and
                     ephemeris binary files into little-endian format.
  14-Feb-2003  AA    Included "lendcheck" flag so that data conversion can be
                     from big-endian to little-endian format and vice-versa.
  31-Mar-2003  AT    Fixed problem with not setting lemdcheck in a case where
                     a previous call to this function( from a separate test
                     sets oldSpacecraftTag but not lendcheck, causing problem
                     when eph file Endinness differes from the machin's 
                     Endianness. Also cleaned up and made a few other changes.
 04-Oct-2004  AA     Introduced a static integer variable "loopcount" which 
                     prevents the function from entering into a recursive loop
                     (NCR 41107).
END_FILE_PROLOG:
******************************************************************************/

/******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get Array of Spacecraft Attitude Records

NAME:
   PGS_EPH_getAttitRecords()

SYNOPSIS:
C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_getAttitRecords(
       PGSt_scTagInfo   *scTagInfo,
       PGSt_double      secTAI93,
       PGSt_integer     maxArraySize,
       PGSt_attitRecord *attitRecordArray,
       PGSt_integer     *totalRecords)
      
DESCRIPTION:
   This tool gets an array of spacecraft attitude records (time and number of
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
   attitRecordArray  array of s/c attitude    N/A
                     records

   totalRecords      number of records passed
                     back in attitRecordArray

RETURNS:
   PGS_S_SUCCESS               successful return 
   PGSEPH_M_SHORT_ARRAY        number of records returned is less than the max.
                               size allowed (i.e. the array size passed in)
   PGSEPH_E_NO_SC_EPHEM_FILE   no s/c attit files could be found for input times
   PGS_E_TOOLKIT               something really gonzo happened
   PGSTSF_E_GENERAL_FAILURE    problem in the thread-safe code

EXAMPLES:
C:
        #define ARRAY_SIZE 500

	PGSt_double      secTAI93 = 205124400.0;
	
	PGSt_scTagInfo   scTagInfo;

	PGSt_attitRecord attitRecordArray[ARRAY_SIZE];

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
   
        returnStatus = PGS_EPH_getAttitRecords(&scTagInfo,secTAI93,ARRAY_SIZE,
	                                       attitRecordArray,&totalRecords);

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
   PGSg_TSF_EPHattitoldSpacecraftTag
   PGSg_TSF_EPHattitnextRecordPtr
   PGSg_TSF_EPHattitlastArraySize
   PGSg_TSF_EPHattitfile_num
   PGSg_TSF_EPHattitnum_files  
   PGSg_TSF_EPHattitloopcont

FILES:
   This file accesses s/c attitude files (specified in the PCF).

FUNCTIONS_CALLED:
   PGS_EPH_getAttitHeaders()
   PGS_IO_Gen_Open()
   PGS_IO_Gen_Close()
   PGS_SMF_SetStaticMsg()    sets the message buffer
   PGS_SMF_SetDynamicMsg()   sets the message buffer
   PGS_SMF_TestErrorLevel()
   PGS_TSF_GetTSFMaster()
   PGS_TSF_GetMasterIndex()

END_PROLOG:
******************************************************************************/

#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <PGS_IO.h>
#include <PGS_EPH.h>
#include <PGS_TSF.h>
#include <arpa/inet.h>

extern int byteswap(char *, int l);

/* name of the function */

#define FUNCTION_NAME "PGS_EPH_getAttitRecords()"
#define FUNCTION_NAME2 "PGS_EPH_getAttitRecords_UN()"

PGSt_SMF_status
PGS_EPH_getAttitRecords(
    PGSt_scTagInfo*   scTagInfo,
    PGSt_integer      *lendcheck,
    PGSt_double       secTAI93,
    PGSt_integer      maxArraySize,
    PGSt_attitRecord* attitRecordArray,
    PGSt_integer*     totalRecords)
{
    PGSt_attitHeader  fileHeader;

    int               i;
    int               seekCheck;

    size_t            temp;
    size_t            numCheck;

    PGSt_integer      numRecords;
    
    FILE              *attitFilePtr=NULL;
    
    PGSt_SMF_status   returnStatus;
    char              *ptr, *ptr1, *ptr2;
    int               ll, ll1, n;
    int               jdata;
    char              *c;
    int               ndata;
    static PGSt_integer  loopcount=0; 

    /* static variables */
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status   retVal;
    /* Create non-static variables and get globals for the 
       thread-safe version */    
    PGSt_tag      attitoldSpacecraftTag;
    long          attitnextRecordPtr;
    PGSt_integer  attitlastArraySize;
    PGSt_integer  attitfile_num;
    PGSt_integer  attitnum_files;
    PGSt_integer  attitloopcount;
    PGSt_hdrSummary *attitheader_array;
    int masterTSFIndex;
    extern PGSt_tag PGSg_TSF_EPHattitoldSpacecraftTag[];
    extern long PGSg_TSF_EPHattitnextRecordPtr[];
    extern PGSt_integer PGSg_TSF_EPHattitlastArraySize[];
    extern PGSt_integer PGSg_TSF_EPHattitfile_num[];
    extern PGSt_integer PGSg_TSF_EPHattitnum_files[];
    extern PGSt_integer PGSg_TSF_EPHattitloopcount[];

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
    attitoldSpacecraftTag = PGSg_TSF_EPHattitoldSpacecraftTag[masterTSFIndex];
    attitnextRecordPtr = PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex];
    attitlastArraySize = PGSg_TSF_EPHattitlastArraySize[masterTSFIndex];
    attitfile_num = PGSg_TSF_EPHattitfile_num[masterTSFIndex];
    attitnum_files = PGSg_TSF_EPHattitnum_files[masterTSFIndex];
    attitloopcount = PGSg_TSF_EPHattitloopcount[masterTSFIndex];

    attitheader_array = (PGSt_hdrSummary *) pthread_getspecific(
                               masterTSF->keyArray[PGSd_TSF_KEYEPHATTITHEADER_ARRAY]);
#else
    static PGSt_tag      oldSpacecraftTag=0;

    static long          nextRecordPtr;

    static PGSt_integer  lastArraySize=-1;
    static PGSt_integer  file_num=-1;
    static PGSt_integer  num_files=-1;
    /*static PGSt_integer  loopcount=0;*/
    static PGSt_hdrSummary *header_array=NULL;
#endif

#ifdef _PGS_THREADSAFE
    if (scTagInfo->spacecraftTag == attitoldSpacecraftTag &&
        attitlastArraySize > 0 &&
	secTAI93 >= attitRecordArray[0].secTAI93 &&
	secTAI93 <= attitRecordArray[attitlastArraySize-1].secTAI93)
    {
	return PGS_S_SUCCESS;	
    }
    
    if ((scTagInfo->spacecraftTag != attitoldSpacecraftTag)|| 
	(!( *lendcheck == 0 ||  *lendcheck == 1)))
    {
	returnStatus = PGS_EPH_getAttitHeaders(scTagInfo, lendcheck, &attitheader_array,
					       &attitnum_files);

        /* Reset TSD key and global */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHATTITHEADER_ARRAY],
                            attitheader_array);
        PGSg_TSF_EPHattitnum_files[masterTSFIndex] = attitnum_files;

	if (returnStatus != PGS_S_SUCCESS)
	{
	    return returnStatus;
	}
	attitoldSpacecraftTag = scTagInfo->spacecraftTag;
	attitlastArraySize = -1;
	attitfile_num = -1;
	attitnextRecordPtr = 0;

        /* Reset globals */
        PGSg_TSF_EPHattitoldSpacecraftTag[masterTSFIndex] = attitoldSpacecraftTag;
        PGSg_TSF_EPHattitlastArraySize[masterTSFIndex] = attitlastArraySize;
        PGSg_TSF_EPHattitfile_num[masterTSFIndex] = attitfile_num;
        PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex] = attitnextRecordPtr;
    }

    if (secTAI93 < attitheader_array[0].startTAI93 ||
	secTAI93 > attitheader_array[attitnum_files-1].stopTAI93)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "input time outside range of staged attitude "
			      "file(s)", FUNCTION_NAME);
	return returnStatus;
    }
    
    /* If the currently open file has already been read from (true if this
       routine has been called with the same spacecraft ID and time as in a
       previous call) set the file pointer to next record to be read.  
       Otherwise save the new time and/or s/c ID information. */
    
    *totalRecords = 0;
    numRecords = maxArraySize;

    /* although the following is already as clear as mud, perhaps it could 
       use a bit more explanation */

    for (i=0; i<attitnum_files; i++)
    {
	if (secTAI93 < attitheader_array[i].stopTAI93)
	{
	    if (secTAI93 < attitheader_array[i].startTAI93)
	    {
		/* this handles the special case of a time that falls inbetween
		   the last time of one physical file and the first time of the
		   (chronologically) next physical file */

		i--;
	    }
	    
	    if (i == attitfile_num && secTAI93 >= attitRecordArray[0].secTAI93)
	    {
		memcpy((char *) attitRecordArray,
		       (char*) (attitRecordArray+attitlastArraySize-1),
		       sizeof(PGSt_attitRecord));
		*totalRecords = 1;
		numRecords -= 1;
		break;
	    }
	    else
	    {
		attitfile_num = i;
		attitnextRecordPtr = 0;
        /* Reset globals */
        PGSg_TSF_EPHattitfile_num[masterTSFIndex] = attitfile_num;
        PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex] = attitnextRecordPtr;
		*totalRecords = 0;
		break;
	    }
	}
    }

    /* open s/c attitude file at requested time */

    returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA, 
				   PGSd_IO_Gen_Read,
				   &attitFilePtr,
				   attitheader_array[attitfile_num].file_version_num);
#else
    if (scTagInfo->spacecraftTag == oldSpacecraftTag &&
        lastArraySize > 0 &&
        secTAI93 >= attitRecordArray[0].secTAI93 &&
        secTAI93 <= attitRecordArray[lastArraySize-1].secTAI93)
    {
        return PGS_S_SUCCESS;
    }
                                   
    if ((scTagInfo->spacecraftTag != oldSpacecraftTag)|| 
	(!( *lendcheck == 0 ||  *lendcheck == 1)))
    {
        returnStatus = PGS_EPH_getAttitHeaders(scTagInfo, lendcheck, &header_array,
                                               &num_files);
        if (returnStatus != PGS_S_SUCCESS)
        {   
            return returnStatus;
        }
        oldSpacecraftTag = scTagInfo->spacecraftTag;
        lastArraySize = -1;
        file_num = -1;
        nextRecordPtr = 0;
    }       
        
    if (secTAI93 < header_array[0].startTAI93 ||
        secTAI93 > header_array[num_files-1].stopTAI93)
    {
        returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
        PGS_SMF_SetDynamicMsg(returnStatus,
                              "input time outside range of staged attitude "
                              "file(s)", FUNCTION_NAME);
        return returnStatus;
    }

    /* If the currently open file has already been read from (true if this
       routine has been called with the same spacecraft ID and time as in a
       previous call) set the file pointer to next record to be read.  
       Otherwise save the new time and/or s/c ID information. */
    
    *totalRecords = 0;
    numRecords = maxArraySize;
                                   
    /* although the following is already as clear as mud, perhaps it could 
       use a bit more explanation */
        
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

            if (i == file_num && secTAI93 >= attitRecordArray[0].secTAI93)
            {
                memcpy((char *) attitRecordArray,
                       (char*) (attitRecordArray+lastArraySize-1),
                       sizeof(PGSt_attitRecord));
                *totalRecords = 1;
                numRecords -= 1; 
                break;
            }
            else
            {
                file_num = i;
                nextRecordPtr = 0;
                *totalRecords = 0;
                break;
            }
        }
    }
             
    /* open s/c attitude file at requested time */
            
    returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA,
                                   PGSd_IO_Gen_Read,
                                   &attitFilePtr,
                                   header_array[file_num].file_version_num);
#endif
    
    if (returnStatus != PGS_S_SUCCESS) 
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "Unexpected failure opening attitude file.",
			      FUNCTION_NAME);
	return returnStatus;
    }

    numCheck = fread(&fileHeader, sizeof(PGSt_attitHeader), 1, attitFilePtr);
    if (numCheck != 1)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "attitude file header.", FUNCTION_NAME);
        PGS_IO_Gen_Close(attitFilePtr);
	return returnStatus;
    }

    if(*lendcheck ==1)
     {
       /* Convert to local byte order, assume IEEE */

       ptr = (char *) (&fileHeader.startTime);
       ll  = sizeof(PGSt_double);
       byteswap(ptr, ll);

       ptr1 = (char *) (&fileHeader.endTime);
       ll1   = sizeof(PGSt_double);
       byteswap(ptr1, ll1);
      
       ptr2 = (char *) (&fileHeader.interval);
       for(n=0, ptr2; n<26; n++, ptr2+=4) byteswap(ptr2, 4);
     }

    seekCheck = fseek(attitFilePtr, (long)(fileHeader.nURs*PGSd_UR_FIELD_SIZE),
		      SEEK_CUR);
    if (seekCheck != 0)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "attitude file header.", FUNCTION_NAME);
        PGS_IO_Gen_Close(attitFilePtr);
	return returnStatus;
    }
    if (numRecords > fileHeader.nRecords)
    {
	numRecords = fileHeader.nRecords;
    }

#ifdef _PGS_THREADSAFE
    if (attitnextRecordPtr != 0)
    {
	seekCheck = fseek(attitFilePtr,attitnextRecordPtr,SEEK_SET);
	if (seekCheck != 0)
	{
	    returnStatus = PGS_E_TOOLKIT;
	    PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
				  "attitude file.", FUNCTION_NAME);
            PGS_IO_Gen_Close(attitFilePtr);
	    return returnStatus;
	}

	/* Prior to Aug 20, 1999 code read:

	temp = nextRecordPtr - sizeof(PGSt_attitHeader);
	
        In the case that nURs is != 0, this calculates too large a value 
	for "temp", which was obviously intended to mark the record pointer 
	relative to true start-of-data, after the header and the URs (univeral
	record locators, giving the pedigree of the data.) But, instead, 
	"temp" was marking the record pointer relative to the end of the 
	header, which is too early, with the strange result that, being too 
	large a number, it caused too few records to be read thereafter! 
	Indeed, the function next attempts to calculate the remaining records 
	waiting to be read, which was coming out too small, because temp, 
	which really stands for the number of bytes worth of records already
        read, was coming out too large.  The next executable line corrects that
        problem.  PDN  20 August, 1999.  */

	temp = attitnextRecordPtr - sizeof(PGSt_attitHeader) - 
	  fileHeader.nURs * PGSd_UR_FIELD_SIZE;
#else
    if (nextRecordPtr != 0)
    {
        seekCheck = fseek(attitFilePtr,nextRecordPtr,SEEK_SET);  
        if (seekCheck != 0)
        {
            returnStatus = PGS_E_TOOLKIT;
            PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
                                  "attitude file.", FUNCTION_NAME);
            PGS_IO_Gen_Close(attitFilePtr);
            return returnStatus;
        }
        
	/* Prior to Aug 20, 1999 code read:
   
        temp = nextRecordPtr - sizeof(PGSt_attitHeader);
        
        In the case that nURs is != 0, this calculates too large a value 
	for "temp", which was obviously intended to mark the record pointer 
	relative to true start-of-data, after the header and the URs (univeral
	record locators, giving the pedigree of the data.) But, instead, 
	"temp" was marking the record pointer relative to the end of the 
	header, which is too early, with the strange result that, being too 
	large a number, it caused too few records to be read thereafter! 
	Indeed, the function next attempts to calculate the remaining records
	waiting to be read, which was coming out too small, because temp, 
	which really stands for the number of bytes worth of records already
        read, was coming out too large.  The next executable line corrects that
        problem.  PDN  20 August, 1999.  */
         
        temp = nextRecordPtr - sizeof(PGSt_attitHeader) - 
	  fileHeader.nURs * PGSd_UR_FIELD_SIZE;
#endif
	if (temp%sizeof(PGSt_attitRecord) != 0)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Corrupt attitude file pointer.",
				  FUNCTION_NAME);
            PGS_IO_Gen_Close(attitFilePtr);
	    return PGS_E_TOOLKIT;
	}
	temp = temp/sizeof(PGSt_attitRecord);
	if (numRecords > (fileHeader.nRecords - temp))
	{
	    numRecords = fileHeader.nRecords - (PGSt_integer) temp;
	}
    }
    
    do
    {
      /*    *totalRecords += fread(attitRecordArray+(*totalRecords), */
      ndata = fread(attitRecordArray+(*totalRecords),
		    sizeof(PGSt_attitRecord),numRecords,
		    attitFilePtr);
      
      if(*lendcheck ==1)
	{
	  for (jdata = 0; jdata < ndata; jdata++) {
	    c  = (char *)(attitRecordArray+(*totalRecords)+jdata);
	    for(n=0, c; n<7; n++, c+=8) byteswap(c, 8);
	    byteswap(c, 4);
	  }
	}

       *totalRecords += ndata;

#ifndef _PGS_THREADSAFE
       lastArraySize = *totalRecords;
       nextRecordPtr = ftell(attitFilePtr);
#endif

#ifdef _PGS_THREADSAFE
	attitlastArraySize = *totalRecords;
	attitnextRecordPtr = ftell(attitFilePtr);
        /* Reset globals */
        PGSg_TSF_EPHattitlastArraySize[masterTSFIndex] = attitlastArraySize;
        PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex] = attitnextRecordPtr;

	returnStatus = PGS_IO_Gen_Close(attitFilePtr);
	
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Unexpected failure closing attitude "
				  "file.", FUNCTION_NAME);
	    return PGS_E_TOOLKIT;
	}
	
	if (*totalRecords < maxArraySize)
	{
	    numRecords = maxArraySize - *totalRecords;
	    attitfile_num++;
            /* Reset global */
            PGSg_TSF_EPHattitfile_num[masterTSFIndex] = attitfile_num;

	    if (attitfile_num >= attitnum_files)
	    {
                return PGSEPH_M_SHORT_ARRAY;		
	    }

	    returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA, 
					   PGSd_IO_Gen_Read,
					   &attitFilePtr,
					   attitheader_array[attitfile_num].
					   file_version_num);
#else
        lastArraySize = *totalRecords;
        nextRecordPtr = ftell(attitFilePtr);
        
        returnStatus = PGS_IO_Gen_Close(attitFilePtr);  
    
        if (returnStatus != PGS_S_SUCCESS)
        {
            PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
                                  "Unexpected failure closing attitude "
                                  "file.", FUNCTION_NAME);
            return PGS_E_TOOLKIT;
        }
        
        if (*totalRecords < maxArraySize)
        {
            numRecords = maxArraySize - *totalRecords;
            file_num++;
        
            if (file_num >= num_files)
            {
                return PGSEPH_M_SHORT_ARRAY;
            }
            
            returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA,
                                           PGSd_IO_Gen_Read,
                                           &attitFilePtr,
                                           header_array[file_num].
                                           file_version_num);
#endif

	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,
				      "Unexpected failure opening attitude "
				      "file.", FUNCTION_NAME);
		return returnStatus;
	    }
	    
	    numCheck = fread(&fileHeader, sizeof(PGSt_attitHeader), 1, 
			     attitFilePtr);
	    if (numCheck != 1)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,"Unexpected failure reading "
				      "attitude file header.", FUNCTION_NAME);
                PGS_IO_Gen_Close(attitFilePtr);
		return returnStatus;
	    }

	    if(*lendcheck == 1)
	      {
		ptr = (char *) (&fileHeader.startTime);
		ll  = sizeof(PGSt_double);
		byteswap(ptr, ll);
		
		ptr1 = (char *) (&fileHeader.endTime);
		ll1   = sizeof(PGSt_double);
		byteswap(ptr1, ll1);
		
		ptr2 = (char *) (&fileHeader.interval);
		for(n=0, ptr2; n<26; n++, ptr2+=4) byteswap(ptr2, 4);
	      }
	    
            seekCheck = fseek(attitFilePtr, (long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE), SEEK_CUR);
	    if (seekCheck != 0)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading"
				      " attitude file header.", FUNCTION_NAME);
                PGS_IO_Gen_Close(attitFilePtr);
		return returnStatus;
	    }

	    if (numRecords > fileHeader.nRecords)
	    {
		numRecords = fileHeader.nRecords;
	    }
	}
    }
    while (*totalRecords < maxArraySize);
    
   loopcount++;
    if (attitRecordArray[*totalRecords-1].secTAI93 < secTAI93)
     {  
        if(loopcount > 20 )
        {
          returnStatus = PGS_E_TOOLKIT;
          PGS_SMF_SetDynamicMsg(returnStatus, " Too many recursive calls of PGS_EPH_getAttitRecords(), Ephemeris/Attitude data file may be corrupted", FUNCTION_NAME);
          return returnStatus;
        }
	
        returnStatus = PGS_EPH_getAttitRecords(scTagInfo, lendcheck, secTAI93,
					       maxArraySize,attitRecordArray,
					       totalRecords);
    }
    loopcount = 0;
    return returnStatus;
    
}

PGSt_SMF_status
PGS_EPH_getAttitRecords_UN(
    PGSt_scTagInfo*   scTagInfo,
    PGSt_integer      *lendcheck,
    PGSt_double       secTAI93,
    PGSt_integer      maxArraySize,
    PGSt_attitRecord* attitRecordArray,
    PGSt_integer*     totalRecords)
{
    PGSt_attitHeader  fileHeader;

    int               i;
    int               seekCheck;

    size_t            temp;
    size_t            numCheck;

    PGSt_integer      numRecords;
    
    FILE              *attitFilePtr=NULL;
    
    PGSt_SMF_status   returnStatus;
    char              *ptr, *ptr1, *ptr2;
    int               ll, ll1, n;
    int               jdata;
    char              *c;
    int               ndata;

    /* static variables */
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status   retVal;
    /* Create non-static variables and get globals for the 
       thread-safe version */    
    PGSt_tag      attitoldSpacecraftTag;
    long          attitnextRecordPtr;
    PGSt_integer  attitlastArraySize;
    PGSt_integer  attitfile_num;
    PGSt_integer  attitnum_files;
    PGSt_hdrSummary *attitheader_array;
    int masterTSFIndex;
    extern PGSt_tag PGSg_TSF_EPHattitoldSpacecraftTag[];
    extern long PGSg_TSF_EPHattitnextRecordPtr[];
    extern PGSt_integer PGSg_TSF_EPHattitlastArraySize[];
    extern PGSt_integer PGSg_TSF_EPHattitfile_num[];
    extern PGSt_integer PGSg_TSF_EPHattitnum_files[];

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
    attitoldSpacecraftTag = PGSg_TSF_EPHattitoldSpacecraftTag[masterTSFIndex];
    attitnextRecordPtr = PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex];
    attitlastArraySize = PGSg_TSF_EPHattitlastArraySize[masterTSFIndex];
    attitfile_num = PGSg_TSF_EPHattitfile_num[masterTSFIndex];
    attitnum_files = PGSg_TSF_EPHattitnum_files[masterTSFIndex];

    attitheader_array = (PGSt_hdrSummary *) pthread_getspecific(
                               masterTSF->keyArray[PGSd_TSF_KEYEPHATTITHEADER_ARRAY]);
#else
    static PGSt_tag      oldSpacecraftTag=0;

    static long          nextRecordPtr;

    static PGSt_integer  lastArraySize=-1;
    static PGSt_integer  file_num=-1;
    static PGSt_integer  num_files=-1;

    static PGSt_hdrSummary *header_array=NULL;
#endif

#ifdef _PGS_THREADSAFE
    if (scTagInfo->spacecraftTag == attitoldSpacecraftTag &&
        attitlastArraySize > 0 &&
	secTAI93 >= attitRecordArray[0].secTAI93 &&
	secTAI93 <= attitRecordArray[attitlastArraySize-1].secTAI93)
    {
	return PGS_S_SUCCESS;	
    }
    
    if ((scTagInfo->spacecraftTag != attitoldSpacecraftTag)|| 
	(!( *lendcheck == 0 ||  *lendcheck == 1)))
    {
	returnStatus = PGS_EPH_getAttitHeaders(scTagInfo, lendcheck, &attitheader_array,
					       &attitnum_files);

        /* Reset TSD key and global */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYEPHATTITHEADER_ARRAY],
                            attitheader_array);
        PGSg_TSF_EPHattitnum_files[masterTSFIndex] = attitnum_files;

	if (returnStatus != PGS_S_SUCCESS)
	{
	    return returnStatus;
	}
	attitoldSpacecraftTag = scTagInfo->spacecraftTag;
	attitlastArraySize = -1;
	attitfile_num = -1;
	attitnextRecordPtr = 0;

        /* Reset globals */
        PGSg_TSF_EPHattitoldSpacecraftTag[masterTSFIndex] = attitoldSpacecraftTag;
        PGSg_TSF_EPHattitlastArraySize[masterTSFIndex] = attitlastArraySize;
        PGSg_TSF_EPHattitfile_num[masterTSFIndex] = attitfile_num;
        PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex] = attitnextRecordPtr;
    }

    if (secTAI93 < attitheader_array[0].startTAI93 ||
	secTAI93 > attitheader_array[attitnum_files-1].stopTAI93)
    {
	returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "input time outside range of staged attitude "
			      "file(s)", FUNCTION_NAME2);
	return returnStatus;
    }
    
    /* If the currently open file has already been read from (true if this
       routine has been called with the same spacecraft ID and time as in a
       previous call) set the file pointer to next record to be read.  
       Otherwise save the new time and/or s/c ID information. */
    
    *totalRecords = 0;
    numRecords = maxArraySize;

    /* although the following is already as clear as mud, perhaps it could 
       use a bit more explanation */

    for (i=0; i<attitnum_files; i++)
    {
	if (secTAI93 < attitheader_array[i].stopTAI93)
	{
	    if (secTAI93 < attitheader_array[i].startTAI93)
	    {
		/* this handles the special case of a time that falls inbetween
		   the last time of one physical file and the first time of the
		   (chronologically) next physical file */

		i--;
	    }
	    
	    if (i == attitfile_num && secTAI93 >= attitRecordArray[0].secTAI93)
	    {
		memcpy((char *) attitRecordArray,
		       (char*) (attitRecordArray+attitlastArraySize-1),
		       sizeof(PGSt_attitRecord));
		*totalRecords = 1;
		numRecords -= 1;
		break;
	    }
	    else
	    {
		attitfile_num = i;
		attitnextRecordPtr = 0;
        /* Reset globals */
        PGSg_TSF_EPHattitfile_num[masterTSFIndex] = attitfile_num;
        PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex] = attitnextRecordPtr;
		*totalRecords = 0;
		break;
	    }
	}
    }

    /* open s/c attitude file at requested time */

    returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA, 
				   PGSd_IO_Gen_Read,
				   &attitFilePtr,
				   attitheader_array[attitfile_num].file_version_num);
#else
    if (scTagInfo->spacecraftTag == oldSpacecraftTag &&
        lastArraySize > 0 &&
        secTAI93 >= attitRecordArray[0].secTAI93 &&
        secTAI93 <= attitRecordArray[lastArraySize-1].secTAI93)
    {
        return PGS_S_SUCCESS;
    }
                                   
    if ((scTagInfo->spacecraftTag != oldSpacecraftTag)|| 
	(!( *lendcheck == 0 ||  *lendcheck == 1)))
    {
        returnStatus = PGS_EPH_getAttitHeaders(scTagInfo, lendcheck, &header_array,
                                               &num_files);
        if (returnStatus != PGS_S_SUCCESS)
        {   
            return returnStatus;
        }
        oldSpacecraftTag = scTagInfo->spacecraftTag;
        lastArraySize = -1;
        file_num = -1;
        nextRecordPtr = 0;
    }       
        
    if (secTAI93 < header_array[0].startTAI93 ||
        secTAI93 > header_array[num_files-1].stopTAI93)
    {
        returnStatus = PGSEPH_E_NO_SC_EPHEM_FILE;
        PGS_SMF_SetDynamicMsg(returnStatus,
                              "input time outside range of staged attitude "
                              "file(s)", FUNCTION_NAME2);
        return returnStatus;
    }

    /* If the currently open file has already been read from (true if this
       routine has been called with the same spacecraft ID and time as in a
       previous call) set the file pointer to next record to be read.  
       Otherwise save the new time and/or s/c ID information. */
    
    *totalRecords = 0;
    numRecords = maxArraySize;
                                   
    /* although the following is already as clear as mud, perhaps it could 
       use a bit more explanation */
        
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

            if (i == file_num && secTAI93 >= attitRecordArray[0].secTAI93)
            {
                memcpy((char *) attitRecordArray,
                       (char*) (attitRecordArray+lastArraySize-1),
                       sizeof(PGSt_attitRecord));
                *totalRecords = 1;
                numRecords -= 1; 
                break;
            }
            else
            {
                file_num = i;
                nextRecordPtr = 0;
                *totalRecords = 0;
                break;
            }
        }
    }
             
    /* open s/c attitude file at requested time */
            
    returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA,
                                   PGSd_IO_Gen_Read,
                                   &attitFilePtr,
                                   header_array[file_num].file_version_num);
#endif
    
    if (returnStatus != PGS_S_SUCCESS) 
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus,
			      "Unexpected failure opening attitude file.",
			      FUNCTION_NAME2);
	return returnStatus;
    }

    numCheck = fread(&fileHeader, sizeof(PGSt_attitHeader), 1, attitFilePtr);
    if (numCheck != 1)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "attitude file header.", FUNCTION_NAME2);
        PGS_IO_Gen_Close(attitFilePtr);
	return returnStatus;
    }

    if(*lendcheck ==1)
     {
       /* Convert to local byte order, assume IEEE */

       ptr = (char *) (&fileHeader.startTime);
       ll  = sizeof(PGSt_double);
       byteswap(ptr, ll);

       ptr1 = (char *) (&fileHeader.endTime);
       ll1   = sizeof(PGSt_double);
       byteswap(ptr1, ll1);
      
       ptr2 = (char *) (&fileHeader.interval);
       for(n=0, ptr2; n<26; n++, ptr2+=4) byteswap(ptr2, 4);
     }

    seekCheck = fseek(attitFilePtr, (long)(fileHeader.nURs*PGSd_UR_FIELD_SIZE),
		      SEEK_CUR);
    if (seekCheck != 0)
    {
	returnStatus = PGS_E_TOOLKIT;
	PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
			      "attitude file header.", FUNCTION_NAME2);
        PGS_IO_Gen_Close(attitFilePtr);
	return returnStatus;
    }
    if (numRecords > fileHeader.nRecords)
    {
	numRecords = fileHeader.nRecords;
    }

#ifdef _PGS_THREADSAFE
    if (attitnextRecordPtr != 0)
    {
	seekCheck = fseek(attitFilePtr,attitnextRecordPtr,SEEK_SET);
	if (seekCheck != 0)
	{
	    returnStatus = PGS_E_TOOLKIT;
	    PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
				  "attitude file.", FUNCTION_NAME2);
            PGS_IO_Gen_Close(attitFilePtr);
	    return returnStatus;
	}

	/* Prior to Aug 20, 1999 code read:

	temp = nextRecordPtr - sizeof(PGSt_attitHeader);
	
        In the case that nURs is != 0, this calculates too large a value 
	for "temp", which was obviously intended to mark the record pointer 
	relative to true start-of-data, after the header and the URs (univeral
	record locators, giving the pedigree of the data.) But, instead, 
	"temp" was marking the record pointer relative to the end of the 
	header, which is too early, with the strange result that, being too 
	large a number, it caused too few records to be read thereafter! 
	Indeed, the function next attempts to calculate the remaining records 
	waiting to be read, which was coming out too small, because temp, 
	which really stands for the number of bytes worth of records already
        read, was coming out too large.  The next executable line corrects that
        problem.  PDN  20 August, 1999.  */

	temp = attitnextRecordPtr - sizeof(PGSt_attitHeader) - 
	  fileHeader.nURs * PGSd_UR_FIELD_SIZE;
#else
    if (nextRecordPtr != 0)
    {
        seekCheck = fseek(attitFilePtr,nextRecordPtr,SEEK_SET);  
        if (seekCheck != 0)
        {
            returnStatus = PGS_E_TOOLKIT;
            PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading "
                                  "attitude file.", FUNCTION_NAME2);
            PGS_IO_Gen_Close(attitFilePtr);
            return returnStatus;
        }
        
	/* Prior to Aug 20, 1999 code read:
   
        temp = nextRecordPtr - sizeof(PGSt_attitHeader);
        
        In the case that nURs is != 0, this calculates too large a value 
	for "temp", which was obviously intended to mark the record pointer 
	relative to true start-of-data, after the header and the URs (univeral
	record locators, giving the pedigree of the data.) But, instead, 
	"temp" was marking the record pointer relative to the end of the 
	header, which is too early, with the strange result that, being too 
	large a number, it caused too few records to be read thereafter! 
	Indeed, the function next attempts to calculate the remaining records
	waiting to be read, which was coming out too small, because temp, 
	which really stands for the number of bytes worth of records already
        read, was coming out too large.  The next executable line corrects that
        problem.  PDN  20 August, 1999.  */
         
        temp = nextRecordPtr - sizeof(PGSt_attitHeader) - 
	  fileHeader.nURs * PGSd_UR_FIELD_SIZE;
#endif
	if (temp%sizeof(PGSt_attitRecord) != 0)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Corrupt attitude file pointer.",
				  FUNCTION_NAME2);
            PGS_IO_Gen_Close(attitFilePtr);
	    return PGS_E_TOOLKIT;
	}
	temp = temp/sizeof(PGSt_attitRecord);
	if (numRecords > (fileHeader.nRecords - temp))
	{
	    numRecords = fileHeader.nRecords - (PGSt_integer) temp;
	}
    }
    
    do
    {
      /*    *totalRecords += fread(attitRecordArray+(*totalRecords), */
      ndata = fread(attitRecordArray+(*totalRecords),
		    sizeof(PGSt_attitRecord),numRecords,
		    attitFilePtr);
      
      if(*lendcheck ==1)
	{
	  for (jdata = 0; jdata < ndata; jdata++) {
	    c  = (char *)(attitRecordArray+(*totalRecords)+jdata);
	    for(n=0, c; n<7; n++, c+=8) byteswap(c, 8);
	    byteswap(c, 4);
	  }
	}

       *totalRecords += ndata;

#ifndef _PGS_THREADSAFE
       lastArraySize = *totalRecords;
       nextRecordPtr = ftell(attitFilePtr);
#endif

#ifdef _PGS_THREADSAFE
	attitlastArraySize = *totalRecords;
	attitnextRecordPtr = ftell(attitFilePtr);
        /* Reset globals */
        PGSg_TSF_EPHattitlastArraySize[masterTSFIndex] = attitlastArraySize;
        PGSg_TSF_EPHattitnextRecordPtr[masterTSFIndex] = attitnextRecordPtr;

	returnStatus = PGS_IO_Gen_Close(attitFilePtr);
	
	if (returnStatus != PGS_S_SUCCESS)
	{
	    PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				  "Unexpected failure closing attitude "
				  "file.", FUNCTION_NAME2);
	    return PGS_E_TOOLKIT;
	}
	
	if (*totalRecords < maxArraySize)
	{
	    numRecords = maxArraySize - *totalRecords;
	    attitfile_num++;
            /* Reset global */
            PGSg_TSF_EPHattitfile_num[masterTSFIndex] = attitfile_num;

	    if (attitfile_num >= attitnum_files)
	    {
                return PGSEPH_M_SHORT_ARRAY;		
	    }

	    returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA, 
					   PGSd_IO_Gen_Read,
					   &attitFilePtr,
					   attitheader_array[attitfile_num].
					   file_version_num);
#else
        lastArraySize = *totalRecords;
        nextRecordPtr = ftell(attitFilePtr);
        
        returnStatus = PGS_IO_Gen_Close(attitFilePtr);  
    
        if (returnStatus != PGS_S_SUCCESS)
        {
            PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
                                  "Unexpected failure closing attitude "
                                  "file.", FUNCTION_NAME2);
            return PGS_E_TOOLKIT;
        }
        
        if (*totalRecords < maxArraySize)
        {
            numRecords = maxArraySize - *totalRecords;
            file_num++;
        
            if (file_num >= num_files)
            {
                return PGSEPH_M_SHORT_ARRAY;
            }
            
            returnStatus = PGS_IO_Gen_Open(PGSd_SC_ATTIT_DATA,
                                           PGSd_IO_Gen_Read,
                                           &attitFilePtr,
                                           header_array[file_num].
                                           file_version_num);
#endif

	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,
				      "Unexpected failure opening attitude "
				      "file.", FUNCTION_NAME2);
		return returnStatus;
	    }
	    
	    numCheck = fread(&fileHeader, sizeof(PGSt_attitHeader), 1, 
			     attitFilePtr);
	    if (numCheck != 1)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus,"Unexpected failure reading "
				      "attitude file header.", FUNCTION_NAME2);
                PGS_IO_Gen_Close(attitFilePtr);
		return returnStatus;
	    }

	    if(*lendcheck == 1)
	      {
		ptr = (char *) (&fileHeader.startTime);
		ll  = sizeof(PGSt_double);
		byteswap(ptr, ll);
		
		ptr1 = (char *) (&fileHeader.endTime);
		ll1   = sizeof(PGSt_double);
		byteswap(ptr1, ll1);
		
		ptr2 = (char *) (&fileHeader.interval);
		for(n=0, ptr2; n<26; n++, ptr2+=4) byteswap(ptr2, 4);
	      }
	    
            seekCheck = fseek(attitFilePtr, (long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE), SEEK_CUR);
	    if (seekCheck != 0)
	    {
		returnStatus = PGS_E_TOOLKIT;
		PGS_SMF_SetDynamicMsg(returnStatus, "Unexpected failure reading"
				      " attitude file header.", FUNCTION_NAME2);
                PGS_IO_Gen_Close(attitFilePtr);
		return returnStatus;
	    }

	    if (numRecords > fileHeader.nRecords)
	    {
		numRecords = fileHeader.nRecords;
	    }
	}
    }
    while (*totalRecords < maxArraySize);

    if (attitRecordArray[*totalRecords-1].secTAI93 < secTAI93)
    {
	returnStatus = PGS_EPH_getAttitRecords(scTagInfo, lendcheck, secTAI93,
					       maxArraySize,attitRecordArray,
					       totalRecords);
    }

    return returnStatus;
}

