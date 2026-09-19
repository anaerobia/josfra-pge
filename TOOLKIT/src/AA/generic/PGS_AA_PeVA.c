/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	PGS_AA_PeVA.c
 
DESCRIPTION:
  	This file contains the toolkit routines and support functions to access
  	parameter=value syntax data from file.  

	IMPORTANT:  The input files for these calling sequences must obey the ODL
	syntax.  Otherwise the file will not be parsed into the ODL tree structure.
	

AUTHOR:
  	Richard Morris / EOSL

HISTORY:

	15-Mar-95	RSM	Changed PeV tool to function with ODL and return arrays
	11-July-95	ANS	Introduced routine to handle strings for fortran calls
	11-July-95      ANS     Improved fortran examples in the prologs
	12-July-95	ANS	Fixed bug ECSed00994
	12-July-95      ANS     Fixed bug ECSed00995
        06-July-99      SZ      Updated for the thread-safe functionality

END_FILE_PROLOG:
***************************************************************************/

/*---- include files ----*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <PGS_PC.h>
#include <PGS_AA.h>
#include <PGS_CUC_11.h>
#include <cfortran.h>
#include <PGS_TSF.h>

/*---- ODL includes ----*/
#include <CUC/odldef.h>

/*---- mnemonics ----*/

/*---- global variables ----*/

/*---- function declarations ----*/

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
	Extract string parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeVA_string

SYNOPSIS:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status
	PGS_AA_PeVA_string(
		  PGSt_uinteger pevLogical, 
		  char *parameter,
 		  char *value[] )

FORTRAN:
	include 'PGS_AA_10.f'
	include 'PGS_AA.f'

	integer function pgs_aa_peva_string( pevLogical, parameter, value )
	integer		pevLogical
	character*(*)	parameter
	character*(*)	value

DESCRIPTION:
	This routine returns the value associated with a string type 
	parameter from the given file.

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	pevLogical	file logical for file	see notes
			to be accessed
	parameter	name of parameter to	see notes
			be retrieved

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	value		value associated with	see notes
			retrieved parameter

RETURNS:
	PGS_S_SUCCESS			successful return
	PGSAA_E_PEV_ERROR		error in extracting the required value

	The following errors are reported to the error log

	PGSAA_E_CANT_GET_FILE_ID
        	PGSAA_E_CANT_OPEN_INPUT_FILE
       	PGSAA_E_AGG_CANT_BE_INSERTED
       	PGSAA_E_READLABEL_PARSE_ERROR
        	PGSAA_E_PARAMETER_INVALID
       	PGSAA_E_FIRST_NODE_NOT_FOUND
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:
	#include <PGS_AA.h>
	#define MAX_STRING 30

	PGSt_SMF_status retStatus;
	char *myStringValue[MAX_STRING]={	"                    ",
				              	"                    ",
					"	     "};
	
	ret_status = PGS_AA_PeVA_string(MY_PEV_FILE, 
		  "MY_STRING_PARAMETER", myStringValue);

	if (ret_status != PGS_S_SUCCESS)
	{
		 signal ERROR 
	}

FORTRAN:
	IMPLICIT NONE
	integer  pgs_aa_peva_string
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	character*20 value
	pevLogical = 876
	parameter = "dataType"
	
	return = pgs_aa_peva_string( pevLogical, parameter, value )
	
	
NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is also a string as returned from the data file identifed by the logical.
	The user must malloc enough space to pass back the array of strings,
	relating to the size of each string, and the number within the array.  See
	Example.

REQUIREMENTS:
	PGSTK-1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	None

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
        PGS_PC_GetPCSData()
        PGS_SMF_SetStaticMsg()
        NewAggregate()
        ReadLabel()
        FindParameter()
        FirstValue()
        NextValue()
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeVA_string(
		  PGSt_uinteger pevLogical, 	/* input file logical */
		  char *parameter,		/* parameter name */
		  char *value[] )		/* returned value */

