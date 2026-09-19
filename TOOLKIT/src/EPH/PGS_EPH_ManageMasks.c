
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_EPH_ManageMasks.c

DESCRIPTION:
   This file contains the function PGS_EPH_ManageMasks().
   This function is used to get and/or set the values of the ephemeris and
   attitude quality flags masks.
 
AUTHOR:
   Guru Tej S. Khalsa /	Applied Research Corp.

HISTORY:
   15-Aug-1996  GTSK  Initial version
   09-July-1999  SZ  Updated for the thread-safe functionality

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get/Set EPH Data Quality Flags Masks

NAME:
   PGS_TD_ManageMasks()

SYNOPSIS:
 C:
   #include <PGS_EPH.h>

   PGSt_SMF_status
   PGS_EPH_ManageMasks(
       PGSt_integer  command,
       PGSt_uinteger qualityFlagsMasks[2])

 FORTRAN:
      include 'PGS_SMF.f'
      include 'PGS_PC_9.f'

      integer function pgs_eph_managemasks(command,qualityflagsmasks)
      integer          command
      integer          qualityflagsmasks(2)

DESCRIPTION:
   This function is used to get and/or set the values of the ephemeris and
   attitude quality flags masks.

INPUTS:
   NAME               DESCRIPTION                       UNITS    MIN   MAX
   ----               -----------		        -----    ---   ---
   command            specifies action (get or set)      N/A     N/A   N/A
                      to be taken by this funcion
		      possible value: PGSd_SET,
		      PGSd_GET

   qualityFlagsMasks  ephemeris and attitude quality     N/A     N/A   N/A
                      flags masks

OUTPUTS:	  
   NAME               DESCRIPTION                       UNITS    MIN   MAX
   ----               -----------		        -----    ---   ---
   qualityFlagsMasks  ephemeris and attitude quality     N/A     N/A   N/A
                      flags masks

RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSPC_E_DATA_ACCESS_ERROR    error accessing Process Control File
   PGS_E_TOOLKIT                an unexpected error occurred
   PGSTSF_E_GENERAL_FAILURE     problem in the thread-safe code

EXAMPLES:
   N/A

NOTES:
   This function allows for user defined "masks" for the two data quality flags
   (ephemeris and attitude) associated with spacecraft ephemeris data.  The
   quality flags are (currently) four byte entities (maybe 8 bytes on the cray
   but only the first four bytes will be considered) that are interpreted bit by
   bit for meaning.  Currently the only "fatal" bit (i.e. indicating meaningless
   data) that will be set prior to access by the toolkit is bit 16 (where the
   least significant bit is bit 0).  Additionally the toolkit will set bit 12 of
   the quality flag returned for a given user input time if NO data is found for
   that input time.  Note that this usage is different from most of the other
   bits which indicate the state of some existing data point.  By default this
   function will set the mask for each of the quality flags to include bit 16
   (fatally flawed data) and bit 12 (no data).  This means that any data points
   returned from the tool PGS_EPH_EphemAttit() with an associated quality flag
   that has either bit 12 or bit 16 set will be rejected by any TOOLKIT function
   that makes a call to PGS_EPH_EphemAttit() (note that masking is not applied
   in the tool PGS_EPH_EphemAttit() itself since users calling this tool
   directly can examine the quality flags themselves and make their own
   determination as to which data points to use or reject).

   Users can use this tool or the Process Control File (PCF) to define their own
   masks which the toolkit will then use instead of the defaults mentioned
   above.  The user defined mask should contain set any bit which the user
   considers fatal for their purpose (e.g. red limit exceeded).
   WARNING: if the user defined mask does not have bit 16 set, the toolkit will
   pass through data the associated quality flag of which has bit 16 set.  The
   toolkit will not, however, process any data points if the associated quality
   flag has bit 12 set (i.e. no data exists) whether or not the user mask has
   bit 12 explicitly set.

