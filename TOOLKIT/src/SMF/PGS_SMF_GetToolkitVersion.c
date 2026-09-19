/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
   PGS_SMF_GetToolkitVersion.c

DESCRIPTION:
   This file contains the function PGS_SMF_GetToolkitVersion().
   This function returns a string describing the current version of the Toolkit.

AUTHOR:
   Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
   23-Feb-1994  GTSK Initial Version

END_FILE_PROLOG:
*******************************************************************************/
/*******************************************************************************
BEGIN_PROLOG
 
TITLE: 
   Get Toolkit Version

NAME: 
   PGS_SMF_GetToolkitVersion()

SYNOPSIS: 
C:     
    #include <PGS_SMF.h>

    void
    PGS_SMF_GetToolkitVersion(
        char  version[21])

FORTRAN:
      subroutine pgs_smf_gettoolkitversion(version)
      character*20 version

DESCRIPTION:
   This function returns a string describing the current version of the Toolkit.
      
INPUTS:
   None

OUTPUTS:
  Name       Description
  ----       -----------
  version    character string describing the current version of the Toolkit

RETURNS:
   None

EXAMPLES:
C:   
    char version[21];

    PGS_SMF_GetToolkitVersion(version);

FORTRAN:
       character*20

       call pgs_smf_gettoolkitversion(version)

NOTES:
   User must allocate enough memory to hold the Toolkit version string.
   This function does not allocate any memory for the user.

REQUIREMENTS:
   PGSTK - ????
 
DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS CALLED:
   None
      
END_PROLOG	
*******************************************************************************/

#include <string.h>
#include <PGS_SMF.h>

/* name of this function */

#define FUNCTION_NAME "PGS_SMF_GetToolkitVersion()"

void
PGS_SMF_GetToolkitVersion(  /* Get Toolkit version description string. */
    char    version[21])    /* version string (output) */
{
    /* copy the Toolkit version string to the location pointed to by the input
       variable "version" (use strncpy to ensure that we never copy in a string
       larger than the advertised 20 characters) */

    strncpy(version, PGSd_TOOLKIT_VERSION_STRING, 20);

    /* that's it */

    return;
}
