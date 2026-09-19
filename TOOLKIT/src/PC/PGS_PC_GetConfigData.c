/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_GetConfigData.c

DESCRIPTION:
	This file contains the function PGS_PC_GetConfigData().

AUTHOR:
	Ray Milburn / Applied Research Corp.
	Abe Taaheri / SM&A Corp.

HISTORY:
	05-Aug-94 RM Initial Version
	14-Oct-94 RM Added call to PGS_SMF_CallerID
        11-Jul-95 RM Changed LONG to INT in cfortran.h macros to correct
                        "Unaligned access error" as per DR ECSed00931.
	10-May-99 AT Added a few lines so that product ID number is printed
                     in the LogStatus file when error is 
		     PGSPC_W_NO_CONFIG_FOR_ID as per NCR ECSed17940.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <PGS_PC.h>
#include <string.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Get user defined configuration values.
 
NAME:
	PGS_PC_GetConfigData()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_GetConfigData(
			PGSt_PC_Logical		configParamID,
			char			*configParamVal)

FORTRAN:
		include 'PGS_SMF.f'
		include 'PGS_PC.f'
		include 'PGS_PC_9.f'

		integer function pgs_pc_getconfigdata(configparamid,
	       *    				configparamval)
			integer			configparamid
			character*200		configparamval

DESCRIPTION:
	This tool may be used to import run-time configuration parameters 
	into the PGE.
 
INPUTS:
	Name		Description			Units	Min	Max

	configParamID	User defined constant that 
			internally represents a 
			configuration parameter.

OUTPUTS:
	Name		Description			Units	Min	Max

	configParamVal	A string representation of the 
			configuration parameter value.  
			No interpretation of this value 
			will be done in the toolkit; 
			the value returned will be left 
			to the application programmer.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_CONFIG_FOR_ID    no configuration data for product id
	PGSPC_E_DATA_ACCESS_ERROR   error accessing PCS data

EXAMPLES:

C:	
	#define MODIS1A_CONFIG1 2990

	char		configParamVal[PGSd_PC_VALUE_LENGTH_MAX];
	PGSt_SMF_status	returnStatus;
	long		config1;

	returnStatus = PGS_PC_GetConfigData(MODIS1A_CONFIG1,configParamVal);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{
	 /# MODIS1A_CONFIG1 is integral parameter #/
		config1 = atoi(configParamVal);

		if (config1 > 0)
		{
			/# activate sub-process A #/
		}
		else
		{
			/# activate sub-process B #/
		} 
	}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
		IMPLICIT NONE

		character*200	configparamval
		integer		returnstatus
		integer		pgs_pc_getconfigdata
		integer		config1
		integer		modis1a_config1
		parameter	(modis1a_config1 = 2990)

		returnstatus = pgs_pc_getconfigdata(modis1a_config1,configparamval)

		if (returnstatus .ne. success) then
			goto 9999
		else
C
C			modis1a_config1 is integral parameter
C			assuming you have a function to convert character
C			data to integer data - called.....strtoint.
C

			strtoint(configparamval,config1)

			if (config1 .gt. 0) then
C				activate sub-process A
			else
C				activate sub-process B
			.
			.
			.
			endif

		endif

		return

NOTES:
	All configuration parameter value strings are guaranteed to be less 
	than PGSd_PC_VALUE_LENGTH_MAX characters in length (see PGS_PC.h). 
	There will be a shell script command version of this routine to retrieve 
	configuration information from the script.

REQUIREMENTS:  
	PGSTK-1290

DETAILS:
	NONE

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
PGS_PC_GetConfigData(                   /* get user defined config data */
    PGSt_PC_Logical   configParamID,    /* logical value */
    char             *configParamVal)   /* actual configuration value */

{
    PGSt_integer      numFiles;         /* number of files in product group */
    PGSt_SMF_status   whoCalled;        /* user that called this function */
    PGSt_SMF_status   returnStatus;     /* function return value */
    char errorBuf[PGS_SMF_MAX_MSG_SIZE]; /*Strings appended to error returns*/
    char dynamicMsg[PGS_SMF_MAX_MSG_SIZE];
    char errorMsg[PGS_SMF_MAX_MSGBUF_SIZE];

/****************************************************************************
*    Initialize status.
****************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/****************************************************************************
*    Initialize caller variable.
****************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/****************************************************************************
*    Now, just call PGS_PC_GetPCSData() and let all the error checking and
*    handling take place there.  Remember, this function exists for the 
*    purpose of information hiding. 
****************************************************************************/
    returnStatus = PGS_PC_GetPCSData(PGSd_PC_CONFIGURATION,configParamID,
                                configParamVal,&numFiles);

/****************************************************************************
*    Here is where we see if there was a problem.
****************************************************************************/
    if (returnStatus != PGS_S_SUCCESS)
    {

/****************************************************************************
*    No config value found, this may or may not be a problem so we 
*    just send a warning back to the user.
****************************************************************************/
         if (returnStatus == PGSPC_W_NO_CONFIG_VALUE)
         {
             returnStatus = PGSPC_W_NO_CONFIG_FOR_ID;
         }

/****************************************************************************
*    We do have an error status.  Let the user know.
****************************************************************************/
        else
        {
            returnStatus = PGSPC_E_DATA_ACCESS_ERROR;
        }
    }

/****************************************************************************
*    Update the message log and return.
****************************************************************************/
    if (whoCalled != PGSd_CALLERID_SMF)
    {
	if(returnStatus == PGSPC_W_NO_CONFIG_FOR_ID)
	{
	  if(configParamID != 10256) /* PGSd_MET_LOGICAL_XML is 
					defined for 10256 in PGS_MET.h */
	    {
	      sprintf(dynamicMsg, "Problematic ID is %d.", 
		      configParamID);
	      PGS_SMF_GetMsgByCode(PGSPC_W_NO_CONFIG_FOR_ID,errorBuf);
	      strcat(errorBuf," %s");
	      sprintf(errorMsg, errorBuf, dynamicMsg);
	      PGS_SMF_SetDynamicMsg(PGSPC_W_NO_CONFIG_FOR_ID,errorMsg,
				    "PGS_PC_GetConfigData()");
	    }
	}
	else
	{
	    PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetConfigData()");
	}
    }
    
    return returnStatus;
}