{
    
    /*--- local variables ---*/

 	PGSt_integer        	status;
	AGGREGATE    	 	pev_agg = NULL;
       	PGSt_IO_Gen_FileHandle  *fp = NULL;
       	PARAMETER    		pev_par = NULL;
       	VALUE   		pev_val = NULL;
	PGSt_integer            count =0;
	PGSt_integer		version = 1;

	PGSt_SMF_status         ret_status;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

/* first locate the input file from the fileid using the Process Control Function */

	ret_status = PGS_IO_Gen_Open( pevLogical,
                                        PGSd_IO_Gen_Read,
                                        &fp,
                                        version);

        if (ret_status != PGS_S_SUCCESS)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_CANT_OPEN_INPUT_FILE,
                                         "PGS_AA_PeVA" );
                        return PGSAA_E_PEVA_ERROR;

                }

/* create aggregate */
/* base node(pointer to the node - becoming the parent of the node we are creating*/
/*, kind of node (object), name - pointer to a char string, class - may be null */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (ODL) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

                pev_agg = NewAggregate(pev_agg,KA_OBJECT,"PeVstring","");
                if (pev_agg == NULL)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_AGG_CANT_BE_INSERTED,
                                         "PGS_AA_PeVA" );
			(void)PGS_IO_Gen_Close(fp);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
                }


/* parse PeV file */
/* inputs are : input_file = pointer to file containing label, */
/* output = value of one returned if parsing completed otherwise its a zero */ 


                status = ReadLabel(fp, pev_agg);
		(void)PGS_IO_Gen_Close(fp);
                if (status != 1)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_READLABEL_PARSE_ERROR,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
                }

/* search for the correct parameter */
/* finds the first parameter of an object or group node with a specified name */
/* input = base_node ( pointer to the aggregate node), output = pointer to */
/* a parameter node with the specified name */


        pev_par = FindParameter(pev_agg, parameter);
        if (pev_par == NULL)
        {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_PARAMETER_INVALID,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
        }

        /* get first value from the parameter */

        pev_val = FirstValue(pev_par);
	if(pev_par == NULL)
        {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_FIRST_NODE_NOT_FOUND,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;

        }
	count = 0;
        while(pev_val != NULL)
        {
                (void)strcpy(value[count], pev_val->item.value.string);
                count++;
                pev_val = NextValue(pev_val);
        }
	RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

        return PGS_S_SUCCESS;     
}

/***************************************************************************
BEGIN_PROLOG:

TITLE: 

	Extract string parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeVA_stringF

SYNOPSIS:
C:
	Not applicable
FORTRAN:
	include 'PGS_AA_10.f'
	include 'PGS_AA.f'

	integer function pgs_aa_peva_string( pevLogical, parameter, value )
	integer		pevLogical
	character*(*)	parameter
	character*(*)	value

DESCRIPTION:
	This routine returns the value associated with a string type 
	parameter from the given file. It is especially written to 
	handle fortran calls

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	pevLogical	file logical for file	see notes
			to be accessed
	parameter	name of parameter to	see notes
			be retrieved

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	value		value associated with	see notes
			retrieved parameter

RETURNS:
	PGS_S_SUCCESS			successful return
	PGSAA_E_PEV_ERROR		error in extracting the required value

	The following errors are reported to the error log

	PGSAA_E_CANT_GET_FILE_ID
        	PGSAA_E_CANT_OPEN_INPUT_FILE
       	PGSAA_E_AGG_CANT_BE_INSERTED
       	PGSAA_E_READLABEL_PARSE_ERROR
        	PGSAA_E_PARAMETER_INVALID
       	PGSAA_E_FIRST_NODE_NOT_FOUND
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:
	Not applicable
FORTRAN:
	IMPLICIT NONE
	integer  pgs_aa_peva_string
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	character*20 value
	pevLogical = 876
	parameter = "dataType"
	
	return = pgs_aa_peva_string( pevLogical, parameter, value )
	
	
NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is also a string as returned from the data file identifed by the logical.
	The user must malloc enough space to pass back the array of strings,
	relating to the size of each string, and the number within the array.  See
	Example.

REQUIREMENTS:
	PGSTK-1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	None

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
        PGS_PC_GetPCSData()
        PGS_SMF_SetStaticMsg()
        NewAggregate()
        ReadLabel()
        FindParameter()
        FirstValue()
        NextValue()
        PGS_TSF_LockIt()  
        PGS_TSF_UnlockIt()   
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeVA_stringF(
		  PGSt_uinteger pevLogical, 	/* input file logical */
		  char *parameter,		/* parameter name */
		  void *value,			/* returned value */
		  unsigned int strLength)	/* size of string as defined in the 
						 * fortran routine
						 */

