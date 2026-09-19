/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG

FILENAME:   
	PGS_IO_Gen_Temp_Delete.c

DESCRIPTION:
        The tool listed in this file provides the user with means to delete
	temporary files.

AUTHOR:
	David P. Heroux / Applied Research Corp.

HISTORY:
	14-Apr-1994 DPH Initial Version
	29-Aug-1994 DPH Upgraded to TK3; minor fixes to function prolog
	30-Aug-1994 DPH Removed references to old SMF files in Fortran prolog
			Changed the name of the file to match the function name
	07-Sep-1994 TWA Changed LONG to INT in FCALLSCFUN1 fortran binding
	22-Sep-1994 DPH changed return_value to returnStatus according to Toolkit
			convention.
			Now screens PC call for PGSPC_W_FILE_NOT_ON_DISK status
			and sets IO parallel IO warning.
	04-Oct-1994 DPH Setting success state on exit.
	06-Oct-1994 DPH Removed all references which stated that Intermediate
			files can be removed with this tool.
	10-Feb-1995 TWA Deleting unused label to eliminate compiler warnings
	14-Jul-1995 DPH Updating prolog to reflect discontinued use of 
			environment variables.

END_FILE_PROLOG
***************************************************************************/

#include <PGS_IO.h>
#ifdef	MIN
#undef MIN		/* cfortran.h doesn't like this in the IBM */
#endif
#include <cfortran.h>

/****************************************************************************
BEGIN_PROLOG

TITLE: 
	Permanently delete a Temporary File
 
NAME:	
	PGS_IO_Gen_Temp_Delete

SYNOPSIS:
C: 	
	#include <PGS_IO.h>

        PGSt_SMF_status
	PGS_IO_Gen_Temp_Delete(
    		PGSt_PC_Logical         file_logical)

FORTRAN:
	include 'PGS_SMF.f'
                'PGS_PC_9.f'
                'PGS_IO_1.f'

	integer pgs_io_gen_temp_delete(
		integer	      file_logical)

DESCRIPTION: 	
	Upon a successful call, this function will "effectively" delete the 
	Temporary file currently referenced by the specified logical identifier 
	(see NOTES). Future references to this logical identifier will no longer 
	provide access to a file until such time as a new Temporary file is 
	created with the same logical identifier.

INPUTS:		
	file_logical 	- User defined logical file identifier

OUTPUTS:	
	NONE

RETURNS: 
	PGS_S_SUCCESS
	PGSIO_E_GEN_REFERENCE_FAILURE
	PGSIO_E_GEN_FILE_NODEL
	PGSIO_W_GEN_FILE_NOT_FOUND

EXAMPLE:
	PGSt_SMF_status			ret_val;
	PGSt_PC_Logical			logical;

	#define	INTER_1B 101

	ret_val = PGS_IO_Gen_Temp_Delete( INTER_1B )
	if (ret_val != PGS_S_SUCCESS)
	{
	    goto EXCEPTION;
	}
	    .
	    .
	    .
	EXCEPTION:

NOTES:		
	The actual deletion of Temporary files is not carried-out until after 
	the completion of the PGE run. Instead, these files are marked as deleted
	through the Process Control mechanism. This allows for the preservation
	of all Temporary files generated during a PGE run, to facilitate error
	tracking/debugging following a failed run of a PGE. This in no way 
	prevents the creation of a new temporary file using the same logical
	identifier as one previously deleted.
	
	Unlike all other IO_Gen tools, this function has a FORTRAN binding to C. 
	There is no separate FORTRAN version.

        Logical identifiers used for Temporary and Intermediate files may NOT be
        duplicated.

        By using this tool, the user understands that a truly Temporary file may
        only exist for the duration of a PGE. Whether or not the user deletes
        this file prior to PGE termination, it will be purged by the SDPS system
        during normal cleanup operations.

        IMPORTANT TK5 NOTES
        -------------------
        The following environment variable MUST be set to assure proper operation:
 
                PGS_PC_INFO_FILE        path to process control file
 
        However, the following environment variable is NO LONGER recognized by
        the Toolkit as such:
 
                PGS_TEMPORARY_IO        default path to temporary files
 
        Instead, the default path, which was defined by this environment variable
        in previous Toolkit releases, may now be specified as part of the Process
        Control File (PCF). Essentially, this variable has been replaced by a global 
	path statement for TEMPORARY IO subject fields within the PCF. To define a 
	global path statement, simply create a record which begins with the
        '!' symbol defined in the first column, followed by the global path to be
        applied to each of the records within the TEMPORARY IO subject field. Only 
	one such statement can be defined per subject field and it must be appear 
	prior to any dependent subject entry.
 
REQUIREMENTS:	
	PGSTK-0520

DETAILS: 	
	Entries (1 or more) in the Process Control file that are associated
	with the specified logical identifier will be removed prior to the deletion
	of the actual files.

GLOBALS:
	None

FILES:
	None

FUNCTIONS_CALLED:
	PGS_PC_PutPCSData
	PGS_SMF_SetStaticMsg

END_PROLOG
***************************************************************************/

PGSt_SMF_status PGS_IO_Gen_Temp_Delete(
    PGSt_PC_Logical 	    file_logical) 	/* file logical */
{   
    PGSt_SMF_status 	returnStatus; 		/* PGS status return value */
    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */
 
    PGSt_SMF_status     caller;         /* determines who's calling who */
 
/*------------------------------------------------------
        Get Toolkit CallerId
        if SMF is caller do NOT make SMF calls!
    ------------------------------------------------------*/
    caller = PGS_SMF_CallerID();

    /*------------------------------------------------------
   	Initialize 
    ------------------------------------------------------*/
    returnStatus = PGS_S_SUCCESS; 		/* success */

    /*------------------------------------------------------
     	Issue delete instruction to Process Control    
    ------------------------------------------------------*/
    returnStatus = PGS_PC_PutPCSData( PGSd_PC_DELETE_TEMP,(void *) file_logical );
    if (returnStatus != PGS_S_SUCCESS)
    {
	if (returnStatus == PGSPC_W_NO_FILES_EXIST)
	{
            returnStatus = PGSIO_E_GEN_FILE_NODEL;
	}
	else if (returnStatus == PGSPC_W_FILE_NOT_ON_DISK)
	{
            returnStatus = PGSIO_W_GEN_FILE_NOT_FOUND;
	}
	else
	{
            returnStatus = PGSIO_E_GEN_REFERENCE_FAILURE;
	}
    }

    switch (returnStatus)
    {
    case PGSIO_E_GEN_FILE_NODEL:
        if (caller != PGSd_CALLERID_SMF)
        {
            sprintf(newMsg,"Can not delete file with LID %d. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Delete");
 
        }
        break;
    case PGSIO_W_GEN_FILE_NOT_FOUND:
       if (caller != PGSd_CALLERID_SMF)
        {
            sprintf(newMsg,"File with LID %d does not exist. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Delete");
 
        }
        break;
    case PGSIO_E_GEN_REFERENCE_FAILURE:
       if (caller != PGSd_CALLERID_SMF)
        {
            sprintf(newMsg,"Can not find Physical File Name for LID %d. ",(int)file_logical);
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            strcat(newMsg,msg);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Delete");
        }
       break;
    }
    return( returnStatus );
}

/* set up MACRO to be expanded by cfortran.h depending on system type. */

FCALLSCFUN1(INT,PGS_IO_Gen_Temp_Delete,PGS_IO_GEN_TEMP_DELETE,pgs_io_gen_temp_delete,INT)
