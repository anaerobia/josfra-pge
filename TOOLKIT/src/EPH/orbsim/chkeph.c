/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <PGS_math.h>
#include <PGS_EPH.h>

int byteswap(char *, int l);

int main(int argc, char *argv[])             /* check a s/c ephemeris file */
{					 
    FILE              *ephemFilePtr;     /* pointer to s/c ephemeris file */
    PGSt_ephemHeader  fileHeader;        /* structure containing s/c ephemeris
					    file header information */
    PGSt_attitHeader  tempHeader;        /* structure containing s/c ephemeris
					    file header information */
    PGSt_ephemRecord  scRecord;          /* used to read in and check records
					    from a s/c ephemeris file */
    char              startDay[28];      /* UTC ASCII time equivalent of first
					    time in ephemeris file */
    char              stopDay[28];       /* UTC ASCII time equivalent of last
					    time in ephemeris file */
    size_t            numCheck;          /* checks standard C function calls */
    int               cnt;               /* loop counter */
    int               count;             /* loop counter */
    					 
    PGSt_double       secTAI93;          /* time of a record */
    					 
    PGSt_boolean      foundBad;          /* set to true if any errors are found
					    while checking the file header */

    long              recordsStart;      /* offset to start of first record in
					    file */
    long              recordsEnd;        /* offset to end of last record in
					    file */
    long              fileEnd;
    
    PGSt_scTagInfo    scTagInfo;

    PGSt_SMF_status   returnStatus;
    PGSt_integer     lendcheck;
    char             *ptr, *ptr1, *ptr2, *ptr3, *ptr4;
    char             *ptr5, *ptr6, *ptr7, *ptr8;
    int               ll, ll1, n, m, q;

    /* Check that at least one argument has been specified on the command line.
       If that is not the case issue a usage message and exit. */
    
    if (argc < 2)
    {
	printf("\nThis program checks the contents of the files specified on\n"
	       "the command line to see if they are valid s/c ephemeris\n"
	       "or attitude files.\n\n");
	printf("usage: chkeph <filename> ( <filename2> <filename3> ... )\n\n");
	exit(0);
    }

    /* Assuming each argument on the command line is a file name loop through
       them all checking to see if they are valid s/c ephemeris files. */

    for (cnt=1;cnt<argc;cnt++)
    {
	printf("\n%s:\n",argv[cnt]);
	
	foundBad = PGS_FALSE;
	
	/* Check to see if the file even exists. */
	
	if (PGS_EPH_file_exists(argv[cnt]) == PGS_FALSE)
	{
	    printf("   file not found\n");
	    continue;
	}
	
	/* Attempt to open the file.  This should be successful unless the
	   file does not have read permission. */
	
	ephemFilePtr = fopen(argv[cnt],"r");
	if (ephemFilePtr == NULL)
	{
	    printf("   file could not be opened for reading\n");
	    continue;
	}
	
	/* Read the file header info, checking to make sure that the read is
	   successfully accomplished. */
	
	numCheck = fread(&fileHeader,sizeof(fileHeader),1,ephemFilePtr);
	if (numCheck != 1)
	{
	    printf("   attempt to read header failed\n");
	    fclose(ephemFilePtr);
	    continue;
	}
	
	/* number of orbits is less than 100. So if this is not the case then
	   because of different byte order (since it is an integer number)
	   the number read will be larger than 100. 
	   Therefore, the data read in fileHeader needs to be byte swapped 
	   if they are not char type */

	if(fileHeader.nOrbits > 100)
	  {   
	    lendcheck = 1; /*will swap the bytes read */
	  }
	else
	  {
	    lendcheck = 0; /*will not swap the bytes read */
	  }

	/* byte swap all elements in fileHeader structure, except char 
	   elements */

	if(lendcheck == 1)
	  {
	    ptr = (char *) (&fileHeader.startTime);
	    ll  = sizeof(PGSt_double);
	    byteswap(ptr, ll);
	    
	    ptr1 = (char *) (&fileHeader.endTime);
	    ll   = sizeof(PGSt_double);
	    byteswap(ptr1, ll);
	    
            ptr2 = (char *) (&fileHeader.interval);
	    for(n=0, ptr2; n<6; n++, ptr2+=4) byteswap(ptr2, 4);
	    
            ptr3 = (char *)(&fileHeader.keplerElements);
	    for(m=0, ptr3; m<7; m++, ptr3+=8) byteswap(ptr3, 8);
	    
            ptr4 = (char *) (&fileHeader.qaParameters);
	    for(q=0, ptr4; q<20; q++, ptr4+=4) byteswap(ptr4,4);
	  }

	/* Check the spacecraft ID tag as specified in the file to make sure
	   that it is what is expected. */

	returnStatus = PGS_EPH_GetSpacecraftData(NULL,
						 fileHeader.spacecraftID,
						 PGSe_NAME_SEARCH,
						 &scTagInfo);
	
	if (returnStatus == PGS_S_SUCCESS)
	{
	    printf("   spacecraft ID: %s (%d)\n",fileHeader.spacecraftID,
		   scTagInfo.spacecraftTag);
	}
	else
	{
	    printf("   spacecraft ID: %s (unsupported)\n",
		   fileHeader.spacecraftID);
	}
	
	/* Check the start and stop times to ensure that they both occur on the
	   same day (UTC) as the file name.  Assume unusually large times are
	   not intentional and represent a corrupted file. */

	if (fabs(fileHeader.startTime) >= 1.e11)
	{
	    strcpy(startDay,"unexpected time (too large)");
	    foundBad = PGS_TRUE;
	}
	else
	  PGS_TD_TAItoUTC(fileHeader.startTime,startDay);

	if (fabs(fileHeader.endTime) >= 1.e11)
	{
	    strcpy(stopDay,"unexpected time (too large)");
	    foundBad = PGS_TRUE;
	}
	else
	  PGS_TD_TAItoUTC(fileHeader.endTime,stopDay);

	startDay[27] = '\0';
	printf("   start time: %.6f (%.26s)\n",(double) fileHeader.startTime,
	       startDay);
	startDay[10] = '\0';

	stopDay[27] = '\0';
	printf("   stop time: %.6f (%.26s)\n",(double) fileHeader.endTime,
	       stopDay);
	stopDay[10] = '\0';

	/* time interval should not be less than 1 micro second which is the
	   highest resolution allowed by the toolkit */

	printf("   time interval: %.6f\n",(double) fileHeader.interval);
	if (fabs(fileHeader.interval) < 1.e-6)
	{
	    printf("ERROR: time interval too small\n");
	    foundBad = PGS_TRUE;
	}

	/* The last field(s) of the header is(are) the Universal Reference(s)
	   (UR).  Each UR is 256 bytes long.  The number of URs is specified in
	   the header structure member nURs.  Skip past the URs to get to the
	   first ephemeris or attitude record. */

	fseek(ephemFilePtr, (long) (fileHeader.nURs*PGSd_UR_FIELD_SIZE),
	      SEEK_CUR);

	/* If any of the information above is in error don't even bother to
	   check the records, just move on to the next file. */

	if (foundBad == PGS_TRUE)
	  goto CLOSE_FILE;

	/* Calculate the total number of records expected as determined by the
	   size of the file.  After the header the file is just a series of s/c
	   ephemeris records.  So count the total bytes after the header and
	   divide by the size in bytes of a s/c ephemeris record to get the
	   total number of records expected. */

	recordsStart = ftell(ephemFilePtr);
	fseek(ephemFilePtr,(long) (fileHeader.nRecords*sizeof(scRecord)),
	      SEEK_SET);
	recordsEnd = ftell(ephemFilePtr);
	fseek(ephemFilePtr, 0L, SEEK_END);
	fileEnd = ftell(ephemFilePtr);
	
	fseek(ephemFilePtr,recordsStart,SEEK_SET);
	
	numCheck = fread(&scRecord,sizeof(scRecord),1,ephemFilePtr);
	if (numCheck != 1)
	{
	    printf("\nERROR: no ephemeris records found\n");
	    goto CLOSE_FILE;
	}
	else
	  {		
	    if (lendcheck == 1)
	      {
		ptr5 = (char *) (&scRecord.secTAI93);
		ll  = sizeof(PGSt_double);
		byteswap(ptr5, ll);
		secTAI93 = scRecord.secTAI93;
	      }
	    else if (lendcheck == 0)
	      {
		secTAI93 = scRecord.secTAI93;
	      }
	  }

	if (secTAI93 != fileHeader.startTime)
	{
	    fseek(ephemFilePtr, 0L, SEEK_SET);
	    
	    numCheck = fread(&tempHeader,sizeof(tempHeader),1,ephemFilePtr);
	    if (numCheck != 1)
	    {
		printf("   attempt to read header failed\n");
		fclose(ephemFilePtr);
		continue;
	    }
	    
	    if(lendcheck ==1)
	      {
		/* Convert to local byte order, assume IEEE */
		
		ptr6 = (char *) (&tempHeader.startTime);
		ll  = sizeof(PGSt_double);
		byteswap(ptr6, ll);
		
		ptr7 = (char *) (&tempHeader.endTime);
		ll1   = sizeof(PGSt_double);
		byteswap(ptr7, ll1);
		
		ptr8 = (char *) (&tempHeader.interval);
		for(n=0, ptr8; n<26; n++, ptr8+=4) byteswap(ptr8, 4);
	      }

	    /* The last field(s) of the header is(are) the Universal
	       Reference(s) (UR).  Each UR is 256 bytes long. The number of URs
	       is specified in the header structure member nURs.  Skip past the
	       URs to get to the first ephemeris or attitude record. */
	    
	    fseek(ephemFilePtr, (long) (tempHeader.nURs*PGSd_UR_FIELD_SIZE),
		  SEEK_CUR);

	    recordsStart = ftell(ephemFilePtr);
	    fseek(ephemFilePtr,(long) (tempHeader.nRecords*sizeof(scRecord)),
		  SEEK_SET);
	    recordsEnd = ftell(ephemFilePtr);
	    fseek(ephemFilePtr, 0L, SEEK_END);
	    fileEnd = ftell(ephemFilePtr);
	    
	    fseek(ephemFilePtr,recordsStart,SEEK_SET);
	    
	    numCheck = fread(&scRecord,sizeof(scRecord),1,ephemFilePtr);
	    if (numCheck != 1)
	    {
		printf("\nERROR: no ephemeris records found\n");
		goto CLOSE_FILE;
	    }
	    else
	      {
		if (lendcheck == 1)
		  {
		    ptr5 = (char *) (&scRecord.secTAI93);
		    ll  = sizeof(PGSt_double);
		    byteswap(ptr5, ll);
		    secTAI93 = scRecord.secTAI93;
		  }
		else if (lendcheck == 0)
		  {
		    secTAI93 = scRecord.secTAI93;
		  }
	      }
	    
	    if (secTAI93 != tempHeader.startTime)
	    {
		printf("\nERROR: time of first record != file start time\n");
		goto CLOSE_FILE;
	    }
	    
	}
	
	if (recordsEnd > fileEnd)
	{
	    printf("ERROR: file size too small for number of records "
		   "indicated (%d).\n", fileHeader.nRecords);
	    goto CLOSE_FILE;
	}
	
        foundBad = PGS_FALSE;
	count = 1;
	
	printf("   total records: %d\n",fileHeader.nRecords);
	
	printf("   checking record: 0000001");
	
	/* Read in each record in the file and check that it occurs after the
	   previous record. */
	
	while (1)
	{
	    numCheck = fread(&scRecord,sizeof(scRecord),1,ephemFilePtr);
	    if (numCheck != 1)
	      break;
	    printf("\b\b\b\b\b\b\b%07d",++count);
	  
	    if (lendcheck == 1)
	      {
		ptr5 = (char *) (&scRecord.secTAI93);
		ll  = sizeof(PGSt_double);
		byteswap(ptr5, ll);
	      }

	    if (scRecord.secTAI93 <= secTAI93)
	    {
		printf("\nERROR: record #%d corrupted\n",count);
		foundBad = PGS_TRUE;
		break;
	    }
	    secTAI93 = scRecord.secTAI93;
	    if (count >= fileHeader.nRecords)
	    {
		break;
	    }
	}

	/* If no records were found to be out of sequence (in time), make sure
	   that the actual time of the last record is the same as the time of
	   last record indicated in the file header. */

	if (foundBad == PGS_FALSE)
	{
	    if (secTAI93 != fileHeader.endTime)
	    {
		printf("\nERROR: time of last record != file end time\n");
		printf("   last record: %.6f\n   file end time: %.6f\n",
		       (double) secTAI93, (double) fileHeader.endTime);
	    }
	    
	    else
	      printf(" ... OK.\n");
	}
	

      CLOSE_FILE:;

	fclose(ephemFilePtr);
    }
    printf("\n");
}
