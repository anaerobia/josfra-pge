/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetPCEnv.c

DESCRIPTION:
	This file contains the function PGS_PC_GetPCEnv().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	31-Mar-95 RM Initial Version
	22-Jun-95 RM Added functionality to handle TILDE due to 
			DR ecs00930.
	28-Jun-95 RM Added functionality to remove a trailing slash
			if it exists.
        07-Jul-95 RM Added checks around strrchr() in case the slash (/)
                        does not exist in the path.  This case should
                        never happen, but if it would it could case a
                        memory problem.  This change was mandated by
                        DR ECSed00782.
        11-Jul-95 RM Changed LONG to INT in cfortran.h macros to correct
                        "Unaligned access error" as per DR ECSed00931.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get default file location for file type from shared memory
	or the PCF.
 
NAME:
	PGS_PC_GetPCEnv()

SYNOPSIS:

C:
	#include <PGS_PC.h>

	PGSt_SMF_status
	PGS_PC_GetPCEnv(
		char			*environment,
		char			*defaultLoc)

FORTRAN:
	include 'PGS_SMF.f'
	include 'PGS_PC.f'
	include 'PGS_PC_9.f'

	integer function pgs_pc_getpcenv(environment,defaultloc)
		character*50		environment
		character*100		defaultloc

DESCRIPTION:
	This tool may be used to obtain a default file location that
	is stored in the PCF or shared memory.
 
INPUTS:
	Name		Description			Units	Min	Max

	environment	This is the actual environment
			variable as defined in the 
			file PGS_PC.h.

OUTPUTS:
	Name		Description			Units	Min	Max

	defaultLoc	The actual default file location
			returned as a string.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_DATA_ACCESS_ERROR   problem while accessing PCS data
        PGSPC_E_ENV_NOT_PC          the environment variable is not defined
                                    by the Process Control Tools
	PGSPC_E_ENVIRONMENT_ERROR   environment variable not set
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file

EXAMPLES:

C:
	char		*environment[50];
	char		defaultLoc[PGSd_PC_PATH_LENGTH_MAX];
	PGSt_SMF_status	returnStatus;

	/# Get the default file location of SUPPORT INPUT files #/

	strcpy(environment,PGSd_PC_SUPPT_INPUT_ENVIRONMENT);

	returnStatus = PGS_PC_GetPCEnv(environment,defaultLoc);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{ /# perform necessary operations using default file location #/ }
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
		character*50	environment
		character*100	defaultloc
		integer		returnstatus
		integer		pgs_pc_getpcenv

C		Get the default file location of SUPPORT INPUT files

		environment = PGSd_PC_SUPPT_INPUT_ENVIRONMENT

		returnstatus = getpcenv(PGSd_PC_SUPPORT_IN_DEFLOC,defaultLoc)

		if (returnstatus .ne. pgs_s_success)
			goto 9999
		else
C			perform necessary operations using default file location
			.
			.
			.
	9999	return

NOTES:
	All default file location strings are guaranteed to be no greater 
	than PGSd_PC_PATH_LENGTH_MAX characters in length (see PGS_PC.h).

REQUIREMENTS:  
	PGSTK-1290

DETAILS:
	This function was written to accept a string as input instead of
	an integer mode value.  This purpose of this was to ease the 
	transition of any other tool using getenv() to retrieve any of
	the environment variables defined by the Process Control Tools.

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable.
	PGS_PC_GetPCSData                Get process control information.
	PGS_SMF_SetStaticMsg		 Set static error/status message.

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_GetPCEnv(                       /* get default file location */
    char             *environment,     /* file type to request location */
    char             *defaultLoc)      /* actual reference ID string */

{
    char              buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char              msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char             *resEnv;           /* resolved environment from getenv() */
    char              tempPath[PGSd_PC_PATH_LENGTH_MAX];/* temporary path variable */
    char             *dum;              /* temp hold from strrchr() */
    int               pos;              /* path name position */
    PGSt_integer      numFiles;         /* version number (not used here) */
    PGSt_integer      mode;             /* mode for PGS_PC_GetPCSData() */
    PGSt_PC_Logical   dummyID;          /* dummy identifier */
    PGSt_SMF_status   whoCalled;        /* user that called this function */
    PGSt_SMF_status   returnStatus;     /* function return value */

/****************************************************************************
*    Initialize variables.
****************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    dummyID = 0;
    numFiles = 0;

/****************************************************************************
*    Initialize caller variable.
****************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/****************************************************************************
*    Determine which environment variable the user is sending in.  
*    From that we can set our mode to the proper default file
*    location flag.
****************************************************************************/
    if (strcmp(environment,PGSd_PC_INPUT_FILE_ENVIRONMENT) == 0)
    {
        mode = PGSd_PC_PRODUCT_IN_DEFLOC;
    }
    else if (strcmp(environment,PGSd_PC_OUTPUT_FILE_ENVIRONMENT) ==0)
    {
        mode = PGSd_PC_PRODUCT_OUT_DEFLOC;
    }
    else if (strcmp(environment,PGSd_PC_SUPPT_INPUT_ENVIRONMENT) == 0)
    {
        mode = PGSd_PC_SUPPORT_IN_DEFLOC;
    }
    else if (strcmp(environment,PGSd_PC_SUPPT_OUT_ENVIRONMENT) == 0)
    {
        mode = PGSd_PC_SUPPORT_OUT_DEFLOC;
    }
    else if (strcmp(environment,PGSd_PC_INTER_INPUT_ENVIRONMENT) == 0)
    {
        mode = PGSd_PC_INTER_IN_DEFLOC;
    }
    else if (strcmp(environment,PGSd_PC_INTER_OUTPUT_ENVIRONMENT) == 0)
    {
        mode = PGSd_PC_INTER_OUT_DEFLOC;
    }
    else if (strcmp(environment,PGSd_PC_TEMP_ENVIRONMENT) == 0)
    {
        mode = PGSd_PC_TEMP_FILE_DEFLOC;
    }

/****************************************************************************
*    The user has passed in a string which is not defined in the file
*    PGS_PC.h as being an environment variable.
****************************************************************************/
    else
    {
        returnStatus = PGSPC_E_ENV_NOT_PC;
        if (whoCalled != PGSd_CALLERID_SMF)
        {
            PGS_SMF_GetMsgByCode(returnStatus,msg);
            sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
            PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_PC_GetPCEnv()");
        }
    }


/****************************************************************************
*    Get the default file location.
****************************************************************************/
    if (returnStatus == PGS_S_SUCCESS)
    {
        defaultLoc[0] = PGSd_PC_CHAR_NULL;
        returnStatus = PGS_PC_GetPCSData(mode,dummyID,defaultLoc,&numFiles);
    }

/***************************************************************************
*    If the first character of the path is a tilde (~) then what we
*    want to do is append the path to what is in the environment variable
*    "PGSHOME."
***************************************************************************/
    if ((defaultLoc[0] == PGSd_PC_TILDE) &&
        (returnStatus == PGS_S_SUCCESS))
    {
        resEnv = getenv(PGSd_PC_PGSHOME_ENVIRONMENT);
        if (resEnv)
        {
            strcpy(tempPath,resEnv);
            if (defaultLoc[1] == PGSd_PC_SLASH)
            {
                pos = 2;
            }
            else
            {
                pos = 1;
            }
            dum = strrchr(tempPath,PGSd_PC_SLASH);
            if ((dum != (char *) NULL) && (strlen(dum) > (size_t) 1))
            {
                strcat(tempPath,PGSd_PC_STRING_SLASH);
            }

/***************************************************************************
*    Make sure the user is not trying to pull a fast one on us.
***************************************************************************/
            if ((strlen(tempPath) + strlen(defaultLoc)) > 
                (size_t) PGSd_PC_PATH_LENGTH_MAX)
            {
                returnStatus =  PGSPC_E_LINE_FORMAT_ERROR;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(returnStatus,msg);
                    sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                    PGS_SMF_SetDynamicMsg(returnStatus,buf,"PGS_PC_GetPCEnv()");
                }
            }
            else
            {
                strcat(tempPath,defaultLoc+pos);
                strcpy(defaultLoc,tempPath);
                dum = strrchr(defaultLoc,PGSd_PC_SLASH);
                if ((dum != (char *) NULL) && (strlen(dum) == (size_t) 1))
                {
                    defaultLoc[(strlen(defaultLoc)-1)] = PGSd_PC_CHAR_NULL;
                }
            }
        }
 
/***************************************************************************
*    The environment "PGSHOME" was not set.
***************************************************************************/
        else
        {
            returnStatus = PGSPC_E_ENVIRONMENT_ERROR;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,PGSd_PC_PGSHOME_ENVIRONMENT);
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                  "PGS_PC_GetPCEnv()");
            }
        }
    }
 
/****************************************************************************
*    If a return status of LEV_E was returned, check to determine what
*    it was and re-set if necessary.
****************************************************************************/
    switch (PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            if ((returnStatus != PGSPC_E_NO_DEFAULT_LOC) &&
                (returnStatus != PGSPC_E_ENV_NOT_PC))
            {
                returnStatus = PGSPC_E_DATA_ACCESS_ERROR;
            }
            break;
    }

/****************************************************************************
*    Update message log and return.
****************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
        PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetReference()");
    }

    return returnStatus;
}
