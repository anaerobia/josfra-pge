/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG

FILENAME:   
	PGS_IO_Gen_Temp_Reference.c

DESCRIPTION:
        This low-level tool supports the C & Fortran versions of the 
	PGS_IO_Gen_Temp_Open tools. Most of the Process Control and Temporary
	file creation code has been abstracted from the original C version of 
	the PGS_IO_Gen_Temp_Open tool in order to support reuse and modularity
	for multi-language implementations.

AUTHOR:
	David P. Heroux / Applied Research Corp.

HISTORY:
	22-Jul-94 DPH 	Initial Version
	04-Aug-94 DPH 	Removed Month from the temporary file name generator
  	12-Aug-94 TWA 	Misc. prolog changes
	29-Aug-94 DPH 	Upgraded to meet TK3 design guidelines:
			 -- removed reference to 'validityFlag' in 
			     PGS_IO_Gen_Temp_Create().
			 -- deleted/disabled use of 'truncate' access mode
	19-Dec-94 DPH	Added checks to environment queries
	03-Jan-95 DPH	Completed hooks for TK4:
			 -- added process-id to file name
			 -- reworked method for getting local time
			 -- added loop to guarantee uniqueness of timestamp
			 -- provided option for getting IP address from PCF
			 -- updated prologs
	10-Feb-95 TWA	Deleting unused variables to eliminate compiler warnings
	15-Feb-95 DPH	Modified temp name creation to guarantee proper 
			insertion of null terminators following the call
			to 'strncpy', which every now and then allowed additional
			characters to creep into your buffers.
        10-May-95 DPH   Upgrade for TK5:
			-- replace calls to getenv() with PGS_PC_GetPCEnv()
			   for all but PGSd_IO_INET_HOSTS_PATH.
			-- insert Intermediate File duration value into 
			   attribute token of PCF subject entry.
        14-Jul-95 DPH   Further upgrade for TK5:
			-- Update Notes to exclude the use of environment variables.
	19-Dec-95 RM	Update for TK6.  Added 'universalRef' variable to 
			file structure used in PC Tools.
        05-Jan-96 RM 	Updated for TK6.  Added Universal References to
                        file entries for files.
        30-Jun-97 DH 	Fixed per ECSed07391. Corrected temporary file name 
                        generation for actual ECS DAAC environment. 
	06-Jul-99 RM 	updated for thread-safe functionality
        28-Jan-02 AT    Modified if statement for defining prototypes for
                        popen and pclose.
        15-Jun-04 PN    Put leading zeros to inet_addr's tokens for Linux and Dec_Alpha      

END_FILE_PROLOG
***************************************************************************/

#include <PGS_IO.h>
#include <PGS_TSF.h>
#ifdef	MIN
#undef MIN		/* cfortran.h doesn't like this on the IBM */
#endif
#include <cfortran.h>

void Make_inet_addr_str(char *subnet, char *token );
static 	PGSt_SMF_status PGS_IO_Gen_Temp_Name( char *temp_name );
static 	PGSt_SMF_status PGS_IO_Gen_Temp_Create( 
	PGSt_PC_Long_Int pcmode,
	char		 *path,
	PGSt_PC_Logical	 temp_id,
	char 		 *temp_name);

