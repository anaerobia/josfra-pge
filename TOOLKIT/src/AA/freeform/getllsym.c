/*
 * NAME:	getllvar
 *
 * USAGE:	main(int argc, char *argv[])
 *
 * PURPOSE:	This program reads latitude and longitude from any format
 *			plus a third variable determined from the command line
 *
 * AUTHOR:	Ted Habermann, NGDC, (303) 497-6472, haber@ngdc.noaa.gov
 * modified (TAM)
 * modified (MAO)
 *
 * USAGE:	getllvar data_file variable [format.afm/bfm/fmt] > output
 *
 * COMMENTS:  See the source code for getll.c for a more basic play-by-play
 * discussion.  getllvar.c is the same code as getll.c, plus a little extra
 * to allow displaying a third variable in addition to latitude and longitude.
 * 
 * The third variable is typed on the command line, but the preferred method
 * is to use a variable file with the -v option.  newform provides all the
 * capability of getll and getllvar (plus a lot more!), but this example
 * serves as a small enhancement of getll.
 *
 * Because the third variable typed on the command line deviates from the
 * standard FreeForm command line syntax, and non-standard options are
 * treated as errors, the command line is "pre-processed" before
 * parse_command_line is called.
 * 
 */

#include <limits.h>

/* The FreeForm include file is surrounded by a definition of the
 * constant DEFINE_DATA in the main program so that extern arrays that
 * FreeForm uses get initialized.  The DEFINE_DATA constant must not be
 * defined in any other files.
 */
 
#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

/* This include file defines the data objects */
#include <databin.h>

#define ROUTINE_NAME "getllvar"

static int mkstdbin_cb(int routine_name)
{
	return(routine_name != OUTPUT_FORMAT);
}

