/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_LoggingControl.c

DESCRIPTION:
   This file contains the function PGS_SMF_LoggingControl().
   This function controls the logging of specific SMF messages (or catagories of
   SMF messages).

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   10-Jan-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Control Logging of Specific SMF Messages

NAME:
   PGS_SMF_LoggingControl()

SYNOPSIS:
C:
   #include <PGS_SMF.h>

   PGSt_SMF_status
   PGS_SMF_LoggingControl(
       PGSt_integer    cmd,
       PGSt_SMF_code   input)

FORTRAN:		  
   N/A

DESCRIPTION:
   This function is for internal toolkit usage.  It controls the logging of
   specific SMF messages (or catagories of SMF messages).  This function can
   be used to enable/disable the logging of all SMF messages with a specific
   status level (i.e. M, S, W, E, etc.), the logging of all SMF messages derived
   from a specific seed value, or a single specific SMF code.

INPUTS:
   Name         Description
   ----         -----------
   cmd          input command, determines what action this function should take
                possible values are:

                   PGSd_QUERY
		   determine the logging status of the SMF code passed in (via
	           the variable "input"), i.e. whether or not the SMF code
	           should be logged to the disk log file

                   PGSd_QUERY_ALL
		   determine if logging has been disabled for all SMF log files

		   PGSd_DISABLE_ALL
		   disable all logging to SMF log files

		   PGSd_DISABLE_STATUS_LEVEL
		   disable logging for the status level passed in (via the
        	   variable "input") (the status level is a numerical value
        	   corresponding to the SMF status levels, e.g. M, S, W, E,
        	   etc.)

                   PGSd_DISABLE_SEED
	           disable logging for SMF codes derived from the seed value
		   passed in (via the variable "input")

                   PGSd_DISABLE_CODE
		   disable logging for the specific SMF code passed in (via the
		   variable "input")

		   PGSd_ENABLE_STATUS_LEVEL
		   nullify last request to disable logging for the status level
		   passed in (via the variable "input") (the status level is a
		   numerical value corresponding to the SMF status levels,
		   e.g. M, S, W, E, etc.)

		   PGSd_ENABLE_ALL
		   nullify last request to disable all logging to SMF log files,
		   note that this command does NOT enable logging for status
		   messages for which logging has been selectively disabled,
		   such messages must be enabled by the approriate enable
		   command (that nullifies whatever disable command was used to
		   disable their logging in the first place)

                   PGSd_ENABLE_SEED
	           nullify last request to disable logging for SMF codes derived
		   from the seed value passed in (via the variable "input")

                   PGSd_ENABLE_CODE
		   nullify last request to disable logging for the specific SMF
		   code passed in (via the variable "input")

		   PGSd_FLUSH
		   clear all lists of disabled SMF codes, allow logging of all
		   SMF codes (used to free dynamic memory previously allocated)

   input        SMF status code, seed or status level (as appropriate) to
                perform the requested command on (see "cmd" above).  This
		variable is ignored if the requested command is one of:
		PGSd_ENABLE_ALL, PGSd_DISABLE_ALL, PGSd_FLUSH.

OUTPUTS:
   None
          
RETURNS:
   PGS_S_SUCCESS               successful return 
   PGS_E_TOOLKIT               could not perform requested command
   PGSSMF_M_LOGGING_ENABLED    logging is enabled for input SMF code
   PGSSMF_M_LOGGING_DISABLED   logging is disabled for input SMF code

EXAMPLES:
C:

FORTRAN:
  
NOTES:
   This function is designed to allow for recursive usage.  That is, if a high
   level function turns off logging and then calls a low level function which in
   turn turns off logging and then turns logging back on these events will be
   tracked seperately.  Logging will not actually be enabled until the high
   level function turns logging back on.  Requests to enable logging for
   messages the logging of which have not previously been disabled will be
   ignored.

REQUIREMENTS:
   PGSTK - ????, ????

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   PGS_SMF_ManageLogControlList()

END_PROLOG:
*******************************************************************************/

#include <PGS_SMF.h>