/****************************************************************************
BEGIN_PROLOG

TITLE: 
	Get File Refererence for Temporary/Intermediate files.
 
NAME:	
	PGS_IO_Gen_Temp_Reference

SYNOPSIS:
C: 	
	#include <PGS_IO.h>		// includes PGS_SMF.h & PGS_PC_9.g //

        PGSt_SMF_status
	PGS_IO_Gen_Temp_Reference(
		PGSt_IO_Gen_Duration    file_duration,
    		PGSt_PC_Logical         file_logical,
    		PGSt_IO_Gen_AccessType  file_access,
    		char  			*file_reference,
    		PGSt_boolean  	    	*exist_flag)

FORTRAN:
	include 'PGS_SMF.f'
                'PGS_PC_9.f'
                'PGS_IO_1.f'

	integer pgs_io_gen_temp_open(
		integer       file_duration,
		integer	      file_logical,
		integer	      file_access,
		char*35       file_reference,
    		integer       exist_flag)

DESCRIPTION: 	
	Upon a successful call, this function will provide the return argument 
	file_reference to support other file manipulation tools. The 
	user provides the logical file identifier, which internally gets mapped 
	to the associated physical file, if one already exists. If a file mapping
	can not be performed, a new file will be created to map to the logical 
	identifier. 

	In order to obtain a reference for a temporary file, this call must be 
	invoked with the duration argument set to PGSd_IO_Gen_NoDelay. Likewise, 
	to reference an intermediate file, this call must use PGSd_IO_Gen_Delay for the 
	duration argument (Future delay modes may be temporally more flexible).

	This function will support Most POSIX modes of open for C; the only exception
	being standard C's truncate mode (w+).

INPUTS:		
	file_duration 	- PGSd_IO_Gen_Endurance
			  PGSd_IO_Gen_NoEndurance

	file_logical 	- User defined logical file identifier

  	file_access 	- PGSd_IO_Gen_Write 	   (w)
			  PGSd_IO_Gen_Read  	   (r)
			  PGSd_IO_Gen_Append	   (a)
			  PGSd_IO_Gen_Update	   (r+)
			  PGSd_IO_Gen_AppendUpdate (a+)

OUTPUTS:	
	file_reference	- used as input to higher-level Generic I/O Tools

    	exist_flag	- used to determine pre-existence of file

RETURNS: 
	PGS_S_SUCCESS			Success
	PGSIO_W_GEN_NEW_FILE		File expected, but was missing; new file created
	PGSIO_W_GEN_DURATION_NOMOD	Attempt to alter intermediate file duration ignored
	PGSIO_E_GEN_REFERENCE_FAILURE   Other error accessing $PGS_PC_INFO_FILE
	PGSIO_E_GEN_OPENMODE		Invalid access mode
	PGSIO_E_GEN_FILE_NOEXIST	No entry for file logical in $PGS_PC_INFO_FILE
	PGSIO_E_GEN_BAD_FILE_DURATION   Invalid file duration
	PGSIO_E_GEN_NO_TEMP_NAME	Failed to create temporary filename
	PGSIO_E_GEN_CREATE_FAILURE	Error creating new file entry in $PGS_PC_INFO_FILE
        PGSIO_E_GEN_BAD_ENVIRONMENT	Bad default setting detected for I/O path ...

	"Existing file" means that an entry for the file exists in  $PGS_PC_INFO_FILE.

	(NOTE: the above are short  descriptions only; full text of messages appears in 
	 files $PGSMSG/PGS_IO_1.t. Descriptions may change in future releases depending 
	 on external ECS design.)
	PGSTSF_E_GENERAL_FAILURE	problem in TSF code

EXAMPLE:
	PGSt_SMF_status			returnStatus;
	PGSt_PC_Logical			logical;
	char 				reference[36];
    	PGSt_boolean  	    		alreadyExists;

	#define	INTER_1B 101

	ret_val = PGS_IO_Gen_Temp_Reference( PGSd_IO_Gen_Endurance,INTER_1B,
				   	     PGSd_IO_Gen_Write,reference,&alreadyExists );
	if (ret_val != PGS_S_SUCCESS)
	{
	    goto EXCEPTION;
	}
	if (alreadyExists == PGS_TRUE)
	{
	    // open existing file //
	}
	else
	{
	    // create new file with id INTER_1B //
	}
	    	.
	    	.
	    	.

	EXCEPTION:

NOTES:		
	By using this tool, the user understands that a truly Temporary file may
        only exist for the duration of a PGE. Whether or not the user deletes
        this file prior to PGE termination, it will be purged by the PGS system
        during normal cleanup operations. If the user requires a more static
        instance of a file, one that will exist beyond normal PGE termination,
        that user may elect to create an Intermediate file instead by specifying
        some persistence value (currently, PGSd_IO_Gen_Endurance is the only
        value recognized); note that this value is only valid for the initial
        creation of a file and will not be applied to subsequent accesses of
        the same file.
 
        All temporary and intermediate files generated by this tool are unique
        within the global ECS community. Also, all file names are NOW exactly
        31 characters in length; this should help with the diagnosis of suspect
        temporary files (i.e., check the length first).

        Reference names returned by this function have the following form:

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
 



	IMPORTANT TK5 NOTES
	-------------------
	The following environment variable MUST be set to assure proper operation: 

		PGS_PC_INFO_FILE	path to process control file

        However, the following environment variables are NO LONGER recognized by 
	the Toolkit as such:

		PGS_TEMPORARY_IO	path to temporary files 
		PGS_INTERMEDIATE_INPUT 	path to intermediate input files
		PGS_INTERMEDIATE_OUTPUT	path to intermediate output files
        
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

	The following environment variable is also NO LONGER recognized. The tool 
	will look to the Process Control File (PCF) for runtime parameter 
	PGSd_IO_Gen_HostAddress to obtain the IP address.
 
                PGS_HOST_PATH           path to internet hosts file (e.g., /etc/hosts)


REQUIREMENTS:	
	PGSTK-0530, PGSTK-0531

DETAILS:
        None

GLOBALS:
        None
 
FILES:
        Process Control File
 
FUNCTIONS CALLED:
	PGS_IO_Gen_Temp_Create
        PGS_PC_GetPCSData
        PGS_SMF_SetStaticMsg
        PGS_PC_GetPCEnv

END_PROLOG
***************************************************************************/

