/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_RetrieveConfigData.c
 
DESCRIPTION:
         The file contains PGS_MET_RetrieveConfigData.
         This function is used by PGS_MET_Init() and PGS_MET_GetConfigData
	 to extract and parse configuration parameter values from the PC file

AUTHOR:
  	Alward N.Siyyid / EOSL

HISTORY:
  	18-May-1995 	ANS 	Initial version
	31-May-1995     ANS     Code inspection comments update

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
  	Extracts and parses configuration parameters from the PC file
 
NAME:
  	PGS_MET_RetrieveConfigData()


SYNOPSIS:
  	N/A

DESCRIPTION:
	Extracts and parses configuration parameters values from the PC file and 
	attaches it to the given ODL representation of the parameter

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
        attrNode	aggregate node 		none	N/A	N/A
			representing the 
			parameter

OUTPUTS:
	None

RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_CONFIG_VAL_STR_ERR	Unable to obtain the value of configuration parameter 
					from the PCS file
	PGSMET_E_AGGREGATE_ERR		Unable to create odl aggregate 
	PGSMET_E_CONFIG_CONV_ERR	Unable to convert the value of configuration parameter
					from the PCS file into an ODL format
	PGSMET_E_INCORRECT_VAL		Illegal value of the parameter defined in the PC table 

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
	PGS_MET_GetConfigByLabel
	PGS_MET_CheckAgainstDD
	NewAggregate
	ReadValue
	FindParameter
	RemoveParameter
	CopyParameter
	PasteParameter
	RemoveAggregate
	

END_PROLOG:
***************************************************************************/

#include <PGS_MET.h>

PGSt_SMF_status
PGS_MET_RetrieveConfigData(		    /* Retrieves the value of Config parameter
					     * from the PC table, checks it against DD and
					     * and attaches it to the given aggregate node
					     */
             AGGREGATE attrNode) 	    /* aggregate node representing the parameter */
{
	AGGREGATE		aggTempNode = NULL;
	PARAMETER		parmNode = NULL;	/* parameter 'VALUE' of the given Metadata*/
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char * 			funcName = "PGS_MET_RetrieveConfigData";
	char 			valString[PGSd_PC_LINE_LENGTH_MAX] = "";
	char			valArray[PGSd_PC_LINE_LENGTH_MAX +2] = "";

/* obtain the value string from the PC table */

	retVal = PGS_MET_GetConfigByLabel(attrNode->name, valString);
        if(retVal != PGS_S_SUCCESS)
        {
		if(attrNode == NULL)
		{
			errInserts[0] = "NULL";
		}
		else
		{
			errInserts[0] = attrNode->name;
		}

                /* error message is:
                "Unable to obtain the value of configuration parameter <name> 
		 from the PCS file" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_CONFIG_VAL_STR_ERR,
                                     funcName, 1, errInserts);
                return(PGSMET_E_CONFIG_VAL_STR_ERR);
        }
/* allow to handle arrays of values */

	(void) sprintf(valArray, "(%s)", valString);
	aggTempNode = CopyAggregate(attrNode);
        if (aggTempNode == NULL)
        {
            errInserts[0] = "TEMPORARY";
            /* error message is:
               "Unable to create odl aggregate <aggregate name>" */

            (void) PGS_MET_ErrorMsg(PGSMET_E_AGGREGATE_ERR, "PGS_MET_LoadAggregate",
                                    1, errInserts);
            return(PGSMET_E_AGGREGATE_ERR);
        }

	parmNode = FindParameter(attrNode, PGSd_MET_ATTR_VALUE_STR);
        if(parmNode != NULL)
        {
                RemoveParameter(parmNode);
        }

/* Convert the string into an ODL format and attach it to the given agg node */

	retVal = ReadValue(aggTempNode, "VALUE", valArray);
	if(retVal != 1)
        {
                errInserts[0] = attrNode->name;

                /* error message is:
                "Unable to convert the value of configuration parameter <name> 
                 from the PCS file into an ODL format" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_CONFIG_CONV_ERR,
                                     funcName, 1, errInserts);
		RemoveAggregate(aggTempNode);
                return(PGSMET_E_CONFIG_CONV_ERR);
        }

/* check the new values against the DD */

	retVal = PGS_MET_CheckAgainstDD(aggTempNode);
	if(retVal != PGS_S_SUCCESS)
        {
                errInserts[0] = attrNode->name;

                /* error message is:
                "Illegal value of the parameter <name> defined in the PC table */

                (void) PGS_MET_ErrorMsg(PGSMET_E_INCORRECT_VAL,
                                     funcName, 1, errInserts);
		RemoveAggregate(aggTempNode);
                return(PGSMET_E_INCORRECT_VAL);
        }

/* now remove old value and insert the new value into the given aggregate node */
/* it doesn't matter if the old value has a parameter VALUE defined or not */

	parmNode = FindParameter(attrNode, PGSd_MET_ATTR_VALUE_STR);
	if(parmNode != NULL)
	{
		RemoveParameter(parmNode);
	}

	parmNode = FindParameter(aggTempNode, PGSd_MET_ATTR_VALUE_STR);
	parmNode = CopyParameter(parmNode);
	
	parmNode = PasteParameter(attrNode, parmNode);

/* now remove the temporary aggregate node before exiting */

	RemoveAggregate(aggTempNode);

	return(PGS_S_SUCCESS);
}
