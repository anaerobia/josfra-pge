/* FILENAME:  findfile.c
 *
 * CONTAINS:
 * ff_find_files()
 * db_find_format_files()
 *
 */

#include <freeform.h>
#include <os_utils.h>
#include <databin.h>

#ifdef PROTO
static int check_file_exists(char **trial_fname, char *search_dir, char *filebase, char *ext);
static int find_dir_format_files(char *input_file, char *search_dir,
                                 char **targets);
#else
static int check_file_exists();
static int find_dir_format_files();
#endif

/* NAME:        ff_find_files
 *              
 * PURPOSE:     Search multiple directories for files matching the supplied
 *              file name and extension.
 *
 * USAGE:       int ff_find_files(char *file_base, char *ext, char *first_dir,
 *                                      char ***targets)
 *
 * RETURNS:  number of files found -- string vector targets is filled with
 * found file names.
 *
 * DESCRIPTION:  file_base may include a path (which distinguishes the default
 * directory search from the file-directory search) and first_dir may be
 * NULL.  ff_find_files() searches first in first_dir,
 * second in the default directory, and lastly in the file's home directory
 * (given by the path component of file_base) for files with the file name
 * component of file_base (i.e., not including the path component of file_base)
 * and the given extension ext and then for files with the file extension
 * component of file_base and the given extenstion ext.
 * 
 * In other words,
 * perform the following searches, where filename(file_base) is the file name
 * component of file_base, fileext(file_base) is the file extension component
 * of file_base, and path(file_base) is the directory path component of file_base.
 *
 * 1) Search first_dir for filename(file_base).ext
 * 2) Search first_dir for fileext(file_base).ext
 * 3) Search the default directory for filename(file_base).ext
 * 4) Search the default directory for fileext(file_base).ext
 * 5) Search the directory given by path(file_base) for filename(file_base).ext
 * 6) Search the directory given by path(file_base) for fileext(file_base).ext
 *
 * This means potentially six files can be found.
 *
 * For each file found, an element of the string vector targets
 * is set to point to an allocated string which is the found file name.
 * The parameter targets itself is an address of a string vector, like argv[].
 *
 * Once the calling routine is done with targets[], it should free the memory
 * according to the following paradigm:
 *
 * For each file found, free(targets[i]), then free(targets).
 *
 * ERRORS:
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * GLOBALS:
 *
 * AUTHOR:	Mark A. Ohrenschall, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */

#ifdef PROTO
int ff_find_files(char *file_base, char *ext, char *first_dir, char ***targets)
#else
int ff_find_files(file_base, ext, first_dir, targets)
char *file_base, *ext, *first_dir, ***targets;
#endif

#define MAX_CAN_FIND 6

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "ff_find_files"
{
	char *fileext = NULL; /* the extension on the search file name */
	char  filename[_MAX_PATH];
	char  home_dir[_MAX_PATH];
	int num_found = 0;
	char *filenames[MAX_CAN_FIND] = {NULL, NULL, NULL, NULL, NULL, NULL};

	assert(file_base);
	assert(ok_strlen(file_base));
	
	if (file_base == NULL || ok_strlen(file_base) == 0)
		return(0);

	/* overuse variable fileext to check out ext, which may be a filename.ext */
	fileext = os_path_return_ext(ext);
	if (fileext)
		ext = fileext;

	fileext = os_path_return_ext(file_base);
	os_path_get_parts(file_base, home_dir, filename, NULL);
	if (ok_strlen(filename) == 0)
		return(0);
	
	/* Check for file_base.ext in first_dir */
	if (first_dir && check_file_exists(&filenames[num_found], first_dir, filename, ext))
		++num_found;
	
	/* Check for file_base's extension.ext in first_dir */
	if (fileext && first_dir &&
	    check_file_exists(&filenames[num_found], first_dir, fileext, ext))
		++num_found;

	/* Check for file_base.ext in the current directory */
	if (check_file_exists(&filenames[num_found], NULL, filename, ext))
		++num_found;

	/* Check for file_base's extension.ext in the current directory */
	if (fileext &&
	    check_file_exists(&filenames[num_found], NULL, fileext, ext))
		++num_found;
	
	/* Check for file_base.ext in data home directory */
	if (home_dir &&
	    check_file_exists(&filenames[num_found], home_dir, filename, ext))
		++num_found;

	/* Check for file_base's extension.ext in data home directory */
	if (fileext && home_dir &&
	    check_file_exists(&filenames[num_found], home_dir, fileext, ext))
		++num_found;
	
	if (num_found)
	{
		*targets = (char **)memMalloc(num_found * sizeof(char *), "*targets");
		if (!*targets)
		{
			err_push(ROUTINE_NAME, ERR_MEM_LACK, NULL);
			return(0);
		}
		memMemcpy((char *)*targets, (char *)filenames, num_found * sizeof(char *), "*targets, filenames");
	}
	
	return(num_found);
}

