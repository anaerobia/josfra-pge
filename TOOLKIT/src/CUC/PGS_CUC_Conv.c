/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/

/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME: 
	PGS_CUC_Conv.c

DESCRIPTION:
	This file contains the toolkit routines and support functions to access
	conversion slope and intercept values, needed to convert between units.

AUTHOR:
	Richard Morris /EOSL

HISTORY:
	14/05/94	RM	Initial Version
	13-July-95     ANS     Improved Fortran example
        08-July-99      SZ      Updated for the thread-safe functionality

END_FILE_PROLOG:
*******************************************************************************/

/*---- include files ----*/

#include <limits.h>	/* for _POSIX_MAX_INPUT */
#include <stddef.h>	/* for size_t */
#include <stdio.h>	/* for I/O functions, and NULL */
#include <string.h>	/* for strlen() & memmove() */
#include <errno.h>	/* for errno */
#include <ctype.h>	/* for isspace() */
#include <PGS_CUC.h>
#include <PGS_TSF.h>

/*---- udunits includes ----*/

/*---- mnemonics ----*/

/*---- global variables ----*/

/*---- function declarations ----*/


/*******************************************************************************
BEGIN_PROLOG

TITLE:
	Obtain the slope and intercept needed to calculate the conversion between
 	specified units.

NAME:
	PGS_CUC_Conv

SYNOPSIS:
C:
	#include <udunits.h>
	PGSt_SMF_Status
	PGS_CUC_Conv (					CallingSequence
			char inpUnit[], 		Unit you want to transform
			char outUnit[], 		Unit to be transformed to
			PGSt_double *outSlope,		Multiplication factor for inpUnit
			PGSt_double *outIntercept)	Factor to be added to multiplied 
							value.


FORTRAN:
	include 'PGS_CUC_11.f'
	
	integer function
	PGS_CUC_Conv(inpUnit, outUnit, outSlope, outIntercept)
			character*100	inpUnit,
			character*100	OutUnit,
			PGSt_double	OutSlope,
			PGSt_double	OutIntercept)
DESCRIPTION:
	This routine receives two character descriptions of Units as inputs.  The 
	first input is the unit which the user has, the second input being the
	unit the user wants to transform to.  Both Unit descriptions are held in
	a file, after a search identifies whether each unit is held within the file
	the slope and intercept of the conversion between units is calculated.
	The resulting values for slope and intercept are then passed back to the
	user.

	[start]
	PERFORM		utInit
	PERFORM		utScan
	PERFORM		utScan
	PERFORM		utConvert
	[end]

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	inpUnit		Unit you have		N/A	N/A	N/A

	OutUnit		Unit you want		N/A	N/A	N/A
	

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	OutSlope	mathematical slope	N/A	N/A	N/A

	OutIntercept	mathematical intercept	N/A	N/A	N/A


RETURNS:
	PGS_S_SUCCESS		successful retrn
	PGSCUC_E__ERROR		error in performing conversion match
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

	The following are returned to the error log

	PGSCUC_E_COULDNT_INIT_UDUNITS3
	PGSCUC_E_DONT_KNOW_INP_UNIT
	PGSCUC_E_DONT_KNOW_OUTP_UNIT
	PGSCUC_E_UNITS_ARE_INCOMPATIBLE
	PGSCUC_E_A_UNIT_IS_CORRUPTED

EXAMPLES:
C:
	char inpUnit[] = {"centigrade"};
	char outUnit[] = {"fahrenheit"};
	double *outSlope;
	double *outIntercept;

	ret_status = PGS_CUC_Conv(inpUnit[], OutUnit[], OutSlope, OutIntercept);

FORTRAN:
	implicit none
	integer pgs_cus_conv
	character*100	inpUnit
	character*100 	OutUnit
	double  	OutSlope
	double	OutIntercept
	inpUnit = "metres"
	OutUnit = 'feet'
	call pgs_cuc_conv(inpUnit, OutUnit, OutSlope, OutIntercept)


NOTES:
	For further details on this tool, see the CUC Tools Primer.
	Background on library units used, Units available for conversion and 
	adding own conversion Units to the file.
	The units available for conversion can be found in the Appendix of the User Guide.

	COPYRIGHT NOTICE for UDUNITS libraries

	(C) Copyright 1993 UCAR/Unidata
 
	Permission to use, copy, modify, and distribute this software and its
	documentation for any purpose without fee is hereby granted, provided
	that the above copyright notice appear in all copies, that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of UCAR/Unidata not be used in
	advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  UCAR makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.  It is
	provided with no support and without obligation on the part of UCAR or
	Unidata, to assist in its use, correction, modification, or enhancement.


REQUIREMENTS:
		PGSTK - 1520, PGSTK - 1521, PGSTK - 1522, PGSTK - 1530

DETAILS:
	N/A

GLOBALS:
	None

FILES:
	Udunits3 data file containing conversion will be provided with the tool,
	and loaded with the install script to a predetermined directory.

FUNCTIONS CALLED:
	PGS_SMF_SetStaticMsg()
	utInit()
	utScan()
	utConvert()
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

END_PROLOG:
*******************************************************************************/


