/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	 PGS_MET_NameAndClass.c
 
DESCRIPTION:
         The file contains PGS_MET_NameAndClass.
	 This function simply parses the metadata attribute name
	 into name and class.

AUTHOR:
  	Alward N.Siyyid / EOSL
        Carol S. W. Tsai / Applied Reseach Corporation

HISTORY:
  	18-May-1995 	ANS 	Initial version
	31-May-1995     ANS     Code inspection comments update
        08-May-1997     CSWT    Fixed the coding to prevent the metadata
                                name, one input parameter that users 
                                provide as the attribute name to be set 
                                with the value, from being changed. 
        08-Sep-1997    CSWT     Changed  array size of chararcter string  "charStr" 
                                from 80 to PGSd_MET_GROUP_NAME_L in order to consist
                                with the calling function. 

END_FILE_PROLOG:
***************************************************************************/
/*----- includes ------*/

#include <PGS_MET.h>
#include <ctype.h>

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:  
  	Parses metadata name into name and class
 
NAME:
  	PGS_MET_NameAndClass()

SYNOPSIS:
  	N/A

DESCRIPTION:
	At times two metadata parameters can be called with a same name
	For such cases class is used to distinguish between the two parameters
	Metadata parameter name string with class is passed as name.class

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	nameStr		String containing	none	N/A	N/A
			name and class

OUTPUTS:
	name		attribute name		none	N/A     N/A
	class		attribute class		none    N/A     N/A

RETURNS:
	None

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
	None

END_PROLOG:
***************************************************************************/

void
PGS_MET_NameAndClass(			/* parses attribute string into name and class */
		char	      *nameStr, /* String containing name and class */
		char          name[],	/* attribute name */
		char	      attClass[])   /* attribute class */
{
	char                    *classPtr = NULL;
	char                    charStr[PGSd_MET_NAME_L];
        PGSt_integer            i;
	
        strcpy(charStr, nameStr);

        for(i=0; charStr[i]; i++)
        {
            if(islower(charStr[i])) charStr[i] = toupper(charStr[i]);
            else charStr[i] = charStr[i];
        }

	classPtr = strrchr(charStr, (int) PGSd_MET_NAME_CLASS_DIVIDER);
	if(classPtr != (char *)NULL)
	{	
		classPtr++;
		strcpy(attClass,classPtr);
		classPtr--;
		*classPtr = '\0';
		strcpy(name, charStr);
		*classPtr = PGSd_MET_NAME_CLASS_DIVIDER;
	}
	else
	{
		strcpy(name, charStr);
	}
	return;
}

