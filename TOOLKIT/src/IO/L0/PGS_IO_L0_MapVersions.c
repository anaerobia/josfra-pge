/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG

FILENAME:
  PGS_IO_L0_MapVersions.c

DESCRIPTION:
  This file contains the Level 0 I/O tool to map physical file versions to 
  time sequenced file versions.

AUTHOR:
  Mike Sucher / Applied Research Corp.
  Tom W. Atwater / Applied Research Corp.
  Guru Tej S. Khalsa / Applied Research Corp.
  Abe Taaheri / SM&A Corp.
  Xin Wang / EIT Inc.

HISTORY:
               MS  Designed
  30-Jan-1995 TWA  Initial version
  15-Jul-1995 GTSK Fixed up physical file version ordering scheme.
  21-Jul-1995 GTSK Initialized the pointer "array" to NULL.
  01-Dec-1999 AT   Modified to support GIRD and GIIS formats for PM spacecraft
  13-Mar-2001 AT   Modified so that for AM & PM the header records in 
                   the first (0) version is not sorted.
  01-Nov-2001 XW   Modified to support AURA spacecraft

END_FILE_PROLOG
*****************************************************************************/

#include <memory.h>
#include <string.h>
#include <PGS_IO.h>

/*****************************************************************************
BEGIN_PROLOG

TITLE:
	Map physical file versions to time sequenced file versions
	
NAME:
	PGS_IO_L0_MapVersions
	
SYNOPSIS:

  C:
	#include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_L0_MapVersions(
		PGSt_PC_Logical		file_logical,
		PGSt_tag		spacecraft_tag,
		PGSt_integer		*version_count,
		PGSt_IO_L0_VersionTable	**version_array
		)
  FORTRAN:
	N/A

DESCRIPTION:
	This tool determines how many physical files are associated with the
	Level 0 virtual data set specified by file_logical.  It then allocates
	and populates the array that will contain all of the version mapping 
	information for each of the physical files.

INPUTS:
	file_logical  - The logical file descriptor for this virtual data 
		set.

	spacecraft_tag - The tag identifying which of the supported spacecraft
		platforms generated this virtual data set.  The following types
		are currently defined:

			PGSd_EOS_AM  	    the EOS AM platform
			PGSd_EOS_PM_GIIS    the EOS PM platform (GIIS)
			PGSd_EOS_PM_GIRD    the EOS PM platform (GIRD)
			PGSd_TRMM    	    the TRMM platform
			PGSd_ADEOS_II       the ADEOS-II platform
OUTPUTS:
	version_count -	The number of physical file versions staged
		for this virtual data set.

	version_array - A dynamically allocated array that contains the 
		version numbers and other information for each physical 
		file, sorted by time sequence, from earliest to latest.

NOTES:
	This tool does dynamic memory allocation for the version mapping
	arrays.  It is intended only for use as a support routine for the 
	tool  PGS_IO_L0_Open.  The low-level tool PGS_IO_L0_ManageTable 
	will deallocate the arrays when no longer needed, i.e. when the 
	virtual data set is closed by a call to PGS_IO_L0_Close.

	IMPROPER USE OF THIS TOOL COULD CREATE MEMORY LEAKS.

EXAMPLES:
	

RETURNS:
	PGS_S_SUCCESS
	PGSIO_W_L0_CORRUPT_FILE_HDR
	PGSIO_E_L0_MEM_ALLOC
	PGSIO_E_L0_VERSION_COUNT
	PGSIO_E_L0_VERSION_INFO

REQUIREMENTS:
	PGSTK-0200, 0235

DETAILS:
	This tool will call the appropriate I/O and Process Control tools to
	get attribute information including the total number of physical
	files staged and the time sequence of each physical file.  It
	will then allocate an array that maps file sequence number to 
	physical file version number.

GLOBALS:
        None

FILES:
	Physical files for the specified virtual data set
	Process Control File (PCF)

FUNCTIONS CALLED:
	PGS_IO_L0_FileVersionInfo
	PGS_MEM_Calloc
	PGS_PC_GetNumberOfFiles
	PGS_SMF_SetStaticMsg

END_PROLOG
*****************************************************************************/