PGSt_SMF_status 
    PGS_IO_Gen_Temp_Reference(
    PGSt_IO_Gen_Duration    file_duration,	/* temporary or intermediate? */
    PGSt_PC_Logical 	    file_logical, 	/* file logical */
    PGSt_IO_Gen_AccessType  file_access, 	/* mode to access file in i.e. READ */
    char  		   *file_reference,	/* file name to be returned */
    PGSt_boolean  	   *exist_flag)		/* indicates pre-existence of file */
{   
    PGSt_SMF_status 	returnStatus; 		/* PGS status return value */
    PGSt_PC_Long_Int 	getmode; 		/* value sent to PGS_PC_GetPCSData */
    PGSt_PC_Long_Int 	putmode; 		/* value sent to PGS_PC_PutPCSData */
    PGSt_PC_Long_Int 	mode_array[3]; 		/* modes used by PGS_PC_PutPCSData */
    short		file_intermediate;	/* boolean flag to test for intermediate type */
    short		modes;			/* LCV */
    long 		version; 		/* get a specific file */
    char    		new_name[PGSd_PC_FILE_NAME_MAX]; /* new temp file name */
    char	        *path;  		/* pointer to environment specified path */
    PGSt_integer	newFileVersion;		/* used to prevent clobbering of 'version' */
    char                msg[PGS_SMF_MAX_MSG_SIZE];      /* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];/* new status message */
    char		*envVar;		/* runtime specific message */
    char	        newenvpath[PGSd_PC_PATH_LENGTH_MAX]; 	/* buffer for PC specified default path */

    /*------------------------------------------------------
   	Validate File Duration argument
    ------------------------------------------------------*/
    switch (file_duration)
    {
    case PGSd_IO_Gen_NoEndurance:
   	break;
    case PGSd_IO_Gen_Endurance:
   	break;
    default:
	returnStatus = PGSIO_E_GEN_BAD_FILE_DURATION;
	goto EXCEPTION;
    } /* end switch (file_duration) */

    /*------------------------------------------------------
   	Initialize 
    ------------------------------------------------------*/
    returnStatus = PGS_S_SUCCESS; 		/* success */
    version = 1;				/* always one-to-one correspondence; 
						   logical->physical */
    mode_array[0] = PGSd_PC_INTERMEDIATE_INPUT;
    mode_array[1] = PGSd_PC_INTERMEDIATE_OUTPUT;
    mode_array[2] = PGSd_PC_TEMPORARY_FILE;
    strcpy( new_name,"" );			/* empty name */
    *exist_flag = PGS_FALSE;
    strcpy( file_reference,"" );		/* return empty string */
    file_intermediate = PGS_FALSE;
    path = NULL;
    envVar = NULL;

    /*------------------------------------------------------
        Check for existence of temporary/intermediate file.
	Convert logical identifier into a physical one;
        examine specific fields of the PC table to find
        if reference exists for the logical ID specified.
    ------------------------------------------------------*/
    for (modes=0;modes<3;modes++)
    {
	newFileVersion = version;
    	getmode = mode_array[modes];
    	returnStatus = PGS_PC_GetPCSData( getmode,file_logical,file_reference,&newFileVersion );
        if (returnStatus != PGS_S_SUCCESS)
        {
            if (returnStatus == PGSPC_W_NO_FILES_EXIST)
            {
                continue;
	    }
	    else
	    {
		returnStatus = PGSIO_E_GEN_REFERENCE_FAILURE;
		goto EXCEPTION; 
	    }
        }
        else
	{
	    *exist_flag = PGS_TRUE;
	    if ((mode_array[modes] == PGSd_PC_INTERMEDIATE_INPUT) ||
	        (mode_array[modes] == PGSd_PC_INTERMEDIATE_OUTPUT))
	    {
		file_intermediate = PGS_TRUE;
	    }
	    break;
	}
    } /* end for (modes) */

    /*------------------------------------------------------
        If existing file is of type Intermediate, user
        cannot alter the duration factor.
    ------------------------------------------------------*/
    if ((*exist_flag == PGS_TRUE) &&
        (file_intermediate == PGS_TRUE) &&
	(file_duration == PGSd_IO_Gen_NoEndurance)
       )
    {
        returnStatus = PGSIO_W_GEN_DURATION_NOMOD;
        sprintf( newMsg, "Attempt to alter intermediate file duration with LID %d. ",(int)file_logical);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        strcat(newMsg,msg);
        PGS_SMF_SetDynamicMsg( returnStatus,newMsg,"PGS_IO_Gen_Temp_Reference" );
    }
 

    /*------------------------------------------------------
	Analyze access mode to determine proper action 
    ------------------------------------------------------*/
    switch (file_access) 
    {
    case PGSd_IO_Gen_Read:
    case PGSd_IO_Gen_Update:

	if (*exist_flag == PGS_FALSE)
	{
	    returnStatus = PGSIO_E_GEN_FILE_NOEXIST;
	}
 	break;

    case PGSd_IO_Gen_Write:
    case PGSd_IO_Gen_Append:
    case PGSd_IO_Gen_AppendUpdate:

	if (*exist_flag == PGS_FALSE)
	{
            if (file_duration == PGSd_IO_Gen_NoEndurance)
            {
                putmode = PGSd_PC_TEMPORARY_FILE;
     	        envVar = (char *) PGSd_PC_TEMP_ENVIRONMENT;
	        returnStatus = PGS_PC_GetPCEnv( (char *) PGSd_PC_TEMP_ENVIRONMENT,newenvpath );
            }
            else
            {
                putmode = PGSd_PC_INTERMEDIATE_OUTPUT;
     	        envVar = (char *) PGSd_PC_INTER_OUTPUT_ENVIRONMENT;
	        returnStatus = PGS_PC_GetPCEnv( (char *) PGSd_PC_INTER_OUTPUT_ENVIRONMENT,newenvpath );
            }

	    switch (returnStatus)
	    {
   	    case PGSPC_E_ENV_NOT_PC:
	    case PGSPC_E_DATA_ACCESS_ERROR:
                returnStatus = PGSIO_E_GEN_REFERENCE_FAILURE;
		goto EXCEPTION;
	    case PGSPC_E_NO_DEFAULT_LOC:
	        path = NULL;
                returnStatus = PGSIO_E_GEN_BAD_ENVIRONMENT;
		goto EXCEPTION;
	    case PGS_S_SUCCESS:	
		path = newenvpath;
	    }

            /*------------------------------------------------------
                Create a new Temporary/Intermediate file
            ------------------------------------------------------*/
            returnStatus = PGS_IO_Gen_Temp_Create( putmode,path,file_logical,new_name );
            if (returnStatus != PGS_S_SUCCESS)
            {
                goto EXCEPTION;		/* return values will fall through */
            }
	    sprintf( file_reference,"%s/%s",path,new_name );
	   
            /*------------------------------------------------------
                User specified inapprorpriate access mode for a
		non-existent file. Warn of new file creation
            ------------------------------------------------------*/
	    if (file_access != PGSd_IO_Gen_Write)
	    {
	    	returnStatus = PGSIO_W_GEN_NEW_FILE;
		sprintf(newMsg,"File with LID %d does not exist. ",(int)file_logical);
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                strcat(newMsg,msg);
                PGS_SMF_SetDynamicMsg( returnStatus,newMsg,"PGS_IO_Gen_Temp_Reference" );
	    }
	}

	break;

    default:
	returnStatus = PGSIO_E_GEN_OPENMODE;
    }

EXCEPTION:

    if (returnStatus == PGSPC_W_NO_FILES_EXIST)
    {
	returnStatus = PGSIO_E_GEN_FILE_NOEXIST;
    }

    switch (returnStatus)	/* WARNINGS are statically set in-situ */
    {
    case PGS_S_SUCCESS:		/* SUCCESS message placed in buffer */
	PGS_SMF_SetStaticMsg( returnStatus,"PGS_IO_Gen_Temp_Reference" );
        break;
    case PGSIO_E_GEN_OPENMODE:
        sprintf( newMsg, "Invalid mode with LID %d. ",(int)file_logical);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        strcat(newMsg,msg);
        PGS_SMF_SetDynamicMsg( returnStatus,newMsg,"PGS_IO_Gen_Temp_Reference" );
        break;
    case PGSIO_E_GEN_BAD_FILE_DURATION:
        sprintf(newMsg,"Invalid file duration with LID %d. ",(int)file_logical);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        strcat(newMsg,msg);
        PGS_SMF_SetDynamicMsg( returnStatus,newMsg,"PGS_IO_Gen_Temp_Reference" );
        break;
    case PGSIO_E_GEN_BAD_ENVIRONMENT:
        PGS_SMF_GetMsgByCode( returnStatus,msg );
        sprintf( newMsg,"%s %s",msg,envVar );
        PGS_SMF_SetDynamicMsg( returnStatus,newMsg,"PGS_IO_Gen_Temp_Reference" );
        break;
    case PGSIO_E_GEN_FILE_NOEXIST:
        sprintf(newMsg,"No entry for file with LID %d. ",(int)file_logical);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        strcat(newMsg,msg);
        PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Reference");
        break;
    case PGSIO_E_GEN_REFERENCE_FAILURE:
        sprintf(newMsg,"Can not find Physical File Name with LID %d. ",(int)file_logical);
        PGS_SMF_GetMsgByCode(returnStatus,msg);
        strcat(newMsg,msg);
        PGS_SMF_SetDynamicMsg(returnStatus,newMsg, "PGS_IO_Gen_Temp_Reference");
        break;
    }
    return( returnStatus );
}