/* NAME:        db_find_format_files
 *              
 * PURPOSE:     To search for format files given a (data) file name.
 *
 * USAGE: number = db_find_format_files(dbin, dbin->file_name, &targets);
 *
 * RETURNS: Zero, one, or two:  the number of eligible format files found,
 * targets[0] is input format file name, targets[1] is output format file
 * name, if found.
 *
 * DESCRIPTION: The variable targets is a string vector, type (char (*)[]),
 * like argv.
 *
 * If a format file is not given explicitly on a command line then FreeForm
 * searches for default format files as detailed below.  FORMAT_DIR refers
 * to the directory given by the GeoVu keyword "format_dir".  "format_dir"
 * can also be defined in the operating system variable environment space.  The
 * default directory refers to the current working directory.  The file's home
 * directory is given by the path component of datafile, if any.  If there is
 * no path component, then the file's home directory search is not conducted.
 *
 * So, given the data file datafile.ext conduct the following searches for
 * the default format file(s):
 *
 * 1) Search FORMAT_DIR for datafile.fmt.
 *
 * 2) Search FORMAT_DIR for datafile.afm, datafile.bfm, or datafile.dfm:
 *    i) If ext is "dat" search for datafile.afm -- this will be the input
 *       format.  Then search for datafile.bfm or datafile.dfm -- the first
 *       to be found is taken as the output format file.
 *    ii) If ext is "dab" search for datafile.dfm -- this will be the input
 *        format.  Then search for datafile.afm or datafile.bfm -- the first
 *        to be found is taken as the output file.
 *    iii) If the ext is NULL or other than above, search for datafile.bfm --
 *         this will be the input format.  Then search for datafile.afm or
 *         datafile.dfm -- the first to be found is taken as the output file.
 *
 * 3) Search FORMAT_DIR for ext.fmt
 *
 * 4) Search the default directory for datafile.fmt.
 *
 * 5) Search the default directory for datafile.afm, datfile.bfm or
 *    datafile.dfm as in step 2) above.
 *
 * 6) Search the default directory for ext.fmt
 *
 * Steps 7 - 9 are conducted if datafile has a path component.
 *
 * 7) Search the data file's directory for datafile.fmt.
 *
 * 8) Search the data file's directory for datafile.afm, datfile.bfm or
 *    datafile.dfm as in step 2) above.
 *
 * 9) Search the data file's directory for ext.fmt
 *
 * Each successive search step is conducted only if the previous search fails.
 *
 * To simplify code in the calling routines, the defaulting format
 * file extension rules in db_format_list_mark_io() are duplicated here.  The
 * result of this is that an input .afm/.bfm/.dfm always precedes an output
 * .afm/.bfm/.dfm file in the target vector string of file names.  Note that
 * unlike db_format_list_mark_io() which does not prioritize one eligible
 * output format over another (except by whichever occurs first in the list)
 * this routine does prioritize one type of .afm/.bfm/.dfm output file over
 * another depending on the data file extension.
 *    
 * NOTE that in a Windows environment the default directory has the potential
 * to be changed at any time, and it need not be the same as the application
 * start-up directory.  If the calling routine is relying on formats to be
 * found in the default directory, it must ensure that the default directory
 * has been properly set.
 *
 * See the description for ff_find_files() for an explanation of the parameter
 * targets.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:  Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:
 *
 * KEYWORDS:    
 *
 */

#ifdef PROTO
int db_find_format_files(DATA_BIN_PTR dbin, char *input_file, char ***targets)
#else
int db_find_format_files(dbin, input_file, targets)
DATA_BIN_PTR dbin;
char *input_file, ***targets;
#endif

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_find_format_files"
{
	char home_dir[_MAX_PATH];
	char format_dir[_MAX_PATH];
	char *format_files[2] = {NULL, NULL};
	int num_found;
	
	assert(input_file);
	assert(targets);
	
	if (!input_file || !targets)
		return(0);
	
	*targets = (char **)memCalloc(2, sizeof(char *), "*targets");
	if (!*targets)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, NULL);
		return(0);
	}

#ifdef XVT
	if (nt_askvalue(dbin, "format_dir", FFV_CHAR, home_dir, NULL))
	{
		if (nt_askexist(dbin, "format_dir"))
			/* defined in header, or hopefully eqv section -- do the usual */
			os_path_prepend_special(home_dir, cd_device, format_dir);
		else
			/* defined in OS' environment, take literally */
			strcpy(format_dir, home_dir);
	}
	else
		format_dir[0] = STR_END;
#else
	if (!nt_askvalue(dbin, "format_dir", FFV_CHAR, format_dir, NULL))
		format_dir[0] = STR_END;
