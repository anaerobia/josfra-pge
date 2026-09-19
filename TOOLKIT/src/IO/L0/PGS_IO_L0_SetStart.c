/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_SetStart.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to set the specified open virtual
  data set so that the next call to PGS_IO_L0_GetPacket will read the first
  packet at or after the specified time.

AUTHOR:
  Tom W. Atwater / Applied Research Corp.
  Guru Tej S. Khalsa / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
  30-Jan-1995 TWA  Initial version
  12-Jul-1995 GTSK Improved error handling
  02-Apr-1996 GTSK Added ADEOS-II support, fixed bug in algorithm to determine
                   file that contains target packet
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  04-Aug-2000 AT   Modified code for AM so that if a bit flip warning is
                   recieved from PGS_IO_L0_SeekPacket, the warning is
		   returned to the calling function.
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Set the Start Time for Next Call to PGS_IO_L0_GetPacket
	
NAME:
	PGS_IO_L0_SetStart
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_SetStart(
		PGSt_IO_L0_VirtualDataSet virtual_file,
		PGSt_double		  start_time
		)
  FORTRAN:
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'

	integer function PGS_IO_L0_SetStart(
       +                         virtual_file,
       +                         start_time)
	integer		 virtual_file
	double precision start_time

DESCRIPTION:
	Sets the virtual file pointer so that the next call to the tool 
	PGS_IO_L0_GetPacket will read the first available packet at or
	after the specified time.
	
INPUTS:
	virtual_file - 	The file descriptor for this virtual data set,
		returned by the call to PGS_IO_L0_Open.
	
	start_time -	The start time of the desired packet.
		Format is TAI: continuous seconds since 12AM UTC Jan 1, 1993

OUTPUTS:
	NONE	

NOTES:
	Normal return is PGS_S_SUCCESS.

        A virtual data set must have been opened by PGS_IO_L0_Open
        before this function is called.

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
        variable length part, nor do EOS files have footers.  Preliminary
	support of ADEOS-II packets (file headers) has been provided as well,
	although as of this writing (29-Mar-1996) the format for neither of the
	these has been finalized.


EXAMPLES:

	Set the time to start processing as 20 minutes after the
	data set start time.
	Examples assume the data set start time has previously
	been returned from PGS_IO_L0_Open.
	
  C:
	PGSt_IO_L0_VirtualDataSet  virtual_file;
	PGSt_double                start_time;
	PGSt_double                new_start_time;
	PGSt_SMF_status            returnStatus;

	new_start_time = start_time + 1200.0;

	returnStatus = PGS_IO_L0_SetStart( virtual_file, new_start_time);
	if (returnStatus != PGS_S_SUCCESS) 
	{
	    goto EXCEPTION;  /# GO TO EXCEPTION HANDLING #/
	}


  FORTRAN:
        implicit none
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'
 	integer            pgs_io_l0_setstart
	integer            virtual_file
	double precision   start_time
	double precision   new_start_time
	integer            returnstatus

	new_start_time = start_time + 1200.0

	returnstatus = pgs_io_l0_setstart( virtual_file, new_start_time)
	if (returnStatus .ne. PGS_S_SUCCESS) goto EXCEPTION
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN
	PGSIO_W_L0_TIME_NOT_FOUND
	PGSIO_W_L0_PHYSICAL_CLOSE
	PGSIO_E_L0_MANAGE_TABLE
	PGSIO_E_L0_PHYSICAL_OPEN
	PGSIO_E_L0_SEEK_PACKET
	PGSIO_M_L0_HEADER_CHANGED
	
REQUIREMENTS:
	PGSTK-0140, 0200, 0220, 0240

DETAILS:
	This tool will, if necessary, close the current physical file
	and open the correct physical file containing the next packet.
	It will update the internal table to reflect this and also notify
	the user via the return code.

GLOBALS:
        None

FILES:
	Physical files for the specified virtual data set

