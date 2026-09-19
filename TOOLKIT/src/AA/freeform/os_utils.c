/* 
 * FILENAME: os_utils.c
 *
 * CONTAINS:	
 * os_strlwr
 * os_strupr
 * os_filelength
 * os_get_env
 * os_strcmpi
 * os_strncmpi
 * os_itoa
 * os_file_exist
 * os_open
 * os_close
 * os_mac_load_env		-rf01
 * os_path_cmp_paths
 * os_path_is_native
 * os_path_make_native
 * os_path_return_ext
 * os_path_find_parts
 * os_path_get_parts
 * os_path_put_parts
 * os_str_replace_char
 * os_path_prepend_special
 * os_str_trim_whitespace
 * os_str_replace_unescaped_char1_with_char2(char char1, char char2, char *str)
 *	This file contains OS utilities for portability across the sun,
 *	the mac and the pc, SUNCC, CCLSC, CCMSC respectively 
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	7/28/95		-rf01
 *		Include string.h and stdlib.h to fix memory stomps in CW
 *	Rich Fozzard	9/25/95		-rf02
 *		Include unix.h for CW
*/

#ifdef XVT
#include <xvt.h>
#endif

#ifdef _WINDOWS
#include <windows.h>
#endif

#include <freeform.h>
#include <os_utils.h>

#ifdef CCLSC	/* global environment variable buffer for Mac -rf01*/
char prefsbuffer[4096];	/* assuming we have a small file to read */
#endif

/*
 * NAME:	os_file_exist	
 *		
 * PURPOSE: to determine if a given file exists	
 *
 * USAGE:	os_file_exist( char * filenname)
 *
 * RETURNS: TRUE if the file exists, FALSE if not
 *
 * DESCRIPTION:	If compiled with XVT libraries (as signaled by having the
 * preprocessor macro XVT defined) then call xvt_fsys_get_file_attr() to
 * determine file existence, otherwise call the stat() function.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	stat()
 *
 * GLOBALS:	
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS: 
 *
 * KEYWORDS:	
 *
 */

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "os_file_exist"

#ifdef PROTO
BOOLEAN os_file_exist(char *filename)
#else
BOOLEAN os_file_exist(filename)
char *filename;
#endif

{
#ifdef XVT

	FILE_SPEC file;
	char path[_MAX_PATH];
	
	/* A more conservative assertion would subtract the length of the file name
	   component of filename, but I think this is just fine. */
	assert(strlen(filename) < sizeof(path));
	
	(void)os_path_get_parts(filename, path, NULL, NULL);
	if (strlen(path) && !xvt_fsys_convert_str_to_dir(path, &file.dir))
	{
#ifdef DEBUG_MSG
		xvt_dm_post_error("\"%s\"\n\nError converting path from %s into a directory structure.  Exiting %s() with failure", path, filename, ROUTINE_NAME);
#endif
		return(FALSE);
	}
	else if (!strlen(path))
		xvt_fsys_get_default_dir(&file.dir);
	
	strncpy(file.name, os_path_return_name(filename), sizeof(file.name) - 1);
	file.name[sizeof(file.name) - 1] = STR_END;
	
	return(xvt_fsys_get_file_attr(&file, XVT_FILE_ATTR_EXIST) == TRUE ? TRUE : FALSE);

#else

#ifndef CCLSC
	struct stat buf;
	
	return(stat(filename, &buf) != -1 ? TRUE : FALSE);
#else
#error Need to write non-XVT Mac file exists function
#endif

#endif
}

/*
 * NAME:		os_strlwr
 *		
 * PURPOSE:	convert the string to low case
 *
 * USAGE:	char *os_strlwr( char *)
 *
 * RETURNS:	the converted string
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	
 *
 * AUTHOR:	LPD
 *
 * COMMENTS:	To replace the function strlwr which is not portable to MAC
 *
 * KEYWORDS:	
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_strlwr"

#ifdef PROTO
char *os_strlwr( char *string)
#else
char *os_strlwr(string)
char *string;
#endif	
{ 
#ifdef CCLSC /* for the mac */

	char *temp = string;
	
	while(*temp != '\0'){
		if(*temp >= 'A' && *temp <= 'Z')
			*temp=*temp + 'a' - 'A'; 
		temp++;
	}

#elif defined SUNCC /* for the sun */

	char *temp = string;
	do
	{
		*temp = (isascii(*temp))? tolower(*temp): *temp;
	} while(*(++temp) != '\0');

#else /* for the pc */ 

	char *temp = string;
	strlwr(temp);

#endif

	return(string);

} /* END OS_STRLWR */

/*
 * NAME:		os_strupr
 *		
 * PURPOSE:	convert the string to upper case
 *
 * USAGE:	char *os_strupr( char *)
 *
 * RETURNS:	the converted string
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	
 *
 * AUTHOR:	LPD
 *
 * COMMENTS:	To replace the function strlwr which is not portable to MAC
 *
 * KEYWORDS:	
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_strupr"

#ifdef PROTO
char *os_strupr( char *string)
#else
char *os_strupr(string)
char *string;
#endif	
{ 
#ifdef CCLSC /* for the mac */

	char *temp = string;
	
	while(*temp != '\0'){
		if(*temp >= 'a' && *temp <= 'z')
			*temp=*temp - 'a' + 'A'; 
		temp++;
	}

#elif defined SUNCC /* for the sun */

	char *temp = string;
	do
	{
		*temp = (isascii(*temp))? toupper(*temp): *temp;
	}while(*(++temp) != '\0');

#else /* for the pc */ 

	char *temp = string;
	strupr(temp);

#endif

	return(string);

} /* END OS_STRLWR */

/*
 * NAME:		os_filelength
 *		
 * PURPOSE:		to get the file length
 *
 * USAGE:		long os_filelength(handle)
 *
 * RETURNS:	-1 on error, file size otherwise
 *
 * DESCRIPTION:	If compiled with XVT libraries (as signaled by having the
 * preprocessor macro XVT defined) then call xvt_fsys_get_file_attr() to
 * determine file length, otherwise call the stat() function.
 *
 * SYSTEM DEPENDENT FUNCTIONS: stat()
 *
 * GLOBALS:	
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS: 
 *
 * KEYWORDS:	
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	12/12/95		-rf01
 *		use stat to determine length, since XVT Mac can't get size
 *		of files opened read-write (known XVT bug) The trick is that we 
 *		need to use a different "stat" when using CodeWarrior.	
*/


