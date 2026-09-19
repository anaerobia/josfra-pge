/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_ManageUTCF.c

DESCRIPTION:
   This file contains the function PGS_TD_ManageUTCF().
   This function is used to get and/or set the value of the TRMM Universal Time
   Correlation Factor (UTCF).
 
AUTHOR:
   Guru Tej S. Khalsa /	Applied Research Corp.

HISTORY:
   26-Apr-1996  GTSK  Initial version
   06-Jul-1999  RM    Updated for Thead-safe functionality

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get/Set TRMM UTCF value

NAME:
   PGS_TD_ManageUTCF()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ManageUTCF(
       PGSt_integer  command,
       PGSt_double   *utcf)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_manageutcf(command,utcf)
      integer          command
      double precision utcf

DESCRIPTION:
   This function is used to get and/or set the value of the TRMM Universal Time
   Correlation Factor (UTCF).

INPUTS:
   NAME      DESCRIPTION                       UNITS    MIN   MAX
   ----      -----------		       -----    ---   ---
   command   specifies action (get or set)      N/A     N/A   N/A
             to be taken by this function
             possible value: PGSd_SET,
	     PGSd_GET

   utcf      TRMM UTCF value                   seconds  ANY   ANY

OUTPUTS:	  
   NAME      DESCRIPTION                       UNITS    MIN   MAX
   ----      -----------		       -----    ---   ---
   utcf      TRMM UTCF value                   seconds  ANY   ANY

RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSTD_E_UTCF_UNINITIALIZED   attempted access (get) of uninitialized UTCF
                                value
   PGS_E_TOOLKIT                an unexpected error occurred
   PGSTSF_E_GENERAL_FAILURE     problem in TSF code

EXAMPLES:
   N/A

NOTES:
   UTCF is Universal Time Correlation Factor.  This is a value added to the TRMM
   s/c clock Mission Elapsed Time (MET) to get a value that represents
   Universal Coordinated Time (UTC).

   REFERENCES:
   TRMM-490-137, "Tropical Rainfall Measuring Mission (TRMM) Telemetry And
   Command Handbook", 1994-02-21, Goddard Space Flight Center, Appendix D

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   This function will attempt (on its first invocation) to initialize the value
   of UTCF from a value specified in the Process Control File (PCF).  If the
   first call to this function is a "set" (PGSd_SET) operation, the UTCF value
   will immediately be set to the input value (i.e. ignoring the value found in
   the PCF or any errors in attempting to determine the value from the PCF).
   Once initialized the value of the UTCF can then be accessed via the "get"
   (PGSd_GET) command or altered via the "set" command.

GLOBALS:
   PGSg_TSF_TDutcf

FILES:
   None

FUNCTIONS_CALLED:
   PGS_PC_GetConfigData()
   PGS_SMF_SetStaticMsg()
   PGS_SMF_SetDynamicMsg()
   PGS_TSF_GetMasterIndex()

END_PROLOG:
*******************************************************************************/

#include <PGS_PC.h>
#include <PGS_TD.h>
#include <PGS_TSF.h>

/* Process Control File Logical IDs */

#define PGSd_TRMM_UTCF   10123  /* logical ID of the UTCF value in the PCF */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_ManageUTCF()"

PGSt_SMF_status
PGS_TD_ManageUTCF(               /* get/set UTCF value */
    PGSt_integer  command,       /* input command (PGSd_GET/PGSd_SET) */
    PGSt_double   *utcf_in)      /* universal time correlation factor */
{
    char                 buf[100];       /* temporary buffer */

    int                  check;          /* return value of sscanf() */

    PGSt_SMF_status      returnStatus;   /* return value of calls to Toolkit
					    functions */
    
    static PGSt_boolean  first=PGS_TRUE; /* true on first entry only */

#ifdef _PGS_THREADSAFE
    /* set up initialization of static variable */
    double        utcf;                  /* universal time correlation factor */
    int           masterTSFIndex;
    extern double PGSg_TSF_TDutcf[];

    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        returnStatus = PGSTSF_E_GENERAL_FAILURE;
        return returnStatus;
    }

    /* set static variable from global counterpart */
    utcf = PGSg_TSF_TDutcf[masterTSFIndex];
#else
    static double        utcf;           /* universal time correlation factor */
#endif

    /* initialize returnStatus to PGS_S_SUCCESS */

    returnStatus = PGS_S_SUCCESS;

    /* If this is the first time this function is called, attempt to initialize
       the static variable "utcf" with the value of the UTCF defined in the
       Process Control File (PCF).  Note that if the input variable "command"
       has a value of PGSd_SET, the value of the UTCF thus determined (or any
       errors encountered in determining the UTCF from the PCF) will be ignored
       and the variable "utcf" will immediately be assigned the value of the
       variable "utcf_in". */

    if (first == PGS_TRUE)
    {
	/* get the value of the UTCF from the PCF */

	returnStatus = PGS_PC_GetConfigData(PGSd_TRMM_UTCF, buf);
	if (returnStatus != PGS_S_SUCCESS)
	{
	    returnStatus = PGSTD_E_UTCF_UNINITIALIZED;
	}
	else
	{
	    /* convert the UTCF value from a string to a floating type
	       (PGSt_double) number */

	    check = sscanf(buf, "%lf", &utcf);
	    if (check != 1)
	    {
		returnStatus = PGSTD_E_UTCF_UNINITIALIZED;
	    }
#ifdef _PGS_THREADSAFE
            else
            {
                /* re-set global counterpart */
                PGSg_TSF_TDutcf[masterTSFIndex] = utcf;
            }
#endif
	}

	/* at this point if the value of the UTCF could not be determined AND
	   the current function has not been called with the PGSd_SET command,
	   return an error status value indicating that the UTCF value could not
	   be initialized */

	if (returnStatus != PGS_S_SUCCESS && command != PGSd_SET)
	{
	    PGS_SMF_SetStaticMsg(returnStatus, FUNCTION_NAME);
	    return returnStatus;
	}

	first = PGS_FALSE;
    }
    
    switch (command)
    {
      case PGSd_SET:

	/* set the static UTCF variable to the input UTCF value */

	utcf = *utcf_in;
#ifdef _PGS_THREADSAFE
        /* re-set global counterpart */
        PGSg_TSF_TDutcf[masterTSFIndex] = utcf;
#endif
	break;
	
      case PGSd_GET:

	/* set the input UTCF to the current UTCF value */

	*utcf_in = utcf;
	break;
	
      default:

	/* an invalid value of the input variable "command" was passed in */

	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT, "invalid command switch input",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    return returnStatus;
}
