/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_TraceControl.c

DESCRIPTION:
   This file contains the function PGS_SMF_TraceControl().
   This function controls the trace level of entries in the SMF log files.

AUTHOR:
   Guru Tej S. Khalsa

HISTORY:
   10-Jan-1996  GTSK  Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   Control Trace Level of SMF Messages

NAME:
   PGS_SMF_TraceControl()

SYNOPSIS:
C:
   #include <PGS_SMF.h>

   PGSt_SMF_status
   PGS_SMF_TraceControl(
       PGSt_integer    command)

FORTRAN:		  
   N/A

DESCRIPTION:
   This function is for internal toolkit usage.  It controls the trace level of
   SMF messages in the SMF log files.  The trace level can be any one of:
   none, error or full.

INPUTS:
   Name         Description
   ----         -----------
   command      input command, determines what action this function should take
                possible values are:

                   PGSd_QUERY
		   determine the current trace level, return the current value
		   of the trace level without taking any other action

		   PGSd_SET_NO_TRACE
		   disable all tracing and set the return value to indicate this
		   state

		   PGSd_SET_ERROR_TRACE
		   trace errors only, when this level of tracing is enabled any
		   time a status message is recorded in an SMF log file a trace
		   of the function calls made up to the function from which the
		   status message was sent will be prepended to the status
		   message in the log file (see NOTES below)

		   PGSd_SET_FULL_TRACE
		   do full tracing, when this level of tracing is enabled any
		   time a function is entered or exited the event will be
		   recorded in the SMF status log file (the usual status
		   messages are of course also still recorded when they occur)
		   (see NOTES below)

OUTPUTS:
   None
          
RETURNS:
   PGSSMF_M_TRACE_DISABLED       tracing is disabled
   PGSSMF_M_ERROR_TRACE_ENABLED  error tracing is enabled
   PGSSMF_M_FULLTRACE_ENABLED    full tracing is enabled

EXAMPLES:
C:

FORTRAN:
  
NOTES:
   Tracing works in connection with the SMF functions PGS_SMF_Begin() and
   PGS_SMF_End().  Users wshing to take advantage of tracing in the SMF log
   files must make a call to PGS_SMF_Begin() at the beginning of each of their
   functions to register the function with the SMF utilities and call
   PGS_SMF_End() just before exiting each function to similarly let SMF know
   that the function has completed execution.

   The effect of tracing is to keep track of where in the execution path things
   are occurring.  If "error" tracing is enabled then any time a status message
   is logged to an SMF log file the function heirachy at that point is recorded
   with the status message entry.  If "full" tracing is enabled each entry into
   and exit from a function is recorded in the SMF status log, regardless of
   whether any status messages where logged to the log file.  Following are some
   examples of what this looks like in the log files:

   *** No Tracing ***

   func4():PGSTD_M_ASCII_TIME_FMT_B:26112 (PID=2710)
   the time string passed in is in proper CCSDS ASCII Time Format B

   *** Error Tracing ***

   main()
     func1()
       func2()
         func3()
	   func4():PGSTD_M_ASCII_TIME_FMT_B:26112 (PID=2710)
	   the time string passed in is in proper CCSDS ASCII Time Format B

   *** Full Tracing ***

   PGS_SMF_Begin: main()

     PGS_SMF_Begin: func1()

       PGS_SMF_Begin: func2()

       PGS_SMF_End: func2()

     PGS_SMF_End: func1()

     PGS_SMF_Begin: func1()

       PGS_SMF_Begin: func2()

         PGS_SMF_Begin: func3()

           PGS_SMF_Begin: func4()

             func4():PGSTD_M_ASCII_TIME_FMT_B:26112 (PID=2710)
	     the time string passed in is in proper CCSDS ASCII Time Format B

           PGS_SMF_End: func4()

         PGS_SMF_End: func3()

       PGS_SMF_End: func2()

     PGS_SMF_End: func1()

   PGS_SMF_End: main()
     
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

PGSt_SMF_status
PGS_SMF_TraceControl(   /* control tracing functionality */
    int   command)      /* input command (i.e. what to do) */
{
    /* the variable "trace_level" keeps track of the current SMF log tracing
       value, it is initialized for no tracing */

    static PGSt_SMF_status trace_level=PGSSMF_M_TRACE_DISABLED;
    
    switch (command)
    {
      case PGSd_QUERY:

	/* don't do anything, return current value of trace_level */

	break;
	
      case PGSd_SET_NO_TRACE:

	/* turn tracing off */

	trace_level = PGSSMF_M_TRACE_DISABLED;
	break;
	
      case PGSd_SET_ERROR_TRACE:

	/* enable error tracing only (see INPUTS and NOTES above) */

	trace_level = PGSSMF_M_ERROR_TRACE_ENABLED;
	break;
	
      case PGSd_SET_FULL_TRACE:

	/* enable full tracing (see INPUTS and NOTES above) */

	trace_level = PGSSMF_M_FULL_TRACE_ENABLED;
	break;

      default:

	/* by default don't do anything, return current value of trace_level
	   (this is the same as the PGSd_QUERY case (above)) */

	break;
    }

    /* return current value of trace_level */

    return trace_level;
}