/* set up MACRO to be expanded by cfortran.h depending on system type. */

FCALLSCFUN5(INT,PGS_IO_Gen_Temp_Reference,PGS_IO_GEN_TEMP_REFERENCE,pgs_io_gen_temp_reference,INT,INT,INT,PSTRING,PINT)


/****************************************************************************
BEGIN_PROLOG

TITLE:
        Generate a unique file name for temporary & intermediate files

NAME:
        PGS_IO_Gen_Temp_Name

SYNOPSIS:
        #include <PGS_IO.h>

	PGSt_SMF_status 
	PGS_IO_Gen_Temp_Name( char *temp_name )

DESCRIPTION:
	This function utilizes the time and internet address of the host
	machine, along with the PGS assigned Production Run Id and process id, 
	in order to generate a DAAC unique temporary file name (this assumes 
	that all processing platforms are colocated with the DAAC).

INPUTS:
	None

OUTPUTS:
        temp_name     - unique physical file name

RETURNS:
        PGS_S_SUCCESS
	PGS_E_UNIX
	PGSIO_E_PARTIAL_NAME
        PGSIO_E_GEN_BAD_ENVIRONMENT

EXAMPLE:
	PGSt_SMF_status ret_val;
	char	new_name[ PGSd_PC_FILE_NAME_MAX ];

        ret_val = PGS_IO_Gen_Temp_name( new_name );
        if (ret_val != PGS_S_SUCCESS)
        {
            goto EXCEPTION;
        }
            .
            .
            .
        EXCEPTION:

NOTES:
        All file names generated by this tool are unique within the global ECS 
        community.  Also, all file names are NOW exactly 31 characters in length; 
        this should help with the diagnosis of suspect temporary files (i.e., 
        check the length first).
 
        File names returned by this function NOW have the following form:
 
        [label][global-network-IP-address][process-id][date][time]
 
        where:
 
        label                    : SDP Toolkit Process Control              -> pc
        global-network-IP-address: complete IP address iii.iii.iii.iii      -> iiiiiiiiiiii
                                   (0's padded to maintain triplet groupings)
        process-id               : process identifier of current executable -> pppppp
        date                     : days from beginning of year & the year   -> dddyy
        time                     : time from midnight local time            -> hhmmss
 

        or      'pciiiiiiiiiiiippppppdddddtttttt'
 
        ex.      pc19811819201701028000395104034
 
                                      pc     198118192017 010280 00395 104034
                                      |      |            |      |     |
        (pc) label____________________|      |            |      |     |
        (i)  full-network-IP-address ________|            |      |     |
        (p)  process-id___________________________________|      |     |
        (d)  date________________________________________________|     |
        (t)  time______________________________________________________|
 
 
 

        IMPORTANT TK5 NOTES
        -------------------
        The following environment variable MUST be set to assure proper operation:
 
                PGS_PC_INFO_FILE        path to process control file
 
        However, the following environment variables are NO LONGER recognized by
        the Toolkit as such:
 
                PGS_TEMPORARY_IO        path to temporary files
                PGS_INTERMEDIATE_INPUT  path to intermediate input files
                PGS_INTERMEDIATE_OUTPUT path to intermediate output files
 
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

	The following environment variable may be optionally set. The tool will
	reference this path to obtain the IP address if the variable is set, otherwise,
	it will look to the Process Control File (PCF) for runtime parameter
	PGSd_IO_Gen_HostAddress to obtain the address. 

		PGS_HOST_PATH           path to internet hosts file (e.g., /etc/hosts)


REQUIREMENTS:
        PGSTK-0360

DETAILS:
	The algorithm for obtaining the timestamp for a file no longer performs a delay
        since this could not be guaranteed by the system and it would not be very efficient
	to force programs to wait unless they absolutely have to. Therefore, the algorithm 
	now compares new timestamps with the last one generated and continues to generate
	new timestamps until a difference is detected. For large systems with a lot of 
	processing going on, this looping aspect of the algorithm will most likely never
	be executed. However, for fast processes generating many files consecutively, on
	systems with little or no load, this looping branch will ensure the uniqueness of 
	files being generated.

GLOBALS:
	PGSg_TSF_IOGenLastTime

FILES:
        Process Control File
        Network Host File (optional)

FUNCTIONS CALLED:
        PGS_PC_GetPCSData
        PGS_SMF_SetStaticMsg
        PGS_SMF_SetUNIXMsg
	PGS_TSF_GetMasterIndex()

END_PROLOG
***************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#define	TIME_BUFFER_SIZE	14
#define PROCESS_BUFFER_SIZE	 7
#define NETWORK_BUFFER_SIZE	16

/* Removed for Pre-Rel.B Testbed TK Patch 
#define PRODUCTION_BUFFER_SIZE	10
*/ 

