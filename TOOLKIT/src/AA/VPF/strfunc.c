/*************************************************************************
*
*N  Module STRFUNC
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This module contains functions some additional character string
*     handling functions beyond the standard C string libraries.
*     The strings passed in to each of these functions is actually
*     modified and an identical string is returned.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     N/A
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*     Brian Glover             Nov 1992                       gcc
*E
*************************************************************************/

#include "strfunc.h"
#include <malloc.h>
#include <string.h>
/* #include <strings.h> */
#include <ctype.h>

/*************************************************************************
*
*N  strupr
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function changes all lowercase characters in a string to uppercase.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     string     <inout>  == (char *) string to be made uppercase.
*     strupr     <output> == (char *) pointer to string.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*     Brian Glover	April 1992	Rewrite of non-ANSI function
*E
*************************************************************************/

char *strupr(char *string)
{
  int i;

  if (!string) return string;
  for(i=0;i<(int)strlen(string);i++)
    string[i]=toupper(string[i]);
  return string;
}

/*************************************************************************
*
*N  strlwr
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function changes all uppercase characters in a string to lowercase.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     string     <inout>  == (char *) string to be made lowercase.
*     strlwr     <output> == (char *) pointer to string.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*     Brian Glover	April 1992	Rewrite of non-ANSI function
*E
*************************************************************************/
char *strlwr(char *string)
{
  int i;

  if (!string) return string;
  for(i=0;i<(int)strlen(string);i++)
    string[i]=tolower(string[i]);
  return string;
}

/*************************************************************************
*
*N  strreverse
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function reverses the characters in a string.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     string     <inout>  == (char *) string to be reversed.
*     strreverse <output> == (char *) pointer to string.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*     Brian Glover      April 1992     Rewrite of non-ANSI function    
*E
*************************************************************************/
static char *strreverse(char *str)
{
  register int i,len;
  char *copy;

  len = strlen(str);
  copy = (char *) malloc(sizeof(char)*(len+1));
  (void) strcpy(copy,str);
  for(i=0;i<len;i++)
    str[i]=copy[(len-1)-i];
  free(copy);
  return str;
}

/*************************************************************************
*
*N  leftjust
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function left justifies a string by removing all leading
*     whitespace.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     string     <inout>  == (char *) string to be left justified.
*     leftjust   <output> == (char *) pointer to string.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*     Brian Glover      April 1992                                   
*                       Nov 1992   UNIX port
*E
*************************************************************************/
char *leftjust(char * str)
{
   register char * eol;

   if (!str) return str;

   strcpy(str, str + strspn(str, " \t\n\b"));

   if ((eol = strchr(str, '\n')) != NULL)
     *eol = 0;

   return str;
}

/*************************************************************************
*
*N  rightjust
*
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Purpose:
*P
*     This function right justifies a string by removing all trailing
*     whitespace.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   Parameters:
*A
*     string     <inout>  == (char *) string to be right justified.
*     rightjust  <output> == (char *) pointer to string.
*E
*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
*
*   History:
*H
*     Brian Glover      April 1992                                   
*                       Nov 1992   UNIX port
*E
*************************************************************************/
char *rightjust( char *str )
{
   if (!str) return str;
   return strreverse(leftjust(strreverse(str)));
}


