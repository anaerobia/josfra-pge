/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/***************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:  
  	PGS_AA_PeV.c
 
DESCRIPTION:
  	This file contains the toolkit routines and support functions to access
  	parameter=value syntax data from file.
	There are a few restrictions in the parameter=value file syntax due to 
	problems in FREEFORM; there is no support for comments and blank lines 
	are not permitted.   

AUTHOR:
  	Jolyon Martin / EOSL

HISTORY:
  	07-July-94 	JM 	Initial version
  	20-July-94 	GJB 	Update for SMF
  	11-Aug -94 	JM  	Update for coding standards
	31-August-94    GJB     Post review updates
	24-October-94   GJB     Change from GetPCSDAta to GetReference to allow                                 searching throughout the PCF. table
	28-Jan-95       GJB     Add fortran interfaces and cfortran.h includes  
        23-Jun-95       ANS     Removed cfortran inclusion and binding
        06-July-99      SZ      Updated for thread-safe functionality

END_FILE_PROLOG:
***************************************************************************/
 
/*----- includes ------*/

#undef DEFINE_DATA
#include <freeform.h>
#include <databin.h>
#include <PGS_AA_Tools.h>
#include <PGS_TSF.h>

/* local function declaration */

PGSt_SMF_status
PGS_AA_PeV_get_dbin(
                    PGSt_uinteger PeV_Current_Logical,    /* input file logical */
                    DATA_BIN_PTR *PeV_Current_Dbin );       /* returned dbin pointer */

/*----   mnemonics -----*/

#define PGSd_AA_HeaderFormat "ASCII_input_file_header_separate header_format\ncreate_format_from_data_file 0 0 char 0\n"

/*----   global variables  ----*/
    char pevFileBuffer[PGSd_AA_PEVMAXBUFSIZ];       /* FREEFORM scratch buffer */
    char pevFileHeader[1024];                       /* FREEFORM header format */					

/***************************************************************************
BEGIN_PROLOG:

TITLE: 
	Extract string parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeV_string

SYNOPSIS:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status
	PGS_AA_PeV_string(
		  PGSt_uinteger pevLogical, 
		  char *parameter, 
		  char *value )

FORTRAN:
	include 'PGS_AA_10.f'
	include "PGS_AA.f"

	integer function pgs_aa_pev_string( pevLogical, parameter, value )
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

	PGSAA_E_GETDBIN			cant retreive FREEFORM data structure
					for given file
	PGSAA_E_CANT_GET_VALUE		cant retreive value associated with
					the given parameter
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:
	#include <PGS_AA.h>
	#define MAX_STRING 30

	PGSt_SMF_status retStatus;
	char myStringValue[MAX_STRING];
	
	ret_status = PGS_AA_PeV_string(MY_PEV_FILE, 
		  "MY_STRING_PARAMETER", myStringValue);

	if (ret_status != PGS_S_SUCCESS)
	{
		 signal ERROR 
	}

FORTRAN:
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	character*20 value
	pevLogical = 876
	parameter = "dataType"
	
	return = pgs_aa_pev_string( pevLogical, parameter, value )
	
	
NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is also a string as returned from the data file identifed by the logical.

REQUIREMENTS:
	PGSTK-1265 and 1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	pevFileBuffer			FREEFORM scratch buffer	

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()		set a known static message
	PGS_AA_PeV_get_dbin()		get FREEFORM dbin for given file
	nt_askvalue()			retreive value from FREEFORM dbin
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeV_string(
		  PGSt_uinteger pevLogical, 	/* input file logical */
		  char *parameter,		/* parameter name */
		  char *value )			/* returned value */
{
    
    /*--- local variables ---*/
    
    char *procname = "PGS_AA_PeV_string";	/* function name */
    PGSt_SMF_status pStatus;			/* PGS error return */
    DATA_BIN_PTR dbin; 				/* dbin declaration */
    int fStatus; 				/* FREEFORM status return */

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;       /* lock and unlock return */
#endif

    /*-----  start of processing code   ----*/

    /*
     * first retrieve the dbin pointer
     */
    
    pStatus = PGS_AA_PeV_get_dbin( pevLogical,  &dbin);
    if (pStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_GETDBIN,procname);
	return PGSAA_E_PEV_ERROR;
    }

    /*
     * use the FREEFORM library to extract the appropriate paraemter
     * from the dbin
     */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEFORM) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEFORM);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    fStatus = nt_askvalue(dbin, 
			  parameter, FFV_CHAR, 
			  value, pevFileBuffer);
    if (fStatus != TRUE)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANT_GET_VALUE,procname);
		
