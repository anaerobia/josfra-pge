/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_RetrieveData.c

DESCRIPTION:
	Retrieve data requested from line of PCS data.

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	02-Aug-94 RM Initial version
	14-Oct-94 RM Added call to PGS_SMF_CallerID()
	05-Apr-95 RM Updated for TK5.  Added functionality to 
			retrieve default file location from PCF
			instead of environment variables.
	13-Dec-95 RM Updated for TK6.  Added functionality to allow
			multiple instances of PRODUCT OUTPUT FILES.
        05-Jan-96 RM Updated for TK6.  Added Universal References to
                        file entries for files.
	24-Apr-97 RM Added functionality to allow multiple instances
			of SUPPORT INPUT and SUPPORT OUTPUT FILES.
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Retrieve the requested data from the line of Process Control 
	Status information.
 
NAME:
	PGS_PC_GetPCSDataRetrieveData()

SYNOPSIS:

C:
	#include <PGS_PC.h>
	
	PGSt_SMF_status
	PGS_PC_GetPCSDataRetrieveData(
		FILE			*locationPCS,
		PGSt_integer		mode,
		char			*line,
		char			*outstr,
		PGSt_int		*numFiles);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to retrieve the actual data requested from
	the line of PCS data.
 
INPUTS:
	Name		Description			Units	Min	Max

	locationPCS	pointer to location of PCS data

	mode		context values that will be 
			#define'd in a header file.
			Possible values are:

			PGSd_PC_PRODUCTION_RUN_ID
			PGSd_PC_SOFTWARE_ID
			PGSd_PC_CONFIGURATION
			PGSd_PC_INPUT_FILE_NAME
			PGSd_PC_INPUT_FILE_ATTRIBUTE
			PGSd_PC_INPUT_FILE_NUMFILES
			PGSd_PC_PRODUCT_IN_DEFLOC
			PGSd_PC_PRODUCT_IN_UREF
			PGSd_PC_OUTPUT_FILE_NAME
			PGSd_PC_OUTPUT_FILE_ATTRIBUTE
			PGSd_PC_OUTPUT_FILE_NUMFILES
			PGSd_PC_PRODUCT_OUT_DEFLOC
			PGSd_PC_PRODUCT_OUT_UREF
			PGSd_PC_TEMPORARY_FILE
			PGSd_PC_TEMP_FILE_DEFLOC
			PGSd_PC_INTERMEDIATE_INPUT
			PGSd_PC_INTER_IN_DEFLOC
			PGSd_PC_INTER_IN_UREF
			PGSd_PC_INTERMEDIATE_OUTPUT
			PGSd_PC_INTER_OUT_DEFLOC
			PGSd_PC_INTER_OUT_UREF
			PGSd_PC_SUPPORT_IN_NAME
			PGSd_PC_SUPPORT_IN_ATTR
			PGSd_PC_SUPPORT_IN_DEFLOC
			PGSd_PC_SUPPORT_IN_UREF
			PGSd_PC_SUPPORT_IN_NUMFILES
			PGSd_PC_SUPPORT_OUT_NAME
			PGSd_PC_SUPPORT_OUT_ATTR
			PGSd_PC_SUPPORT_OUT_UREF
			PGSd_PC_SUPPORT_OUT_NUMFILES

	line		line of data containing requested
			PCS data


OUTPUTS:
	Name		Description			Units	Min	Max

	outstr		The output string value of the 
			requested parameter.  Output
			results are:

			If mode is PGSd_PC_PRODUCTION_RUN_ID 
			outstr is a generic string.

			If mode is PGSd_PC_SOFTWARE_ID 
			outstr is a generic string.

			If mode is PGSd_PC_CONFIGURATION 
			outstr is a generic string.

			If mode is PGSd_PC_INPUT_FILE_NAME 
			outstr is a file name containing the 
			full path, numFiles will contain the 
			number of files remaining for the 
			specified identifier.

			If mode is PGSd_PC_INPUT_FILE_ATTRIBUTE 
			outstr is a file name containing the 
			file attribute.

			If mode is PGSd_PC_INPUT_FILE_NUMFILES 
			numFiles contains the number of input 
			files associated with the specified 
			identifier.

			If mode is PGSd_PC_OUTPUT_FILE_NAME 
			outstr is a file name containing the 
			full path.

			If mode is PGSd_PC_OUTPUT_FILE_NUMFILES 
			numFiles contains the number of output 
			files associated with the specified 
			identifier.

			If mode is PGSd_PC_TEMPORARY_FILE 
			outstr is a file name containing the 
			full path.

			If mode is PGSd_PC_INTERMEDIATE_INPUT 
			outstr is a file name containing the 
			full path.

			If mode is PGSd_PC_INTERMEDIATE_OUTPUT 
			outstr is a file name containing the 
			full path.

			If mode is PGSd_PC_SUPPORT_IN_NAME 
                        outstr is a file name containing 
                        the full path.

                        If mode is PGSd_PC_SUPPORT_IN_ATTR 
                        outstr is a file name containing 
                        the file attribute.

			If mode is PGSd_PC_SUPPORT_IN_NUMFILES 
			numFiles contains the number of output 
			files associated with the specified 
			identifier.

                        If mode is PGSd_PC_SUPPORT_OUT_ATTR 
                        outstr is a file name containing 
                        the file attribute.

                        If mode is PGSd_PC_SUPPORT_OUT_NAME 
                        outstr is a file name containing 
                        the full path.

			If mode is PGSd_PC_SUPPORT_OUT_NUMFILES 
			numFiles contains the number of output 
			files associated with the specified 
			identifier.

	numFiles	this value state which version of the 
			INPUT file to receive the requested 
			information on.

RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_W_NO_CONFIG_VALUE     incorrect number of configuration 
                                    parameters
	PGSPC_E_INVALID_MODE        mode value does not match those in PGS_PC.h
	PGSPC_E_FILE_READ_ERROR     error reading input file
	PGSPC_E_LINE_FORMAT_ERROR   error in format of line in file
        PGSPC_E_NO_DEFAULT_LOC      no default file location in PCF

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	FILE		*locationPCS;
	PGSt_integer	mode
	char		line[PGSd_PC_LINE_LENGTH_MAX];
	char		outstr[PGSd_PC_LINE_LENGTH_MAX];
	PGSt_integer	*numFiles;

	/# assume that the correct line has been received #/

	mode = PGSd_PC_TEMPORARY_FILE;
	returnStatus = PGS_PC_GetPCSDataRetrieveData(locationPCS,mode,line,
		outstr,numFiles);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
		{
		 /# temporary file name is now "outstr" #/
		}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	All configuration parameters, except for physical file name(s), will
	have a one-to-one correspondence between a logical value and a  
	parameter value (string).

	In order for this tool to function properly, a valid Process Control
	file will need to be created first.  Please refer to Appendix C 
	(User's Guide) for instructions on how to create such a file.

REQUIREMENTS:  
	PGSTK-1280, 1290, 1310

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE"

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable
	PGS_PC_GetPCSDataGetRequest      Get requested data
	PGS_PC_GetPCSDataGetFileName     Get a file name from line
	PGS_SMF_GetMsgByCode             Get error/status from code
	PGS_SMF_SetStaticMsg             Set static error/status message
	PGS_SMF_SetDynamicMsg            Set dynamic error/status message

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_PC_GetPCSDataRetrieveData(           /* get the data requested */
    FILE             *locationPCS,       /* location of PCS data */
    PGSt_integer      mode,              /* input mode */
    char             *line,              /* line of data */
    char             *outstr,            /* output string */
    PGSt_integer     *numFiles)          /* number of files */

{
    char          buf[PGS_SMF_MAX_MSG_SIZE]; /* message buffer */
    char          msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char          temp[PGSd_PC_LINE_LENGTH_MAX]; /* temporary buffer */
    int           linePos;               /* line position counter */
    int           tempPos;               /* temp line position counter */
    int           count;                 /* line counter */
    int           found;                 /* status of request */
    PGSt_SMF_status whoCalled;           /* user that called this function */
    PGSt_SMF_status returnStatus;        /* function return value */

/***************************************************************************
*    Initialize variables.     
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;

/***************************************************************************
*    Initialize caller variable.     
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*    Switch based on the mode passed in.
***************************************************************************/
    switch (mode)
    {

/***************************************************************************
*    In this case identifier is not used since these are what we call 
*    system defined configuration parameters.  The value met in the 
*    case statement is also the location of the value in the file.
*    That is how many lines below the total number of values stored
*    to begin with.
***************************************************************************/
        case PGSd_PC_PRODUCTION_RUN_ID:
        case PGSd_PC_SOFTWARE_ID:

/***************************************************************************
*    Initialize the variables needed for the search.
***************************************************************************/
            found = 1;
            count = 0;

/***************************************************************************
*    Start looping and counting the number of lines we have looped
*    through.  Make sure we only count valid lines.
***************************************************************************/
            while (count < mode)
            {

/***************************************************************************
*    Read a line from the file.
***************************************************************************/
                if ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,locationPCS))
                         == NULL)
                {
                    returnStatus = PGSPC_E_FILE_READ_ERROR;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(returnStatus,msg);
                        sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                        PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                          "PGS_PC_GetPCSDataRetrieveData()");
                    }
                    found = 0;
                    break;
                }

