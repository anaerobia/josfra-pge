/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG

FILENAME:   
	PGS_IO_Gen_Temp_Open.c

DESCRIPTION:
        The tool listed in this file provides the user with the means to open
	temporary or inmtermediate files.

AUTHOR:
	David P. Heroux / Applied Research Corp.

HISTORY:
	31-Mar-1994 DPH Initial Version

	22-Jul-1994 DPH File reference/creation code replaced with call to
		    	PGS_IO_Gen_Temp_Reference
  	12-Aug-1994 DPH Code inspection fixes.
  	12-Aug-1994 TWA Misc. prolog changes
	29-Aug-1994 DPH	Upgraded for TK3: Disabled use of the standard C
			access mode truncate (w+).
	29-Sep-1994 DPH Added Warning to prolog concerning AppendUpdate (a+) mode.
			Clarified prolog notes concerning file_duration modes.
                        Added details concerning temporary name creation.
	04-Jan-1995 DPH Updated prolog for TK4
	14-Jul-1995 DPH Updated prolog for TK5
			-- Environment variables no longer recognized
	11-Jun-1998 MEDS Updated information on File characteristics

END_FILE_PROLOG
***************************************************************************/

#include <PGS_IO.h>

/****************************************************************************
BEGIN_PROLOG

TITLE: 
	Open a Temporary or Intermediate File
 
NAME:	
	PGS_IO_Gen_Temp_Open

SYNOPSIS:
C: 	
	#include <PGS_IO.h>

        PGSt_SMF_status
	PGS_IO_Gen_Temp_Open(
    		PGSt_IO_Gen_Duration    file_duration,
    		PGSt_PC_Logical         file_logical,
    		PGSt_IO_Gen_AccessType  file_access,
    		PGSt_IO_Gen_FileHandle  **file_handle)
Fortran:	
	(not applicable)

DESCRIPTION: 	
	This routine lets the user create and open Temporary and Intermediate
	files with a variety of access modes. The returned argument
	PGSt_IO_Gen_FileHandle is directly compatible with the standard "C" library
	stream I/O manipulation routines.

INPUTS:		
	file_duration 	- PGSd_IO_Gen_Endurance		// Creates Intermediate File //
			  PGSd_IO_Gen_NoEndurance	// Creates Temporary File    //

	file_logical 	- User defined logical file identifier

  	file_access 	- type of access granted to opened file:
	   Toolkit		C	Description
	   ------		-	-----------
	   PGSd_IO_Gen_Read	"r"	Open file for reading
	   PGSd_IO_Gen_Write	"w"	Open file for writing, truncating existing
					file to 0 length, or creating a new file
	   PGSd_IO_Gen_Append	"a"	Open file for writing, appending to the
					end of existing file, or creating file
	   PGSd_IO_Gen_Update	"r+"	Open file for reading and writing
	   PGSd_IO_Gen_Append   "a+"	Open file for reading and writing,
		       Update		to the end of existing file,
					or creating a new file; whole file
					can be read, but writing only appended


OUTPUTS:	
	file_handle	- used to manipulate files with other "C" library stream 
			  I/O routines.

RETURNS: 
	PGS_S_SUCCESS			Success
	PGSIO_W_GEN_ACCESS_MODIFIED 	Illegal attempt to open existing file for access mode 
					PGSd_IO_Gen_Write
					Access mode reset to PGSd_IO_Gen_AppendUpdate
	PGSIO_W_GEN_NEW_FILE		File expected, but was missing; new file created
	PGSIO_W_GEN_DURATION_NOMOD	Attempt to alter existing intermediate file duration ignored
	PGS_E_UNIX			Unix system error
	PGSIO_E_GEN_OPENMODE		Invalid access mode
	PGSIO_E_GEN_REFERENCE_FAILURE 	Other error accessing $PGS_PC_INFO_FILE
	PGSIO_E_GEN_BAD_FILE_DURATION 	Invalid file duration
	PGSIO_E_GEN_FILE_NOEXIST	No entry for file logical in $PGS_PC_INFO_FILE
	PGSIO_E_GEN_CREATE_FAILURE	Error creating new file entry in $PGS_PC_INFO_FILE
	PGSIO_E_GEN_NO_TEMP_NAME	Failed to create temporary filename
	PGSIO_E_GEN_BAD_ENVIRONMENT     Bad Environment detected for I/O path ...

	"Existing file" means that an entry for the file exists in  $PGS_PC_INFO_FILE.
	(NOTE: the above are short  descriptions only; full text of messages appears in files
	$PGSMSG/PGS*.t . Descriptions may change in future releases depending on external ECS
	design.)


EXAMPLE:
	// This example illustrates how to create an Intermediate File //

	PGSt_SMF_status			returnStatus;
	PGSt_PC_Logical			logical;
	PGSt_IO_Gen_FileHandle 		*handle;

	#define	INTER_1B 101

	returnStatus = PGS_IO_Gen_Temp_Open( PGSd_IO_Gen_Endurance,INTER_1B,
				   	     PGSd_IO_Gen_Write,&handle );
	if (returnStatus != PGS_S_SUCCESS)
	{
	    goto EXCEPTION;
	}
	    .
	    .
	    .
	EXCEPTION:

NOTES:		
	This function will support most POSIX modes of fopen; the only exception
	being truncate mode (w+). 

	Logical identifiers used for files may NOT be duplicated. 

	Existing files will NOT be overwritten by calling this function in 
	mode PGSd_IO_Gen_Write (w). Instead, they will be opened in 
	PGSd_IO_AppendUpdate mode; a warning will be issued signifying that 
	this is the case. Warnings will also be issued in the event that a 
	non-existent file is opened in modes other than explicit write 
	(i.e., PGSd_IO_Gen_Append, or PGSd_IO_Gen_AppendUpdate).

	!!!!!!!!!!! During testing of this tool, the mode AppendUpdate (a+)
	!! ALERT !! was found to produce results that were not consistent
	!!!!!!!!!!! with the documented POSIX standard.The sort of behavior
        that was typically observed was for data, buffered during a read 
	operation, to be appended to the file along with other data that was 
	being written to the file. Note that this behavior could not be attributed 
	to the Toolkit since the same behavior was revealed when purely "POSIX" 
	calls were used. 

	By using this tool, the user understands that a Temporary file may 
        only exist for the duration of a PGE. Whether or not the user deletes
        this Temporary file prior to PGE termination, it will be purged by the 
	SDPS system during normal cleanup operations. If the user requires a more 
	static instance of a file, one that will exist beyond normal PGE termination, 
        that user may elect to create an Intermediate file instead by specifying
	some persistence value (currently, PGSd_IO_Gen_Endurance is the only
	value recognized); note that this value is only valid for the initial
	creation of a file and will not be applied to subsequent accesses of 
	the same file.

	   	FILE		     DURATION FACTORS
           __________________________________________________
          |
	  |     TEMPORARY
          |
        A |  Creation:               PGSd_IO_Gen_NoEndurance
        C |  Repeated Access:        NULL
	T |
        I |     INTERMEDIATE
        O |
        N |  Creation:               PGSd_IO_Gen_Endurance
          |  Repeated Access:        NULL

	
	FILE CHARACTERISTICS
	____________________

	All files created by this function have the following form:

               [label][global-network-IP-address][process-id][date][time]

        where:
 
        label                    : SDP Toolkit Process Control 		    -> pc
        global-network-IP-address: complete IP address iii.iii.iii.iii      -> iiiiiiiiiiii
                                   (0's padded to maintain triplet groupings)
        process-id               : process identifier of current executable -> pppppp
        date                     : days from beginning of year & the year   -> dddyy
        time                     : time from midnight local time  	    -> hhmmss
 
        or 	'pciiiiiiiiiiiippppppdddddtttttt'

	ex.      pc19811819201701028000395104034  

                                      pc     198118192017 010280 00395 104034  
                                      |      |            |      |     |
        (pc) label____________________|      |            |      |     |
        (i)  full-network-IP-address ________|            |      |     |
        (p)  process-id___________________________________|      |     |             
        (d)  date________________________________________________|     |
        (t)  time______________________________________________________|
 

 	All temporary and intermediate files generated by this tool are unique
        within the global ECS community. Also, all file names are NOW exactly
        31 characters in length; this should help with the diagnosis of suspect
        temporary files (i.e., check the length first).


        IMPORTANT TK5 NOTES
        -------------------
        The following environment variable MUST be set to assure proper operation:
 
                PGS_PC_INFO_FILE        path to process control file
 
        However, the following environment variables are NO LONGER recognized by
        the Toolkit as such:
 
                PGS_TEMPORARY_IO        default path to temporary files
                PGS_INTERMEDIATE_INPUT  default path to intermediate input files
                PGS_INTERMEDIATE_OUTPUT default path to intermediate output files
 
        Instead, the default paths, which were defined by these environment variables
        in previous Toolkit releases, may now be specified as part of the Process
        Control File (PCF). Essentially, each has been replaced by a global path
        statement for each of the respective subject fields within the PCF. To
        define a global path statement, simply create a record which begins with the
        '!' symbol defined in the first column, followed by the global path to be
        applied to each of the records within that subject field. Only one such
        statement can be defined per subject field and it must be appear prior to
        any dependent subject entry.
 
        The status condition PGSIO_E_GEN_BAD_ENVIRONMENT now indicates an error
        status on the global path statement as defined in the PCF, and NOT on an
        environment variable. However, as with previous releases, the status message
        associated with this condition may reference the above "tokens", but this is
        only to indicate which of the global path statements is problematic.
 
  
	The environment variable PGS_HOST_PATH, formerly used to direct the Toolkit 
	to the location of the internet protocol address for the local host, has been
	replaced by PDPS functionality which can perform this function in a more effective
	manner. For this reason, the use of this environment variable is no longer 
	supported. FAILURE TO HEED THIS WARNING MAY RESULT IN UNPREDICTABLE RESULTS FOR 
	THE PGE! To properly emulate the manner in which the PDPS system provides this
	information to the Toolkit, continue to use the runtime parameter 
	PGSd_IO_Gen_HostAddress to advertise the IP address of the local host.

 
REQUIREMENTS:	
	PGSTK-0360, 0530, 0531

DETAILS: 	
	This function uses the "C" fopen() call to open a "stream". Another 
	toolkit routine is called to map the file logical to its physical file 
	name. 
	
	Currently, our design assumes that the SDPS Data Server will define
	discrete values for the PGSt_IO_Gen_Duration data type (e.g., #hours > 0; 
	or HOUR,DAY,WEEK,MONTH,...). Only the boolean values of 
	PGSd_IO_Gen_Endurance & PGSd_IO_Gen_NoEndurance are currently supported;
	more values will be supported in the future as details of the SDPS Data Server
	become available.

        The status value PGSIO_E_GEN_NO_TEMP_NAME may be returned even when 
        PGS_IO_Gen_Temp_Name was able to produce a partial temporary file name.
        PGS_IO_Gen_Create blocks creation of a file reference for PC unless a
        complete temporary file name can be constructed; The uniqueness of these
	names must be guaranteed to manage Intermediate files in the DAAC 
	environment. The fact that PGS_IO_Gen_Temp_Name allows for partial name 
	creation in the first place is due to a design consideration which encourages 
	this flexibility in low level units.

GLOBALS:
	None

FILES:
        Process Control File ($PGS_PC_INFO_FILE)
	(optional) Network Host File ($PGS_HOST_PATH)

FUNCTIONS CALLED:
	PGS_IO_Gen_Temp_Reference
	PGS_SMF_SetStaticMsg
	PGS_SMF_SetUNIXMsg
	PGS_IO_Gen_Temp_Delete

END_PROLOG
***************************************************************************/