#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

	return PGSAA_E_PEV_ERROR;
    }
    
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

    return(PGS_S_SUCCESS);
}
/* fortran interface */
/***************************************************************************
BEGIN_PROLOG:

TITLE: 
	Extract real parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeV_real

SYNOPSIS:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status
	PGS_AA_PeV_real(
		  PGSt_uinteger pevLogical, 
		  char *parameter, 
		  PGSt_double *value )

FORTRAN:
        include 'PGS_AA_10.f'
	include "PGS_AA.f"
	integer function pgs_aa_pev_real( pevLogical, parameter, value )
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

	PGSAA_E_GETDBIN			cant retreive FREEFORM data structure
					for given file
	PGSAA_E_CANT_GET_VALUE		cant retreive value associated with
                    			the given parameter
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code
 
EXAMPLES:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status retStatus;
	PGSt_double myRealValue;
	
	ret_status = PGS_AA_PeV_real(MY_PEV_FILE, 
		  "MY_STRING_PARAMETER", &myRealValue);

	if (ret_status != PGS_S_SUCCESS)
	{
		 signal ERROR 
	}

FORTRAN:
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	double precision value
	pevLogical = 876
	parameter = "maxLat"
	
	return = pgs_aa_pev_real( pevLogical, parameter, value )	

NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is a real as returned from the data file identifed by the logical.

REQUIREMENTS:
	PGSTK-1265 and 1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	pevFileBuffer			FREEFORM scratch buffer	

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()		set a known static message
	PGS_AA_PeV_get_dbin()		get FREEFORM dbin for given file
	nt_askvalue()			retreive value from FREEFORM dbin
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeV_real(
		PGSt_uinteger pevLogical, 	/* input file logical */
		char *parameter,		/* parameter name */
		PGSt_double *value )		/* returned value */
{
    
    /*--- local variables ---*/
    
    char *procname = "PGS_AA_PeV_real";	/* function name */
    PGSt_SMF_status pStatus;			/* PGS error return */
    DATA_BIN_PTR dbin; 				/* dbin declaration */
    int fStatus; 				/* FREEFORM status return */

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;                 /* lock and unlock return */
#endif
    /*-----  start of processing code   ----*/

    /*
     * first retrieve the dbin pointer
     */
    
    pStatus = PGS_AA_PeV_get_dbin( pevLogical,  &dbin);
    if (pStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_GETDBIN,procname);
	return PGSAA_E_PEV_ERROR;
    }

    /*
     * use the FREEFORM library to extract the appropriate paraemter
     * from the dbin
     */
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEFORM) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEFORM);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    fStatus = nt_askvalue(dbin, 
			  parameter, FFV_DOUBLE, 
			  value, pevFileBuffer);
    if (fStatus != TRUE)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANT_GET_VALUE,procname);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

	return PGSAA_E_PEV_ERROR;
    }

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

    return(PGS_S_SUCCESS);
}
/* fortran interface */
    
