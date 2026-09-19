/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:
 
FILENAME:  
         PGS_MET_CheckAttr.c
 
DESCRIPTION:
         The file contains PGS_MET_CheckAttr.
         This function is used by PGS_MET_Write() to check that all the
         mandatory metadata is set prior to being written to the product file.
         Non-mandatory metadata parameters which are not set and all the spurious 
         information such as data location etc which are only used by the MET
         tools are also deleted
 
AUTHOR:
        Alward N.Siyyid / EOSL 
        Carol S. W. Tsai / Applied Reseach Corporation
        Abe Taaheri/Space Applications Corporation
HISTORY:
        18-May-1995     ANS     Initial version
        31-May-1995     ANS     Code inspection comments update
        20-July-1995    ANS     Fixed bug ECSed01021
        08-Apr-1997     CSWT    Added DAAC, DSS, DP, and TK for Datat_Location
                                checking. 
                                changed code for writing to the HDF file only those  
                                attributes with Data_Location set to MCF, PGE, TK, or
                                PCF. 
                                Changed code for generating a error message only when Data_Location
                                is equal to PGE and no value has been set. 
        20-Jun-1997     CSWT    Fixed code in order the class value in Group can be   
                                consistent with the class value of Object in the 
                                Metadata file. 
        23-Nov-1997     CSWT    Changed the source code back to the TK5.2 April version because the
                                TK5.2.1 version was designed to generate the error message for any 
                                metadata arameter specified in the MCF with a data location is equal 
                                to PGE and andatory is set to TRUE or FALSE has not been set vaule
                                by the PGE. But this will cause the cease of the execution for Ingest
                                and ASTER when they tried to create the metadata(This change is for bug
                                ECSed10099 about changing the toolkit status return to 'warning' rather
                                than 'error')
       09-Oct-1998     AT       changed:
                                classnodevalueforGroup->item.value.string = 
                                classnodevalueforObject->item.value.string;
                                to
                                *(classnodevalueforGroup->item.value.string) =
                                *(classnodevalueforObject->item.value.string);
                                Asigning the pointer of 
				classnodevalueforObject->item.value.string
				to
				classnodevalueforGroup->item.value.string
				causes problem in OLD's RemoveAggregate(), 
				where after freeing 
				classnodevalueforGroup->item.value.string
				other nodes cannot be freed, causing "freeing
				unallocated memory" problem and memory leaks.
				(Related to NCR18258).
      06-May-1999      AT       re-changed the line changed on 09-Oct-1998 to
                                use strcpy instead, and added a line to
				correct the number of digits in the group's
				parameter CLASS (i.e. item.length for paramter
				value of the CLASS parameter) This change is 
				for NCR ECSed22542.
END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/
 
#include <PGS_MET.h>
#include <PGS_TD.h>
#include <sys/stat.h>
#include <ctype.h>
/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>
 
