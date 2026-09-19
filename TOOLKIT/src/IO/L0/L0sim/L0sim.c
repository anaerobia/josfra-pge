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
#include <dirent.h>
#include <PGS_math.h>
#include <PGS_IO.h>
#include <PGS_PC.h>
#include <PGS_TD.h>
#include <PGS_EPH.h>
#include <PGS_SIM.h>

#define  MAX_TRIES       5U                       /* maximum number of times a
						     user may attempt to enter
						     in a requested input
						     quantity before the program
						     gives up on said user */


static void quit(void);
static int file_exists(char *);
static PGSt_boolean get_scTag(char *, PGSt_tag *);

PGSt_SMF_status  main()
{
    PGSt_tag     spacecraftTag;
    
    PGSt_double  startTAI93;          /* file start time in seconds since 12AM
					 UTC 1-1-1993 */
    PGSt_double  stopTAI93;           /* file stop time in seconds since 12AM
					 UTC 1-1-1993 */
    PGSt_double  defaultInterval;     /* default time interval between data sets
					 (spacecraft dependent) */
    unsigned int numTries;            /* number of times the user has attempted
					 to input something */
    int          numCheck;            /* used to verify calls to sscanf() and
					 fwrite() */
    int          numFiles;
    int          tempInt;
    int          cnt;
    int          sumAppData;
    int          fileSize;
    
    double       tempDouble;
    
    PGSt_double  timeInterval;

    PGSt_integer pktHdrSize;
    PGSt_integer numPkts;
    PGSt_integer filePkts;
    PGSt_integer granuleSize;
    PGSt_integer count;
    PGSt_integer numAppIDs;
    PGSt_integer pktsPerAppID;
    PGSt_integer extraPkts;
    PGSt_integer sumDataLengths;
    PGSt_integer appDataArrayMaxSize;
    PGSt_uinteger ftrLengths[2];      /* lengths of the QAC lists and MDUL
					 respectively */
    PGSt_integer otherFlags[2];       /* Processing options and Data Type Flags
					 respectively */
    PGSt_integer scID;
    PGSt_integer *appID=NULL;
    PGSt_integer *dataLength=NULL;
    PGSt_integer dataBytes;
    
    unsigned char *appData=NULL;
    
    PGSt_boolean gotData;             /* used to determine if an input value as
					 been successfully ingested */
    PGSt_boolean housekeeping;        /* used to determine if a data set is
					 housekeeping data */
    PGSt_boolean haveAppData;
    

    FILE         *appDataFilePtr;
    
    char         appDataFile[100];    /* name of user supplied file containing
					 Application Data */
    char         inputBuffer[100];    /* input buffer */
    char         filename[100];       /* name of s/c ephemeris files */
    char         sfdu_filename[100];  /* name of detached sfdu header file */
    char         scName[10];          /* spacecraft name */
    char         startUTC[28];        /* beginning UTC time */
    char         stopUTC[28];         /* ending UTC time */
    char         asciiUTC[28];
    char         *newlinePtr;         /* pointer to newline character in the
					 input buffer */

    char         mnemonic[PGS_SMF_MAX_MNEMONIC_SIZE]; /* status mnemonic
							 returned by
							 PGS_SMF_GetMsg() */
    char         msg[PGS_SMF_MAX_MSG_SIZE];           /* status messsage
							 returned by call to
							 PGS_SMF_GetMsg() */
    
    PGSt_SMF_status returnStatus;     /* return status of calls to PGS toolkit
					 functions */
    PGSt_SMF_status code;             /* status code returned by
					 PGS_SMF_GetMsg() */
    
    /*******************
     * BEGIN EXECUTION *
     *******************/

    scName[0] = '\0';

    system("clear\n");

    /* print out lovely picture because Tejmo was bored one day */

    printf("\n       ********************\n"
	   "       * ------O--------- *\n"
	   "       * ___/\\__/\\_______ *\n"
	   "       * __/  \\/  \\______ *\n"
	   "       *  /    \\___\\_____ *\n"
	   "       * /                *\n"
	   "       * ^^^^^^^^^^^^^^^^ *\n"
	   "       * ^^^^^^^^^^^^^^^^ *\n"
	   "       * ^^^^^^^^^^^^^^^^ *\n"
	   "       * =EOS=            *\n"
	   "       ********************\n\n");
    printf("  ECS L0 FILE SIMULATOR\n\n");

    printf("Enter <return> at a prompt to select the default\n"
	   "option (indicated by []).  Enter '?' at any prompt\n"
	   "for additional information.  Enter 'q' at any prompt\n"
	   "to quit.\n\n");
    
    gotData = PGS_FALSE;
    numTries = 0;
 	
    /* determine spacecraft, set appropriate values to the spacecraft name
       string (scName) and default time interval (defaultInterval) */
   
    strcpy(scName,"TRMM");
    do
    {
	if (strlen(scName) == 0)
	  printf("enter spacecraft ID (TRMM, EOS_AM, EOS_PM_GIIS, EOS_PM_GIRD, EOS_AURA, ADEOS_II):\n"
		 "-->");
	else
	  printf("enter spacecraft ID (TRMM, EOS_AM, EOS_PM_GIIS, EOS_PM_GIRD, EOS_AURA, ADEOS_II) [%s]:\n"
		 "-->",scName);
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '?')
	{
	    printf("enter the name (acronym) of one ECS supported spacecraft\n"
		   "exactly as shown below:\n"
		   "    TRMM    \n    EOS_AM\n    EOS_PM_GIIS\n    EOS_PM_GIRD\n    EOS_AURA\n    ADEOS_II\n");
	    continue;
	}
	if (inputBuffer[0] == '\n' )
        {
            if (strlen(scName) != 0)
            {
	        strcpy(inputBuffer,scName);
            }
            else
            {
                strcpy(inputBuffer,"TRMM");
            }
        }
	newlinePtr = strchr(inputBuffer,'\n');
	if (newlinePtr != NULL)
	  *newlinePtr = '\0';
	
	if (get_scTag(inputBuffer, &spacecraftTag) == PGS_FALSE)
	  continue;
	   
	gotData = PGS_TRUE;
	switch (spacecraftTag)
	{
	  case PGSd_TRMM:
	    strcpy(scName,"TRMM");
	    defaultInterval = 6.6;
	    pktHdrSize = PGSd_IO_L0_PrimaryPktHdrSize +
	                 PGSd_IO_L0_SecPktHdrSizeTRMM;
	    break;
	  case PGSd_EOS_AM:
	    strcpy(scName,"EOS_AM");
	    defaultInterval = 1.024;
	    pktHdrSize = PGSd_IO_L0_PrimaryPktHdrSize +
                         PGSd_IO_L0_SecPktHdrSizeEOS_AM;
	    break;
	  case PGSd_EOS_PM_GIIS:
	    strcpy(scName,"EOS_PM_GIIS");
	    defaultInterval = 1.024;
	    pktHdrSize = PGSd_IO_L0_PrimaryPktHdrSize +
                         PGSd_IO_L0_SecPktHdrSizeEOS_PM;
	    break;

	  case PGSd_EOS_PM_GIRD:
	    strcpy(scName,"EOS_PM_GIRD");
	    defaultInterval = 1.024;
	    pktHdrSize = PGSd_IO_L0_PrimaryPktHdrSize +
                         PGSd_IO_L0_SecPktHdrSizeEOS_PM;
	    break;

	  case PGSd_EOS_AURA:
            strcpy(scName,"EOS_AURA");
            defaultInterval = 1.024;
            pktHdrSize = PGSd_IO_L0_PrimaryPktHdrSize +
                         PGSd_IO_L0_SecPktHdrSizeEOS_AURA;
            break;
 
          case PGSd_ADEOS_II:
	    strcpy(scName,"ADEOS_II");
	    defaultInterval = 1.024;
	    pktHdrSize = PGSd_IO_L0_PrimaryPktHdrSize +
                         PGSd_IO_L0_SecPktHdrSizeADEOS_II;
	    break;
	  default:
	    gotData = PGS_FALSE;
	}	    
    }
    while (gotData == PGS_FALSE && numTries++ < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    /* get start time and time interval */

    printf("\n");
    
    gotData = PGS_FALSE;
    numTries = 0;
    
    /* get start date/time */
    
    do
    {
	printf("enter start date:\n-->");
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '\n' )
	  continue;
	if (inputBuffer[0] == '?')
	{
	    printf("\nenter start date in CCSDS ASCII "
		   "(format A or B)\n");
	    printf(" A) YYYY-MM-DDThh:mm:ss.dddddd\n"
		   " B) YYYY-DDDThh:mm:ss.dddddd\n\n");
	    continue;
	}
	newlinePtr = strchr(inputBuffer,'\n');
	if (newlinePtr != NULL)
	  *newlinePtr = '\0';
	returnStatus = PGS_TD_timeCheck(inputBuffer);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_M_ASCII_TIME_FMT_B:
	    strcpy(startUTC,inputBuffer);
	    gotData = PGS_TRUE;
	    break;
	  default:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    printf("\n%s\n\n",msg);
	    continue;
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    /* add some error checking? */

    returnStatus = PGS_TD_UTCtoTAI(startUTC,&startTAI93);
    
    gotData = PGS_FALSE;
    numTries = 0;
    
    /* get stop date/time */
    
    do
    {
	printf("enter stop date:\n-->");
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '\n' )
	  continue;
	if (inputBuffer[0] == '?')
	{
	    printf("\nenter stop date in CCSDS ASCII "
		   "(format A or B)\n");
	    printf(" A) YYYY-MM-DDThh:mm:ss.dddddd\n"
		   " B) YYYY-DDDThh:mm:ss.dddddd\n");
	    printf("the stop date must not be before the start date\n\n");
	    continue;
	}
	newlinePtr = strchr(inputBuffer,'\n');
	if (newlinePtr != NULL)
	  *newlinePtr = '\0';
	returnStatus = PGS_TD_timeCheck(inputBuffer);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_M_ASCII_TIME_FMT_B:
	    strcpy(stopUTC,inputBuffer);
	    gotData = PGS_TRUE;
	    break;
	  default:
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    printf("\n%s\n\n",msg);
	    continue;
	}
	returnStatus = PGS_TD_UTCtoTAI(stopUTC,&stopTAI93);
	if (stopTAI93 < startTAI93)
	{
	    printf("stop time must not be earlier than start time\n");
	    gotData = PGS_FALSE;
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    gotData = PGS_FALSE;
    numTries = 0;
    
    /* get time interval */
    
    do
    {
	printf("enter time interval in seconds [%f sec]:\n"
	       "-->",(double) defaultInterval);
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '?')
	{
	    printf("\nenter the desired time interval between packets\n"
		   "as a real number of seconds (e.g. 60.0)\n\n");
	    continue;
	}
	if (inputBuffer[0] != '\n')
	{
	    numCheck = sscanf(inputBuffer,"%lf",&tempDouble);
	    if (numCheck == 0)
	      continue;
	    if (tempDouble < 1.0e-6)
	    {
		printf("the interval may not be less than 1 microsecond\n");
		continue;
	    }
	    timeInterval = (PGSt_double) tempDouble;
	}
	else
	  timeInterval = defaultInterval;
	    
	gotData = PGS_TRUE;
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    gotData = PGS_FALSE;
    numTries = 0;
    
    /* get number of files */
    
    do
    {
	if (spacecraftTag == PGSd_EOS_AM || 
            spacecraftTag == PGSd_EOS_PM_GIIS || 
	    spacecraftTag == PGSd_EOS_PM_GIRD ||
            spacecraftTag == PGSd_EOS_AURA)
	{
	    printf("enter the desired number of files containing packet data "
		   "[1]:\n-->");
	}
	else
	{	
	    printf("enter the desired number of files [1]:\n"
		   "-->");
	}
	fgets(inputBuffer,99,stdin);
	if (inputBuffer[0] == 'q')
	  quit();
	if (inputBuffer[0] == '?')
	{
	    printf("The number of packets generated by this program is\n"
		   "determined by the start and stop times and the time\n"
		   "interval.  By default these packets will be written\n"
		   "to a single file.  If multiple files are specified,\n"
		   "the packets will be divided into granules of equal\n"
		   "size as nearly as possible and written out in time\n"
		   "order to that number of files (extra packets will be\n"
		   "written out to the last file).\n\n");
	    if (spacecraftTag == PGSd_EOS_AM || 
		spacecraftTag == PGSd_EOS_PM_GIIS || 
		spacecraftTag == PGSd_EOS_PM_GIRD ||
                spacecraftTag == PGSd_EOS_AURA)
	    {
		printf("Do NOT include the Construction Record file in this\n"
		       "count (i.e. entering a value of '1' will produce two\n"
		       "files: the Construction Record file and a single file\n"
		       "containing CCSDS Path SDU packets).\n\n");
	    }
	    continue;
	}
	if (inputBuffer[0] != '\n')
	{
	    numCheck = sscanf(inputBuffer,"%d",
			      &numFiles);
	    if (numCheck == 0)
	      continue;
	    if (numFiles < 1)
	      continue;
	    gotData = PGS_TRUE;
	}
	else
	{
	    numFiles = 1;
	    gotData = PGS_TRUE;
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
    
    /* determine average number of packets/file */
    
    /* the number of packets is the difference between the start time and stop
       time divided by the time interval (plus one since the times are
       inclusive) (also of course only the whole number of packets is desired so
       any fractional part is ignore) */

    numPkts = 1 + (PGSt_integer) ((stopTAI93 - startTAI93)/timeInterval);

    /* the "average number of packets/file (aka granularity) is just the number
       of total packets requested (calculated above) divided by the number of
       files specified (this is a whole number, excess packets will be written
       to the last file if applicable) */

    granuleSize = numPkts/numFiles;

    gotData = PGS_FALSE;
    numTries = 0;

    /* Determine file type.  The file may contain "housekeeping", "quicklook" or
       "production" data (EOS_AM and EOS_PM do not allow "quicklook"). */

    do
    {
	printf("is this Housekeeping data (y/[n]):\n"
	       "-->");
	fgets(inputBuffer,99,stdin);
	switch (inputBuffer[0])
	{
	  case 'q':
	    quit();
	  case 'n':
	  case 'N':
	  case '\n':
	    housekeeping = PGS_FALSE;
	    gotData = PGS_TRUE;
	    break;
	  case 'y':
	  case 'Y':
	    housekeeping = PGS_TRUE;
	    gotData = PGS_TRUE;
	    break;
	  case '?':
	    printf("Production data files contain only one Application ID\n"
		   "per file.  Housekeeping data files may contain multiple\n"
		   "Application IDs in a single file.  The default is one\n"
		   "Application ID per file.  To create files with more\n"
		   "than one Application ID each, answer 'y' at this prompt."
		   "\n\n");
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
      quit();
	      
    /* assume the data is NOT quicklook unless otherwise specified */

    otherFlags[1] = 1;
    if (spacecraftTag == PGSd_TRMM)
    {
	gotData = PGS_FALSE;
	numTries = 0;
	
	do
	{
	    printf("is this Quicklook data (y/[n]):\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    switch (inputBuffer[0])
	    {
	      case 'q':
		quit();
	      case 'n':
	      case 'N':
	      case '\n':
		gotData = PGS_TRUE;
		break;
	      case 'y':
	      case 'Y':
		otherFlags[1] = 2;
		gotData = PGS_TRUE;
		break;
	      case '?':
		printf("The data may be specified as either routine\n"
		       "production data (the default) or as quicklook\n"
		       "data.  This information is stored in the file\n"
		       "header but otherwise has no effect on the file\n"
		       "to be created.  This information is not used\n"
		       "by any Toolkit functions.\n\n");
	    }
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	  quit();
    }
    
    /* if this is housekeeping data ask for number of Application IDs */

    if (housekeeping == PGS_TRUE)
    {
	gotData = PGS_FALSE;
	numTries = 0;
	
	do
	{
	    printf("enter number of Application IDs:\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    switch (inputBuffer[0])
	    {
	      case 'q':
		quit();
	      case '?':
		printf("Housekeeping data files may contain packets with\n"
		       "differing Application IDs (routine production data\n"
		       "may not).  Specify the number of Application IDs to\n"
		       "to used as a whole number not less than 1 or greater\n"
		       "than 255.  The file(s) created will contain packets\n"
		       "in time order and staggered by Application ID.\n"
		       "e.g. if 2 Application IDs are specified the 1st\n"
		       "     packet will correspond to the first Application\n"
		       "     ID specified, the second packet will correspond\n"
		       "     to the second Application ID specified, the\n"
		       "     third packet will correspond to the first\n"
		       "     Application ID, etc.\n\n");
		continue;
	      default:
		numAppIDs = atoi(inputBuffer);
		if (numAppIDs < 1 || numAppIDs > 255)
		{
		    printf("the number of Application IDs must be in the range"
			   " [1,255]\n");
		    continue;
		}
	    }
	    gotData = PGS_TRUE;
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	  quit();
    }
    else
      numAppIDs = 1;
    
    /* Determine the Application ID and Data Length.  If this is housekeeping
       data, determine numAppIDs number of Application IDs and Data Lengths.  */

    gotData = PGS_FALSE;
    numTries = 0;

    /* calloc the space for the appIDs */

    returnStatus = PGS_MEM_Calloc((void **) &appID,
				  sizeof(PGSt_integer)*numPkts, 1);
    returnStatus = PGS_MEM_Calloc((void **) &dataLength,
				  sizeof(PGSt_integer)*numPkts, 1);
    if (housekeeping == PGS_FALSE)
    {
	do
	{
	    printf("enter the Application ID [0]:\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	    if (inputBuffer[0] == '?')
	    {
		printf("Enter the Application ID as a non-negative number\n"
		       "not greater than 2047.\n\n");
		continue;
	    }
	    if (inputBuffer[0] != '\n')
	    {
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		appID[0] = (PGSt_integer) tempInt;
		if (appID[0] < 0 || appID[0] > 2047)
		{
		    printf("Enter the Application ID as a non-negative number\n"
			   "not greater than 2047.\n\n");
		    continue;
		}
	    }
	    else
	      appID[0]=0;
	    printf("enter the Application Data Length [0]:\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	    if (inputBuffer[0] == '?')
	    {
		printf("Enter the length of the Application Data field of\n"
		       "packets with Application ID: %d\n\n", (int) appID[0]);
		continue;
	    }
	    if (inputBuffer[0] != '\n')
	    {
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		dataLength[0] = (PGSt_integer) tempInt;
		if (dataLength[0] < 0)
		{
		    printf("the Application Data Length must be "
			   "non-negative\n");
		    continue;
		}
	    }
	    else
	      appID[0]=0;
	    gotData = PGS_TRUE;
	    for (cnt=1;cnt<numPkts;cnt++)
	    {
		appID[cnt] = appID[0];
		dataLength[cnt] = dataLength[0];
	    }
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	{
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    quit();
	}
    }
    else /* ELSE: if (housekeeping... */
    {
	for (cnt=0;cnt<numAppIDs;cnt++)
	{
	    do
	    {
		printf("enter Application ID #%d:\n-->",cnt+1);
		fgets(inputBuffer,99,stdin);
		if (inputBuffer[0] == 'q')
		{
		    PGS_MEM_Free(appID);
		    PGS_MEM_Free(dataLength);
		    quit();
		}
		if (inputBuffer[0] == '?')
		{
		    printf("Enter the Application ID as a non-negative number\n"
			   "not greater than 2047.\n\n");
		    continue;
		}
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		appID[cnt] = (PGSt_integer) tempInt;
		if (appID[cnt] < 0 || appID[cnt] > 2047)
		{
		    printf("Enter the Application ID as a non-negative number\n"
			   "not greater than 2047.\n\n");
		    continue;
		}
		printf("enter the Application Data Length:\n-->");
		fgets(inputBuffer,99,stdin);
		if (inputBuffer[0] == 'q')
		{
		    PGS_MEM_Free(appID);
		    PGS_MEM_Free(dataLength);
		    quit();
		}
		if (inputBuffer[0] == '?')
		{
		    printf("Enter the length of the Application Data field of\n"
			   "packets with Application ID: %d\n\n", 
			   (int) appID[cnt]);
		    continue;
		}
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		dataLength[cnt] = (PGSt_integer) tempInt;
		if (dataLength[cnt] < 0)
		{
		    printf("the Application Data Length must be "
			   "non-negative\n");
		    continue;
		}
		gotData = PGS_TRUE;
	    }
	    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	    if (gotData == PGS_FALSE)
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	}

	/* stagger the Application IDs and Data Lengths */

	for (cnt=numAppIDs;cnt<numPkts;cnt++)
	{
	    appID[cnt] = appID[cnt-numAppIDs];
	    dataLength[cnt] = dataLength[cnt-numAppIDs];
	}
    } /* END: if (housekeeping... */
    
	
    /* if this is EOS_AM, determine spacecraft ID */

    if (spacecraftTag == PGSd_EOS_AM)
    {
	do
	{
	    printf("enter the Spacecraft ID [169]:\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	    if (inputBuffer[0] == '?')
	    {
		printf("Enter the Spacecraft ID as a non-negative number\n"
		       "not greater than 255 (actual values for EOS AM1 are:\n"
		       "42 (telemetry), 169 (CTIU-1), 170 (CTIU-2)).\n\n");
		continue;
	    }
	    if (inputBuffer[0] != '\n')
	    {
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		scID = (PGSt_integer) tempInt;
		if (scID < 0 || scID > 255)
		{
		    printf("Enter the spacecraft ID as a non-negative number\n"
			   "not greater than 255.\n\n");
		    continue;
		}
	    }
	    else
	      scID=169;
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	{
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    quit();
	}
    }
    
    /* if this is EOS_AURA, determine spacecraft ID */
 
    if (spacecraftTag == PGSd_EOS_AURA)
    {
        do
        {
            printf("enter the Spacecraft ID [204]:\n"
                   "-->");
            fgets(inputBuffer,99,stdin);
            if (inputBuffer[0] == 'q')
            {
                PGS_MEM_Free(appID);
                PGS_MEM_Free(dataLength);
                quit();
            }
            if (inputBuffer[0] == '?')
            {
                printf("Enter the Spacecraft ID as a non-negative number\n"
                       "not greater than 255 (actual values for EOS AURA are:\n"
                       "204 (telemetry).\n\n");
                continue;
            }
            if (inputBuffer[0] != '\n')
            {
                numCheck = sscanf(inputBuffer,"%d", &tempInt);
                if (numCheck == 0)
                  continue;
                scID = (PGSt_integer) tempInt;
                if (scID < 0 || scID > 255)
                {
                    printf("Enter the spacecraft ID as a non-negative number\n"
                           "not greater than 255.\n\n");
                    continue;
                }
            }
            else
              scID=204;
        }
        while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
        if (gotData == PGS_FALSE)
        {
            PGS_MEM_Free(appID);
            PGS_MEM_Free(dataLength);
            quit();
        }
    }

    /* if this is EOS_PM_GIIS, determine spacecraft ID */

    if (spacecraftTag == PGSd_EOS_PM_GIIS)
    {
	do
	{
	    printf("enter the Spacecraft ID [154]:\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	    if (inputBuffer[0] == '?')
	    {
		printf("Enter the Spacecraft ID as a non-negative number\n"
		       "not greater than 255 (actual values for EOS PM1 are:\n"
		       "154 (telemetry).\n\n");
		continue;
	    }
	    if (inputBuffer[0] != '\n')
	    {
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		scID = (PGSt_integer) tempInt;
		if (scID < 0 || scID > 255)
		{
		    printf("Enter the spacecraft ID as a non-negative number\n"
			   "not greater than 255.\n\n");
		    continue;
		}
	    }
	    else
	      scID=154;
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	{
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    quit();
	}
    }
    
    /* if this is EOS_PM_GIRD, determine spacecraft ID */

    if (spacecraftTag == PGSd_EOS_PM_GIRD)
    {
	do
	{
	    printf("enter the Spacecraft ID [154]:\n"
		   "-->");
	    fgets(inputBuffer,99,stdin);
	    if (inputBuffer[0] == 'q')
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	    if (inputBuffer[0] == '?')
	    {
		printf("Enter the Spacecraft ID as a non-negative number\n"
		       "not greater than 255 (actual values for EOS PM1 are:\n"
		       "154 (telemetry).\n\n");
		continue;
	    }
	    if (inputBuffer[0] != '\n')
	    {
		numCheck = sscanf(inputBuffer,"%d", &tempInt);
		if (numCheck == 0)
		  continue;
		scID = (PGSt_integer) tempInt;
		if (scID < 0 || scID > 255)
		{
		    printf("Enter the spacecraft ID as a non-negative number\n"
			   "not greater than 255.\n\n");
		    continue;
		}
	    }
	    else
	      scID=154;
	}
	while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
	if (gotData == PGS_FALSE)
	{
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    quit();
	}
    }


    /* determine total size of Application Data */

    pktsPerAppID = numPkts/numAppIDs;
    extraPkts = numPkts%numAppIDs;
    sumDataLengths = 0;
    sumAppData = 0;
    
    for (cnt=0;cnt<numAppIDs;cnt++)
    {
	sumDataLengths = sumDataLengths + dataLength[cnt];
	sumAppData = sumAppData + pktsPerAppID*dataLength[cnt];
	if (cnt < extraPkts)
	  sumAppData = sumAppData + dataLength[cnt];
    }
    
	
    /* determine if Application Data is available in a user supplied file */

    gotData = PGS_FALSE;
    numTries = 0;

    do
    {
	printf("read in Application Data from file [<none>]:\n"
	       "-->");
	fgets(appDataFile,99,stdin);
	newlinePtr = strchr(appDataFile,'\n');
	if (newlinePtr != NULL)
	  *newlinePtr = '\0';
	switch (appDataFile[0])
	{
	  case '\0':
	    haveAppData = PGS_FALSE;
	    appData = NULL;
	    gotData = PGS_TRUE;
	    break;
	  case 'q':
	    if (strlen(appDataFile) == 1)
	    {
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		quit();
	    }
	  case '?':
	    printf("By default the Application Data fields of all packets\n"
		   "generated by this program will contain all zeros.  A\n"
		   "file on disk containing simulated Application Data may\n"
		   "alternatively be specified here.  This program will\n"
		   "check that any file specified is large enough to contain\n"
		   "enough data to fill all the Application Data requested\n"
		   "(as determined by the information given above).  If the\n"
		   "specified file is large enough and can be read, it will\n"
		   "be opened and its contents copied sequential into each\n"
		   "packet's Application Data field.  This data is in no way\n"
		   "checked or processed by this program.  Specify the file\n"
		   "name (including the full or relative path to the file if"
		   "the file does not reside in the directory from which this\n"
		   "program was launched.\n\n");
	    continue;
	  default:
	    if (file_exists(appDataFile) == PGS_FALSE)
	    {
		printf("cannot find file: %s\n",appDataFile);
		continue;
	    }
	    appDataFilePtr = fopen(appDataFile,"r");
	    if (appDataFilePtr == NULL)
	    {
		printf("unable to open file: %s\n",appDataFile);
		continue;
	    }
	    if (fseek(appDataFilePtr,-1,SEEK_END))
	    {
		printf("error determinig size of file: %s\n",appDataFile);
		fclose(appDataFilePtr);
		continue;
	    }
	    if ((fileSize=ftell(appDataFilePtr)) < sumAppData-1)
	    {
		printf("the file: %s is too small (%d kb) for the amount of\n"
		       "Application Data specified (%d kb)\n", appDataFile, 
		       fileSize/1000, sumAppData/1000);
		fclose(appDataFilePtr);
		continue;
	    }
	    extraPkts = numPkts%numFiles;
	    appDataArrayMaxSize = ((sumAppData/numFiles +1) +
				   (extraPkts/numAppIDs + 1)*sumDataLengths);
	    returnStatus=PGS_MEM_Calloc((void**) &appData, appDataArrayMaxSize,
					1);
	    rewind(appDataFilePtr);
	    haveAppData = PGS_TRUE;
	    gotData = PGS_TRUE;
	}
    }
    while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
    if (gotData == PGS_FALSE)
    {
	PGS_MEM_Free(appID);
	PGS_MEM_Free(dataLength);
	quit();
    }

    /* get Processing Options */
    
    
    otherFlags[0] = 0;
    if (spacecraftTag == PGSd_TRMM)
    {
	gotData = PGS_FALSE;
	numTries = 0;
    
	do 
        {
	    printf("specify processing options (y/[n]):\n-->");
	    fgets(inputBuffer,99,stdin);
       	    switch (inputBuffer[0])
	    {
	      case 'q':
	        quit();
	      case '\n':
	      case 'n':
	      case 'N':
	        gotData = PGS_TRUE;
	        break;
	      case 'y':
	      case 'Y':
	        printf("enter one or more processing options ([3],6,7):\n-->");
	        fgets(inputBuffer,99,stdin);
	        if (inputBuffer[0] == 'q')
		  quit();
	        if (inputBuffer[0] == '\n')
		  inputBuffer[0] = '3';
	        for (cnt=0;cnt< (PGSt_integer) strlen(inputBuffer);cnt++)
		  switch (inputBuffer[cnt])
		  {
		    case ' ':
		    case ',':
		    case '\n':
		      break;
		    case '3':
		      gotData=PGS_TRUE;
		      otherFlags[0] |= 4;
		      break;
		    case '6':
		      gotData=PGS_TRUE;
		      otherFlags[0] |= 32;
		      break;
		    case '7':
		      gotData=PGS_TRUE;
		      otherFlags[0] |= 64;
		      break;
		  }
	        if (gotData == PGS_TRUE)
		  break;
	      case '?':
	        printf("This information is stored in the file\n"
		       "header but otherwise has no effect on the file\n"
		       "to be created.  This information is not used\n"
		       "by any Toolkit functions.\n"  
		       "Valid processing options are:\n"
		       "  3 (Redundant Data Deleted)\n"
		       "  6 (Data Merging)\n"
		       "  7 (RS Decoding)\n"
		       "Processing options may be entered run together (e.g."
		       " 36),\nspace separated (e.g. 3 7), or comma separated"
		       " (e.g. 3,6,7)\n\n");
	    }
        }
        while (gotData == PGS_FALSE && ++numTries < MAX_TRIES);
        if (gotData == PGS_FALSE)
	    quit();
    }

    ftrLengths[0] = 0U;
    ftrLengths[1] = 0U;

    /* calculate total size in bytes of all files to be generated
       NOTE: this is only approximate for EOS_AM and EOS_PM because it includes
             the variable length header which are not currently utilized by
	     those platforms */

    switch (spacecraftTag)
    {
      case PGSd_TRMM:
	dataBytes = sumAppData + numPkts*pktHdrSize + numFiles*(56 +
								(2*numAppIDs) +
								ftrLengths[0] +
								ftrLengths[1]);
	break;
	
      case PGSd_EOS_AM:
	dataBytes = sumAppData + numPkts*pktHdrSize +
                    150 + 96*numAppIDs + (40 + 24*numAppIDs)*(numFiles);
	break;
	
      case PGSd_EOS_PM_GIIS:
        dataBytes = sumAppData + numPkts*pktHdrSize + numFiles*56;
	break;

      case PGSd_EOS_PM_GIRD:
        dataBytes = sumAppData + numPkts*pktHdrSize + numFiles*56;
	break;

      case PGSd_EOS_AURA:
        dataBytes = sumAppData + numPkts*pktHdrSize + numFiles*56;
        break;
 
      default:
	dataBytes = 0xffffffff; /* a glaring value in case we blew it */
    }
    
    dataBytes = sumAppData + numPkts*pktHdrSize + numFiles*(56 + (2*numAppIDs) +
							    ftrLengths[0] +
							    ftrLengths[1]);
    

    /* Print out the start and stop days, time interval and the
       corresponding total size (in MB) of the files this will generate.
       This is here because these files can be quite big. */
    
    printf("\nstart date:    %s\nstop date:     %s\ntime interval: %.4f"
	   " seconds\n\n",startUTC, stopUTC, (double) timeInterval); 
    printf("This will create approximately %.2f MB of data.\n",
	   dataBytes/1.e6);
    
    gotData = PGS_FALSE;
    numTries = 0;
    
    /* Verify start and stop time and time interval input.  This is to give
       the user a chance to alter the input values if the size of the files
       to be generated is larger than anticipated. */
    
    do
    {
	printf("accept ([y]/n)?\n-->");
	fgets(inputBuffer,99,stdin);
	switch (inputBuffer[0])
	{
	  case '\n':
	  case 'y':
	  case 'Y':
	    gotData = PGS_TRUE;
	    numTries = 2*MAX_TRIES;
	    break;
	  case 'n':
	  case 'N':
	    numTries = 2*MAX_TRIES;
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    if (haveAppData == PGS_TRUE)
	    {
		PGS_MEM_Free(appData);
		fclose(appDataFilePtr);
            }
	    quit();
	    break;
	  case 'q':
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    if (haveAppData == PGS_TRUE)
	    {
		PGS_MEM_Free(appData);
		fclose(appDataFilePtr);
            }
	    quit();
	}
    }
    while (numTries++ < MAX_TRIES);
    if (gotData == PGS_FALSE)
    {
	PGS_MEM_Free(appID);
	PGS_MEM_Free(dataLength);
	if (haveAppData == PGS_TRUE)
	{
	    PGS_MEM_Free(appData);
	    fclose(appDataFilePtr);
        }
	quit();
    }
	
    if (spacecraftTag == PGSd_EOS_AM)
    {
	printf("Writing Constuction Record file:\n");
	
	dataBytes = sumAppData + numPkts*pktHdrSize;
	strcpy(filename, "P");
	for (cnt=0;(cnt<numAppIDs)&&(cnt<3);cnt++)
	{
	    sprintf(filename+1+(cnt*7),"%03d%04d",scID,appID[cnt]);
	}
	for (cnt=numAppIDs;cnt<3;cnt++)
	{
	    sprintf(filename+1+(cnt*7),"AAAAAAA");
	}
	PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
	PGS_TD_ASCIItime_AtoB(asciiUTC, asciiUTC);
	sprintf(filename+22, "%.2s%.3s%.2s%.2s%.2s000.PDS",
		asciiUTC+2, asciiUTC+5, asciiUTC+9, asciiUTC+12, asciiUTC+15);
	
	printf("- writing file: %s ...\n", filename);
	returnStatus = PGS_IO_L0_EDOS_hdr_Sim(scID, numAppIDs, appID,
					      dataLength, numFiles+1, dataBytes,
					      numPkts, startTAI93, stopTAI93,
					      timeInterval, filename,spacecraftTag);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    printf("- Error writing file: status = %d\n", (int) returnStatus);
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code == returnStatus)
	    {
		printf("  (%s)\n\n",msg);
	    }
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    if (haveAppData == PGS_TRUE)
	    {
		PGS_MEM_Free(appData);
		fclose(appDataFilePtr);
	    }
	    
	    return PGS_S_SUCCESS;
	}
    }
    

    if (spacecraftTag == PGSd_EOS_PM_GIIS)
    {
	printf("Writing Constuction Record file:\n");
	
	dataBytes = sumAppData + numPkts*pktHdrSize;
	strcpy(filename, "P");
	for (cnt=0;(cnt<numAppIDs)&&(cnt<3);cnt++)
	{
	    sprintf(filename+1+(cnt*7),"%03d%04d",scID,appID[cnt]);
	}
	for (cnt=numAppIDs;cnt<3;cnt++)
	{
	    sprintf(filename+1+(cnt*7),"AAAAAAA");
	}
	PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
	PGS_TD_ASCIItime_AtoB(asciiUTC, asciiUTC);
	sprintf(filename+22, "%.2s%.3s%.2s%.2s%.2s000.PDS",
		asciiUTC+2, asciiUTC+5, asciiUTC+9, asciiUTC+12, asciiUTC+15);
	
	printf("- writing file: %s ...\n", filename);
	
	returnStatus = PGS_IO_L0_EDOS_hdr_Sim(scID, numAppIDs, appID,
					      dataLength, numFiles+1, dataBytes,
					      numPkts, startTAI93, stopTAI93,
					      timeInterval,
					      filename,spacecraftTag);
	
	if (returnStatus != PGS_S_SUCCESS)
	{
	    printf("- Error writing file: status = %d\n", (int) returnStatus);
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code == returnStatus)
	    {
		printf("  (%s)\n\n",msg);
	    }
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    if (haveAppData == PGS_TRUE)
	    {
		PGS_MEM_Free(appData);
		fclose(appDataFilePtr);
	    }
	    
	    return PGS_S_SUCCESS;
	}
    }

    if (spacecraftTag == PGSd_EOS_PM_GIRD)
    {
	printf("Writing Constuction Record file:\n");
	
	dataBytes = sumAppData + numPkts*pktHdrSize;
	strcpy(filename, "P");
	for (cnt=0;(cnt<numAppIDs)&&(cnt<3);cnt++)
	{
	    sprintf(filename+1+(cnt*7),"%03d%04d",scID,appID[cnt]);
	}
	for (cnt=numAppIDs;cnt<3;cnt++)
	{
	    sprintf(filename+1+(cnt*7),"AAAAAAA");
	}
	PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
	PGS_TD_ASCIItime_AtoB(asciiUTC, asciiUTC);
	sprintf(filename+22, "%.2s%.3s%.2s%.2s%.2s000.PDS",
		asciiUTC+2, asciiUTC+5, asciiUTC+9, asciiUTC+12, asciiUTC+15);
	
	printf("- writing file: %s ...\n", filename);
	returnStatus = PGS_IO_L0_EDOS_hdr_Sim(scID, numAppIDs, appID,
					      dataLength, numFiles+1, dataBytes,
					      numPkts, startTAI93, stopTAI93,
					      timeInterval, filename,spacecraftTag);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    printf("- Error writing file: status = %d\n", (int) returnStatus);
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code == returnStatus)
	    {
		printf("  (%s)\n\n",msg);
	    }
	    PGS_MEM_Free(appID);
	    PGS_MEM_Free(dataLength);
	    if (haveAppData == PGS_TRUE)
	    {
		PGS_MEM_Free(appData);
		fclose(appDataFilePtr);
	    }
	    
	    return PGS_S_SUCCESS;
	}
    }
 
    if (spacecraftTag == PGSd_EOS_AURA)
    {
        printf("Writing Constuction Record file:\n");
 
        dataBytes = sumAppData + numPkts*pktHdrSize;
        strcpy(filename, "P");
        for (cnt=0;(cnt<numAppIDs)&&(cnt<3);cnt++)
        {
            sprintf(filename+1+(cnt*7),"%03d%04d",scID,appID[cnt]);
        }
        for (cnt=numAppIDs;cnt<3;cnt++)
        {
            sprintf(filename+1+(cnt*7),"AAAAAAA");
        }
        PGS_TD_TAItoUTC(stopTAI93, asciiUTC);
        PGS_TD_ASCIItime_AtoB(asciiUTC, asciiUTC);
        sprintf(filename+22, "%.2s%.3s%.2s%.2s%.2s000.PDS",
                asciiUTC+2, asciiUTC+5, asciiUTC+9, asciiUTC+12, asciiUTC+15);
 
        printf("- writing file: %s ...\n", filename);
        returnStatus = PGS_IO_L0_EDOS_hdr_Sim(scID, numAppIDs, appID,
                                              dataLength, numFiles+1, dataBytes,
                                              numPkts, startTAI93, stopTAI93,
                                              timeInterval, filename,spacecraftTag);
        if (returnStatus != PGS_S_SUCCESS)
        {
            printf("- Error writing file: status = %d\n", (int) returnStatus);
            PGS_SMF_GetMsg(&code,mnemonic,msg);
            if (code == returnStatus)
            {
                printf("  (%s)\n\n",msg);
            }
            PGS_MEM_Free(appID);
            PGS_MEM_Free(dataLength);
            if (haveAppData == PGS_TRUE)
            {
                PGS_MEM_Free(appData);
                fclose(appDataFilePtr);
            }
 
            return PGS_S_SUCCESS;
        }
    }
 

    if (numFiles == 1)
      printf("Writing packets out to %d file:\n", numFiles);
    else
      printf("Writing packets out to %d files:\n", numFiles);
   
    for (cnt=0; cnt<numFiles; cnt++)
    {
	
	PGS_TD_TAItoUTC(startTAI93+cnt*granuleSize*timeInterval,asciiUTC);
	printf("- start time of next file: %s\n", asciiUTC);

	filePkts = (cnt == numFiles-1) ? numPkts-cnt*granuleSize : granuleSize;
	printf("- number of packets in next file: %d\n", (int) filePkts);
	

	if (spacecraftTag == PGSd_EOS_AM)
	{
	    sprintf(filename+34,"%02d.PDS", cnt+1);
	}
	else if(spacecraftTag == PGSd_EOS_PM_GIIS)
	{
	    sprintf(filename+34,"%02d.PDS", cnt+1);
	}
	else if(spacecraftTag == PGSd_EOS_PM_GIRD)
	{
	    sprintf(filename+34,"%02d.PDS", cnt+1);
	}
	else if(spacecraftTag == PGSd_EOS_AURA)
        {
            sprintf(filename+34,"%02d.PDS", cnt+1);
        }
        else
	{
	    sprintf(filename,"%s_G001_%.19sZ_V01.DATASET_01", scName, asciiUTC);
	    sprintf(sfdu_filename,"%s_G001_%.19sZ_V01.SFDU_01", scName, 
		    asciiUTC);
	}
	
	/* TEST FOR EXISTENCE OF FILE!!!! */
	
	if (file_exists(filename))
	{
	    printf("file: %s already exists,\noverwrite (y/[n]):\n-->",
		   filename);
	    fgets(inputBuffer,99,stdin);
	    switch (inputBuffer[0])
	    {
	      case 'y':
	      case 'Y':
		break;
	      case 'q':
		PGS_MEM_Free(appID);
		PGS_MEM_Free(dataLength);
		if (haveAppData == PGS_TRUE)
		{
		    PGS_MEM_Free(appData);
		    fclose(appDataFilePtr);
		}
		quit();
	      default:
		printf("skipping...\n\n");
		continue;
	    }
	}
	if (spacecraftTag == PGSd_TRMM)
	  printf("- writing files: %s\n                 %s\n", filename,
		 sfdu_filename);
	else
	  printf("- writing file: %s ...\n", filename);
	
	/* read in application data (if appropriate) */

	if (haveAppData == PGS_TRUE)
	  if (numAppIDs == 1)
	    fread(appData,dataLength[0],filePkts,appDataFilePtr);
	  else
	    for(count=0;count<filePkts;count++)
	    {
		fread(appData+count,dataLength[cnt*granuleSize+count],1,
		      appDataFilePtr);
	    }
	
		    
	returnStatus = PGS_IO_L0_File_Sim(spacecraftTag,appID+granuleSize*cnt,
					  0,asciiUTC,filePkts,timeInterval,
					  dataLength+granuleSize*cnt,
					  otherFlags,filename,appData,
					  ftrLengths,NULL,NULL);

	if (returnStatus != PGS_S_SUCCESS)
	{
	    printf("- Error writing file: status = %d\n", (int) returnStatus);
	    PGS_SMF_GetMsg(&code,mnemonic,msg);
	    if (code == returnStatus)
	    {
		printf("  (%s)\n\n",msg);
	    }
	}
	
    }
    PGS_MEM_Free(appID);
    PGS_MEM_Free(dataLength);
    if (haveAppData == PGS_TRUE)
    {
	PGS_MEM_Free(appData);
	fclose(appDataFilePtr);
    }

    return PGS_S_SUCCESS;
}








static void quit()
{
    printf("\nquitting\nno data generated\n\n");
    exit(0);
}

static PGSt_boolean get_scTag(char *scNameStr, PGSt_tag *spacecraftTag)
{
    char  *spacecraftNameString;
    
    spacecraftNameString = scNameStr;
    *spacecraftTag = 0;
    
    /* allow for use of new PGSd_ prefix */

    if (strncmp(spacecraftNameString,"PGSd_",5) == 0)
	spacecraftNameString += 5;
    else if (strncmp(spacecraftNameString,"pgsd_",5) == 0)
	spacecraftNameString += 5;
    
    if (!strcmp(spacecraftNameString,"TRMM"))
    {
	*spacecraftTag = PGSd_TRMM;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_AM"))
    {
	*spacecraftTag = PGSd_EOS_AM;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_PM_GIIS"))
    {
	*spacecraftTag = PGSd_EOS_PM_GIIS;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_PM_GIRD"))
    {
	*spacecraftTag = PGSd_EOS_PM_GIRD;
	return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"EOS_AURA"))
    {
        *spacecraftTag = PGSd_EOS_AURA;
        return PGS_TRUE;
    }
    if (!strcmp(spacecraftNameString,"ADEOS_II"))
    {
	*spacecraftTag = PGSd_ADEOS_II;
	return PGS_TRUE;
    }
    
    /* if input is not actually supported, test every case before
       returning since I don't feel like putting a test after each
       case */

    if (!strcmp(spacecraftNameString,"trmm"))
      *spacecraftTag = PGSd_TRMM;
    if (!strcmp(spacecraftNameString,"eos_am"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"EOS AM"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"EOS-AM"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eos am"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eos-am"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"EOSAM"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eosam"))
      *spacecraftTag = PGSd_EOS_AM;
    if (!strcmp(spacecraftNameString,"eos_pm_giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos_pm-giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos_pm giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOS PM GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOS PM_GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOS PM-GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOS-PM GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOS-PM_GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOS-PM-GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos pm giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos pm_giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos pm-giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos-pm giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos-pm_giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos-pm-giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOSPM GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOSPMGIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOSPM_GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"EOSPM-GIIS"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eospm giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eospmgiis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eospm_giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eospm-giis"))
      *spacecraftTag = PGSd_EOS_PM_GIIS;
    if (!strcmp(spacecraftNameString,"eos_pm_gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos_pm-gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos_pm gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOS PM GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOS PM_GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOS PM-GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOS-PM GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOS-PM_GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOS-PM-GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos pm gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos pm_gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos pm-gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos-pm gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos-pm_gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos-pm-gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOSPM GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOSPMGIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOSPM_GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"EOSPM-GIRD"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eospm gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eospmgird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eospm_gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eospm-gird"))
      *spacecraftTag = PGSd_EOS_PM_GIRD;
    if (!strcmp(spacecraftNameString,"eos_aura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"EOS AURA"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"EOS-AURA"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"eos aura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"eos-aura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"EOSAURA"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"eosaura"))
      *spacecraftTag = PGSd_EOS_AURA;
    if (!strcmp(spacecraftNameString,"adeos_II"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"ADEOS II"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"ADEOS-II"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"adeos II"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"adeos-II"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"ADEOSII"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"adeosII"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"adeosii"))
      *spacecraftTag = PGSd_ADEOS_II;
    if (!strcmp(spacecraftNameString,"adeos2"))
      *spacecraftTag = PGSd_ADEOS_II;

    if (*spacecraftTag)
      return PGS_TRUE;
    else
      return PGS_FALSE;
}


/*
 * function: file_split_path()
 * author:   mike sucher
 * purpose:  split path prefix from file name
 * arguments:
 *     char *name;  input:  full name of file including path
 *     char *path;  output: the path, split from the filename
 *     char *fname; output: the filename, split from the path
 * returns:
 *     (1) Success
 * notes:
 *     the path separator is the slash ('/') character, thus this
 *     routine is only intended for Unix filesystems
 */
static int file_split_path(char *name, char *path, char *fname )
{
    char c, *p1, *p2, *q;

    p1 = p2 = name;
        
    while(1)
    {
	for(q=p2; (*q != '/') && *q; q++);
	if(*q) p2 = q+1;
	else break;
    }
    
    strcpy(fname, p2);
 
    if(p2 > p1)
    {
	c = *(p2-1); 
	*(p2-1) = 0;
	strcpy(path, p1);
	*(p2-1) = c; 
    }
    else path[0] = 0;
        
    return 1;
}

/*
 * function: file_exists()
 * author:   mike sucher
 * purpose:  check to see if file exists
 * arguments:
 *     char *name; name, including path, of the file to check
 * returns:
 *     FOUND     (1) file does exist
 *     NOT_FOUND (0) file does not exist
 */

#define FOUND 1
#define NOT_FOUND 0

static int file_exists(char *name)
{
    DIR *dirp;
    struct dirent *dp;
    char fname[128], path[128];

    file_split_path(name,path,fname);
    if(strlen(path) != 0) dirp = opendir(path);
    else dirp = opendir( "." );
    if (dirp == NULL)
      return NOT_FOUND;
    while ( (dp = readdir( dirp )) != NULL )
    if( strcmp( dp->d_name, fname ) == 0 )
    {
	closedir(dirp);
	return FOUND;
    }
    closedir(dirp);
    return NOT_FOUND;
}