/***************************************************************************
BEGIN_PROLOG:

TITLE: 
	Extract integer parameter from Parameter=Value formatted file
  
NAME:  
	PGS_AA_PeV_integer

SYNOPSIS:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status
	PGS_AA_PeV_string(
		  PGSt_uinteger pevLogical, 
		  char *parameter, 
		  char *value )

FORTRAN:
	include 'PGS_AA_10.f'
	include "PGS_AA.f"

	integer function pgs_aa_pev_integer( pevLogical, parameter, value )
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

	PGSAA_E_GETDBIN			cant retreive FREEFORM data structure
					for given file
	PGSAA_E_CANT_GET_VALUE		cant retreive value associated with
					the given parameter
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
C:
	#include <PGS_AA.h>

	PGSt_SMF_status retStatus;
	PGSt_integer myIntValue;
	
	ret_status = PGS_AA_PeV_integer(MY_PEV_FILE, 
		  "MY_STRING_PARAMETER", &myIntValue);

	if (ret_status != PGS_S_SUCCESS)
	{
		 signal ERROR 
	}

FORTRAN:
	include 'PGS_AA.f'
	include 'PGS_AA_10.f'
	integer pevLogical, return
	character*30 parameter
	integer value
	pevLogical = 876
	parameter = "size"
	
	return = pgs_aa_pev_real( pevLogical, parameter, value )	

NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	The parameter is a data set dependent character string and the value
	is an integer as returned from the data file identifed by the logical.

REQUIREMENTS:
	PGSTK-1265 and 1365

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	pevFileBuffer			FREEFORM scratch buffer	

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()		set a known static message
	PGS_AA_PeV_get_dbin()		get FREEFORM dbin for given file
	nt_askvalue()			retreive value from FREEFORM dbin
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()

END_PROLOG:
***************************************************************************/

PGSt_SMF_status
PGS_AA_PeV_integer(
		   PGSt_uinteger pevLogical, 	/* input file logical */
		   char *parameter,		/* parameter name */
		   PGSt_integer *value ) 	/* returned value */
{   
    
    /*--- local variables ---*/
    
    char *procname = "PGS_AA_PeV_integer";	/* function name */
    PGSt_SMF_status pStatus;			/* PGS error return */
    DATA_BIN_PTR dbin; 				/* dbin declaration */
    int fStatus; 				/* FREEFORM status return */
    long tempVal = 0;

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retTSF;                 /* lock and unlock return */
#endif

    /*-----  start of processing code   ----*/

    /*
     * first retrieve the dbin pointer
     */

    pStatus = PGS_AA_PeV_get_dbin( pevLogical,  &dbin);
    if (pStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_GETDBIN,procname);
	return PGSAA_E_PEV_ERROR;
    }

    /*
     * use the FREEFORM library to extract the appropriate paraemter
     * from the dbin
     */
#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEFORM) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEFORM);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

     fStatus = nt_askvalue(dbin, 
			  parameter, FFV_LONG, 
			  (void *)&tempVal, pevFileBuffer);
    if (!fStatus)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANT_GET_VALUE,procname);

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

	return PGSAA_E_PEV_ERROR;
    }
    *value = (PGSt_integer) tempVal;

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

    return(PGS_S_SUCCESS);
}
/* fortran interface */

