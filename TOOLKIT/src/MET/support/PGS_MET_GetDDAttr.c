/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_GetDDAttr.c
 
DESCRIPTION:
         This file contains PGS_MET_GetDDAttr()and PGS_MET_CheckRange.
         These functions are used by the MET tools to retrieve specific attributes 
	 belonging to metadata parameters and checking the range of the given metadata 
	 parameter.

AUTHOR:
  	Alward N.Siyyid / EOSL

HISTORY:
  	18-May-1995 	ANS 	Initial version
	31-May-1995     ANS     Code inspection comments update
	20-July-1995	ANS	Fixed DR ECSed01012

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <PGS_MET.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  
  	This function gets the metadata parameter attribute(type, range etc) 
	value from the data dictionary
 
NAME:
  	PGS_MET_GetDDAttr()

SYNOPSIS:
  	N/A

DESCRIPTION:
	The routine implements the data dictionary as an ODL representation 
	the very first time it is called and retrieves the requested attribute
	for the given metadata. It also parses the information for relevent 
	datatypes

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       	metaDataNode	ODL aggregate node	none	N/A	N/A
			containing metadat
			information 

OUTPUTS:
	None

RETURNS:
	PGSMET_E_LOAD_ERR		Unable to load data dictionary information
	PGSMET_E_DD_UNKNOWN_PARM	The requested metadata parameter 
					could not be found in data dictionary
	PGSMET_E_UNKNOWN_PARM_ATTR	The requested metadata attribute(type, range etc)
					is not defined for the given metadata
	
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
	PGS_MET_LoadAggregate
	FindObject
	FindParameter
	FirstValue
	NextValue
END_PROLOG:
***************************************************************************/


AGGREGATE        PGSg_MET_DDHandle = NULL; /* ODL tree node for Data dictionary */

PGSt_SMF_status
PGS_MET_GetDDAttr(			    /* Retrieves data dictionary info */
             char *parmName, 		    /* Parameter name */
             char *attrName, 		    /* Parameter attr to be retrieved */
             void *attrValue)              /* Attribute value buffer to hold
                                             * the retrieved value
                                             */
{
	OBJECT			parmNode = NULL;	/* parameter requeted */
	PARAMETER		attrNode = NULL;	/* parameter attribute requested */
	VALUE			valueNode = NULL;	/* value of the attribute */
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer           loopCount = 0;
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char *			outString;	/* pointer used to place values in void buffer */
	PGSt_integer *		outInteger;	/* pointer used to place values in void buffer */
	PGSt_double *		outDouble;	/* pointer used to place values in void buffer */

	/* if this routine is called first time then load DD */

	if(PGSg_MET_DDHandle == NULL)
	{
		retVal = PGS_MET_LoadAggregate(PGSd_MET_DD_FILE_ID,
					       "DDaggregate",
					       PGSd_PC_INPUT_FILE_NAME,
					       &PGSg_MET_DDHandle);
		if(retVal != PGS_S_SUCCESS)
           	{
 	               /* error message is:
                	"Unable to load <DD> information"*/
			errInserts[0] = "Data Dictionary";

                	(void) PGS_MET_ErrorMsg(PGSMET_E_LOAD_ERR,
					"PGS_MET_GetDDAttr", 1, errInserts);
                	return(PGSMET_E_LOAD_ERR);
           	}
	}

	parmNode = FindObject(PGSg_MET_DDHandle, parmName, "");
	if(parmNode == NULL)
	{
                errInserts[0] = parmName;
		errInserts[1] = PGSg_MET_DDHandle->name;

                /* error message is:
                "The requested parameter <parameter name> could not be found in <agg node>" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_DD_UNKNOWN_PARM,
                                     "PGS_MET_GetDDAttr", 2, errInserts);
                return(PGSMET_E_DD_UNKNOWN_PARM);
        }
	 
	attrNode = FindParameter(parmNode, attrName);
        if(attrNode == NULL)
        {
                errInserts[0] = attrName;
		errInserts[1] = parmName;

                /* error message is:
                "Attribute <attribute name> is not defined for parameter <parameter name> */

                (void) PGS_MET_ErrorMsg(PGSMET_E_UNKNOWN_PARM_ATTR,
                                     "PGS_MET_GetDDAttr", 2, errInserts);
                return(PGSMET_E_UNKNOWN_PARM_ATTR);
        }
		
	valueNode = FirstValue(attrNode);
	/* no check for error return since such an error would be trapped while loading */

	for(loopCount =0; loopCount < attrNode->value_count; loopCount++)
	{
		if(valueNode->item.type == TV_STRING || valueNode->item.type == TV_SYMBOL)
		{
			if(loopCount == 0)
                        {
                                outString= (char *) attrValue;
                        }
                        else
                        {
                                outString = outString + PGSd_MET_DD_STRING_L;
                        }
			strcpy(outString, valueNode->item.value.string);
		}
		else if(valueNode->item.type == TV_INTEGER)
		{
			if(loopCount == 0)
			{
				outInteger = (PGSt_integer *) attrValue;
			}
                	else
			{
				outInteger++;
			}
			*outInteger = valueNode->item.value.integer.number;
        	}
		else if(valueNode->item.type == TV_REAL)
        	{
                	if(loopCount == 0)
                        {
                                outDouble = (PGSt_double *) attrValue;
                        }
                        else
                        {
                                outDouble++;
                        }
                	*outDouble = valueNode->item.value.real.number;
        	}
		valueNode = NextValue(valueNode);
	}
	return(PGS_S_SUCCESS);
}