/***************************************************************************
*    If the line is a comment line get out of the loop so we can read
*    the next line.
***************************************************************************/
                else if (line[0] == PGSd_PC_COMMENT)
                {
                    continue;
                }

/***************************************************************************
*    If the line is a divider line then we have a problem.  We should
*    not hit a divider here!
***************************************************************************/
                else if (line[0] == PGSd_PC_DIVIDER)
                {
                    returnStatus = PGSPC_W_NO_CONFIG_VALUE;
                    found = 0;
                    break;
                }

/***************************************************************************
*     We hit valid line so let's increment our counter.
***************************************************************************/
                else
                {
                    count++;
                }
            }   /* end while */

/***************************************************************************
*     We found the line we were looking for let's load our string with
*     whatever is in there (don't forget to add the NULL character
*     at the end) and send it back to whomever is calling us.
***************************************************************************/
            if (found)
            {
                linePos = 0;
                while (line[linePos] != PGSd_PC_NEWLINE)
                {  
                    outstr[linePos] = line[linePos];
                    linePos++;
                }
                outstr[linePos] = '\0'; 
                returnStatus = PGS_S_SUCCESS;
            }
            break;

/***************************************************************************
*    This is what we are calling user-defined configuration parameters.
***************************************************************************/
        case PGSd_PC_CONFIGURATION:

/***************************************************************************
*    OK, it seems that we have the correct line so let's get what we
*    need from the line, as always we are checking for problems.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetRequest(2,PGSd_PC_NEWLINE,
                                           line,outstr);

            if (returnStatus == PGSPC_W_NO_DATA_PRESENT)
            {
                returnStatus = PGSPC_W_NO_CONFIG_VALUE;
            } 

            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
              
/***************************************************************************
*    Make sure that the length is correct.
***************************************************************************/
            if ((strlen(outstr)) > (size_t) PGSd_PC_VALUE_LENGTH_MAX)
            {
                returnStatus = PGSPC_E_LINE_FORMAT_ERROR;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_GetMsgByCode(returnStatus,msg);
                    sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                    PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                         "PGS_PC_GetPCSDataRetrieveData()");
                }
            }

            break;

/***************************************************************************
*    This is where we look for the input file information.  Currently,
*    we are only returning the validityFlag or the fileName, but it will
*    be easy to add more options.
***************************************************************************/
        case PGSd_PC_INPUT_FILE_NAME:
        case PGSd_PC_INPUT_FILE_ATTRIBUTE:

/***************************************************************************
*    Get the number of files remaining for this product id.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetRequest(6,PGSd_PC_NEWLINE,
                                           line,outstr);
            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
            *numFiles = atoi(outstr);

/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                               PGSd_PC_INPUT_FILE_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    This is where we look for the output file information.
***************************************************************************/
        case PGSd_PC_OUTPUT_FILE_NAME:
        case PGSd_PC_OUTPUT_FILE_ATTRIBUTE:

/***************************************************************************
*    Get the number of files remaining for this product id.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetRequest(6,PGSd_PC_NEWLINE,
                                           line,outstr);
            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
            *numFiles = atoi(outstr);

/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                                       PGSd_PC_OUTPUT_FILE_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    Get Temporary file names.
***************************************************************************/
        case PGSd_PC_TEMPORARY_FILE: 

/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                                  PGSd_PC_TEMP_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    Get Intermediate input file names.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_INPUT: 

/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                                  PGSd_PC_INTER_INPUT_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    Get Intermediate output file names.
***************************************************************************/
        case PGSd_PC_INTERMEDIATE_OUTPUT: 

/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                                  PGSd_PC_INTER_OUTPUT_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    Get Support input file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_IN_NAME: 
        case PGSd_PC_SUPPORT_IN_ATTR: 

            returnStatus = PGS_PC_GetPCSDataGetRequest(6,PGSd_PC_NEWLINE,
                                           line,outstr);
            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
            *numFiles = atoi(outstr);
/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                                  PGSd_PC_SUPPT_INPUT_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    Get Support output file information.
***************************************************************************/
        case PGSd_PC_SUPPORT_OUT_NAME: 
        case PGSd_PC_SUPPORT_OUT_ATTR: 

            returnStatus = PGS_PC_GetPCSDataGetRequest(6,PGSd_PC_NEWLINE,
                                           line,outstr);
            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
            *numFiles = atoi(outstr);
