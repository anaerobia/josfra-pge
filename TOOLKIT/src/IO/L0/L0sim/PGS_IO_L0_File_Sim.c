/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
   PGS_IO_L0_File_Sim.c

DESCRIPTION:
   This file contains the tool to create a file of simulated Level 0 data.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corp.
   Michael E. Sucher / Applied Research Corp.
   Tom W. Atwater / Applied Research Corp.
   Xin Wang / EIT Inc.

HISTORY:
   10-Dec-1994 GTSK Initial version
   28-Dec-1994 MES  Spiffed up, added some stuff
   03-Feb-1995 TWA  Further spiffing up.  Added prologs.
   01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG

TITLE:
	Create a Simulated Level 0 Data File

NAME:
	PGS_IO_L0_File_Sim

SYNOPSIS

  C:
	#include <PGS_IO.h>
	#include <PGS_IO_L0.h>

	PGSt_SMF_status
	PGS_IO_L0_File_Sim(
            PGSt_tag         spacecraftTag,
	    PGSt_integer     appID[],        
	    PGSt_integer     firstPacketNum
	    char             startUTC[28], 
	    PGSt_integer     numValues,    
	    PGSt_double      timeInterval, 
	    PGSt_integer     dataLength[], 
	    PGSt_integer     otherFlags[2],
	    char             *filename,
	    void             *appData,
	    PGSt_uinteger    qualMissLen[2])
	    void             *qualData)
	    void             *missData)

  FORTRAN:
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'

	integer function pgs_io_l0_file_sim( spacecrafttag, appid,
       +                      firstpacketnum, startutc, numvalues,
       +                      timeinterval, datalength, otherflags, filename,
       +                      appdata, qualmisslen, qualdata, missdata )
	integer		 spacecrafttag
	integer		 appid(*)
	integer		 firstpacketnum
	character*27     startutc
	integer		 numvalues
	double precision timeinterval
	integer          datalength(*)
	integer          otherflags(2)
	character*(*)    filename
	(any)            appdata
	integer          qualmisslen(2)
	(any)            qualdata
	(any)            missdata

DESCRIPTION:
	This tool creates file(s) containing simulated Level 0 data,
        each of which has a file header, packet data, and a file footer.
	For TRMM, a detached SFDU header file is also created for each
        Level 0 data file.
 
INPUTS:
	spacecraftTag - The spacecraft identifier desired for the output data.

	appID         - Array of application process identifiers (APIDs),
	                one for each packet to be generated.

	firstPacketNum- Value of Packet Sequence Count to use for the initial 
	                packet.

	startUTC      - The UTC time of the first packet.  Formats supported:
	                    a) YYYY-MM-DDThh:mm:ss.dddddd
	                    b) YYYY-DDDThh:mm:ss.dddddd

	numValues     - The number of packets to generate.

	timeInterval  - Time interval (in seconds) between packets.

	dataLength    - Array of lengths, in bytes, of the Application Data 
	                for each packet. Does not include lengths of
	                primary and secondary packet headers.

	otherFlags    - Array of length 2 with file header values
	                otherFlags[0]: bit-packed "Processing Options" byte
	                   TRMM values:
	                      bit 3 on -- Redundant Data Deleted
	                      bit 6 on -- Data Merging
	                      bit 7 on -- RS Decoding
	                      bits 1,2,4,5,8 -- always off
	                   For example, to simulate Redundant Data Deleted
	                   and RS Decoding, turn bits 3 and 7 on, which is
	                   decimal 68. So set otherFlags[0]=68 .
	                otherFlags[1]: "Data type Flags" byte
	                   TRMM values: 
	                      otherFlags[1]=1, Routine production data
	                      otherFlags[1]=2, Quicklook data
	                (NOTE: These two fields are simply written to
	                the appropriate place in the file header; no processing
	                is done in this function based on their values.)
	                
	filename      - The name of the file to be created containing the L0 
	                packets.

	appData       - Optional user-defined input of the packet application
	                data field.  Does not include packet header data.
	                In C, if appData=NULL, a block of data of length
                        equal to the largest value in array dataLength 
                        is filled with zeroes, for each packet. 

(The remaining inputs are for TRMM file footer processing only.
They are ignored fo other platforms.)

	qualMissLen  - Array of length 2 with file footer section lengths
                          qualMissLen[0]: quality (QAC) buffer length
                             if qualMissLen[0]=0, no quality data
                             are written to the file
                          qualMissLen[1]: missing data (MDUL) buffer length
                             if qualMissLen[1]=0 or qualMissLen[0]=0, 
                             no missing data are written to the file
                          (QAC length and MDUL length are always
                             written to the file)

	qualData      - Quality and Accounting Capsule (QAC) data
                           In C, if qualData=NULL, a block of data of length
                              qualMissLen[0] is filled with zeroes.
                              and written to the file
                           (In Fortran you pass a zero-filled array for this.)

	missData      - Missing Data Unit List (MDUL) data
                           In C, if missData=NULL, a block of data of length
                              qualMissLen[1] is filled with zeroes.
                              and written to the file
                           (In Fortran you pass a zero-filled array for this.)

