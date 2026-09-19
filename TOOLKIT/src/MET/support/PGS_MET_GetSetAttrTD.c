/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
        PGS_MET_GetSetAttrTD
 
DESCRIPTION:
	Enables PGS_MET_Write to get the values of metadata ProductionDateTime
	which are already set by the initialization procedure or
	previously set by the user in the MCF in memory. If parameter cannot be
	found in MCF or if it is not set, and data location is PGE , it returns 
	warning so that PGS_MET_Write does not try setting it.
	
AUTHOR: 
        Alward N. Siyyid/ EOSL
        Carol S. W. Tsai / Applied Reseach Corporation
	Abe Taaheri / Emergent Information Technologies, Inc.

HISTORY:
        18-MAY-95      ANS     Initial version
	01-JUN-95      ANS     Code inspection updates
	13-July-95     ANS     Improved Fortran example
	09-Aug-95      ANS     Changed pgs_getsetattr to pgs_met_getsetattr
	11-MAR-96      ANS     updated for tk5+
	13-Jun-1996     ANS     corrected so that tk5 files can be read using this version
        04-Apr-1997    CSWT    Added code to provide the capability of locating the
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
        01-Dec-97      CSWT    Modified code to replace the original code that assigned a NULL
                               character whenever the assign a NULL character to a pointer used to
                               place values in void buffer to the data type of string in the 
                               following conditions:
                               1: The variable loopCount declared as a index to count the
                                  exact number of elements of attribute values defined by 
                                  users when calling the function PGS_MET_SetAttr() to set 
                                  values for the alttribute, is equal to the variable,
                                  mdNumOfVal declared as a integer number for the NUM_VAL 
                                  defined in the MCF (Metadata Configuration File) to be
                                  the expected maximun number of values that users can put 
                                  the attribute; 
                               2: The value of current element of array is not equal to
                                  the NULL.  
                               (This change is for NCR 10390 about it will core dump if the 
                                entry is different between the MCF and input files)
        27-May-98      CSWT    Modified the if statement
                               from:
                               if(valueNode->item.precision == 99) Zcharth = PGS_TRUE;
                               to:
                               if(valueNode->item.precision == -99) Zcharth = PGS_TRUE;
                               dued to item.precision, a field defined in the function cvtvalue() 
                               to hold date/time value, was modified from the original value
                               of 99 to -99 if zone indicator existing in the data file (This
                               change is for NCR ECSed15136 about The date_time values are too long)
	07-Jul-99    RM        Updated for TSF functionality
        12-Sep-00    AT        Created this function from PGS_MET_GetSetAttr and
	                       modified it to support PGS_MET_Write in setting
			       PRODUCTIONDATETIME correctly if data location is
			       "TK" and the parameter PRODUCTIONDATETIME exists
			       in the MCF
                                
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
        PGS_MET_GetSetAttrTD()

SYNOPSIS:

        N/A
   
DESCRIPTION:
	Metadata file is first initialized into memory and some of the parameters 
	are automatically set and some are set by the user using PGS_MET_SetAttr().
	This tool is used to retrieve value for ProductionDateTime to see it is
	set or not.

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
	PGS_MET_E_ILLEGAL_HANDLE        Handle is illegal. Check that initialization has taken place
	PGSTSF_E_GENERAL_FAILURE	problem in TSF code
	PGSMET_M_METADATA_NOT_SET_TK    Metadata has not been set, TOOLKIT may set it
	PGSMET_M_METADATA_NOT_SET_PGE   Metadata has not been set, TOOLKIT will
	not set it