REQUIREMENTS:  
   PGSTK - 0141, 0720, 0740

DETAILS:
   This function will attempt (on its first invocation) to initialize the values
   of the ephemeris data quality flag mask and the attitude data quality flag
   mask from values specified in the Process Control File (PCF).  If the
   first call to this function is a "set" (PGSd_SET) operation, the quality
   flags masks will immediately be set to the input values (i.e. ignoring the
   values found in the PCF or any errors in attempting to determine the values
   from the PCF).  Once initialized the values of the quality flags masks can
   then be accessed via the "get" (PGSd_GET) command or altered via the "set"
   command.

GLOBALS:
   PGSg_TSF_EPHfirst
   PGSg_TSF_EPHqFlagsMasks

FILES:
   None

FUNCTIONS_CALLED:
   PGS_PC_GetConfigData()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_SMF_TestErrorLevel()
   PGS_TSF_GetTSFMaster()
   PGS_TSF_GetMasterIndex()

END_PROLOG:
*******************************************************************************/

#include <PGS_PC.h>
#include <PGS_EPH.h>
#include <PGS_TSF.h>

/* Process Control File Logical IDs */

#define  PGSd_EPH_QFLAG_MASK  10507
#define  PGSd_ATT_QFLAG_MASK  10508

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_ManageMasks()"

PGSt_SMF_status
PGS_EPH_ManageMasks(                     /* get/set ephemeris/attitude quality
					    flags masks */
    PGSt_integer  command,               /* input command (PGSd_GET/PGSd_SET) */
    PGSt_integer  qualityFlagsMasks[2])  /* ephemeris/attitude quality flags
					    masks (output) */
{
    char                 buf[100];       /* temporary buffer */

    int                  check;          /* return value of sscanf() */

    PGSt_SMF_status      returnStatus;   /* return value of calls to Toolkit
					    functions */
    
#ifdef _PGS_THREADSAFE
    /* Create non-static variables and get globals for the thread-safe version */
    PGSt_uinteger qFlagsMasks[2];
    PGSt_boolean  first;
    int masterTSFIndex;
    int loopVar;
    extern PGSt_boolean PGSg_TSF_EPHfirst[];
    extern PGSt_uinteger PGSg_TSF_EPHqFlagsMasks[][2];

    /* Set up global index for the thread-safe */
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart for the thread-safe */      
    for (loopVar = 0; loopVar < 2; loopVar++)
    {
        qFlagsMasks[loopVar] = PGSg_TSF_EPHqFlagsMasks[masterTSFIndex][loopVar];
    }
    first = PGSg_TSF_EPHfirst[masterTSFIndex];
#else
    static PGSt_uinteger qFlagsMasks[2]; /* ephemeris/attitude quality flags
					    masks */
    
    static PGSt_boolean  first=PGS_TRUE; /* true on first entry only */
#endif


    /* initialize returnStatus to PGS_S_SUCCESS */

    returnStatus = PGS_S_SUCCESS;

    /* If this is the first time this function is called, attempt to initialize
       the static variable "qFlagsMasks" with the values of the ephemeris
       quality flags mask and the attitude quality flags mask defined in the
       Process Control File (PCF).  Note that if the input variable "command"
       has a value of PGSd_SET, the values of the quality flags masks thus
       determined (or any errors encountered in determining the values from the
       PCF) will be ignored and the variable "qFlagsMasks" will immediately be
       assigned the values of the variable "qualityFlagsMasks". */

    if (first == PGS_TRUE)
    {
	/* check to see if the ephemeris quality flag mask has been defined in
	   the PCF */

	returnStatus = PGS_PC_GetConfigData(PGSd_EPH_QFLAG_MASK, buf);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:

	    /* convert the ephemeris quality flag mask value found in the PCF
	       from a string to an unsigned integer (PGSt_uinteger) type */

	    check = sscanf(buf, "%u", qFlagsMasks);
	    if ((check != 1) && (command != PGSd_SET))
	    {
		PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				      "error interpreting ephemeris quality "
				      "flag parameter defined in process "
				      "control file (logical ID 10507)",
				      FUNCTION_NAME);
		return PGS_E_TOOLKIT;
	    }
	    break;

	  case PGSPC_W_NO_CONFIG_FOR_ID:

	    /* no ephemeris quality flag mask was set in the PCF, set a default
	       value (only reject meaningless or unavailable data points) */

	    qFlagsMasks[0] = (PGSt_integer)(PGSd_PLATFORM_FATAL | PGSd_NO_DATA);
#ifdef _PGS_THREADSAFE
            /* Reset global */
            PGSg_TSF_EPHqFlagsMasks[masterTSFIndex][0] = qFlagsMasks[0];
#endif 

            returnStatus = PGS_S_SUCCESS;
	    break;
	    
	  default:

	    /* some sort of error occurred while attempting to access the PCF
	       file (ignore if command is PGSd_SET, in which case it doesn't
	       matter that the qualityFlags could not be initialized here) */

	    if (command != PGSd_SET)
	    {
		return returnStatus;
	    }
	}
	
	/* check to see if the attitude quality flag mask has been defined in
	   the PCF */

	returnStatus = PGS_PC_GetConfigData(PGSd_ATT_QFLAG_MASK, buf);
	switch (returnStatus)
	{
	  case PGS_S_SUCCESS:

	    /* convert the attitude quality flag mask value found in the PCF
	       from a string to an unsigned integer (PGSt_uinteger) type */

	    check = sscanf(buf, "%u", qFlagsMasks+1);
	    if ((check != 1) && (command != PGSd_SET))
	    {
		PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT,
				      "error interpreting attitude quality flag"
				      " parameter defined in process control "
				      "file (logical ID 10508)", FUNCTION_NAME);
		return PGS_E_TOOLKIT;
	    }
	    break;

	  case PGSPC_W_NO_CONFIG_FOR_ID:

	    /* no attitude quality flag mask was set in the PCF, set a default
	       value (only reject meaningless or unavailable data points) */

	    qFlagsMasks[1] = (PGSt_integer)(PGSd_PLATFORM_FATAL | PGSd_NO_DATA);