/***************************************************************************
BEGIN_PROLOG:
 
        This is a private routine used by the toolkit functions and therefore
        not all the prolog fields are applicable to the routine and are marked
        as N/A
 
TITLE:
        Checks Metadata values for range
 
NAME:
        PGS_MET_CheckRange()
 
SYNOPSIS:
        N/A
 
DESCRIPTION:
	This routine checks a single metadata value against the limits
	defined in data dictionary

INPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
        parmName	meta data parameter	none	N/A	N/A
			name
	parmValue	parameter value to be	none	N/A	N/A
			checked
	odlType		ODL type definition	none	defined defined
			(TV_INTEGER, TV_STRING
			etc)
 
OUTPUTS:
        None 

RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_DD_UNKNOWN_PARM	The requested metadata parameter is not defined
					in DD
	PGSMET_E_UNKNOWN_PARM_ATTR	Attribute range is not defined for metadata parameter
	PGSMET_E_OUTOFRANGE		Metadata value is out of range
	PGSMET_E_INV_ODL_TYPE		Input odl type is invalid
	
 
EXAMPLES:
        N/A
 
NOTES:
        If the range is defined as FREE_RANGE in the data dictionary
	for any value type, the routine simply returns PGS_S_SUCCESS.
	This allows for datatypes with no specific range.
 
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
	FindObject
	FindParameter
	FirstValue
	NextValue
 
END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_MET_CheckRange(			    /* Checks a given single value  of metadata against
					     * possible values in the data dictionary 
					     */
             char *parmName, 		    /* meta data parameter name */
             void *parmValue,		    /* parameter value to be checked */
	     PGSt_integer odlType)           /* data type as defined in odl eg. TV_INTEGER etc */