static PGSt_SMF_status PGS_IO_Gen_Temp_Name( 
    char *temp_name )
{
    PGSt_SMF_status 	returnStatus;	/* PGS status return value */
    char		*hosts;		/* location of hosts file  */

/* Removed for Pre-Rel.B Testbed TK Patch 
    char		run_id[PRODUCTION_BUFFER_SIZE];	
					// results of call to _GetPCSData //
*/

    char		inet_addr[NETWORK_BUFFER_SIZE];	
					/* temp buffers for host internet address */
    char		time_stamp[TIME_BUFFER_SIZE+1]; 
					/* temp buffer for system time */
    char		process_id[PROCESS_BUFFER_SIZE];	
					/* temp buffer for process id */
    char		input[PGSd_IO_GEN_BUF_MAX+1]; 		
					/* stream input buffer */
    char		command[240];	/* popen command */
    pid_t		process;	/* actual erocess id */
    FILE    		*stream;	/* pipe stream pointer */
    /*PGSt_integer	dummy;*/	/* version # NA */
    char                msg[PGS_SMF_MAX_MSG_SIZE];      	
					/* predefined status message */
    char                newMsg[PGS_SMF_MAX_MSGBUF_SIZE];	
					/* new status message */
    char                *envVar;        /* runtime specific message */
    char            	auxHost[PGSd_PC_VALUE_LENGTH_MAX]; 	
					/* auxiliary IP address */
    time_t		internalClock;	/* system dependent time format */
    struct tm		*standardClock;	/* system independent time format */
    size_t		clockSize;	/* characters placed into time_stamp */	
#ifdef _PGS_THREADSAFE
    int			lastTime;	/* last recorded time aquired (non-static) */
#else
    static int		lastTime=0;	/* last recorded time aquired */
#endif
    int			nextTime;	/* current time aquired */

    /* NOTE: popen() and pclose() are neither ANSI nor POSIX conformant, however
       they are conformant with X/Open Portability Guide, Issue 3, 1989.
       Therefore if _XOPEN_SOURCE (_INCLUDE_XOPEN_SOURCE on the HP) are defined,
       the function prototypes will be defined in <stdio.h>.  Otherwise define
       the prototypes here. */

#if (!defined(_XOPEN_SOURCE) && !defined(_INCLUDE_XOPEN_SOURCE)) && \
(!defined(SUN) && !defined(__cplusplus))
    extern FILE* popen(const char*, const char*);
    extern int   pclose(FILE*);
#endif

    char		netbuf[NETWORK_BUFFER_SIZE];	
				  /* temp buffer for host internet address */
    int			netlen;
    char                *token;   /* string token */
    char                subnet[4]; /* IP component */

#ifdef _PGS_THREADSAFE
    char *lasts;   /* used for strtok_r() */
    struct tm localClock;	/* system independent time format */
    extern int PGSg_TSF_IOGenLastTime[];
    int masterTSFIndex;

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
        goto EXCEPTION;
    }

    /* set from global counterpart */
    lastTime = PGSg_TSF_IOGenLastTime[masterTSFIndex];
