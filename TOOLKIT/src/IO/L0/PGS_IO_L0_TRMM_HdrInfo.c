/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_TRMM_HdrInfo.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to get TRMM file header 
  information and place in version table entry.

AUTHOR:
  Tom Atwater / Applied Research Corp.

HISTORY:
  30-Jan-1995 TWA  Initial version
  06-Jul-1999 RM   updated for TSF functionality

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>
#include <PGS_TSF.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Get TRMM file header information and place in version table entry
	
NAME:
	PGS_IO_L0_TRMM_HdrInfo
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_TRMM_HdrInfo(
        	PGSt_IO_Gen_FileHandle  *open_fs,
		PGSt_IO_L0_VersionTable	*version_entry
     	)

  FORTRAN:
	N/A

DESCRIPTION:
	This tool reads the TRMM file header from the currently open 
	physical file.   It determines the file start time, stop time.
        total packet count, and sizes and start bytes of static
        header, variable length header, packets, and footer.   
        These values are then stored in the file version table entry.

INPUTS:
	open_fs - file handle of currently open physical file.

OUTPUTS:
	version_entry - The version table entry where the calculated
		data for the specified physical file are stored.

NOTES:
	This is a low-level tool.  It is not intended to be called directly by 
	any user code.

EXAMPLES:
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_TIME_CONVERSION
	PGSTSF_E_GENERAL_FAILURE
	
REQUIREMENTS:
	PGSTK-0200, 0240

DETAILS:
	File start/stop times are converted from PB5 to TAI since 1/1/93.
        Other physical file header data are extracted.
        Tool assumes high-order byte is first when converting byte data 
        to integer.

GLOBALS:
        None

FILES:
        Physical files for the specified virtual data set

FUNCTIONS CALLED:
        PGS_TD_PB5toTAI
	PGS_SMF_SetStaticMsg
	PGS_SMF_SetUNIXMsg
	PGS_SMF_TestErrorLevel
	PGS_TSF_GetTSFMaster


END_PROLOG
*****************************************************************************/
PGSt_SMF_status 
PGS_IO_L0_TRMM_HdrInfo(
    PGSt_IO_Gen_FileHandle  *open_fs,
    PGSt_IO_L0_VersionTable *version_entry
    )

{
#ifdef _PGS_THREADSAFE
    /* set up non-static var */
    PGSt_IO_L0_FileHdrTRMM    *file_hdr_TRMM; /* file header (PGS_IO_L0.h) */
#else
    static PGSt_IO_L0_FileHdrTRMM    file_hdr_TRMM; /* file header (PGS_IO_L0.h) */
#endif

    PGSt_integer             count;

    long                     unixerror;              /* retains errno value */

    char                     *ptr;
    int                      i;

    long                     hdr_struct_size;        /* Size of header struct */

    PGSt_SMF_status returnStatus;
    static char *toolname = "PGS_IO_L0_TRMM_HdrInfo()";

#ifdef _PGS_THREADSAFE
/* This section is separated because the TRMM header is a pointer for 
   the TSF mode */

    /* get TSF master struct */
    PGSt_TSF_MasterStruct *masterTSF;

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(returnStatus))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* load header from TSD key */
    file_hdr_TRMM = (PGSt_IO_L0_FileHdrTRMM *) pthread_getspecific(
                                 masterTSF->keyArray[PGSd_TSF_KEYIOL0FILEHDRTRMM]);

/* Determine size of structure into which header will be read 
   For TRMM, this structure has a static part and a variable length part
   It is defined in PGS_IO_L0.h */

    hdr_struct_size = sizeof(PGSt_IO_L0_FileHdrTRMM);

/* Clear the header buffer */

    ptr = (char *) file_hdr_TRMM;
    for(i=0; i < hdr_struct_size; i++)
	*ptr++ = 0;

/*  Read in the fixed-length portion of the file header */

    version_entry->stat_hdr_size = hdr_struct_size - TRMM_HDR_VAR_LEN;

    count = fread( file_hdr_TRMM, 1, version_entry->stat_hdr_size, open_fs);
    pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYIOL0FILEHDRTRMM],
                              file_hdr_TRMM);
    unixerror=errno;
    if (count != version_entry->stat_hdr_size )   /* incomplete read */
    {
	returnStatus = PGS_E_UNIX;
	PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
	return returnStatus;
    }

/* Get file start time in TAI seconds (for sorting files in time order) */

    returnStatus = PGS_TD_PB5toTAI(
	file_hdr_TRMM->spacecraftClockFirst, &(version_entry->start_time) );
    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
    {
	returnStatus = PGSIO_E_L0_TIME_CONVERSION;
	PGS_SMF_SetStaticMsg( returnStatus, toolname );
	return returnStatus;
    }

/* Get file stop time in TAI seconds */

    returnStatus = PGS_TD_PB5toTAI(
	file_hdr_TRMM->spacecraftClockLast, &(version_entry->stop_time) );
    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
    {
	returnStatus = PGSIO_E_L0_TIME_CONVERSION;
	PGS_SMF_SetStaticMsg( returnStatus, toolname );
	return returnStatus;
    }

/* Get no. packets in this file */

    version_entry->packet_count =
	file_hdr_TRMM->packetCount[3] + 256UL * (
	    file_hdr_TRMM->packetCount[2] + 256UL * (
		file_hdr_TRMM->packetCount[1] + 256UL * (
		    file_hdr_TRMM->packetCount[0] )));

