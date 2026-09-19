/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  COPYRIGHT[copyright mark] 2000, Raytheon System Company, its vendors,  */
/*  and suppliers.  ALL RIGHTS RESERVED.                                   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*******************************************************************************
BEGIN_FILE_PROLOG:

FILENAME:
  PGS_EPH_fileHandlingStuff.c

DESCRIPTION:
  This file contains the functions:
     PGS_EPH_file_split_path()
     PGS_EPH_file_exits()

AUTHOR:
  Mike Sucher / Applied Research Corporation
  Guru Tej S. Khalsa / Applied Research Corporation

HISTORY:
         1994  MS  Initial version
  17-Oct-1994 GTSK Prepended PGS_EPH_ to function names.  Altered return types
                   of functions.
  09-July-1999 SZ  Updated for the thread-safe functionality
END_FILE_PROLOG:
*******************************************************************************/

/*******************************************************************************
BEGIN_PROLOG:

TITLE:
   file_split_path()

NAME:
   dotProduct

SYNOPSIS:
C:

int file_split_path(char *name, char *path, char *fname )

DESCRIPTION:
  
INPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
 
OUTPUTS:
   Name         Description               Units       Min   Max
   ----         -----------               -----       ---   ---
 
          
RETURNS:
   
EXAMPLES:
C:

FORTRAN:
  
NOTES:
   None

REQUIREMENTS:
   PGSTK - 0720

DETAILS:
   None

GLOBALS:
   None

FILES:
   None

FUNCTIONS_CALLED:
   none 

END_PROLOG:
*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <PGS_EPH.h>
#include <PGS_SMF.h>


/*
 * function: file_split_path()
 * author:   mike sucher
 * purpose:  split path prefix from file name
 * arguments:
 *     char *name;  input:  full name of file including path
 *     char *path;  output: the path, split from the filename
 *     char *fname; output: the filename, split from the path
 * returns:
 *     none
 * notes:
 *     the path separator is the slash ('/') character, thus this
 *     routine is only intended for Unix filesystems
 */
void PGS_EPH_file_split_path(char *name, char *path, char *fname )
{
    
    char c, *p1, *p2, *q;
    

    p1 = p2 = name;
    

    while(1)
    {
	
        for(q=p2; (*q != '/') && *q; q++);
	
        if(*q) p2 = q+1;
	
        else break;
	
    }
    

    strcpy(fname, p2);
    

    if(p2 > p1)
    {
	
        c = *(p2-1);
	
        *(p2-1) = 0;
	
        strcpy(path, p1);
	
        *(p2-1) = c;
	
    }
    
    else path[0] = 0;
    

    return;
    
}


/*
 * function: file_exists()
 * author:   mike sucher
 * purpose:  check to see if file exists
 * arguments:
 *     char *name; name, including path, of the file to check
 * returns:
 *     FOUND     (1) file does exist
 *     NOT_FOUND (0) file does not exist
 */

#define FOUND 1
#define NOT_FOUND 0

PGSt_boolean PGS_EPH_file_exists(char *name)
{
    
    DIR *dirp;
    
#ifdef _PGS_THREADSAFE
    /* used by readdir_r() */
    struct dirent dRes;   
    struct dirent *result;  
#else
    struct dirent *dp;
#endif
    
    char fname[128], path[128];
    

    PGS_EPH_file_split_path(name,path,fname);
    if(strlen(path) != 0) dirp = opendir(path);
    
    else dirp = opendir( "." );
    
    if (dirp == NULL)
      return NOT_FOUND;
    
#ifdef _PGS_THREADSAFE
    /* readdir() is not thread-safe, use readdir_r() */
    while ( (readdir_r( dirp, &dRes, &result )) != 0 )
    if( strcmp( dRes.d_name, fname ) == 0 )
#else
    while ( (dp = readdir( dirp )) != NULL )
    if( strcmp( dp->d_name, fname ) == 0 )
#endif
    {
	
        closedir(dirp);
	
        return FOUND;
	
    }
    
    closedir(dirp);
    
    return NOT_FOUND;
    
}

