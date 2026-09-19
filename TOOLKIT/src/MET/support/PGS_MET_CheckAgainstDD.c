/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_CheckAgainstDD.c
 
DESCRIPTION:
         The file contains PGS_MET_CheckAgainstDD.
         This function is used by PGS_MET_Init() to check the metadata set
	 in the MCF file for type and range by comparing it against values and
	 limits given in the Data Dictionary

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
        Checks Metadata values for type, range and number of values if the metadata 
	is supposed to be an array
  	
 
NAME:
  	PGS_MET_CheckAgainstDD()

SYNOPSIS:
  	N/A

DESCRIPTION:
	All the required information is given in the Data Dictionary which is 
	also implemeted as an ODL representation in memory. The routines return error 
	if the metadata value(s) does not satisfy the criteria.

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
       	metaDataNode	ODL aggregate node	none	N/A	N/A
			containing metadat
			information 

OUTPUTS:
	None

RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_UNKNOWN_PARM_ATTR 	Attribute <metadata value> is not defined for
					parameter <metadata name>
	PGSMET_E_DD_ERR			Unable to access the data dictionary to obtain 
					<type, range, etc> of parameter <metadata name>
	PGSMET_E_MCF_DD_CONFLICT	Conflict with data dictionary for Meta data <name>.
					The data dictionary definition of <type, num of values, etc>
					is <definition in data dictionary>
	PGSMET_E_OUTOFRANGE		Value of metadata <name> at position <position> is out of range
	PGSMET_E_CHECK_RANGE_ERR	Unable to check the range for metaData <name>
	

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
	PGS_MET_GetDDAttr
	PGS_MET_CheckRange
	FindParameter
	FirstValue
	NextValue

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_MET_CheckAgainstDD(			    /* Checks a given metadata for type range etc */
             AGGREGATE metaDataNode) 	    /* Parameter odl node to be tested */
{
	PARAMETER		parmNode = NULL;	/* parameter 'VALUE' of the given Metadata*/
	VALUE			valueNode = NULL;	/* value node to point to value(s) of Metadata */
	VALUE			typeValueNode = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer            mdNumOfVals = 0;	/* number of values as defined in DD */
	PGSt_integer            valCount = 0;
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char 			metaDataType[PGSd_MET_DD_STRING_L]; /* type of metadata as defined in DD */
	VALUE_TYPE		valueType = TV_NULL;

/* find the sub-parameter containing the values */
	
	parmNode = FindParameter(metaDataNode, PGSd_MET_ATTR_VALUE_STR);
	valueNode = FirstValue(parmNode);
	valCount = 1;
        while (valueNode != NULL)
        {
                valueNode = NextValue(valueNode);
                if(valueNode != NULL)
                {
                        valCount++;
                }
        }
	valueNode = FirstValue(parmNode);

	parmNode = FindParameter(metaDataNode, PGSd_MET_ATTR_TYPE_STR);
	typeValueNode = FirstValue(parmNode);
	strcpy(metaDataType, typeValueNode->item.value.string);
	if(strcmp(metaDataType, PGSd_MET_INTEGER_STR) == 0 || strcmp(metaDataType, PGSd_MET_UINTEGER_STR) == 0)
	{
		valueType = TV_INTEGER;
	}
	else if(strcmp(metaDataType, PGSd_MET_FLOAT_STR) == 0 ||
           strcmp(metaDataType, PGSd_MET_DOUBLE_STR) == 0)
	{
		valueType = TV_REAL;
	}
	else if(strcmp(metaDataType, PGSd_MET_STRING_STR) == 0)
	{
		/* in Odl, a string of one word can also have type TV_SYMBOL
		   if the value of the datatype consist of just one word
		   and not enclosed by quotes */

		valueType = TV_STRING;
		if(valueNode->item.type == TV_SYMBOL)
		{
			valueType = TV_SYMBOL;
		}
	}

/* check if the type is correct */

	if(valueNode->item.type != valueType)
	{
                retVal = PGSMET_E_MCF_TYPE_CONFLICT;
        }
	
	/* check if the number of values for the metadata is correct */
	
	parmNode = FindParameter(metaDataNode, PGSd_MET_ATTR_NUMOFVAL_STR);
	valueNode = FirstValue(parmNode);
	mdNumOfVals = (PGSt_integer)valueNode->item.value.integer.number;
	
	if(valCount > mdNumOfVals)
        {
                retVal = PGSMET_E_MCF_NUMVAL_CONFLICT;
        }

	/* Now check the range of each of the values defined for the meta data */

	return(retVal);
}