#endif

    /*------------------------------------------------------
      Initialize routine
    ------------------------------------------------------*/
    returnStatus = PGS_S_SUCCESS;
    strcpy( temp_name,"pc" );
    envVar = NULL;
    nextTime=0;
    strcpy( inet_addr,"" );

    /*------------------------------------------------------
      Get the Production Run #
    ------------------------------------------------------*/
/* Removed for Pre-Rel.B Testbed TK Patch 
    returnStatus = PGS_PC_GetPCSData( PGSd_PC_PRODUCTION_RUN_ID,NULL,run_id,&dummy );
    if ((returnStatus != PGS_S_SUCCESS) ||
	(strcmp( run_id,"0" ) == 0)
       )
    {
    	strcpy( run_id,"_" );		// default in case no value is present //
	returnStatus = PGSIO_E_PARTIAL_NAME;
    }	
    strcat( temp_name,run_id );
*/
       
    /*------------------------------------------------------
	Get the Host internet (global) address 
	'aaa.bbb.ccc.ddd -> aaabbbcccddd' from primary source
    ------------------------------------------------------*/
    hosts = (char*) getenv( PGSd_IO_INET_HOSTS_PATH );
    if (hosts == NULL)
    {
        /*------------------------------------------------------
	    Check backup source before issuing error
        ------------------------------------------------------*/
        returnStatus = PGS_PC_GetConfigData( PGSd_IO_Gen_HostAddress,auxHost );
        if (returnStatus == PGS_S_SUCCESS)
	{
          /*
           * copy input line to temporary buffer
           * because strtok does destructive editing
           */
          strncpy(netbuf, auxHost, NETWORK_BUFFER_SIZE);
        }
        else
        {
            envVar = (char *) PGSd_IO_INET_HOSTS_PATH;
            returnStatus = PGSIO_E_GEN_BAD_ENVIRONMENT;
	    goto EXCEPTION;
        }
    }
    else
    {
        sprintf( command,"grep `uname -n` %s |cut -f 1-4 -d . | cut -f 1",hosts );
#ifdef _PGS_THREADSAFE
        /* locking an area here due to the popen() */
        returnStatus = PGS_TSF_LockIt(PGSd_TSF_IOLOCK);
        if (PGS_SMF_TestErrorLevel(returnStatus))
        {
            returnStatus = PGSTSF_E_GENERAL_FAILURE;
	    goto EXCEPTION;
        }
        sleep(1);  /* to ensure no two files will have same name */
#endif
        stream = (FILE*) popen( command,"r" );
        if (stream == NULL)
        {
    	  PGS_SMF_SetUNIXMsg( errno,NULL,"PGS_IO_Gen_Temp_Name");
          returnStatus = PGS_E_UNIX;
#ifdef _PGS_THREADSAFE
        /* unlock  due to leaving the function */
        PGS_TSF_UnlockIt(PGSd_TSF_IOLOCK);
#endif
	  goto EXCEPTION;
        }
        while( fgets( input,PGSd_IO_GEN_BUF_MAX,stream ) != NULL )
        {
          /*
           * copy input line to temporary buffer
           * because strtok does destructive editing
           */

          /* WARNING: This alg. will retrieve the IP address of the LAST
             record by the same name.
           */
          strncpy(netbuf, input, NETWORK_BUFFER_SIZE);
        }
        pclose( stream );
#ifdef _PGS_THREADSAFE
        /* unlock due to closing the stream */
        returnStatus = PGS_TSF_UnlockIt(PGSd_TSF_IOLOCK);
#endif
    }
     

    if (netbuf != (char *)NULL)
    {
        netlen = strlen(netbuf);
        if (netbuf[netlen] == '\n')
        {
            netbuf[netlen] = '\0';
        }
    }

