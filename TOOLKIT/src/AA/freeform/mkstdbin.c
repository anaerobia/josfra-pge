/*
 * FILENAME:  mkstdbin.c
 *              
 * CONTAINS:  fill_std_args_from_command_line()
 *            make_standard_dbin()
 *
 */

#include <freeform.h>
#include <os_utils.h>
#include <databin.h>

static char *endptr;
static char err_buf[MAX_ERRSTR_BUFFER];

/* Each of the following functions reflects the first letter of an option
   flag.  Single letter option flags have simple functions below (e.g., -b),
   but multiple letter options flags have more complex functions with
   additional levels of switch and case statements (e.g., -ift).
   
   Currently, I have broken only one level of switch and cases into "dispatch
   center" functions.  Presumably, when certain options become more complex
   (e.g., -i*, which could add another letter for header or record format,
   and title, such as -ihft) then I will have to use function calls for those.
   
   In naming these functions, the number of underscores following the first
   letter of the option flag indicates the number of possible additional
   characters to follow.  For each additional character is another level of
   switches.
*/

#undef ROUTINE_NAME
#define ROUTINE_NAME "parse_command_line"

static int option_B(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -b? */
	{
		case STR_END :
			(*i)++;
	
			if (!argv[*i] || argv[*i][0] == '-')
			{
				sprintf(err_buf, "Expecting a positive value for buffer size (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, err_buf);
			}
			else
			{
				std_args->user.set_local_buffer_size = 1;
				errno = 0;
				std_args->local_buffer_size = strtol(argv[*i], &endptr, 10);
				/* add reasonable value check */
				if (ok_strlen(endptr))
					error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, argv[*i]);
				
				if (errno == ERANGE)
					error = err_push(ROUTINE_NAME, errno, argv[*i]);
			}
		break;
	
		default:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	}
	
	return(error);
}

static int option_C(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -c? */
	{
		case STR_END :
			(*i)++;

			if (!argv[*i])
			{
				sprintf(err_buf, "Expecting a value for count (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, err_buf);
			}
			else
			{
				errno = 0;
				std_args->user.set_records_to_read = 1;
				std_args->records_to_read = strtol(argv[*i], &endptr, 10);
				if (ok_strlen(endptr))
					error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, argv[*i]);
			
				if (errno == ERANGE)
					error = err_push(ROUTINE_NAME, errno, argv[*i]);
			}
		break;
							
		default:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	}
	
	return(error);
}