/***************************************************************************
BEGIN_PROLOG:

  	This is a private routine used by the toolkit functions and therefore
  	not all the prolog fields are applicable to the routine and are marked
  	as N/A

TITLE:
  	Fill data bin header with file contents.
 
NAME:
  	PGS_AA_PeV_get_dbin

SYNOPSIS:
  	N/A

DESCRIPTION:
	This routine retreives or creates a FREEFORM dbin data bin structure
	for the given file.

INPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	pevLogical	file logical for file	see notes
			to be accessed
OUTPUTS:
	Name		Description		Units	Min	Max
	----		-----------		-----	---	---
	pevDbin		dbin pointer for file	see notes
			to be accessed

RETURNS:
	PGS_S_SUCCESS			successful return
	PGSAA_E_PEV_ERROR		error from called function

	The following errors may be reported to the error log

	PGSAA_E_PEV_XS_SUPPFILES	too many PeV files opened
	PGSAA_E_CANT_PARSE_FILE		cant retreive phys file from PC file
	PGSAA_E_FFDBIN			error in FREEFORM dbin creation
	PGSAA_E_FFDBSET			error in FREEFORM dbin set
        PGSTSF_E_GENERAL_FAILURE        problem in the thread-safe code

EXAMPLES:
	N/A

NOTES:
	The logical is an integer whose value is supplied through the PC tools.
	pevDbin is the data bin handle.

REQUIREMENTS:
	N/A

DETAILS:
	In support of the Process Control information the PGS_PC_INFO_FILE
	logical variable must be set

GLOBALS:
	pevFileBuffer			FREEFORM scratch buffer	
	pevFileHeader			FREEFORM header definition
        PGSg_TSF_AApevSavedLogicals        
        PGSg_TSF_AApevNFiles

FILES:
	This tool reads from the file defined in the environment
	variable "PGS_PC_INFO_FILE", and accesses the file specified
	by the input logical identifier.

FUNCTIONS_CALLED:
	PGS_SMF_SetStaticMsg()		set a known static message
	PGS_PC_GetReference()		query the process control data
	db_make()			FREEFORM dbin creation routine
	db_set()			FREEFORM dbin access routine
	nt_putvalue()			FREEFORM support routine
        PGS_TSF_LockIt()
        PGS_TSF_UnlockIt()
        PGS_SMF_TestErrorLevel()
        PGS_TSF_GetTSFMaster()
        PGS_TSF_GetMasterIndex()

END_PROLOG:
***************************************************************************/
PGSt_SMF_status 
PGS_AA_PeV_get_dbin(
	PGSt_uinteger pevLogical, 		/* input file logical */
	DATA_BIN_PTR *pevDbin )			/* returned dbin pointer */
{
    
    /*----   local variables  ----*/

    char pevPhysFileName[PGSd_AA_MAXNOCHARS];	/* physical filename */
    DATA_BIN_PTR dbin;				/* freeform databin */
    int fStatus;				/* status from freeform */
    PGSt_integer loop;				/* loop variable */
    PGSt_integer numFiles = 1;  		/* input/output value */
    PGSt_SMF_status pStatus;			/* status from toolkit */

#ifdef _PGS_THREADSAFE
        PGSt_SMF_status retVal;
        PGSt_SMF_status retTSF;                 /* lock and unlock return */
#endif    
    /*----  static variables  -----*/

    /* 
     * stored for the duration of the executable 
     * this information is hidden from the calling routines so a 
     * more optimal storage and retreival method may be implemented 
     */

#ifdef _PGS_THREADSAFE
    /* Create non-static variable and get globals for the thread-safe version */
    PGSt_uinteger pevSavedLogicals[PGSd_AA_PEVMAXFILES];
    DATA_BIN_PTR pevSavedDbins[PGSd_AA_PEVMAXFILES];
    PGSt_integer pevNFiles;
    int masterTSFIndex;
    int loopVar;
    extern PGSt_uinteger PGSg_TSF_AApevSavedLogicals[][PGSd_AA_PEVMAXFILES];
    extern DATA_BIN_PTR PGSg_TSF_AApevSavedDbins[][PGSd_AA_PEVMAXFILES];
    extern PGSt_integer PGSg_TSF_AApevNFiles[];
#else
						/* saved file logicals */    
    static PGSt_uinteger pevSavedLogicals[PGSd_AA_PEVMAXFILES];
    						/* saved file dbin */
    static DATA_BIN_PTR	pevSavedDbins[PGSd_AA_PEVMAXFILES];
    static PGSt_integer	pevNFiles = 0;		/* no of files opened */
#endif

#ifdef _PGS_THREADSAFE
    /* Set up global index, and TSF key keeper for the thread-safe */
    PGSt_TSF_MasterStruct *masterTSF;
    retVal = PGS_TSF_GetTSFMaster(&masterTSF);
    masterTSFIndex = PGS_TSF_GetMasterIndex();
    if (PGS_SMF_TestErrorLevel(retVal) || masterTSFIndex == PGSd_TSF_BAD_MASTER_INDEX)
    {
       return PGSTSF_E_GENERAL_FAILURE;
    }
    /*  Set from globals counterpart for the thread-safe */
    for (loopVar = 0; loopVar < PGSd_AA_PEVMAXFILES; loopVar++)
    {
        pevSavedLogicals[loopVar] = PGSg_TSF_AApevSavedLogicals[masterTSFIndex][loopVar];
        pevSavedDbins[loopVar] = PGSg_TSF_AApevSavedDbins[masterTSFIndex][loopVar];
    }

    pevNFiles = PGSg_TSF_AApevNFiles[masterTSFIndex];
#endif

    
    /*-----  start of processing code   ----*/
    
    /* 
     * loop through the current stored PeV files to see whether
     * the file has previously been opened, if so return the dbin
     */
    

    for (loop=0;loop<pevNFiles;loop++)
    {
	if (pevSavedLogicals[loop] == pevLogical)
	{
	    *pevDbin = pevSavedDbins[loop];

	    return(PGS_S_SUCCESS);
	}
    }

    /*
     * the execution of code has passed the loop, therefore
     * this is a new file, first check to see if the maximum
     * number of files has been exceeded
     */
    
    if (pevNFiles == PGSd_AA_PEVMAXFILES - 1)
    {
	
	PGS_SMF_SetStaticMsg (PGSAA_E_PEV_XS_SUPPFILES, "PGS_AA_PeV_get_dbin");

	return PGSAA_E_PEV_ERROR;    
    }

    /*
     * retreive the physical file name from the PC table
     */

    pStatus = PGS_PC_GetReference(
				  pevLogical, 
				  &numFiles,
				  pevPhysFileName);
    if (pStatus != PGS_S_SUCCESS)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_CANT_PARSE_FILE, "PGS_PC_GetReference");

	return PGSAA_E_PEV_ERROR;    
    }
    
    /* 
     * allocate and initialize freeform dbin 
     */

