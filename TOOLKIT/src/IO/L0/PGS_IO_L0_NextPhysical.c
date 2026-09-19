/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_NextPhysical.c

DESCRIPTION:
  This file contains the I/O tool to open the next physical file version
  for the specified Level 0 Virtual Data Set.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Carol S. W. Tsai / Applied Research Corp.

HISTORY:
  30-Jan-1995 TWA  Initial version
  05-Mar-1997 CSWT Added an output parameter pack_ct for function
                   PGS_IO_L0_SeekPacket()

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Open the Next Physical File Version of this Virtual Data Set
	
NAME:
	PGS_IO_L0_NextPhysical
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_NextPhysical(
		PGSt_IO_L0_VirtualDataSet	virtual_file
	)

  FORTRAN:
	N/A

DESCRIPTION:

	This tool closes the current physical file and opens the correct 
	physical file containing the next packet.

INPUTS:
	virtual_file - 	The file descriptor for this virtual data set,
		returned by the call to PGS_IO_L0_Open.

OUTPUTS:
	NONE
	
NOTES:
	Normal return is PGS_S_SUCCESS.  If there are no more physical
	file versions to be opened, the return status is set to the 
	value PGSIO_W_L0_END_OF_VIRTUAL_DS.  The data set may 
	then be re-read by using a call to PGS_IO_L0_SetStart or it 
	may be closed with a call to PGS_IO_L0_Close.
	
EXAMPLES:
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_W_L0_END_OF_VIRTUAL_DS
	PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN
	PGSIO_W_L0_PHYSICAL_CLOSE
	PGSIO_E_L0_PHYSICAL_OPEN
	PGSIO_E_L0_MANAGE_TABLE
	PGSIO_E_UNIX
	
REQUIREMENTS:
	PGSTK-0190, 0200, 0235

DETAILS:
	The tool will update the internal file table to reflect which file
	is currently open.

GLOBALS:
        None

FILES:
	Physical files for the specified virtual data set

FUNCTIONS CALLED:
	PGS_IO_Gen_Close
	PGS_IO_Gen_Open
	PGS_IO_L0_ManageTable
	PGS_IO_L0_SeekPacket
	PGS_SMF_SetStaticMsg

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_NextPhysical(
	PGSt_IO_L0_VirtualDataSet	virtual_file
	)
{

    static char *toolname = "PGS_IO_L0_NextPhysical";

    PGSt_SMF_status returnStatus = PGS_S_SUCCESS;   /* return status */

    PGSt_IO_L0_FileTable         table_entry;

    PGSt_IO_L0_VersionTable      *version_array;

    PGSt_integer                 packet_ct;

    PGSt_integer                 file_version;


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


    /*
     *    Check to see if any more file versions are left.
     *    If not, return with status set to indicate the
     *    end of the virtual data set.
     */

    if (table_entry.open_file_seq_no + 1 >= table_entry.version_count)
    {
        returnStatus = PGSIO_W_L0_END_OF_VIRTUAL_DS;
        return returnStatus;
    }
    

    /* 
     *    Close the current physical file and set the open file stream 
     *    pointer to zero to indicate this, then update the file table.
     */

    if (table_entry.open_file_stream == 0)    /* no physical file is open */
    {
   	returnStatus = PGSIO_E_L0_PHYSICAL_NOT_OPEN;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    returnStatus = PGS_IO_Gen_Close( table_entry.open_file_stream );

    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    table_entry.open_file_stream = 0; 

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
     *    Set up table entries to open the next physical file
     */

    table_entry.open_file_seq_no++;

    version_array = table_entry.version_array;

    file_version =
      version_array[table_entry.open_file_seq_no].file_version;

    /*
    *    Reset counter for num packets read in file.
    */
    version_array[table_entry.open_file_seq_no].num_pkts_read = 0L;    

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
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
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


    /* 
     *    Set file pointer so the next call to PGS_IO_L0_GetPacket will 
     *    read the first packet in the newly-opened physical file.
     */

    returnStatus = PGS_IO_L0_SeekPacket(virtual_file, 1, &packet_ct, 0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
	returnStatus = PGSIO_E_L0_SEEK_PACKET;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    return returnStatus;
}

