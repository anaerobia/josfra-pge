/* 
 *
 * CONTAINS:    Functions for tracking allocation of memory:
 */
/*
 * HISTORY:
 *	r fozzard	4/21/95		-rf01 
 *		malloc.h not needed on mac?
 * 		(char *) for Think C
 *		function declarations for MEMMemcpy and MEMMemmove
 *	r fozzard	7/18/95		-rf02
 *		add _fMEMMemcpy definition for mac 
*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#ifdef CCMSC
#include <io.h>
#endif

#include <string.h>
#include <memtrack.h>

#ifdef SUNCC
#include <errno.h>
#endif

#ifdef XVT
#include <xvt.h>
#endif

/* the next functions are only called by the other mem functions */

static void MEMTrack_alloc(void *vp, unsigned long size, char *tag, char *routine_name, char *cfile_name, int line_number);
static unsigned long MEMTrack_free(void *vp, char *tag, char *routine_name, char *cfile_name, int line_number);
static FILE	*MEMTrack_fp(char *tag, char *routine_name, char *cfile_name, int line_number);
static int	MEMTrack_msg(char *msg, char *routine_name, char *cfile_name, int line_number);
static unsigned long MEMTrack_find(void *to_find, char *tag, char *routine_name, char *cfile_name, int line_number);

#define  ALLOC          'A'
#define  FREE           'F'
#define  MESSAGE_LETTER 'M'
#define  FILL_CHAR      '\xCC'
#define  MAX_LTH        255

#ifndef BOOLEAN
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#define BOOLEAN unsigned char
#define TRUE 1
#define FALSE 0
#endif /* BOOLEAN */

/*
 * NAME:    MEMTrack_alloc
 *      
 * PURPOSE: Track an allocation. Write it in the memory log file
 *          in the format A memlocation bytesallocated tag, where memlocation is a number
 *          in %lx format (long int, hex notation)
 *
 * USAGE:   void MEMTrack_alloc(void *allocated, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: None
 *
 * DESCRIPTION: This function is called by the memory allocation functions
 *              which are active if MEMTRACK is defined.
 *              MEMTrack_alloc records the allocation of memory in the
 *              memory tracking log file. The name of this file is given by the
 *              environment variable MEMTRACK.
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMTrack_alloc" 

static void MEMTrack_alloc(void  *allocated, unsigned long size, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	FILE  *fp;

	if ((fp = MEMTrack_fp(tag, routine_name, cfile_name, line_number)) != NULL)
	{
		fseek(fp,0L,SEEK_END);
		fprintf(fp,"%c %lx %lu %s %s %d %s\n", ALLOC, allocated, size, routine_name, cfile_name, line_number, tag);
		fclose(fp);
	}
	else
	{/* MEMTRACK is not defined */
	}
}

/*
 * NAME:    MEMTrack_free
 *      
 * PURPOSE: Track and perform a memory free. Check the memory tracking log
 * file to make sure memory being freed was allocated.
 *
 * USAGE:   void MEMTrack_free(void *to_free, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: size of allocated block to be free()'d, else zero or ULONG_MAX.
 *
 * DESCRIPTION: This function is called by the memory functions
 *              which are active if MEMTRACK is defined.
 *              MEMTrack_free records the freeing of memory in the
 *              memory log file. The name of this file is given by the
 *              environment variable MEMTRACK.
 *
 * Returns the allocation size of the block to be free()'d as recorded in the
 * log file.  If no record, or is a NULL pointer, returns zero.  If MEMTRACK 
 * logging is disabled (environment variable MEMTRACK is undefined) and
 * pointer is not NULL, return ULONG_MAX.
 *
 * Logs a message if:
 * 1) address was not previously recorded as an allocation
 * 2) address is NULL
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMTrack_free" 

static unsigned long MEMTrack_free(void  *to_free, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	FILE  *fp;
	void  *addr_in_file = 0;
	char  line[MAX_LTH];
	char  found = 0;
	int   ii;
	long  loc;
	unsigned long size = 0;
	char  msg[MAX_LTH];

	if ((fp = MEMTrack_fp(tag, routine_name, cfile_name, line_number)) != NULL)
	{
		rewind(fp);
		for (loc=0L; fgets(line,MAX_LTH,fp); loc = ftell(fp))
		{
			if (line[0] != ALLOC)         /* Is the line an 'Allocated' line?  */
				continue;                  /*   If not, back to top of loop.    */
			ii = sscanf(line,"%*c %lx %ul", &addr_in_file, &size);
			if (ii==0 || ii==EOF)
				continue;
			/* Is addr in file the one we want?  */
			if ((long int *)addr_in_file - (long int *)to_free == 0)
			{
				found = 1;                 
				fseek(fp,loc,SEEK_SET);    /* Back to start of line    */
				fputc(FREE,fp);            /* Over-write the ALLOC tag */
				break;
			}
			else
				size = 0;
		}
		fclose(fp);
		if (found)
		{
			return(size);
		}
		else
		{
			sprintf(msg, "BAD free(), unallocated:%lx (%s)", to_free, tag);
			MEMTrack_msg(msg, routine_name, cfile_name, line_number);
			return(0);
		}
	}
	else
	{/* MEMTRACK is not defined */
		/* do special check on freeing NULL */
		if (to_free == NULL)
		{
			sprintf(msg, "BAD free(), NULL (%s)", tag);
			MEMTrack_msg(msg, routine_name, cfile_name, line_number);
			return(0);
		}
		else
			return(ULONG_MAX);
	}
}

