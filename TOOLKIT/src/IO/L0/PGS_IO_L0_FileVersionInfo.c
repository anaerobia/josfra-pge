/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_FileVersionInfo.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to get file version 
  information and place in version table entry.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Tom W. Atwater / Applied Research Corp. 
  Guru Tej S. Khalsa / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
               MS  Designed
  30-Jan-1995 TWA  Initial version
  20-Jul-1995 GTSK Defined constants to be unsigned long type.  Altered 
                   version_entry structure members' values as follows: for
		   EOS_AM and EOS_PM set var_hdr_start to the value of
		   pkts_start and footer_start to point to the value of
		   file_size.  The values of var_hdr_size and footer_size in
		   these cases is set to 0.
  06-Jul-1999 RM   updated for thread-safe functionality 
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  05-Dec-2000 AT   Changed :
                   if(version_entry->start_time == start_stop[count][0])
                   to:
                   if(fabs(version_entry->start_time - start_stop[count][0])
                                            <=PGSd_TD_GIRD_GIIS_EPSILON)
                   for the PGSd_EOS_PM_GIRD case to aviod problems resulting 
                   from the rounding (or truncation) of the last
                   digit in the microsecond field of  start_stop time.
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>
#include <PGS_TSF.h>
#include <math.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Get file version information and place in version table entry
	
NAME:
	PGS_IO_L0_FileVersionInfo
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_FileVersionInfo(
		PGSt_PC_Logical		file_logical,
		PGSt_integer		file_version,
		PGSt_tag		spacecraft_tag,
		PGSt_IO_L0_VersionTable	*version_entry
		)
  FORTRAN:
	N/A

DESCRIPTION:
	This tool reads the spacecraft-dependent header from the unique physical
	file specified by file_logical and file_version.  It determines the file
	start time, stop time and packet count.  These values are then stored in
	the file version table entry.

INPUTS:
	file_logical - 	The logical file descriptor for this virtual data 
		set.

	file_version -	The version number number of the specified physical file
	spacecraft_tag - The tag identifying which of the supported spacecraft
		         platforms generated this virtual data set.
			 The following types are currently defined:

			PGSd_EOS_AM  	    the EOS AM platform
			PGSd_EOS_PM_GIIS    the EOS PM platform (GIIS)
			PGSd_EOS_PM_GIRD    the EOS PM platform (GIRD)
			PGSd_TRMM    	    the TRMM platform
			PGSd_ADEOS_II  	    the ADEOS-II platform

OUTPUTS:
	version_entry - The version table entry where the version number, 
		packet count, and start and stop times for the specified 
		physical file are stored.

NOTES:
	This is a low-level tool.  It is not intended to be called directly by 
	any user code.

        IMPORTANT TK4 PROTOTYPE NOTES (28-Dec-1994):
        ============================================
        The prototype version of the Level 0 tools uses fake file header
        formats because the real formats were not available in time for
        use in the code.  The file headers read by this tool use these
        fake file header formats.   The code which is used to determine
	file timestamps is likely to change in the next release.

EXAMPLES:
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_W_L0_PHYSICAL_CLOSE
	PGSIO_E_L0_PHYSICAL_OPEN
	PGSTSF_E_GENERAL_FAILURE
	
REQUIREMENTS:
	PGSTK-0200, 0240

DETAILS:
	This tool will call PGS_IO_Gen_Open to open the specified physical file.
	It will then read the spacecraft-dependent file header and extract the 
	needed values.  The specified physical file is then closed via a call
	to PGS_IO_Gen_Close.
        Tool assumes high-order byte is first when converting byte data 
        to integer.

GLOBALS:
        None

FILES:
        Physical files for the specified virtual data set
        Process Control File (PCF)

