/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*****************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_IO_Gen_Close.c

DESCRIPTION:
  This file contains the "C" version of the Generic I/O close tool.

AUTHOR:
  Charles Ruedi / Applied Research Corp.
  Phuong T Nguyen/ L3 Communication Corp.
HISTORY:
  18-Mar-1994 CR  	Initial version
  31-Mar-1994 MES 	Standardize prologs, SMF support, add Generic source flag
  03-Aug-1994 DPH 	Conform to new standards for TK3. Fix call to support 
			high-level handle returned from Open tools.
  12-Aug-1994 DPH 	Code inspection fixes.
  30-Sep-1994 DPH 	Added Warning about attempted closure of non-initialized
			file pointers.
  17-Jun-2002 PTN       Modified to avoid segmentation faults for linux.

END_FILE_PROLOG:
*****************************************************************************/

#define  PGSd_IO_Gen_c          /* flag PGS_IO.h that this is Generic 
                                   tools source code */
#include <PGS_IO.h>

/****************************************************************************
BEGIN_PROLOG:

TITLE:  
	Close a Generic, Temporary, or Intermediate file

NAME:	
	PGS_IO_Gen_Close

SYNOPSIS:
C:	
	#include <PGS_IO.h>

	PGSt_SMF_status
	PGS_IO_Gen_Close(
    		PGSt_IO_Gen_FileHandle 	*file_handle);
Fortran:
	(not applicable)

DESCRIPTION:
        This tool closes a stream opened by a call to the "C" version of the 
	Generic I/O Open tools.

INPUTS:
	file_handle - file handle returned by PGS_IO_Gen_Open or PGS_IO_Gen_Temp_Open.

OUTPUTS:
        NONE

RETURNS:
	PGS_S_SUCCESS			Success
	PGSIO_E_GEN_CLOSE		Error closing file

EXAMPLES:
        PGSt_IO_Gen_FileHandle 		*handle;
        PGSt_SMF_status			returnStatus;

        returnStatus = PGS_IO_Gen_Close( handle );
        if (returnStatus != PGS_S_SUCCESS) 
	{
		goto EXCEPTION;
	}
	else
	{
		.
		.
		.
	}

	EXCEPTION:

NOTES:	
        Usage of this tool is optional, but failure to close a file could 
        result in loss of data, destroyed files, or possible intermittent 
        errors in your program.

	As a consequence of calling this tool, any data left unwritten in the output 
	buffer will be flushed to the output stream; likewise, any data left unread 
	in the input buffer will be discarded.

	!!!!!!!!!!! Never attempt to close a file which has not been initialized,
	!! ALERT !! or previously used in an open call. Failure to heed this 
	!!!!!!!!!!! warning will result in program abort on many platforms.

REQUIREMENTS:	
        PGSTK-0360

DETAILS:
       This function calls the C POSIX fclose() routine.

GLOBALS:	
        None

FILES:
	None

FUNCTIONS_CALLED:
        PGS_SMF_SetStaticMsg     	Register status condition with SMF

END_PROLOG:
*****************************************************************************/

PGSt_SMF_status
PGS_IO_Gen_Close(
    PGSt_IO_Gen_FileHandle 	*file_handle) 	/* handle of file to close */

{
    PGSt_SMF_status	returnStatus;
    returnStatus = PGS_S_SUCCESS;

    /* Clean up any outstanding errors on the stream (since, presumably, that
       information is no longer needed).  This is especially relevant on HPs
       since the fclose() statement below may otherwise fail on that platform
       (if the user has performed an erroneous operation on the file while it
       was open). */

        if (file_handle != NULL)
    {
	clearerr(file_handle);
    }
    
    /*The linux gets segmentation faults when it close the file that is NULL, so here it's changed to return the error message*/
        if (file_handle == NULL)
	  {
          #if defined(LINUX) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL)
	       {
	         returnStatus = PGSIO_E_GEN_CLOSE;
	       }
	       /* Check to see if any errors while closing file,
	       if any, go to exception handling, otherwise return success. */
         #else
	      if (fclose( file_handle )  != 0)
	         {	
		   returnStatus = PGSIO_E_GEN_CLOSE;
	         }
         #endif
	  }
	else 
	  {
	    returnStatus = fclose( file_handle );
	    if (returnStatus != 0)
	      { 
		returnStatus = PGSIO_E_GEN_CLOSE;
	      }
	    else
	      {
		file_handle = NULL;
	      }
	  }
	
        /* set message and return error */

	PGS_SMF_SetStaticMsg( returnStatus,"PGS_IO_Gen_Close" );
	return( returnStatus );
}