/*
 * NAME:    MEMTrack_fp
 *      
 * PURPOSE: Return FILE pointer for memory tracking log file.
 *
 * USAGE:   static FILE  *MEMTrack_fp()
 *
 * RETURNS: File pointer if environmental variable MEMTRACK is defined.
 *
 * DESCRIPTION: This function is called by the memory functions
 *              which are active if MEMTRACK is defined.
 *              The name of the memory log file is given by the
 *              environment variable MEMTRACK.
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then no action is taken.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:  
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMTrack_fp" 

static FILE *MEMTrack_fp(char *tag, char *routine_name, char *cfile_name, int line_number)
{
	FILE *fp = NULL; /* File pointer to return */
	static char *ep = NULL; /* Points to environment var that names file */
	static BOOLEAN virgin = TRUE;

	if (ep == NULL)
	{              /* First time through, just create blank file */

#if defined(CCMSC) || defined(SUNCC)
		ep = getenv("MEMTRACK");
#else
#error need to insert (Mac) getenv() equivalent
#error Do not use FreeForm to do this!
#endif

		if (!ep && virgin == TRUE)
		{ /* fp used as virginity clause */

#ifdef XVT
			xvt_dm_post_note("environment variable MEMTRACK is undefined -- memory tracking disabled");
#else
			fprintf(stderr, "environment variable MEMTRACK is undefined -- memory tracking disabled\n");
#endif

			virgin = FALSE; /* so as above message is received only first time in this function */
		}
		if (ep == NULL)
			return(NULL);
			
		fp = fopen(ep,"w");
		if (!fp)
		{

#ifdef XVT
			switch (xvt_dm_post_ask("CONTINUE", "TERMINATE", NULL,
			                        "Cannot write to file \"%s\"\n \
			                        Called from %s %s %d (%s)",
			                        ep, routine_name, cfile_name, line_number, tag))
			{
				case RESP_DEFAULT:
					break;
				
				case RESP_2:
					xvt_dm_post_fatal_exit("Terminating and Exiting");
					break;
				
				default:
					xvt_dm_post_error("Invalid return from ask.");
					break;
			}
#else
			fprintf(stderr, "Cannot write to file \"%s\"\n", ep);
			fprintf(stderr, "Called from %s %s %d (%s)\n", routine_name, cfile_name, line_number, tag);
			exit(1);
#endif

		}
		fclose(fp);
		fp = (FILE *)NULL;
	}
	if (ep)
	{                    /* If we have a file name, proceed.          */
		fp = fopen(ep,"r+");     /* Open debugging file for append access.    */
		if (!fp)
		{
#ifdef XVT
			switch (xvt_dm_post_ask("CONTINUE", "TERMINATE", NULL,
			                        "Cannot read/append to file \"%s\"\n \
			                        Called from %s %s %d (%s)",
			                        ep, routine_name, cfile_name, line_number, tag))
			{
				case RESP_DEFAULT:
					break;
				
				case RESP_2:
					xvt_dm_post_fatal_exit("Terminating and Exiting");
					break;
				
				default:
					xvt_dm_post_error("Invalid return from ask.");
					break;
			}
#else
			fprintf(stderr, "Cannot read/append to file \"%s\"\n", ep);
			fprintf(stderr, "Called from %s %s %d (%s)\n", routine_name, cfile_name, line_number, tag);
			exit(EXIT_FAILURE);
#endif
		}
	}
	return(fp);
}