#undef ROUTINE_NAME
#define ROUTINE_NAME "os_filelength" 

#ifdef PROTO
long os_filelength(char *filename)
#else
long os_filelength(filename)
char *filename;
#endif
{

#ifdef XVT
	long filelen; 	/* use separate return variable, since we have different ways of filling it -rf01 */

/* use stat to determine length, since XVT Mac can't get size
		of files opened read-write -rf01 */
#if XVTOS == MACOS

#if XVT_CC == XVT_CC_MACCW	/* CodeWarrior uses a strange struct stat definition (see CW <stat.h>) -rf01 */
/*
 *	Local typedefs for statCW struct
 */
typedef unsigned long	mode_t;
typedef unsigned long	ino_t;
typedef unsigned long	dev_t;
typedef short			nlink_t;
typedef unsigned long	uid_t;
typedef unsigned long	gid_t;
typedef long			off_t;
	struct statCW
	{
		mode_t		st_mode;		/* File mode; see #define's below */
		ino_t		st_ino;			/* File serial number */
		dev_t		st_dev;			/* ID of device containing this file */
		nlink_t		st_nlink;		/* Number of links */
		uid_t		st_uid;			/* User ID of the file's owner */
		gid_t		st_gid;			/* Group ID of the file's group */
		dev_t		st_rdev;		/* Device type */
		off_t		st_size;		/* File size in bytes */
		time_t		st_atime;		/* Time of last access */
		time_t		st_mtime;		/* Time of last data modification */
		time_t		st_ctime;		/* Time of last file status change */
		long		st_blksize;		/* Optimal blocksize */
		long		st_blocks;		/* blocks allocated for file */
	};
	struct statCW buffer;
#else	/* else use the normal struct stat for others (ie Symantec) -rf01 */
	struct stat buffer;
#endif

	if (stat(filename, (struct stat *)(&buffer)) == -1) /* have to cast pointer, since struct is different */
	{
		err_push(ROUTINE_NAME, errno, "Unable to get file attributes");
		filelen = (long)-1;
	}
	else
		filelen = (long)(buffer.st_size);
	
#else	/* else not Mac -rf01 */

	FILE_SPEC file;
	char path[_MAX_PATH];
	
	/* A more conservative assertion would subtract the length of the file name
	   component of filename, but I think this is just fine. */
	assert(strlen(filename) < sizeof(path));
	
	(void)os_path_get_parts(filename, path, NULL, NULL);
	if ( strlen(path) && !xvt_fsys_convert_str_to_dir(path, &(file.dir)))
	{
#ifdef DEBUG_MSG
		xvt_dm_post_error("\"%s\"\n\nError converting path from %s into a directory structure.  Exiting %s() with failure", path, filename, ROUTINE_NAME);
#endif
		return(FALSE);
	}
	else if (!strlen(path))
		xvt_fsys_get_default_dir(&(file.dir));
	
	strncpy(file.name, os_path_return_name(filename), sizeof(file.name) - 1);
	file.name[sizeof(file.name) - 1] = STR_END;
	filelen = (long)xvt_fsys_get_file_attr(&file, XVT_FILE_ATTR_SIZE);
	
#endif	/* endif else not Mac -rf01 */
	
	return(filelen); 	/* use separate return variable, since we have different ways of filling it -rf01 */

#else

#ifndef CCLSC
	struct stat buffer;

	if (stat(filename, &buffer) == -1)
	{
		err_push(ROUTINE_NAME, errno, "Unable to get file attributes");
		return ((long)-1);
	}
	else
		return((long)(buffer.st_size));

#else
#error Need to write non-XVT Mac file length function
#endif

#endif
}/* END OS_FILELENGTH */

/*
 * NAME:		os_strcmpi
 *		
 * PURPOSE:	case-insensitive versions of strcmp	
 *
 * USAGE:	int os_strcmpi(const char *s1, const char *s2)	
 *
 * RETURNS:	< 0 	if s1 < s2
 *		= 0	if s1 identical to s2(except case) 
 *		> 0	fi s1 > s2
 *
 * DESCRIPTION:
 *
 * SYSTEM DEPENDENT FUNCTIONS: strcmpi()	
 *				using tolower and toupper is more portable then
 *				referencing the ascii characters	
 *
 * GLOBALS:	
 *
 * AUTHOR:	tam
 *
 * COMMENTS: 
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_strcmpi"

#ifdef PROTO
int os_strcmpi(const char* s1, const char* s2)
#else
int os_strcmpi(s1, s2)
const char* s1;
const char* s2;
#endif
{

#ifdef CCMSC

	return(strcmpi(s1, s2));

#elif SUNCC

	return(strcasecmp(s1, s2));

#else

	for( ; tolower((int)(*s1)) == tolower((int)(*s2)); s1++, s2++)
		if (*s1 == '\0')
			return(0);

	return(*s1 - *s2);

#endif

} /* END OS_STRCMPI */

/*
 * NAME:		os_strncmpi
 *		
 * PURPOSE:	case-insensitive versions of strncmp	
 *
 * USAGE:	int os_strncmpi(const char *s1, const char *s2, size_t n)	
 *
 * RETURNS:	< 0 	if s1 < s2
 *		= 0	if s1 identical to s2(except case) 
 *		> 0	fi s1 > s2
 *
 * DESCRIPTION:
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:	
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS: 
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_strcmpi"

#ifdef PROTO
int os_strncmpi(const char* s1, const char* s2, size_t n)
#else
int os_strncmpi(s1, s2, n)
const char* s1;
const char* s2;
size_t n;
#endif
{

#ifdef CCMSC

	return(strnicmp(s1, s2, n));

#elif SUNCC

	return(strncasecmp(s1, s2, n));

#else      

	int i;
	for(i = 0; i < n && tolower((int)(*s1)) == tolower((int)(*s2)); i++, s1++, s2++)
		if (*s1 == '\0')
			return(0);

	if (i == n)
		return(0);
	else
		return(*s1 - *s2);

#endif

} /* END OS_STRNCMPI */