OUTPUTS:
	None

RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_BAD_NUM_PKTS
	PGSIO_E_L0_BAD_APP_ID
	PGSIO_E_L0_BAD_FIRST_PKTNUM
	PGSTD_E_SC_TAG_UNKNOWN
	PGSIO_E_L0_BAD_NUM_APP_IDS
	PGSIO_E_L0_BAD_DATA_LENGTH
	PGSTD_E_TIME_FMT_ERROR
	PGSTD_E_TIME_VALUE_ERROR
	PGSTD_E_DATE_OUT_OF_RANGE
	PGS_E_TOOLKIT
	PGS_E_UNIX
	PGSMEM_E_MAXSIZE
	PGSIO_E_L0_PHYSICAL_OPEN

NOTES:
	This tool is intended for use in science software development and 
	testing, but not for production purposes.  

	TK4 RELEASE NOTES:
        For Level 0 access tools, this delivery consists of prototype
        code.
	This delivery supports the TRMM Level 0 File format, as defined in
	"Interface Control Document between the
	Sensor Data Processing Facility (SPDF) and the Tropical Rainfall
	Measuring Mission (TRMM) Customers", NASA Mission Operations
	and Data Systems Directorate, Draft, Nov. 1994.
        Support for EOS AM and PM Level 0 File formats is limited to
        packet data; file header data for these platforms is not
        defined at this writing (Feb. 1995). For now it is assumed that 
        EOS file headers are identical to the static part of TRMM file headers. 
        Unlike TRMM, it is assumed that EOS file headers have no 
        variable length part, nor do EOS files have footers.
        The internal structure of the TRMM Quality and Accounting Capsule
        (QAC) and Missing Data Unit List (MDUL) is not simulated. It is
        not useful for the user to input real QAC and MDUL data
        either, since the L0 read access tools do not process it.


EXAMPLES:
	Generate a CERES L0 science telemetry file named 
        TRMM_G0088_1997-12-01T00:00:00Z_V01.dataset_01,
	containing 3 packets of different lengths, starting at midnight
	Dec. 1, 1997 and spaced at 6.6 second intervals;
	also add QAC and MDUL data, filled with zeroes. 

  C:
	#define N 3
	PGSt_tag     spacecraftTag = PGSd_TRMM;
	PGSt_integer appID[N] = {54,54,54};
	PGSt_integer firstPacketNum = 1;
	char         *startUTC = "1997-12-01T00:00:00"; 
	PGSt_integer numValues = N;
	PGSt_double  timeInterval = 6.6;
	PGSt_integer dataLength[N];
	PGSt_integer otherFlags[2];
	char         *filename 
                        = "TRMM_G0088_1997-12-01T00:00:00Z_V01.dataset_01"; 
	char         appData[9000];
	PGSt_uinteger qualMissLen[2]={28,16}; 
	char         *qualData=NULL; 
	char         *missData=NULL; 

	PGSt_SMF_status returnStatus; 

	otherFlags[0] = 68; ** Redundant Data Deleted & RS Decoding **
	otherFlags[1] =  1; ** Routine production data **

	** Set lengths of packet application data **
	dataLength[0] = 2000;
	dataLength[1] = 3000;
	dataLength[2] = 4000;

        ** Fill appData buffer as desired here.
           Do not include packet header data -- it is filled by this tool.
           Fill first 2000 bytes with first packet data,
                 next 3000 bytes with second packet data,
                 last 4000 bytes with third packet data **


        ** Create simulated file **

	returnStatus = 
	  PGS_IO_L0_File_Sim(
                              spacecraftTag,    
                              appID,            
                              firstPacketNum,  
                              startUTC,         
			      numValues,         
                              timeInterval,     
                              dataLength,       
                              otherFlags,       
                              filename,         
                              appData,         
                              qualMissLen,         
                              qualData,         
                              missData,         
                              );            
  FORTRAN:
	integer pgs_io_l0_file_sim

	integer spacecrafttag
	integer appid(3)
	integer firstpacketnum
	character*27 startutc
	integer numvalues
	double precision timeinterval
	integer datalength(3)
	integer otherflags(2)
	character*256 filename
	character*9000 appdata
	integer qualmisslen(2)
	character*28 qualdata
	character*16 missdata

	integer returnstatus

        spacecrafttag = pgsd_trmm
	appid(1) = 54
	appid(2) = 54
	appid(3) = 54
	firstpacketnum = 1
	startutc = '1994-12-31T12:00:00.000000' 
	numvalues = 3
	timeinterval = 6.6
C Set lengths of packet application data
	datalength(1) = 2000
	datalength(2) = 3000
	datalength(3) = 4000
C Fill data to write to file header
	otherflags(1) = 68 ! Redundant Data Deleted & RS Decoding
	otherflags(2) =  1 ! Routine production data
	filename = 'TRMM_G0088_1997-12-01T00:00:00Z_V01.dataset_01'
	qualmisslen(1) = 28
	qualmisslen(2) = 16