/*
 * NAME:    MEMTrack_msg(char *msg)
 *      
 * PURPOSE: Write a message to the debugging file.
 *
 * USAGE:   static int MEMTrack_msg(char *msg, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: non-zero if an error or failure to write to file
 *
 * DESCRIPTION: This function is called by the memory functions
 *              which are active if MEMTRACK is defined.
 *              It writes msg to the memory tracking log file
 * Will write to stderr if fseek() on file fails
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then no action is taken.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
static int MEMTrack_msg(char *msg, char *routine_name, char *cfile_name, int line_number)
{
	FILE *fp;

	if ((fp = MEMTrack_fp(msg, routine_name, cfile_name, line_number)) == NULL)
		return(1);
		
	if (fseek(fp,0L,SEEK_END) != 0)
	{

#ifdef XVT
		switch (xvt_dm_post_ask("CONTINUE", "TERMINATE", NULL,
		                        "Cannot perform fseek operation on memory log file\n \
		                        Called from %s %s %d (%s)", routine_name, cfile_name, line_number, msg))
		{
			case RESP_DEFAULT:
				break;
				
			case RESP_2:
				xvt_dm_post_fatal_exit("Terminating and Exiting");
				break;
				
			default:
				xvt_dm_post_error("Invalid return from ask.");
				break;
		}
#else
		fprintf(stderr, "Cannot perform fseek operation on memory log file\n");
		fprintf(stderr, "Called from %s %s %d (%s)\n", routine_name, cfile_name, line_number, msg);
		exit(EXIT_FAILURE);
#endif

	}
	if (fprintf(fp,"\n%c %s %s %d %s\n", MESSAGE_LETTER, routine_name, cfile_name, line_number, msg) <= 0)
	{
		fclose(fp);
		return(1);
	}
	fclose(fp);
	return(0);
}

static unsigned long MEMTrack_find(void *to_find, char *tag, char *routine_name, char *cfile_name, int line_number)
/*
 * NAME:    MEMTrack_find
 *      
 * PURPOSE: Locate a memory allocation in the memory tracking log, and if found,
 * return the bytes allocated. 
 *
 * USAGE:   int MEMTrack_find()
 *
 * RETURNS: 0 if memtrack file does not show a previous allocation for given address,
 *				  allocated size if found.
 *
 * DESCRIPTION: 
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS: 
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */

{
	void  *addr_in_file = 0; 
	unsigned long buffer_size = 0;
	char  line[MAX_LTH];
	int   ii;
	FILE *fp = NULL;

	if ((fp = MEMTrack_fp(tag, routine_name, cfile_name, line_number)) == NULL)
		return(0);
		
	rewind(fp);
	for (; fgets(line,MAX_LTH,fp); )
	{
		if (line[0] != ALLOC)         /* Is the line an 'Allocated' line?  */
			continue;                  /*   If not, back to top of loop.    */
		ii = sscanf(line,"%*c %lx %lu",&addr_in_file, &buffer_size);
		if (ii==0 || ii==EOF)
			continue;
		/* Is addr in file the one we want?  */
		if ((long int *)addr_in_file - (long int *)to_find == 0)
		{
			fclose(fp);
			return(buffer_size);
		}
	}
	fclose(fp);
	return(0);
}