PGSt_SMF_status
PGS_SMF_LoggingControl(     /* control logging of messages to disk log files */
    PGSt_integer    cmd,    /* input command (i.e. what to do) */
    PGSt_SMF_code   input)  /* input quantity (SMF level, seed or code) */
{
    static PGSt_SMF_status *code_list=NULL;     /* pointer to list of specific
						   SMF codes that should NOT be
						   logged */
    static PGSt_SMF_status *seed_list=NULL;     /* pointer to list of SMF seeds
						   for which the derived codes
						   should NOT be logged */
    static PGSt_SMF_status *level_list=NULL;    /* pointer to list of SMF status
						   levels which should NOT be
						   logged */
    
    static size_t          code_list_length=0;  /* length (no. of elements) of
						   code_list */
    static size_t          seed_list_length=0;  /* length (no. of elements) of
						   seed_list */
    static size_t          level_list_length=0; /* length (no. of elements) of
						   level_list */
    
    static PGSt_boolean    all_empty=PGS_TRUE;  /* T/F value used to as sort of
						   short cut.  This value is
						   true if ALL the lists have
						   zero elements. */

    static PGSt_integer    logging=PGSd_ON;     /* keeps track of whether ALL
						   SMF status code (as distinct
						   from particular status codes
						   or types of status codes)
						   logging has been disabled */

    static unsigned int    nest_level=0;        /* keeps track of the number of
						   consecutive times requests to
						   disable logging have been
						   made (i.e. without a
						   corresponding request to
						   enable logging) */

    size_t                 *length;             /* temp. variable, pointer to
						   the appropriate list length
						   (see above "length" vars.) */
    size_t                 index;               /* looping variable */
    
    PGSt_SMF_status        **list;              /* temp. variable, pointer to
						   pointer to the appropriate
						   list (see "list" vars.) */
    PGSt_SMF_status        level;               /* SMF status level of an input
						   SMF code */
    PGSt_SMF_status        seed;                /* SMF seed of an input SMF
						   code */
    PGSt_SMF_status        operation;           /* used to specify the operation
						   to be performed in call to
						   list manager function */
    PGSt_SMF_status        returnStatus;        /* return status of function
						   calls made herein */
    
    /* determine which command has been input, perform the specified action */

    switch (cmd)
    {
      case PGSd_QUERY:

	/* In this case "input" is expected to be an SMF status code.  This
	   function will return PGSSMF_M_LOGGING_ENABLED if the logging has not
	   been disabled for the input status code, otherwise it will return
	   PGSSMF_M_LOGGING_DISABLED.  Logging for a status code is disabled if
	   logging has been disabled for:
	   
	   1) all codes
	   2) the status level of the input code (e.g. N, S, W, E)
	   3) the seed from which the status code was derived (this allows,
	      e.g., logging of all messages from a specific toolkit group to be
	      disabled)
	   4) the specific status code itself
	   */

	/* check if ALL logging has been disabled */

	if (logging == PGSd_OFF)
	{
	    return PGSSMF_M_LOGGING_DISABLED;
	}
	
	/* quick check to see if NO special requests to disable particular
	   status codes or groups of status codes have been made */

	if (all_empty == PGS_TRUE)
	{
	    return PGSSMF_M_LOGGING_ENABLED;
	}
	
	/* check the level list first, it can't be very long */

	level =  input & PGS_SMF_MASK_LEVEL;
	for (index=0; index<level_list_length; index++)
	{
	    if (level == level_list[index])
	    {
		return PGSSMF_M_LOGGING_DISABLED;
	    }
	}

	/* check seed next */

	seed = input >> 13U;
	for (index=0; index<seed_list_length; index++)
	{
	    if (seed == seed_list[index])
	    {
		return PGSSMF_M_LOGGING_DISABLED;
	    }
	}

	/* finally check for status code itself, this is potentially the longest
	   list */

	for (index=0; index<code_list_length; index++)
	{
	    if (input == code_list[index])
	    {
		return PGSSMF_M_LOGGING_DISABLED;
	    }
	}

	return PGSSMF_M_LOGGING_ENABLED;
	
	
      case PGSd_QUERY_ALL:

	/* check if ALL logging has been disabled */

	if (logging == PGSd_OFF)
	{
	    return PGSSMF_M_LOGGING_DISABLED;
	}

	return PGSSMF_M_LOGGING_ENABLED;
	
      case PGSd_DISABLE_ALL:

	/* disable all logging of messages to disk log files */

	nest_level++;
	logging = PGSd_OFF;
	return PGS_S_SUCCESS;

      case PGSd_DISABLE_STATUS_LEVEL:

	/* In this case "input" is expected to be the value of an SMF status
	   level (see PGS_SMF.h for hexidecimal definitions of these values).
	   Logging will be disabled for all SMF status codes whose levels
	   (e.g. M, N, W, etc.) correspond to the level defined by "input". */

	list = &level_list;
	length = &level_list_length;
	operation = PGSd_ADD_ELEMENT;
	break;
	
      case PGSd_DISABLE_SEED:

	/* In this case "input" is expected to be the value of an SMF seed (SMF
	   status codes are generated from an initial integer seed).  Logging
	   will be disabled for all SMF status codes derived from the seed
	   corresponding to the value of "input" (e.g. error messages used by
	   the toolkit TD group are generated from the seed 3, if the value of
	   "input" is 3 then logging will be disabled for all TD (PGSTD_...)
	   error messages). */

	list = &seed_list;
	length = &seed_list_length;
	operation = PGSd_ADD_ELEMENT;
	break;

      case PGSd_DISABLE_CODE:

	/* In this case "input" is expected to a specific SMF code.   Logging
	   will be disabled for this specific SMF code (e.g. if the "input"
	   corresponds to the code for PGS_E_TOOLKIT, that error message will
	   not be recorded in the log file). */

	list = &code_list;
	length = &code_list_length;
	operation = PGSd_ADD_ELEMENT;
	break;
	
      case PGSd_ENABLE_ALL:

	/* nullify last request to disable logging of all messages (see NOTES in
	   prolog above for more detail) */

	if (nest_level != 0U)
	{
	    if (--nest_level == 0U)
	    {
		logging = PGSd_ON;
	    }
	}
	return PGS_S_SUCCESS;

      case PGSd_ENABLE_STATUS_LEVEL:

	/* In this case "input" is expected to be the value of an SMF status
	   level (see PGS_SMF.h for hexidecimal definitions of these values).
	   The last request for logging to be disabled for this level will be
	   nullified (see NOTES in prolog above for more detail). */

	list = &level_list;
	length = &level_list_length;
	operation = PGSd_DELETE_ELEMENT;
	break;
	
      case PGSd_ENABLE_SEED:

	/* In this case "input" is expected to be the value of an SMF seed (SMF
	   codes are generated from an initial integer seed).  The last request
	   for logging to be disabled for this seed will be nullified (see NOTES
	   in prolog above for more detail). */

	list = &seed_list;
	length = &seed_list_length;
	operation = PGSd_DELETE_ELEMENT;
	break;
	
      case PGSd_ENABLE_CODE:

	/* In this case "input" is expected to be a specific SMF status code.
	   The last request for logging to be disabled for this specific status
	   code will be nullified (see NOTES in prolog above for more
	   detail). */

	list = &code_list;
	length = &code_list_length;
	operation = PGSd_DELETE_ELEMENT;
	break;
	
      case PGSd_FLUSH:

	/* In this case the value of "input" is ignored.  The list pointers in
	   this function are pointers to memory space allocated, controlled and
	   freed in the function PGS_SMF_ManageLogControlList().  This case is a
	   request to free all memory associated with the list pointers (this of
	   course has the effect of zeroing out the lists, the list lengths will
	   be set to zero). */

	PGS_SMF_ManageLogControlList(&code_list, &code_list_length, 
				     PGSd_FREE_LIST, NULL);
	PGS_SMF_ManageLogControlList(&level_list, &level_list_length, 
				     PGSd_FREE_LIST, NULL);
	PGS_SMF_ManageLogControlList(&seed_list, &seed_list_length,
				     PGSd_FREE_LIST, NULL);
	all_empty = PGS_TRUE;
	return PGS_S_SUCCESS;
	
      default:
	return PGS_SMF_LoggingControl(PGSd_QUERY, input);
	
    }
    
    /* This section of the code is only reached if the input command was a
       request to enable or disable logging of a particular status code or type
       of status code (where type is defined by a status codes "status level" or
       the seed value from which it was generated).  Here a call is made
       to the list manager function to add/delete an element to/from the
       appropriate list and increment/decrement the appropriate list length
       variable. */ 

    returnStatus = PGS_SMF_ManageLogControlList(list, length, operation, input);

    /* Check to see if ALL lists have zero length.  If they do then set
       "all_empty" to PGS_TRUE, otherwise set it to PGS_FALSE. */

    if ( (level_list_length == 0) && 
	 (seed_list_length == 0) &&
	 (code_list_length == 0) )
    {
	all_empty = PGS_TRUE;
    }
    else
    {
	all_empty = PGS_FALSE;
    }
    
    return returnStatus;
}