C Fill appData buffer as desired here.
C Do not include packet header data -- it is filled by this tool.
C Fill first 2000 bytes with first packet data,
C       next 3000 bytes with second packet data,
C       last 4000 bytes with third packet data 


C Create simulated file

	returnstatus = pgs_io_l0_file_sim(
       +                      spacecrafttag,    
       +                      appid,            
       +                      firstpacketnum,  
       +                      startutc,         
       +		      numvalues,         
       +                      timeinterval,     
       +                      datalength,       
       +                      filename,         
       +                      otherflags         
       +                      filename,         
       +                      appdata,         
       +                      qualmisslen,         
       +                      qualdata,         
       +                      missdata)            



REQUIREMENTS:
	There is no SDP Toolkit requirement for this functionality. 
	This tool was created to support internal ECS SDP Toolkit development 
	and testing, and it is being provided as a service to the user.

DETAILS:
	None

GLOBALS:
	None

FILES:
        This tool creates the data file specified by the parameter filename.
        For TRMM, a Detached SFDU Header file is also created.

FUNCTIONS CALLED:
	PGS_MEM_Calloc
	PGS_MEM_Free
	PGS_SMF_SetUnknownMsg
	PGS_SMF_SetStaticMsg
	PGS_TD_UTCtoTAI
	PGS_TD_TAItoUTC
	PGS_TD_UTCtoTRMM
	PGS_TD_UTCtoEOSAM
	PGS_TD_UTCtoEOSPMGIIS
	PGS_TD_UTCtoEOSPMGIRD
        PGS_TD_UTCtoEOSAURAGIRD

END_PROLOG
*******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <PGS_math.h>
#include <cfortran.h>
#include <PGS_IO.h>
#include <PGS_TD.h>


PGSt_SMF_status
PGS_IO_L0_File_Sim(    
    PGSt_tag         spacecraftTag,  /* unique s/c ID */
    PGSt_integer     appID[],        /* Application Process ID list */
                                     /* one entry for each packet */
                                     /* all the same for Science APID file */
    PGSt_integer     firstPacketNum, /* packet number of first packet */
    char             startUTC[28],   /* time of first packet */
    PGSt_integer     numValues,      /* number of packets requested */
    PGSt_double      timeInterval,   /* time interval between packets */
    PGSt_integer     dataLength[],   /* length of Application Data (bytes)
                                        one entry for each packet */
    PGSt_integer     otherFlags[],   /* Processing options and Data Type Flags:
					Processing options is a single byte with
					possible "on bits" of 3, 6, 7.  This
					corresponds to possible integer values
					of: 0, 4, 32, 36, 64, 68, 96 and 100.
					Data Type Flags has a value of either:
					1 (Routine Production Data) or 2
					(Quicklook Data).  Only TRMM may have
					Quicklook Data.  */
    char             *filename,      /* name of file to write data to */
    void             *appData,       /* Application Data
                                        if NULL, does zero-fill */

    /* remaining arguments are for TRMM footer processing only;
       they are ignored for other platforms */
    
    PGSt_uinteger     qualMissLen[2], /* [0]:length of Quality Data (bytes)
                                        if qualLength=0, only qualLength
                                        and missLength=0 are written to file
					[1]:length of Missing Data Unit List (bytes)
					if missLength=0, only missLength
					is written to file */
    void             *qualData,      /* Quality Data (QAC) or NULL
                                        if NULL, does zero-fill */
    void             *missData)      /* Missing Data Unit List (MDUL) or NULL
                                        if missLength=0, does zero-fill */