/*
 * NAME:    MEMCalloc
 *      
 * PURPOSE: Same as calloc(), but registers activity using MEMTrack().
 *
 * USAGE:   void *MEMCalloc(size_t num_elems, size_t bytes_per_elem, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: same as calloc()
 *
 * DESCRIPTION: This function provides a wrapper around calloc which is
 *              called only if MEMTRACK is defined. The wrapper calls
 *              MEMTrack_alloc which writes out the location and size
 *              of the allocated block of memory along with the tag
 *              given to this function.
 *
 * <PLEASE NOTE THAT THE FOLLOWING STATEMENT IS NO LONGER TRUE>
 *              The fill of allocated memory with the value 0XCC is
 *              used to clearly indicate allocated memory while in
 *              the debugger. This was suggested by Steve Maguire in
 *              his book "Writing Solid Code".
 *
 * <ADD'L NOTE>
 * To perform a memset() with the fill char on the allocated block is to render
 * MEMCalloc() no different than MEMMalloc(), which was causing  problems with
 * newform.
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
void *MEMCalloc(size_t num_elems, size_t bytes_per_elem, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	void *allocated;
	
	allocated = calloc(num_elems, bytes_per_elem);
	MEMTrack_alloc(allocated, (unsigned long)num_elems * bytes_per_elem, tag, routine_name, cfile_name, line_number);
	return(allocated);
}

/*
 * NAME:    MEMFree
 *      
 * PURPOSE: Same as free(), but registers activity using MEMTrack().
 *
 * USAGE:   void MEMFree(void *to_free, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: None
 *
 * DESCRIPTION: This function provides a wrapper around free which is
 *              called only if MEMTRACK is defined. The wrapper calls
 *              MEMTrack_free which checks to insure that the memory
 *              being freed was allocated and modifies the line for
 *              that block in the memory tracking log file. 
 *
 * Pointers deemed invalid are not free()'d.  Memory blocks that are deemed
 * valid are filled with the FILL_CHAR character before being free()'d. 
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMFree" 

void MEMFree(void  *to_free, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	unsigned long size = MEMTrack_free(to_free, tag, routine_name, cfile_name, line_number);

	if (size != 0 && size != ULONG_MAX)
	{
		memset(to_free, FILL_CHAR, (size_t)size);
		free(to_free);
	}
}

/*
 * NAME:    MEMMalloc
 *      
 * PURPOSE: Same as malloc(), but registers activity using MEMTrack().
 *
 * USAGE:   void *MEMMalloc(size_t number_of_bytes, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: same as malloc() (note that memory area is filled with FILL_CHAR value)
 *
 * DESCRIPTION: This function provides a wrapper around malloc which is
 *              called only if MEMTRACK is defined. The wrapper calls
 *              MEMTrack_alloc which writes out the location and size
 *              of the allocated block of memory along with the tag
 *              given to this function.
 *              The fill of allocated memory with the value 0XCC is
 *              used to clearly indicate allocated memory while in
 *              the debugger. This was suggested by Steve Maguire in
 *              his book "Writing Solid Code".
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMMalloc" 

void *MEMMalloc(size_t bytes, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	void *allocated;
	
	allocated = malloc(bytes);
	MEMTrack_alloc(allocated, bytes, tag, routine_name, cfile_name, line_number);
	if (allocated != NULL)
		memset(allocated, FILL_CHAR, bytes);
	return(allocated);
}

/*
 * NAME:    MEMRealloc
 *      
 * PURPOSE: Same as realloc(), but registers activity using MEMTrack().
 *
 * USAGE:   void *MEMRealloc(void *allocated, size_t bytes, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: same as realloc()
 *
 * DESCRIPTION: This function provides a wrapper around realloc which is
 *              called only if MEMTRACK is defined. The wrapper calls
 *              MEMTrack_alloc which writes out the location and size
 *              of the allocated block of memory along with the tag
 *              given to this function.
 *
 * If the memory block is being expanded, then the expanded tail is filled
 * with FILL_CHAR.  If the memory block is being shrinked, then
 *
 * Logs the reallocation as first a free of the address then a new allocation
 * (regardless if the memory area is simply resized or moved).
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:         
 *
 * ERRORS:  
 *
 */
void *MEMRealloc(void *allocated, size_t bytes, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	void *old_allocated = allocated;
	unsigned long old_size = MEMTrack_free(old_allocated, tag, routine_name, cfile_name, line_number);
	
	allocated = realloc(allocated, bytes);
	if (allocated)
	{
		MEMTrack_alloc(allocated, bytes, tag, routine_name, cfile_name, line_number);
		
		if (old_size > bytes)
			memset((char *)allocated + old_size, FILL_CHAR, (size_t)(old_size - bytes));
	}
	return(allocated);
}

/*
 * NAME:    MEMStrdup
 *      
 * PURPOSE: Same as strdup(), but registers activity using MEMTrack().
 *
 * USAGE:   char *MEMStrdup(void *allocated, size_t bytes, char *tag, char *routine_name, char *cfile_name, int line_number)
 *
 * RETURNS: same as strdup()
 *
 * DESCRIPTION: This function provides a wrapper around strdup which is
 *              called only if MEMTRACK is defined. The wrapper calls
 *              MEMTrack_alloc which writes out the location and size
 *              of the allocated block of memory along with the tag
 *              given to this function.
 *              The fill of allocated memory with the value 0XCC is
 *              used to clearly indicate allocated memory while in
 *              the debugger. This was suggested by Steve Maguire in
 *              his book "Writing Solid Code".
 *
 * Logs a warning if:
 * 1) the memory allocation of the string to be duplicated was not logged
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
char *MEMStrdup(const char *string, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char *allocated;
	char msg[MAX_LTH];
	
	if (!MEMTrack_find((char *)string, tag, routine_name, cfile_name, line_number)) {
		sprintf(msg, "Duplicating untracked string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	allocated = (char *)strdup((char *)string); /* cast to satisfy unix; and Mac -rf01 */
	MEMTrack_alloc(allocated, strlen(string) + 1, tag, routine_name, cfile_name, line_number);
	return(allocated);
}