#endif
	
	os_path_get_parts(input_file, home_dir, NULL, NULL);

	/* Search format_dir first */
	num_found = find_dir_format_files(input_file, format_dir, format_files);

	/* Search default directory second */
	if (num_found == 0)
		num_found = find_dir_format_files(input_file, NULL, format_files);
		
	/* Search data file's directory last */
	if (ok_strlen(home_dir) && num_found == 0)
		num_found = find_dir_format_files(input_file, home_dir, format_files);

	if (num_found >= 1)
		(*targets)[0] = format_files[0];
	if (num_found == 2)
		(*targets)[1] = format_files[1];

	return(num_found);
}

/*****************************************************************************
 * NAME: find_dir_format_files
 *
 * PURPOSE:  Search the directory given by search_dir for format files
 *
 * USAGE: num_found = find_dir_format_files(input_file, format_dir, fileext, format_files);
 *
 * RETURNS:  0, 1, or 2 -- 0: no files found, 1: either filename.fmt,
 * filename.?fm, or ext.fmt found, 2: two .afm/.bfm/.dfm files found
 *
 * DESCRIPTION:  First looks in the supplied directory for filename.fmt,
 * then filename.afm, filename.bfm, or filename.dfm (depending on filename's
 * extension), then for ext.fmt.  Each successive search proceeds only if the
 * previous search failed.  See the DESCRIPTION for db_find_format_files() for
 * more information.
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

#ifdef PROTO
static int find_dir_format_files(char *input_file, char *search_dir, char **targets)
#else
static int find_dir_format_files(input_file, search_dir, targets)
char *input_file, *search_dir, **targets;
#endif

#undef ROUTINE_NAME
#define ROUTINE_NAME "find_dir_format_files"
{
	int num_found, i;
	char *fileext = os_path_return_ext(input_file);
	char filename[_MAX_PATH];

	os_path_get_parts(input_file, NULL, filename, NULL);
	if (ok_strlen(filename) == 0)
		return(0);

	/* Search search_dir for datafile.fmt */
	num_found = check_file_exists(&(targets[0]), search_dir,
	                              filename, "fmt");
	if (num_found == 1)
		return(1);
	
	/* Search search_dir for datafile.?fm  -- may return two file names */
	if (fileext && strcmp(fileext, "dat") == 0)
	{
		/* input format */
		num_found = check_file_exists(&(targets[0]), search_dir,
		                              filename, "afm");
		/* output format */
		num_found += i =
		             check_file_exists(&(targets[1]), search_dir,
		                               filename, "bfm");
		if (i == 0)
			num_found += check_file_exists(&(targets[1]), search_dir,
			                               filename, "dfm");
	}
	else if (fileext && strcmp(fileext, "dab") == 0)
	{
		/* input format */
		num_found = check_file_exists(&(targets[0]), search_dir,
		                              filename, "dfm");
		/* output format */
		num_found += i =
		             check_file_exists(&(targets[1]), search_dir,
		                               filename, "afm");
		if (i == 0)
			num_found += check_file_exists(&(targets[1]), search_dir,
			                               filename, "bfm");
	}
	else
	{
		/* input format */
		num_found = check_file_exists(&(targets[0]), search_dir,
		                              filename, "bfm");
		/* output format */
		num_found += i =
		             check_file_exists(&(targets[1]), search_dir,
		                               filename, "afm");
		if (i == 0)
			num_found += check_file_exists(&(targets[1]), search_dir,
			                               filename, "dfm");
	}
	
	if (num_found == 2)
		return(2);
	
	if (num_found == 0)
		/* Search search_dir for ext.fmt */
		num_found = check_file_exists(&(targets[0]), search_dir,
		                              fileext, "fmt");
	return(num_found);
}

/*****************************************************************************
 * NAME:  check_file_exists() -- a module function of findfile.c
 *
 * PURPOSE:  Test existence for a file, composed from parts, and return name
 *
 * USAGE:  found = check_file_exists(trial_fname, search_dir, filebase, ext);
 *
 * RETURNS: 1 if file is found, zero otherwise
 *
 * DESCRIPTION:  search_dir, filebase, and ext are concatenated, with native
 * directory separator.  Variable trial_fname is set to point to an allocated
 * string containing the existing file name, otherwise trial_fname is set to
 * NULL.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@ngdc.noaa.gov
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

#ifdef PROTO
static int check_file_exists(char **fname, char *search_dir, char *filebase, char *ext)
#else
static int check_file_exists(fname, search_dir, filebase, ext)
char **fname, *search_dir, *filebase, *ext;
#endif

#undef ROUTINE_NAME
#define ROUTINE_NAME "check_file_exists"
{
	char trial_fname[_MAX_PATH];
	
	(void)os_path_put_parts(trial_fname, search_dir, filebase, ext);
	if (os_file_exist(trial_fname))
	{
		*fname = (char *)memStrdup(trial_fname, "trial_fname");
		if (*fname == NULL)
		{
			err_push(ROUTINE_NAME, ERR_MEM_LACK, NULL);
			return(0);
		}
		return(1);
	}
	else
		return(0);
}