#ifdef _PGS_THREADSAFE
    /* strtok() is not threadsafe - so for the threadsafe version 
       we replace it with its threadsafe counterpart strtok_r() */
    token = strtok_r(netbuf,".",&lasts);
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

    token = strtok_r((char *)NULL,".",&lasts);
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

    token = strtok_r((char *)NULL,".",&lasts);
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

    token = strtok_r((char *)NULL,"\n",&lasts);
   (void) Make_inet_addr_str(subnet, token); 
   /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

#else  /* -D_PGS_THREADSAFE */

    token = strtok(netbuf,".");
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

    token = strtok((char *)NULL,".");
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

    token = strtok((char *)NULL,".");
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);

    token = strtok((char *)NULL,"\n");
    (void) Make_inet_addr_str(subnet, token);
    /*sprintf(subnet,"%03s",token);*/
    strcat(inet_addr,subnet);
#endif  /* -D_PGS_THREADSAFE */


    strcat( temp_name,inet_addr );

    /*------------------------------------------------------
        Get the Process Id -> 'nnnnnn'
    ------------------------------------------------------*/
    process = getpid( );
    sprintf( process_id,"%06d",(int) process );
    strcat( temp_name,process_id );
 
    /*------------------------------------------------------
	Get the System Time -> 'dddyyhhmmss'
        Compute synchronization check : hhmmss
    ------------------------------------------------------*/
    time( &internalClock );
#ifdef _PGS_THREADSAFE
    /* localtime() is not threadsafe, so for the threadsafe version
       we replace it with its threadsafe counterpart locattime_r() */
    standardClock = localtime_r( &internalClock, &localClock );
    standardClock = &localClock;
#else
    standardClock = localtime( &internalClock );
#endif
    nextTime = (standardClock->tm_hour * 10000) + 
               (standardClock->tm_min * 100) + 
               (standardClock->tm_sec);

    while (nextTime == lastTime)
    {
        time( &internalClock );
#ifdef _PGS_THREADSAFE
        /* localtime() is not threadsafe, so for the threadsafe version
           we replace it with its threadsafe counterpart locattime_r() */
        standardClock = localtime_r( &internalClock, &localClock );
        standardClock = &localClock;
#else
        standardClock = localtime( &internalClock );
#endif
        nextTime = (standardClock->tm_hour * 10000) + 
                   (standardClock->tm_min * 100) + 
                   (standardClock->tm_sec);
    }
    lastTime = nextTime;
#ifdef _PGS_THREADSAFE
    /* re-set the global */
    PGSg_TSF_IOGenLastTime[masterTSFIndex] = lastTime;
#endif
    
    /*------------------------------------------------------
        Format local time for file time stamp
    ------------------------------------------------------*/
    clockSize = strftime( time_stamp,TIME_BUFFER_SIZE,"%j%y%H%M%S",standardClock );   
    if (clockSize <= 0)
    {
        /*------------------------------------------------------
	     buffer not big enough for results
    	------------------------------------------------------*/
    	returnStatus = PGSIO_E_PARTIAL_NAME;
        goto EXCEPTION;
    }

    strcat( temp_name,time_stamp );

EXCEPTION:

    switch (returnStatus)
    {
    case PGSIO_E_PARTIAL_NAME:
    	PGS_SMF_SetStaticMsg( returnStatus,"PGS_IO_Gen_Temp_Name" );
        break;
    case PGSIO_E_GEN_BAD_ENVIRONMENT:
        PGS_SMF_GetMsgByCode( returnStatus,msg );
        sprintf( newMsg,"%s %s",msg,envVar );
        PGS_SMF_SetDynamicMsg( returnStatus,newMsg,"PGS_IO_Gen_Temp_Name" );
    }
    return( returnStatus );

} /* end PGS_IO_Gen_Temp_Name( ) */
	