#ifdef CCLSC
/*
 * NAME:		os_mac_load_env
 *		
 * PURPOSE:	To fill global Mac environment variable linked list
 *
 * USAGE:	void * os_mac_load_env(char * buffer)
 *
 * RETURNS:	nothing.
 *
 * DESCRIPTION:This function fills an environment variable linked list for 
 *			  the MAC case. It opens a preferences file which has 
 *			 variables defined using a NAME=value syntax.  *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	
 *
 * AUTHOR:	Rich Fozzard, NGDC, (303) 497 - 6764, fozzard@ngdc.noaa.gov
 *
 * COMMENTS: called only by main()	
 *
 * KEYWORDS:	
 *
 */
 /*
 * HISTORY:
 *	r fozzard	7/28/95		written 
*/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_mac_load_env"

void *os_mac_load_env(char *buffer)
{
	/* Fill the prefsbuffer from the file (return NULL if couldnt read it) */
	if (ff_file_to_buffer("GeoVu Prefs", buffer) <= 0)
		return NULL;
} /* END os_mac_load_env */


/*
 * NAME:	PathNameFromFSSpec
 *		
 * PURPOSE:	To obtain full path from FSSpecPtr on a Mac.
 *
 * USAGE:	Handle PathNameFromFSSpec(FSSpecPtr myFSSPtr)
 *
 * RETURNS:	A handle to a string that contains the full path to the specified file.
 *
 * DESCRIPTION:	Return a handle to the full pathname of the file specified
 *                  by FSSpecPtr. 
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	
 *
 * AUTHOR:	Tom Carey, adapted from code by Theodore W. Liz‰rd
 *
 * COMMENTS: 	
 *
 * KEYWORDS:	
 *
 */

/* #include <Aliases.h> */

Handle PathNameFromFSSpec(FSSpecPtr myFSSPtr)
{
	AliasHandle		alias = nil;
	Handle 			pathName = nil;
	AliasInfoType	aliasType = asiAliasName;	/* Set the index to the parent */
	long			theSize;
	Str63			theString;
	OSErr			error;
	
	/* Create a temporary alias */
	error = NewAlias(nil, myFSSPtr, &alias);
	if (alias == nil) return pathName;
	
	pathName = NewHandleClear(1000); /* max path size of 1000 */

	if (pathName == nil) return pathName;
	
	/* Get the parent name */
	if (GetAliasInfo(alias, aliasType, theString) == noErr) {
		while (*theString) {
			theSize = (long)theString[0] + 1;
			
			/* Use the size byte to store the ':' */
			theString[0] = ':';
			theSize = Munger(pathName, 0, nil, 0, &theString, theSize);
			
			/* Set the index to the next parent */
			aliasType += asiParentName;
			error = GetAliasInfo(alias, aliasType, theString);
		}
		
		error = GetAliasInfo(alias, asiVolumeName, theString);
		theSize = (long)theString[0];
		theSize = Munger(pathName, 0, nil, 0, &theString[1], theSize);
	}
	
	/* Dispose temporary alias */
	DisposeHandle((Handle)alias);
	
	return pathName;
}

#endif

/*
 * NAME:		os_get_env
 *		
 * PURPOSE:	To get the value of a variable defined in the PREFERENCES File
 *
 * USAGE:	char * os_get_env(VARIABLE_NAME)
 *
 * RETURNS:	A pointer to the value of the string.
 *
 * DESCRIPTION:This function gets an environment from DOS or Unix environments. 
 *			 In the MAC case, this function opens a preferences file which has 
 *			 variables defined using a NAME=value syntax. If no matching 
 *			 variable name is found, NULL is returned. In the MS windows case,
 *			 this function also checks the freefrom.ini file in the windows
 *			 directory to find the environment if the environment can not bee
 *			 find in DOS environment. Two applications are defined in 
 *			 freeform.ini including GeoVu and FREEFORM. This function first
 *		   	 searchs the GeoVu application, if does not find, then searchs
 *			 the freeform application.
 *			 The format to define an environment value in the freeform.ini
 *			 is the same as the microsoft private initial file.
 *			 [application_name]
 *			 environmental_variable_name=variable_value
 *                .
 *                .
 *                .
 *			 The X-windows version of initial file should be written in near
 *             future.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	prefsbuffer
 *
 * AUTHOR:	Liping Di, T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS: All functions to get enviornmental variable should call this 
 *		    function.	
 *
 * KEYWORDS:	
 *
 */
 /*
 * HISTORY:
 *	r fozzard	4/21/95		-rf01 
 *		Li Ping used XVT types and functions here, so we need to remove it all
 * 		to get things to compile. Also, this will need complete reworking for the Mac
 *	r fozzard	6/23/95		-rf02
 *		Add code for reading a prefs file to simulate environment vars on Mac
 *	r fozzard	7/19/95		-rf03
 *		Switch from memMalloc to malloc() to avoid infinite recursion into os_get_env
*/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_get_env"

#ifdef PROTO
char *os_get_env(char * variable_name)
#else
char *os_get_env(variable_name)
char * variable_name;
#endif
{
char *variable = NULL;

#ifdef _WINDOWS
	static char geovudir[_MAX_PATH + 1];

	GetPrivateProfileString("GeoVu", variable_name, "not_find", geovudir, _MAX_PATH, "freeform.ini");
	if (strcmp(geovudir, "not_find") == 0)
	{
		GetPrivateProfileString("FREEFORM", variable_name, "not_find", geovudir, _MAX_PATH, "freeform.ini");
		if (strcmp(geovudir, "not_find") == 0)
			variable = NULL;
		else 
			variable = geovudir;
	}
	else
		variable = geovudir;

#endif

#ifdef CCLSC /* for the mac */

/* Li Ping used XVT types and functions here, so we need to remove it all
	to get things to compile. Also, this will need complete reworking for the Mac	-rf01 */
	
	/* New code to simulate environment variables with a "GeoVu Prefs" file
		in the default directory where GeoVu lives. The file uses the same 
		NAME=VALUE format as with WINDOWS .ini files. This is a simple
		text file that any word processor can edit. -rf02 */
		
	char *varname;			/* pointer to the env var name in prefsbuffer */
	char *varvalue;			/* pointer to the env var value in prefsbuffer */
	size_t varlength;		/* calculated length of the var value in bytes */
	int i; OSErr error;

	variable = (char *)malloc(14);	/* dummy */
	
	/* Search for the var (return NULL if not found */
	if ((varname = strstr(prefsbuffer, variable_name)) == NULL)
		return NULL;
	/* The var value is just past the '=' */
	varvalue = strchr(varname, '=') + 1;
	while (*varvalue == ' ')
		++varvalue; /* skip spaces */
	/* Figure the length of the var's value string by looking for the EOL */
	varlength = strcspn(varvalue, "\n");
	/* Allocate space for the var, fill it, and mark the end of string */
	variable = (char *)malloc(varlength + 1);	/* Switch from memMalloc to malloc() 
									to avoid infinite recursion into os_get_env */
	if (variable == NULL)
		return(NULL);

	error = MemError();
	if (error != 0)
		return(NULL);
	
	memcpy(variable, varvalue, varlength);
	variable[varlength] = '\0';
	
	return(variable);

	/* end of New code -rf02 */
	
#else
	if (!variable)
		variable = getenv(variable_name);
#endif

	if (variable)
		return((char *)strdup(variable));
	else
		return(NULL);

} /* END OS_GET_ENV */

