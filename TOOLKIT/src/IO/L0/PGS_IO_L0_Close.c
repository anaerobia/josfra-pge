/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_Close.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to close a virtual data set
  that was opened with a call to PGS_IO_L0_Open.

AUTHOR:
  Mike Sucher / Applied Research Corp.

HISTORY:
  30-Jan-1995 TWA  Initial version

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Close a Virtual Level 0 Data Set

NAME:
	PGS_IO_L0_Close
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_Close(
		PGSt_IO_L0_VirtualDataSet	virtual_file
		)
  FORTRAN:
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'

	integer function PGS_IO_L0_Open(
       +                         virtual_file)
	integer		virtual_file

DESCRIPTION:
	This tool closes a virtual data set opened by PGS_IO_L0_Open.
	
INPUTS:
	virtual_file - 	The file descriptor for this virtual data set,
		returned by the call to PGS_IO_L0_Open.

OUTPUTS:
	NONE

NOTES:
	If a physical file is currently open, PGS_IO_Gen_Close is called
	to close it.   Otherwise this step is skipped.  In either case,
	the return will be PGS_S_SUCCESS.

EXAMPLES:
	Close a virtual data set opened with a call to PGS_IO_L0_Open. 
	Go to exception handling if there was an error.
	
  C:
	PGSt_SMF_status returnStatus = PGS_S_SUCCESS;
	PGSt_IO_L0_VirtualDataSet  virtual_file;

	returnStatus = PGS_IO_L0_Close(virtual_file);
	if (returnStatus != PGS_S_SUCCESS) goto EXCEPTION;


  FORTRAN:
	implicit none
	INCLUDE   'PGS_SMF.f'
	INCLUDE   'PGS_PC.f'
	INCLUDE   'PGS_PC_9.f'
	INCLUDE   'PGS_TD.f'
	INCLUDE   'PGS_IO.f'
	INCLUDE   'PGS_IO_1.f'
	integer pgs_io_l0_close
	integer returnstatus
	integer virtual_file

	returnstatus = pgs_io_l0_close(virtual_file)
	if (returnstatus != PGS_S_SUCCESS) goto 9999


RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_VIRTUAL_DS_NOT_OPEN
	PGSIO_E_L0_MANAGE_TABLE
	PGSIO_W_L0_PHYSICAL_CLOSE
	
REQUIREMENTS:
	PGSTK-0140, 0190

DETAILS:
	The tool closes the physical file that is currently open for the
	specified virtual data set.  It then calls PGS_IO_L0_ManageTable
	to delete the internal table entry that was set up to handle the
	virtual to physical file mapping by PGS_IO_L0_Open.

GLOBALS:
        None

FILES:
	Physical files for the specified virtual data set

FUNCTIONS CALLED:
	PGS_IO_Gen_Close
	PGS_IO_L0_ManageTable
	PGS_SMF_SetStaticMsg

END_PROLOG
*****************************************************************************/


PGSt_SMF_status 
PGS_IO_L0_Close(
	PGSt_IO_L0_VirtualDataSet	virtual_file
	)
{
    static char *toolname = "PGS_IO_L0_Close";

    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */

    PGSt_IO_L0_FileTable         table_entry;


    /* 
     *    Get file table entry 
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
     *    Close the currently opened physical file, if needed
     */

    if (table_entry.open_file_stream != 0)     /* do only if file is open */
    {
        returnStatus = PGS_IO_Gen_Close( table_entry.open_file_stream );

        if (returnStatus != PGS_S_SUCCESS) 
        {
            returnStatus = PGSIO_W_L0_PHYSICAL_CLOSE;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
        }

    /* Reset num pkts read counter */

    table_entry.version_array[ table_entry.open_file_seq_no ]
       .num_pkts_read = 0L;

    }


    /*
     *    Delete entry in the file table
     */

    returnStatus = PGS_IO_L0_ManageTable(
					 PGSd_IO_L0_DeleteTableEntry, 
					 &virtual_file,  0, 0);
    if (returnStatus != PGS_S_SUCCESS) 
    {
  	returnStatus = PGSIO_E_L0_MANAGE_TABLE;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    PGS_SMF_SetStaticMsg( returnStatus, toolname);
    return returnStatus;
}
