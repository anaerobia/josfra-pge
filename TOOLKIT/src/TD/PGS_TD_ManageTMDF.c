/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:   
   PGS_TD_ManageTMDF.c

DESCRIPTION:
   This file contains the function PGS_TD_ManageTMDF().
   This function is used to get and/or set the ADEOS-II Time Difference (TMDF)
   values.
 
AUTHOR:
   Guru Tej S. Khalsa /	Applied Research Corp.
   Ray Milburn / Steven Myers & Associates

HISTORY:
   26-Apr-1996  GTSK  Initial version
   02-Jul-1999  RM    Added thread-safe code

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Get/Set TRMM TMDF value

NAME:
   PGS_TD_ManageTMDF()

SYNOPSIS:
 C:
   #include <PGS_TD.h>

   PGSt_SMF_status
   PGS_TD_ManageTMDF(
       PGSt_integer  command,
       PGSt_double   *period,
       PGSt_double   *sc_ref,
       PGSt_double   *grnd_ref)

 FORTRAN:
      include 'PGS_TD_3.f'
      include 'PGS_SMF.f'

      integer function pgs_td_managetmdf(command,period,sc_ref,grnd_ref)
      integer          command
      double precision period
      double precision sc_ref
      double precision grnd_ref


DESCRIPTION:
   This function is used to get and/or set the ADEOS-II Time Difference (TMDF)
   values.

INPUTS:
   NAME      DESCRIPTION
   ----      -----------
   command   specifies action (get or set)
             to be taken by this function
             possible value: PGSd_SET,
	     PGSd_GET

   period    period or the s/c clock

   sc_ref    s/c clock reference value

   grnd_ref  ground clock reference value

OUTPUTS:	  
   NAME      DESCRIPTION
   ----      -----------
   period    period or the s/c clock

   sc_ref    s/c clock reference value

   grnd_ref  ground clock reference value

RETURNS:
   PGS_S_SUCCESS                successful execution
   PGSTD_E_TMDF_UNINITIALIZED   attempted access (get) of uninitialized TMDF
                                values
   PGS_E_TOOLKIT                an unexpected error occurred
   PGSTSF_E_GENERAL_FAILURE     error getting master index

EXAMPLES:
   N/A

NOTES:
   This function is a preliminary implementation of the code required to
   interpret the s/c clock time of the ADEOS-II L0 packet secondary headers.
   The final specifications for the s/c clock time and the TMDF values is not
   currently available (5/15/96).

REQUIREMENTS:  
   PGSTK-1160, PGSTK-1170

DETAILS:
   This function will attempt (on its first invocation) to initialize the values
   of TMDF from values specified in the Process Control File (PCF).  If the
   first call to this function is a "set" (PGSd_SET) operation, the TMDF values
   will immediately be set to the input values (i.e. ignoring the values found
   in the PCF or any errors in attempting to determine the values from the PCF).
   Once initialized the values of the TMDF can then be accessed via the "get"
   (PGSd_GET) command or altered via the "set" command.

GLOBALS:
   PGSg_TSF_TDPeriod
   PGSg_TSF_TDSCReference
   PGSg_TSF_TDGroundReference

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

/* Process Control File Logical IDs for ADEOS-II TMDF values */

#define PGSd_ADEOS_II_SC_REF        10120  /* s/c clock reference value */
#define PGSd_ADEOS_II_GRND_REF      10121  /* ground reference value */
#define PGSd_ADEOS_II_CLOCK_PERIOD  10122  /* s/c clock period */

/* name of this function */

#define FUNCTION_NAME "PGS_TD_ManageTMDF()"

PGSt_SMF_status
PGS_TD_ManageTMDF(               /* get/set TMDF values */
    PGSt_integer  command,       /* input command (get/set) */
    PGSt_double   *period_in,    /* s/c clock period */
    PGSt_double   *sc_ref_in,    /* s/c clock reference value */
    PGSt_double   *grnd_ref_in)  /* ground reference value */
{
    char                 buf[100];           /* temporary buffer */

    int                  check;              /* return value of sscanf() */

    PGSt_SMF_status      returnStatus;       /* return value of this function */
    PGSt_SMF_status      returnStatus1;      /* return value of calls to Toolkit
						functions */
    
    static PGSt_boolean  first=PGS_TRUE;     /* true on first entry only */

#ifdef _PGS_THREADSAFE
    double        period;                    /* s/c clock period */
    double        sc_reference;              /* s/c clock reference value */
    double        ground_reference;          /* ground reference value */
    int           masterTSFIndex;
    extern double PGSg_TSF_TDPeriod[];
    extern double PGSg_TSF_TDSCReference[];
    extern double PGSg_TSF_TDGroundReference[];
 
    /* get the master TSF index value */
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
        return PGSTSF_E_GENERAL_FAILURE;
    }

    /* set variables from their global counterparts */
    period = PGSg_TSF_TDPeriod[masterTSFIndex];
    sc_reference = PGSg_TSF_TDSCReference[masterTSFIndex];
    ground_reference = PGSg_TSF_TDGroundReference[masterTSFIndex];
