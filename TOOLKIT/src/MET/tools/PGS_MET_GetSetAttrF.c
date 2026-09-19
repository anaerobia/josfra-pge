/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_GetSetAttrF
 
DESCRIPTION:
	Enables the user to get the values of metadata parameters 
	which are already set by the initialization procedure or
	previously set by the user in the MCF in memory
	
AUTHOR: 
        Alward N. Siyyid/ EOSL
        Carol S. W. Tsai / Applied Reseach Corporation

HISTORY:
        18-MAY-95      ANS     Initial version
	01-JUN-95      ANS     Code inspection updates
	13-July-95     ANS     Improved Fortran example
	09-Aug-95      ANS     Changed pgs_getsetattr to pgs_met_getsetattr
	11-MAR-96      ANS     updated for tk5+
	13-Jun-1996    ANS     corrected so that tk5 files can be read using this version
        09-Apr-1997    CSWT    Added code to provide the capability of locating the
                               attributes consistantly for Core Metadata and Product
                               Specific Metadata
        15-Jun-1996    CSWT    Added code enable the datetime data that is not in the type of
                               string but in the type of UTC DATETIME format without double 
                               quotes surrounding it can be retreived from the memory. 
        02-Oct-97      CSWT    Fixed Bug ECSed09222 about a defect in retrieving the Attribute
                               of Date and Time values
        16-Oct-97      CSWT    Modified code to match the new definition of "NOT SET:(data
                               type)" for Data Location PGE, "NOT SUPPLIED:(data type)" for
                               Data Location MCF, and "NOT FOUND:(data type) for Data
                               Location NONE in the function PGS_MET_CheckAttr() when the
                               attribute was not set up(This change is for NCR ECSed09386 about
                               appending a data type the value Node)
        18-Oct-97      CSWT    Changed the variable zone_hours, Zone hours from GMT (-12 - +12),
                               that defined as a data type of long to be the data type of short
                               in order to prevent a core dump problem on sgi old 32 bit and
                               sgi new 32 bit from executing the MET TestDriver program to
                               retrieve the attribute value with the data type of DateTime
        23-Oct-97      CSWT    Modified code to change back the definition of "NOT SET"
                               for Data Location PGE, "NOT SUPPLIED" for Data Location
                               MCF, and "NOT FOUND" for Data Location NONE in the function
                               PGS_MET_CheckAttr()  when the attribute was not set up
                               because of the removing of the new function
                               PGS_MET_ConvertToMCF() 
        27-May-98      CSWT    Modified the if statement
                               from:
                               if(valueNode->item.precision == 99) Zcharth = PGS_TRUE;
                               to:
                               if(valueNode->item.precision == -99) Zcharth = PGS_TRUE;
                               dued to item.precision, a field defined in the function cvtvalue() 
                               to hold date/time value, was modified from the original value
                               of 99 to -99 if zone indicator existing in the data file (This
                               change is for NCR ECSed15136 about The date_time values are too long)
	07-Jul-99    RM        updated for TSF functionality

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
        Accesses the metadata parameters already set in the memory
  
NAME:  
        PGS_MET_GetSetAttrF()

SYNOPSIS:
C:
	N/A	Only to be used through fortran interface, see PGS_MET_GetSetAttr() for more details
FORTRAN:
  	N/A 
DESCRIPTION:
	Metadata file is first initialized into memory and some of the parameters 
	are automatically set and some are set by the user using PGS_MET_SetAttr().
	This tool is used to retrieve these values.

INPUTS:
        Name            Description            Units    	Min          Max
        ----            -----------             -----   	---          ---
	mmdHandle	metadata group		none		N/A          N/A
			in MCF
        attrName	name.class of parameter none            N/A          N/A

OUTPUTS:
	attrValue       value of attribute to	none            N/A          N/A
			be passed back to the
			user

RETURNS:   
   	PGS_S_SUCCESS			
	PGSMET_E_NO_INITIALIZATION	Metadata file is not initialized
	PGSMET_E_DD_UNKNOWN_PARM	The requested parameter <parameter name> 
					could not be found in <agg node>	

	PGSMET_W_METADATA_NOT_SET	The metadata <name> is not yet set
	PGSMET_E_NO_DEFINITION		Unable to obtain <attr> of metadata <parameter>
					Either NUM_VAL or type is not defined
	PGSTSF_E_GENERAL_FAILURE	problem in TSF code

