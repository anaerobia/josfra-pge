/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_GetConfigByLabel.c
 
DESCRIPTION:
         The file contains PGS_MET_GetConfigByLabel.
         This function is used by PGS_MET_PGS_MET_RetrieveConfigData.c()
	 to retrieve the value string for the given Config metadata from
	 the PC table. 

AUTHOR:
  	Alward N.Siyyid / EOSL
        Carol S. W. Tsai / Space Applications Corporation

HISTORY:
  	18-May-1995 	ANS 	Initial version
	31-May-1995     ANS     Code inspection comments update
        10-Nov-1997     CSWT    Added code to close a PCF (Process Control
                                file)  that never closes after openning to 
                                retrieve attributes (This change is for NCR
                                ECSed09822 about A PCF never closes after
                                openning in function PGS_MET_GetConfigByLabel)

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <PGS_MET.h>
/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  
  	Retrieves Config metadata parameter value string from the PC table
 
NAME:
  	PGS_MET_GetConfigByLabel()

SYNOPSIS:
  	N/A

DESCRIPTION:
	There are user defined Configuration parameters in the PC table
	This routine provides access to the values of such parameters.
	However the values are written as strings in ODL format and have to be
	parsed by the calling function

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	attrName 	metadata attribute name	none	N/A	N/A

OUTPUTS:
	data		metadata value string	none    N/A     N/A
RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_PCS_OPEN_ERR		Unable to open PCS file
	PGSMET_E_LABEL_NOT_FOUND	Unable to find <label> in file <fileName>

EXAMPLES:
	N/A

NOTES:
	N/A

REQUIREMENTS:
	N/A

DETAILS:
	NONE

GLOBALS:
	NONE

FILES:
	NONE

FUNCTIONS_CALLED:
	PGS_MET_ErrorMsg
	PGS_PC_GetPCSDataOpenPCSFile
	PGS_PC_GetPCSDataAdvanceArea
	PGS_PC_GetPCSDataGetRequest
	fgets
	

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_MET_GetConfigByLabel(		    /* This function returns the ODL compatible
					     * parm=value configuration string from 
					     * the PC table
					     */
            char *	attrName,	    /* The name of the attribute */
	    char *	data)		    /* retrieved string data */
{
	FILE			*PCSfile = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char * 			funcName = "PGS_MET_GetConfigByLabel";
	char  			attrStr[PGSd_PC_LINE_LENGTH_MAX] = ""; /* file id value as string */
	PGSt_integer		labelFound = PGS_FALSE;
	PGSt_integer            delimiterNum = 0;


	strcpy(data, "");
/* Create a temporary file to hold the data */
	
	retVal = PGS_PC_GetPCSDataOpenPCSFile(&PCSfile);
        if(retVal != PGS_S_SUCCESS)
        {
                /* error message is:
                "Unable to open PCS file" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_PCS_OPEN_ERR, funcName,
                                    0, errInserts);
                return(PGSMET_E_PCS_OPEN_ERR);
        }

/* advance to the configuration section in the PCF file */

	retVal = PGS_PC_GetPCSDataAdvanceArea(PCSfile, PGSd_PC_CONFIG_COUNT);
	
/* search for the label */

	if(retVal == PGS_S_SUCCESS)
	{
	   do
	   {
		(void) fgets(attrStr, PGSd_PC_LINE_LENGTH_MAX, PCSfile);
		if(feof(PCSfile) == 0) /* non zero if EOF detected */
		{
			/* check if not another section or a comment line */
			if(attrStr[0] != PGSd_PC_DIVIDER &&
			   attrStr[0] != PGSd_PC_COMMENT) 
			{
				/* Its a valid line and now get the label which is after 
				 * the first delimeter */

				delimiterNum = 1;
				(void) PGS_PC_GetPCSDataGetRequest(delimiterNum,
					PGSd_PC_DELIMITER, attrStr, data);
	
				/* compare the returned string with the input string 
				 * if same then get the actual value string */	
				
				if(strcmp(attrName, data) == 0)
				{
					/* retrieve the value string */

					delimiterNum = 2;
					retVal = PGS_PC_GetPCSDataGetRequest(delimiterNum,
                                        PGSd_PC_NEWLINE, attrStr, data);

					if(retVal == PGS_S_SUCCESS)
					{
						labelFound = PGS_TRUE;
					}
				}
			}
		}
	   }
	   while(attrStr[0] != PGSd_PC_DIVIDER && labelFound == PGS_FALSE);
	}
	if(labelFound == PGS_FALSE) /* something is wrong with the PC file */
        {
                /* error message is:
                "Unable to find <label> in file <fileName>" */
                errInserts[0] = attrName;
                errInserts[1] = "PCS";

                (void) PGS_MET_ErrorMsg(PGSMET_E_LABEL_NOT_FOUND,
                                        funcName, 2, errInserts);
                fclose(PCSfile);
                return(PGSMET_E_LABEL_NOT_FOUND);
        }	

        if(retVal == PGS_S_SUCCESS)
        {
                fclose(PCSfile);
        }
        return(retVal);
}