{
    
    /*--- local variables ---*/

 	PGSt_integer        	status;
	AGGREGATE    	 	pev_agg = NULL;
       	PGSt_IO_Gen_FileHandle  *fp = NULL;
       	PARAMETER    		pev_par = NULL;
       	VALUE   		pev_val = NULL;
	char * 			outString = NULL;
	char *                  blankPtr = NULL;
	char *                  valueString = NULL;
	PGSt_integer            charcterCount = 0;
	PGSt_integer            secLoopCount = 0;
	PGSt_integer		version = 1;

	PGSt_SMF_status         ret_status;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

/* first locate the input file from the fileid using the Process Control Function */

	ret_status = PGS_IO_Gen_Open( pevLogical,
                                        PGSd_IO_Gen_Read,
                                        &fp,
                                        version);
        if (ret_status != PGS_S_SUCCESS)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_CANT_OPEN_INPUT_FILE,
                                         "PGS_AA_PeVA" );
                        return PGSAA_E_PEVA_ERROR;

                }
/* create aggregate */
/* base node(pointer to the node - becoming the parent of the node we are creating*/
/*, kind of node (object), name - pointer to a char string, class - may be null */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (ODL) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

        pev_agg = NewAggregate(pev_agg,KA_OBJECT,"PeVstring","");
        if (pev_agg == NULL)
        {
                PGS_SMF_SetStaticMsg (
                                         PGSAA_E_AGG_CANT_BE_INSERTED,
                                         "PGS_AA_PeVA" );
		(void)PGS_IO_Gen_Close(fp);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                return PGSAA_E_PEVA_ERROR;
        }

/* parse PeV file */
/* inputs are : input_file = pointer to file containing label, */
/* output = value of one returned if parsing completed otherwise its a zero */ 


        status = ReadLabel(fp, pev_agg);
	(void)PGS_IO_Gen_Close(fp);
        if (status != 1)
        {
                PGS_SMF_SetStaticMsg (
                                     PGSAA_E_READLABEL_PARSE_ERROR,
                                     "PGS_AA_PeVA" );
		RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                return PGSAA_E_PEVA_ERROR;
        }

/* search for the correct parameter */
/* finds the first parameter of an object or group node with a specified name */
/* input = base_node ( pointer to the aggregate node), output = pointer to */
/* a parameter node with the specified name */

        pev_par = FindParameter(pev_agg, parameter);
        if (pev_par == NULL)
        {
                PGS_SMF_SetStaticMsg (
                                         PGSAA_E_PARAMETER_INVALID,
                                         "PGS_AA_PeVA" );
		RemoveAggregate(pev_agg); /* aggregate should be removed before exit */

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                return PGSAA_E_PEVA_ERROR;
        }

        /* get first value from the parameter */

        pev_val = FirstValue(pev_par);
	if(pev_val == NULL)
        {
                PGS_SMF_SetStaticMsg (
                                      PGSAA_E_FIRST_NODE_NOT_FOUND,
                                      "PGS_AA_PeVA" );
		RemoveAggregate(pev_agg); /* aggregate should be removed before exit */

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                return PGSAA_E_PEVA_ERROR;
        }
	outString = (char *) value;
        while(pev_val != NULL)
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
               valueString = pev_val->item.value.string;
               while(charcterCount != (strLength) &&
                     valueString[charcterCount] != '\0')
               {
                      outString[charcterCount] = valueString[charcterCount];
                      charcterCount++;
               }
               outString = outString + strLength; /* point to the next array string */
               pev_val = NextValue(pev_val);
        }