EXAMPLES:

        N/A

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
PGS_MET_GetSetAttrTD(			    /* Retrieves metadata attr values
					     * which are already set by the Init 
					     * or the user. If the metadata value
					     * is not set, a warning is returned
					     */
             PGSt_MET_handle  	mdHandle,   /* list of groups within MCF */
             char 		*attrNameStr,  /* Parameter attr to be retrieved */
             void 		*attrValue) /* Attribute value buffer to hold
                                             * the retrieved value
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

	AGGREGATE               mdGroupNode = NULL;
	AGGREGATE		mdNode = NULL;	/* parameter requeted */
	PARAMETER		mdParmNode = NULL;	/* parameter attribute requested */
	PARAMETER               maxValparm = NULL;
	VALUE			mdValueNode = NULL;	/* value of the attribute */
	VALUE                   maxValNode = NULL;

	PGSt_SMF_status 	retVal = PGS_S_SUCCESS;	/* SDPS toolkit ret value */
	PGSt_integer            maxVal = 0;
	PGSt_integer		valCount = 0;
        PGSt_integer            i;
        static PGSt_integer     month1[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
        static PGSt_integer     month2[12] = {31,29,31,30,31,30,31,31,30,31,30,31};

	char *			errInserts[PGSd_MET_MAX_ERR_INSERTS] = {NULL}; 
							/* Dynamic strings inserted in 
							 * error messages
							 */
	char *			funcName = "PGS_MET_GetSetAttrTD";
	char **			outDatetime;	/* pointer used to place values in void buffer */
	char **			outString;	/* pointer used to place values in void buffer */
	char  			attrClass[PGSd_MET_CLASS_L]  = ""; /* attribute class */
	char                    attrName[PGSd_MET_NAME_L]  = ""; /* attribute Name */
	PGSt_integer *		outInteger;	/* pointer used to place values in void buffer */
	PGSt_double *		outDouble;	/* pointer used to place values in void buffer */
	char 			newLine[3] = "";
	char                    newLineConst[5] = "";
        char                    cmonth[PGSd_MET_NAME_L] = "";
        char                    cday[PGSd_MET_NAME_L] = "";
        char                    czonehours[PGSd_MET_NAME_L] = "";
        char                    czoneminutes[PGSd_MET_NAME_L] = "";
        char                    chours[PGSd_MET_NAME_L] = "";
        char                    cminutes[PGSd_MET_NAME_L] = "";
        char                    cseconds[PGSd_MET_NAME_L] = "";
        char                    date_time_name[PGSd_MET_NAME_L] = "";
        char *                  ccinanoseconds;
        char                    cinanoseconds[PGSd_MET_NAME_L] = "";
	PGSt_integer		mcfNumber = 0;
	char *			mcfNumPtr = NULL;
	char *                  errPtr = NULL;
	char 			locMdHandle[PGSd_MET_GROUP_NAME_L] = "";
        PGSt_boolean            leapyear = PGS_FALSE;
        PGSt_boolean            yearday = PGS_FALSE;
        PGSt_boolean            secondiszero = PGS_FALSE;
        PGSt_boolean            zonehoursiszero = PGS_FALSE;
        PGSt_boolean            Zchar = PGS_FALSE;
        PARAMETER               locNode = NULL;
        VALUE                   locValue = NULL;
        PARAMETER               mandNode = NULL;
        VALUE                   mandValue = NULL;
#ifdef _PGS_THREADSAFE
    PGSt_SMF_status retTSF;
    char *lasts;
    AGGREGATE *PGSg_MET_MasterNode;
    PGSt_TSF_MasterStruct *masterTSF;

    /* get struct that houses TSD keys */
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
	/* find out the MCF number contained in the mdHandle argument*/
	
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
	
	strcpy(newLineConst, "\\");
	strcat(newLineConst, "n");

	/* if master node is null then initialization has not taken  place */

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
        /* calling COTS (ODL) need to lock here */
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

        outString = (char **) attrValue;
        outDatetime = (char **) attrValue;
        outInteger = (PGSt_integer *) attrValue;
        outDouble = (PGSt_double *) attrValue;
 
	mdNode = FindObject(mdGroupNode, attrName, attrClass);
	if(mdNode == NULL)
	{
           /* This routine may also be reading nodes which simply define attributes
            * as P=V since this routine is also used by PGS_MET_GetPCAttr
            */

            retVal = PGS_MET_SearchAttr(mdHandle, mdGroupNode, attrName, attrValue);

            if (retVal != PGS_S_SUCCESS)
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
		/*
                (void) PGS_MET_ErrorMsg(PGSMET_E_DD_UNKNOWN_PARM,
                                        funcName, 2, errInserts);
		*/
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - we want user to know about
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                return(PGSMET_E_DD_UNKNOWN_PARM);
            }
            else
            {
#ifdef _PGS_THREADSAFE
                /* unlock - do not check return - we want user to know about
                   previous error */
                PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                return(PGS_S_SUCCESS); 
            }
        } 
	else 
	{
		/* get the parameter node */
		
		mdParmNode = FindParameter(mdNode, PGSd_MET_ATTR_VALUE_STR);
		locNode = FindParameter(mdNode, PGSd_MET_DATA_LOC_STR);
		locValue = FirstValue(locNode);
		if(strcmp(locValue->item.value.string, PGSd_MET_SET_BY_TK) 
		   == 0) /* Attribute will be set by TOOLKIT */
		{
		    return(PGSMET_M_METADATA_NOT_SET_TK);
		}  
		mandNode = FindParameter(mdNode, PGSd_MET_MANDATORY_STR);
		mandValue = FirstValue(mandNode);
	        if(mdParmNode == NULL)
        	{
		    if((strcmp(locValue->item.value.string, PGSd_MET_SET_BY_PGE)
			== 0) && (strcmp(mandValue->item.value.string, "TRUE")
				  == 0))
		    {
                	errInserts[0] = attrName;

 	               /* error message is:
        	        "The metadata <name> is not yet set */

 	               (void) PGS_MET_ErrorMsg(PGSMET_W_METADATA_NOT_SET,
        	                             funcName, 1, errInserts);
#ifdef _PGS_THREADSAFE
                        /* unlock - do not check return - we want user to know about
                           previous error */
                        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                	return(PGSMET_W_METADATA_NOT_SET);
		    }
		    else if((
		       strcmp(locValue->item.value.string, PGSd_MET_SET_BY_PGE)
		       == 0) && (strcmp(mandValue->item.value.string, "TRUE")
				 != 0))
		    {
			return(PGSMET_M_METADATA_NOT_SET_PGE);
		    }
		    else
		    {
			return(PGSMET_M_METADATA_NOT_SET_TK);
		    }
		    
		}
        }

	/* get the number of values for this particular metadata */
	maxValparm = FindParameter(mdNode, PGSd_MET_ATTR_NUMOFVAL_STR);
	
	maxVal = 0;
	if(maxValparm == NULL)
	{
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
                        /* unlock - do not check return - we want user to know about
                           previous error */
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
		   strcmp(mdValueNode->item.value.string, "NOT OBTAINED")  == 0 ||
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
                        /* unlock - do not check return - we want user to know about
                           previous error */
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
			/* sometimes ODL has a new line character in fronty of the variable */
			newLine[0] = (mdValueNode->item.value.string)[0];
			newLine[1] = (mdValueNode->item.value.string)[1];
			newLine[2] = '\0';
		        if(strcmp(newLine, newLineConst) == 0)
			{
				strcpy(*outString,mdValueNode->item.value.string + 2);

			}
			else
			{
				strcpy(*outString,mdValueNode->item.value.string);
			}
			outString++;
			if(valCount != maxVal && (*outString == NULL || strcmp(*outString, " ") ==0))
			{
			        ; 
			}
			else if(valCount != maxVal && (*outString != NULL || !(strcmp(*outString, " ") ==0)))
			{
				*(*outString) = '\0';
			}
		}
                else if(mdValueNode->item.type == TV_DATE || mdValueNode->item.type == TV_TIME || mdValueNode->item.type ==TV_DATE_TIME || mdValueNode->item.type == TV_SYMBOL)
                {
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
                                if((izoneminutes / 10) < 1) sprintf(czoneminutes, ":0%ld",izoneminutes);
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
 
                                date_time_Ptr->nanoseconds = mdValueNode->item.value.date_time.nanoseconds;
                                inanoseconds = (double) (date_time_Ptr->nanoseconds) / 1.0E9;
                                if(iseconds == 0.0 && inanoseconds == 0.0) secondiszero = PGS_TRUE;
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
                                strcpy(*outDatetime, date_time_name);
 
                                free(date_time_Ptr);
                                mdValueNode = NextValue(mdValueNode);
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
	return(PGS_S_SUCCESS);
}