{
    FILE             *pktFile;       /* pointer to file */

    PGSt_IO_L0_PriPktHdr       primaryHeader={{8,0},{0,0},{0,0}};
    PGSt_IO_L0_SecPktHdrTRMM   trmmSecHdr;
    PGSt_IO_L0_SecPktHdrEOS_AM eosamSecHdr;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIIS eospmSecHdr1;
    PGSt_IO_L0_SecPktHdrEOS_PM_GIRD eospmSecHdr2;
    PGSt_IO_L0_SecPktHdrEOS_AURA eosauraSecHdr;
    PGSt_IO_L0_SecPktHdrADEOS_II adeosIISecHdr;

    PGSt_IO_L0_FileHdrTRMM     trmmFileHdr;
    PGSt_IO_L0_FileHdrADEOS_II adeosIIFileHdr;
    
    PGSt_double      startTAI93;     /* initial packet time in TAI */
    PGSt_double      beginTAI93;     /* initial packet time in TAI */
    PGSt_double      endTAI93;       /* initial packet time in TAI */
    PGSt_double      secTAI93;       /* current packet time in TAI */
    PGSt_double      jdUTC[2];
    
    PGSt_integer     count;          /* loop counter */
    PGSt_integer     maxLength;
    PGSt_integer     maxValues;
    PGSt_integer     packetLength;
    PGSt_integer     packetNum;
    PGSt_integer     varHeaderLength;
    PGSt_integer     fileHeaderSize;
    PGSt_integer     numAppIDs;
    PGSt_integer     sfduAppID;
    
    PGSt_integer     *sortedIndex;
    PGSt_integer     *uniqueAppIDs;
    
    PGSt_boolean     onLeap;
    
    PGSt_uinteger    totalLength;
    PGSt_uinteger    offsetQAC;
    PGSt_uinteger    qualLength;    /* length of Quality Data (bytes)
                                       if qualLength=0, only qualLength
                                       and missLength=0 are written to file */
    PGSt_uinteger    missLength;    /* length of Missing Data Unit List (bytes)
                                       if missLength=0, only missLength
                                       is written to file */
    
    size_t           primaryHdrSize; /* size of primary header */
    size_t           secHdrSize;     /* size of secondary header */

    
    void             *secHdrPtr;     /* pointer to secondary header */
    
    unsigned char    packetCount[4]; /* Packet count in char */

    unsigned int     orbit_time;     /* ADEOS-II orbit time */
    char             *dataPtr;       /* pointer to Application Data */
    char             asciiUTC[28];   /* current packet time in UTC */
    char             beginUTC[28];   /* current packet time in UTC */
    char             endUTC[28];     /* current packet time in UTC */
    char             sfdu_file_name[100];
    char             sfdu_type_flag;
    char             *dotPtr;
    
    unsigned char    staticPriPktHdr; /* static part of primary pkt header
					including Version Number, Type, Sec. Hdr
					Flag */
    PGSt_boolean     noAppData;      /* true if no Application Data has been
					supplied */

    unsigned char    qualLenChar[4]; /* Qual buffer length in char */
    char             *qualPtr;       /* pointer to Quality Data */
    unsigned char    missLenChar[4]; /* Missing Data list buffer length 
                                          in char */
    char             *missPtr;       /* pointer to Missing Data Unit list
                                          (MDUL) */

    PGSt_SMF_status  returnStatus;   /* function return status */
    
    /* intialize some variables, this is here because the calling sequence
       changed and it has been deemed easier to do this than actually fix the
       code */

    qualLength = qualMissLen[0];
    missLength = qualMissLen[1];
    
    /* Make sure numValues is not less than zero.  If numValues is equal to zero
       it will be treated as a request for one value (this is for compatible
       behavior with the rest of the PGS Toolkit). */

    if (numValues < 0)
      return PGSIO_E_L0_BAD_NUM_PKTS;
    else
      maxValues = (numValues) ? numValues : 1;

    /* The Application Process ID field is 11 bits so the largest this value can
       be is 2^11 - 1 (i.e. 2047) and of course it cannot be negative. */

    for(count=0;count<maxValues;count++)
    {
        if (appID[count] < 0 || appID[count] > 2047)
        return PGSIO_E_L0_BAD_APP_ID;
    }

    /* The Packet Sequence Count field is 14 bits so the largest this value can
       be is 2^14 - 1 (i.e. 16383) and of course it cannot be negative.  The
       Packet Sequence Count field will start at the value of firstPacketNum and
       have a maximum value of firstPacketNum+maxValues-1 so check these
       boundary areas. */

    if (firstPacketNum <0 || firstPacketNum + maxValues - 1 > 16383)
      return PGSIO_E_L0_BAD_FIRST_PKTNUM;
    
    /* Check for valid spacecraft ID tag.  If a valid tag is found set the
       secondary header pointer (secHdrPtr) to point to the address of the
       appropriate structure.  Also get the size of this structure. */

    switch (spacecraftTag)
    {
      case PGSd_TRMM:
	secHdrSize = PGSd_IO_L0_SecPktHdrSizeTRMM;
	secHdrPtr = (void*) &trmmSecHdr;
	break;
      case PGSd_EOS_AM:
	secHdrSize = PGSd_IO_L0_SecPktHdrSizeEOS_AM;
	secHdrPtr = (void*) &eosamSecHdr;
	break;
      case PGSd_EOS_PM_GIIS:
	secHdrSize = PGSd_IO_L0_SecPktHdrSizeEOS_PM;
	secHdrPtr = (void*) &eospmSecHdr1;
	break;
      case PGSd_EOS_PM_GIRD:
	secHdrSize = PGSd_IO_L0_SecPktHdrSizeEOS_PM;
	secHdrPtr = (void*) &eospmSecHdr2;
	break;
      case PGSd_EOS_AURA:
        secHdrSize = PGSd_IO_L0_SecPktHdrSizeEOS_AURA;
        secHdrPtr = (void*) &eosauraSecHdr;
        break;
      case PGSd_ADEOS_II:
	secHdrSize = PGSd_IO_L0_SecPktHdrSizeADEOS_II;
	secHdrPtr = (void*) &adeosIISecHdr;
	break;
      default:
	return PGSTD_E_SC_TAG_UNKNOWN;
    }
    
    sortedIndex = NULL;
    PGS_MEM_Malloc((void **) &sortedIndex,sizeof(PGSt_integer)*maxValues*2);
    uniqueAppIDs = sortedIndex+maxValues;
    PGS_IO_L0_sortArrayIndices(appID,maxValues,sortedIndex);
    
    *uniqueAppIDs = appID[sortedIndex[0]];
    numAppIDs = 1;
    if (*uniqueAppIDs > 2047)
      return  PGSIO_E_L0_BAD_APP_ID;
    for (count=1; count<maxValues; count++)
    {
	if (appID[sortedIndex[count]] > appID[sortedIndex[count-1]])
	{
	    *(uniqueAppIDs+numAppIDs) = appID[sortedIndex[count]];
	    if (*uniqueAppIDs > 2047)
	      return  PGSIO_E_L0_BAD_APP_ID;
	    numAppIDs++;
	}
    }
    if (numAppIDs > 255)
      return PGSIO_E_L0_BAD_NUM_APP_IDS;
    
    /* The Packet Length field is 16 bits so the largest this value can be is
       2^16 - 1 (i.e. 65535).  The Packet Length is the total size in bytes of
       the secondary header and the Application Data.  The variable packetLength
       specifies the length in bytes of the Application Data field of each
       packet so this number should not be negative. */

    totalLength = 0;
    maxLength = 0;
    for (count=0; count<maxValues; count++)
      if (dataLength[count] < 0)
	break;
      else
      {
	  maxLength = (dataLength[count] > maxLength) ? 
	    dataLength[count] : maxLength;
	  totalLength = totalLength + dataLength[count];
      }
    
    
    /* Check that the Packet Length field is not too large (see comment above)
       and make sure that the loop was not terminated prematurely (i.e. count is
       less than maxValues) which will happen if a negative value has
       been specified for a packetLength. */
    
    if (maxLength > 65535-secHdrSize || count < maxValues)
      return PGSIO_E_L0_BAD_DATA_LENGTH;
    
    /* Intialize some variables:  
          primaryHdrSize -- static size of the Primary Header Structure 
          dataPtr is initialy set to point to the beginning of the Application
	  Data and later will be incremented to point to the Application
	  Data for each respective packet */

    primaryHdrSize = PGSd_IO_L0_PrimaryPktHdrSize;
    dataPtr = (char *) appData;
	
    /* The offset in byes to the QAC is the total size in bytes of all the
       packets.  The size of each packet is the sum of the the sizes of the
       primary header, the secondary header and the application data.  The total
       size should not be greater than 2^32 - 1 since the size in stored a
       four bytes. */

    offsetQAC = totalLength + maxValues * (primaryHdrSize + secHdrSize);
    
    /* Convert the input reference time (startUTC) to PGS Toolkit internal time
       format (TAI seconds since 12 AM UTC 1-1-93). */

    returnStatus = PGS_TD_UTCtoTAI(startUTC,&startTAI93);
    switch (returnStatus)
    {
      case PGS_S_SUCCESS:
      case PGSTD_W_PRED_LEAPS:
      case PGSTD_E_NO_LEAP_SECS:
	break;
      case PGSTD_E_TIME_FMT_ERROR:
      case PGSTD_E_TIME_VALUE_ERROR:
      case PGS_E_TOOLKIT:
	return returnStatus;
      default:
	PGS_SMF_SetUnknownMsg(returnStatus,"PGS_IO_L0_File_Sim()");
	return PGS_E_TOOLKIT;
    }
	
    /* The following assignments set the static part of primaryHeader.packetID
       as follows:

       bits  1- 3: Version Number -- always 000 for TRMM, EOS AM, EOS PM
       bit      4: Type           -- always   0 for TRMM, EOS AM, EOS PM
       bit      5: Sec hdr flag   -- always   1 for TRMM, EOS AM, EOS PM
       bits  6-8: part of the Application Process ID (AppID), are assigned
          below for each packet

       Binary 00001000 is decimal 8
    */

    staticPriPktHdr = (unsigned char) 8;

    /* Check to see if a null pointer was passed in for the Application Data.
       If so the malloc a block of memory the size of maxLength (as determined
       above), initialize it to all zeros and use this to write out the
       Application Data portion of the packets. */

    if (dataPtr == (char*) NULL)
    {
	noAppData = PGS_TRUE;
	returnStatus = PGS_MEM_Calloc( (void **) &dataPtr,maxLength,1);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	    break;
	  case PGS_E_UNIX:
	  case PGSMEM_E_MAXSIZE:
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_IO_L0_File_Sim()");
	    return PGS_E_TOOLKIT;
	}
    }
    else
      noAppData = PGS_FALSE;

    /*
     * Open file specified by "filename". 
     * Clobber existing file, if any.
     */

    pktFile = fopen(filename,"w");
    if (pktFile == (FILE*) NULL)
    {
	PGS_SMF_SetStaticMsg(PGSIO_E_L0_PHYSICAL_OPEN,"PGS_IO_L0_File_Sim()");
	return PGSIO_E_L0_PHYSICAL_OPEN;
    }

    /* Write the file header to the file. */

    /* the packet count is the number of packets in the file and should be the
       same as maxValues */
    
    packetCount[0] = (unsigned char) (maxValues/16777216UL);
    packetCount[1] = (unsigned char) ((maxValues/65536UL)%256UL);
    packetCount[2] = (unsigned char) ((maxValues/256UL)%256UL);
    packetCount[3] = (unsigned char) (maxValues%256UL);
    
    /* determine the start and stop times of the packets in the file */

    beginTAI93 = startTAI93;
    returnStatus = PGS_TD_TAItoUTC(beginTAI93,beginUTC);
    endTAI93 = startTAI93+(maxValues-1)*timeInterval;
    returnStatus = PGS_TD_TAItoUTC(endTAI93,endUTC);

    /* the format of the file header depends on the spacecraft */

    switch (spacecraftTag)
    {
      case PGSd_TRMM:
	trmmFileHdr.spacecraftID[0] = 0;
	trmmFileHdr.spacecraftID[1] = 0x6b;
	trmmFileHdr.spare1 = 0;
	trmmFileHdr.spare2 = 0;
	trmmFileHdr.processingOptions = (unsigned char) (otherFlags[0]%256);
	trmmFileHdr.dataTypeFlags = (unsigned char) (otherFlags[1]%256);
	trmmFileHdr.spare3 = 0;
	trmmFileHdr.spare4 = 0;
	trmmFileHdr.spare5 = 0;
	trmmFileHdr.selectOptions = 2;
	memcpy((char*) trmmFileHdr.packetCount, (char*) packetCount,
	       sizeof(packetCount));
	returnStatus = PGS_TD_UTCtoUTCjd(beginUTC,jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	  onLeap = PGS_TRUE;
	else 
	  onLeap = PGS_FALSE;
	returnStatus = PGS_TD_UTCjdtoPB5(jdUTC,onLeap,
					 trmmFileHdr.spacecraftClockFirst);
	returnStatus = PGS_TD_UTCtoUTCjd(endUTC,jdUTC);
	if (returnStatus == PGSTD_M_LEAP_SEC_IGNORED)
	  onLeap = PGS_TRUE;
	else
	  onLeap = PGS_FALSE;
	returnStatus = PGS_TD_UTCjdtoPB5(jdUTC,onLeap,
					 trmmFileHdr.spacecraftClockLast);
	memcpy((char*) trmmFileHdr.timeOfReceipt, 
	       (char*) trmmFileHdr.spacecraftClockLast,7);
	trmmFileHdr.numAPID = (unsigned char) numAppIDs;
	for (count=0;count<numAppIDs;count++)
	{
	    trmmFileHdr.varLenBuf[2*count] = (unsigned char) 
	                                     (uniqueAppIDs[count]/256U);
	    trmmFileHdr.varLenBuf[2*count+1] = (unsigned char) 
	                                       (uniqueAppIDs[count]%256U);
	}
	count--;
	trmmFileHdr.varLenBuf[2*count+2] = 0;
	trmmFileHdr.varLenBuf[2*count+3] = 1U;
	trmmFileHdr.varLenBuf[2*count+4] = (unsigned char) (offsetQAC/
							    16777216UL);
	trmmFileHdr.varLenBuf[2*count+5] = (unsigned char) ((offsetQAC/
							     65536UL)%256UL);
	trmmFileHdr.varLenBuf[2*count+6] = (unsigned char) ((offsetQAC/256UL)
							    %256UL);
	trmmFileHdr.varLenBuf[2*count+7] = (unsigned char) (offsetQAC%256UL);
	varHeaderLength = 6 + 2*trmmFileHdr.numAPID;
	fileHeaderSize = sizeof(PGSt_IO_L0_FileHdrTRMM) - TRMM_HDR_VAR_LEN + 
	                 varHeaderLength;
	fwrite(&trmmFileHdr,fileHeaderSize,1,pktFile);
	break;

      case PGSd_EOS_AM:
	break;
      case PGSd_EOS_PM_GIIS:
	break;
      case PGSd_EOS_PM_GIRD:
	break;
      case PGSd_EOS_AURA:
        break;
	
      case PGSd_ADEOS_II:
	memcpy((char*) adeosIIFileHdr.packetCount, (char*) packetCount,
	       sizeof(packetCount));
	returnStatus = PGS_TD_UTCtoADEOSII(beginUTC,
					   adeosIIFileHdr.spacecraftClockFirst);
	returnStatus = PGS_TD_UTCtoADEOSII(endUTC,
					   adeosIIFileHdr.spacecraftClockLast);
	fwrite(&adeosIIFileHdr,PGSd_IO_L0_FileHdrSizeADEOS_II,1,pktFile);

	break;

      default:
	return PGSTD_E_SC_TAG_UNKNOWN;
    } /* END switch (spacecraftTag) */
    
    /* Write the packets to the file (one file containing "maxValues"
       packets). */

    for (count=0;count<maxValues;count++)
    {

        /* Define Application ID part of packet ID field  */

	primaryHeader.packetID[0] = 
           staticPriPktHdr | (unsigned char) (appID[count]/256U);
	primaryHeader.packetID[1] = (unsigned char) (appID[count]%256U);

	/* Increment the Packet Sequence Count field 
	 * and calculate the Packet Length 
         */

        /*
         * IMPORTANT TK4 PROTOTYPE NOTE (29-Dec-1994): 
         * Packet Length is defined in section 4.2.6 of the white paper,
         * "Level 0 Data Issues for the ECS Project" (Sept 1994), as: 
         * "the length of the entire packet,  in bytes, less the length  
         * of the primary packet header (6 bytes) less one byte".
         * 
         * This is equivalent to the total size in bytes of Secondary 
	 * Header plus the Application Data minus 1. 
         * 
         */

	packetNum = firstPacketNum + count;
	primaryHeader.pktSeqCntl[0] = (unsigned char) (packetNum/256);
	primaryHeader.pktSeqCntl[1] = (unsigned char) (packetNum%256);
	packetLength = dataLength[count] + secHdrSize - 1;
	primaryHeader.pktLength[0] = (unsigned char) (packetLength/256);
	primaryHeader.pktLength[1] = (unsigned char) (packetLength%256);
	
	if (spacecraftTag == PGSd_ADEOS_II)
	{
	    primaryHeader.pktSeqCntl[0] = primaryHeader.pktSeqCntl[0] | 192U;
	    
	}
	
	/* Increment the internal time by "timeInterval" seconds and convert to
	   the equivalent ASCII time.  The internal time is converted to ASCII
	   time here simply because the functions to convert from ASCII to s/c
	   time already exist and there are no analogous functions to convert
	   from internal time to s/c time. */

	secTAI93 = startTAI93 + count*timeInterval;
	returnStatus = PGS_TD_TAItoUTC(secTAI93,asciiUTC);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_W_PRED_LEAPS:
	  case PGSTD_E_NO_LEAP_SECS:
	    break;
	  case PGS_E_TOOLKIT:
	    return returnStatus;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_IO_L0_File_Sim()");
	    return PGS_E_TOOLKIT;
	}

	/* Call the appropriate ASCII UTC to s/c clock time conversion
	   function. */

	switch (spacecraftTag)
	{
	  case PGSd_TRMM:
	    returnStatus = PGS_TD_UTCtoTRMM(asciiUTC,trmmSecHdr.scTime);
	    break;
	  case PGSd_EOS_AM:
	    returnStatus = PGS_TD_UTCtoEOSAM(asciiUTC,eosamSecHdr.scTime);
	    break;
	  case PGSd_EOS_PM_GIIS:
	    returnStatus = PGS_TD_UTCtoEOSPMGIIS(asciiUTC,eospmSecHdr1.scTime);
	    break;
	  case PGSd_EOS_PM_GIRD:
	    returnStatus = PGS_TD_UTCtoEOSPMGIRD(asciiUTC,eospmSecHdr2.scTime);
	    break;
	  case PGSd_EOS_AURA:
            returnStatus = PGS_TD_UTCtoEOSAURAGIRD(asciiUTC,eosauraSecHdr.scTime);
            break;
          case PGSd_ADEOS_II:
	    returnStatus = PGS_TD_UTCtoADEOSII(asciiUTC,
					       adeosIISecHdr.scTime);
	    orbit_time = (unsigned int) (fmod(secTAI93, 101.0)*32.0);
	    adeosIISecHdr.orbitTime[0] = (PGSt_scTime) (orbit_time/16777216U);
	    adeosIISecHdr.orbitTime[1] = (PGSt_scTime)
		                         ((orbit_time/65536U)%256U);
	    adeosIISecHdr.orbitTime[2] = (PGSt_scTime) ((orbit_time/256U)%256U);
	    adeosIISecHdr.orbitTime[3] = (PGSt_scTime) (orbit_time%256U);

	    break;

	  default:
	    return PGS_E_TOOLKIT;
	}
	
	/* Check return value from call to conversion routine above. */

	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:
	  case PGSTD_W_PRED_LEAPS:
	  case PGSTD_E_NO_LEAP_SECS:
	    break;
	  case PGSTD_E_DATE_OUT_OF_RANGE:
	    return returnStatus;
	  case PGS_E_TOOLKIT:
	    return PGS_E_TOOLKIT;
	  default:
	    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_IO_L0_File_Sim()");
	    return PGS_E_TOOLKIT;
	}
	
	/* Write the new packet to the file.  Each packet consists of a Primary
	   Header, a Secondary Header and an Application Data block. */

	fwrite(&primaryHeader,primaryHdrSize,1,pktFile);
	fwrite(secHdrPtr,secHdrSize,1,pktFile);
	fwrite(dataPtr,dataLength[count],1,pktFile);

	/* Increment the data pointer to point to the Application Data for the
	   next packet.  This is only done if the Application Data was passed in
	   the calling sequence.  If a null pointer was passed in the calling
	   sequence there is no need to increment the pointer since the same
	   (all zero) space is used for each packet. */

	if (noAppData == PGS_FALSE)
	  dataPtr += dataLength[count];
    }



    /* Footer processing:
       Includes quality/accounting (QAC) and Missing Data Unit List (MDUL)
       used for TRMM only */

    if( spacecraftTag == PGSd_TRMM)
    {
	
	/* Write the length of the quality buffer to the file. */
	
	qualLenChar[0] = (unsigned char) (qualLength/16777216UL);
	qualLenChar[1] = (unsigned char) ((qualLength/65536UL)%256UL);
	qualLenChar[2] = (unsigned char) ((qualLength/256UL)%256UL);
	qualLenChar[3] = (unsigned char) (qualLength%256UL);
	fwrite(qualLenChar,sizeof(unsigned char),4,pktFile);
	
	/* Skip writing qual data if desired */
	
	if (qualLength > 0)
	{
	    
	    /* Check to see if a null pointer was passed in for the Quality Data.
	       If so the malloc a block of memory the size of qualLength,
	       initialize it to all zeros and use this to write out the
	       Quality Data portion of the footer. */
	    
	    qualPtr = (char *) qualData;
	    if (qualPtr == (char*) NULL)
	    {
		returnStatus = PGS_MEM_Calloc( (void **) &qualPtr,qualLength,1);
		switch (returnStatus)
		{
		  case PGS_S_SUCCESS:
		    break;
		  case PGS_E_UNIX:
		  case PGSMEM_E_MAXSIZE:
		    return returnStatus;
		  default:
		    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_IO_L0_File_Sim()");
		    return PGS_E_TOOLKIT;
		}
		fwrite(qualPtr,qualLength,1,pktFile);
		PGS_MEM_Free(qualPtr);
	    }
	    else
	      fwrite(qualPtr,qualLength,1,pktFile);
	    
	}
	
	/* Write the length of the Missing Data Unit list buffer to the file. */
	
	missLenChar[0] = (unsigned char) (missLength/16777216UL);
	missLenChar[1] = (unsigned char) ((missLength/65536UL)%256UL);
	missLenChar[2] = (unsigned char) ((missLength/256UL)%256UL);
	missLenChar[3] = (unsigned char) (missLength%256UL);
	fwrite(missLenChar,sizeof(unsigned char),4,pktFile);
	
	/* Skip writing MDUL data if desired */
	/* MDUL data ignored if no qual data */
	
	if( (qualLength > 0) && (missLength > 0) )
	{
	    
	    /* Check to see if a null pointer was passed in for the MDUL Data.
	       If so the malloc a block of memory the size of missLength,
	       initialize it to all zeros and use this to write out the
	       MDUL portion of the footer. */
	    
	    missPtr = (char *) missData;
	    if (missPtr == (char*) NULL)
	    {
		returnStatus = PGS_MEM_Calloc( (void **) &missPtr,missLength,1);
		switch (returnStatus)
		{
		  case PGS_S_SUCCESS:
		    break;
		  case PGS_E_UNIX:
		  case PGSMEM_E_MAXSIZE:
		    return returnStatus;
		  default:
		    PGS_SMF_SetUnknownMsg(returnStatus,"PGS_IO_L0_File_Sim()");
		    return PGS_E_TOOLKIT;
		}
		fwrite(missPtr,missLength,1,pktFile);
		PGS_MEM_Free(missPtr);
	    }
	    else
	      fwrite(missPtr,missLength,1,pktFile);
	}
	
    }  /* end TRMM footer processing */

    /* Write SFDU detached header file. */

    if (spacecraftTag == PGSd_TRMM)
    {
	if (otherFlags[1] == 2)
	  sfdu_type_flag = (char) 2;
	else
	  sfdu_type_flag = (char) 1;
	
	if (numAppIDs == 1)
	  sfduAppID = appID[0];
	else 
	  sfduAppID = 1;
	
	strcpy(sfdu_file_name,filename);
	if ((dotPtr=strstr(sfdu_file_name,".DATASET_01")) == NULL)
	  strcat(sfdu_file_name,".SFDU");
	else 
	  strcpy(dotPtr,".SFDU_01");
	
	returnStatus = PGS_IO_L0_SFDU_Sim(spacecraftTag,filename,
					  sfdu_file_name,sfduAppID,beginUTC,
					  endUTC,sfdu_type_flag);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    fclose(pktFile);
	    if (noAppData == PGS_TRUE)
		PGS_MEM_Free(dataPtr);
	    return returnStatus;
	}
    }
    
    
    fclose(pktFile);
    if (noAppData == PGS_TRUE)
      PGS_MEM_Free(dataPtr);
    return PGS_S_SUCCESS;
}