#else
    static double        period;             /* s/c clock period */
    static double        sc_reference;       /* s/c clock reference value */
    static double        ground_reference;   /* ground reference value */
#endif
    
    returnStatus = PGS_S_SUCCESS;
	
    /* If this is the first time this function is called, attempt to initialize
       the static variables "period", "sc_reference", and "ground_reference"
       with their respective values as defined in the Process Control File
       (PCF).  Note that if the input variable "command" has a value of
       PGSd_SET, the values of the TMDF variables thus determined (or any errors
       encountered in determining the TMDF values from the PCF) will be ignored
       and the static TMDF variables will immediately be assigned the value of
       the the input TMDF variables. */

    if (first == PGS_TRUE)
    {
	/* get the value of the s/c reference time from the PCF */

	returnStatus1 = PGS_PC_GetConfigData(PGSd_ADEOS_II_SC_REF, buf);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    returnStatus = PGSTD_E_TMDF_UNINITIALIZED;
	}
	else
	{
	    check = sscanf(buf, "%lf", &sc_reference);
	    if (check != 1)
	    {
		returnStatus = PGSTD_E_TMDF_UNINITIALIZED;
	    }
#ifdef _PGS_THREADSAFE
            else
            {
                /* re-set the global counterpart */
                PGSg_TSF_TDSCReference[masterTSFIndex] = sc_reference;
            }
#endif
	}

	/* get the value of the ground reference time from the PCF */

	/* The ground reference value is here implemented as secTAI93 but
	   this is only prototype code, the actual format is not yet known */

	returnStatus1 = PGS_PC_GetConfigData(PGSd_ADEOS_II_GRND_REF, buf);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    returnStatus = PGSTD_E_TMDF_UNINITIALIZED;
	}
	else
	{
	    check = sscanf(buf, "%lf", &ground_reference);
	    if (check != 1)
	    {
		returnStatus = PGSTD_E_TMDF_UNINITIALIZED;
	    }
#ifdef _PGS_THREADSAFE
            else
            {
                /* re-set the global counterpart */
                PGSg_TSF_TDGroundReference[masterTSFIndex] = ground_reference;
            }
#endif
	}

	/* get the value of the s/c clock period from the PCF */

	returnStatus1 = PGS_PC_GetConfigData(PGSd_ADEOS_II_CLOCK_PERIOD, buf);
	if (returnStatus1 != PGS_S_SUCCESS)
	{
	    returnStatus = PGSTD_E_TMDF_UNINITIALIZED;
	}
	else
	{
	    check = sscanf(buf, "%lf", &period);
	    if (check != 1)
	    {
		returnStatus = PGSTD_E_TMDF_UNINITIALIZED;
	    }
#ifdef _PGS_THREADSAFE
            else
            {
                /* re-set the global counterpart */
                PGSg_TSF_TDPeriod[masterTSFIndex] = period;
            }
#endif
	}

	/* at this point if any value of the TMDF could not be determined AND
	   the current function has not been called with the PGSd_SET command,
	   return an error status value indicating that the TMDF values could
	   not be initialized */

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

	/* set the static TMDF variables to the input TMDF values */

	period = *period_in;
	sc_reference = *sc_ref_in;
	ground_reference = *grnd_ref_in;
#ifdef _PGS_THREADSAFE
        /* re-set the global counterparts */
        PGSg_TSF_TDPeriod[masterTSFIndex] = period;
        PGSg_TSF_TDSCReference[masterTSFIndex] = sc_reference;
        PGSg_TSF_TDGroundReference[masterTSFIndex] = ground_reference;
#endif
	break;
	
      case PGSd_GET:

	/* set the input TMDF variables to the current TMDF values */

	*period_in = period;
	*sc_ref_in = sc_reference;
	*grnd_ref_in = ground_reference;
	break;
	
      default:

	/* an invalid value of the input variable "command" was passed in */

	PGS_SMF_SetDynamicMsg(PGS_E_TOOLKIT, "invalid command switch input",
			      FUNCTION_NAME);
	return PGS_E_TOOLKIT;
    }

    return returnStatus;
}