/*
 * NAME:		os_itoa
 *		
 * PURPOSE:	convert an integer to a string
 *
 * USAGE:       char *os_itoa(int value, char* string, int radix)
 *
 * RETURNS:     a pointer to string	
 *
 * DESCRIPTION:
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:	
 *
 * AUTHOR:	mvg
 *
 * COMMENTS: 
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_itoa"

#ifdef PROTO
char *os_itoa(int value, char *string, int radix)
#else
char *os_itoa(value, string, radix)
int value;
char *string;
int radix;
#endif
{
#ifdef CCMSC /* for the pc */

	assert(string);

	return(itoa(value, string, radix));

#else /* for the sun */

	int i, sign, temp, j;
	
	assert(string);
	assert(radix == 10);

	if((sign = value) < 0)   /* record sign */
		value = -value;      /* make n positive */
	i = 0;
	do {		     /* generate digits in reverse order */
		string[i++] = value % 10 + '0';   /* get next digit */
	} while ((value /= 10) > 0);   /* delete it */
	if (sign < 0)
		string[i++] = '-';
	string[i] = '\0';
	/* reverse the string in place */
	for (i = 0, j = strlen(string) - 1; i < j; i++, j--) {	
		temp = string[i];
		string[i] = string[j];
		string[j] = temp;
	}
	return(string);
#endif

} /* END os_itoa */

/*****************************************************************************
 * NAME:  os_path_cmp_paths()
 *
 * PURPOSE:  compare two paths, ignoring different directory separators
 *
 * USAGE:  if (os_path_cmp_paths(path1, path2) == 0) true clause here;
 *
 * RETURNS:  an integer less than zero if path1 is lexicographically less
 * than path2, zero if path1 is identical to path2, or an integer greater
 * than zero of path1 is lexicographically greater than path2, with the
 * caveat that differences to different directory separator characters are
 * ignored.
 *
 * DESCRIPTION:  Pair-wise character comparisons are performed between path1
 * and path2 progressing through each string, until a difference is found or
 * both NULL-terminators are found in coincidence.  In comparisons, if a pair
 * of characters both belong to the set UNION_DIR_SEPARATORS then that
 * comparison is not performed.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  NONE
 *
 * GLOBALS:  NONE
 *
 * COMMENTS:  
 *
 * KEYWORDS:  UNION_DIR_SEPARATORS, directory separator lenient string comparisons
 *
 * ERRORS:
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_cmp_paths"

#ifdef PROTO
int os_path_cmp_paths(char *s, char *t)
#else
int os_path_cmp_paths(s, t)
char *s, *t;
#endif

{
	size_t shortest_length,
	               i = 0;
	
	assert(s && t);
	
	if (s == NULL && t == NULL)
		return(0);
	else if (s == NULL)
		return(-1);
	else if (t == NULL)
		return(1);
	
	shortest_length = min(strlen(s), strlen(t));
	
	for (i = 0; i <= shortest_length; i++)
	{
		if ((unsigned char)s[i] - (unsigned char)t[i])
		{
			if (s[i] == STR_END || t[i] == STR_END)
				return((unsigned char)s[i] - (unsigned char)t[i]);
			
			if (strcspn(s + i, UNION_DIR_SEPARATORS) == 0 &&
			    strcspn(t + i, UNION_DIR_SEPARATORS) == 0)
				continue;
			
			return((unsigned char)s[i] - (unsigned char)t[i]);
		}
	}
	return(0);
}
			
/*****************************************************************************
 * NAME:  os_path_is_native()
 *
 * PURPOSE:  Determine if a native path
 *
 * USAGE:  if (os_path_is_native(unknown_path)) ; else ;
 *
 * RETURNS:  FALSE (0) if not a native path, TRUE (1) if a native path, or if
 * NULL.
 *
 * DESCRIPTION:  If any occurence of a non-native directory separator is
 * found (i.e., the set of the three platform directory separators minus
 * the current platform's native directory separator -- see freeform.h) then
 * FALSE is return.  However, if the current platform is DOS-based, then a
 * driver letter/colon combination is allowed.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:  UNION_DIR_SEPARATORS, NUM_DIR_SEPARATORS, NATIVE_DIR_SEPARATOR
 *
 * ERRORS:
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_is_native"

#ifdef PROTO
BOOLEAN os_path_is_native(char *path)
#else
BOOLEAN os_path_is_native(path)
char *path;
#endif

{
	char foreign_dir_sep[NUM_DIR_SEPARATORS + 1];
	char *temp;
	
	if (path == NULL)
		return(FALSE);

	memStrcpy(foreign_dir_sep, UNION_DIR_SEPARATORS, "foreign_dir_sep, UNION_DIR_SEPARATORS");
	temp = memStrchr(foreign_dir_sep, NATIVE_DIR_SEPARATOR, "foreign_dir_sep, NATIVE_DIR_SEPARATOR");

	/* NATIVE_DIR_SEPARATOR must be an element of UNION_DIR_SEPARATORS */
	if (temp == NULL)
		assert(0);
	
	/* remove NATIVE_DIR_SEPARATOR from foreign_dir_sep, and shift left */
	while (*temp != STR_END)
	{
		*temp = *(temp + 1);
		++temp;
	}
	
#ifdef CCMSC
	if (isalpha(path[0]) && path[1] == ':')
		path += 2;
#endif

	path += strcspn(path, foreign_dir_sep);
	if (*path == STR_END)
		return(TRUE);
	else
		return(FALSE);
}


