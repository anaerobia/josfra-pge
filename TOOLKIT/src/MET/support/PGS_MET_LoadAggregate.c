/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_LoadAggregate.c
 
DESCRIPTION:
         The file contains PGS_MET_LoadAggregate.
         This function is used by MET tools to Load ASCII files in ODL 
	 format into ODL Aggregates in memory

AUTHOR:
  	Alward N.Siyyid / EOSL
        Carol S. W. Tsai
        Abe Taaheri / Emergent Information Technologies, Inc.

HISTORY:
  	18-May-1995 	ANS 	Initial version
	31-May-1995	ANS	Code inspection comments update
        08-Apr-1997     CSWT    Added DAAC, DSS, DP, and TK for the checking
                                of Data Location.
        17-Apr-1997     CSWT    added code to handle those that only declaring 
                                CLASS = M for groups within container objects
                                can also generate CLASS = M in the subojects 
                                when writing metadata to the HDF file. 
        03-Jul-1997     CSWT    Change code to enable the user can pre-set
                                class value with any numerical number, such as 1, 
                                2 , 3, or 11 in MCF.

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
  	Loads ASCII files in ODL format into ODL aggregates (memory representation)
 
NAME:
  	PGS_MET_LoadAggregate()

SYNOPSIS:
  	N/A

DESCRIPTION:
	Loads ASCII files in ODL format into ODL aggregates (memory representation)

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	fileId		PC file id containing   none	variable variable
			attributes
	aggName		Aggregate name		none	none	none
	fileMode	PC file mode		none	defined defined 

OUTPUTS:
	aggHandle       meta data handle pointer

RETURNS:
	PGS_S_SUCCESS
	PGSMET_E_AGGREGATE_ERR	Unable to create odl aggregate 
	PGSMET_E_OPEN_ERR	Unable to open given file
	PGSMET_E_ODL_READ_ERR	Unable to create ODL tree 
	PGSMET_W_CLASS_TYPE	Illegal class type for <parameter name>
	
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
	MCF, User defined files

FUNCTIONS_CALLED:
	PGS_MET_ErrorMsg
	PGS_IO_Gen_Open
	PGS_IO_Gen_Temp_Open
	PGS_IO_Gen_Close
	ReadLable
	NewAggregate
	NextObject
	FindParameter
	FirstValue

END_PROLOG:
***************************************************************************/

#include <PGS_MET.h>