static int option_F_(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -f? */
	{
		case STR_END : /* single format file */
			(*i)++;
			/* Check File */

			if (!argv[*i])
			{
				sprintf(err_buf, "Expecting a name for format file (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
			}
			else
			{
				if (!os_file_exist(argv[*i]))
					error = err_push(ROUTINE_NAME, ERR_FILE_NOTEXIST, argv[*i]);
	
				std_args->user.set_output_format_file = 1;
				std_args->user.set_input_format_file = 1;
				std_args->output_format_file = argv[*i];
				std_args->input_format_file = argv[*i];
			}
		break;
							
		case 'T' : /* single format title */
			switch (toupper(argv[*i][3])) /* -ft? */
			{
				case STR_END :
					(*i)++;

					if (!argv[*i])
					{
						sprintf(err_buf, "Expecting a title for formats (following %s)", argv[*i - 1]);
						error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
					}
					else
					{
						std_args->user.set_input_format_title = 1;
						std_args->user.set_output_format_title = 1;
						std_args->input_format_title = argv[*i];
						std_args->output_format_title = argv[*i];
					}
				break;
								
				default :
					sprintf(err_buf, "==> %s <==", argv[*i]);
					error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
				break;
			}
		break;
							
		default :
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	} /* switch on second letter of -f flag */
	
	return(error);
}

static int option_I__(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -i? */
	{
		case STR_END:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
							
		case 'F' :
			switch (toupper(argv[*i][3])) /* -if? */
			{
				case STR_END : /* input format file */
					(*i)++;
					/* Check File */

					if (!argv[*i])
					{
						sprintf(err_buf, "Expecting a name for input format file (following %s)", argv[*i - 1]);
						error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
					}
					else
					{
						if (!os_file_exist(argv[*i]))
							error = err_push(ROUTINE_NAME, ERR_FILE_NOTEXIST, argv[*i]);
	
						std_args->user.set_input_format_file = 1;
						std_args->input_format_file = argv[*i];
					}
				break;
								
				case 'T' : /* input format title */
					switch (toupper(argv[*i][4]))
					{
						case STR_END :
							(*i)++;

							if (!argv[*i])
							{
								sprintf(err_buf, "Expecting a title for input format (following %s)", argv[*i - 1]);
								error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
							}
							else
							{
								std_args->user.set_input_format_title = 1;
								std_args->input_format_title = argv[*i];
							}
						break;
							
						default:
							sprintf(err_buf, "==> %s <==", argv[*i]);
							error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
						break;
					}
				break;
									
				default :
					sprintf(err_buf, "==> %s <==", argv[*i]);
					error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
				break;
			} /* switch on third letter of -I flag */
		break;

		default :
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	} /* switch on second letter of -I flag */
	
	return(error);
}

static int option_M_(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -m? */
	{
		case STR_END :
			(*i)++;
								
			if (!argv[*i] || argv[*i][0] == '-')
			{
				sprintf(err_buf, "Expecting a positive value for maxbins (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, err_buf);
			}
			else
			{
				std_args->user.set_cv_maxbins = 1;
				errno = 0;
				std_args->cv_maxbins = (int)strtod(argv[*i], &endptr);
				if (ok_strlen(endptr))
					error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, argv[*i]);
			
				if (errno == ERANGE)
					error = err_push(ROUTINE_NAME, errno, argv[*i]);
			}
		break;
							
		case 'D' :
			switch (toupper(argv[*i][3])) /* -md? */
			{
				case STR_END :
					(*i)++;

					if (!argv[*i])
					{
						sprintf(err_buf, "Expecting a value for missing data flag (following %s)", argv[*i - 1]);
						error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, err_buf);
					}
					else
					{
						std_args->user.set_cv_missing_data = 1;
						errno = 0;
						std_args->cv_missing_data = strtod(argv[*i], &endptr);
						if (ok_strlen(endptr))
							error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, argv[*i]);
					
						if (errno == ERANGE)
							error = err_push(ROUTINE_NAME, errno, argv[*i]);
					}
				break;

				default:
					sprintf(err_buf, "==> %s <==", argv[*i]);
					error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
				break;
			}
		break;
							
		case 'M' :
			switch (toupper(argv[*i][3])) /* -mm? */
			{
				case STR_END :
					std_args->user.set_cv_maxmin_only = 1;
				break;

				default:
					sprintf(err_buf, "==> %s <==", argv[*i]);
					error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
				break;
			}
		break;
							
		default:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	} /* switch on second letter of -m flag */
	
	return(error);
}