/*****************************************************************************
 * NAME: os_path_make_native(char *native_path, char *path)
 *
 * PURPOSE:  Make a native path for the current platform
 *
 * USAGE:  native_path = os_path_make_native(native_path, other_path);
 *
 * RETURNS:  NULL if an error in translation, a pointer to the native path
 * otherwise, which is also copied into the native_path parameter.
 *
 * DESCRIPTION:  Any character contained in the set UNION_DIR_SEPARATORS
 * found in path is converted into NATIVE_DIR_SEPARATOR -- exception:  if DOS
 * is the current platform and a DOS drive letter/colon combination is found,
 * then the colon is not converted.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * COMMENTS:  native_path and path may be the same variable in function
 * calls, thus overwriting path, assuming that the calling routine does
 * not need to preserve an original copy.
 *
 * KEYWORDS:  NATIVE_DIR_SEPARATOR, UNION_DIR_SEPARATORS
 *
 * ERRORS:  No space checking is performed on native_path; if native_path is
 * not NULL but points to unallocated or insufficiently allocated space then
 * memory corruption will occur.
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_make_native"

#ifdef PROTO
char *os_path_make_native(char *native_path, char *path)
#else
char *os_path_make_native(native_path, path)
char *native_path, *path;
#endif

{
	size_t next_sep;
	int i = 0;

	if (native_path == NULL)
		return(NULL);

	if (path == NULL)
	{
		native_path[0] = STR_END;
		return(NULL);
	}
	
	if (os_path_is_native(path))
	{ /* simply perform "safe" copy of path onto native_path */
		for (i = strlen(path); i >= 0; i--)
			native_path[i] = path[i];
		return(native_path);
	}

	/* look for DOS driver letter/colon combo */

#ifdef CCMSC	
	if (isalpha(path[0]) && path[1] == ':')
	{
		native_path[0] = path[0];
		native_path[1] = path[1];
		i = 2;
	}
#endif

	while (path[i] != STR_END)
	{
		if ((next_sep = strcspn(path + i, UNION_DIR_SEPARATORS)) != 0)
		{ /* copy interim (non-directory separator) characters */
			next_sep += i; /* next_sep was relative to path + i -- make absolute */
			for (; i < (int)next_sep; i++)
				native_path[i] = path[i];
		}
		/* path has been copied into native_path up to a dir sep or NULL-terminator */
		
		if (path[i] != STR_END)
			native_path[i++] = NATIVE_DIR_SEPARATOR;
	}
	native_path[i] = STR_END;

	return(native_path);	
}

/*****************************************************************************
 * NAME: os_path_find_parts()
 *
 * PURPOSE:  Find directory, file name, and extension components of a path
 *
 * USAGE:  (void)os_path_find_parts(path, &filepath, &filename, &fileext);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  *filepath is made to point to path (if DOS is the current
 * platform, and a drive letter/colon is present, *filepath is advanced past
 * the drive letter and colon.  *filename is made to point one
 * character beyond the last directory separator in path (i.e., any character
 * contained in UNION_DIR_SEPARATORS) -- if path contains no directory
 * separator then *filename is made to point to path (see DOS caveat above).
 * *fileext is made to point one character beyond the last dot ('.') in path
 * unless such a dot is contained in a file path (no file extension, but a
 * directory is named with a dot).  If a qualifying file path, extension, or
 * file name can not be found, then *filepath, *filename or *fileext is made
 * NULL, respectively.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  NONE
 *
 * GLOBALS:  NONE
 *
 * COMMENTS:  
 * 
 * Since *filename is made to point to the file name component of path, any
 * extension will be a part of *filename.  To get (a copy of) the file name
 * component of path w/o extension, instead call os_path_get_parts().
 *
 * It is recommended that os_path_put_parts() be called to construct a new
 * path.
 *
 * KEYWORDS:  UNION_DIR_SEPARATORS, DOS DRIVE LETTER
 *
 * ERRORS:  
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_find_parts"

#ifdef PROTO
void os_path_find_parts(char *path, char **filepath, char **filename, char **fileext)
#else
void os_path_find_parts(path, filepath, filename, fileext)
char *path, **filepath, **filename, **fileext;
#endif

{
	char *temp_cp = path;
	size_t temp_i = 0;
	
	if (path == NULL)
	{
		if (filepath)
			*filepath = NULL;
		if (filename)
			*filename = NULL;
		if (fileext)
			*fileext = NULL;
		return;
	}
	
	if (filepath)
	{
		temp_cp = *filepath = path;
#ifdef CCMSC
		if (isalpha(path[0]) && path[1] == ':')
			temp_cp = *filepath = &path[2];
#endif
	}

	/* Find last (if any) directory separator in path */
	if ((temp_i = strcspn(temp_cp, UNION_DIR_SEPARATORS)) != strlen(temp_cp))
		do
			temp_cp += temp_i + 1;
		while ((temp_i = strcspn(temp_cp, UNION_DIR_SEPARATORS)) != strlen(temp_cp));

	if (filename)
		*filename = temp_cp;

	if (fileext){
		*fileext = memStrrchr(temp_cp, '.', "temp_cp, '.'");
		if (*fileext)
			(*fileext)++;
	}

	return;
}