FUNCTIONS CALLED:
	PGS_IO_Gen_Open
	PGS_IO_Gen_Close
	PGS_IO_L0_TRMM_HdrInfo
	PGS_SMF_SetStaticMsg
	PGS_SMF_TestStatusLevel
	PGS_SMF_TestErrorLevel
	PGS_TSF_GetTSFMaster

	IMPORTANT TK4 PROTOTYPE NOTE (28-Dec-1994):
	The following tools are used in the TK4 prototype to get file start 
	and stop times.  Since the actual time formats are TBD, these calls 
	will probably be removed or replaced in future versions:

	PGS_TD_EOSAMtoTAI
	PGS_TD_EOSPMtoTAI

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_FileVersionInfo(
	PGSt_PC_Logical		file_logical,
	PGSt_integer		file_version,
	PGSt_tag		spacecraft_tag,
	PGSt_IO_L0_VersionTable	*version_entry
	)
{
    static char *toolname = "PGS_IO_L0_FileVersionInfo()";
    
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */

    PGSt_IO_Gen_FileHandle *open_fs;
    PGSt_IO_Gen_FileHandle *open_hdr;

    unsigned char*                    file_hdr_EOS_AM;  /* file header */
    unsigned char*                    file_hdr_EOS_PM_GIRD;  /* file header */
    unsigned char*                    file_hdr_EOS_PM_GIIS;  /* file header */
    unsigned char*                    file_hdr_EOS_AURA;  /* file header */
#ifdef _PGS_THREADSAFE
    /* non static for TSF */
    PGSt_IO_L0_FileHdrADEOS_II *file_hdr_ADEOS_II;/* file header, fixed */
#else
    static PGSt_IO_L0_FileHdrADEOS_II file_hdr_ADEOS_II;/* file header, fixed */
#endif

    PGSt_integer             count;
    PGSt_integer             num_files;

    long                     unixerror;              /* retains errno value */

    char                     *ptr;
    int                      i;

    unsigned long            hdr_struct_size;        /* Size of header struct */
    unsigned long            file_size;
    
    long                     offset;

    PGSt_double  (*start_stop)[2];

#ifdef _PGS_THREADSAFE
    /* 
     *    Set up to get TSDs 
     */
    PGSt_TSF_MasterStruct *masterTSF;

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* 
     *    Get the TSDs and load into our non-statics vars
     */
    file_hdr_ADEOS_II = (PGSt_IO_L0_FileHdrADEOS_II *) pthread_getspecific(
                             masterTSF->keyArray[PGSd_TSF_KEYIOL0FILEHDRADEOSII]);
#endif


    /* 
     *    Open the specified physical file
     */

    returnStatus = PGS_IO_Gen_Open(
				   file_logical, 
				   PGSd_IO_Gen_Read,
				   &open_fs,
				   file_version);
    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }


    /* 
     *    Put the file version number into the version table entry.
     */

    version_entry->file_version = file_version;


    /* 
     *    Read the appropriate spacecraft-dependent file header and 
     *    extract the following values:
     *    
     *        start time
     *        stop time
     *        packet count
     *        start byte of 1st  packet
     *        end   byte of last packet
     *        plus other values (TRMM only)
     *    
     *    Use them to populate the version table entry.
     */   
    /* 
     *    !!! IMPORTANT TK4 PROTOTYPE NOTES (28-Dec-1994): !!!
     *    !!! Current implementation is prototype only !!!
     *    !!! Implemented using dummy file headers pending availability of
     *    !!! full header information !!!
     *    !!! Code which determines start and stops times, as well as packet
     *    !!! count will probably change !!!
     */   
    switch (spacecraft_tag)
    {

      case PGSd_EOS_AM:
	
	if (file_version == 1)
	{
	    open_hdr = open_fs;
	}
	else
	{
	    returnStatus = PGS_IO_Gen_Open(file_logical,
					   PGSd_IO_Gen_Read,
					   &open_hdr,
					   1);
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	}
	
	fseek(open_hdr, 0L,  SEEK_END);
	
	hdr_struct_size = ftell(open_hdr);
	
	rewind(open_hdr);
	
	file_hdr_EOS_AM = (unsigned char*) calloc(hdr_struct_size, 1);

	if (file_hdr_EOS_AM == NULL)
	{
	    PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
	    PGS_IO_Gen_Close( open_fs );
	    return PGSIO_E_L0_MEM_ALLOC;
	}
	
        /* 
         *    Read in the file header
         */

        count = fread( file_hdr_EOS_AM, 1, hdr_struct_size, open_hdr);
        unixerror=errno;
        if (count != hdr_struct_size )                 /* incomplete read */
        {
	    free(file_hdr_EOS_AM);
            returnStatus = PGS_E_UNIX;
            PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	if (file_version != 1)
	{
	    returnStatus = PGS_IO_Gen_Close( open_hdr );
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		free(file_hdr_EOS_AM);
		returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	}

        /* 
         *    Get file  start time, stop time, packet count, 
         *    start byte location of first packet,
         *    end   byte location of file
         */

	offset = 50;
	
	offset = offset + 2 + 16*(256*file_hdr_EOS_AM[offset] +
				  file_hdr_EOS_AM[offset+1]) + 12;
	
        returnStatus = PGS_TD_EOSAMtoTAI(
            file_hdr_EOS_AM+offset,
            &(version_entry->start_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
	    free(file_hdr_EOS_AM);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	offset = offset + 8;
	
        returnStatus = PGS_TD_EOSAMtoTAI(
            file_hdr_EOS_AM+offset,
            &(version_entry->stop_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
	    free(file_hdr_EOS_AM);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	offset = offset + 28;
	
        /* Fill rest of file table data
           Note that much of the following is currently unused,
           since EOS_AM header is static length -- it is included
           here for consistency, and possible future use */

	if (file_version == 1)
	{
	    version_entry->packet_count = 
                (file_hdr_EOS_AM+offset)[3] + 256UL * (
		(file_hdr_EOS_AM+offset)[2] + 256UL * (
		(file_hdr_EOS_AM+offset)[1] + 256UL * (
        	(file_hdr_EOS_AM+offset)[0] )));

	    version_entry->stat_hdr_size = hdr_struct_size;
	    version_entry->var_hdr_start = version_entry->stat_hdr_size;
	    version_entry->var_hdr_size = 0UL;
	    version_entry->pkts_start = hdr_struct_size;
	    version_entry->file_size = hdr_struct_size;
	    
	    version_entry->pkts_size = 0UL;
	    version_entry->footer_start = version_entry->file_size;
	    version_entry->footer_size = 0UL;
	    
	}
	else
	{
	    fread(file_hdr_EOS_AM, 1, 
		  PGSd_IO_L0_PrimaryPktHdrSize + PGSd_IO_L0_SecPktHdrSizeEOS_AM,
		  open_fs);
	    
	    returnStatus = PGS_TD_EOSAMtoTAI((file_hdr_EOS_AM +
					      PGSd_IO_L0_PrimaryPktHdrSize),
					     &(version_entry->start_time) );
	    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
	    {
		free(file_hdr_EOS_AM);
		returnStatus = PGSIO_E_L0_TIME_CONVERSION;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	    
	    start_stop = (PGSt_double (*)[2])( calloc(255,
						      2*sizeof(PGSt_double)));
	    if (start_stop == (PGSt_double (*)[2]) NULL)
	    {
		PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
		return PGSIO_E_L0_MEM_ALLOC;
	    }
	    
	    returnStatus = PGS_IO_L0_GetEOSAMfileTimes(file_hdr_EOS_AM,
						       start_stop,
						       &num_files,
						       &file_size);
	    
	    if( returnStatus != PGS_S_SUCCESS )
	    {
		free(file_hdr_EOS_AM);
		free(start_stop);
		returnStatus = PGSIO_E_L0_TIME_CONVERSION;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	    
	    start_stop = (PGSt_double (*)[2]) realloc((void*) start_stop,
						      2*sizeof(PGSt_double)*
						      num_files);
	    
	    if (start_stop == (PGSt_double (*)[2]) NULL)
	    {
		PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
		return PGSIO_E_L0_MEM_ALLOC;
	    }

	    for (count=1;count<num_files;count++)
	    {
		if (version_entry->start_time == start_stop[count][0])
		{
		    version_entry->stop_time = start_stop[count][1];
		    break;
		}
	    }

	    free(start_stop);
	    
	    if (file_size != hdr_struct_size)
	    {
		free(file_hdr_EOS_AM);
		PGS_IO_Gen_Close( open_fs );
		PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
				      "invalid construction record size",
				      toolname);
		return PGSIO_E_L0_BAD_FILEINFO;
	    }
	    
	    if (count == num_files)
	    {
		free(file_hdr_EOS_AM);
		PGS_IO_Gen_Close( open_fs );
		PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
				      "corrupt construction record, invalid "
				      "EDS start time",
				      toolname);
		return PGSIO_E_L0_BAD_FILEINFO;
	    }

	    version_entry->stat_hdr_size = 0UL;
	    version_entry->var_hdr_start = version_entry->stat_hdr_size;
	    version_entry->var_hdr_size = 0UL;
	    version_entry->pkts_start = 0UL;
	    
	    fseek( open_fs, 0L, SEEK_END );
	    version_entry->file_size = ftell( open_fs );
	    version_entry->packet_count = version_entry->file_size;
	    version_entry->pkts_size = version_entry->file_size;
	    version_entry->footer_start = version_entry->file_size;
	    version_entry->footer_size = 0UL;
	}

	free(file_hdr_EOS_AM);
	
	break;

      case PGSd_EOS_PM_GIRD:
	
	if (file_version == 1)
	{
	    open_hdr = open_fs;
	}
	else
	{
	    returnStatus = PGS_IO_Gen_Open(file_logical,
					   PGSd_IO_Gen_Read,
					   &open_hdr,
					   1);

	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	}
	
	fseek(open_hdr, 0L,  SEEK_END);
	
	hdr_struct_size = ftell(open_hdr);
	
	rewind(open_hdr);
	
	file_hdr_EOS_PM_GIRD = (unsigned char*) calloc(hdr_struct_size, 1);

	if (file_hdr_EOS_PM_GIRD == NULL)
	{
	    PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
	    PGS_IO_Gen_Close( open_fs );
	    return PGSIO_E_L0_MEM_ALLOC;
	}
	
        /* 
         *    Read in the file header
         */

        count = fread( file_hdr_EOS_PM_GIRD, 1, hdr_struct_size, open_hdr);
        unixerror=errno;
        if (count != hdr_struct_size )                 /* incomplete read */
        {
	    free(file_hdr_EOS_PM_GIRD);
            returnStatus = PGS_E_UNIX;
            PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	if (file_version != 1)
	{
	    returnStatus = PGS_IO_Gen_Close( open_hdr );
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		free(file_hdr_EOS_PM_GIRD);
		returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	}

        /* 
         *    Get file  start time, stop time, packet count, 
         *    start byte location of first packet,
         *    end   byte location of file
         */

	offset = 50;
	
	offset = offset + 2 + 16*(256*file_hdr_EOS_PM_GIRD[offset] +
				  file_hdr_EOS_PM_GIRD[offset+1]) + 12;
	
        returnStatus = PGS_TD_EOSPMGIIStoTAI(
            file_hdr_EOS_PM_GIRD+offset,
            &(version_entry->start_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
	    free(file_hdr_EOS_PM_GIRD);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	offset = offset + 8;
	
        returnStatus = PGS_TD_EOSPMGIIStoTAI(
            file_hdr_EOS_PM_GIRD+offset,
            &(version_entry->stop_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
	    free(file_hdr_EOS_PM_GIRD);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	offset = offset + 28;
	
        /* Fill rest of file table data
           Note that much of the following is currently unused,
           since EOS_PM_GIRD header is static length -- it is included
           here for consistency, and possible future use */

	if (file_version == 1)
	{
	    version_entry->packet_count = 
                (file_hdr_EOS_PM_GIRD+offset)[3] + 256UL * (
		(file_hdr_EOS_PM_GIRD+offset)[2] + 256UL * (
		(file_hdr_EOS_PM_GIRD+offset)[1] + 256UL * (
        	(file_hdr_EOS_PM_GIRD+offset)[0] )));

	    version_entry->stat_hdr_size = hdr_struct_size;
	    version_entry->var_hdr_start = version_entry->stat_hdr_size;
	    version_entry->var_hdr_size = 0UL;
	    version_entry->pkts_start = hdr_struct_size;
	    version_entry->file_size = hdr_struct_size;
	    
	    version_entry->pkts_size = 0UL;
	    version_entry->footer_start = version_entry->file_size;
	    version_entry->footer_size = 0UL;
	    
	}
	else
	{
	    fread(file_hdr_EOS_PM_GIRD, 1, 
		  PGSd_IO_L0_PrimaryPktHdrSize + PGSd_IO_L0_SecPktHdrSizeEOS_PM,
		  open_fs);
	    /* Note that for PM_GIRD timestamp is 1 byte after 
	       Primary header unlike the AM and PM_GIIS that 
	       timestamp is rigth after Primary header */
	    returnStatus = PGS_TD_EOSPMGIRDtoTAI((file_hdr_EOS_PM_GIRD +
					      PGSd_IO_L0_PrimaryPktHdrSize +1),
					     &(version_entry->start_time) );
	    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
	    {
		free(file_hdr_EOS_PM_GIRD);
		returnStatus = PGSIO_E_L0_TIME_CONVERSION;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	    
	    start_stop = (PGSt_double (*)[2]) calloc(255,
						      2*sizeof(PGSt_double));
	    if (start_stop == (PGSt_double (*)[2]) NULL)
	    {
		PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
		return PGSIO_E_L0_MEM_ALLOC;
	    }
	    
	    returnStatus = PGS_IO_L0_GetEOSPMGIRDfileTimes(file_hdr_EOS_PM_GIRD,
						       start_stop,
						       &num_files,
						       &file_size);
	    
	    if( returnStatus != PGS_S_SUCCESS )
	    {
		free(file_hdr_EOS_PM_GIRD);
		free(start_stop);
		returnStatus = PGSIO_E_L0_TIME_CONVERSION;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	    
	    start_stop = (PGSt_double (*)[2]) realloc((void*) start_stop,
						      2*sizeof(PGSt_double)*
						      num_files);
	    
	    if (start_stop == (PGSt_double (*)[2]) NULL)
	    {
		PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
		return PGSIO_E_L0_MEM_ALLOC;
	    }

	    for (count=1;count<num_files;count++)
	    {
		if (fabs(version_entry->start_time - start_stop[count][0]) <= PGSd_TD_GIRD_GIIS_EPSILON)
		{
		    version_entry->stop_time = start_stop[count][1];
		    break;
		}
	    }

	    free(start_stop);
	    
	    if (file_size != hdr_struct_size)
	    {
		free(file_hdr_EOS_PM_GIRD);
		PGS_IO_Gen_Close( open_fs );
		PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
				      "invalid construction record size",
				      toolname);
		return PGSIO_E_L0_BAD_FILEINFO;
	    }
	    
	    if (count == num_files)
	    {
		free(file_hdr_EOS_PM_GIRD);
		PGS_IO_Gen_Close( open_fs );
		PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
				      "corrupt construction record, invalid "
				      "EDS start time",
				      toolname);
		return PGSIO_E_L0_BAD_FILEINFO;
	    }

	    version_entry->stat_hdr_size = 0UL;
	    version_entry->var_hdr_start = version_entry->stat_hdr_size;
	    version_entry->var_hdr_size = 0UL;
	    version_entry->pkts_start = 0UL;
	    
	    fseek( open_fs, 0L, SEEK_END );
	    version_entry->file_size = ftell( open_fs );
	    version_entry->packet_count = version_entry->file_size;
	    version_entry->pkts_size = version_entry->file_size;
	    version_entry->footer_start = version_entry->file_size;
	    version_entry->footer_size = 0UL;
	}

	free(file_hdr_EOS_PM_GIRD);
	
	break;


      case PGSd_EOS_PM_GIIS:
	
	if (file_version == 1)
	{
	    open_hdr = open_fs;
	}
	else
	{
	    returnStatus = PGS_IO_Gen_Open(file_logical,
					   PGSd_IO_Gen_Read,
					   &open_hdr,
					   1);
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	}
	
	fseek(open_hdr, 0L,  SEEK_END);
	
	hdr_struct_size = ftell(open_hdr);
	
	rewind(open_hdr);
	
	file_hdr_EOS_PM_GIIS = (unsigned char*) calloc(hdr_struct_size, 1);

	if (file_hdr_EOS_PM_GIIS == NULL)
	{
	    PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
	    PGS_IO_Gen_Close( open_fs );
	    return PGSIO_E_L0_MEM_ALLOC;
	}
	
        /* 
         *    Read in the file header
         */

        count = fread( file_hdr_EOS_PM_GIIS, 1, hdr_struct_size, open_hdr);
        unixerror=errno;
        if (count != hdr_struct_size )                 /* incomplete read */
        {
	    free(file_hdr_EOS_PM_GIIS);
            returnStatus = PGS_E_UNIX;
            PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	if (file_version != 1)
	{
	    returnStatus = PGS_IO_Gen_Close( open_hdr );
	    
	    if (returnStatus != PGS_S_SUCCESS) 
	    {
		free(file_hdr_EOS_PM_GIIS);
		returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	}

        /* 
         *    Get file  start time, stop time, packet count, 
         *    start byte location of first packet,
         *    end   byte location of file
         */

	offset = 50;
	
	offset = offset + 2 + 16*(256*file_hdr_EOS_PM_GIIS[offset] +
				  file_hdr_EOS_PM_GIIS[offset+1]) + 12;
	
        returnStatus = PGS_TD_EOSPMGIIStoTAI(
            file_hdr_EOS_PM_GIIS+offset,
            &(version_entry->start_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
	    free(file_hdr_EOS_PM_GIIS);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	offset = offset + 8;
	
        returnStatus = PGS_TD_EOSPMGIIStoTAI(
            file_hdr_EOS_PM_GIIS+offset,
            &(version_entry->stop_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
	    free(file_hdr_EOS_PM_GIIS);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

	offset = offset + 28;
	
        /* Fill rest of file table data
           Note that much of the following is currently unused,
           since EOS_PM_GIIS header is static length -- it is included
           here for consistency, and possible future use */

	if (file_version == 1)
	{
	    version_entry->packet_count = 
                (file_hdr_EOS_PM_GIIS+offset)[3] + 256UL * (
		(file_hdr_EOS_PM_GIIS+offset)[2] + 256UL * (
		(file_hdr_EOS_PM_GIIS+offset)[1] + 256UL * (
        	(file_hdr_EOS_PM_GIIS+offset)[0] )));

	    version_entry->stat_hdr_size = hdr_struct_size;
	    version_entry->var_hdr_start = version_entry->stat_hdr_size;
	    version_entry->var_hdr_size = 0UL;
	    version_entry->pkts_start = hdr_struct_size;
	    version_entry->file_size = hdr_struct_size;
	    
	    version_entry->pkts_size = 0UL;
	    version_entry->footer_start = version_entry->file_size;
	    version_entry->footer_size = 0UL;
	    
	}
	else
	{
	    fread(file_hdr_EOS_PM_GIIS, 1, 
		  PGSd_IO_L0_PrimaryPktHdrSize + PGSd_IO_L0_SecPktHdrSizeEOS_PM,
		  open_fs);
	    
	    returnStatus = PGS_TD_EOSPMGIIStoTAI((file_hdr_EOS_PM_GIIS +
					      PGSd_IO_L0_PrimaryPktHdrSize),
					     &(version_entry->start_time) );
	    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
	    {
		free(file_hdr_EOS_PM_GIIS);
		returnStatus = PGSIO_E_L0_TIME_CONVERSION;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	    
	    start_stop = (PGSt_double (*)[2]) calloc(255,
						      2*sizeof(PGSt_double));
	    if (start_stop == (PGSt_double (*)[2]) NULL)
	    {
		PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
		return PGSIO_E_L0_MEM_ALLOC;
	    }
	    
	    returnStatus = PGS_IO_L0_GetEOSPMGIISfileTimes(file_hdr_EOS_PM_GIIS,
						       start_stop,
						       &num_files,
						       &file_size);
	    
	    if( returnStatus != PGS_S_SUCCESS )
	    {
		free(file_hdr_EOS_PM_GIIS);
		free(start_stop);
		returnStatus = PGSIO_E_L0_TIME_CONVERSION;
		PGS_SMF_SetStaticMsg( returnStatus, toolname);
		PGS_IO_Gen_Close( open_fs );
		return returnStatus;
	    }
	    
	    start_stop = (PGSt_double (*)[2]) realloc((void*) start_stop,
						      2*sizeof(PGSt_double)*
						      num_files);
	    
	    if (start_stop == (PGSt_double (*)[2]) NULL)
	    {
		PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
		return PGSIO_E_L0_MEM_ALLOC;
	    }

	    for (count=1;count<num_files;count++)
	    {
		if (version_entry->start_time == start_stop[count][0])
		{
		    version_entry->stop_time = start_stop[count][1];
		    break;
		}
	    }

	    free(start_stop);
	    
	    if (file_size != hdr_struct_size)
	    {
		free(file_hdr_EOS_PM_GIIS);
		PGS_IO_Gen_Close( open_fs );
		PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
				      "invalid construction record size",
				      toolname);
		return PGSIO_E_L0_BAD_FILEINFO;
	    }
	    
	    if (count == num_files)
	    {
		free(file_hdr_EOS_PM_GIIS);
		PGS_IO_Gen_Close( open_fs );
		PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
				      "corrupt construction record, invalid "
				      "EDS start time",
				      toolname);
		return PGSIO_E_L0_BAD_FILEINFO;
	    }

	    version_entry->stat_hdr_size = 0UL;
	    version_entry->var_hdr_start = version_entry->stat_hdr_size;
	    version_entry->var_hdr_size = 0UL;
	    version_entry->pkts_start = 0UL;
	    
	    fseek( open_fs, 0L, SEEK_END );
	    version_entry->file_size = ftell( open_fs );
	    version_entry->packet_count = version_entry->file_size;
	    version_entry->pkts_size = version_entry->file_size;
	    version_entry->footer_start = version_entry->file_size;
	    version_entry->footer_size = 0UL;
	}

	free(file_hdr_EOS_PM_GIIS);
	
	break;

      case PGSd_EOS_AURA:
 
        if (file_version == 1)
        {
            open_hdr = open_fs;
        }
        else
        {
            returnStatus = PGS_IO_Gen_Open(file_logical,
                                           PGSd_IO_Gen_Read,
                                           &open_hdr,
                                           1);
 
            if (returnStatus != PGS_S_SUCCESS)
            {
                returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
                PGS_SMF_SetStaticMsg( returnStatus, toolname);
                PGS_IO_Gen_Close( open_fs );
                return returnStatus;
            }
        }
 
        fseek(open_hdr, 0L,  SEEK_END);
 
        hdr_struct_size = ftell(open_hdr);
 
        rewind(open_hdr);
 
        file_hdr_EOS_AURA = (unsigned char*) calloc(hdr_struct_size, 1);
 
        if (file_hdr_EOS_AURA == NULL)
        {
            PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
            PGS_IO_Gen_Close( open_fs );
            return PGSIO_E_L0_MEM_ALLOC;
        }
 
        /*
         *    Read in the file header
         */
 
        count = fread( file_hdr_EOS_AURA, 1, hdr_struct_size, open_hdr);
        unixerror=errno;
        if (count != hdr_struct_size )                 /* incomplete read */
        {
            free(file_hdr_EOS_AURA);
            returnStatus = PGS_E_UNIX;
            PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }
 
        if (file_version != 1)
        {
            returnStatus = PGS_IO_Gen_Close( open_hdr );
 
            if (returnStatus != PGS_S_SUCCESS)
            {
                free(file_hdr_EOS_AURA);
                returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
                PGS_SMF_SetStaticMsg( returnStatus, toolname);
                PGS_IO_Gen_Close( open_fs );
                return returnStatus;
            }
        }
 
        /*
         *    Get file  start time, stop time, packet count,
         *    start byte location of first packet,
         *    end   byte location of file
         */
 
        offset = 50;
 
        offset = offset + 2 + 16*(256*file_hdr_EOS_AURA[offset] +
                                  file_hdr_EOS_AURA[offset+1]) + 12;
 
        returnStatus = PGS_TD_EOSAURAGIIStoTAI(
            file_hdr_EOS_AURA+offset,
            &(version_entry->start_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
            free(file_hdr_EOS_AURA);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }
 
        offset = offset + 8;
 
        returnStatus = PGS_TD_EOSAURAGIIStoTAI(
            file_hdr_EOS_AURA+offset,
            &(version_entry->stop_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
            free(file_hdr_EOS_AURA);
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }
 
        offset = offset + 28;
 
        /* Fill rest of file table data
           Note that much of the following is currently unused,
           since EOS_AURA header is static length -- it is included
           here for consistency, and possible future use */
 
        if (file_version == 1)
        {
            version_entry->packet_count =
                (file_hdr_EOS_AURA+offset)[3] + 256UL * (
                (file_hdr_EOS_AURA+offset)[2] + 256UL * (
                (file_hdr_EOS_AURA+offset)[1] + 256UL * (
                (file_hdr_EOS_AURA+offset)[0] )));
 
            version_entry->stat_hdr_size = hdr_struct_size;
            version_entry->var_hdr_start = version_entry->stat_hdr_size;
            version_entry->var_hdr_size = 0UL;
            version_entry->pkts_start = hdr_struct_size;
            version_entry->file_size = hdr_struct_size;
 
            version_entry->pkts_size = 0UL;
            version_entry->footer_start = version_entry->file_size;
            version_entry->footer_size = 0UL;
 
        }
        else
        {
            fread(file_hdr_EOS_AURA, 1,
                  PGSd_IO_L0_PrimaryPktHdrSize + PGSd_IO_L0_SecPktHdrSizeEOS_AURA,
                  open_fs);
            /* Note that for AURA timestamp is 1 byte after
               Primary header unlike the AM and PM_GIIS that
               timestamp is rigth after Primary header */
            returnStatus = PGS_TD_EOSAURAGIRDtoTAI((file_hdr_EOS_AURA +
                                              PGSd_IO_L0_PrimaryPktHdrSize +1),
                                             &(version_entry->start_time) );
            if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
            {
                free(file_hdr_EOS_AURA);
                returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                PGS_SMF_SetStaticMsg( returnStatus, toolname);
                PGS_IO_Gen_Close( open_fs );
                return returnStatus;
            }
 
            start_stop = (PGSt_double (*)[2]) calloc(255,
                                                      2*sizeof(PGSt_double));
            if (start_stop == (PGSt_double (*)[2]) NULL)
            {
                PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
                return PGSIO_E_L0_MEM_ALLOC;
            }
 
            returnStatus = PGS_IO_L0_GetEOSAURAfileTimes(file_hdr_EOS_AURA,
                                                       start_stop,
                                                       &num_files,
                                                       &file_size);
 
            if( returnStatus != PGS_S_SUCCESS )
            {
                free(file_hdr_EOS_AURA);
                free(start_stop);
                returnStatus = PGSIO_E_L0_TIME_CONVERSION;
                PGS_SMF_SetStaticMsg( returnStatus, toolname);
                PGS_IO_Gen_Close( open_fs );
                return returnStatus;
            }
 
            start_stop = (PGSt_double (*)[2]) realloc((void*) start_stop,
                                                      2*sizeof(PGSt_double)*
                                                      num_files);
 
            if (start_stop == (PGSt_double (*)[2]) NULL)
            {
                PGS_SMF_SetStaticMsg(PGSIO_E_L0_MEM_ALLOC, toolname);
                return PGSIO_E_L0_MEM_ALLOC;
            }
 
            for (count=1;count<num_files;count++)
            {
                if (fabs(version_entry->start_time - start_stop[count][0]) <= PGSd_TD_GIRD_GIIS_EPSILON)
                {
                    version_entry->stop_time = start_stop[count][1];
                    break;
                }
            }
 
            free(start_stop);
 
            if (file_size != hdr_struct_size)
            {
                free(file_hdr_EOS_AURA);
                PGS_IO_Gen_Close( open_fs );
                PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
                                      "invalid construction record size",
                                      toolname);
                return PGSIO_E_L0_BAD_FILEINFO;
            }
 
            if (count == num_files)
            {
                free(file_hdr_EOS_AURA);
                PGS_IO_Gen_Close( open_fs );
                PGS_SMF_SetDynamicMsg(PGSIO_E_L0_BAD_FILEINFO,
                                      "corrupt construction record, invalid "
                                      "EDS start time",
                                      toolname);
                return PGSIO_E_L0_BAD_FILEINFO;
            }
 
            version_entry->stat_hdr_size = 0UL;
            version_entry->var_hdr_start = version_entry->stat_hdr_size;
            version_entry->var_hdr_size = 0UL;
            version_entry->pkts_start = 0UL;
 
            fseek( open_fs, 0L, SEEK_END );
            version_entry->file_size = ftell( open_fs );
            version_entry->packet_count = version_entry->file_size;
            version_entry->pkts_size = version_entry->file_size;
            version_entry->footer_start = version_entry->file_size;
            version_entry->footer_size = 0UL;
        }
 
        free(file_hdr_EOS_AURA);
 
        break;
 

      case PGSd_TRMM:

      /* The following values are determined in PGS_IO_L0_TRMM_HdrInfo():

      version_entry->start_time
      version_entry->stop_time
      version_entry->packet_count
      version_entry->stat_hdr_size
      version_entry->var_hdr_start
      version_entry->var_hdr_size
      version_entry->pkts_start
      version_entry->pkts_size
      version_entry->footer_start
      version_entry->footer_size
      version_entry->file_size

      */

        returnStatus = PGS_IO_L0_TRMM_HdrInfo(open_fs, version_entry);

        if (returnStatus != PGS_S_SUCCESS) 
        {
  	    returnStatus = PGSIO_E_L0_BAD_FILEINFO;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    PGS_IO_Gen_Close( open_fs );
	    return returnStatus;
        }

	break;


      case PGSd_ADEOS_II:
	
        /* 
         *    Clear the header buffer
         *    Read in the fixed-length portion of the file header
         */
        hdr_struct_size = PGSd_IO_L0_FileHdrSizeADEOS_II;
#ifdef _PGS_THREADSAFE
        /* 
         *    Read in header value and load into TSD key.
         */
        ptr = (char *) file_hdr_ADEOS_II;
        for(i=0; i < hdr_struct_size; i++) *ptr++ = 0;

        count = fread( file_hdr_ADEOS_II, 1, hdr_struct_size, open_fs);
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL0FILEHDRADEOSII],
                            (void *) file_hdr_ADEOS_II);
#else
        ptr = (char *) &file_hdr_ADEOS_II;
        for(i=0; i < hdr_struct_size; i++) *ptr++ = 0;

        count = fread( &file_hdr_ADEOS_II, 1, hdr_struct_size, open_fs);
#endif
        unixerror=errno;
        if (count != hdr_struct_size )       /* incomplete read */
        {
            returnStatus = PGS_E_UNIX;
            PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }


        /* 
         *    Get file  start time, stop time, packet count, 
         *    start byte location of first packet,
         *    end   byte location of file
         */

        /* SPECIAL NOTE: The function PGS_TD_ADEOSIItoTAI is used
           to translate the packet time stamp to TAI in PGS_IO_L0_GetPacket.
           Here it is assumed that the file header time format
           is the same as the packet time stamp format, in lieu
           of information to the contrary, as EOS PM file
           header format is unavailable (Feb. 1995). */

#ifdef _PGS_THREADSAFE
        /* 
         *    This section had to be separated due to the fact that 
         *    the TSD is a pointer whereas the static header is not.
         */
        returnStatus = PGS_TD_ADEOSIItoTAI(             /* !!! prototype code */
            file_hdr_ADEOS_II->spacecraftClockFirst,
            &(version_entry->start_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

        returnStatus = PGS_TD_ADEOSIItoTAI(             /* !!! prototype code */
            file_hdr_ADEOS_II->spacecraftClockLast,
            &(version_entry->stop_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

        version_entry->packet_count =                 /* !!! prototype code */
            file_hdr_ADEOS_II->packetCount[3] + 256UL * (
            file_hdr_ADEOS_II->packetCount[2] + 256UL * (
            file_hdr_ADEOS_II->packetCount[1] + 256UL * (
            file_hdr_ADEOS_II->packetCount[0] )));
#else
        returnStatus = PGS_TD_ADEOSIItoTAI(             /* !!! prototype code */
            file_hdr_ADEOS_II.spacecraftClockFirst,
            &(version_entry->start_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

        returnStatus = PGS_TD_ADEOSIItoTAI(             /* !!! prototype code */
            file_hdr_ADEOS_II.spacecraftClockLast,
            &(version_entry->stop_time) );
        if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
        {
            returnStatus = PGSIO_E_L0_TIME_CONVERSION;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            PGS_IO_Gen_Close( open_fs );
            return returnStatus;
        }

        version_entry->packet_count =                 /* !!! prototype code */
            file_hdr_ADEOS_II.packetCount[3] + 256UL * (
            file_hdr_ADEOS_II.packetCount[2] + 256UL * (
            file_hdr_ADEOS_II.packetCount[1] + 256UL * (
            file_hdr_ADEOS_II.packetCount[0] )));
#endif

        /* Fill rest of file table data
           Note that much of the following is currently unused,
           since ADEOS_II header is static length -- it is included
           here for consistency, and possible future use */

        version_entry->stat_hdr_size = hdr_struct_size;
        version_entry->var_hdr_start = version_entry->stat_hdr_size;
        version_entry->var_hdr_size = 0UL;
        version_entry->pkts_start = hdr_struct_size;

        fseek( open_fs, 0L, SEEK_END );
        version_entry->file_size = ftell( open_fs );

        version_entry->pkts_size = version_entry->file_size - hdr_struct_size;
        version_entry->footer_start = version_entry->file_size;
        version_entry->footer_size = 0UL;

	break;


      default:
	returnStatus = PGSIO_E_L0_BAD_SPACECRAFT_TAG;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);

	PGS_IO_Gen_Close( open_fs );
	return returnStatus;
    }

    /* Initialize no. packets currently read */
    version_entry->num_pkts_read = 0UL;

    /* 
     *    Close the physical file
     */

    returnStatus = PGS_IO_Gen_Close( open_fs );

    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    return returnStatus;
}