#ifdef _PGS_THREADSAFE
            /* Reset Global */
            PGSg_TSF_EPHqFlagsMasks[masterTSFIndex][1] = qFlagsMasks[1];
#endif
            returnStatus = PGS_S_SUCCESS;
	    break;
	    
	  default:

	    /* some sort of error occurred while attempting to access the PCF
	       file (ignore if command is PGSd_SET, in which case it doesn't
	       matter that the qualityFlags could not be initialized here) */

	    if (command != PGSd_SET)
	    {
		return returnStatus;
	    }
	}

	first = PGS_FALSE;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_EPHfirst[masterTSFIndex] = first;
#endif

    }
    
    switch (command)
    {
      case PGSd_SET:

	/* set the static quality flags masks variables to the values of the
	   input quality flags masks variables */ 

	qFlagsMasks[0] = qualityFlagsMasks[0];
	qFlagsMasks[1] = qualityFlagsMasks[1];

#ifdef _PGS_THREADSAFE
        /* Reset global */
        for (loopVar = 0; loopVar < 2; loopVar++)
        {
            PGSg_TSF_EPHqFlagsMasks[masterTSFIndex][loopVar] = qFlagsMasks[loopVar];
        }
#endif
	break;
	
      case PGSd_GET:

	/* set the input quality flags masks variables to the values of the
	   static quality flags masks variables */ 

	qualityFlagsMasks[0] = qFlagsMasks[0];
	qualityFlagsMasks[1] = qFlagsMasks[1];
	break;
	
      default:

	/* an invalid value of the input variable "command" was passed in */

	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT, "invalid command switch input",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    return returnStatus;
}