/***************************************************************************
BEGIN_PROLOG:
 
        This is a private routine used by the toolkit functions and therefore
        not all the prolog fields are applicable to the routine and are marked
        as N/A
 
TITLE:
        Checks that mandatory metadata parameters are all set and strips off
        spuriuous information from the MCF ODL representation
 
NAME:
        PGS_MET_CheckAttr()
 
SYNOPSIS:
        N/A
 
DESCRIPTION:
        Checks that mandatory metadata parameters are all set and strips of
        spurious information from the MCF ODL representation. Spurious information
        is only deleted if mandatory criteria is satisfied.
 
INPUTS:
        Name            Description             Units   Min     Max
        ----            -----------             -----   ---     ---
        metaDataNode    ODL aggregate node      none    N/A     N/A
                        containing group
                        information
 
OUTPUTS:
        None
 
RETURNS:
        PGS_S_SUCCESS
        PGSMET_E_PARM_NOT_SET   Mandatory Parameter <parmName> not set in <group name>
        PGSMET_W_NO_MAND_PARM   No Mandatory Parameter found in  <core data name>
 
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
        FindParameter
        FirstValue
        RemoveParameter
        NextSubObject
 
END_PROLOG:
***************************************************************************/
PGSt_SMF_status
PGS_MET_CheckAttr(                          /* This function ensures that all
                                             * parameters are set before they are
                                             * written to product file
                                             */
              AGGREGATE groupNode)          /* odl handle for the metadata parameters */
{

	OBJECT                  tempNodeforObject = NULL;
	OBJECT                  parmNode = NULL;        /* parameter node */
	OBJECT                  deleteNode = NULL;
        OBJECT                  previousNode = NULL;    /* node that gets deleted */
        OBJECT                  nextNode = NULL;
	OBJECT                  dateObject = NULL;
	OBJECT                  timeObject = NULL;
	OBJECT                  childNode = NULL;
	GROUP			deleteGroup = NULL;
	GROUP                   previousGroup = NULL;
        GROUP                   nextGroup = NULL;
        PARAMETER               attrNode = NULL;        /* parameter attribute */
        PARAMETER               attrNodeforGroup = NULL;        /* parameter attribute */
        PARAMETER               attrNodeforObject = NULL;        /* parameter attribute */
        PARAMETER               valueParm = NULL;
        PARAMETER               locNode = NULL;
	PARAMETER               typeNode = NULL;
	PARAMETER               dateTimeValNode = NULL;
	PARAMETER               fgdcParameter = NULL;
	VALUE                   fgdcParameterValue = NULL;
	VALUE                   typeValue = NULL;
        VALUE                   locValue = NULL;
        VALUE                   attrValue = NULL;
        VALUE                   classnodevalueforGroup = NULL;
        VALUE                   classnodevalueforObject = NULL;
        VALUE_DATA              newValueData;
        PGSt_SMF_status         retVal = PGS_S_SUCCESS; /* SDPS toolkit ret value */
	PGSt_SMF_status         fgdcRetVal = PGS_S_SUCCESS;
        char *                  errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL};
                                                        /* Dynamic strings inserted in
                                                         * error messages
                                                         */
	char                    fgdcDateStr[30];
	char                    fgdcTimeStr[30];
	char                    reqDateStr[30];
	char                    reqTimeStr[30];
	char                    year[30];
	char                    month[30];
	char                    day[30];
	char                    hour[30];
	char                    minute[30];
	char                    second[30];
	char                    sec[30];
	PGSt_integer            index = 0;
	char                    utcInputStr[30];
	char                    newName[200];
	char                    timeName[200];
	char 	                valueStr[50];
	char *                  funcName = "PGS_MET_CheckAttr";
	PGSt_integer		odlretVal = 0;