PGSt_SMF_status
PGS_MET_LoadAggregate(                  /* Parses a given file containing  
                                         * attribute data and loads the
                                         * data into memory as an ODL tree
                                         * structure
                                         */
             PGSt_PC_Logical fileId,    /* PC file id containing attributes */
             char            *aggName,  /* Aggregate name */
	     PGSt_integer    fileMode,  /* PC file mode */
	     AGGREGATE	     *aggHandle)  /* meta data handle pointer */
{
	PGSt_IO_Gen_FileHandle	*fileHandle = NULL;		/* file pointer to the file containing
							 * aggregate data in odl format
							 */
	AGGREGATE 		aggNode = NULL;		/* ODL tree node */
	AGGREGATE		objectNode = NULL; 	/* individual metadata aggregates */
	AGGREGATE               parentObject = NULL;
	AGGREGATE               childObject = NULL;
	AGGREGATE               baseNode = NULL;
	AGGREGATE               parentAgg = NULL;
	PARAMETER               parentAggParameter = NULL;
	PARAMETER		classParmNode = NULL;	/* node pointing to class parameter */
	PARAMETER               parmNode = NULL;
	VALUE			classValue = NULL;  /* vallue of class */
	VALUE			classValueNode = NULL;  /* vallue of class */
	VALUE                   valueNode = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer		odlRetVal = 0;  /* ODl function return value */
	PGSt_integer            numClassState = 0;
        PGSt_integer            numObjects = 0;
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char 			fileIdStr[PGSd_MET_FILE_ID_L]; /* file id value as string */
	char *                  funcName = "PGS_MET_LoadAggregate";

	/* create an aggregate */

	aggNode = NewAggregate(aggNode, KA_GROUP, aggName, "");
	if (aggNode == NULL)
	{
	    errInserts[0] = aggName;
	    /* error message is:
	       "Unable to create odl aggregate <aggregate name>" */

	    (void) PGS_MET_ErrorMsg(PGSMET_E_AGGREGATE_ERR, "PGS_MET_LoadAggregate",
				    1, errInserts);
	    return(PGSMET_E_AGGREGATE_ERR);
	}

	if(fileMode != PGSd_PC_TEMPORARY_FILE)
	{
	   retVal = PGS_IO_Gen_Open(fileId, PGSd_IO_Gen_Read, &fileHandle, 1);
	   if(retVal != PGS_S_SUCCESS)
	   {
		sprintf(fileIdStr, "%d", fileId);
	   	errInserts[0] = "input" ;
		errInserts[1] = fileIdStr;
		
            	/* error message is:
               	"Unable to open <input> file with file id <aggregate name>" */

               	(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_LoadAggregate",
                                    2, errInserts);
		aggNode = RemoveAggregate(aggNode);
               	return(PGSMET_E_OPEN_ERR);
	   }
	}
	else
	{
	   retVal = PGS_IO_Gen_Temp_Open(PGSd_IO_Gen_NoEndurance, fileId,
					PGSd_IO_Gen_Read, &fileHandle);
           if(retVal != PGS_S_SUCCESS)
           {
                sprintf(fileIdStr, "%d", fileId);
                errInserts[0] = "temporary";
	   	errInserts[1] = fileIdStr;
               	/* error message is:
               	"Unable to open <temporary> file with file id <aggregate name>" */

               	(void) PGS_MET_ErrorMsg(PGSMET_E_OPEN_ERR, "PGS_MET_LoadAggregate",
                                    2, errInserts);
		aggNode = RemoveAggregate(aggNode);
               	return(PGSMET_E_OPEN_ERR);
            }
        }

	odlRetVal = ReadLabel(fileHandle, aggNode);
	if (odlRetVal != 1)
	{
		sprintf(fileIdStr, "%d", fileId);
                errInserts[0] = aggName;
                errInserts[1] = fileIdStr;
                /* error message is:
                "Unable to create ODL tree <aggName> with file id <fileId>" */

                (void) PGS_MET_ErrorMsg(PGSMET_E_ODL_READ_ERR, "PGS_MET_LoadAggregate",
                                    2, errInserts);
		aggNode = RemoveAggregate(aggNode);
		if(fileHandle != NULL)
        	{
           		(void) PGS_IO_Gen_Close(fileHandle);
        	}
                return(PGSMET_E_ODL_READ_ERR);
	}
	
	/* close the file */

	if(fileHandle != NULL)
	{
	   (void) PGS_IO_Gen_Close(fileHandle);
	}
	/* set the class of each metadata if defined  and check each object description */
	objectNode = aggNode;
	do
	{
		objectNode = NextObject(objectNode);
		if(objectNode != NULL)
		{
				
			/* check for the data location and MANDATORY fields */
			/* determine if unit object i.e no more children
			   if unit object then test for NUNVAL and TYPE
				test parenting should have only one parent or NULL
				if parent and MULTIPLE, all (parent and child should have class defined)
				if just multiple should have enclosing group statement
				check that no other object is defiend with the same name
			   else
			   do nothing */

			parmNode = FindParameter(objectNode, PGSd_MET_MANDATORY_STR);
			if(parmNode == NULL)
			{
				errInserts[0] = objectNode->name;
                                        /* error message is:
					MANDATORY field is not defined for metadata <name>
                                        */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_MANDATORY_FIELD, "PGS_MET_LoadAggregate",
                                                       1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_MANDATORY_FIELD);
                         }
			 else
			 {
				valueNode = FirstValue(parmNode);
				if(valueNode->item.type != TV_STRING &&
                                   valueNode->item.type != TV_SYMBOL)
				{
					errInserts[0] = objectNode->name;
                                        /* error message is:
                                        MANDATORY field type is not correct for metadata <name>. It should be a STRING
                                        */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_MANDATORY_DEF, "PGS_MET_LoadAggregate",
                                                       1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_MANDATORY_DEF);
				}
				if(strcmp(valueNode->item.value.string, "TRUE") != 0 &&
				   strcmp(valueNode->item.value.string, "FALSE")!= 0)
				{
					errInserts[0] = objectNode->name;
                                        /* error message is:   
                                        MANDATORY field value is not correct for metadata <name>. It should be a TRUE or FALSE
                                        */     
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_MANDATORY_VALUE, "PGS_MET_LoadAggregate", 
                                                       1, errInserts); 
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_MANDATORY_VALUE);   
                                } 
			}	
			
			parmNode = FindParameter(objectNode, PGSd_MET_DATA_LOC_STR);
                        if(parmNode == NULL)
                        {
                                errInserts[0] = objectNode->name;
                                        /* error message is:
                                        Data location field is not defined for metadata <name>
                                        */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_LOC_FIELD, "PGS_MET_LoadAggregate",
                                                       1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_LOC_FIELD);
                         }
                         else
                         {
                                valueNode = FirstValue(parmNode);
                                if(valueNode->item.type != TV_STRING &&
                                   valueNode->item.type != TV_SYMBOL)
                                {
                                        errInserts[0] = objectNode->name;
                                        /* error message is:
                                        DATA_LOCATION field type is not correct for metadata <name>. It should be a STRING
                                        */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_LOC_DEF, "PGS_MET_LoadAggregate",
                                                       1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_LOC_DEF);
                                }
                                if(strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_PCF) != 0 &&
                                   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_MCF)!= 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_PGE)!= 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_DSS)!= 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_DP)!= 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_TK)!= 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_DAAC)!= 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_SET_BY_NONE) != 0)
                                {
                                        errInserts[0] = objectNode->name;
                                        /* error message is:
                                        DATA-LOCATION field value is not correct for metadata <name>. It should be MCF, PGE, PCF or NONE
                                        */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_LOC_VALUE, "PGS_MET_LoadAggregate",
                                                       1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_LOC_VALUE);
                                }
                        }
			/* check that its a unit object. A unit object has no children */
                        childObject = NextSubObject(objectNode, objectNode);
                        if(childObject == NULL) /* it is a unit object */
                        {
                                /* check for the type field */
                                parmNode = FindParameter(objectNode, PGSd_MET_ATTR_TYPE_STR);
                                if(parmNode == NULL)
                                {
                                        errInserts[0] = PGSd_MET_ATTR_TYPE_STR;
                                        errInserts[1] = objectNode->name;
 
                                        /* error message is:
                                        "Unable to obtain \
                                         <attr> of metadata <parameter>" */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_NO_DEFINITION,
                                        funcName, 2, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_NO_DEFINITION);
                                }
                                valueNode = FirstValue(parmNode);
                                if(valueNode->item.type != TV_STRING && valueNode->item.type != TV_SYMBOL)
                                {
                                        errInserts[0] = objectNode->name;
 
                                        /* error message is:
                                        "Illegal type definition for metadata <attrName>. It should be a string" */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_ILLEGAL_TYPE,
                                        funcName, 1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_ILLEGAL_TYPE);
                                }
				if(strcmp(valueNode->item.value.string, PGSd_MET_INTEGER_STR) != 0 &&
                                   strcmp(valueNode->item.value.string, PGSd_MET_UINTEGER_STR)!= 0 &&
                                   strcmp(valueNode->item.value.string, PGSd_MET_FLOAT_STR)!= 0 &&
                                   strcmp(valueNode->item.value.string, PGSd_MET_STRING_STR) != 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_DOUBLE_STR) != 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_DATE_STR) != 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_TIME_STR) != 0 &&
				   strcmp(valueNode->item.value.string, PGSd_MET_DATETIME_STR) != 0)
                                {
                                        errInserts[0] = objectNode->name;
                                        /* error message is:
					Illegal value for the TYPE of metadata <name>. See user guide for valid values */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_INV_DATATYPE, "PGS_MET_LoadAggregate",
                                                       1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_INV_DATATYPE);
                                }
				/* check for the numval field */
                                parmNode = FindParameter(objectNode, PGSd_MET_ATTR_NUMOFVAL_STR);
                                if(parmNode == NULL)
                                {
                                        errInserts[0] = PGSd_MET_ATTR_NUMOFVAL_STR;
                                        errInserts[1] = objectNode->name;
 
                                        /* error message is:
                                        "Unable to obtain \
                                         <attr> of metadata <parameter>" */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_NO_DEFINITION,
                                        funcName, 2, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_NO_DEFINITION);
                                }
                                valueNode = FirstValue(parmNode);
                                if(valueNode->item.type != TV_INTEGER)
                                {
                                        errInserts[0] = objectNode->name;
 
                                        /* error message is:
                                        "Illegal NUMVAL definition for metadata <attrName>. It should be an integer " */
 
                                        (void) PGS_MET_ErrorMsg(PGSMET_E_ILLEGAL_NUMVAL,
                                        funcName, 1, errInserts);
					aggNode = RemoveAggregate(aggNode);
                                        return(PGSMET_E_ILLEGAL_NUMVAL);
                                }
				else
				{
					if(valueNode->item.value.integer.number <1)
					{
						errInserts[0] = objectNode->name;
 
                                	        /* error message is:
                                       		 "Illegal NUMVAL value for metadata <attrName>. It should be greater than or equal to 1" */
 
                                        	(void) PGS_MET_ErrorMsg(PGSMET_E_INV_NUMVAL,
                                        	funcName, 1, errInserts);
						aggNode = RemoveAggregate(aggNode);
                                        	return(PGSMET_E_ILLEGAL_NUMVAL);
					}
				}
				
				/* check for the parent object */
				parentObject = ParentObject(ParentObject(objectNode));
				if(parentObject != NULL) /* illegal three levels of hierarchy in Objects */
				{
					errInserts[0] = objectNode->name;
					/* more than one level in the object hierarchy is being used which is not allowed */
                                                /* error message is:
						Metadata objects can only be enclosed by one level of Container objects.
						The offending object is <name> */ 
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_CONTAINER_LEVELS,
                                                funcName, 1, errInserts);
						aggNode = RemoveAggregate(aggNode);
                                                return(PGSMET_E_CONTAINER_LEVELS);
                                 }
                        }
			else
			{
				/* object could be a container object which should not have a parent object */
				parentObject = ParentObject(objectNode);
                                if(parentObject != NULL) /* illegal three levels of hierarchy in Objects */
                                {
                                        errInserts[0] = objectNode->name;
                                        /* more than one level in the object hierarchy is being used which is not allowed */
                                                /* error message is:
                                                Metadata objects can only be enclosed by one level of Container objects.
                                                The offending object is <name> */
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_CONTAINER_LEVELS,
                                                funcName, 1, errInserts);
						aggNode = RemoveAggregate(aggNode);
                                                return(PGSMET_E_CONTAINER_LEVELS);
                                }
			}
			/* check both the object and its parent and siblings for the class definition */
			parentObject = ParentObject(objectNode);
			if(parentObject == NULL)
			{
				parentObject = objectNode;
			}
			baseNode = parentObject;
			numClassState = 0;
			numObjects = 0;
			do
			{
				classParmNode = FindParameter(parentObject, PGSd_MET_CLASS_STR);

				if(numObjects > 0) /* now dealing with sub objects of container */
				{
					if(classParmNode == NULL) /* does not contain the class statement */
								  /* so we are going to put one there */
					{
						odlRetVal = ReadValue(parentObject, PGSd_MET_CLASS_STR, PGSd_MET_MULTIPLE_FLAG);
						if (odlRetVal != 1)
        					{
							/* this means that memory allocation has failed within ODL */
                                        		/* error message is:
                                        		"ODL routine failed to allocate memory" */
 
                                        		(void) PGS_MET_ErrorMsg(PGSMET_E_ODL_MEM_ALLOC,funcName, 0, errInserts);
							aggNode = RemoveAggregate(aggNode);
                                        		return(PGSMET_E_ODL_MEM_ALLOC);
        					}
					}
				}
				classParmNode = FindParameter(parentObject, PGSd_MET_CLASS_STR);
                                classValue = FirstValue(classParmNode);

				if(classParmNode  != NULL || classValue != NULL) /* object is a multiiple */
				{
					classValueNode = FirstValue(classParmNode);
					if(classValueNode->item.type == TV_STRING ||
				   	classValueNode->item.type == TV_SYMBOL)
					{
						if(parentObject->objClass != NULL)
						{
							free(parentObject->objClass);
							parentObject->objClass = NULL;
						parentObject->objClass = (char *) malloc(strlen(classValueNode->item.value.string) + 1);
						}
						strcpy(parentObject->objClass, classValueNode->item.value.string);
				
				   		/* parentObject->objClass = classValueNode->item.value.string; */
				   		if(strcmp(parentObject->objClass, classValue->item.value.string) != 0)
				   		{
							errInserts[0] = parentObject->name;
							errInserts[1] = "MULTIPLE";
                                   			/* error message is:
                                   			"Illegal class definition for metadata <metedata name>. It should
					 		always be PGSd_MET_MULTIPLE_FLAG
                                    			*/
 
                                   			(void) PGS_MET_ErrorMsg(PGSMET_E_CLASS_DEF, "PGS_MET_LoadAggregate",
                                                       	2, errInserts);
							aggNode = RemoveAggregate(aggNode);
                                   			return(PGSMET_E_CLASS_DEF);
				   		}
				   
				   		/* odlRetVal = CommentAggregate(parentObject, classValueNode->item.value.string); */
						   parentObject->appl1 = PGSd_MET_MULTI_FLAG;
					}
					else
					{
                                   		errInserts[0] = parentObject->name;
                                   		/* error message is:
                                   		"Illegal class type for metadata <name>.
                                    		*/

                                   		(void) PGS_MET_ErrorMsg(PGSMET_E_CLASS_TYPE, "PGS_MET_LoadAggregate",
                                                       		1, errInserts);
						aggNode = RemoveAggregate(aggNode);
                                   		return(PGSMET_E_CLASS_TYPE);
					}
					/* multiple objects or the correspoonding container objects must be enclosed by a group 
					   statement */
					parentAgg = ParentAggregate(parentObject);
					/* perform this test only for single metadata or container objects */
					if(numObjects == 0)
					{
                                		if(parentAgg->kind != KA_GROUP)
                                		{
                                        		/* error message is:
                                        		"Multiple objects must have enclosing groups around them" */
 
                                        		(void) PGS_MET_ErrorMsg(PGSMET_E_PARENT_GROUP,
                                        		funcName, 0, errInserts);
							aggNode = RemoveAggregate(aggNode);
                                        		return(PGSMET_E_PARENT_GROUP);
                                		}
                                		else
                                		{
                                        		parentAggParameter = FindParameter(parentAgg, PGSd_MET_GROUP_TYPE_STR);
                                        		if(parentAggParameter != NULL)
                                        		{
                                               		 	/* error message is:
                                                		"Multiple objects must have enclosing groups around them" */
 
                                                		(void) PGS_MET_ErrorMsg(PGSMET_E_PARENT_GROUP,
                                                		funcName, 0, errInserts);
								aggNode = RemoveAggregate(aggNode);
                                                		return(PGSMET_E_PARENT_GROUP);
                                        		}
						}
                                	}
					numClassState++;
				}
				numObjects++; 
				parentObject = NextSubObject(baseNode, parentObject);
			}
			while(parentObject != NULL);
			if(numClassState != 0) /* there are some class statemenets */
			{
				if(numObjects != numClassState) /* theyshould be equal */
                                {
                                        errInserts[0] = objectNode->name;
                                                /* error message is:
                                                Class statements should be defined for all the sister objects as well as the 
						container objects.The offending object is <name> */
                                                (void) PGS_MET_ErrorMsg(PGSMET_E_CLASS_STATEMENTS,
                                                funcName, 1, errInserts);
						aggNode = RemoveAggregate(aggNode);
                                                return(PGSMET_E_CLASS_STATEMENTS);
                                }	
			}	
		}
	}
	while(objectNode != NULL);
	*aggHandle = aggNode;

	return(PGS_S_SUCCESS);
}

