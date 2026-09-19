/* $Header:   /home/pvcs/tempdir/strnstr.c_v   1.6   04/03/95 17:32:46   pvcs  $
 *
 * CONTAINS:	ff_strnstr()
 *				   
$Log:   /home/pvcs/tempdir/strnstr.c_v  $
 * 
 *    Rev 1.6   04/03/95 17:32:46   pvcs
 * converted memset() to memMemset()
 * 
 *    Rev 1.8   19 Mar 1992 15:16:26   tedh
 * No change.
 * 
 *    Rev 1.7   10 Feb 1992 08:59:28   tam
 * revised ptr defs and added casts to HUGE
 * 
 *    Rev 1.6   10 Jan 1992 22:44:10   mvg
 * Added error messages and initialized pointer declarations to NULL
 * Note: searching for a NULL character is considered an error in this version
 * 
 *    Rev 1.5   03 Jan 1992 10:39:36   tedh
 * Second effort for Medium model
 * 
 *    Rev 1.4   31 Dec 1991 14:51:48   tedh
 * changed Pointer definitions
 * 
 *    Rev 1.3   27 Dec 1991 14:04:58   tedh
 * First MAC Version
 * 
 *    Rev 1.2   18 Jun 1991 17:33:32   unknown
 * header modifications: to match documentation
*/

/***************************************************************
 *
 * Boyer-Moore string search routine
 *
 * Author:    John Rex
 * References: (1) Boyer RS, Moore JS: "A fast string searching
 *                 algorithm"  CACM 20(10):762-777, 1977
 *             (2) plus others--see text of article
 *
 * Compilers: Microsoft C V5.1  - compile as is
 *            Turbo C V2.0      - compile as is
 *
 * Compile time preprocessor switches:
 *    DEBUG - if defined, include test driver
 *
 * Usage:
 *
 *   char *pattern, *text;  - search for pattern in text
 *   unsigned length;       - length of text (the routine does
 *                            NOT stop for '\0' bytes, thus
 *                            allowing it to search strings
 *                            stored sequentially in memory.
 *   char *start;           - pointer to match
 *
 *   char *Boyer_Moore(char *, char *, unsigned);
 *
 *   start = Boyer_Moore(pattern, text, strlen(text);
 *
 *   NULL is returned if the search fails.
 *
 *   Switches: if defined:
 *
 *      DEBUG will cause the search routine to dump its tables
 *            at various times--this is useful when trying to
 *            understand how upMatchJump is generated
 *
 *      DRIVER will cause a test drive to be compiled
 *
 * Source code may be used freely if source is acknowledged.
 * Object code may be used freely.
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "Strnstr"

#define AlphabetSize 256

/*
 * NAME:	ff_strnstr
 *		
 * PURPOSE:
 *			-- see above --
 * AUTHOR:
 *
 * USAGE:	ff_strnstr(
 *				char *pcPattern,	we search for this ... 
 *				char *pcText,		... in this text ...   
 *				size_t uTextLen)	... up to this length  
 *							   							   	
 * COMMENTS:
 *
 * RETURNS:
 *
 * ERRORS:
 *		Out of memory,"upMatchJump"
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

FF_DATA_BUFFER ff_strnstr(char *pcPattern,
	FF_DATA_BUFFER pcText,
	size_t uTextLen)
{
             /* array of character mis-match offsets */
    unsigned uCharJump[AlphabetSize];
             /* array of offsets for partial matches */
    unsigned *upMatchJump = NULL;
             /* temporary array for upMatchJump calc */
    unsigned *upBackUp = NULL;
    unsigned u, uPatLen;
    unsigned uText, uPat, uA, uB;

	/* Error checking on NULL parameters pcPattern and pcText */

	assert(pcPattern && pcText);

	/* Setup and initialize arrays */
    uPatLen = strlen(pcPattern);
    upMatchJump = (unsigned *)
         memMalloc(2 * (sizeof(unsigned) * (uPatLen + 1)), "ff_strnstr: upmatchjump" );

    if (!upMatchJump) {
		err_push(ROUTINE_NAME,ERR_MEM_LACK,"upMatchJump");
		return(NULL);
    }

	upBackUp = upMatchJump + uPatLen + 1;

    /* Heuristic #1 -- simple char mis-match jumps ... */
    memMemset((void *)uCharJump, 0, AlphabetSize*sizeof(unsigned),"uCharJump,0,AlphabetSize*sizeof(unsigned)");
    for (u = 0 ; u < uPatLen; u++)
        uCharJump[((unsigned char) pcPattern[u])]
                     = uPatLen - u - 1;

    /* Heuristic #2 -- offsets from partial matches ... */
    for (u = 1; u <= uPatLen; u++)
        upMatchJump[u] = 2 * uPatLen - u;
                                /* largest possible jump */
    u = uPatLen;
    uA = uPatLen + 1;
    while (u > 0) {
        upBackUp[u] = uA;
        while( uA <= uPatLen &&
          pcPattern[u - 1] != pcPattern[uA - 1]) {
            if (upMatchJump[uA] > uPatLen - u)
                upMatchJump[uA] = uPatLen - u;
            uA = upBackUp[uA];
        }
        u--;
        uA--;
    }


    for (u = 1; u <= uA; u++)
        if (upMatchJump[u] > uPatLen + uA - u)
            upMatchJump[u] = uPatLen + uA - u;

    uB = upBackUp[uA];

    while (uA <= uPatLen) {
        while (uA <= uB) {
            if (upMatchJump[uA] > uB - uA + uPatLen)
                upMatchJump[uA] = uB - uA + uPatLen;
            uA++;
        }
        uB = upBackUp[uB];
    }

    /* now search */
    uPat = uPatLen;         /* tracks position in Pattern */
    uText = uPatLen - 1;    /* tracks position in Text */
    while (uText < uTextLen && uPat != 0) {
        if (pcText[uText] == pcPattern[uPat - 1]) { /* match? */
            uText--;    /* back up to next */
            uPat--;
        }
        else { /* a mismatch - slide pattern forward */
            uA = uCharJump[((unsigned char) pcText[uText])];
            uB = upMatchJump[uPat];
            uText += (uA >= uB) ? uA : uB;  /* select larger jump */
            uPat = uPatLen;
        }
    }

    /* return our findings */
    memFree(upMatchJump, "ff_strnstr: upmatchjump");
    if (uPat == 0)
        return(pcText + (uText + 1)); /* have a match */
    else
        return (NULL); /* no match */
}