EXAMPLES:
C:

	N/A
FORTRAN:

	N/A
NOTES:
	It is very important that variable string pointers are used for string manipulations.
        This is because void interface is used. For e.g. the following piece of code would give
        an error or unexpected results:

        .
        .
        char a[100];
        .
        .
        retVal = PGS_MET_GetSetAttr(mdHandles[GROUP_GRANULE_DATA], "SATELLITE_NAME", a);
        retVal = PGS_MET_GetSetAttr(mdHandles[GROUP_GRANULE_DATA], "SATELLITE_NAME", &a);

        The first call is wrong because the routine expects char** but cannot force it
        because of void interface. The second call is wrong too because of the declaration
        of 'a' which is a constant pointer, i.e. it would always point to the same
        location in memory of 100 bytes. Only the following construct will work with
        the routine in which the srting pointer is declared as a variable

        char *a;
        .
        .
        retVal = PGS_MET_GetSetAttr(mdHandles[GROUP_GRANULE_DATA], "SATELLITE_NAME", &a);

        The above discussion is also true for arrays of strings. For e.g. the following
        is not allowed for the same reasons as above

        .
        .
        char a[10][100];
        .
        .
        retVal = PGS_MET_GetSetAttr(mdHandles[GROUP_GRANULE_DATA], "SATELLITE_NAME", &a[0]);

        while the following is accepatable

        .
        .
        char *a[10];
        .
        .
        retVal = PGS_MET_GetSetAttr(mdHandles[GROUP_GRANULE_DATA], "SATELLITE_NAME", &a[0]);

        Another important point is that there may be cases where metadata name is shared.
        For e.g. there could be a metadata attribute called LATITUDE defining sub-satellite point
        and there could be another giving grid reference. In such cases the tool distinguishes
        between the two using the CLASS of the metadata which is part of the input name string.
        For e.g. The above mentioned latitudes can be represented as follows:

        attrNameStr = "LATITUDE.GRID"
        attrNameStr = "LATITUDE.SATELLITE"

        where GRID and SATELLITE are the two classes respectively.

        The CLASS field is optional and is needed only under the aforementioned circumstances. 

	Addendum for tk5+
 
        In Tk5, the number of values for a particular metadata parameter was fixed in the datadictionary.
        This has now changed and the user has the freedom to set 1 to n values for a particlar parameter
        where n is defined in the NUM_VAL field in the MCF. In this case where the values are being
        retrieved, the end of array is marked by:
 
                INT_MAX                 for integers
	       UINT_MAX                 for unsigned integers
                DBL_MAX                 for doubles
                NULL                    char * (strings)

		FORTRAN Users:

		Use PGSd_MET_INT_MAX, PGSd_MET_DBL_MAX and PGSd_MET_STR_END respectively.
 
        The user can check for these values to determine the actual number of values retrieved.
        In case where the number of values retreived is equal to n, there is no end of array marker
        since user is expected to know n for setting the return buffer.
 
        The return types supported for the void interface are PGSt_integer, PGSt_double and char * and
        their array counterparts. PGSt_real has been omitted because of the changes in tk5+. This routine
        now simply retrieves the values from the HDF headers or MCF and does not perform type and range 
        checking. The user is still required to assign enough space for the returned values.

	The use of name.class stated above still holds true but now class is used to define matadata
	with multiple instances. See PGS_MET_SetAttr for more details on multiplicity.
 
        ***IMPORTANT***
 
        The void buffer should always be large enough for the returned values otherwise routine behavior
        is uncertain.

REQUIREMENTS:
        PGSTK-0290 PGSTK-380

DETAILS:
	The tool provides a void interface through which different types of metadata
        can retrieved. The types supported are
                PGSt_integer
                PGSt_float
                PGSt_double
                string
        and their arrays counterparts. There is a small price to pay regarding 'strings'
        where 'void' interfaces are concerned which is explained in the 'notes' section
        above.
	
	
GLOBALS:
	PGSg_MET_MasterNode

FILES:
	None
FUNCTIONS_CALLED:
	PGS_MET_NameAndClass
	PGS_SMF_SearchAttr
	PGS_SMF_SetStaticMsg
	PGS_MET_ErrorMsg
	FindGroup
	FindObject
	FindParameter
	FirstValue
	PGS_TSF_GetTSFMaster
	PGS_SMF_TestErrorLevel
	
	
END_PROLOG:
***************************************************************************/