/* remove the aggregate */

	RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

        return PGS_S_SUCCESS;     
}

/* FORTRAN CALLING INTERFACE */
/*FCALLSCFUN3(INT, PGS_AA_PeVA_stringF, PGS_AA_PEVA_STRING, pgs_aa_peva_string,
   INT, PSTRING, STRVOID)*/  /* Moved to ../src/AA/DCW/PGS_AA_bindFORTRAN.c */

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
	Extract real parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeVA_real

SYNOPSIS:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status
	PGS_AA_PeVA_real(
		  PGSt_uinteger pevLogical, 
		  char *parameter, 
		  PGSt_double *value )

FORTRAN:
        include 'PGS_AA_10.f'
	include "PGS_AA.f"
	integer function pgs_aa_peva_real( pevLogical, parameter, value )
	integer			pevLogical
	character*(*)		parameter
	double precision	value

DESCRIPTION:
	This routine returns the value associated with a string type 
	parameter from the given file.

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	pevLogical	file logical for file	see notes
			to be accessed
	parameter	name of parameter to	see notes
			be retrieved

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	value		value associated with	see notes
			retrieved parameter

RETURNS:
	PGS_S_SUCCESS			successful return
	PGSAA_E_PEV_ERROR		error in extracting the required value

	The following errors are reported to the error log

        PGSAA_E_CANT_GET_FILE_ID
        PGSAA_E_CANT_OPEN_INPUT_FILE
        PGSAA_E_AGG_CANT_BE_INSERTED
        PGSAA_E_READLABEL_PARSE_ERROR
        PGSAA_E_PARAMETER_INVALID
        PGSAA_E_FIRST_NODE_NOT_FOUND
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status retStatus;
	PGSt_double myRealValue[MAX_SIZE_ARRAY];
	
	ret_status = PGS_AA_PeVA_real(MY_PEV_FILE, 
		  "MY_STRING_PARAMETER", &myRealValue);

	if (ret_status != PGS_S_SUCCESS)
	{
		 signal ERROR 
	}

FORTRAN:
	IMPLICIT NONE
	integer  pgs_aa_peva_real
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	double precision value
	pevLogical = 876
	parameter = "maxLat"
	
	return = pgs_aa_peva_real( pevLogical, parameter, value )	

NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is a real as returned from the data file identifed by the logical.

REQUIREMENTS:
	PGSTK-1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	None

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
        PGS_PC_GetPCSData()
        PGS_SMF_SetStaticMsg()
        NewAggregate()
        ReadLabel()
        FindParameter()
        FirstValue()
        NextValue()
        PGS_TSF_LockIt()  
        PGS_TSF_UnlockIt()   
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeVA_real(
		  PGSt_uinteger pevLogical, 	/* input file logical */
		  char *parameter,		/* parameter name */
		  PGSt_double *value )		/* returned value */