/*
 * NAME:    MEMStrcpy
 *      
 * PURPOSE: Same as strcpy(), but warns for some memory errors.
 *
 * USAGE:   MEMStrcpy(char *s, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strcpy()
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around strcpy which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) >= allocation size of s (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:  strlen(ct) may dump core if ct is not properly NULL-terminated
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrcpy" 

char *MEMStrcpy(char *s, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR)
	{
		sprintf(msg,"copying FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if (s == NULL)
	{
		sprintf(msg,"copying into NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0)
	{
		sprintf(msg, "copying %lu bytes into untracked string:%lx (%s)", (unsigned long)strlen(ct), s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}


	if (buffer_size && buffer_size <= strlen(ct))
	{
		sprintf(msg,"copying %lu bytes into %lu byte buffer (%s)", (unsigned long)strlen(ct), buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	return(strcpy(s, ct));
}

/*
 * NAME:    MEMStrncpy
 *      
 * PURPOSE: Same as strncpy(), but warns for some memory errors.
 *
 * USAGE:   MEMStrncpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strncpy()
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around strncpy which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) or n (which ever is less) >= allocation size of s (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:  strlen(ct) may dump core if ct is not a properly NULL-terminated string
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrncpy" 

char *MEMStrncpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"copying FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"copying into NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"copying %lu bytes into untracked string:%lx (%s)", (unsigned long)(n < strlen(ct) ? n : strlen(ct)), s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	if (buffer_size && buffer_size <= (n < strlen(ct) ? n : strlen(ct))) {
		sprintf(msg,"copying up to %lu bytes into %lu byte buffer (%s)", (unsigned long)(n < strlen(ct) ? n : strlen(ct)), buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	return(strncpy(s, ct, n));
}

/*
 * NAME:    MEMStrcat
 *      
 * PURPOSE: Same as strcat(), but warns for some memory errors.
 *
 * USAGE:   MEMStrcat(char *s, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strcat()
 *
 * DESCRIPTION:	This function provides a wrapper around strcat which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of
 * ct plus s.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) >= allocation size of s minus strlen(s) (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:  strlen({ct,s}) may dump core if ct or s are not properly NULL-terminated
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrcat"

char *MEMStrcat(char *s, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"concatenating FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"concatentating onto NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"concatenating %lu bytes onto untracked string:%lx (%s)", (unsigned long)strlen(ct), s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	if (buffer_size && buffer_size <= strlen(ct) + strlen(s)) {
		sprintf(msg,"concatenating %lu bytes onto %lu byte buffer from offset %lu (%s)", (unsigned long)strlen(ct), buffer_size, (unsigned long)strlen(s), tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	return(strcat(s, ct));
}

/*
 * NAME:    MEMStrncat
 *      
 * PURPOSE: Same as strncat(), but checks for some memory errors.
 *
 * USAGE:   MEMStrncat(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strncat()
 *
 * DESCRIPTION:	This function provides a wrapper around strncat which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of
 * ct plus s.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) or n (which ever is less) >= allocation size of s minus strlen(s) (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:  strlen({ct,s}) may dump core if ct or s are not properly NULL-terminated
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrncat"

char *MEMStrncat(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"concatenating FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"concatentating onto NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"concatenating %lu bytes onto untracked string:%lx (%s)", (unsigned long)(n < strlen(ct) ? n : strlen(ct)), s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	if (buffer_size && buffer_size <= (n < strlen(ct) ? n : strlen(ct)) + strlen(s)) {
		sprintf(msg,"concatenating %lu bytes onto %lu byte buffer from offset %lu (%s)", (unsigned long)(n < strlen(ct) ? n : strlen(ct)), buffer_size, (unsigned long)strlen(s), tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}

	return(strncat(s, ct, n));
}

/*
 * NAME:    MEMStrcmp
 *      
 * PURPOSE: Same as strcmp(), but checks for some memory errors.
 *
 * USAGE:   int MEMStrcmp(const char *cs, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: Same as strcmp().
 *
 * DESCRIPTION:	This function provides a wrapper around strcmp which
 *						is called only if MEMTRACK is defined. The wrapper checks to
 *						make sure that the strings being compared are allocated and filled.
 *              
 * Logs a warning if:
 * 1) cs is NULL
 * 2) first character of cs is FILL_CHAR
 * 3) ct is NULL
 * 4) first character of ct is FILL_CHAR
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrcmp" 

int MEMStrcmp(const char *cs, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	
	if(cs == NULL){
		sprintf(msg,"comparing NULL string (cs) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*cs == FILL_CHAR){
		sprintf(msg,"comparing FILL_CHAR string (cs) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(ct == NULL){
		sprintf(msg,"searching NULL string (ct) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*ct == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (ct) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	return(strcmp(cs, ct));
}

/*
 * NAME:    MEMStrncmp
 *      
 * PURPOSE: Same as strncmp(), but checks for some memory errors.
 *
 * USAGE:   int MEMStrncmp(const char *cs, const char *ct, size_t bytes, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: Same as strncmp().
 *
 * DESCRIPTION:	This function provides a wrapper around strncmp which
 *						is called only if MEMTRACK is defined. The wrapper checks to
 *						make sure that the strings being compared are allocated and filled.
 *              
 * Logs a warning if:
 * 1) cs is NULL
 * 2) first character of cs is FILL_CHAR
 * 3) ct is NULL
 * 4) first character of ct is FILL_CHAR
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrncmp" 

int MEMStrncmp(const char *cs, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	
	if(cs == NULL){
		sprintf(msg,"comparing NULL string (cs) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*cs == FILL_CHAR){
		sprintf(msg,"comparing FILL_CHAR string (cs) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(ct == NULL){
		sprintf(msg,"searching NULL string (ct) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*ct == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (ct) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	return(strncmp(cs, ct, n));
}

/*
 * NAME:    MEMStrchr
 *      
 * PURPOSE: Same as strchr(), but checks for some memory errors.
 *
 * USAGE:   char *MEMStrchr(const char *cs, int c, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strchr()
 *
 * DESCRIPTION:	This function provides a wrapper around strchr which
 *						is called only if MEMTRACK is defined. The wrapper makes several
 *						memory checks: it checks to make sure that the string being searched
 *						is not NULL, and that that string is not pointing to memory
 *						which is allocated, but not filled.
 *              
 * Logs a warning if:
 * 1) cs is NULL
 * 2) first character of cs is FILL_CHAR
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrchr" 

char *MEMStrchr(const char *cs, int c, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	
	if(cs == NULL){
		sprintf(msg,"searching NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*cs == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	return((char *)strchr(cs, c));
}

/*
 * NAME:    MEMStrrchr
 *      
 * PURPOSE: Same as strrchr(), but checks for some memory errors.
 *
 * USAGE:   char *MEMStrrchr(const char *cs, int c, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strrchr()
 *
 * DESCRIPTION:	This function provides a wrapper around strrchr which
 *						is called only if MEMTRACK is defined. The wrapper makes several
 *						memory checks: it checks to make sure that the string being searched
 *						is not NULL, and that that string is not pointing to memory
 *						which is allocated, but not filled.
 *              
 * Logs a warning if:
 * 1) cs is NULL
 * 2) first character of cs is FILL_CHAR
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrrchr" 

char *MEMStrrchr(const char *cs, int c, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	
	if(cs == NULL){
		sprintf(msg,"searching NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*cs == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	return((char *)strrchr(cs, c));
}

/*
 * NAME:    MEMStrstr
 *      
 * PURPOSE: Same as strstr(), but checks for some memory errors.
 *
 * USAGE:   char *MEMStrstr(const char *cs, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as strstr()
 *
 * DESCRIPTION:	This function provides a wrapper around strchr which
 *						is called only if MEMTRACK is defined. The wrapper makes several
 *						memory checks: it checks to make sure that the string being searched
 *						is not NULL, and that that string is not pointing to memory
 *						which is allocated, but not filled.
 *              
 * Logs a warning if:
 * 1) cs is NULL
 * 2) first character of cs is FILL_CHAR
 * 3) ct is NULL
 * 4) first character of ct is FILL_CHAR
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMStrstr" 

char *MEMStrstr(const char *cs, const char *ct, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	char msg[MAX_LTH];
	
	if(cs == NULL){
		sprintf(msg,"searching NULL string (cs) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*cs == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (cs) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(ct == NULL){
		sprintf(msg,"searching NULL string (ct) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	if(*ct == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (ct) (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	return((char *)strstr(cs, ct));
}

/*
 * NAME:    MEMMemcpy
 *      
 * PURPOSE: Same as memcpy(), but checks for some memory errors.
 *
 * USAGE:   void *MEMMemcpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as memcpy()
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around memcpy which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) > allocation size of s (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMMemcpy" 

#ifdef SUNCC
char *MEMMemcpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
#endif
#ifdef CCMSC
void *MEMMemcpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
#endif
#ifdef CCLSC	/* Mac memcpy returns void* -rf01 */
void *MEMMemcpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number) /* -rf01 */
#endif /* -rf01 */