/***************************************************************************
*    Get the file name and full path to send back to the calling
*    function.  Once again, error checking all the way.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetFileName(mode,line,
                                  PGSd_PC_SUPPT_OUT_ENVIRONMENT,outstr);
            break;

/***************************************************************************
*    Get the total number of files for the proper file type.
***************************************************************************/
        case PGSd_PC_INPUT_FILE_NUMFILES:
        case PGSd_PC_OUTPUT_FILE_NUMFILES:
        case PGSd_PC_SUPPORT_IN_NUMFILES: 
        case PGSd_PC_SUPPORT_OUT_NUMFILES: 
            returnStatus = PGS_PC_GetPCSDataGetRequest(6,PGSd_PC_NEWLINE,
                                      line,outstr);

            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
            *numFiles = atoi(outstr);
            break;

/***************************************************************************
*    This line contains default file location data.
***************************************************************************/
        case PGSd_PC_PRODUCT_IN_DEFLOC:
        case PGSd_PC_PRODUCT_OUT_DEFLOC:
        case PGSd_PC_TEMP_FILE_DEFLOC:
        case PGSd_PC_INTER_IN_DEFLOC:
        case PGSd_PC_INTER_OUT_DEFLOC:
        case PGSd_PC_SUPPORT_IN_DEFLOC:
        case PGSd_PC_SUPPORT_OUT_DEFLOC:

/***************************************************************************
*    Find the first character of data.
***************************************************************************/
            linePos = 1;
            while ((!(isgraph(line[linePos]))) && 
                     (line[linePos] != PGSd_PC_NEWLINE))
            {
                linePos++;
            }

/***************************************************************************
*    Copy the data into a temporary string.
***************************************************************************/
            tempPos = 0;
            while (line[linePos] != PGSd_PC_NEWLINE)
            {
                temp[tempPos] = line[linePos];
                tempPos++;
                linePos++;
            }
 
/***************************************************************************
*    Ensure that the string has data, but does not exceed the maximum
*    allowable path length.
***************************************************************************/
            if (tempPos > 0)
            {
                temp[tempPos] = PGSd_PC_CHAR_NULL;
 
                if ((int) strlen(temp) <= PGSd_PC_PATH_LENGTH_MAX)
                {
                    strcpy(outstr,temp); 
                }
                else 
                {
                    returnStatus = PGSPC_E_LINE_FORMAT_ERROR;
                    if (whoCalled != PGSd_CALLERID_SMF)
                    {
                        PGS_SMF_GetMsgByCode(returnStatus,msg);
                        sprintf(buf,msg,getenv(PGSd_PC_INFO_FILE_ENVIRONMENT));
                        PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                        "PGS_PC_GetPCSDataRetrieveData()");
                    }

                }
            }
            else
            {
                returnStatus = PGSPC_E_NO_DEFAULT_LOC;
                if (whoCalled != PGSd_CALLERID_SMF)
                {
                    PGS_SMF_SetStaticMsg(returnStatus,
                                      "PGS_PC_GetPCSDataRetrieveData()");
                }
            }
            break;

/***************************************************************************
*    Get the universal reference associated with this product id
*    and version.
***************************************************************************/
        case PGSd_PC_PRODUCT_IN_UREF:
        case PGSd_PC_PRODUCT_OUT_UREF:
        case PGSd_PC_INTER_IN_UREF:
        case PGSd_PC_INTER_OUT_UREF:
        case PGSd_PC_SUPPORT_IN_UREF:
        case PGSd_PC_SUPPORT_OUT_UREF:

/***************************************************************************
*    Get the number of files remaining for this product id.
***************************************************************************/
            returnStatus = PGS_PC_GetPCSDataGetRequest(6,PGSd_PC_NEWLINE,
                                           line,outstr);
            if (returnStatus != PGS_S_SUCCESS)
            {
                break;
            }
            *numFiles = atoi(outstr);

            returnStatus = PGS_PC_GetPCSDataGetRequest(4,PGSd_PC_DELIMITER,
                                           line,outstr);
            break;

/***************************************************************************
*    Hey, the mode value does not match anything we handle.
***************************************************************************/
        default:
            returnStatus = PGSPC_E_INVALID_MODE;
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_GetMsgByCode(returnStatus,msg);
                sprintf(buf,msg,mode);
                PGS_SMF_SetDynamicMsg(returnStatus,buf,
                                  "PGS_PC_GetPCSDataRetrieveData()");
            }
            break;
      }   /* end switch */

/***************************************************************************
*    Set the message log and return the status to the calling function.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetPCSDataRetrieveData()");
            }
            break;
    }
    return returnStatus;
}
