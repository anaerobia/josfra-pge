/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME: 
	PGS_CUC_Cons.c

DESCRIPTION:
	This file contains the toolkit routines and support functions to access
	constant values from a predetermined input file.

AUTHOR:
	Richard Morris /EOSL

HISTORY:
	12/05/94	RM	Initial Version
	13-July-95     ANS     Improved Fortran example
	28-Aug-96	ANS	Modified so that only doubles and
				integers are retrieved
        07-July-99      SZ      Updated for the thread-safe functionality

END_FILE_PROLOG:
*******************************************************************************/
/*---- include files ----*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_PC.h>
#include <PGS_CUC_11.h>
#include <PGS_TSF.h>

/*---- ODL includes ----*/
#include <PGS_CUC.h>

/*---- mnemonics ----*/

/*---- global variables ----*/

/*---- function declarations ----*/

/*******************************************************************************
BEGIN_PROLOG

TITLE:
	Obtain a value for a user specified constant.

NAME:
	PGS_CUC_Cons

SYNOPSIS:
C:
	#include <odldef.h>
	PGSt_SMF_Status
	PGS_CUC_Cons (	PGSt_integer inpfileid,
			char *inpParameter,
			PGSt_double *outvalue)

FORTRAN:
	#include 'PGS_CUC_11.f'

	integer function
	PGS_CUC_Cons ( inpfileid, inpParameter, outvalue)
			integer 	inpfileid,
			character  	inpParameter,
			double precision 	outvalue)
	
DESCRIPTION:
	This routine receives the fileid and the constant name from the user.
	The fileid allows more than one input file to be used, thus allowing
	the user to implement their own specialised input files with constants.
	The parameter is a character string representing the constant which
	the user is seeking the numerical value to.  The resulting value
	is passed back to the user as the result.

	[start]
	PERFORM	PGS_PC_GetPCSData
	PERFORM NewAggregate
	PERFORM ReadLabel
	PERFORM FindParameter
	PERFORM FirstValue
	[end]

INPUTS:
	Name		Description	Units	Min	Max
	----		-----------	-----	---	---
	fileid		file identifier	 Int	N/A	N/A
	
	parameter	Constant wanted	 N/A	N/A	N/A


OUTPUTS:
	Name		Description	Units	Min	Max
	----		-----------	-----	---	---
	value		constant value	 N/A	N/A	N/A


RETURNS:
	PGS_S_SUCCESS		successful return
	PGSCUC_E__ERROR		error in finding the constant value
        PGSTSF_E_GENERAL_FAILURE    problem in the thread-safe code

	The following are returned to the error log

	PGSCUC_E_CANT_GET_FILE_ID
	PGSCUC_E_CANT_OPEN_INPUT_FILE
	PGSCUC_E_NEW_AGG_CANNOT_BE_INSERTED
	PGSCUC_E_READLABEL_PARSE_ERROR
	PGSCUC_E_PARAMETER_INVALID
	PGSCUC_E_FIRST_NODE_NOT_FOUND

EXAMPLES:
C:
	char parameter[] = {"PI"};
	int  fileid	= 19701;
	double *result;
	
	ret_status = PGS_CUC_Cons(fileid, parameter, *result);

FORTRAN:
	implicit none
	integer pgs_cuc_cons
	integer	      	inpfileid
	character*100 	inpParameter
	real*8	outvalue
	inpfileid = 10790
	inpParameter = "pi"

	call pgs_cuc_cons(inpfileid, inpParameter, outvalue)


NOTES:
	User defines key word to be searched for within a logical file
	User also defines fileid so that location of file can be found.
	Tool uses ODL libraries to conduct a parameter equals value
	search.  For further information see CUC Tools Primer
	ODL documentation.

REQUIREMENTS:
	PGSTK - 1520, PGSTK - 1521, PGSTK - 1522, PGSTK - 1530

DETAILS:
	Constants accessed are those supplied by the Science Office.

GLOBALS:
	N/A

FILES:
	Tool accesses the Constants File supplied by the Science Office.  User
	may also define their own input file with personal constants.

FUNCTIONS CALLED:
	PGS_PC_GetPCSData()
	PGS_SMF_SetStaticMsg()
	NewAggregate()
	ReadLabel()
	FindParameter()
	FirstValue()
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

END_PROLOG:
*******************************************************************************/




AGGREGATE         cuc_agg;

