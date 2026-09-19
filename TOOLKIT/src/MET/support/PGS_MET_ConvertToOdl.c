/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_ConvertToOdl.c
 
DESCRIPTION:
         The file contains PGS_MET_ConvertToOdl.
	 The function translates a char buffer containing data
	 in ODL format to ODL memory representation

AUTHOR:
  	Alward N.Siyyid / EOSL

HISTORY:
  	18-May-1995 	GJB 	Initial version
	31-May-1995     ANS     Code inspection comments update

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
  	Converts a char buffer to ODL aggregate
 
NAME:
  	PGS_MET_ConvertToOdl()

SYNOPSIS:
  	N/A

DESCRIPTION:
	Unfortunately, the ODL parser function only deals with
	ASCII files as input. This function circumvents this problem 

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	aggName		The name to be given 	none	N/A 	N/A
			to the new aggregate
	data		data to be converted 	none    N/A     N/A
			to ODL

OUTPUTS:
	aggNode		ODL aggregate to be 	none    N/A     N/A
			returned

RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_OPEN_ERR	Unable to open temporary file 
	PGSMET_E_LOAD_ERR	Unable to load aggregate information

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
	PGS_IO_Gen_Temp_Open
	PGS_IO_Gen_Close
	PGS_MET_LoadAggregate
	PGS_IO_Gen_Temp_Delete

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_MET_ConvertToOdl(			    /* This function is used to convert 
					     * a char buffer into an ODL format */
            char *	aggName,	    /* The name to be given to the new aggregate */
	    char *	data,		    /* data to be converted to ODL */
	    AGGREGATE   *aggNode) 	    /* ODL aggregate to be returned */
{
	PGSt_IO_Gen_FileHandle	*fileHandle = NULL;
	AGGREGATE		aggNode1 = NULL;
	AGGREGATE               objectNode = NULL;
	PARAMETER		classParmNode = NULL;
	VALUE			classValueNode = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer 		odlRetVal = 0;
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char * 			funcName = "PGS_MET_ConvertToOdl";
	char  			fileIdStr[PGSd_MET_FILE_ID_L]; /* file id value as string */

/* create the odl aggregate */

	aggNode1 = NewAggregate(aggNode1, KA_GROUP, aggName, "");
        if (aggNode1 == NULL)
        {
            errInserts[0] = aggName;
            /* error message is:
               "Unable to create odl aggregate <aggregate name>" */
 
            (void) PGS_MET_ErrorMsg(PGSMET_E_AGGREGATE_ERR, "PGS_MET_LoadAggregate",
                                    1, errInserts);
            return(PGSMET_E_AGGREGATE_ERR);
        }
	retVal = PGS_IO_Gen_Open((PGSt_PC_Logical) PGSd_MET_TEMP_ATTR_FILE,
                                        PGSd_IO_Gen_Read, &fileHandle, 1);
        if(retVal != PGS_S_SUCCESS)
        {
                sprintf(fileIdStr, "%d", PGSd_MET_TEMP_ATTR_FILE);
                errInserts[0] = "temporary";
                errInserts[1] = fileIdStr;
                /* error message is:
                "Unable to open <temporary> file with file id <aggregate name>" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, funcName,
                                    2, errInserts);
		aggNode1 = RemoveAggregate(aggNode1);
                return(PGSMET_E_OPEN_ERR);
        }
	odlRetVal = (PGSt_integer)ReadLabel(fileHandle, aggNode1);
	
        if(odlRetVal != 1)
        {
		errInserts[0] = aggName;
        	/* error message is:
                "Unable to load <agg name> information"*/
                errInserts[0] = aggName;

                (void) PGS_MET_ErrorMsg(PGSMET_E_LOAD_ERR,
                			funcName, 1, errInserts);
		(void) PGS_IO_Gen_Close(fileHandle);
		aggNode1 = RemoveAggregate(aggNode1);
                return(PGSMET_E_LOAD_ERR);
        }

/* need to set the class attribute */

	objectNode = aggNode1;
        do
        {
                objectNode = NextObject(objectNode);
                if(objectNode != NULL)
                {
			classParmNode = FindParameter(objectNode, PGSd_MET_CLASS_STR);
			if(classParmNode != NULL) /* object is a multiiple */
                                {
                                        classValueNode = FirstValue(classParmNode);
					if(classValueNode->item.type == TV_STRING ||
                                        classValueNode->item.type == TV_SYMBOL)
                                        {
						if(objectNode->objClass != NULL)
                                                {
                                                        free(objectNode->objClass);
                                                        objectNode->objClass = NULL;
                                                objectNode->objClass = (char *) malloc(strlen(classValueNode->item.value.string) + 1);
                                                }
						strcpy(objectNode->objClass, classValueNode->item.value.string);
					}
				}
		}
	}
	while(objectNode != NULL);
/* Delete  the temporary file */

	*aggNode = aggNode1;
	(void) PGS_IO_Gen_Close(fileHandle);

	return(PGS_S_SUCCESS);
}
