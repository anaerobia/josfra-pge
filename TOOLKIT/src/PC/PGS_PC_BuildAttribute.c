/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
	PGS_PC_BuildAttribute.c

DESCRIPTION:
	This file contains the function PGS_PC_BuildAttribute().

AUTHOR:
	Ray Milburn / Applied Research Corp.

HISTORY:
	05-Aug-94 RM Initial Version
	14-Oct-94 RM Added call to PGS_SMF_CallerID().
	18-Oct-94 RM Initialized file pointer for DR ECSed0262
 
END_FILE_PROLOG:
***************************************************************************/

#include <stdio.h>
#include <cfortran.h>
#include <PGS_PC.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE:
	Build the attribute from the attribute file.
 
NAME:
	PGS_PC_BuildAttribute()

SYNOPSIS:

C:
		#include <PGS_PC.h>
	
		PGSt_SMF_status
		PGS_PC_BuildAttribute(
			char			*attributeLoc,
			PGSt_integer		maxSize,
			char			*fileAttribute);

FORTRAN:
	NONE

DESCRIPTION:
	This tool may be used to retrieve an attribute associated with a 
	product from the attribute file.  The attribute is returned as 
	a string and left for the user to parse.
 
INPUTS:
	Name		Description			Units	Min	Max

	attributeLoc	The location (file) of the   
			attribute.

	maxSize		Maximum number of bytes to use
			in fileAttribute.

OUTPUTS:
	Name		Description			Units	Min	Max

	fileAttribute	The actual file attribute.
	
RETURNS:
	PGS_S_SUCCESS               successful execution
	PGSPC_E_FILE_OPEN_ERROR     error opening input file
	PGSPC_W_TRUNCATED           the attribute was truncated

EXAMPLES:

C:
	PGSt_SMF_status	returnStatus;
	PGSt_integer	maxSize;
	char		attributeLoc[PGSd_PC_FILE_PATH_MAX];
	char		fileAttribute[USER_DEFINED_LENGTH];

	/# the USER_DEFINED_LENGTH mentioned above is not controlled #/
	/# in this function and MUST be properly addressed by the #/
	/# calling function.  Failing to allocate the proper amount #/
	/# of memory will give undefined results #/

	maxSize = USER_DEFINED_LENGTH;

	/# assuming the attribute file name was successfully retrieved #/

	returnStatus = PGS_PC_BuildAttribute(attributeLoc,maxSize,
					fileAttribute);

	if (returnStatus != PGS_S_SUCCESS)
		goto EXCEPTION;
	else
	{

	 /# parse attribute for proper information #/

	}
			.
			.
			.
	EXCEPTION:
		return returnStatus;

FORTRAN:
	NONE

NOTES:
	Allocating enough space for the attribute variable will be the 
	responsibility of the application programmer.  This function will
	just return the contents of the attribute file to fileAttribute
	for maxSize bytes or the end of the file, which ever comes first.

REQUIREMENTS:  
	PGSTK-1290

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	This function opens and reads from the file defined in the variable
	attributeLoc.

FUNCTIONS_CALLED:
	PGS_SMF_CallerID		 Get value of user variable.

END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_PC_BuildAttribute(               /* build the attribute string */
    char             *attributeLoc,  /* attribute location */
    PGSt_integer      maxSize,       /* maximum size of attribute */
    char             *fileAttribute) /* file attribute */

{
    char              buf[PGS_SMF_MAX_MSG_SIZE];    /* message buffer */
    char              msg[PGS_SMF_MAX_MSGBUF_SIZE]; /* message buffer */
    char              line[PGSd_PC_LINE_LENGTH_MAX]; /* line read from file */ 
    FILE             *fp;            /* pointer to attribute file */
    PGSt_integer      bytesUsed;     /* number of bytes used */
    PGSt_integer      extra;         /* extra bytes to be truncated */
    PGSt_integer      pos;           /* position of NULL during truncation */
    PGSt_SMF_status   whoCalled;     /* user who called this function */
    PGSt_SMF_status   returnStatus;  /* function return */

/***************************************************************************
*   Initialize variables.
***************************************************************************/
    returnStatus = PGS_S_SUCCESS;
    bytesUsed = 0;
    fp = (FILE *) NULL;

/***************************************************************************
*   Initialize caller variable.
***************************************************************************/
    whoCalled = PGS_SMF_CallerID();

/***************************************************************************
*   Open the attribute file.  If there is a problem report it here and
*   skip everything else.
***************************************************************************/
    if ((fp = fopen(attributeLoc,"r")) == NULL)
    {
        returnStatus = PGSPC_E_FILE_OPEN_ERROR;
        if (whoCalled != PGSd_CALLERID_SMF)
        {
            PGS_SMF_GetMsgByCode(PGSPC_E_FILE_OPEN_ERROR,msg);
            sprintf(buf,msg,attributeLoc);
            PGS_SMF_SetDynamicMsg(PGSPC_E_FILE_OPEN_ERROR,buf,
                                 "PGS_PC_BuildAttribute()");
        }
    }

/***************************************************************************
*   Everything is OK.  Just read a line and append it, then read a line
*   and append it.....get it.  Do this until the EOF or you exceed
*   maxSize.
***************************************************************************/
    else
    {
        fileAttribute[0] = '\0';
        while ((fgets(line,PGSd_PC_LINE_LENGTH_MAX,fp)) != NULL)
        {
            bytesUsed += strlen(line);
            if (bytesUsed < maxSize)
            {
                strcat(fileAttribute,line);
            }
            else
            {
                extra = bytesUsed - maxSize;
                pos = strlen(line) - extra;
                line[pos] = '\0';
                strcat(fileAttribute,line);
                returnStatus = PGSPC_W_TRUNCATED;
                break;
            }
        }   /* end while */
    }   /* end else */

/***************************************************************************
*   Close the file.
***************************************************************************/
    if (fp != (FILE *) NULL)
    {
        fclose(fp);
    }

/***************************************************************************
*   Update the message log and return.
***************************************************************************/
    switch(PGS_SMF_TestStatusLevel(returnStatus))
    {
        case PGS_SMF_MASK_LEV_E:
            break;
        default:
            if (whoCalled != PGSd_CALLERID_SMF)
            {
                PGS_SMF_SetStaticMsg(returnStatus,"PGS_PC_GetFileAttr()");
            }
    }
    return returnStatus;
}