PGSt_SMF_status 
    PGS_IO_Gen_Temp_Open(
    PGSt_IO_Gen_Duration    file_duration,	/* temporary or intermediate? */
    PGSt_PC_Logical 	    file_logical, 	/* file logical */
    PGSt_IO_Gen_AccessType  file_access, 	/* mode to open file in e.g. READ */
    PGSt_IO_Gen_FileHandle  **file_handle)	/* file handle to be returned */
{
    PGSt_SMF_status 	returnStatus; 		/* PGS status return value */
    FILE 		*temp_ptr; 		/* POSIX stream pointer */
    PGSt_boolean	file_exists;		/* boolean flag to test for existence */
    long		unixerror;		/* retains errno value */
    char 		outstr[PGSd_IO_GEN_REFERENCE_MAX]; 
						/* variable to retrieve file name in */
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
    *file_handle = NULL;			/* clear out */
    strcpy( outstr,"" );			/* empty reference */

    /*------------------------------------------------------
	Get File Reference; create new file if necessary
	For TK2,TK2.5 & TK3, file reference takes the form
	of a Unix path and filename.
    ------------------------------------------------------*/
    returnStatus = PGS_IO_Gen_Temp_Reference( file_duration,file_logical,file_access,
					      outstr,&file_exists );
   
    /*------------------------------------------------------
	Continue processing if status is Success or Warning
    ------------------------------------------------------*/
    if ((returnStatus != PGS_S_SUCCESS) &&
	(returnStatus != PGSIO_W_GEN_NEW_FILE) &&
	(returnStatus != PGSIO_W_GEN_DURATION_NOMOD)
       )
    {
	goto EXCEPTION;
    }

    /*------------------------------------------------------
	Filter : Disallow create/truncate on existing files
	REMEMBER: intermediate files last beyond termination
    ------------------------------------------------------*/
    if ( (file_exists == PGS_TRUE) && 
        ((file_access == PGSd_IO_Gen_Write) 	|| 
         (file_access == PGSd_IO_Gen_Trunc)
        )
       )
    {
    	file_access = PGSd_IO_Gen_AppendUpdate;
	returnStatus = PGSIO_W_GEN_ACCESS_MODIFIED;

        if (caller != PGSd_CALLERID_SMF)
        {
          sprintf(newMsg,
          "Illegal attempt to open existing file with LID %d for PGSd_IO_Gen_Write\nReset access mode to PGSd_IO_Gen_AppendUpdate\n",
           (int)file_logical);
            PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Open");
 
        }
    } 

    /*------------------------------------------------------
	Open physical file under proper access mode
    ------------------------------------------------------*/
    switch (file_access) 
    {
    case PGSd_IO_Gen_Write:	/* File cannot already exist ... see filter above */

   	if ((temp_ptr = fopen(outstr,"w"))== NULL)
   	{
	    unixerror=errno;
	    PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Open");
	    returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
	    *file_handle = temp_ptr;	
   	}
	break;

    case PGSd_IO_Gen_Read:

   	if ((temp_ptr = fopen(outstr,"r"))== NULL)
   	{
	    unixerror=errno;
	    PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Open");
	    returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
	    *file_handle = temp_ptr;
	}
 	break;

    case PGSd_IO_Gen_Append:

 	if ((temp_ptr = fopen(outstr,"a"))== NULL)
   	{
	    unixerror=errno;
	    PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Open");
	    returnStatus = PGS_E_UNIX;
   	}
   	else
   	{
   	    *file_handle = temp_ptr;
   	}
	break;

    case PGSd_IO_Gen_Update:

        if ((temp_ptr = fopen(outstr,"r+"))== NULL)
        {
	    unixerror=errno;
            PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Open");
            returnStatus = PGS_E_UNIX;
        }
        else
        {
            *file_handle = temp_ptr;
        }
        break;

/*  				Disabled for TK3!
    case PGSd_IO_Gen_Trunc:	
	/# file cannot already exist ... see filter above #/

	if ((temp_ptr = fopen(outstr,"w+"))== NULL)
	{
	    unixerror=errno;
	    PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Open");
	    returnStatus = PGS_E_UNIX;
	}
	else
	{
	    *file_handle = temp_ptr;
    	}
	break;
*/

    case PGSd_IO_Gen_AppendUpdate:

        if ((temp_ptr = fopen(outstr,"a+"))== NULL)
        {
	    unixerror=errno;
            PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Open");
            returnStatus = PGS_E_UNIX;
        }
        else
        {
            *file_handle = temp_ptr;
        }
        break;

    default:
	returnStatus = PGSIO_E_GEN_OPENMODE;
    }

EXCEPTION:

    switch (returnStatus)
    {
    case PGS_E_UNIX:
      	/*------------------------------------------------------
            Issue delete instruction to Process Control
	    if NEW Temp file could not be opened
      	------------------------------------------------------*/
      	if (file_exists == PGS_FALSE) 	/* implies that new file was created! */
      	{
	    returnStatus = PGS_IO_Gen_Temp_Delete( file_logical );
    	    /*------------------------------------------------------
                Ignore latest return; Reset Unix error condition.
		If I/O error was set in previous call, it will be
		recorded in a log file.
    	    ------------------------------------------------------*/

	    PGS_SMF_SetUNIXMsg( unixerror,NULL,"PGS_IO_Gen_Temp_Open");
            returnStatus = PGS_E_UNIX;
      	}
	break;
    case PGSIO_E_GEN_OPENMODE:
        sprintf(newMsg,"Invalid mode with LID %d. ",(int)file_logical);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        strcat(newMsg,msg);
        PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Open");
        break;
    case PGS_S_SUCCESS:
	PGS_SMF_SetStaticMsg( returnStatus,"PGS_IO_Gen_Temp_Open" );

      	/*------------------------------------------------------
	    Other status return values will pass through
      	------------------------------------------------------*/
    }
    return( returnStatus );
}