/****************************************************************************
BEGIN_PROLOG

TITLE:
        Create a temporary/intermediate file

NAME:
        PGS_IO_Gen_Temp_Create

SYNOPSIS:
        #include <PGS_IO.h>

DESCRIPTION:
	This tool generates a new temporary file name and updates the 
	Process Control Mapping table with the new information.

INPUTS:
        pcmode 		- process control access mode

        path		- directory path for temporary file

        temp_id		- logical file identifier referencing the temporary file

OUTPUTS:
        temp_name     	- unique physical file name for temporary file

RETURNS:
        PGS_S_SUCCESS
	PGSIO_E_GEN_NO_TEMP_NAME
	PGSIO_E_GEN_CREATE_FAILURE

EXAMPLE:
	#define INTER_1B 101
        PGSt_SMF_status  ret_val;
        char             tmp_path[PGSd_PC_PATH_LENGTH_MAX],
        char    	 temp_name[PGSd_PC_FILE_NAME_MAX];

        ret_val = PGS_PC_GetPCEnv( (char *) PGSd_PC_TEMP_ENVIRONMENT,tmp_path );
	if (ret_val != PGS_S_SUCCESS) 
	{
	    goto EXCEPTION;
	}

        ret_val = PGS_IO_Gen_Temp_Create( PGSd_PC_TEMPORARY_FILE,
				 	  tmp_path,INTER_1B,temp_name )
        if (ret_val != PGS_S_SUCCESS)
        {
            goto EXCEPTION;
        }
            .
            .
            .
        EXCEPTION:

NOTES:
	(see PGS_IO_Gen_Temp_Name() )

REQUIREMENTS:
        PGSTK-0360

DETAILS:
	None
GLOBALS:
        None

FILES:
        Process Control File

FUNCTIONS CALLED:
        PGS_IO_Gen_Temp_Name
	PGS_PC_PutPCSData
        PGS_SMF_SetStaticMsg

END_PROLOG
***************************************************************************/
static PGSt_SMF_status PGS_IO_Gen_Temp_Create( 
    PGSt_PC_Long_Int pcmode,
    char             *path,
    PGSt_PC_Logical  temp_id,
    char             *temp_name )
{
    PGSt_SMF_status 	returnStatus;	/* PGS status return value */
    PGSt_PC_File_Struct fileinfo;       /* data for Process Control file mapping */

    returnStatus = PGS_S_SUCCESS;

    /*------------------------------------------------------
	Create new temporary file
    ------------------------------------------------------*/
    returnStatus = PGS_IO_Gen_Temp_Name( temp_name );
    if (returnStatus != PGS_S_SUCCESS)
    {
	returnStatus = PGSIO_E_GEN_NO_TEMP_NAME;
	goto EXCEPTION;
    }

    /*------------------------------------------------------
	Update Process Control file mapping for TEMP OUTPUT
    ------------------------------------------------------*/
    fileinfo.index = temp_id;
    fileinfo.size = 0;              /* FUTURE - check stat( ) for size */
    /* fileinfo.bufferSize = 0;   ** changed to universalRef for TK6 */

    /* fileinfo.validityFlag = 0;    ** Removed for TK3 */

    fileinfo.entries = 0;
    strcpy( fileinfo.universalRef,"");
    strcpy( fileinfo.fileName,temp_name );
    sprintf( fileinfo.attributeLoc,"%ld",(long) pcmode );
    strcpy( fileinfo.path,path );
    returnStatus = PGS_PC_PutPCSData( pcmode,(void *) &fileinfo );
    if (returnStatus != PGS_S_SUCCESS)
    {
	returnStatus = PGSIO_E_GEN_CREATE_FAILURE;
	goto EXCEPTION;
    }

EXCEPTION:

    switch (returnStatus)
    {
    case PGSIO_E_GEN_NO_TEMP_NAME:
    case PGSIO_E_GEN_CREATE_FAILURE:
    	PGS_SMF_SetStaticMsg( returnStatus,"PGS_IO_Gen_Temp_Create" );
    }
    return( returnStatus );

} /* end PGS_IO_Gen_Temp_Create( ) */

void Make_inet_addr_str(char *subnet, char *token)
/* Function to put leading zeros to inet_addr's token for Linux and Dec_Alpha */
{
#if defined(LINUX) || defined(DEC_ALPHA) || defined(LINUX32) || defined(LINUX64) || defined(IA64) || defined(MACINTEL)

int len=strlen(token);
 
  if ((len > 0) && (len < 4))
    {
      if(len == 3)
        {
          sprintf(subnet,"%s",token);
        }
      else if (len == 2)
        {
            sprintf(subnet,"0%s",token);
        }
      else if (len == 1)
        {
            sprintf(subnet,"00%s",token);
        }
     }
  else
     {
       sprintf(subnet,"%03s",token);
     }
#else
  sprintf(subnet,"%03s",token);
 
#endif
}