/*****************************************************************************
 * NAME:  check_for_unused_flags()
 *
 * PURPOSE:  Has user asked for an unimplemented option?
 *
 * USAGE:  check_for_unused_flags(std_args_ptr);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  All FreeForm utilities do not employ all of the "standard"
 * FreeForm command line options.  Check if the user has unwittingly asked
 * for any options which this utility will ignore.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC
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
static void check_for_unused_flags(FFF_STD_ARGS_PTR std_args)
#else
static void check_for_unused_flags(std_args)
FFF_STD_ARGS_PTR std_args;
#endif
{
	if (std_args->user.set_var_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "variable file");
	}
	
	if (std_args->user.set_query_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "query file");
	}
	
	if (std_args->user.set_cv_precision)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "precision (checkvar only)");
	}
	
	if (std_args->user.set_cv_missing_data)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "missing data flag (checkvar only)");
	}
	
	if (std_args->user.set_cv_maxbins)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "maximum number of histogram bins (checkvar only)");
	}
	
	if (std_args->user.set_cv_maxmin_only)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "maximum and minimum processing only (checkvar only)");
	}
	
	if (err_state())
		err_disp();
}

#ifdef PROTO
void main(int argc, char *argv[])
#else
void main(argc, argv)
int argc;
char *argv[]
#endif
{
	char	       *output_buffer = NULL;
	long          output_bytes  =    0;
	int           error         =    0;
	int           var_length    =    0; /* field width for the third variable */
	char         *var_name      = NULL; /* name of the third variable */
	VARIABLE_PTR  var           = NULL; /* this structure holds information about the third variable */
	DATA_BIN_PTR  input         = NULL;
	FILE         *pfile         = NULL;
	
	int i;
	
	int    quasi_argc; /* substitute for argc */
	char **quasi_argv; /* substitute for argv */

	FFF_STD_ARGS std_args;
	
	/* argc is compared against two because of the third variable typed on the
	   command line.
	*/
	if (argc <= 2)
	{
		fprintf(stderr, "%s%s",
#ifdef ALPHA
"\nWelcome to getllvar alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm example application\n\n",
#elif defined(BETA)
"\nWelcome to getllvar beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm example application\n\n",
#else
"\nWelcome to getllvar release "FF_LIB_VER" -- an NGDC FreeForm example application\n\n",
#endif
"Default extensions: .bin = binary, .dat = ASCII, .dab = dBASE\n\
\t.fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE variable description file\n\n\
getllvar data_file variable [-f format_file] [-if input_format_file]\n\
                   [-of output_format_file] [-ft \"format title\"]\n\
                   [-ift \"input format title\"] [-oft \"output format title\"]\n\
                   [-c count] No. records to process at head(+)/tail(-) of file\n\
                   [-o output_file] default = output to screen\n\n\
See the FreeForm User's Guide for detailed information.\n"
		       );
		exit(EXIT_FAILURE);
	}
     
	output_buffer = (char *)malloc((size_t)DEFAULT_CACHE_SIZE);
	if (!output_buffer)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Output Buffer");
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	/* This is a special hack to use a non-standard command line argument
	   and not err-out in make_standard_dbin, which expects only standard
	   command line arguments.
	*/
	
	var_name = argv[2]; /* remember the third command line argument */
	
	quasi_argc = argc - 1;
	quasi_argv = (char **)malloc(sizeof(char *) * argc);
	if (quasi_argv == NULL)
	{
		err_push(  ROUTINE_NAME, ERR_MEM_LACK
		         , "Please unload unnecessary resources (e.g., TSR's) , and try again"
		        );
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	quasi_argv[0] = argv[0];
	quasi_argv[1] = argv[1];
	
	for (i = 2; i < argc; i++)
		quasi_argv[i] = argv[i + 1];
     
	if (parse_command_line(quasi_argc, quasi_argv, &std_args))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	free(quasi_argv);
	
	check_for_unused_flags(&std_args);
	
	if (make_standard_dbin(&std_args, &input, mkstdbin_cb))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
			
	if (err_state())
		err_clear();
	
	if (std_args.output_file)
	{
		pfile = fopen(std_args.output_file, "wb");
		if (!pfile)
		{
			err_push(ROUTINE_NAME, ERR_CREATE_FILE, std_args.output_file);
			err_disp();
			exit(EXIT_FAILURE);
		}
	}
	else
		pfile = stdout;
			
	/* Find out some things about the third variable */

	var = ff_find_variable(var_name, input->input_format);
	if (var == NULL)
	{
		err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND, var_name);
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	if (IS_ASCII(input->input_format))
		var_length = FF_VAR_LENGTH(var);
	else
	{
		/* The input format is binary, so we'll have to make an educated guess
	     as to the field width for the variable type in its ASCII representation.
	   */

		/* A quick and dirty table of variable types and sizes.  Except for
		   the float and double variable types, the sizes given are the maximum
		   field width possible.  The sizes for float and double are just guesses.
		*/
		struct 
		{
			FF_TYPES_t var_type;
			int        field_width;
		} ASCII_sizes[] = 
		{{FFV_NULL,    0},
		 {FFV_UCHAR,   3},
		 {FFV_USHORT,  5},
		 {FFV_SHORT,   6},
		 {FFV_ULONG,  10},
		 {FFV_LONG,   11},
		 {FFV_FLOAT,  15},
		 {FFV_DOUBLE, 20}
		};

		if (IS_CHAR(var))
		{
			/* This is a character string stored in a binary file.  It is safe to
			   use the binary field width, since the character string is the exact
			   same size stored in an ASCII file.
			*/
			var_length = FF_VAR_LENGTH(var);
		}
		else
		{
			var_length = 0;
			for (i = sizeof(ASCII_sizes) / sizeof(ASCII_sizes[0]); i; i--)
				if (FFV_TYPE(var) == ASCII_sizes[i - 1].var_type)
				{
					var_length = ASCII_sizes[i - 1].field_width;
					break;
				}
			
			if (!var_length)
			{
				/* If the type-size table above is complete, this can never happen. */
				err_push(ROUTINE_NAME, ERR_UNKNOWN
, "Cannot determine field width for symbol using ASCII representation");
				err_disp();
				exit(EXIT_FAILURE);
			}
		}
	}

	sprintf(output_buffer, "\
ASCII_output_data \"hard-coded in getllvar.c:main()\"\n\
longitude 1  11 double 6\n\
latitude  13 25 double 6\n\
%s 27 %d %s %d\n",
var->name, 27 + var_length - 1, ff_lookup_string(variable_types, var->type), var->precision
         );
	if (db_set(input,
	               OUTPUT_FORMAT, NULL, output_buffer,
	           END_ARGS
	          )
	   )
	{
		err_push(ROUTINE_NAME, ERR_MAKE_FORM, output_buffer);
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	db_show(input, SHOW_FORMAT_LIST, FFF_INFO, END_ARGS);
	fprintf(stderr, "%s", input->buffer);

	/*
	** process the data
	*/
     
	while ((error = db_events(input,
		                           PROCESS_FORMAT_LIST, FFF_ALL_TYPES,
		                        END_ARGS
		                       )
		     ) == 0
		    )
	{
		db_show(input, 
		            DBIN_BYTE_COUNTS, DBIN_OUTPUT_CACHE, &output_bytes, END_ARGS,
		        END_ARGS
		       );

		if ((unsigned long)output_bytes > (unsigned long)UINT_MAX)
		{
			error = 1;
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation size too big");
			break;
		}
		if (output_bytes > DEFAULT_CACHE_SIZE)
		{
			output_buffer = (char *)realloc(output_buffer, (size_t)output_bytes);
			if (!output_buffer)
			{
				error = 1;
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation of output_buffer");
				break;
			}
		}
         
		error = db_events(input,
		                      DBIN_DATA_TO_NATIVE, NULL, NULL, NULL,
		                      DBIN_CONVERT_CACHE, output_buffer, NULL, &output_bytes,
		                  END_ARGS
		                 );
		if (error)
			break;

		if (  (long)fwrite(output_buffer, sizeof(char), (size_t)output_bytes, pfile)
		    < output_bytes
		   )
		{
			err_push(ROUTINE_NAME, ERR_WRITE_FILE,   std_args.output_file
			                                       ? std_args.output_file
			                                       : "to screen"
			        );
			break;
		}
	} /* End Processing */

	if (std_args.output_file)
		fclose(pfile);

	free(output_buffer);
	db_free(input);

	if ((error && error != EOF) || err_state())
	{
		err_push(ROUTINE_NAME, ERR_PROCESS_DATA, NULL);
		err_disp();
		exit(EXIT_FAILURE);
	}

} /* end main() for program getllvar */