/* search for the object and a mandatory parameter */
 
	parmNode = NextSubObject(groupNode, groupNode);
        NextGroup(groupNode);
        while(parmNode!=NULL)
        {

                valueParm = FindParameter(parmNode, PGSd_MET_ATTR_VALUE_STR);
                if(valueParm == NULL) /* i.e value is not defined */
                {
                        attrNode = FindParameter(parmNode, PGSd_MET_MANDATORY_STR);
			attrValue = FirstValue(attrNode);
			if(strcmp(attrValue->item.value.string, "FALSE") == 0)
			{
				deleteNode = parmNode;
			}
                        locNode = FindParameter(parmNode, PGSd_MET_DATA_LOC_STR);
                        locValue = FirstValue(locNode);
 
                        /* only those attributes with Data_Location set to MCF, PGE, TK or PCF will 
                         * be written to the HDF file 
                         */

                        if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_PGE) == 0)
                        {
                                strcpy(valueStr, "\"NOT SET\"");
                        }
                        else if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_MCF) == 0)
                        {                                
                                strcpy(valueStr, "\"NOT SUPPLIED\"");
                        }
                        else if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_TK) == 0)
                        {
                                strcpy(valueStr, "\"NOT OBTAINED\"");
                        }

                        /* those attributes with Data_Location set to DSS, DAAC, or DP will not
                         *  be written to the HDF 
                         */
        
                        else if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_DSS) == 0)
                        {
                                strcpy(valueStr, "\"NOT PROVIDED\"");
                                if ((strcmp(attrValue->item.value.string, "TRUE") == 0) &&
                                   (strcmp(locValue->item.value.string, PGSd_MET_SET_BY_DSS) == 0))
                                {
                                   deleteNode = parmNode;
                                } 
                        }

                        else if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_DP) == 0)
                        {                                
                                strcpy(valueStr, "\"NOT INCLUDED\""); 
                                if ((strcmp(attrValue->item.value.string, "TRUE") == 0) &&
                                   (strcmp(locValue->item.value.string, PGSd_MET_SET_BY_DP) == 0))
                                {
                                   deleteNode = parmNode;
                                }
                        }
                        else if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_DAAC) == 0)
                        {                                
                                strcpy(valueStr, "\"NOT SUPPORTED\""); 
                                if ((strcmp(attrValue->item.value.string, "TRUE") == 0) &&
                                   (strcmp(locValue->item.value.string, PGSd_MET_SET_BY_DAAC) == 0))
                                {
                                   deleteNode = parmNode;
                                }
                        }
                        else
                        {
                               strcpy(valueStr, "\"NOT FOUND\""); 
                        }

			/* skip container objects */

			childNode = NextSubObject(parmNode, parmNode);
			if(childNode == NULL)
			{
                        	odlretVal = ReadValue(parmNode, PGSd_MET_ATTR_VALUE_STR, valueStr);
                        	if(odlretVal != 1)
                        	{
                               		 /* this means that memory allocation has failed within ODL */
                                         /* error message is:
                                	"ODL routine failed to allocate memory" */
 
                                	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                             funcName, 0, errInserts);
                                 	return(PGSMET_E_ODL_MEM_ALLOC);
                        	}
                                /* a error message will be generated only when Data_Location is equal 
                                 * to PGE and no value has been set
                                 */
                         
                        	if((strcmp(attrValue->item.value.string, "TRUE") == 0) && 
                                   (strcmp(locValue->item.value.string, PGSd_MET_SET_BY_PGE) == 0))
                        	{
                                	retVal = PGSMET_E_MAND_NOT_SET;                        
				}
				if(retVal != PGSMET_E_MAND_NOT_SET)
                        	{
                                	retVal = PGSMET_W_META_NOT_SET;
                        	}
			}
                }

		/* create FGDC time aggregates */

		typeNode = FindParameter(parmNode, PGSd_MET_ATTR_TYPE_STR);
		typeValue = FirstValue(typeNode);
		dateTimeValNode = FindParameter(parmNode, PGSd_MET_ATTR_VALUE_STR);
		if(strcmp(parmNode->name, PGSd_MET_CDT) == 0)
		{
			strcpy(newName, PGSd_MET_CD);
			strcpy(timeName, PGSd_MET_CT);
		}
		else if(strcmp(parmNode->name, PGSd_MET_RBDT) == 0)
		{
			strcpy(newName, PGSd_MET_RBD);
                        strcpy(timeName, PGSd_MET_RBT);
		}
		else if(strcmp(parmNode->name, PGSd_MET_REDT) == 0)
                {
                        strcpy(newName, PGSd_MET_RED);
                        strcpy(timeName, PGSd_MET_RET);
                }
		if(typeValue != NULL && deleteNode == NULL)
		{
			if(strcmp(typeValue->item.value.string, PGSd_MET_DATETIME_STR) ==0 &&
				  (strcmp(parmNode->name, PGSd_MET_CDT) == 0 ||
				   strcmp(parmNode->name, PGSd_MET_RBDT) == 0 ||
                                   strcmp(parmNode->name, PGSd_MET_REDT) == 0))
				   
			{
				dateObject = NewAggregate(NULL, KA_OBJECT, newName, NULL);
				if(dateObject == NULL)
                        	{
                                	/* this means that memory allocation has failed within ODL */
                                	/* error message is:
                                	"ODL routine failed to allocate memory" */
 
                                	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                             funcName, 0, errInserts);
                                 	return(PGSMET_E_ODL_MEM_ALLOC);
                        	}
				else
				{
					/* copy NUM_VAL parameter */
					fgdcParameter = FindParameter(parmNode, PGSd_MET_ATTR_NUMOFVAL_STR);
					fgdcParameter = CopyParameter(fgdcParameter);
                                        if(fgdcParameter == NULL)
                                        {
                                                /* this means that memory allocation has failed within ODL */
                                                /* error message is:
                                                "ODL routine failed to allocate memory" */
 
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                funcName, 0, errInserts);
                                                return(PGSMET_E_ODL_MEM_ALLOC);
                                        }
					else
					{
						fgdcParameter = PasteParameter(dateObject, fgdcParameter);
					}
					/* copy the date time parameter and insert the FGDC date strings */
					fgdcParameter = CopyParameter(dateTimeValNode);
					if(fgdcParameter == NULL)
					{
						/* this means that memory allocation has failed within ODL */
						/* error message is:
                                        	"ODL routine failed to allocate memory" */
 
                                        	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                             	funcName, 0, errInserts);
                                        	return(PGSMET_E_ODL_MEM_ALLOC);
                                	}
                                        if(fgdcParameter == NULL)
                                        {
                                                /* this means that memory allocation has failed within ODL */
                                                /* error message is:
                                                "ODL routine failed to allocate memory" */
 
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                funcName, 0, errInserts);
                                                return(PGSMET_E_ODL_MEM_ALLOC);
                                        }
					fgdcParameterValue = FirstValue(dateTimeValNode);
					if(fgdcParameterValue->item.type == TV_STRING &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT SUPPLIED") != 0 &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT FOUND") != 0 &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT SET") != 0 &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT OBTAINED") != 0 &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT INCLUDED") != 0 &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT SUPPORTED") != 0 &&
					   strcmp(fgdcParameterValue->item.value.string, "NOT PROVIDED") != 0)
					{
						/* remove all the values first */
							
						fgdcParameter = RemoveParameter(fgdcParameter);
						fgdcParameter = NewParameter(NULL, dateTimeValNode->node_kind, dateTimeValNode->name);
						if(fgdcParameter == NULL)
                                        	{
                                                	/* this means that memory allocation has failed within ODL */
                                                	/* error message is:
                                                	"ODL routine failed to allocate memory" */
 
                                                	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                	funcName, 0, errInserts);
                                               		 return(PGSMET_E_ODL_MEM_ALLOC);
                                        	}
						while(fgdcParameterValue != NULL)
						{
							/* call the TD function */

							strcpy(utcInputStr, fgdcParameterValue->item.value.string);
							fgdcRetVal = PGS_TD_UTCtoFGDC(utcInputStr,
									 NULL, fgdcDateStr, fgdcTimeStr);
							if(fgdcRetVal != PGS_S_SUCCESS)
							{
								errInserts[0] = parmNode->name;
								/* error message is:
                                                		"Unable to convert UTC to FGDC format" */
 
                                                		(void) PGS_MET_ErrorMsg(PGSMET_E_FGDC_ERR,
                                                		funcName, 1, errInserts);
                                                		return(PGSMET_E_FGDC_ERR);
							}
							strcpy(year, fgdcDateStr);
							year[4] = '\0';
							strcpy(month, &fgdcDateStr[4]);
							month[2] = '\0';
							strcpy(day, &fgdcDateStr[6]);
							sprintf(reqDateStr, "%s-%s-%s", year, month, day);
						
							/* insert the string into the value strings */
	
							newValueData = ODLConvertString(reqDateStr, strlen(reqDateStr));
							if(newValueData.valid == 0)
                                                	{
                                                        	/* this means that memory allocation has failed within ODL */
                                                        	/* error message is:
                                                        	"ODL routine failed to allocate memory" */
 
                                                        	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                        	funcName, 0, errInserts);
                                                       		  return(PGSMET_E_ODL_MEM_ALLOC);
                                                	}
							NewValue(fgdcParameter, &newValueData);
							fgdcParameterValue = NextValue(fgdcParameterValue);
						}
					}
					fgdcParameter = PasteParameter(dateObject, fgdcParameter);
				}
                                timeObject = NewAggregate(NULL, KA_OBJECT, timeName, NULL);
                                if(timeObject == NULL)
                                {
                                        /* this means that memory allocation has failed within ODL */
                                        /* error message is:
                                        "ODL routine failed to allocate memory" */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                             funcName, 0, errInserts);
                                        return(PGSMET_E_ODL_MEM_ALLOC);
                                }
                                else
                                {
					/* copy NUM_VAL parameter */
                                        fgdcParameter = FindParameter(parmNode, PGSd_MET_ATTR_NUMOFVAL_STR);
                                        fgdcParameter = CopyParameter(fgdcParameter);
                                        if(fgdcParameter == NULL)
                                        {
                                                /* this means that memory allocation has failed within ODL */
                                                /* error message is:
                                                "ODL routine failed to allocate memory" */
 
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                funcName, 0, errInserts);
                                                return(PGSMET_E_ODL_MEM_ALLOC);
                                        }
                                        else
                                        {
                                                fgdcParameter = PasteParameter(timeObject, fgdcParameter);
                                        }
                                        /* copy the date time parameter and insert the FGDC time strings */
                                        fgdcParameter = CopyParameter(dateTimeValNode);
                                        if(fgdcParameter == NULL)
                                        {
                                                /* this means that memory allocation has failed within ODL */
                                                /* error message is:
                                                "ODL routine failed to allocate memory" */
 
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                funcName, 0, errInserts);
                                                return(PGSMET_E_ODL_MEM_ALLOC);
                                        }
					fgdcParameterValue = FirstValue(dateTimeValNode);
					if(fgdcParameterValue->item.type == TV_STRING &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT SUPPLIED") != 0 &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT FOUND") != 0 &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT SET") != 0 &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT OBTAINED") != 0 &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT INCLUDED") != 0 &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT SUPPORTED") != 0 &&
                                           strcmp(fgdcParameterValue->item.value.string, "NOT PROVIDED") != 0) 
					{
						/* remove all the values first */
							
						fgdcParameter = RemoveParameter(fgdcParameter);
						fgdcParameter = NewParameter(NULL, dateTimeValNode->node_kind, dateTimeValNode->name);
						if(fgdcParameter == NULL)
                                        	{
                                                	/* this means that memory allocation has failed within ODL */
                                                	/* error message is:
                                                	"ODL routine failed to allocate memory" */
 
                                                	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                	funcName, 0, errInserts);
                                               		 return(PGSMET_E_ODL_MEM_ALLOC);
                                        	}
						while(fgdcParameterValue != NULL)
						{
							/* call the TD function */

							strcpy(utcInputStr, fgdcParameterValue->item.value.string);
							fgdcRetVal = PGS_TD_UTCtoFGDC(utcInputStr,
									 NULL, fgdcDateStr, fgdcTimeStr);
							if(fgdcRetVal != PGS_S_SUCCESS)
							{
								errInserts[0] = parmNode->name;
								/* error message is:
                                                		"Unable to convert UTC to FGDC format" */
 
                                                		(void) PGS_MET_ErrorMsg(PGSMET_E_FGDC_ERR,
                                                		funcName, 1, errInserts);
                                                		return(PGSMET_E_FGDC_ERR);
							}
						
							strcpy(hour, fgdcTimeStr);
                                                        hour[2] = '\0';
                                                        strcpy(minute, &fgdcTimeStr[2]);
                                                        minute[2] = '\0';
                                                        strcpy(second, &fgdcTimeStr[4]);
							sec[0] = second[0];
							sec[1] = second[1];
							sec[2] = '.';
							index = 2;
							while(isdigit(second[index]) && second[index] != '\0')
							{
								sec[index + 1] = second[index];
								index++;
							}
							sec[index + 1] = '\0';
                                                        sprintf(reqTimeStr, "%s:%s:%s", hour, minute, sec);

							/* insert the string into the value strings */
	
							newValueData = ODLConvertString(reqTimeStr, strlen(reqTimeStr));
							if(newValueData.valid == 0)
                                                	{
                                                        	/* this means that memory allocation has failed within ODL */
                                                        	/* error message is:
                                                        	"ODL routine failed to allocate memory" */
 
                                                        	(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,
                                                        	funcName, 0, errInserts);
                                                       		  return(PGSMET_E_ODL_MEM_ALLOC);
                                                	}
							NewValue(fgdcParameter, &newValueData);
							fgdcParameterValue = NextValue(fgdcParameterValue);
						}
					}
					fgdcParameter = PasteParameter(timeObject, fgdcParameter);
                                }
				timeObject->right_sibling = parmNode->right_sibling;
				if(parmNode->right_sibling != NULL)
				{
					parmNode->right_sibling->left_sibling = timeObject;
				}
				parmNode->right_sibling = dateObject;
				dateObject->right_sibling = timeObject;
				dateObject->parent = ParentAggregate(parmNode);
				dateObject->left_sibling = parmNode;
				
				timeObject->left_sibling = dateObject;
				timeObject->parent = ParentAggregate(parmNode);
			}
		}	

		parmNode = NextSubObject(groupNode, parmNode);
		if(deleteNode != NULL && childNode == NULL)
		{
			deleteNode = RemoveAggregate(deleteNode);
			deleteNode = NULL;
		}
        }
 
        /* strip off all the unnecessary parameters */
        previousNode = NextSubObject(groupNode, groupNode);
        nextNode = NextSubObject(groupNode, previousNode);
        while(previousNode!=NULL)
        {
		/* remove empty containers */
		valueParm = FindParameter(previousNode, PGSd_MET_ATTR_VALUE_STR);
		childNode = NextSubObject(previousNode, previousNode);
		if(valueParm == NULL && childNode == NULL)
		{
			deleteNode = previousNode;
			previousNode = nextNode;
			nextNode = NextSubObject(groupNode, nextNode);
			deleteNode = RemoveAggregate(deleteNode);
		}
		else
		{
			attrNode = FindParameter(previousNode, PGSd_MET_MANDATORY_STR);
                	if(attrNode != NULL)
                	{
                		attrNode = RemoveParameter(attrNode);
                	}

                	attrNode = FindParameter(previousNode, PGSd_MET_DATA_LOC_STR);
                	if(attrNode != NULL)
                	{
                        	attrNode = RemoveParameter(attrNode);
                	}
                	attrNode = FindParameter(previousNode, PGSd_MET_ATTR_TYPE_STR);
                	if(attrNode != NULL)
                	{
                        	attrNode = RemoveParameter(attrNode);
                	}               
                	previousNode = nextNode;
                	nextNode = NextSubObject(groupNode, nextNode);
		}
        }
	/* remove any empty group nodes */
	previousGroup = NextSubGroup(groupNode, groupNode);
        nextGroup = NextSubGroup(groupNode, previousGroup);
        while(previousGroup!=NULL)
        {
                /* remove empty containers */
		deleteGroup = NextSubAggregate(previousGroup, previousGroup);
		if(deleteGroup == NULL)
		{
			deleteGroup = previousGroup;
			previousGroup = nextGroup;
                	nextGroup = NextSubGroup(groupNode, nextGroup);
			deleteGroup = RemoveAggregate(deleteGroup);
		}
		else
		{
			previousGroup = nextGroup;
                        attrNodeforGroup = FindParameter(previousGroup, PGSd_MET_CLASS_STR);                  
                        classnodevalueforGroup = FirstValue(attrNodeforGroup);
                        if(classnodevalueforGroup != NULL)  
                        {
                           tempNodeforObject = NextSubObject(previousGroup,previousGroup);
                           if (tempNodeforObject != NULL) 
                           {
                               attrNodeforObject =FindParameter(tempNodeforObject, PGSd_MET_CLASS_STR);
                               classnodevalueforObject = FirstValue(attrNodeforObject);
			       /*  *(classnodevalueforGroup->item.value.string)
				   =
				   *(classnodevalueforObject->item.value.string); */
			       /* changed the line above to the following lines
				  to write correct number of digits for class
				  field in a group. For NCR ECSed22542 */
			       free(classnodevalueforGroup->item.value.string);
			       classnodevalueforGroup->item.value.string = NULL;
			       classnodevalueforGroup->item.value.string = 
				 (char *) malloc(strlen(classnodevalueforObject->item.value.string) +  1);
			       strcpy(classnodevalueforGroup->item.value.string,
				      classnodevalueforObject->item.value.string);
			       classnodevalueforGroup->item.length =
				 strlen(classnodevalueforObject->item.value.string);
                           }
                        }
                        nextGroup = NextSubGroup(groupNode, nextGroup);
		}
        }
        return(retVal);
}