PGSt_SMF_status
PGS_MET_GetSetAttrF(			    /* Retrieves metadata attr values
					     * which are already set by the Init 
					     * or the user. If the metadata value
					     * is not set, a warning is returned
					     */
             PGSt_MET_handle  	mdHandle,   /* list of groups within MCF */
             char 		*attrNameStr,  /* Parameter attr to be retrieved */
             void 		*attrValue, /* Attribute value buffer to hold
                                             * the retrieved value
                                             */
	     unsigned int	strLength) /* string length if the void buffer is character string 
                                            */
{
        struct ODLDate
        {
          short             year;                 /* Year number                    */
          short             doy;                  /* Day of year (0-366)            */
          char              month;                /* Month of year (1-12)           */
          char              day;                  /* Day of month (1-31)            */
          short             zone_hours;           /* Zone hours from GMT (-12 - +12)*/
          char              zone_minutes;         /* Zone minutes from GMT (0-59)   */
          char              hours;                /* Hours of day (0-23)            */
          char              minutes;              /* Minutes of hour (0-59)         */
          char              seconds;              /* Seconds of minute (0-59)       */
          long              nanoseconds;          /* Nanoseconds (0-999999999)      */
        };
 
 
        long                    iyear;
        long                    imonth;
        long                    iday;
        long                    nday;
        long                    izonehours;
        long                    izoneminutes;
        long                    ihours;
        long                    iminutes;
        long                    iseconds;
        double                  inanoseconds;
 
        struct ODLDate *        date_time_Ptr;

	AGGREGATE		mdGroupNode;
	AGGREGATE		mdNode = NULL;	/* parameter requeted */
	PARAMETER		mdParmNode = NULL;	/* parameter attribute requested */
	PARAMETER               maxValparm = NULL;
	VALUE                   maxValNode = NULL;
	VALUE                   mdValueNode = NULL;
	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer            secLoopCount = 0;
        PGSt_integer            maxVal = 0;
        PGSt_integer            valCount = 0;
	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char *			funcName = "PGS_MET_GetSetAttr";
	char *			blankPtr = NULL;
	char *			valueString = NULL;
	char                    attrClass[PGSd_MET_CLASS_L]  = ""; /* attribute class */
        char                    attrName[PGSd_MET_NAME_L]  = ""; /* attribute Name */
	char *			SearchoutString;	/* pointer used to place values in void buffer */
        char *                  datetimePtr = NULL;
        char                    cmonth[PGSd_MET_NAME_L] = "";
        char                    cday[PGSd_MET_NAME_L] = "";
        char                    chours[PGSd_MET_NAME_L] = "";
        char                    czonehours[PGSd_MET_NAME_L] = "";
        char                    czoneminutes[PGSd_MET_NAME_L] = "";
        char                    cminutes[PGSd_MET_NAME_L] = "";
        char                    cseconds[PGSd_MET_NAME_L] = "";
        char                    date_time_name[PGSd_MET_NAME_L] = "";
        char *                  ccinanoseconds;
        char                    cinanoseconds[PGSd_MET_NAME_L] = "";
	char *			outString;	/* pointer used to place values in void buffer */
        static PGSt_integer     month1[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
        static PGSt_integer     month2[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

	PGSt_integer *		outInteger;	/* pointer used to place values in void buffer */
	PGSt_double *		outDouble;	/* pointer used to place values in void buffer */
	PGSt_integer		charcterCount = 0;
	char 			newLine[3] = "";
	char                    newLineConst[5] = "";
        PGSt_integer            i;
	PGSt_integer            mcfNumber = 0;
        char *                  mcfNumPtr = NULL;
	char *                  errPtr = NULL;
        char                    locMdHandle[PGSd_MET_GROUP_NAME_L] = "";
        PGSt_boolean            leapyear = PGS_FALSE;
        PGSt_boolean            yearday = PGS_FALSE;
        PGSt_boolean            secondiszero = PGS_FALSE;
        PGSt_boolean            zonehoursiszero = PGS_FALSE;
        PGSt_boolean            Zchar = PGS_FALSE;

#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retTSF;
    char *lasts;
    AGGREGATE *PGSg_MET_MasterNode;
    PGSt_TSF_MasterStruct *masterTSF;

    /* get struct with all the keys in it */
    retTSF = PGS_TSF_GetTSFMaster(&masterTSF);
    if (PGS_SMF_TestErrorLevel(retTSF))
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* set local from TSD value */
    PGSg_MET_MasterNode = (AGGREGATE *) pthread_getspecific(
                                  masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE]);
#else
	extern AGGREGATE	PGSg_MET_MasterNode[];
#endif

	/* clear the errno for the ODL routines */

        if (errno == ERANGE)
        {
                errno = 0;
        }
	/* find out the MCF number contained in the mdHandle argument */

	if(mdHandle != NULL)
	{
		mcfNumPtr = strrchr(mdHandle, '#');
	}
        if(mcfNumPtr == NULL)
        {
                (void) PGS_SMF_SetStaticMsg(PGSMET_E_ILLEGAL_HANDLE, funcName);
                return(PGSMET_E_ILLEGAL_HANDLE);
        }
        mcfNumPtr++;
        mcfNumber = (PGSt_integer)strtol(mcfNumPtr, &errPtr, 10);
        if(mcfNumber < 0 || mcfNumber > PGSd_MET_NUM_OF_MCF || errPtr == mcfNumPtr)
        {
                (void) PGS_SMF_SetStaticMsg(PGSMET_E_ILLEGAL_HANDLE, funcName);
                return(PGSMET_E_ILLEGAL_HANDLE);
        }

	
	/* if master node is null then initialization has not taken place */

	strcpy(newLineConst, "\\");
        strcat(newLineConst, "n");

        if(PGSg_MET_MasterNode[mcfNumber] == NULL)
        {
                (void) PGS_SMF_SetStaticMsg(PGSMET_E_NO_INITIALIZATION, funcName);
                return(PGSMET_E_NO_INITIALIZATION);
        }

	/* now make a local copy of the mdHandle */

        strcpy(locMdHandle, mdHandle);
        mcfNumPtr = strrchr(locMdHandle, '#');
        *mcfNumPtr = '\0';

	/* separate out the parm name and class */

        (void) PGS_MET_NameAndClass(attrNameStr, attrName, attrClass);

#ifdef _PGS_THREADSAFE
        /* calling COTS (ODL) - must lock here */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	/* locate the group node */
	mdGroupNode = FindGroup(PGSg_MET_MasterNode[mcfNumber], locMdHandle);
#ifdef _PGS_THREADSAFE
        /* re-set TSD value */
        pthread_setspecific(masterTSF->keyArray[PGSd_TSF_KEYMETMASTERNODE],
                         PGSg_MET_MasterNode);
#endif

	/* find the given metadata parameter according to name and class */

        SearchoutString = (char *) attrValue;
        outString = (char *) attrValue;
        outInteger = (PGSt_integer *) attrValue;
        outDouble = (PGSt_double *) attrValue;
 
	mdNode = FindObject(mdGroupNode, attrName, attrClass);
	if(mdNode == NULL)
	{
		/* This routine may also be reading nodes which simply define attributes
		 * as P=V since this routine is also used by PGS_MET_GetPCAttr
		 */
	        retVal = PGS_MET_SearchAttrF(mdHandle, mdGroupNode, attrName, SearchoutString,strLength);	

		if(retVal != PGS_S_SUCCESS)
		{
			errInserts[0] = attrName;
			if(mdGroupNode != NULL)
                        {
                                errInserts[1] = mdGroupNode->name;
                        }
                        else
                        {
                                errInserts[1] = "NULL";
                        }
			if(attrName == (char *) NULL)
			{
				errInserts[0] = "NULL";
			}

	                /* error message is:
        	        "The requested parameter <parameter name> could not be found in <agg node>" */

 	               (void) PGS_MET_ErrorMsg(PGSMET_E_DD_UNKNOWN_PARM,
        	                             funcName, 2, errInserts);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - we want user to know
                           about previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                	return(PGSMET_E_DD_UNKNOWN_PARM);
		}
                else
                {
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - we want user to know
                           about previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
			PGS_SMF_SetDynamicMsg(PGS_S_SUCCESS, "",funcName);
                        return(PGS_S_SUCCESS);
                }
        }
	else
	{
		/* get the parameter node */
		
		mdParmNode = FindParameter(mdNode, PGSd_MET_ATTR_VALUE_STR);
		
	        if(mdParmNode == NULL)
        	{
                	errInserts[0] = attrName;

 	               /* error message is:
        	        "The metadata <name> is not yet set */

 	               (void) PGS_MET_ErrorMsg(PGSMET_W_METADATA_NOT_SET,
        	                             funcName, 1, errInserts);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - we want user to know
                           about previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                	return(PGSMET_W_METADATA_NOT_SET);
		}
        }
		
	/* get the number of values for this particular metadata */
        maxValparm = FindParameter(mdNode, PGSd_MET_ATTR_NUMOFVAL_STR);
	maxVal = 0;
        if(maxValparm == NULL)
        {
                /* tk5 version does not have NUM_VAL field. set the maxVal
                   to the actual number of values present */
                mdValueNode = FirstValue(mdParmNode);
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
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - we want user to know
                           about previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                        return(PGSMET_E_NO_DEFINITION);
        	}
        	else
        	{
                	maxVal = maxValNode->item.value.integer.number;
        	}
	}

	mdValueNode = FirstValue(mdParmNode);
	if(mdValueNode->item.type == TV_STRING && maxValparm != NULL)
        {
                if(strncmp(mdValueNode->item.value.string, "NOT FOUND", strlen("NOT FOUND")) == 0 ||
                   strncmp(mdValueNode->item.value.string, "NOT SUPPLIED", strlen("NOT SUPPLIED"))  == 0 ||
                   strncmp(mdValueNode->item.value.string, "NOT SET", strlen("NOT SET"))  == 0 ||
                   strcmp(mdValueNode->item.value.string, "NOT INCLUDED")  == 0 ||
                   strcmp(mdValueNode->item.value.string, "NOT SUPPORTED")  == 0 ||
                   strcmp(mdValueNode->item.value.string, "NOT PROVIDED")  == 0)
                {
                        errInserts[0] = attrName;
 
                       /* error message is:
                        "The metadata <name> is not yet set */
 
                       (void) PGS_MET_ErrorMsg(PGSMET_W_METADATA_NOT_SET,
                                             funcName, 1, errInserts);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - we want user to know
                           about previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                        return(PGSMET_W_METADATA_NOT_SET);
                }
        }
	/* no check for error return since such an error would be trapped while loading */

/* Initialize various type pointers to point to the start of the void data space
 * this is where this routines differs from the original routine. Fortran strings
 * have trailing blanks which are automatically handled by the cfortran.h. In this
 * particular case, because the void interface is used, anther type called STRVOID
 * was defined in the cfortran.h which would pass the length of the string as a 
 * hidden variable through the wrapper routine. Once the length of the string is known
 * the question of trailing blanks is handled by this routine when manipulating strings
 */

/* once onto the value nodes, simply fill the buffer with values from the 
 * value nodes according to the type of data
 */
	valCount = 1;
	while(mdValueNode != NULL && valCount <= maxVal)
	{
		if(mdValueNode->item.type == TV_STRING || mdValueNode->item.type == TV_SYMBOL)
		{
			/* fill the array with blanks first */
			blankPtr = outString; /* point to the first location of individual string */
			for(secLoopCount =0; secLoopCount < (strLength); secLoopCount++)
			{
				*blankPtr = ' ';
				 blankPtr++;
			}	
			/* now fill with the value string untill end of string character
			   or the input str length is exhausted */
			charcterCount =0;
			/* sometimes ODL adds the newline character at front */

			newLine[0] = (mdValueNode->item.value.string)[0];
                        newLine[1] = (mdValueNode->item.value.string)[1];
                        newLine[2] = '\0';
                        if(strcmp(newLine, newLineConst) == 0) /* thne string value contains the new line character */
                        {
                                valueString = mdValueNode->item.value.string + 2;
                        }
                        else
                        {
                                valueString = mdValueNode->item.value.string;
                        }
			while(charcterCount != (strLength) &&
			      valueString[charcterCount] != '\0')
			{
				outString[charcterCount] = valueString[charcterCount];
				charcterCount++;
			}
			outString = outString + strLength; /* point to the next array string */
			if(valCount != maxVal)
			{
				blankPtr = outString; /* point to the first location of individual string */
                        	for(secLoopCount =0; secLoopCount < (strLength); secLoopCount++)
                        	{
                                	*blankPtr = ' ';
                                 	blankPtr++;
                        	}
				*outString = '$';
			}
		}
                else if(mdValueNode->item.type == TV_DATE || mdValueNode->item.type == TV_TIME|| mdValueNode->item.type == TV_DATE_TIME || mdValueNode->item.type == TV_SYMBOL)
                {
                        datetimePtr = (char *) attrValue;
 
                        date_time_Ptr = (struct ODLDate *) malloc ( sizeof(short) +
                                                 sizeof(short) +
                                                 sizeof(char) +
                                                 sizeof(char) +
                                                 sizeof(short) +
                                                 sizeof(char) +
                                                 sizeof(char) +
                                                 sizeof(char) +
                                                 sizeof(char) +
                                                 sizeof(long));
 
                        while(mdValueNode != NULL)
                        {
                                date_time_Ptr->year = mdValueNode->item.value.date_time.year;
                                iyear = (long) (date_time_Ptr->year);
 
                                date_time_Ptr->month = mdValueNode->item.value.date_time.month;
                                imonth = (long) (date_time_Ptr->month);
 
                                if ((imonth / 10) < 1) sprintf(cmonth,"0%ld",imonth);
                                else sprintf(cmonth,"%ld",imonth);
 
                                date_time_Ptr->doy = mdValueNode->item.value.date_time.doy;
                                iday = (long) (date_time_Ptr->doy);
 
                                date_time_Ptr->day = mdValueNode->item.value.date_time.day;
                                nday = (long) (date_time_Ptr->day);
 
                                if(nday == 0 || imonth == 0) yearday = PGS_TRUE;
 
                                if (yearday == PGS_TRUE)
                                {
 
                                    if ((iday / 100) >= 1) sprintf(cday,"%ld",iday);
                                    else
                                    {
                                       if ((iday / 10) >= 1 && (iday / 100) < 1) sprintf(cday,"0%ld",iday);
                                       else sprintf(cday,"00%ld",iday);
                                    }
                                }
                                else
                                {
                                    if ((nday / 10) < 1) sprintf(cday,"0%ld",nday);
                                    else sprintf(cday,"%ld",nday);
                                }
 
 
                                /* check year is a leap year or not */
                                if((iyear % 100) == 0)
                                {
                                    if((iyear / 100) == 0) leapyear = PGS_TRUE;
                                }
                                else
                                {
                                    if((iyear % 4) == 0) leapyear = PGS_TRUE;
                                }
 
                                for (i=0; i<12; i++)
                                {
                                     if(leapyear == PGS_TRUE)
                                     {
                                        if(iday > month2[i]) iday -= month2[i];
                                        else break;
                                     }
                                     else
                                     {
                                         if(iday > month1[i]) iday -= month1[i];
                                         else break;
                                     }
                                }
 
 
                                date_time_Ptr->zone_hours = mdValueNode->item.value.date_time.zone_hours;
                                izonehours = (long) (date_time_Ptr->zone_hours);
 
                                date_time_Ptr->zone_minutes = mdValueNode->item.value.date_time.zone_minutes;
                                izoneminutes = (long) (date_time_Ptr->zone_minutes);
 
                                if(mdValueNode->item.precision == -99) Zchar = PGS_TRUE;
                                if(izonehours == 0) zonehoursiszero = PGS_TRUE;
                                if(izonehours > 0) sprintf(czonehours, "+%ld",izonehours);
                                if(izonehours < 0) sprintf(czonehours, "%ld",izonehours);
                                if((izoneminutes / 10) < 1) sprintf(czoneminutes,":0%ld",izoneminutes);
                                else sprintf(czoneminutes, ":%ld",izoneminutes);
 
                                date_time_Ptr->hours = mdValueNode->item.value.date_time.hours;
                                ihours = (long) (date_time_Ptr->hours);
                                if ((ihours /10) < 1) sprintf(chours,"%0ld",ihours);
                                else sprintf(chours,"%ld",ihours);
 
                                date_time_Ptr->minutes = mdValueNode->item.value.date_time.minutes;
                                iminutes = (long) (date_time_Ptr->minutes);
                                if((iminutes / 10) < 1) sprintf(cminutes,"0%ld",iminutes);
                                else sprintf(cminutes,"%ld",iminutes);
 
                                date_time_Ptr->seconds = mdValueNode->item.value.date_time.seconds;
                                iseconds = (long) (date_time_Ptr->seconds);
                                if((iseconds / 10) < 1) sprintf(cseconds,"0%ld",iseconds);
                                else sprintf(cseconds,"%ld",iseconds);
 
                                date_time_Ptr->nanoseconds = mdValueNode->item.value.date_time.nanoseconds;
                                inanoseconds = (double) (date_time_Ptr->nanoseconds) / 1.0E9;
 
                                if(iseconds == 0.0 && inanoseconds == 0.0) secondiszero =PGS_TRUE;
                                if(secondiszero == PGS_FALSE)
                                {
                                   if((iseconds / 10) < 1) sprintf(cseconds,"0%ld",iseconds);
                                   else sprintf(cseconds,"%ld",iseconds);
                                   sprintf(cinanoseconds,"%f",inanoseconds);
#ifdef _PGS_THREADSAFE
                                   /* strtok() not threadsafe - use strtok_r() */
                                   ccinanoseconds = strtok_r(cinanoseconds,".",
                                                             &lasts);
                                   ccinanoseconds = strtok_r(NULL,".",&lasts);
#else
                                   ccinanoseconds = strtok(cinanoseconds,".");
                                   ccinanoseconds = strtok(NULL,".");
#endif
                                }
                                else
                                {
                                   sprintf(cseconds,"0%ld",iseconds);
                                   sprintf(cinanoseconds,"%f",inanoseconds);
#ifdef _PGS_THREADSAFE
                                   /* strtok() not threadsafe - use strtok_r() */
                                   ccinanoseconds = strtok_r(cinanoseconds,".",
                                                             &lasts);
                                   ccinanoseconds = strtok_r(NULL,".",&lasts);
#else
                                   ccinanoseconds = strtok(cinanoseconds,".");
                                   ccinanoseconds = strtok(NULL,".");
#endif
                                }
 
                                if((yearday == PGS_TRUE && secondiszero == PGS_TRUE) && zonehoursiszero == PGS_TRUE )
                                {
                                   if(Zchar == PGS_TRUE) sprintf(date_time_name, "%d-%sT%s:%s:%s.%s%s%sZ",date_time_Ptr->year,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                   else sprintf(date_time_name, "%d-%sT%s:%s:%s.%s%s%s",date_time_Ptr->year,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                }
                                else if(yearday == PGS_TRUE && zonehoursiszero == PGS_FALSE)
                                {
                                   if(Zchar == PGS_TRUE) sprintf(date_time_name, "%d-%sT%s:%s:%s.%s%s%sZ",date_time_Ptr->year,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                   else sprintf(date_time_name, "%d-%sT%s:%s:%s.%s%s%s",date_time_Ptr->year,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                }
                                else if(yearday == PGS_TRUE)
                                {
                                   if(Zchar == PGS_TRUE) sprintf(date_time_name, "%d-%sT%s:%s:%s.%s%s%sZ",date_time_Ptr->year,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                   else sprintf(date_time_name, "%d-%sT%s:%s:%s.%s%s%s",date_time_Ptr->year,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                }
                                else
                                {
                                   if(Zchar == PGS_TRUE) sprintf(date_time_name, "%d-%s-%sT%s:%s:%s.%s%s%sZ",date_time_Ptr->year,cmonth,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                   else sprintf(date_time_name, "%d-%s-%sT%s:%s:%s.%s%s%s",date_time_Ptr->year,cmonth,cday,chours,cminutes,cseconds,ccinanoseconds,czonehours,czoneminutes);
                                }
                                /* now fill with the value string untill end of string character
                                or the input str length is exhausted */
 
                                charcterCount =0;
                                while(charcterCount != (strLength) &&
                                date_time_name[charcterCount] != '\0')
                                {
                                        datetimePtr[charcterCount] = date_time_name[charcterCount];
                                        charcterCount++;
                                }

                                datetimePtr = datetimePtr + strLength; /* point to the next array string */ 
                                mdValueNode = NextValue(mdValueNode);
                        }
 
                }

		else if(mdValueNode->item.type == TV_INTEGER)
		{
                        *outInteger = (PGSt_integer) mdValueNode->item.value.integer.number;
			outInteger++;
			if(valCount != maxVal)
                        {
                                *outInteger = INT_MAX;
                        }
        	}
		else if(mdValueNode->item.type == TV_REAL)
        	{
                        *outDouble = (PGSt_double) mdValueNode->item.value.real.number;
			outDouble++;
			if(valCount != maxVal)
                        {
                                *outDouble = DBL_MAX;
                        }
        	}
		mdValueNode = NextValue(mdValueNode);
		valCount++;
	}
#ifdef _PGS_THREADSAFE
        retTSF = PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
	PGS_SMF_SetDynamicMsg(PGS_S_SUCCESS, "",funcName);
	return(PGS_S_SUCCESS);
}
