/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_LogPID.c

DESCRIPTION:
   This file contains the function PGS_SMF_LogPID().
   This function maintains a character string representation of the current
   Process ID (PID), if requested.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   10-Jan-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Return Current PID as String

NAME:
   PGS_SMF_LogPID()

SYNOPSIS:
C:
   #include <PGS_SMF.h>

   char*
   PGS_SMF_LogPID(
       PGSt_integer    command)

FORTRAN:		  
   N/A

DESCRIPTION:
   This function is for internal toolkit usage.  It maintains a string
   representation of the current process ID (PID).  This value is returned on
   request by this function (or a null string if no PID is being maintained).

INPUTS:
   Name         Description
   ----         -----------
   command      input command, determines what action this function should take
                possible values are:

                   PGSd_QUERY
		   return a pointer to the current PID value (expressed as a
		   character string), take no other action

		   PGSd_DISABLE_PID_LOGGING
		   set the value of the PID character string to the null string

		   PGSd_ENABLE_PID_LOGGING
		   set the value of the PID character string to the current
		   process ID (PID)

OUTPUTS:
   None
          
RETURNS:
   pid_string - pointer to character string representation of current PID

EXAMPLES:
C:

FORTRAN:
  
NOTES:
   This function is intended for internal SMF use only.  A separate SMF function
   is expected to call this function to initialize the the PID string to the
   value of the current PID if the PID is to be added to log entries in the SMF
   log files.  By default the value of the PID string is null.  SMF logging
   functions will subsequently access this function to determine the value of
   the PID  string and include this value in log entries if it is not null.

REQUIREMENTS:
   PGSTK - ????, ????

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   None

END_PROLOG:
*******************************************************************************/

#include <PGS_SMF.h>
#include <sys/types.h>
#include <unistd.h>

char *
PGS_SMF_LogPID(              /* get/set current PID as character string */
    PGSt_integer   command)  /* input command (i.e. what to do) */
{
    /* current process ID (PID) as character string */

    static char pid_string[20]="";
    
    switch (command)
    {
      case PGSd_QUERY:

	/* query the value of pid_string */

	break;
	
      case PGSd_DISABLE_PID_LOGGING:

	/* set pid_string to the null string */

	pid_string[0] = '\0';
	break;
	
      case PGSd_ENABLE_PID_LOGGING:

	/* set pid_string to the value of the current process ID */

	sprintf(pid_string," (PID=%u)", getpid());
	break;
	
      default:
	break;
    }

    return pid_string;
}
