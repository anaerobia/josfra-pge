/*
 * NAME:	getll
 *
 * PURPOSE:	This program reads latitude and longitude in any recognized format,
 * converting to values in decimal degrees.
 *
 * AUTHOR: Ted Habermann, NGDC, (303) 497-6472, haber@mail.ngdc.noaa.gov
 * Modified (MAO)
 *
 * USAGE:		getll data_file
 *
 * COMMENTS:
 *
 * FreeForm applications are designed to run on many different types of
 * computers. One of the differences between these computers is the names
 * of various include files. These differences are taken care of by defining
 * your environment by defining one of the following three preprocessor
 * macros:  1) CCMSC (PC, Microsoft C), 2) SUNCC (Unix workstation, ANSI C),
 * or 3) CCLSC (Macintosh, ANSI C).  
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

#define ROUTINE_NAME "getll"

/* An error call back routine -- it tells make_standard_dbin which events
   are okay if they fail.  getll "dynamically" creates the output data
   format, and throws away any existing output data format, so we don't
   require an output data format in the format file.  This function allows
   make_standard_dbin() to process other events, even if the OUTPUT_FORMAT
   event fails to produce an output format.
*/
#ifdef PROTO
static int mkstdbin_cb(int routine_name)
#else
static int mkstdbin_cb(routine_name)
int routine_name;
#endif
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
 * The following "standard" command line options are not used by this
 * application:
 *
 * -v  variable file
 * -q  query file
 * -p  precision (checkvar only)
 * -md missing data flag (checkvar only)
 * -mb maximum number of bins (checkvar only)
 * -mm maximum/minimum processing only (checkvar only)
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
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, 
		         "variable file"
		        );
		}
	
	if (std_args->user.set_query_file)
		{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION,
		         "query file"
		        );
		}
	
	if (std_args->user.set_cv_precision)
		{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION,
		         "precision (checkvar only)"
		        );
		}
	
	if (std_args->user.set_cv_missing_data)
		{                                        
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION,
		         "missing data flag (checkvar only)"
		        );
		}
	
	if (std_args->user.set_cv_maxbins)
		{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION,
		        "maximum number of histogram bins (checkvar only)"
		        );
		}
	
	if (std_args->user.set_cv_maxmin_only)
		{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION,
		         "maximum and minimum processing only (checkvar only)"
		        );
		}
	
	if (err_state())
		{
		err_disp();
		}
	}