#ifdef _PGS_THREADSAFE
        /* We need to lock non-threadsafe COTS (FREEFORM) */
        retTSF = PGS_TSF_LockIt(PGSd_TSF_LOCKFREEFORM);
        if (PGS_SMF_TestErrorLevel(retTSF))
        {
           return PGSTSF_E_GENERAL_FAILURE;
        }
#endif

    dbin =  db_make(pevPhysFileName);
    if(dbin == NULL)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_FFDBIN, "db_make");

#ifdef _PGS_THREADSAFE
        /* We do not check for error since we already had
           an error that we want the user to know about */
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

	return PGSAA_E_PEV_ERROR;
    }
    
    /* 
     * use the header file name as the data file name, 
     * create a name table.  
     */
    
    fStatus = db_set(dbin,
		     BUFFER, pevFileBuffer,
		     DBIN_FILE_NAME, pevPhysFileName,
		     DBIN_FILE_HANDLE, (void *)NULL,
		     END_ARGS);
    if (fStatus)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_FFDBSET, pevPhysFileName);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

	return PGSAA_E_PEV_ERROR;
    }
    
    /* 
     * set the header file name in the name table so that DEFINE_HEADER
     * can get the header file name
     */
    
    /* nt_putvalue(dbin, "header_file_name", FFV_CHAR, pevPhysFileName, 0); */
    
    /* 
     * Create format and define the header so that the header 
     * can be retrieved by nt_askvalue.
     */
    
    sprintf(pevFileHeader, "%s %s\n%s\n%s\n%s\n%s\n%s\n%s %s\n%s\n",
	"ASCII_input_file_header_separate", "\"header format\"",
	"create_format_from_data_file 0 0 char 0",
	"binary_input_data \"data format\"",
	"data 1 1 uchar 0",
	"input_eqv",
	"begin constant",
	"header_file_name char", pevPhysFileName,
	"end constant");
	 
    fStatus = db_set(dbin, 
		     INPUT_FORMAT, (void *)NULL, pevFileHeader,
		     END_ARGS); 
    
    if (fStatus)
    {
	PGS_SMF_SetStaticMsg (PGSAA_E_FFDBSET, pevPhysFileName);

#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif

	return PGSAA_E_PEV_ERROR;
    }
    
#ifdef _PGS_THREADSAFE
        PGS_TSF_UnlockIt(PGSd_TSF_LOCKFREEFORM);
#endif
   
    /* 
     * store the dbin and increment the no. of files
     */
    
    pevSavedLogicals[pevNFiles] = pevLogical;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_AApevSavedLogicals[masterTSFIndex][pevNFiles] = pevSavedLogicals[pevNFiles];
#endif
    pevSavedDbins[pevNFiles] = dbin;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_AApevSavedDbins[masterTSFIndex][pevNFiles] = pevSavedDbins[pevNFiles];
#endif
    pevNFiles++;
#ifdef _PGS_THREADSAFE
        /* Reset global */
        PGSg_TSF_AApevNFiles[masterTSFIndex] = pevNFiles;
#endif

    *pevDbin = dbin;

    return(PGS_S_SUCCESS);
}

