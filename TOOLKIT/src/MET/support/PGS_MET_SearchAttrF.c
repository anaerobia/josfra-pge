/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_SearchAttrF.c
 
DESCRIPTION:
        The file contains PGS_MET_SearchAttrF.
        This functiont is used by PGS_MET_GetSetAttr() to provide the more
        efficient capability for users can locate attributes within a long
        listing of Product Specific Metadata.
	
AUTHOR: 
        Carol S. W. Tsai / Applied Reseach Corporation
	Abe Taaheri/SM&A corporation(east)

HISTORY:
        09-Apr-1997    CSWT   Initial version 
        17-Jun-1997    CSWT   Fixed code to handle case of a empty Group.
        26-NOV-1997    CSWT   Changed the size of the array elements from 80 to be
                              PGSd_MET_NAME_L(100) for the input parameter attrName, a
                              character string that defined to hold the attribute name
                              string, in this function PGS_MET_SearchAttrF() in order to
                              make consistent with the input parameter defined in the
                              calling function PGS_MET_GetSetAttrF()(This change is for
                              bug ECSed10160 about bug in PGS_MET_SearchAttr)
        23-June-1999   AT     Used strcpy for copying onto valueString,ValueString, and 
	                      tempSearchString (instead of equating pointers) so that
	                      toupper function not to change
			      mdValueNode->item.value.string strings
			      to capital letters. This is for NCR ECSed23234.


END_FILE_PROLOG
*******************************************************************************/

/* include files */

#include "PGS_MET.h"
#include <ctype.h>
#include <errno.h>

/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Searches the attribute within the Product Specific Metadat
  
NAME:  
        PGS_MET_SearchAttrF()

SYNOPSIS:
        N/A

DESCRIPTION:
        Searches the attribute within a long listing of Product Specific Metadat

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
        mdGroupNode     list of groups within 	none		N/A          N/A
                        MCF

        attrName	name.class of parameter none            N/A          N/A

OUTPUTS:
	attrValue       value of attribute to	none            N/A          N/A
			be passed back to the
			user

RETURNS:   
   	PGS_S_SUCCESS			
        PGSMET_E_SEARCH_FAILED     The requested parameter could not be found after
                                   searching the file listing

EXAMPLES:
        N/A

NOTES:
        N/A

REQUIREMENTS:
        N/A

DETAILS:
        N/A	
	
GLOBALS:
        N/A	

FILES:
	NONE
FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg
	PGS_MET_ErrorMsg
        NextSubObject
        NextGroup
        NextValue
	FindGroup
	FindObject
	FindParameter
	FirstValue
	
	