char *os_path_return_ext(char *pfname)
/*****************************************************************************
 * NAME:  os_path_return_ext()
 *
 * PURPOSE:  Return a pointer to the extension of a path-file name
 *
 * USAGE:  if (strcmp(os_path_return_ext(pfname), test_extension)) found = TRUE;
 *
 * RETURNS:  a pointer to the (last) extension, NULL if no extension
 *
 * DESCRIPTION:  Calls os_path_find_parts() to locate the substring of pfname
 * which follows the last extension delimiter (dot -- '.').
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/
{
	char *temp = NULL;
	
	assert(pfname);
	
	(void)os_path_find_parts(pfname, NULL, NULL, &temp);
	return(temp);
}

char *os_path_return_name(char *pfname)
/*****************************************************************************
 * NAME:  os_path_return_name()
 *
 * PURPOSE:  Return a pointer to the name of a path-file name
 *
 * USAGE:  if (strcmp(os_path_return_name(pfname), test_name)) found = TRUE;
 *
 * RETURNS:  a pointer to the name substring of a path-file name, NULL if no name
 *
 * DESCRIPTION:  Calls os_path_find_parts() to locate the substring of pfname
 * which follows the last directory separator character (e.g., slash -- '/').
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/
{
	char *temp = NULL;
	
	assert(pfname);
	
	(void)os_path_find_parts(pfname, NULL, &temp, NULL);
	return(temp);
}

/*****************************************************************************
 * NAME: os_path_get_parts()
 *
 * PURPOSE:  Return a copy of file path, name, and extension
 *
 * USAGE:  (void)os_path_get_parts(path, filepath, filename, fileext);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  os_path_find_parts() is called to locate the file path, name
 * and extension components of path.  These components are then copied into
 * filepath, filename, and fileext.  The file name and extension is not copied
 * into filepath, and the file extension is not copied into filename.  If
 * components can not be found then zero-length strings are returned.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  NONE
 *
 * GLOBALS:  NONE
 *
 * COMMENTS:  path MUST BE a native path, i.e., with native directory
 * separators.  path is assumed to have only one '.' within the file name/
 * extension component (but any number of dots in directory path).
 *
 * "Safer" in-line character copying is used instead of strcpy()'s in the
 * event that incoming strings overlap.
 * 
 * KEYWORDS:
 *
 * ERRORS:  path must be a properly allocated string.  Sufficient storage
 * space must be allocated for filepath, filename, and fileext.
 * Pointer arithmetic is used, which could be a problem with far pointers.
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_get_parts"

#ifdef PROTO
void os_path_get_parts(char *path, char *filepath, char *filename, char *fileext)
#else
void os_path_get_parts(path, filepath, filename, fileext)
char *path, *filepath, *filename, *fileext;
#endif

{
	char *pfname = NULL, /* file name component in path */
	     *pfext = NULL;  /* file extension component in path */
	int i = 0;
	
	if (path == NULL)
	{
		if (filepath)
			*filepath = STR_END;
		if (filename)
			*filename = STR_END;
		if (fileext)
			*fileext = STR_END;
		return;
	}
	
	os_path_find_parts(path, NULL, &pfname, &pfext);
	
	if (fileext)
	{
		if (pfext == NULL)
			*fileext = STR_END; /* no extension -- make NULL string */
		else
			for (i = 0; i <= (int)strlen(pfext); i++)
				fileext[i] = pfext[i];
	}

	if (filename)
	{
		if (pfname == NULL)
			*filename = STR_END;
		else
		{
			if (pfext == NULL) /* no extension; copy all *pfname */
				for (i = 0; i <= (int)strlen(pfname); i++)
					filename[i] = pfname[i];
			else
			{ /* filename might not have enough storage space to include ext */
				for (i = 0; pfname[i] != '.'; i++)
					filename[i] = pfname[i];
				filename[i] = STR_END;
			}
		}
	}
	
	if (filepath)
	{
		if (pfname == NULL && pfext == NULL)
			pfname = path + strlen(path);
		else if (pfname == NULL)
				pfname = pfext;

		while ((char HUGE *)path < (char HUGE *)pfname && *path != STR_END)
			*filepath++ = *path++;
		*filepath = STR_END;
	}
	return;
}

/*****************************************************************************
 * NAME: os_path_put_parts()
 *
 * PURPOSE:  Make a full path from component parts
 *
 * USAGE:  pathname = os_path_put_parts(fullpath, dirpath, filename, fileext);
 *
 * RETURNS:  fullpath is overwritten and a pointer to fullpath is returned
 *
 * DESCRIPTION:  dirpath, filename, and fileext are put together into
 * fullpath (all prior contents of fullpath are lost).  In concatenating
 * dirpath and filename, a native directory separator is interplaced if no
 * trailing separator is found in dirpath, and no leading separator is found
 * in filename.  In placing fileext a '.' is
 * interplaced if no '.' is found in filename (trailing) or fileext (leading).
 * Unless fileext is NULL, any extension in filename is overwritten with
 * fileext.  Where component arguments are NULL (but NOT fullpath) this
 * indicates the absence of that particular component in constructing the
 * new path.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  NONE
 *
 * GLOBALS:  NONE
 *
 * COMMENTS:  fullpath and either dirpath or filename may be identical
 * parameters in function calls, as a temporary buffer is used for construction.
 * If a DOS drive letter/colon ONLY (e.g., "C:" comes in as dirpath, a native
 * directory separator (backslash) should NOT be appended.
 *
 * If fileext is NULL (or empty) then no trailing '.' is appended to fullpath.
 *
 * KEYWORDS:  NATIVE_DIR_SEPARATOR, UNION_DIR_SEPARATORS, DEBUG, _DEBUG
 *
 * ERRORS:  Sufficient storage space must be allocated for fullpath.
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_put_parts"

#ifdef PROTO
char *os_path_put_parts(char *fullpath, char *dirpath, char *filename, char *fileext)
#else
char *os_path_put_parts(fullpath, dirpath, filename, fileext)
char *fullpath, *dirpath, *filename, *fileext;
#endif

{
	char newfullpath[2 * _MAX_PATH], /* for intermediate concats */
	     *temp_buf = newfullpath; /* assign pointer for inter. buffer */
	short pos = 0;
	
	/* make sure sufficient space exists for intermediate concats */
	
	if (2 * _MAX_PATH < ok_strlen(dirpath) + ok_strlen(filename) +
	                    ok_strlen(fileext))
	{
#if defined(DEBUG) || defined(_DEBUG)
		assert(0);
#else
		/* create new intermediary buffer for concats */
		 temp_buf = (char *)memMalloc(ok_strlen(dirpath) + ok_strlen(filename) +
		                               ok_strlen(fileext) + 2, "temp_buf");
		if (temp_buf == NULL)
		{
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "internal buffer");
			err_disp();
			return(NULL);
		}