/* Calculate size of variable length part of file header
   Read variable length part of file header */

    version_entry->var_hdr_size = 2UL * file_hdr_TRMM->numAPID + 6UL;
    count = 
	fread( file_hdr_TRMM->varLenBuf, 1, version_entry->var_hdr_size, open_fs);
    unixerror=errno;
    if (count != version_entry->var_hdr_size )   /* incomplete read */
    {
	returnStatus = PGS_E_UNIX;
	PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
	return returnStatus;
    }

/* Calculate:
   Start byte of variable length part of file header
   Start byte of first packet
   Size in bytes of packets
   (Last 4 bytes of header are the offset in bytes to the start 
   of footer data, measured from the last byte of the header; this is
   equal to the total number of bytes in packets)
   Start byte of footer
   End byte of file */

    version_entry->var_hdr_start = version_entry->stat_hdr_size;

    version_entry->pkts_start = 
	version_entry->stat_hdr_size + version_entry->var_hdr_size;

    version_entry->pkts_size =
	file_hdr_TRMM->varLenBuf[version_entry->var_hdr_size-1] + 256UL * (
	    file_hdr_TRMM->varLenBuf[version_entry->var_hdr_size-2] + 256UL * (
		file_hdr_TRMM->varLenBuf[version_entry->var_hdr_size-3] + 256UL * (
		    file_hdr_TRMM->varLenBuf[version_entry->var_hdr_size-4] )));

#else /* -D_PGS_THREADSAFE */


/* Determine size of structure into which header will be read 
   For TRMM, this structure has a static part and a variable length part
   It is defined in PGS_IO_L0.h */

    hdr_struct_size = sizeof(PGSt_IO_L0_FileHdrTRMM);

/* Clear the header buffer */

    ptr = (char *) &file_hdr_TRMM;
    for(i=0; i < hdr_struct_size; i++)
	*ptr++ = 0;

/*  Read in the fixed-length portion of the file header */

    version_entry->stat_hdr_size = hdr_struct_size - TRMM_HDR_VAR_LEN;

    count = fread( &file_hdr_TRMM, 1, version_entry->stat_hdr_size, open_fs);
    unixerror=errno;
    if (count != version_entry->stat_hdr_size )   /* incomplete read */
    {
	returnStatus = PGS_E_UNIX;
	PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
	return returnStatus;
    }

/* Get file start time in TAI seconds (for sorting files in time order) */

    returnStatus = PGS_TD_PB5toTAI(
	file_hdr_TRMM.spacecraftClockFirst, &(version_entry->start_time) );
    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
    {
	returnStatus = PGSIO_E_L0_TIME_CONVERSION;
	PGS_SMF_SetStaticMsg( returnStatus, toolname );
	return returnStatus;
    }

/* Get file stop time in TAI seconds */

    returnStatus = PGS_TD_PB5toTAI(
	file_hdr_TRMM.spacecraftClockLast, &(version_entry->stop_time) );
    if( PGS_SMF_TestStatusLevel(returnStatus) >= PGS_SMF_MASK_LEV_E )
    {
	returnStatus = PGSIO_E_L0_TIME_CONVERSION;
	PGS_SMF_SetStaticMsg( returnStatus, toolname );
	return returnStatus;
    }

/* Get no. packets in this file */

    version_entry->packet_count =
	file_hdr_TRMM.packetCount[3] + 256UL * (
	    file_hdr_TRMM.packetCount[2] + 256UL * (
		file_hdr_TRMM.packetCount[1] + 256UL * (
		    file_hdr_TRMM.packetCount[0] )));

/* Calculate size of variable length part of file header
   Read variable length part of file header */

    version_entry->var_hdr_size = 2UL * file_hdr_TRMM.numAPID + 6UL;
    count = 
	fread( file_hdr_TRMM.varLenBuf, 1, version_entry->var_hdr_size, open_fs);
    unixerror=errno;
    if (count != version_entry->var_hdr_size )   /* incomplete read */
    {
	returnStatus = PGS_E_UNIX;
	PGS_SMF_SetUNIXMsg( unixerror, NULL, toolname); /* error */
	return returnStatus;
    }

/* Calculate:
   Start byte of variable length part of file header
   Start byte of first packet
   Size in bytes of packets
   (Last 4 bytes of header are the offset in bytes to the start 
   of footer data, measured from the last byte of the header; this is
   equal to the total number of bytes in packets)
   Start byte of footer
   End byte of file */

    version_entry->var_hdr_start = version_entry->stat_hdr_size;

    version_entry->pkts_start = 
	version_entry->stat_hdr_size + version_entry->var_hdr_size;

    version_entry->pkts_size =
	file_hdr_TRMM.varLenBuf[version_entry->var_hdr_size-1] + 256UL * (
	    file_hdr_TRMM.varLenBuf[version_entry->var_hdr_size-2] + 256UL * (
		file_hdr_TRMM.varLenBuf[version_entry->var_hdr_size-3] + 256UL * (
		    file_hdr_TRMM.varLenBuf[version_entry->var_hdr_size-4] )));

#endif /* -D_PGS_THREADSAFE */

    version_entry->footer_start = 
	version_entry->pkts_start + version_entry->pkts_size;

    fseek( open_fs, 0L, SEEK_END );
    version_entry->file_size = ftell( open_fs );
    version_entry->footer_size = 
	version_entry->file_size - version_entry->footer_start;

    return PGS_S_SUCCESS;

}