#ifdef PROTO
void main(int argc, char *argv[])
#else
void main(argc, argv)
int argc;
char *argv[]
#endif
	{
	int error = 0;    /* to hold error return values */

	char *output_buffer = NULL; /* output data buffer */
	long  output_bytes  = 0;    /* bytes written into output buffer */

	FFF_STD_ARGS  std_args; /* holds command line information */
	DATA_BIN_PTR  input         = NULL; /* the data bin */
	FILE         *pfile         = NULL; /* output file */
	     
	if (argc == 1)
		{
		fprintf(stderr, "%s%s",
#ifdef ALPHA
"\nWelcome to getll alpha "FF_LIB_VER" "__DATE__\
" -- an NGDC FreeForm example application\n\n",
#elif defined(BETA)
"\nWelcome to getll beta "FF_LIB_VER" "__DATE__\
" -- an NGDC FreeForm example application\n\n",
#else
"\nWelcome to getll release "FF_LIB_VER\
" -- an NGDC FreeForm example application\n\n",
#endif
"Default extensions: .bin = binary, .dat = ASCII, .dab = dBASE\n\
\t.fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE variable description file\n\n\
getll data_file [-f format_file] [-if input_format_file]\n\
                [-of output_format_file] [-ft \"format title\"]\n\
                [-ift \"input format title\"] [-oft \"output format title\"]\n\
                [-c count] No. records to process at head(+)/tail(-) of file\n\
                [-o output_file] default = output to screen\n\n\
See the FreeForm User's Guide for detailed information.\n"
		       );
		exit(EXIT_FAILURE);
		}
     
	 /* The FREEFORM system uses a hierarchical error handling system
	 which allows each layer of an application to add error messages to
	 a queue. err_push is the function which adds messages to the queue.
	 It is called by any function which runs into an error. err_disp is
	 the function that interactivly displays those errors to the user.
	 It is called by the main application program when an error occurs.*/
	     
	 /* Allocate the output buffer:
	 FREEFORM uses two types of buffers extensively and defines default
	 buffer sizes in the include file freeform.h. The local or scratch
	 buffers are used as temporary work space. The cache buffers are
	 used for reading large blocks of data.*/
     
	output_buffer = (char *)malloc((size_t)DEFAULT_CACHE_SIZE);
	if (!output_buffer)
		{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "Output Buffer");
		err_disp();
		exit(EXIT_FAILURE);
		}
     
	/* Collect options entered on the command line, this information will be
	   used in the call to make_standard_dbin(), below.
	*/
	if (parse_command_line(argc, argv, &std_args))
		{
		free(output_buffer);

		err_disp();
		exit(EXIT_FAILURE);
		}
	check_for_unused_flags(&std_args);

	/* Create and initialize the data bin */
	if (make_standard_dbin(&std_args, &input, mkstdbin_cb))
		{
		free(output_buffer);

		err_disp();
		exit(EXIT_FAILURE);
		}
	
	/* make_standard_dbin may have generated an incidental error, in case the
	   OUTPUT_FORMAT event failed.  mkstdbin_cb downgrades such an error from
	   a terminal error to a warning, but an error message might still have
	   been queued.  If so, clear it.
	*/
	if (err_state())
		err_clear();
	
	/* Has user indicated an output file? */
	if (std_args.output_file)
		{
		pfile = fopen(std_args.output_file, "wb");
		if (!pfile)
			{
			free(output_buffer);

			err_push(ROUTINE_NAME, ERR_CREATE_FILE, std_args.output_file);
			err_disp();
			exit(EXIT_FAILURE);
			}
		}
	else
		{
		/* If not, write to standard output */
		pfile = stdout;
		}
			
	/* Rather than using an output format contained in a file, create a
	   "dynamic" buffer, write an output format description into it, and
	   use that to initialize the data bin's output format
	*/
     
	sprintf(output_buffer, "\
ASCII_output_data \"hard-coded in getll.c:main()\"\n\
longitude  1 11 double 6\n\
latitude  13 25 double 6\n"
         );
         
  /* Use the FORMAT_BUFFER event to set the output format.  The data bin
     knows that this is an output format because of the format type,
     "ASCII_output_data".
  */
	if (db_set(input,
	               OUTPUT_FORMAT, NULL, output_buffer,
	           END_ARGS
	          )
	   )
		{
	/* Error in the output format creation -- this must never happen!
	   Ensure that the output buffer is syntactically correct, since it
	   is hard-coded into the program!
	*/
		free(output_buffer);
		if (std_args.output_file)
			fclose(pfile);

		err_push(ROUTINE_NAME, ERR_MAKE_FORM, output_buffer);
		err_disp();
		exit(EXIT_FAILURE);
		}
	
	/* Display some information about the data formats */
	db_show(input, SHOW_FORMAT_LIST, FFF_INFO, END_ARGS);
	/* db_show writes into data bin's working buffer */
	fprintf(stderr, "%s", input->buffer);

	/*
	** process the data
	*/
     
	/* use PROCESS_FORMAT_LIST to fill cache and fill headers */
	while ((error = db_events(input,
		                           PROCESS_FORMAT_LIST, FFF_ALL_TYPES,
		                        END_ARGS
	                         )
	       ) == 0
	      )
		{
		/* Make sure output buffer is large enough for the cache */
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
			/* The default cache size was too small for the number of output bytes
			   needed.  This contigency is coded for, but is extremely unlikely
			   to happen.  However, it is possible that the program will error out
			   if it can not resize the output buffer.
			*/
			char *cp = NULL;
			
			cp = (char *)realloc(output_buffer, (size_t)output_bytes);
			if (cp)
				{
				output_buffer = cp;
				}
			else
				{
				error = 1;
				err_push(ROUTINE_NAME, ERR_MEM_LACK,
				         "reallocation of output_buffer");
				break;
				}
			}
         
		/* Convert the cache into the output_buffer -- this will perform a binary
		   to ASCII conversion if necessary.  (The example shows getll running on
		   llmaxmin.dat, an ASCII file, but this program works equally well on
		   llmaxmin.bin, which is created by running newform on llmaxmin.dat.)
		*/
		error = db_events(input,
		                      DBIN_DATA_TO_NATIVE, NULL, NULL, NULL,
		                      DBIN_CONVERT_CACHE, output_buffer, NULL, &output_bytes,
		                  END_ARGS
		                 );
		if (error)
			break;

		/* Write the contents of the output buffer to the file (or screen). */
		if (  (long)fwrite(output_buffer, 
		                   sizeof(char), 
		                   (size_t)output_bytes,
		                   pfile
		                  )
		    < output_bytes
		   )
			{
			err_push(ROUTINE_NAME, ERR_WRITE_FILE,   std_args.output_file
			                                       ? std_args.output_file
			                                       : "to screen"
			        );
			break;
			}
		}/* End Processing */

	if (std_args.output_file)
		fclose(pfile);

	free(output_buffer);
	
	/* Deallocate all memory assocated with the data bin */
	db_free(input);

	/* The error stack is checked to see if anything went wrong during processing */
	if ((error && error != EOF) || err_state())
		{
		err_push(ROUTINE_NAME, ERR_PROCESS_DATA, NULL);
		err_disp();
		exit(EXIT_FAILURE);
		}

	} /* end main() for program getll */