#endif
	}

	if (fullpath == NULL)
		return(NULL);
	
	*temp_buf = STR_END;
	
	/* copy directory component */
	if (ok_strlen(dirpath))
	{
		while (*dirpath != STR_END)
			temp_buf[pos++] = *dirpath++;

		/* append a separator, if 1) no filename, or filename has no leading
		   separator, and 2) dirpath has no trailing separator */
		if (!filename || (filename && strcspn(filename, UNION_DIR_SEPARATORS)))
			if (strcspn(dirpath - 1, UNION_DIR_SEPARATORS))
				temp_buf[pos++] = NATIVE_DIR_SEPARATOR;
  }
  
	/* copy filename component, stop at extension */
	if (ok_strlen(filename))
	{
		if (ok_strlen(dirpath))
		{
		  if (temp_buf[pos - 1] == NATIVE_DIR_SEPARATOR)
		  	while (*filename == NATIVE_DIR_SEPARATOR)
		  		++filename;
		}
  
		while (*filename != STR_END)
		{
			if (fileext && *filename == '.' && 
			    strcspn(filename, UNION_DIR_SEPARATORS) == strlen(filename))
				break;
			temp_buf[pos++] = *filename++;
		}
	}

	if (ok_strlen(fileext))
	{
		while (*fileext == '.')
			fileext++;

		/* interplace '.' if needed */
		if (pos)
		{
			if (temp_buf[pos - 1] != '.')
				temp_buf[pos++] = '.';
		}
		else
			temp_buf[pos++] = '.';
		while (*fileext != STR_END)
			temp_buf[pos++] = *fileext++;
	}

	temp_buf[pos] = STR_END;

	memStrcpy(fullpath, temp_buf, "fullpath, temp_buf");
	
#if !defined(DEBUG) && !defined(_DEBUG)
	if (2 * _MAX_PATH < ok_strlen(dirpath) + ok_strlen(filename) +
	                    ok_strlen(fileext))
		memFree(temp_buf, "temp_buf");
#endif

	return(fullpath);
}

/*
 * NAME:  os_str_replace_char()
 *              
 * PURPOSE:  To replace all occurences of a character in a string with another
 * character.
 *
 * USAGE: void os_str_replace_char(char *string, char old, char new)
 *
 * RETURNS: void
 *
 * DESCRIPTION: To replace all ocurrences of old in string with new    
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 * rewritten by Mark A. Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    formerly nt_replace()
 *
 * KEYWORDS:    string, variable_length header
 *
 */                                          

#undef ROUTINE_NAME
#define ROUTINE_NAME "os_str_replace_char"

#ifdef PROTO
void os_str_replace_char(char *string, char oldc, char newc)
#else
void os_str_replace_char(string, oldc, newc)
char *string;
char old, new;
#endif
{
	if (string == NULL)
		return;

	for (; *string != STR_END; string++)
		if (*string == oldc)
			*string = newc;
}

/*
 * NAME:  os_path_prepend_special()
 *              
 * PURPOSE:  Prepend a directory, GEOVUDIR, or nothing to a file name
 *
 * USAGE:  BOOLEAN os_path_prepend_special(char *in_name, char *home_dir, char *out_name)
 *
 * RETURNS: FALSE if in_name or out_name are NULL, or the first character of
 * in_name is neither a caret ('^') not an ampersand ('&') and environment
 * variable GEOVUDIR is not defined.
 * Otherwise, TRUE
 *
 * DESCRIPTION:  If the first character of in_name is a caret ('^') then
 * in_name + 1 (w/o the caret) is copied into out_name -- no translation to
 * a native path is performed.  If the first character of in_name is an
 * ampersand ('&') then the string home_dir is copied into out_name, and is
 * appended with in_name + 1 (w/o the ampersand).  The entire string out_name
 * is then translated to a native path.  If the first character of in_name is
 * neither a caret nor an ampersand, then the value of GEOVUDIR is copied
 * into out_name, and in_name is appended to out_name.  The portion of
 * out_name following the value of GEOVUDIR is translated into a native path
 * (GEOVUDIR should already be a native path).
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     Uses env. variable "GEOVUDIR"
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 * modified by Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    Uses freeform.h file
 * formerly nt_file_name()
 *
 * KEYWORDS:  ^ CARET, & AMPERSAND, GEOVUDIR
 * 
 * ERRORS:  Sufficient space must be pre-allocated for out_name
 */


#undef ROUTINE_NAME
#define ROUTINE_NAME "os_path_prepend_special"

#ifdef PROTO
BOOLEAN os_path_prepend_special(char *in_name, char *home_path, char *out_name)
#else
BOOLEAN os_path_prepend_special(in_name, home_path, out_name)
char *in_name, *home_path, out_name;
#endif
{
	char *ch = NULL;
	
	assert(in_name);
	assert(out_name);

	if (!in_name || !out_name)
		return(FALSE);

	/* file at cd */
	if (*in_name == '&')
	{
		assert(home_path);
		
		if (!home_path)
		{
			out_name[0] = STR_END;
			return(FALSE);
		}
		(void)os_path_put_parts(out_name, home_path, in_name + 1, NULL);
		(void)os_path_make_native(out_name, out_name);
		return(TRUE);
	}
	else if (*in_name == '^')
	{       /* file name already include path */
		memStrcpy(out_name, (in_name + 1),"out_name,in_name+1");
		return(TRUE);
	}
	else if ((ch = os_get_env("GEOVUDIR")) != NULL)
	{	/* the file is located in the default GEOVU dir */
		(void)os_path_put_parts(out_name, ch, in_name, NULL);
		(void)os_path_make_native(out_name + strlen(ch), out_name + strlen(ch));
		memFree(ch, "ch"); /* MAO:c safe to do as os_get_env() allocates return block */
		return(TRUE);
	}
	else
	{
		out_name[0] = STR_END;
		return(FALSE);
	}
}

/*
 * NAME:  os_str_trim_whitespace()
 *              
 * PURPOSE:  Remove leading and trailing whitespace
 *
 * USAGE:  os_str_trim_whitespace(dest, source)
 *
 * RETURNS:  a copy of source w/o leading and trailing whitespace; dest is
 * overwritten and a pointer to dest is returned, else NULL if either source
 * or dest are NULL.
 *
 * DESCRIPTION:  First and last whitespace characters of source are located --
 * these and all intervening characters in source are moved into dest in a
 * front to back copy order. 
 *
 * SYSTEM DEPENDENT FUNCTIONS:  NONE
 *
 * GLOBALS:  NONE
 *
 * AUTHOR:  Mark A. Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:  source and dest may be the same argument in calls; source is
 * safely copied into dest without losing original address -- this is
 * important for future free()'s on source.
 *
 * KEYWORDS:  whitespace
 * 
 * ERRORS:  If inadequate memory is allocated for dest then memory corruption
 * may occur.
 */


#undef ROUTINE_NAME
#define ROUTINE_NAME "os_str_trim_whitespace"

