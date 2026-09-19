/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_ManageTable.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to manage the virtual to physical 
  file mapping table.

AUTHOR:
  Mike Sucher / Applied Research Corp.

HISTORY:
  30-Jan-1995 TWA  Initial version
  06-Jul-1999  RM  updated for thread-safe functionality

END_FILE_PROLOG
*****************************************************************************/

#include <PGS_IO.h>
#include <PGS_TSF.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Manage Virtual to Physical File Mapping Table 
	
NAME:
	PGS_IO_L0_ManageTable
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_ManageTable(
		PGSt_integer		command,
		PGSt_IO_L0_VirtualDataSet *file_table_index,
		PGSt_IO_L0_FileTable	*table_entry,
		PGSt_IO_L0_FileTable	**table_pointer)

  FORTRAN:
	N/A

DESCRIPTION:
	This tool maintains a the table needed to keep track of the
	state of an open virtual data set.

	
INPUTS:
	command - operation to perform:
		PGSd_IO_L0_InitTable		
		PGSd_IO_L0_AddTableEntry		
		PGSd_IO_L0_DeleteTableEntry		
		PGSd_IO_L0_UpdateTableEntry		
		PGSd_IO_L0_GetTableEntry		
		PGSd_IO_L0_GetTablePointer		
	
	file_table_index - index to desired table entry for the commands:
		PGSd_IO_L0_UpdateTableEntry
		PGSd_IO_L0_DeleteTableEntry		
		PGSd_IO_L0_GetTableEntry		

	table_entry - table entry information for the commands:
		PGSd_IO_L0_AddTableEntry
		PGSd_IO_L0_UpdateTableEntry

OUTPUTS:
	file_table_index - the index to new table entry from the command:
		PGSd_IO_L0_AddTableEntry

		This value becomes the handle used to refer to the
		virtual data set opened by a call to PGS_IO_L0_Open.  

	table_entry - table entry information from the command:
		PGSd_IO_L0_GetTableEntry

	table pointer - pointer to the file table from the command:	
		PGSd_IO_L0_GetTablePointer		
	

NOTES:
	This is a low-level tool.  It is not intended to be called directly by 
	any user code.

	All file table values can be managed via the following command subset:

		PGSd_IO_L0_AddTableEntry		
		PGSd_IO_L0_DeleteTableEntry		
		PGSd_IO_L0_UpdateTableEntry		
		PGSd_IO_L0_GetTableEntry		

	The command PGSd_IO_L0_GetTablePointer returns a copy of the pointer to
	the file table itself.   This gives the calling program direct access to
	all the data in the file table,  which eliminates the overhead of calling
	PGS_IO_L0_ManageTable every time a value is needed.   Currently the only
	tool which uses this is PGS_IO_L0_GetPacket.  THIS METHOD OF ACCESSING 
	THE FILE TABLE SHOULD ONLY BE DONE IN CASES WHERE SPEED IS ESSENTIAL.
	IMPROPER USE COULD CORRUPT THE FILE TABLE!
	
	Command PGSd_IO_L0_InitTable is a table re-initialization command, reserved
	for future use.  The file table is automatically initialized the first time 
	this tool is called.

EXAMPLES:
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_E_L0_UNUSED_TABLE_ENTRY
	PGSIO_E_L0_INDEX_OUT_OF_RANGE
	PGSIO_E_L0_MAX_OPEN_EXCEEDED
	PGSTSF_E_GENERAL_FAILURE

REQUIREMENTS:
	PGSTK-0235, 0240

DETAILS:
	The file table is allocated in this routine as an array of type
	PGSt_IO_L0_FileTable, (defined in PGS_IO_L0.h).   It is used by
	all the Level 0 toolkit routines to implement the virtual data
	set interface.   It tracks such information as the number of
	physical file versions, which physical file is currently open,
	the stream pointer used for I/O calls to the file and other items.

GLOBALS:
	PGSg_TSF_IOL0InitFlag

FILES:
	NONE