PGSt_SMF_status 
PGS_IO_L0_MapVersions(
	PGSt_PC_Logical		file_logical,
	PGSt_tag		spacecraft_tag,
	PGSt_integer		*version_count,
	PGSt_IO_L0_VersionTable	**version_array
	)
{
    static char *toolname = "PGS_IO_L0_MapVersions()";
    
    PGSt_SMF_status  returnStatus = PGS_S_SUCCESS;   /* return status */

    PGSt_IO_L0_VersionTable	*array=NULL;

    PGSt_IO_L0_VersionTable	sort_buf;

    PGSt_integer		local_v_count;

    PGSt_integer		base;

    PGSt_integer		next;

    PGSt_integer		file_version;

    PGSt_integer		i;

    PGSt_integer                first_version_to_sort;

    size_t                      swap_size;

    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */
 
    PGSt_SMF_status     caller;         /* determines who's calling who */
 
    /*------------------------------------------------------
        Get Toolkit CallerId
        if SMF is caller do NOT make SMF calls!
    ------------------------------------------------------*/
    caller = PGS_SMF_CallerID();
 
    /*------------------------------------------------------*/


    /*
     *    Get number of file versions
     */

    local_v_count = 1;

    returnStatus = PGS_PC_GetNumberOfFiles( file_logical, &local_v_count);

    if (returnStatus != PGS_S_SUCCESS ||
	((spacecraft_tag == PGSd_EOS_AM || 
	  spacecraft_tag == PGSd_EOS_PM_GIIS || 
	  spacecraft_tag == PGSd_EOS_PM_GIRD ||
          spacecraft_tag == PGSd_EOS_AURA) && local_v_count == 1))
    {
	returnStatus = PGSIO_E_L0_VERSION_COUNT;
         if (caller != PGSd_CALLERID_SMF)
           {
            sprintf(newMsg,"LID %d does not include a field for number of file versions", (int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg,toolname );
           }

	return returnStatus;
    }

    *version_count = local_v_count;  /* store in ouput parameter */


    /*
     *    Allocate the version array and pass the address back to
     *    the calling program.
     */

    returnStatus = PGS_MEM_Calloc( (void **) &array, local_v_count, 
                                   sizeof(PGSt_IO_L0_VersionTable) );
    if (returnStatus != PGS_S_SUCCESS)
    {
 	returnStatus = PGSIO_E_L0_MEM_ALLOC;
	PGS_SMF_SetStaticMsg( returnStatus, toolname);
	return returnStatus;
    }

    *version_array = array;         /* store in ouput parameter */

    /*
     *    Get packet count, start and stop times for each file version
     *    Use them to populate the version array
     */

    for (i=0; i<local_v_count; i++)
    {
        file_version = i+1;
        returnStatus = 
            PGS_IO_L0_FileVersionInfo( file_logical, file_version,
                                       spacecraft_tag, &array[i]);
        if (returnStatus != PGS_S_SUCCESS)
        {
            returnStatus = PGSIO_E_L0_VERSION_INFO;
            PGS_SMF_SetStaticMsg( returnStatus, toolname);
            return returnStatus;
        }
    }


    /*
     *    Sort the version array by start time, from earliest to latest
     */
    /*
     * AM & PM have header records in the first (0) version, so don't sort that
     * one
     */
    if ( spacecraft_tag == PGSd_EOS_AM ||
	 spacecraft_tag == PGSd_EOS_PM_GIIS ||
	 spacecraft_tag == PGSd_EOS_PM_GIRD ||
         spacecraft_tag == PGSd_EOS_AURA)
      {
	first_version_to_sort = 1;
      }
    else
      {
	first_version_to_sort = 0;
      }
    
    for (base=first_version_to_sort; base<local_v_count; base++)    /* step through each element */
    {

        /* 
         *    Get next earliest element
         */
        for (i=base, next=base; i<local_v_count; i++) 
        {
            if (array[i].start_time < array[next].start_time)
            {
                next=i;
            }
        }

        /* 
         *    Swap into position, if needed
         */
        if (next != base)
        {
	    swap_size = sizeof(PGSt_IO_L0_VersionTable);

	    memcpy((char*) &sort_buf, (char*) &array[base], swap_size);
	    memcpy((char*) &array[base], (char*) &array[next], swap_size);
	    memcpy((char*) &array[next], (char*) &sort_buf, swap_size);
	}

    }

    if (returnStatus == PGS_S_SUCCESS)
	returnStatus = PGS_IO_L0_VersionInfoCheck(&array[0],spacecraft_tag);
    
    return returnStatus;
}