#ifdef PROTO
char *os_str_trim_whitespace(char *dest, char *source)
#else
char *os_str_trim_whitespace(dest, source)
char *dest, *source;
#endif

{
	int start = 0, /* first nonwhitespace character of source */
	    stop  = 0, /* last nonwhitespace character of source */
	    i = 0;

	if (source == NULL || dest == NULL)
		return(NULL);

	for (stop = strlen(source) - 1; stop >= 0 && isspace(source[stop]); stop--)
		;
	for (start = 0; start <= stop && isspace(source[start]); start++)
		;
	
	for (i = start; i <= stop; i++)
		dest[i - start] = source[i];
	
	dest[stop - start + 1] = STR_END;
	
	return(dest);
}

void os_str_replace_unescaped_char1_with_char2(char char1, char char2, char *str)
/*****************************************************************************
 * NAME: os_str_replace_unescaped_char1_with_char2()
 *
 * PURPOSE:  Replace unescaped char1 characters with char2 characters
 *
 * USAGE:  os_str_replace_unescaped_char1_with_char2(char1, char2, char *str);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Replaces all occurences of char1 in str with char2, unless
 * char1 is escaped with a preceding backslash ('\'); if an immediately
 * preceding backslash is itself escaped, then char1 is replaced and the
 * escaped backslash is replaced with a single backslash.  For example, if
 * char1 is '%' and char2 is '$' then the string "%" becomes "$" (and "\\%"
 * becomes "\$") but the string "\%" becomes "%".  Only backslashes
 * preceding char1 are treated specially -- backslashes preceding other
 * characters are ignored.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:  if char1 is OS_ESCAPER, then char1 cannot be escaped -- all
 * occurrences of char1 in str are unconditionally replaced.
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	os_str_replace_xxxcaped_char1_with_char2(OS_INVERSE_ESCAPE, char1, char2, str);
}

void os_str_replace_escaped_char1_with_char2(char char1, char char2, char *str)
/*****************************************************************************
 * NAME: os_str_replace_escaped_char1_with_char2()
 *
 * PURPOSE:  Replace escaped char1 characters with char2 characters
 *
 * USAGE:  os_str_replace_escaped_char1_with_char2(char1, char2, char *str);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Replaces all occurences of char1, which are escaped with a
 * preceding backslash ('\'), in str with char2; if an immediately
 * preceding backslash is itself escaped, then char1 is not replaced and the
 * escaped backslash is replaced with a single backslash.  For example, if
 * char1 is '%' and char2 is '$' then the string "\%" becomes "$" (and "\\\%"
 * becomes "\$") but the string "\\%" becomes "\%".  Only backslashes
 * preceding char1 are treated specially -- backslashes preceding other
 * characters are ignored.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:  if char1 is OS_ESCAPER, then char1 cannot be escaped -- all
 * occurrences of char1 in str are unconditionally replaced.
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	os_str_replace_xxxcaped_char1_with_char2(OS_NORMAL_ESCAPE, char1, char2, str);
}

void os_str_replace_xxxcaped_char1_with_char2(int mode, char char1, char char2, char *str)
/*****************************************************************************
 * NAME: os_str_replace_xxxcaped_char1_with_char2()
 *
 * PURPOSE:  Replace either escaped or unescaped char1 characters with char2
 * characters according to mode
 *
 * USAGE:  os_str_replace_xxxcaped_char1_with_char2(mode, char1, char2, char *str);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  See DESCRIPTION for os_str_replace_unescaped_char1_with_char2()
 * and os_str_replace_escaped_char1_with_char2() for general overview.
 *
 * cp1 is set to each occurrence of char1 in str, and cp2 is set to the first
 * OS_ESCAPER character in a string of OS_ESCAPER characters immediately preceding
 * cp1. The immediately preceding OS_ESCAPER character is itself
 * being escaped if it is preceded by a OS_ESCAPER character, unless that OS_ESCAPER
 * character is itself being escaped, etc. ad naaseaum.  It boils down to
 * whether there are an even or an odd number of OS_ESCAPER characters in a 
 * homogeneous string of OS_ESCAPER's preceding char1 at cp1.
 *
 * The number of OS_ESCAPER characters is n = (cp1 - cp2).  If odd then char1
 * is being escaped; if mode is OS_INVERSE_ESCAPE then char1 should NOT be
 * replaced, and if mode is OS_NORMAL_ESCAPE then char1 should be replaced.
 * If even then char1 is NOT being escaped; if mode is OS_INVERSE_ESCAPE then
 * char1 should be replaced, and if mode is OS_NORMAL_ESCAPE then char1 should
 * NOT be replaced.  If n > 0 and n odd then (n - 1) / 2 preceding OS_ESCAPER
 * characters are being ESCAPEd and so n - 1 count must be replaced with
 * (n - 1) / 2 count.  If n > 0 and n even then n / 2 preceding OS_ESCAPER
 * characters are being ESCAPEd and so n count must be replaced with n / 2
 * count.  Fortuitously, integer arithmetic (in which remainders are ignored)
 * allows both cases to be covered with a single division.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:  if char1 is OS_ESCAPER, then char1 cannot be escaped -- all
 * occurrences of char1 in str are unconditionally replaced.
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	char *cp1, *cp2;
	int num_ESCAPERs = 0;
	
	assert(str);
	
	if (!str)
		return;
	
	cp1 = strchr(str, char1);
	
	while (cp1)
	{
		cp2 = cp1 - 1;
		while (*cp2 == OS_ESCAPER && cp2 >= str)
			--cp2;
		++cp2; /* cp2 points to first OS_ESCAPER in a string preceding char1 */
	
		num_ESCAPERs = cp1 - cp2;
		if ((mode == OS_INVERSE_ESCAPE && num_ESCAPERs % 2 == 0) ||
		    (mode == OS_NORMAL_ESCAPE && num_ESCAPERs % 2 == 1))
			*cp1 = char2;
		num_ESCAPERs /= 2; /* This is how many OS_ESCAPER's will be retained */
		cp2 += num_ESCAPERs; /* move cp2 one past last retained OS_ESCAPER */
		memmove(cp2, cp1, strlen(cp1) + 1); /* overwrite discarded OS_ESCAPER's */
		/* now cp2 points to char1/char2 */

		cp1 = strchr(cp2 + 1, char1);
	}
}