PGSt_SMF_status
PGS_CUC_Cons(					/* Calling Sequence */
	         PGSt_integer  inpfileid, 	/* file identifier */
	    	 char *inpParameter,		/* input parameter */ 
	    	 PGSt_double *outvalue)		/* output value */
{
	PGSt_integer		status;
	FILE		*fp;
	PARAMETER 	cuc_par;
	VALUE 		cuc_val;
	/* char		*parameter; */
	char	 	filepath[255];
	/* PGSt_double 	*value; */
	/* char 		*file; */
	/* PGSt_integer 	fileid; */
	PGSt_integer	numfiles = 1;
	PGSt_SMF_status		ret_status;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;                 /* lock and unlock return */
#endif

/* First locate the input file from the fileid using the Process Control function */

	ret_status = PGS_PC_GetPCSData(PGSd_PC_INPUT_FILE_NAME,
				inpfileid,
				filepath,
				&numfiles);
	if (ret_status != PGS_S_SUCCESS)
	{
		PGS_SMF_SetStaticMsg (
				 PGSCUC_E_CANT_GET_FILE_ID,
				 "PGS_PC_GetPCSData");
		return PGSCUC_E__ERROR;
	}
	
		fp = fopen(filepath,"r");
		if (fp == NULL)
		{
			PGS_SMF_SetStaticMsg (
					 PGSCUC_E_CANT_OPEN_INPUT_FILE,
					 "PGS_CUC_Cons()" );
			return PGSCUC_E__ERROR;
		}


/* create aggregate */
/* base node(pointer to the node - becoming the parent of the node we are creating*/
/*, kind of node (object), name - pointer to a char string, class - may be null */

#ifdef _PGS_THREADSAFE
        /* We need to lock the non-threadsafe COTS (ODL) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
            fclose(fp);
            return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

		cuc_agg = NewAggregate(cuc_agg,KA_OBJECT,"TEST1","");
		if (cuc_agg == NULL)
		{
			PGS_SMF_SetStaticMsg (
					 PGSCUC_E_AGG_CANT_BE_INSERTED,
					 "PGS_CUC_Cons()" );

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                        fclose(fp);
			return PGSCUC_E__ERROR;
		}


/* parse CUC file */
/* inputs are : input_file = pointer to file containing label, */
/* output = value of one returned if parsing completed otherwise its a zero */ 
	

		status = ReadLabel(fp, cuc_agg);
		if (status != 1)
		{
			PGS_SMF_SetStaticMsg (
					 PGSCUC_E_READLABEL_PARSE_ERROR,
					 "PGS_CUC_Cons()" );
			cuc_agg = RemoveAggregate(cuc_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);        
#endif

                        fclose(fp);
			return PGSCUC_E__ERROR;
		}
                fclose(fp);


/* search for the correct parameter */
/* finds the first parameter of an object or group node with a specified name */
/* input = base_node ( pointer to the aggregate node), output = pointer to */
/* a parameter node with the specified name */


	cuc_par = FindParameter(cuc_agg, inpParameter);
	if (cuc_par == NULL)
	{
			PGS_SMF_SetStaticMsg (
					 PGSCUC_E_PARAMETER_INVALID,
					 "PGS_CUC_Cons()" );
			cuc_agg = RemoveAggregate(cuc_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

			return PGSCUC_E__ERROR;
	}

	/* get first value from the parameter */

	cuc_val = FirstValue(cuc_par);
	if (cuc_val == NULL)
	{
			PGS_SMF_SetStaticMsg (
					 PGSCUC_E_FIRST_NODE_NOT_FOUND,
					 "PGS_CUC_Cons()" );
			cuc_agg = RemoveAggregate(cuc_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

			return PGSCUC_E__ERROR;
	
	}

	if(cuc_val->item.type == TV_REAL)
	{
		*outvalue = (PGSt_double) cuc_val->item.value.real.number;
	}
	else if(cuc_val->item.type == TV_INTEGER)
	{
		*outvalue = (PGSt_double) cuc_val->item.value.integer.number;
	}
	else
	{
		PGS_SMF_SetStaticMsg (
                                         PGSCUC_E_PARAMETER_INVALID,
                                         "PGS_CUC_Cons()" );
			cuc_agg = RemoveAggregate(cuc_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSCUC_E__ERROR;
	}
	cuc_agg = RemoveAggregate(cuc_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

	return PGS_S_SUCCESS;
}