{
    
    /*--- local variables ---*/

  	PGSt_integer            status;
	AGGREGATE       	pev_agg = NULL;
        PGSt_IO_Gen_FileHandle  *fp = NULL;
        PARAMETER       	pev_par = NULL;
        VALUE    		pev_val = NULL;
	PGSt_integer		count = 0;
	PGSt_integer		version = 1;

	PGSt_SMF_status         ret_status;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

/* first locate the input file from the fileid using the Process Control Function */

	ret_status = PGS_IO_Gen_Open( pevLogical,
					PGSd_IO_Gen_Read,
					&fp,
					version);
        if (ret_status != PGS_S_SUCCESS)
                {
			PGS_SMF_SetStaticMsg (
                                         PGSAA_E_CANT_OPEN_INPUT_FILE,
                                         "PGS_AA_PeVA" );
                        return PGSAA_E_PEVA_ERROR;

                }

		
/* create aggregate */
/* base node(pointer to the node - becoming the parent of the node we are creating*/
/*, kind of node (object), name - pointer to a char string, class - may be null */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (ODL) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

                pev_agg = NewAggregate(pev_agg,KA_OBJECT,"PeVreal","");
                if (pev_agg == NULL)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_AGG_CANT_BE_INSERTED,
                                         "PGS_AA_PeVA" );
			(void) PGS_IO_Gen_Close(fp);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
                }


/* parse PeV file */
/* inputs are : input_file = pointer to file containing label, */
/* output = value of one returned if parsing completed otherwise its a zero */ 


                status = ReadLabel(fp, pev_agg);
		(void) PGS_IO_Gen_Close(fp);
                if (status != 1)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_READLABEL_PARSE_ERROR,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif
                        return PGSAA_E_PEVA_ERROR;
                }

/* search for the correct parameter */
/* finds the first parameter of an object or group node with a specified name */
/* input = base_node ( pointer to the aggregate node), output = pointer to */
/* a parameter node with the specified name */


        pev_par = FindParameter(pev_agg, parameter);
        if (pev_par == NULL)
        {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_PARAMETER_INVALID,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
        }

        /* get first value from the parameter */

        pev_val = FirstValue(pev_par);
	if (pev_val == NULL)
        {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_FIRST_NODE_NOT_FOUND,
                                         "PGS_AA_PeVA" );
                        RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
        }
 	count = 0;
        while(pev_val != NULL)
        {
                value[count] = pev_val->item.value.real.number;
                count++;
                pev_val = NextValue(pev_val);
        }

	RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

        return PGS_S_SUCCESS;
     
}


/* FORTRAN CALLING INTERFACE */
/*FCALLSCFUN3(INT, PGS_AA_PeVA_real, PGS_AA_PEVA_REAL, pgs_aa_peva_real, INT,
	    PSTRING, PDOUBLE) */ /* Moved to ../src/AA/DCW/PGS_AA_bindFORTRAN.c */

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
	Extract integer parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeVA_integer

SYNOPSIS:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status
	PGS_AA_PeVA_integer(
		  PGSt_uinteger pevLogical, 
		  char *parameter, 
		  PGSt_Integer *value )

FORTRAN:
	include 'PGS_AA_10.f'
	include 'PGS_AA.f'

	integer function pgs_aa_peva_integer( pevLogical, parameter, value )
	integer		pevLogical
	character*(*)	parameter
	integer		value


DESCRIPTION:
	This routine returns the value associated with a string type 
	parameter from the given file.

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	pevLogical	file logical for file	see notes
			to be accessed
	parameter	name of parameter to	see notes
			be retrieved

OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	value		value associated with	see notes
			retrieved parameter


RETURNS:
	PGS_S_SUCCESS			successful return
	PGSAA_E_PEV_ERROR		error in extracting the required value

	The following errors are reported to the error log

 
        PGSAA_E_CANT_GET_FILE_ID
        PGSAA_E_CANT_OPEN_INPUT_FILE
        PGSAA_E_AGG_CANT_BE_INSERTED
        PGSAA_E_READLABEL_PARSE_ERROR
        PGSAA_E_PARAMETER_INVALID
        PGSAA_E_FIRST_NODE_NOT_FOUND
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code 

EXAMPLES:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status retStatus;
	PGSt_integer myIntValue[MAX_SIZE_ARRAY];
	
	ret_status = PGS_AA_PeVA_integer(MY_PEV_FILE, 
		  "MY_STRING_PARAMETER", &myIntValue);

	if (ret_status != PGS_S_SUCCESS)
	{
		 signal ERROR 
	}