END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_MET_SearchAttrF(

			   
    PGSt_MET_handle    mdHandle,
    AGGREGATE  	       mdGroupNode,      /* list of groups within MCF */
    char 	       attrName[PGSd_MET_NAME_L],     /* Parameter attr to be retrieved */
    void               *attrValue,
    unsigned int       strLength)         /* string length if the void buffer is character */ 
{
    AGGREGATE		previousGroup = NULL;	/* parameter requeted */
    AGGREGATE		nextGroup = NULL;	/* parameter requeted */
    AGGREGATE		foundNode = NULL;	/* parameter requeted */
    PARAMETER		mdParmNode = NULL;	/* parameter attribute requested */
    PARAMETER		foundParmNode = NULL;	/* parameter attribute requested */
    PARAMETER           maxValparm = NULL;
    PARAMETER           attrNode = NULL;  
    PARAMETER           pattrNode = NULL;  
    PARAMETER           valueParm = NULL;
    VALUE	        classnodevalue = NULL;	/* value of the attribute */
    VALUE	        mdValueNode = NULL;	/* value of the attribute */
    VALUE               maxValNode = NULL;
    OBJECT              previousNode = NULL;    /* node that gets deleted */
    OBJECT              nextNode = NULL;
    OBJECT              childNode = NULL;

    PGSt_integer        maxVal = 0;
    PGSt_integer	SearchvalCount = 0;
    PGSt_integer        charcterCount = 0;
    PGSt_integer        secLoopCount = 0;

    char *		errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
			/* Dynamic strings inserted in 
			 * error messages
			 */
    char *		funcName = "PGS_MET_SearchAttrF";
    char *              blankPtr = NULL;
    char *              valueString = NULL;
    char *              ValueString = NULL;
    char *		SaveString;	/* pointer used to place values in void buffer */
    char  		tempSearchString[PGSd_MET_MAX_STRING_SET_L];	/*
									  pointer used 
									  for searching values in void buffer */
    char *  		classstr  = NULL; /* attribute class */
    char *  		tempvalueclass = NULL; /* attribute class */
    PGSt_integer *	outInteger;	/* pointer used to place values in void buffer */
    PGSt_double *	outDouble;	/* pointer used to place values in void buffer */
    char 		SearchnewLine[3] = "";
    char                SearchnewLineConst[5] = "";
    PGSt_integer        i;
	
    PGSt_boolean        SearchFound = PGS_FALSE;  
    PGSt_boolean        forGetPCAttr = PGS_FALSE;
    PGSt_boolean        foundlocation = PGS_FALSE;

                                                            

    /* clear the errno for the ODL routines */
    if (errno == ERANGE)
    {
        errno = 0;
    }
    valueString = (char *) malloc(PGSd_MET_MAX_STRING_SET_L);
    ValueString = (char *) malloc(PGSd_MET_MAX_STRING_SET_L);
    SaveString = (char *) attrValue;
    outInteger = (PGSt_integer *) attrValue;
    outDouble = (PGSt_double *) attrValue;

    previousGroup = NextGroup ( mdGroupNode);
    nextGroup = NextGroup ( previousGroup);

    /* if group is empty, an error message PGSMET_E_SEARCH_FAILED will be returned */
     
    if (previousGroup ==NULL || nextGroup ==NULL)
    {
 	free(valueString);
 	free(ValueString);
        return(PGSMET_E_SEARCH_FAILED);
    }
    else
    {
	while (previousGroup != NULL)
	{
	    mdParmNode = FindParameter(previousGroup, attrName);
 
	    if(mdParmNode == NULL)
	    {
		/* check wheter the calling function is the function of PGS_MET_GetPCAttr or not */
		if (strcmp(mdHandle,"locAggr#0") == 0)
		{
		    forGetPCAttr = PGS_TRUE;
		}
		else
		{
		    forGetPCAttr = PGS_FALSE;
		}
 
		/* search the attribute name throuth the whole metadata file if the
		 * calling function is PGS_MET_GetPCAttr
		 */
		if (forGetPCAttr == PGS_TRUE)
		{
		    previousNode = NextSubObject(previousGroup,previousGroup);
		    nextNode = NextSubObject(previousGroup,previousNode);

		    /* search the attribute name untill the search Flag is become True */
		    while(previousNode!=NULL && SearchFound == PGS_FALSE)
		    {
			/* skip all the objects that is not belong to the container objects */
			valueParm = FindParameter(previousNode, PGSd_MET_ATTR_VALUE_STR);
			childNode = NextSubObject(previousNode, previousNode);
                
			if(valueParm == NULL && childNode == NULL)
			{
			    previousNode = nextNode;
			    nextNode = NextSubObject(previousGroup, nextNode);
			}
			else
			{
			    pattrNode = FindParameter(previousNode, PGSd_MET_CLASS_STR);
			    if(pattrNode != NULL)
			    {
				previousNode = nextNode;
				attrNode = FindParameter(previousNode, PGSd_MET_ATTR_VALUE_STR);
				if(attrNode != NULL)
				{
				    SearchvalCount = 1;
				    maxVal = 0;
				    classnodevalue = FirstValue(pattrNode);
				    classstr = classnodevalue->item.value.string; 
				    foundNode = FindObject(previousGroup, previousNode->name, classstr);
				    if(foundNode != NULL)
				    {
					foundParmNode = FindParameter(foundNode,PGSd_MET_ATTR_VALUE_STR);
				    }

                                /* get the number of values for this particular metadata */ 
				    maxValparm = FindParameter(foundNode, PGSd_MET_ATTR_NUMOFVAL_STR);
				    if(maxValparm == NULL)
				    {
					mdValueNode = FirstValue(foundParmNode);
					while(mdValueNode != NULL)
					{
					    maxVal++;
					    mdValueNode = NextValue(mdValueNode);
					}
				    }
				    else
				    {
					maxValNode = FirstValue(maxValparm);
					if(maxValNode == NULL)
					{
					    errInserts[0] = PGSd_MET_ATTR_NUMOFVAL_STR;
					    errInserts[1] = attrName;
 
				/* error message is:
				   "Unable to obtain \
				   <attr> of metadata <parameter>" */
 
					    (void) PGS_MET_ErrorMsg(PGSMET_E_NO_DEFINITION,funcName, 2, errInserts);
					    free(valueString);
					    free(ValueString);
					    return(PGSMET_E_NO_DEFINITION);
					}
					else
					{
					    maxVal = maxValNode->item.value.integer.number;
					}
				    }

				    mdValueNode = FirstValue(foundParmNode);
               
                                /* fill the buffer with the values from the searched parameter node
                                 *  according to the type of data
                                 */
				    while(mdValueNode != NULL && SearchvalCount <= maxVal)
				    {
					if(mdValueNode->item.type == TV_STRING || mdValueNode->item.type == TV_SYMBOL)
					{
				/* now fill with the value
				   string untill end of string
				   character or the input str
				   length is exhausted */
					    charcterCount =0;
				/* sometimes ODL adds the newline character at front */

					    SearchnewLine[0] = (mdValueNode->item.value.string)[0];
					    SearchnewLine[1] = (mdValueNode->item.value.string)[1];
					    SearchnewLine[2] = '\0';
					    if(strcmp(SearchnewLine, SearchnewLineConst) == 0)
					    {
						strcpy(valueString, mdValueNode->item.value.string + 2);
						strcpy(tempSearchString, valueString);
 
 
						for (i=0;tempSearchString[i];i++)
						{
						    tempSearchString[i] = toupper(tempSearchString[i]);
						}

				/* set up the Flag to be PGS_TRUE if the attribute name been
				 * found in the metadata file
				 */
						if(strcmp(tempSearchString,attrName) == 0)
						{
						    SearchFound = PGS_TRUE;
						    tempvalueclass = classnodevalue->item.value.string;
						}
						else
						{
						    SearchFound = PGS_FALSE;
						}
					    }
					    else
					    {
						strcpy(valueString, mdValueNode->item.value.string);
						strcpy(tempSearchString, valueString);
 
						for (i=0;tempSearchString[i];i++)
						{
						    tempSearchString[i] = toupper(tempSearchString[i]);
						}

				/* set up the Flag to be PGS_TRUE if the attribute name been
				 * found in the metadata file
				 */
						if(strcmp(tempSearchString,attrName) == 0)
						{
						    SearchFound = PGS_TRUE;
						    tempvalueclass = classnodevalue->item.value.string;
						}
						else
						{
						    SearchFound = PGS_FALSE;
						}

					    }
					    if(SearchvalCount != maxVal)
					    {
						*(valueString) = '\0';
					    }
					}

					mdValueNode = NextValue(mdValueNode);
					SearchvalCount++;
				    } 
 
				    previousNode = nextNode;
				    nextNode = NextSubObject(previousGroup, nextNode);
 
				} 
				else
				{
				    previousNode = nextNode;
				    nextNode = NextSubObject(previousGroup, nextNode);
				}
			    } 
			    else
			    {
				previousNode = nextNode;
				nextNode = NextSubObject(previousGroup, nextNode);
			    } 
			} 
		    } 

		    /* get the value/values of the serched attribute name if the search Flag
		     * is True
		     */
		    if(SearchFound == PGS_TRUE)
		    {
			previousNode = nextNode;
			nextNode = NextSubObject(previousGroup,previousNode);

			{
			    while(previousNode != NULL && foundlocation != PGS_TRUE)
			    {
				pattrNode = FindParameter(previousNode, PGSd_MET_CLASS_STR);
				if(pattrNode != NULL)
				{
				    attrNode = FindParameter(previousNode, PGSd_MET_ATTR_VALUE_STR);
				    if(attrNode != NULL)
				    {
 
					classnodevalue = FirstValue(pattrNode);
					classstr = classnodevalue->item.value.string;
					if (strcmp(classstr,tempvalueclass) == 0) 
					{
					    foundlocation = PGS_TRUE;
					    foundNode = FindObject(previousGroup, previousNode->name, classstr);
					    if(foundNode != NULL)
					    {
						foundParmNode = FindParameter(foundNode,PGSd_MET_ATTR_VALUE_STR);
						maxValparm = FindParameter(foundNode, PGSd_MET_ATTR_NUMOFVAL_STR);
						if(maxValparm == NULL)
						{
						    mdValueNode = FirstValue(foundParmNode);
						    while(mdValueNode != NULL)
						    {
							maxVal++;
							mdValueNode = NextValue(mdValueNode);
						    }
						}
						else
						{
						    maxValNode = FirstValue(maxValparm);
						    if(maxValNode == NULL)
						    {
							errInserts[0] = PGSd_MET_ATTR_NUMOFVAL_STR;
							errInserts[1] = attrName;
 
				/* error message is:
				   "Unable to obtain \
				   <attr> of metadata <parameter>" */
 
							(void) PGS_MET_ErrorMsg(PGSMET_E_NO_DEFINITION,
										funcName, 2, errInserts);
							free(valueString);
 							free(ValueString);
							return(PGSMET_E_NO_DEFINITION);
						    }
						    else
 
						    {
							maxVal = maxValNode->item.value.integer.number;
						    }
						}

						SearchvalCount = 1; 
						mdValueNode = FirstValue(foundParmNode);
				/* fill the buffer with values from the serched attribute name
				 * according to the type of data
				 */
						while(mdValueNode != NULL && SearchvalCount <= maxVal)
						{
						    if(mdValueNode->item.type == TV_STRING || mdValueNode->item.type == TV_SYMBOL)
						    {
				/* fill the array with blanks first */
							blankPtr = SaveString; /* point to the first location of individual string */
							for(secLoopCount =0; secLoopCount < (strLength); secLoopCount++)
							{
							    *blankPtr = ' ';
							    blankPtr++;
							}
				/* now fill with the value string untill end of string character or the input str length is exhausted */
							charcterCount =0;
				/* sometimes ODL adds the newline character at front */

							SearchnewLine[0] = (mdValueNode->item.value.string)[0];
							SearchnewLine[1] = (mdValueNode->item.value.string)[1];
							SearchnewLine[2] = '\0';
							if(strcmp(SearchnewLine, SearchnewLineConst) == 0)
							{
							    strcpy(ValueString, mdValueNode->item.value.string + 2);
 
							}
							else
							{
							    strcpy(ValueString, mdValueNode->item.value.string);
 
							}
							while(charcterCount != (strLength) &&
							      ValueString[charcterCount] != '\0')
							{
							    SaveString[charcterCount] = ValueString[charcterCount];
							    charcterCount++;
							}
							SaveString = SaveString + strLength; /* point to the next array string */
							if(SearchvalCount != maxVal)
							{
							    blankPtr = SaveString; /* point to the first location of individual string */
							    for(secLoopCount =0; secLoopCount < (strLength); secLoopCount++)
							    {
								*blankPtr = ' ';
								blankPtr++;
							    }
							    *SaveString = '$';
							}

						    }
						    else if(mdValueNode->item.type == TV_INTEGER)
						    {
							*outInteger = (PGSt_integer) mdValueNode->item.value.integer.number;
							outInteger++;
 
							{
							    *outInteger = INT_MAX;
							}
						    }
						    else if(mdValueNode->item.type == TV_REAL)
						    {
							*outDouble = (PGSt_double) mdValueNode->item.value.real.number;
							outDouble++;
							if(SearchvalCount != maxVal)
							{
							    *outDouble = DBL_MAX;
							}
						    }
						    mdValueNode = NextValue(mdValueNode);
						    SearchvalCount++;
						} 
						free(valueString);
						free(ValueString);
						return (PGS_S_SUCCESS);
					    }
					    else 
					    {
						foundlocation = PGS_FALSE;
						previousNode = nextNode;
						nextNode = NextSubObject(previousGroup, nextNode);
					    }
					}
				    }
				}
			    }
			}
		    }
		}

		/* return back the error message if the attribute name can not be found or
		 * the calling function is not the function of PGS_MET_GetPCAttr
		 */
		if (SearchFound == PGS_FALSE || forGetPCAttr == PGS_FALSE)
		{
		    free(valueString);
		    free(ValueString);
		    return(PGSMET_E_SEARCH_FAILED);
		}
	    }
	}
	previousGroup=nextGroup;
	nextGroup=NextGroup(nextGroup);
    }
    if(valueString != NULL)
    {
	free(valueString);
	valueString = NULL;
    }
    if(ValueString != NULL)
    {
	free(ValueString);
	ValueString = NULL;
    }
    return(PGS_S_SUCCESS);
}