/* name of this function */

#define FUNCTION_NAME "PGS_CUC_Conv()"

PGSt_SMF_status 
PGS_CUC_Conv(char inpUnit[50],			/* unit to be converted */
		char outUnit[50], 		/* unit to be converted to */
		PGSt_double *outSlope, 		/* resulting value of slope */
		PGSt_double *outIntercept)	/* resulting value of intercept */

{


   	utUnit		HaveUnit, WantUnit;	/* UNIT value */
   	static char	*UnitsPath = NULL;		/* dir of data file */
	static char     referenceID[PGSd_PC_FILE_PATH_MAX]="";
   	int		nparms = 1;		/* number of inputs */
   	int		i;			/* loop */
   	double		slope, intercept;	/* output values */
	PGSt_integer    version=1;
	PGSt_SMF_status		ret_status;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retLock;                /* lock and unlock return */
        PGSt_SMF_status retTSF;
#endif

/* initialize input data file */
#ifdef _PGS_THREADSAFE
        /* We need to lock all shared memory management */
        retLock = PGS_TSF_LockIt(PGSd_TSF_CUCLOCK);
        if (PGS_SMF_TestErrorLevel(retLock))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	if (referenceID[0] == '\0')
	{
	    UnitsPath = referenceID;
	    ret_status = PGS_PC_GetReference(PGSd_UDUNITS_DAT, &version,
					     referenceID);
	    if (ret_status != PGS_S_SUCCESS)
#ifdef PGS_DAAC
	    {
		PGS_SMF_SetDynamicMsg(PGSCUC_E__ERROR,
				      "Error getting reference to udunits.dat "
				      "file from PCF.",
				      FUNCTION_NAME);
#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_CUCLOCK);
#endif
		return PGSCUC_E__ERROR;
	    }
#else
	    {
		PGS_SMF_SetDynamicMsg(PGSCUC_E__ERROR,
				      "Error getting reference to udunits.dat "
				      "file from PCF.\n  NOTE: this error is "
				      "ignored in the SCF version of the "
				      "Toolkit.\nDefaulting to old (obsolete) "
				      "method of locating file.",
				      FUNCTION_NAME);
		UnitsPath = NULL;
		referenceID[0] = '1';
		referenceID[1] = '\0';
	    }
#endif
	}

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (UDUNITS) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKUDUNITS);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

	ret_status = utInit(UnitsPath);

#ifdef _PGS_THREADSAFE
        /* unlock all shared memory management */
        retLock = PGS_TSF_UnlockIt(PGSd_TSF_CUCLOCK);
#endif

	if ( ret_status != 0 )
	{
	PGS_SMF_SetStaticMsg (
			 PGSCUC_E_COULDNT_INIT_UDUNITS3,
			FUNCTION_NAME);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKUDUNITS);
#endif

	return PGSCUC_E__ERROR;
    	}
  
	else

	for(i = 0; i < nparms; i++) 
 	{

/* search data file for user specified input UNITS */

	
	if (utScan(inpUnit, &HaveUnit) !=0)
		{
		PGS_SMF_SetStaticMsg (
			 PGSCUC_E_DONT_KNOW_INP_UNIT,
			FUNCTION_NAME);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKUDUNITS);
#endif

		return PGSCUC_E__ERROR;
    		}

/* search data file for UNITS to convert to */

	
	
	if (utScan(outUnit, &WantUnit) !=0)
		{
		PGS_SMF_SetStaticMsg (
			 PGSCUC_E_DONT_KNOW_OUTP_UNIT,
			FUNCTION_NAME);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKUDUNITS);
#endif

		return PGSCUC_E__ERROR;
		}

/* Carry out conversion from one set of UNITS to the other */
    	


		 ret_status = utConvert(&HaveUnit, &WantUnit, 
			  &slope, &intercept);
		 if (ret_status == UT_ECONVERT) 
		{
		 PGS_SMF_SetStaticMsg (
			 PGSCUC_E_UNITS_ARE_INCOMPATIBLE,
			FUNCTION_NAME);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKUDUNITS);
#endif

		 return PGSCUC_E__ERROR;
		 }
		 else if (ret_status == UT_EINVALID) 
		{
		 PGS_SMF_SetStaticMsg (
			 PGSCUC_E_A_UNIT_IS_CORRUPTED,
			FUNCTION_NAME);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKUDUNITS);
#endif

		 return PGSCUC_E__ERROR;
		}
      	}
  
	
	*outSlope = slope;
	*outIntercept = intercept;

	PGS_SMF_SetStaticMsg(PGS_S_SUCCESS, FUNCTION_NAME);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKUDUNITS);
#endif

	return PGS_S_SUCCESS;
	
}