FORTRAN:
	IMPLICIT NONE
	integer	 pgs_aa_peva_real
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	integer value
	pevLogical = 876
	parameter = "size"
	
	return = pgs_aa_peva_real( pevLogical, parameter, value )	

NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is an integer as returned from the data file identifed by the logical.

REQUIREMENTS:
	PGSTK-1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
        NONE

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
        PGS_PC_GetPCSData()
        PGS_SMF_SetStaticMsg()
        NewAggregate()
        ReadLabel()
        FindParameter()
        FirstValue()
        NextValue()
        PGS_TSF_LockIt()  
        PGS_TSF_UnlockIt()   
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeVA_integer(
		   PGSt_uinteger pevLogical, 	/* input file logical */
		   char *parameter,		/* parameter name */
		   PGSt_integer *value ) 	/* returned value */


{
    
   /*--- local variables ---*/

  	PGSt_integer            status;
	AGGREGATE       	pev_agg = NULL;
        PGSt_IO_Gen_FileHandle  *fp = NULL;
        PARAMETER       	pev_par = NULL;
        VALUE    		pev_val = NULL;
	PGSt_integer		count = 0;
	PGSt_integer    	version = 1;

	PGSt_SMF_status         ret_status;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

/* first locate the input file from the fileid using the Process Control Function */

	ret_status = PGS_IO_Gen_Open( pevLogical,
                                        PGSd_IO_Gen_Read,
                                        &fp,
                                        version);
        if (ret_status != PGS_S_SUCCESS)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_CANT_OPEN_INPUT_FILE,
                                         "PGS_AA_PeVA" );
                        return PGSAA_E_PEVA_ERROR;

                }

/* create aggregate */
/* base node(pointer to the node - becoming the parent of the node we are creating*/
/*, kind of node (object), name - pointer to a char string, class - may be null */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (ODL) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKODL);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

                pev_agg = NewAggregate(pev_agg,KA_OBJECT,"PeVstring","");
                if (pev_agg == NULL)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_AGG_CANT_BE_INSERTED,
                                         "PGS_AA_PeVA" );
			(void)PGS_IO_Gen_Close(fp);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
                }


/* parse PeV file */
/* inputs are : input_file = pointer to file containing label, */
/* output = value of one returned if parsing completed otherwise its a zero */ 


                status = ReadLabel(fp, pev_agg);
		(void)PGS_IO_Gen_Close(fp);
                if (status != 1)
                {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_READLABEL_PARSE_ERROR,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
                }

/* search for the correct parameter */
/* finds the first parameter of an object or group node with a specified name */
/* input = base_node ( pointer to the aggregate node), output = pointer to */
/* a parameter node with the specified name */


        pev_par = FindParameter(pev_agg, parameter);
        if (pev_par == NULL)
        {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_PARAMETER_INVALID,
                                         "PGS_AA_PeVA" );
			RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;
        }

        /* get first value from the parameter */

        pev_val = FirstValue(pev_par);
	if (pev_val == NULL)
        {
                        PGS_SMF_SetStaticMsg (
                                         PGSAA_E_FIRST_NODE_NOT_FOUND,
                                         "PGS_AA_PeVA" );
                        RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

                        return PGSAA_E_PEVA_ERROR;

        }
	count = 0;
        while(pev_val != NULL)
        {
                value[count] = pev_val->item.value.integer.number;
                count++;
                pev_val = NextValue(pev_val);
        }

	RemoveAggregate(pev_agg);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKODL);
#endif

        return PGS_S_SUCCESS;
     
     
}

/* FORTRAN CALLING INTERFACE */
/*FCALLSCFUN3(INT, PGS_AA_PeVA_integer, PGS_AA_PeVA_INTEGER,
  pgs_aa_peva_integer, INT, PSTRING, PINT)*/ /* Moved to ../src/AA/DCW/PGS_AA_bindFORTRAN.c */