{
	OBJECT			parmNode = NULL;	/* parameter requeted */
	PARAMETER		attrNode = NULL;	/* parameter attribute requested */
	VALUE                   minimum = NULL;
	VALUE			maximum = NULL;
	VALUE			possibleValue = NULL;
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char *			attrName = NULL;
        char *                  inString;      /* pointer used to place values in void buffer */
        PGSt_integer *          inInteger;     /* pointer used to place values in void buffer */
        PGSt_double *           inDouble;      /* pointer used to place values in void buffer */

	PGSt_integer		compareValue;
	char *			funcName = "PGS_MET_Checkrange";
	PGSt_double		minVal = 0.0;
	PGSt_double		maxVal = 0.0;

/* PGSg_MET_DDHandle is available to all the functions in this file only */

	parmNode = FindObject(PGSg_MET_DDHandle, parmName, "");
	if(parmNode == NULL)
	{
                errInserts[0] = parmName;
		errInserts[1] = PGSg_MET_DDHandle->name;

                /* error message is:
                "The requested parameter <parameter name> could not be found in <agg node>" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_DD_UNKNOWN_PARM,
                                     funcName, 2, errInserts);
                return(PGSMET_E_DD_UNKNOWN_PARM);
        }
	 
	attrName = "RANGE";
	attrNode = FindParameter(parmNode, attrName);
        if(attrNode == NULL)
        {
                errInserts[0] = attrName;
		errInserts[1] = parmName;
		errInserts[2] = PGSg_MET_DDHandle->name;

                /* error message is:
                "Attribute <attribute name> is not defined for parameter <parameter name> in 
		 <aggregate name>" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_UNKNOWN_PARM_ATTR,
                                     funcName, 3, errInserts);
                return(PGSMET_E_UNKNOWN_PARM_ATTR);
        }
		
	if(odlType == TV_INTEGER || odlType == TV_REAL)
	{
		minimum = FirstValue(attrNode);
		maximum = NextValue(minimum);
		if(odlType == TV_INTEGER)
		{
			minVal = (PGSt_double) minimum->item.value.integer.number;
			maxVal = (PGSt_double) maximum->item.value.integer.number;
		}
		else
		{
			minVal = (PGSt_double) minimum->item.value.real.number;
			maxVal = (PGSt_double) maximum->item.value.real.number;
		}
		if(minVal > maxVal)
		{
			errInserts[0] = parmName;
        	        errInserts[1] = PGSg_MET_DDHandle->name;
	
                	/* error message is:
        	        "Minimum value is greater than the maximum value for paramter <name> in the <aggregate name>"
 	                <aggregate name>" */

                	(void) PGS_MET_ErrorMsg(PGSMET_E_MINMAX_ERR,
                                     funcName, 2, errInserts);
                	return(PGSMET_E_MINMAX_ERR);
        	}

		if(odlType == TV_INTEGER)
                {
                    inInteger = (PGSt_integer *) parmValue;
		    if(*inInteger < minimum->item.value.integer.number ||
		       *inInteger > maximum->item.value.integer.number)
		    {
			return(PGSMET_E_OUTOFRANGE);
		    }
                }
                else
                {
                    inDouble = (PGSt_double *) parmValue; 
                    if(*inDouble < minimum->item.value.real.number ||
                       *inDouble > maximum->item.value.real.number) 
		    {
                        return(PGSMET_E_OUTOFRANGE);
                    }
                }
	}
	else if (odlType == TV_STRING || odlType == TV_SYMBOL)
	{
		inString = (char *) parmValue;
		possibleValue = FirstValue(attrNode);

		/* simply return success if PGSd_MET_FREE_RANGE */

		compareValue = strcmp(possibleValue->item.value.string, PGSd_MET_FREE_RANGE);
		if(compareValue != 0)
		{
			while(possibleValue != NULL && compareValue != 0)
			{
				compareValue = strcmp(possibleValue->item.value.string, inString);
				possibleValue = NextValue(possibleValue);
			}
			if(compareValue != 0)
			{
				return(PGSMET_E_OUTOFRANGE);
                	}
		}
	}
	else
	{
                /* error message is:
                "Input odl type is invalid" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_INV_ODL_TYPE,
                                     funcName, 0, errInserts);
                return(PGSMET_E_INV_ODL_TYPE);
        }
			
	return(PGS_S_SUCCESS);
}
