/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_GetConfigData
 
DESCRIPTION:
	Enables the user to get the values of Configuration data parameters 
	held in the PC table
        
	
AUTHOR: 
        Alward N. Siyyid/ EOSL

HISTORY:
        18-MAY-95      ANS     Initial version
	01-JUN-95      ANS     Code inspection updates
	13-July-95     ANS     Improved Fortran Example
	07-Jul-99      RM      updated for TSF functionality

END_FILE_PROLOG
*******************************************************************************/

/* include files */

#include "PGS_MET.h"
#include "PGS_TSF.h"
#include <errno.h>

/* odl include files */

#include <CUC/odldef.h>
#include <CUC/odlinter.h>

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
        Accesses the configuration parameter data in the PC table
  
NAME:  
        PGS_MET_GetConfigDataF()

SYNOPSIS:
C:
  	N/A , Only to be used through fortran interface. See PGS_MET_GetConfigData() for more details
 
F:
	N/A
DESCRIPTION:
	Certain configuration parameters are held in the PC table as follows
		10220|REMOTEHOST|sandcrab
	This tool would retrieve the value "sandcrab" from the PC table given the
	name of the parameter "REMOTEHOST". The parameter id 10220 is not used here.
	The value string (eg. sandcrab) is assumed to be in odl format and therefore 
	different types are supported. 

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
        attrName	name of parameter	none            N/A 	     N/A

OUTPUTS:
	attrValue       value of attribute to   none            N/A          N/A
			be passed back to the 
			user

RETURNS:   
   	PGS_S_SUCCESS			
	PGSMET_E_AGGREGATE_ERR		"Unable to create odl aggregate <aggregate name>"
					This should never occur unless the process runs out of memory

	PGSMET_E_CONFIG_VAL_STR_ERR	"Unable to obtain the value of configuration parameter <name>
					from the PCS file". Likelyhood is that either the parameter
					does not exist in the PCF or the PCF itself is in error which can
					be tested using pccheck.
	PGSMET_E_CONFIG_CONV_ERR	"Unable to convert the value of configuration parameter <name>
					from the PCS file into an ODL format". Its most likely that 
					the string values is not in ODL format.
	PGSTSF_E_GENERAL_FAILURE	problem in TSF code

EXAMPLES:
C:
	N/A
F:
	N/A

NOTES:
	Although This tool ignores the first field in the PCF file depicting the 
	config id, it is still important that this field is unique for the PC utility
	to function correctly

	User is responsible for the returned buffers to be large enough to
        hold the returned values.
        It is very important that variable string pointers are used for string manipulations.
        This is because void interface is used. For e.g. the following piece of code would give
        an error or unexpected results:

        .
        .
        char a[100];
        .
        .
        retVal = PGS_MET_GetConfigDataAttr("SATELLITE_NAME", a);
        retVal = PGS_MET_GetConfigData("SATELLITE_NAME", &a);

        The first call is wrong because the routine expects char** but cannot force it
        because of void interface. The second call is wrong too because of the declaration
        of 'a' which is a constant pointer, i.e. it would always point to the same
        location in memory of 100 bytes. Only the following construct will work with
        the routine in which the string pointer is declared as a variable

        char *a;
        .
        .
        a = (char *) malloc(100) / remember the user is responsible for the memory space /
        retVal = PGS_MET_GetConfigData("SATELLITE_NAME", &a);

        The above discussion is also true for arrays of strings. For e.g. the following
        is not allowed for the same reasons as above

        .
        .
        char a[10][100];
        .
        .
        retVal = PGS_MET_GetConfigData("SATELLITE_NAME", a[0]);

        while the following is acceptable

        .
        .
        char *a[10];
        .
        .
        a[0] = (char *) malloc(100) / remember the user is responsible for the memory space /
        retVal = PGS_MET_GetConfigData("SATELLITE_NAME", &a[0]);


	Addendum for tk5+

	The return types supported for the void interface are PGSt_integer, PGSt_double and char * and 
	their array counterparts. PGSt_real has been omitted because of the changes in tk5+. This routine 
	now simply retrieves the values from the PCF and does not perform type and range checking. The
	user is still required to assign enough space for the returned values.

	***IMPORTANT***
 
        The void buffer should always be large enough for the returned values otherwise routine behavior
        is uncertain.

REQUIREMENTS:
        PGSTK-0290 PGSTK-0380

DETAILS:
	See MET userguide
	
	
GLOBALS:
	PGSg_MET_MasterNode

FILES:
	PCF.v5 

FUNCTIONS_CALLED:
		PGS_MET_ErrorMsg
		PGS_MET_GetConfigByLabel
		NewAggregate
		ReadValue
		RemoveAggregate
		FindParameter
		FirstValue
		PGS_TSF_LockIt
		PGS_TSF_UnlockIt
		PGS_SMF_TestErrorLevel
		
END_PROLOG:
***************************************************************************/


