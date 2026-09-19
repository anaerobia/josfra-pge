/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_TD_TimeInterval.c

DESCRIPTION:
  This file contains the function PGS_TD_TimeInterval()

AUTHOR:
  Anubha Singhal / Applied Research Corp.

HISTORY:
  07-July-1994 AS Initial version

END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
      Compute elapsed TAI time 

NAME: 
      PGS_TD_TimeInterval()

SYNOPSIS:
C:   
     #include <PGS_TD.h>
  
     pgs_status
     PGS_TD_TimeInterval(
          PGSt_double        startTAI,
          PGSt_double        stopTAI,
          PGSt_double        *interval)

FORTRAN:
     include   'PGS_SMF.f'
     include   'PGS_TD_3.f'

     integer function  pgs_td_timeinterval(starttai,stoptai,interval)
     double precision   starttai
     double precision   stoptai
     double precision   interval

DESCRIPTION:
     This function computes the elapsed TAI time in seconds between any two 
     time intervals
   
INPUTS:
     
     Name       Description               Units       Min       Max
     ----       -----------               -----       ---       ---
     startTAI   start time in TAI         seconds     none      none
             
     stopTAI    stop time in TAI          seconds     none      none

OUTPUTS:
     
     Name       Description               Units       Min       Max
     ----       -----------               -----       ---       ---
     interval   elapsed time interval     seconds     none      none
          
RETURNS:
     PGS_S_SUCCESS                    successful return

EXAMPLES:
C:
     PGSt_SMF_status    returnStatus;
     PGSt_double        startTAI;
     PGSt_double        stopTAI;
     PGSt_double        *interval;

     startTAI = 34523.5;
     stopTAI  = 67543.2;
     returnStatus  = PGS_TD_TimeInterval(startTAI,stopTAI,&interval); 

FORTRAN:
     integer            pgs_td_timeinterval
     integer            returnstatus
     double precision   starttai
     double precision   stoptai
     double precision   interval

     returnstatus = pgs_td_timeinterval(starttai,stoptai,interval)          
    
NOTES:
      This interval is the same as elapsed internal time and is the true
      interval in SI seconds.

REQUIREMENTS:
      PGSTK-1190

DETAILS:
      This function can accept any of the following input:
      i)   stopTAI greater than startTAI
      ii)  stopTAI less than startTAI
      iii) stopTAI equal to startTAI

GLOBALS:
      none
			  
FILES:
      none

FUNCTIONS CALLED:
      none

END_PROLOG:
*******************************************************************************/

#include <PGS_TD.h>

PGSt_SMF_status
PGS_TD_TimeInterval(          /* computes elapsed TAI time between two epochs */
    PGSt_double   startTAI,   /* Start time in TAI */
    PGSt_double   stopTAI ,   /* Stop time in TAI */
    PGSt_double   *interval)  /* Elapsed time interval */
{
    PGSt_SMF_status returnStatus; /* value returned by function indicating
			            success or the nature of any errors */

    /* initialize return value and message to indicate success */

    returnStatus = PGS_S_SUCCESS;
    PGS_SMF_SetStaticMsg(returnStatus,"PGS_TD_TimeInterval()");
    
    /* compute the elapsed time between the two epochs */
    
    *interval = stopTAI - startTAI;
    
    /* return to calling function */
    
    return returnStatus;
}