{
	char msg[MAX_LTH];
	unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"copying FILL_CHAR buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"copying into NULL buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"copying %lu bytes into untracked buffer:%lx (%s)", (unsigned long)n, s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if (buffer_size && buffer_size < n) {
		sprintf(msg,"copying %lu bytes into %lu byte buffer (%s)", (unsigned long)n, buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return((char *)memcpy(s, ct, n));
}

/*
 * NAME:    MEMMemmove
 *      
 * PURPOSE: Same as memmove(), but checks for some memory errors.
 *
 * USAGE:  void *MEMMemmove(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as memmove()
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around memmove which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) > allocation size of s (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMMemmove" 

#ifdef SUNCC
char *MEMMemmove(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
#endif
#ifdef CCMSC
void *MEMMemmove(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)
#endif
#ifdef CCLSC	/* Mac memmove returns void* -rf01 */
void *MEMMemmove(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number) /* -rf01 */
#endif /* -rf01 */
{
	char msg[MAX_LTH];
  unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"moving FILL_CHAR buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"moving into NULL buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"moving %lu bytes into untracked buffer:%lx (%s)", (unsigned long)n, s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if (buffer_size && buffer_size < n) {
		sprintf(msg,"moving %lu bytes into %lu byte buffer (%s)", (unsigned long)n, buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return((char *)memmove(s, ct, n));
}

/*
 * NAME:    _fMEMStrchr
 *      
 * PURPOSE: Same as _fstrchr(), but checks for some memory errors.  Calls MEMStrchr
 * if SUNCC defined.
 *
 * USAGE:   char __far * __far _fMEMStrchr(const char __far *cs, int ch, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as _fstrchr() (strchr() if SUNCCC defined)
 *
 * DESCRIPTION:	This function provides a wrapper around _fstrchr which
 *						is called only if MEMTRACK is defined. The wrapper makes several
 *						memory checks: it checks to make sure that the string being searched
 *						is not NULL, and that that string is not pointing to memory
 *						which is allocated, but not filled.
 *              
 * Logs a warning if:
 * 1) cs is NULL
 * 2) first character of cs is FILL_CHAR
 *              
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "_fMEMStrchr" 

#ifdef CCMSC
char __far * __far _fMEMStrchr(const char __far *cs, int c, char *tag, char *routine_name, char *cfile_name, int line_number)

{
	char msg[MAX_LTH];
	
	if(cs == NULL){
		sprintf(msg,"searching NULL string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if(*cs == FILL_CHAR){
		sprintf(msg,"searching FILL_CHAR string (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return(_fstrchr(cs, c));
}
#endif

#ifdef SUNCC
char *_fMEMStrchr(char *cs, int c, char *tag, char *routine_name, char *cfile_name, int line_number)

{	
	return(MEMStrchr(cs, c, tag, routine_name, cfile_name, line_number));
}
#endif

/*
 * NAME:    _fMEMMemcpy
 *      
 * PURPOSE: Same as _fmemcpy(), but checks for some memory errors.  Calls MEMMemcpy()
 * if SUNCC defined.
 *
 * USAGE:   void __far __far * _fMEMMemcpy(__far char *s, const _far char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as _fmemcpy() (memcpy() if SUNCC defined)
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around _fmemcpy which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) > allocation size of s (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "_fMEMMemcpy" 

#ifdef CCMSC
void __far * __far _fMEMMemcpy(char __far *s, const char __far *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)

{
	char msg[MAX_LTH];
	unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"copying FILL_CHAR buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"copying into NULL buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"copying %lu bytes into untracked buffer:%lx (%s)", (unsigned long)n, s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if (buffer_size && buffer_size < n) {
		sprintf(msg,"copying %lu bytes into %lu byte buffer (%s)", (unsigned long)n, buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return(_fmemcpy(s, ct, n));
}

#endif

#ifdef SUNCC
void  *_fMEMMemcpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)

{
	return(MEMMemcpy(s, ct, n, tag, routine_name, cfile_name, line_number));
}
#endif

#ifdef CCLSC	/* define this for Mac -rf02 */
void  *_fMEMMemcpy(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)

{
	return(MEMMemcpy(s, ct, n, tag, routine_name, cfile_name, line_number));
}
#endif	/* define this for Mac -rf02 */

/*
 * NAME:    _fMEMMemmove
 *      
 * PURPOSE: Same as _fmemmove(), but checks for some memory errors.  Calls MEMMemmove()
 * if SUNCC defined.
 *
 * USAGE:  void __far __far *_fMEMMemmove(__far char *s, const __far char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as _fmemmove (memmove() if SUNCC defined)
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around _fmemmove which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) first character of ct is FILL_CHAR
 * 2) s is NULL
 * 3) memory allocation of s is not tracked in log file
 * 4) strlen(ct) > allocation size of s (when tracked)
 *  
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "_fMEMMemmove" 

#ifdef CCMSC
void __far * __far _fMEMMemmove(char __far *s, const char __far *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)

{
	char msg[MAX_LTH];
  unsigned long buffer_size = 0;
	
	if (*ct == FILL_CHAR) {
		sprintf(msg,"moving FILL_CHAR buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
		
	if(s == NULL) {
		sprintf(msg,"moving into NULL buffer (%s)", tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if ((buffer_size = MEMTrack_find(s, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"moving %lu bytes into untracked buffer:%lx (%s)", (unsigned long)n, s, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if (buffer_size && buffer_size < n) {
		sprintf(msg,"moving %lu bytes into %lu byte buffer (%s)", (unsigned long)n, buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return(_fmemmove(s, ct, n));
}
#endif

#ifdef SUNCC
void  *_fMEMMemmove(char *s, const char *ct, size_t n, char *tag, char *routine_name, char *cfile_name, int line_number)

{
	return(MEMMemmove(s, ct, n, tag, routine_name, cfile_name, line_number));
}
#endif

/*
 * NAME:    MEMRead
 *      
 * PURPOSE: Same as read(), but checks for some memory errors.
 *
 * USAGE:  unsigned int MEMRead(int handle, void *buffer, unsigned int count, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as read()
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around memread which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) memory allocation of buffer is not tracked in log file
 * 2) count > allocation size of buffer (if tracked)
 *
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMRead" 

int MEMRead(int handle, void *buffer, unsigned int count, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	unsigned long buffer_size = 0;
	char msg[MAX_LTH];

	if ((buffer_size = MEMTrack_find(buffer, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"reading %lu bytes into untracked buffer:%lx (%s)", (unsigned long)count, buffer, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if (buffer_size && buffer_size < count) {
		sprintf(msg,"reading %lu bytes into %lu byte buffer (%s)", (unsigned long)count, buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return(read(handle, buffer, count));
}

/*
 * NAME:    MEMMemset
 *      
 * PURPOSE: Same as memset(), but checks for some memory errors.
 *
 * USAGE:  void  *MEMMemset(void *dest, int c, unsigned int count, char *tag, char *routine_name, char *cfile_name, int line_number);
 *
 * RETURNS: same as memset()
 * 
 *
 * DESCRIPTION:	This function provides a wrapper around memset which is called only if
 * MEMTRACK is defined.  Furthermore, checks memtrack file for previous allocation and
 * checks sizes for fit.  Warns if FILL_CHAR is first character of ct, if address of
 * cs cannot be found in memory log, or if given allocated size is less than size of ct.
 *              
 * Logs a warning if:
 * 1) memory allocation of buffer is not tracked in log file
 * 2) count > allocation size of buffer (if tracked)
 *
 * AUTHOR:  Adapted from an article in the August 1990 issue of Dr. Dobbs
 *          Journal (Copyright (c) 1990, Cornerstone Systems Group, Inc.)
 *          by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:This function is only called if MEMTRACK is defined.  If environmental
 * variable MEMTRACK is not defined then memory log tracking is disabled.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "MEMMemset" 

void *MEMMemset(void *dest, int c, unsigned int count, char *tag, char *routine_name, char *cfile_name, int line_number)
{
	unsigned long buffer_size = 0;
	char msg[MAX_LTH];

	if ((buffer_size = MEMTrack_find(dest, tag, routine_name, cfile_name, line_number)) == 0) {
		sprintf(msg,"setting %lu bytes in untracked buffer:%lx (%s)", (unsigned long)count, dest, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	if (buffer_size && buffer_size < count) {
		sprintf(msg,"setting %lu bytes in %lu byte buffer (%s)", (unsigned long)count, buffer_size, tag);
		MEMTrack_msg(msg, routine_name, cfile_name, line_number);
	}
	
	return(memset(dest, c, count));
}