PGSt_SMF_status
PGS_MET_GetConfigDataF(			    /* Retrieves metadata attr values
					     * from the configuration section of the PC file
					     */
             char 		*attrName,  /* Parameter attr to be retrieved */
             void 		*attrValue, /* Attribute value buffer to hold
                                             * the retrieved value
                                             */
	     unsigned int	stringSize) /* size of the fotran character array element */
{
	AGGREGATE               aggNode = NULL;
	AGGREGATE               groupNode = NULL;
	PARAMETER               parmNode = NULL;
        VALUE                   valueNode = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char *			funcName = "PGS_MET_GetConfigDataF";
        char *                 strPtr = NULL;
	PGSt_integer*           intPtr = NULL;
        PGSt_double*            dblPtr = NULL;
        char                    valString[PGSd_PC_LINE_LENGTH_MAX] = "";
        char                    valArray[PGSd_PC_LINE_LENGTH_MAX +2] = "";
	PGSt_integer            charcterCount = 0;
	char *                  blankPtr = NULL;
        char *                  valueString = NULL;
        PGSt_integer            secLoopCount = 0;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;                   /* lock/unlock return */
#endif
	

	if (errno == ERANGE)
        {
                errno = 0;
        }
	retVal = PGS_MET_GetConfigByLabel(attrName, valString);
        if(retVal != PGS_S_SUCCESS)
        {
                        errInserts[0] = attrName;
 
                /* error message is:
                "Unable to obtain the value of configuration parameter <name>
                 from the PCS file" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_CONFIG_VAL_STR_ERR,
                                     funcName, 1, errInserts);
                return(PGSMET_E_CONFIG_VAL_STR_ERR);
        }
	/* allow to handle arrays of values */
 
#ifdef _PGS_THREADSAFE
        /* since we are calling COTS (ODL) lock it */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif
        (void) sprintf(valArray, "(%s)", valString);
	aggNode = NewAggregate(groupNode, KA_OBJECT, attrName, "");
        if (aggNode == NULL)
        {
            errInserts[0] = attrName;
            /* error message is:
               "Unable to create odl aggregate <aggregate name>" */
 
            (void) PGS_MET_ErrorMsg(PGSMET_E_AGGREGATE_ERR, funcName,
                                    1, errInserts);
#ifdef _PGS_THREADSAFE
            /* Unlock - do not check return since we want user to know
               about previous error */
            PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
            return(PGSMET_E_AGGREGATE_ERR);
        }

	retVal = ReadValue(aggNode, "VALUE", valArray);
        if(retVal != 1)
        {
                errInserts[0] = attrName;
 
                /* error message is:
                "Unable to convert the value of configuration parameter <name>
                 from the PCS file into an ODL format" */
 
                (void) PGS_MET_ErrorMsg(PGSMET_E_CONFIG_CONV_ERR,
                                     funcName, 1, errInserts);
                RemoveAggregate(aggNode);
#ifdef _PGS_THREADSAFE
                /* Unlock - do not check return since we want user to know
                   about previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                return(PGSMET_E_CONFIG_CONV_ERR);
        }
	parmNode = FindParameter(aggNode, PGSd_MET_ATTR_VALUE_STR);
        valueNode = FirstValue(parmNode);
 
        /* copy the value string(s) into user provided string */
        if(valueNode->item.type == TV_STRING || valueNode->item.type == TV_SYMBOL)
        {
                strPtr = (char *) attrValue;
                while(valueNode != NULL)
                {
			/* fill the array with blanks first */
                        blankPtr = strPtr; /* point to the first location of individual string */
                        for(secLoopCount =0; secLoopCount < (stringSize); secLoopCount++)
                        {
                                *blankPtr = ' ';
                                 blankPtr++;
                        }
                        /* now fill with the value string untill end of string character
                           or the input str length is exhausted */
                        charcterCount =0;
                        valueString = valueNode->item.value.string;
                        while(charcterCount != (stringSize) &&
                              valueString[charcterCount] != '\0')
                        {
                                strPtr[charcterCount] = valueString[charcterCount];
                                charcterCount++;
                        }
                        strPtr = strPtr + stringSize; /* point to the next array string */
                        valueNode = NextValue(valueNode);
                }
        }
        else if(valueNode->item.type == TV_INTEGER)
        {
                intPtr = (PGSt_integer *) attrValue;
                while(valueNode != NULL)
                {
                        *intPtr = (PGSt_integer)valueNode->item.value.integer.number;
                        intPtr++;
                        valueNode = NextValue(valueNode);
                }
        }
        else /* it must be a double */        {
                dblPtr = (PGSt_double *) attrValue;
                while(valueNode != NULL)
                {
                        *dblPtr = (PGSt_double)valueNode->item.value.real.number;
                        dblPtr++;
                        valueNode = NextValue(valueNode);
                }
        }
	RemoveAggregate(aggNode);
#ifdef _PGS_THREADSAFE
        retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
        return(PGS_S_SUCCESS);
}