FUNCTIONS CALLED:
	PGS_SMF_SetStaticMsg
	PGS_SMF_TestErrorLevel
	PGS_TSF_GetTSFMaster
	PGS_TSF_GetMasterIndex

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_ManageTable(
	PGSt_integer		command,
	PGSt_IO_L0_VirtualDataSet *file_table_index,
	PGSt_IO_L0_FileTable	*table_entry,
	PGSt_IO_L0_FileTable	**table_pointer)
{
    static char *toolname = "PGS_IO_L0_ManageTable";
    
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */

 
    PGSt_integer i;


#ifdef _PGS_THREADSAFE
    /*
     *    Set up non-statics and TSF globals
     */
    PGSt_IO_L0_FileTable *file_table;
    PGSt_integer init_flag;
    extern PGSt_integer PGSg_TSF_IOL0InitFlag[];
    int masterTSFIndex;
    PGSt_TSF_MasterStruct *masterTSF;

    returnStatus = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if ((PGS_SMF_TestErrorLevel(returnStatus)) ||
        (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }
    masterTSFIndex = PGS_TSF_GetMasterIndex();

    /*
     *    Get data from TSD key and global counterpart
     */
    file_table = (PGSt_IO_L0_FileTable *) pthread_getspecific(
                             masterTSF->keyArray[PGSd_TSF_KEYIOL0FILETABLE]);
    init_flag = PGSg_TSF_IOL0InitFlag[masterTSFIndex];
#else
    static PGSt_IO_L0_FileTable file_table[PGSd_IO_L0_MaxOpenFiles];

    static PGSt_integer init_flag = 0;
#endif


    /*
     *    If this function is being called for the first time,
     *    then initialize the file table.
     */
    if (init_flag == 0)
    {
	for (i=0; i<PGSd_IO_L0_MaxOpenFiles ; i++)
	{
	    file_table[i].file_logical = 0;
	    file_table[i].version_count = 0;
            file_table[i].spacecraft_tag = 0;
	    file_table[i].open_file_seq_no = 0;
	    file_table[i].open_file_stream = 0;
	    file_table[i].version_array = 0;
	}
        init_flag = 1;
#ifdef _PGS_THREADSAFE
        /*
         *    Re-set global
         */
        PGSg_TSF_IOL0InitFlag[masterTSFIndex] = init_flag;
#endif
    }
    
    /*
     *    Execute the specified command.
     */
    switch (command)
    {
      case PGSd_IO_L0_InitTable:            /* initialize the file table */

	for (i=0; i<PGSd_IO_L0_MaxOpenFiles ; i++)
	{
	    file_table[i].file_logical = 0;
	    file_table[i].version_count = 0;
            file_table[i].spacecraft_tag = 0;
	    file_table[i].open_file_seq_no = 0;
	    file_table[i].open_file_stream = 0;
	    file_table[i].version_array = 0;
	}

	break;
	
      case PGSd_IO_L0_AddTableEntry:        /* add a table entry */

	for (i=0; i<PGSd_IO_L0_MaxOpenFiles ; i++)
	{
	    if (file_table[i].file_logical == 0) break;
	}

	if ( i == PGSd_IO_L0_MaxOpenFiles) 
	{
	    returnStatus = PGSIO_E_L0_MAX_OPEN_EXCEEDED;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	
	file_table[i].file_logical = table_entry->file_logical;
	file_table[i].version_count = table_entry->version_count;
        file_table[i].spacecraft_tag = table_entry->spacecraft_tag;
	file_table[i].open_file_seq_no = table_entry->open_file_seq_no;
	file_table[i].open_file_stream = table_entry->open_file_stream;
	file_table[i].version_array = table_entry->version_array;

	*file_table_index = i;
		
	break;
	
      case PGSd_IO_L0_DeleteTableEntry:     /* delete a table entry */
	
	i = *file_table_index;

	if (( i >= PGSd_IO_L0_MaxOpenFiles) || (i < 0) )
	{
	    returnStatus = PGSIO_E_L0_INDEX_OUT_OF_RANGE;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	
	if (file_table[i].file_logical == 0) 
	{
	    returnStatus = PGSIO_E_L0_UNUSED_TABLE_ENTRY;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}

	PGS_MEM_Free( file_table[i].version_array );
	
	file_table[i].file_logical = 0;
	file_table[i].version_count = 0;
        file_table[i].spacecraft_tag = 0;
	file_table[i].open_file_seq_no = 0;
	file_table[i].open_file_stream = 0;
	file_table[i].version_array = 0;

	break;
	
      case PGSd_IO_L0_UpdateTableEntry:    /* update a table entry */

	i = *file_table_index;

	if (( i >= PGSd_IO_L0_MaxOpenFiles) || (i < 0) )
	{
	    returnStatus = PGSIO_E_L0_INDEX_OUT_OF_RANGE;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	
	if (file_table[i].file_logical == 0) 
	{
	    returnStatus = PGSIO_E_L0_UNUSED_TABLE_ENTRY;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	
	file_table[i].file_logical = table_entry->file_logical;
	file_table[i].version_count = table_entry->version_count;
        file_table[i].spacecraft_tag = table_entry->spacecraft_tag;
	file_table[i].open_file_seq_no = table_entry->open_file_seq_no;
	file_table[i].open_file_stream = table_entry->open_file_stream;
	file_table[i].version_array = table_entry->version_array;

	break;
	
      case PGSd_IO_L0_GetTableEntry:    /* get copy of a table entry */

	i = *file_table_index;

	if (( i >= PGSd_IO_L0_MaxOpenFiles) || (i < 0) )
	{
	    returnStatus = PGSIO_E_L0_INDEX_OUT_OF_RANGE;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	
	if (file_table[i].file_logical == 0) 
	{
	    returnStatus = PGSIO_E_L0_UNUSED_TABLE_ENTRY;
	    PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
	}
	
        table_entry->file_logical = file_table[i].file_logical;
        table_entry->version_count = file_table[i].version_count;
        table_entry->spacecraft_tag = file_table[i].spacecraft_tag;
        table_entry->open_file_seq_no = file_table[i].open_file_seq_no;
        table_entry->open_file_stream = file_table[i].open_file_stream;
        table_entry->version_array = file_table[i].version_array;

	break;
	
      case PGSd_IO_L0_GetTablePointer:    /* get pointer to the file table */

	*table_pointer = file_table;
	
	break;
    
      default:
        returnStatus = PGSIO_E_L0_ILLEGAL_COMMAND;
        PGS_SMF_SetStaticMsg( returnStatus, toolname);
        return returnStatus;
    }
    
    return returnStatus;
}