FUNCTIONS CALLED:
	PGS_IO_L0_ManageTable
	PGS_IO_Gen_Close
	PGS_IO_Gen_Open
	PGS_IO_L0_SeekPacket
	PGS_SMF_SetStaticMsg

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_SetStart(
	PGSt_IO_L0_VirtualDataSet  virtual_file,
	PGSt_double                start_time
	)
{
    static char *toolname = "PGS_IO_L0_SetStart()";

    PGSt_SMF_status  returnStatus  = PGS_S_SUCCESS;   /* return status */
    PGSt_SMF_status  returnStatus1 = PGS_S_SUCCESS;   /* return status */

    PGSt_IO_L0_FileTable         table_entry;

    PGSt_IO_L0_VersionTable      *version_array;

    PGSt_integer                 file_version;
    PGSt_integer                 packet_cnt;

    PGSt_integer                 seq_no;  /* time-ordered physical file index */

    int                          hdr_file;

    PGSt_boolean                 single_hdr;
    

    /* 
     *    Get table entry 
     */

    returnStatus = PGS_IO_L0_ManageTable(
					 PGSd_IO_L0_GetTableEntry, 
					 &virtual_file, 
					 &table_entry, 0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }


    switch (table_entry.spacecraft_tag)
    {
      case PGSd_EOS_AM:
	hdr_file = 1;
	single_hdr = PGS_TRUE;
	break;

      case PGSd_EOS_PM_GIIS:
	hdr_file = 1;
	single_hdr = PGS_TRUE;
	break;

      case PGSd_EOS_PM_GIRD:
	hdr_file = 1;
	single_hdr = PGS_TRUE;
	break;
	
      case PGSd_EOS_AURA:
        hdr_file = 1;
        single_hdr = PGS_TRUE;
        break;
 
      default:
	hdr_file = 0;	
	single_hdr = PGS_FALSE;
    }
    
    /* 
     *    Set up version array pointer
     */

    version_array = table_entry.version_array;



    /* 
     *    Find the file version which contains the first packet 
     *    at or after the specified start time.
     */

    if (start_time < version_array[hdr_file].start_time) /* error */
    {
        returnStatus = PGSIO_W_L0_TIME_NOT_FOUND; 
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
        return returnStatus;
    }

    for (seq_no=hdr_file; seq_no<table_entry.version_count;seq_no ++) 
    {
        if ( start_time <= version_array[seq_no].stop_time )
        {
            break;
        }
    }

    if (seq_no == table_entry.version_count)    /* error: time not found */
    {
        returnStatus = PGSIO_W_L0_TIME_NOT_FOUND; 
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
        return returnStatus;
    }


    /* 
     *    If necessary, close the current physical file and
     *    open the version containing the desired packet.
     *    Also, update the file table.
     */

    if (seq_no != table_entry.open_file_seq_no)
    {

        /* 
         *    Close the current physical file and set the open file stream 
         *    pointer to zero to indicate this, then update the file table.
         */

        if (table_entry.open_file_stream != 0)   /* if physical file is open */
        {

 	    returnStatus = PGS_IO_Gen_Close( table_entry.open_file_stream );

	    if (returnStatus != PGS_S_SUCCESS) 
	    {
                returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
                PGS_SMF_SetStaticMsg( returnStatus, toolname);
                return returnStatus;
	    }

        }


	table_entry.open_file_stream = 0;     /* flag file as closed */
	returnStatus = PGS_IO_L0_ManageTable(
					 PGSd_IO_L0_UpdateTableEntry, 
					 &virtual_file, 
					 &table_entry, 0);
	if (returnStatus != PGS_S_SUCCESS) 
	{
            returnStatus = PGSIO_E_L0_MANAGE_TABLE;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	

	/* 
	 *    Set up table entries to open the desired physical file
	 */
	
	table_entry.open_file_seq_no = seq_no;
	
	version_array = table_entry.version_array;
	
	file_version  = version_array[seq_no].file_version;
	

	/* 
	 *    Open the file and update the file table
	 */

	returnStatus = PGS_IO_Gen_Open(
				       table_entry.file_logical, 
				       PGSd_IO_Gen_Read,
				       &table_entry.open_file_stream,
				       file_version);
	if (returnStatus != PGS_S_SUCCESS) 
	{
            returnStatus = PGSIO_E_L0_PHYSICAL_OPEN;
            PGS_SMF_SetStaticMsg(returnStatus, toolname);
            return returnStatus;
 	}

	returnStatus = PGS_IO_L0_ManageTable(
					     PGSd_IO_L0_UpdateTableEntry, 
					     &virtual_file, 
					     &table_entry, 0);
	if (returnStatus != PGS_S_SUCCESS) 
	{
            returnStatus = PGSIO_E_L0_MANAGE_TABLE;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}

	if (single_hdr != PGS_TRUE)
	{
	    returnStatus = PGSIO_M_L0_HEADER_CHANGED;
	}
    }


    /* 
     *   Set file pointer  so the next call to PGS_IO_L0_GetPacket
     *   will return the desired packet .
     */

    returnStatus1 = PGS_IO_L0_SeekPacket(virtual_file, 0, &packet_cnt,
					 start_time);
    if (returnStatus1 != PGS_S_SUCCESS) 
    {
	if(returnStatus1 == PGSIO_W_L0_BITFLIP_IN_MICSEC)
	{
	    returnStatus = PGSIO_W_L0_BITFLIP_IN_MICSEC;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
	}
	else
	{
	    returnStatus = PGSIO_E_L0_SEEK_PACKET;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
	    return returnStatus;
	}
	
    }

    PGS_SMF_SetStaticMsg( returnStatus, toolname);

    return returnStatus;
}
