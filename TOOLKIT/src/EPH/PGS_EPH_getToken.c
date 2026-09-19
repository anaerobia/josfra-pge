#include <string.h>
#include <PGS_EPH.h>
#include <PGS_TSF.h>

/* name of this function */

#define FUNCTION_NAME "PGS_EPH_getToken()"

char*
PGS_EPH_getToken(              /* parse a string ala strtok, but strip leading
				  and trailing white space */
    char*         inputString, /* string to parse */
    const char*   delimeter)   /* string of delimeter character(s) */
{
    char*         beginPtr;    /* pointer to begining of token to be returned */
    char*         endPtr;      /* pointer used to strip white space from end of
				  token returned by strtok() */
    
    /* parse input string, get next token */

#ifdef _PGS_THREADSAFE
    char *lasts;    /* used by strtok_r() */
    /* strtok() is not thread-safe, use strtok_r() */
    beginPtr = strtok_r(inputString, delimeter,&lasts);
#else
    beginPtr = strtok(inputString, delimeter);
#endif

    if (beginPtr == NULL)
    {
	return beginPtr;
    }

    /* move pointer to ignore any leading blanks, tabs or newlines */

    while ( (*beginPtr == ' ') || (*beginPtr == '\t') || (*beginPtr == '\n') )
    {
	beginPtr++;
    }
    if (*beginPtr == '\0')
    {
	/* if the string is a null string, then ignore this "token" and get the
	   next one */

	return PGS_EPH_getToken(NULL, delimeter);
    }

    /* move backward along the string removing any trailing blanks, tabs,
       newlines */

    endPtr = beginPtr + strlen(beginPtr) - 1;
    
    while ( (*endPtr == ' ') || (*endPtr == '\t') || (*beginPtr == '\n') )
    {
	*endPtr = '\0';
	endPtr--;
    }

    return beginPtr;
}