static int option_O__(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -o? */
	{
		case STR_END:
			(*i)++;

			if (!argv[*i])
			{
				sprintf(err_buf, "Specify a name for the output file (following %s).", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
			}
			else
			{
				if (os_file_exist(argv[*i]))
				{
					sprintf(err_buf, "Specify a new name or move %s.",
					        os_path_return_name(argv[*i]));
					error = err_push(ROUTINE_NAME, ERR_FILE_EXISTS, err_buf);
				}
				else
				{
					FILE *fp = NULL;
										
					fp = fopen(argv[*i], "w"); /* Can we write to file? */
					if (!fp)
					{
						error = err_push(ROUTINE_NAME, ERR_CREATE_FILE,
						                 "Specify a new name or location for the output file.");
					}
					else
						fclose(fp);
				}
	
				std_args->user.set_output_file = 1;
				std_args->output_file = argv[*i];
			}
		break;
							
		case 'F' :
			switch (toupper(argv[*i][3])) /* -of? */
			{
				case STR_END : /* output format file */
					(*i)++;
					/* Check File */

					if (!argv[*i])
					{
						sprintf(err_buf, "Expecting a name for output format file (following %s)", argv[*i - 1]);
						error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
					}
					else
					{
						if (!os_file_exist(argv[*i]))
							error = err_push(ROUTINE_NAME, ERR_FILE_NOTEXIST, argv[*i]);
	
						std_args->user.set_output_format_file = 1;
						std_args->output_format_file = argv[*i];
					}
				break;
								
				case 'T' : /* output format title */
					switch (toupper(argv[*i][4])) /* -oft? */
					{
						case STR_END :
							(*i)++;

							if (!argv[*i])
							{
								sprintf(err_buf, "Expecting a title for output format (following %s)", argv[*i - 1]);
								error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
							}
							else
							{
								std_args->user.set_output_format_title = 1;
								std_args->output_format_title = argv[*i];
							}
						break;
							
						default:
							sprintf(err_buf, "==> %s <==", argv[*i]);
							error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
						break;
					}
				break;
									
				default :
					sprintf(err_buf, "==> %s <==", argv[*i]);
					error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
				break;
			} /* switch on third letter of -O flag */
		break;

		default :
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;

	} /* switch on second letter of -O flag */
	
	return(error);
}

static int option_P(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -p? */
	{
		case STR_END :
			(*i)++;

			if (!argv[*i])
			{
				sprintf(err_buf, "Expecting a value for precision (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, err_buf);
			}
			else
			{
				std_args->user.set_cv_precision = 1;
				errno = 0;
				std_args->cv_precision = (int)strtod(argv[*i], &endptr);
				if (ok_strlen(endptr))
					error = err_push(ROUTINE_NAME, ERR_BAD_NUMBER_ARGV, argv[*i]);
			
				if (errno == ERANGE)
					error = err_push(ROUTINE_NAME, errno, argv[*i]);
			}
		break;

		default:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	}
	
	return(error);
}

static int option_Q(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -q? */
	{
		case STR_END :
			(*i)++;

			if (!argv[*i])
			{
				sprintf(err_buf, "Expecting a name for query file (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
			}
			else
			{
				if (!os_file_exist(argv[*i]))
					error = err_push(ROUTINE_NAME, ERR_FILE_NOTEXIST, argv[*i]);
			
				std_args->user.set_query_file = 1;
				std_args->query_file = argv[*i];
			}
		break;
							
		default:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	}
	
	return(error);
}

static int option_V(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	switch (toupper(argv[*i][2])) /* -v? */
	{
		case STR_END :
			(*i)++;

			if (!argv[*i])
			{
				sprintf(err_buf, "Expecting a name for variable file (following %s)", argv[*i - 1]);
				error = err_push(ROUTINE_NAME, ERR_PARAM_VALUE, err_buf);
			}
			else
			{
				if (!os_file_exist(argv[*i]))
					error = err_push(ROUTINE_NAME, ERR_FILE_NOTEXIST, argv[*i]);
			
				std_args->user.set_var_file = 1;
				std_args->var_file = argv[*i];
			}
		break;

		default:
			sprintf(err_buf, "==> %s <==", argv[*i]);
			error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
		break;
	}
	
	return(error);
}

/*
static int option_(char *argv[], FFF_STD_ARGS_PTR std_args, int *i)
{
	int error = 0;

	
	return(error);
}
*/

/*****************************************************************************
 * NAME:  parse_command_line()
 *
 * PURPOSE:  parses a command line for standard FreeForm arguments
 *
 * USAGE:  error = fill_std_args_from_command_line(argc, argv, std_args_ptr);
 *
 * RETURNS:  Zero on success, LAST error code on failure
 *
 * DESCRIPTION:  For each command line argument after the zero'th, look at the
 * first character, and then possibly every character thereafter in succession.
 * The "first" command line argument is treated a little differently, since it
 * can be either a file name, or an option flag (if input is being redirected).
 *
 * If the first character is a minus, then this indicates the beginning of an
 * option flag.  If it is an option flag, then examine every successive
 * character in turn to determine exactly which option.  This is done until
 * the NULL-terminator for the option string has been reached.  Once the option has
 * been determined, the next command line parameter may be examined, as
 * appropriate for that option.
 *
 * Multiple errors are possible -- each command line argument
 * is parsed in turn, each one possibly generating an error.  Only the last
 * error, which may not be the most severe, generates the return error code.
 * 
 * std_args must be a pre-allocated structure.
 *
 * 	Checks for these command line arguments:
 * 
 * -b   local_buffer_size
 * -c   count
 * -if  input_format_file
 * -ift input_format_title
 * -f   format_file
 * -ft  format_title
 * -m   maximum number of bins (checkvar only)
 * -md  missing data value (checkvar only)
 * -mm  maxmin only processing (checkvar only)
 * -o   output_data_file
 * -of  output_format_file
 * -oft output_format_title
 * -p   precision (checkvar only)
 * -q   query_file
 * -v   variable file
 *
 * checkvar can have optional arguments:
 *  -m maxbins
 *  -mm (maxmin only)
 *  -md (ignore missing data)
 *  -p precision
 *
 * The following are possible error return codes:
 *
 * ERR_BAD_NUMBER_ARGV (because a numeric string could not be converted to an integer)
 * ERANGE (because of an overflow or underflow in converting above)
 * ERR_FILE_NOTEXIST (because a specified file does not exist)
 * ERR_FILE_EXISTS (because the output file would be overwritten)
 * ERR_UNKNOWN_OPTION (because of an unrecognized flag typed on the command line)
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

int parse_command_line(int argc, char *argv[], FFF_STD_ARGS_PTR std_args)
{
	int i;
	int error;
	int num_flags = 0;

	assert(std_args);
	
	if (!std_args)
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "standard args structure"));
	
	std_args->input_file = NULL;
	std_args->output_file = NULL;
	std_args->input_format_file = NULL;
	std_args->output_format_file = NULL;
	std_args->input_format_title = NULL;
	std_args->output_format_title = NULL;
	std_args->var_file = NULL;
	std_args->query_file = NULL;
	std_args->local_buffer = NULL;
	std_args->local_buffer_size = 0;
	std_args->cache_size = DEFAULT_CACHE_SIZE;
	std_args->records_to_read = 0L;
	std_args->cv_precision = 0;
	std_args->cv_missing_data = 0;
	std_args->cv_maxbins = 0;

	std_args->user.set_input_file = 0;
	std_args->user.set_output_file = 0;
	std_args->user.set_input_format_file = 0;
	std_args->user.set_output_format_file = 0;
	std_args->user.set_input_format_title = 0;
	std_args->user.set_output_format_title = 0;
	std_args->user.set_var_file = 0;
	std_args->user.set_query_file = 0;
	std_args->user.set_local_buffer_size = 0;
	std_args->user.set_local_buffer_size = 0;
	std_args->user.set_records_to_read = 0;
	std_args->user.set_cv_precision = 0;
	std_args->user.set_cv_missing_data = 0;
	std_args->user.set_cv_maxbins = 0;
	std_args->user.set_cv_maxmin_only = 0;
	std_args->user.set_unused_1 = 0;
	std_args->user.set_unused_2 = 0;
	
	error = 0;
		
	/* Interpret the command line */
	for (i = 1; i < argc; i++)
	{
		switch (argv[i][0]) /* ? */
		{
			case '-' :
				num_flags++;
				
				switch (toupper(argv[i][1])) /* -? */
				{
					case 'B' : /* local buffer size */
						error = option_B(argv, std_args, &i);
					break;
					
					case 'C' : /* head 'n tail count */
						error = option_C(argv, std_args, &i);
					break;
				
					case 'F' : /* single format file/title */
						error = option_F_(argv, std_args, &i);
					break;
					
					case 'I' : /* input format file/title */
						error = option_I__(argv, std_args, &i);
					break;
	
					case 'M' : /* Checkvar maxbins, missing data, or max/min only */
						error = option_M_(argv, std_args, &i);
					break;
					
					case 'O' : /* output data file, or output format file/title */
						error = option_O__(argv, std_args, &i);
					break;
					
					case 'P' : /* Checkvar precision */
						error = option_P(argv, std_args, &i);
					break; /* Checkvar precision */
					
					case 'Q' : /* query file */
						error = option_Q(argv, std_args, &i);
					break;
					
					case 'V' : /* variable file */
						error = option_V(argv, std_args, &i);
					break;
					
					default :
						if (i > 1)
						{
							sprintf(err_buf, "==> %s <==", argv[i]);
							error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
						}
					break;
				} /* switch on flag letter code */
			break; /* case '-' */

			default :
				if (i > 1)
				{
					sprintf(err_buf, "Expecting an option flag (beginning with '-')\n==> %s <==",
					        argv[i]);
					error = err_push(ROUTINE_NAME, ERR_UNKNOWN_OPTION, err_buf);
				}
			break;
		} /* switch on argv[?][0] */
	} /* End of Command line interpretation (for loop)*/

	/* Reading from stdin if no filename argument given.  Could determine
	   this by seeing if argv[1][0] == '-', but I'll allow that the data
	   file name may begin with a '-'.  Instead, count the number of command-
	   line arguments, minus flag/option pairs (-mm has no mate).  If the
	   number is two, filename was given, if the number is one, not. */
	   
	if (argc - ((2 * num_flags) - (std_args->user.set_cv_maxmin_only ? 1 : 0)) == 
	    2)
	{
		std_args->user.set_input_file = 1;
		std_args->input_file = argv[1];
	}

	return(error);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "make_standard_dbin"

int make_standard_dbin(FFF_STD_ARGS_PTR std_args, DATA_BIN_HANDLE dbin,
                       int (*error_cb)(int))
/*****************************************************************************
 * NAME:  make_standard_dbin()
 *
 * PURPOSE:  Makes a data bin and calls db_set() events according to FreeForm
 * standard arguments structure
 *
 * USAGE:  error = make_standard_dbin(std_args, &dbin, call_back);
 *
 * RETURNS:  NULL on (relative) success or: 1) ERR_MEM_LACK if db_make() or
 * malloc() fails, 2) (if reading from standard input) EINVAL or EBADF if
 * file length cannot be found or ERR_BINFORM_DEFINED if no input format file
 * is defined, or 3) the data bin event which failed, PROVIDED THAT the error
 * call-back function returns a non-zero result taking that event code as
 * argument.
 *
 * DESCRIPTION:  Allocates the scratch buffer, and calls the following events:
 * DBIN_FILE_NAME, DBIN_FILE_HANDLE, CHECK_FILE_SIZE, BUFFER, MAKE_NAME_TABLE,
 * INPUT_FORMAT, OUTPUT_FORMAT, DBIN_CACHE_SIZE, DBIN_BYTE_ORDER,
 * and DBIN_SET_TOTAL_RECORDS.
 *
 * If the call-back function is NULL, then make_standard_dbin returns with an
 * error code on the first error.
 *
 * The following are possible error return codes:
 *
 * DBIN_FILE_NAME, DBIN_FILE_HANDLE, CHECK_FILE_SIZE, BUFFER, MAKE_NAME_TABLE,
 * INPUT_FORMAT, OUTPUT_FORMAT, DBIN_CACHE_SIZE, DBIN_BYTE_ORDER,
 * DBIN_SET_TOTAL_RECORDS, ERR_MEM_LACK, EINVAL, EBADF, ERR_BINFORM_DEFINED
 *
 * The data bin handle (DATA_BIN_HANDLE dbin) may point to a non-NULL address,
 * in which case it is assumed that the data bin has been previously allocated,
 * and that make_standard_dbin() should only fill the DATA_BIN, and not create
 * one from scratch.  This is taken advantage of in newform(), when called
 * by DoNewform() in topmenu.c (GeoVu), so that the existing menu information
 * (e.g., name table information) can be "grafted" into the newform()-local
 * DATA_BIN from the GeoVu DATA_BIN.  Incidentally, this involves a fair amount
 * of redundancy, in that make_standard_dbin() will be repeating much of the
 * work already done in db_create(), but this is to insulate newform() operations
 * on the DATA_BIN from the GeoVu DATA_BIN.
 *
 * Needless to say, this is an extremely dangerous feature to allow, because
 * if a non-GeoVu application calls newform() with an uninitialized data bin
 * pointer, all sorts of memory tromps may happen.  So, NEVER DO THIS:
 *
 * DATA_BIN_PTR dbin;
 * make_standard_dbin(std_args_ptr, &dbin, ferr_cb);
 *
 * NEVER DO THE ABOVE!  Instead, do this...
 *
 * DATA_BIN_PTR dbin = NULL;
 * make_standard_dbin(std_args_ptr, &dbin, ferr_cb);
 *
 * OR, do this...
 *
 * DATA_BIN_PTR dbin;
 * ...
 * dbin = NULL;
 * make_standard_dbin(std_args_ptr, &dbin, ferr_cb);
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
	char error_buf[MAX_ERRSTR_BUFFER];
	
	long file_size = 0L;
			
	/* Replace the input and output format file names with FORMAT_BUFFER_IS_A_TITLE
	   if they have not been specified and format titles have been */
	if (!std_args->input_format_file && std_args->input_format_title)
		std_args->input_format_file = FORMAT_BUFFER_IS_A_TITLE;

	if (!std_args->output_format_file && std_args->output_format_title)
		std_args->output_format_file = FORMAT_BUFFER_IS_A_TITLE;
	
	/* Allocate scratch buffer */
	if (std_args->local_buffer_size == 0)
		std_args->local_buffer_size = DEFAULT_BUFFER_SIZE;

	std_args->local_buffer = (char *)memMalloc((size_t)std_args->local_buffer_size, "std_args->local_buffer");
	if (!(std_args->local_buffer))
	{
		*dbin = NULL;
		return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Local Buffer"));
	}

	/* If input is coming from standard input, a format must be specified */
	if (!std_args->user.set_input_file && !std_args->user.set_input_format_file)
	{
		*dbin = NULL;
		return(err_push(ROUTINE_NAME, ERR_BINFORM_DEFINED, "Standard Input Without Format"));
	}
	
	if (*dbin == NULL)
	{
		*dbin = db_make((char *)(std_args->user.set_input_file ? std_args->input_file : "standard input"));
		if (!*dbin)
			return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "Standard Data Bin"));
	}

	/* Set the input file for the data bin */
	if (std_args->user.set_input_file)
	{
		if (db_set(*dbin,
			DBIN_FILE_NAME, std_args->input_file,
			END_ARGS))
		{
			sprintf(error_buf, "Setting file name for %s", std_args->input_file);
			err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
			if (!error_cb || error_cb(DBIN_FILE_NAME))
				return(DBIN_FILE_NAME);
		}
	
		if (db_set(*dbin,
			DBIN_FILE_HANDLE,
			END_ARGS))
		{
			sprintf(error_buf, "Setting file handle for %s", std_args->input_file);
			err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
			if (!error_cb || error_cb(DBIN_FILE_HANDLE))
				return(DBIN_FILE_HANDLE);
		}
	} /* not standard input */
	else
	{
		if (db_set(*dbin,
			DBIN_FILE_NAME, "Standard Input",
			END_ARGS))
		{
			sprintf(error_buf, "Setting file name for %s", std_args->input_file);
			err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
			if (!error_cb || error_cb(DBIN_FILE_NAME))
				return(DBIN_FILE_NAME);
		}

		/* The DBIN_FILE_HANDLE event usually opens the input file
		and sets the file handle if the input file can be opened. In the standard
		input case, the file handle must be set directly and the mode of stdin
		must be set to binary */
		(*dbin)->data_file = fileno(stdin);
#ifdef CCMSC
		setmode((*dbin)->data_file, O_BINARY);
#endif
		(*dbin)->state.std_input = 1;
		(*dbin)->file_location = 0L;
		if ((file_size = os_filelength((*dbin)->file_name)) < 0)
			return(err_push(ROUTINE_NAME, errno, (*dbin)->file_name));

		(*dbin)->data_available = (*dbin)->bytes_available = file_size;
	} /* if standard input */

	if (db_set(*dbin,
		BUFFER, std_args->local_buffer,
		END_ARGS))
	{
		sprintf(error_buf, "Setting buffer for %s", std_args->input_file);
		err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
		if (!error_cb || error_cb(BUFFER))
			return(BUFFER);
	}

	/* Now set the formats and the auxillary files */

	if (db_set(*dbin,
		MAKE_NAME_TABLE, (void*)NULL,
		END_ARGS))
	{
		sprintf(error_buf, "making name table for %s", std_args->input_file);
		err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
		if (!error_cb || error_cb(MAKE_NAME_TABLE))
			return(MAKE_NAME_TABLE);
	}
	
	if (!(*dbin)->input_format)
	{
		if (db_set(*dbin,
			INPUT_FORMAT, std_args->input_format_file, std_args->input_format_title,
			END_ARGS))
		{
			sprintf(error_buf, "setting input format for %s", std_args->input_file);
			err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
			if (!error_cb || error_cb(INPUT_FORMAT))
				return(INPUT_FORMAT);
		}
	}
	
	if (!(*dbin)->output_format)
	{
		if (db_set(*dbin,
			OUTPUT_FORMAT, std_args->output_format_file, std_args->output_format_title,
			END_ARGS))
		{
			sprintf(error_buf, "setting output format for %s", std_args->input_file);
			err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
			if (!error_cb || error_cb(OUTPUT_FORMAT))
				return(OUTPUT_FORMAT);
		}
	}
	
	if (std_args->user.set_input_file)
		if (db_set(*dbin, CHECK_FILE_SIZE, END_ARGS))
		{
			sprintf(error_buf, "file size check for %s", std_args->input_file);
			err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
			if (!error_cb || error_cb(CHECK_FILE_SIZE))
				return(CHECK_FILE_SIZE);
		}

	if (std_args->records_to_read != 0 && db_set(*dbin,
		DBIN_SET_TOTAL_RECORDS, std_args->records_to_read,
		END_ARGS))
	{
		sprintf(error_buf, "Setting starting record for %s", std_args->input_file);
		err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
		if (!error_cb || error_cb(DBIN_SET_TOTAL_RECORDS))
			return(DBIN_SET_TOTAL_RECORDS);
	}

	if (db_set(*dbin,
		DBIN_CACHE_SIZE, std_args->cache_size,
		END_ARGS))
	{
		sprintf(error_buf, "setting data cache for %s", std_args->input_file);
		err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
		if (!error_cb || error_cb(DBIN_CACHE_SIZE))
			return(DBIN_CACHE_SIZE);
	}

	if (db_set(*dbin,
		DBIN_BYTE_ORDER,
		END_ARGS))
	{
		sprintf(error_buf, "setting data byte order for %s", std_args->input_file);
		err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buf);
		if (!error_cb || error_cb(DBIN_BYTE_ORDER))
			return(DBIN_BYTE_ORDER);
	}

	return(0);
}

