#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#ifdef CCMSC
#include <conio.h>
#endif

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "gethdr"

#define LOCAL_BUFFER_SIZE 61440
#define LINE_SIZE 256

#define GREET           1
#define FMT             2

int print_info(int);

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

static void check_for_unused_flags(FFF_STD_ARGS_PTR std_args)
{
	if (std_args->user.set_var_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "variable file");
	}
	
	if (std_args->user.set_query_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "query file");
	}
	
	if (std_args->user.set_records_to_read)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "\"head 'n tail\" record count");
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
	
/*
 * NAME:		gethdr
 *
 * PURPOSE:	To get the headers from a data set.
 *			Messeges and display of headers are written to stderr
 *			Converted headers are written to stdout
 *                      
 * AUTHOR:   TAM, NGDC, tam@ngdc.noaa.gov
 * (Modified) MAO
 *
 * USAGE:	
 *
 * COMMENTS: 
 *
 */

void main(int argc, char **argv)
{
/* System Dependent Definitions */

#ifdef CCMSC
	char eol_string[] = {'\015', '\012', '\0'};
#endif
#ifdef CCLSC
	char eol_string[] = {'\015', '\0'};
#endif
#ifdef SUNCC
	char eol_string[] = {'\012', '\0'};
#endif

	DATA_BIN_PTR	input	= NULL;		/* This program uses one data bin */

	FORMAT_PTR header_output_format = NULL;
	FILE *pfile = NULL;

	char 		*buffer 	= NULL;
	char 		*point 		= NULL;
	char		*data_file  = NULL;
	char		*dbin_format_file   = NULL;
	char		*header_format_file = NULL;
	char		*tmp_ptr = NULL;
	char		*output_format_name = NULL;

	int error 	= 0;
	int done 	= 0;

	long input_bytes	= 0L;
	long output_bytes	= 0L;

	unsigned num_vars = 0;
	size_t out_count = 0;
	
	FFF_STD_ARGS std_args;
	                        
	if (argc == 1)
	{
	    print_info(GREET);
	    exit(EXIT_FAILURE);
	}
	
	if (parse_command_line(argc, argv, &std_args))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
  check_for_unused_flags(&std_args);
  
	/* Allocate scratch buffer :
	Use the local buffer to translate data into. make_standard_dbin
	sets the size of that buffer to DEFAULT_BUFFER_SIZE, which is somewhat small
	(32K). The size of this buffer can be controlled from the command line, but
	we set the default here to LOCAL_BUFFER_SIZE (defined above). This
	overrides the definition in make_standard_dbin. */

  /* Has the user specified a specific buffer size?  If not, override
     the default with a size appropriate for this application. */
	if (!std_args.user.set_local_buffer_size)
		std_args.local_buffer_size = LOCAL_BUFFER_SIZE;

	input = NULL;
	if (make_standard_dbin(&std_args, &input, mkstdbin_cb))
	{
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, *(argv + 1));
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
	
	buffer = std_args.local_buffer;

#ifdef DEBUG_MSG
	fprintf(stderr,"Local Buffer Size: %ld\n", std_args.local_buffer_size);
#endif

	/*
	** SET UP THE Input Data Bin:
	*/

	/* Check the format list for a header output format */
	header_output_format = db_find_format(  input->format_list
	                                      , FFF_GROUP
	                                      , FFF_HD | FFF_OUTPUT
	                                      , END_ARGS
	                                     );

	/*
	** process the headers 
	*/

	if (header_output_format)
		fprintf(stderr,"\n\nConverting headers for file %s:\n\n", input->file_name);
	else	
		fprintf(stderr,"\n\nHeaders being displayed for %s:\n\n", input->file_name);
	fflush(stdout);

	/* Determine if the headers are in a separate file */
	if (IS_SEPARATE(input->header_format))
	{
		error = db_set(  input
		               , DBIN_FILE_NAME, input->header_file_name
		               , DBIN_FILE_HANDLE
		               , END_ARGS
		              );
		if (error)
		{
			err_push(ROUTINE_NAME, ERR_SET_DBIN, data_file);
			err_disp();
			exit(EXIT_FAILURE);
		}

		/* delete the data formats from the format list */
		/* Graft the header format to the input format */
		db_delete_format(input->format_list, FFF_INPUT | FFF_DATA);
		input->input_format = input->header_format; 
		input->input_format->type = FFF_BINARY | FFF_INPUT | FFF_DATA;

		db_delete_format(input->format_list, FFF_OUTPUT | FFF_DATA);
		input->output_format = NULL;

		memFree(input->header, "input->header");

		if (header_output_format)
		{
			input->output_format = header_output_format;

			if (IS_ASCII(input->output_format))
				input->output_format->type = FFF_ASCII | FFF_OUTPUT | FFF_DATA;
			else if (IS_BINARY(input->output_format))
				input->output_format->type = FFF_BINARY | FFF_OUTPUT | FFF_DATA;
			else
				input->output_format->type = FFF_DBASE | FFF_OUTPUT | FFF_DATA;
		}			
		
		while (!done)
		{
			/* use PROCESS_FORMAT_LIST to fill cache and fill headers */
			error = db_events(input, PROCESS_FORMAT_LIST,
							FFF_ALL_TYPES, END_ARGS);

			if (error && error != EOF)
			{
				done++;
				err_push(ROUTINE_NAME, ERR_PROCESS_DATA, NULL);
				break;
			}
			else if (error == EOF)
			{
				done++;
				error = 0;
			}
	
			/* Make sure buffer is large enough for the cache */
			(void)db_show(input, DBIN_BYTE_COUNTS, &input_bytes, &output_bytes, NULL);
			
			if ((unsigned long)output_bytes > (unsigned long)UINT_MAX)
			{
				error = 1;
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation size too big");
				break;
			}
			if (output_bytes > std_args.local_buffer_size)
			{
				buffer = (char *)memRealloc((char *)buffer, (size_t)output_bytes, "buffer");
			}
			if (!buffer)
			{
				error = 1;
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation of buffer");
				break;
			}
	
			/* Convert the cache into the buffer */
			output_bytes = 0L;
			error = db_events(  input
			                  , DBIN_CONVERT_CACHE, buffer, (void*)NULL, &output_bytes
			                  , END_ARGS
			                 );
			if (error)
				break;
	
			out_count = fwrite(buffer, sizeof(char), (size_t)output_bytes, pfile);
			if ((long)out_count < output_bytes)
			{
				err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
				err_disp();
				exit(EXIT_FAILURE);
			}	
		}/* End Processing */

		done++;

	} /* End of separate headers */
  else
  {
		while (!done)
		{
			/* use PROCESS_FORMAT_LIST to fill headers */
			if (db_events(input, PROCESS_FORMAT_LIST, FFF_HD, END_ARGS))
				break;
	
			/* If there is an output format, show the converted header */
			if (header_output_format && input->state.new_record)
			{
				error = db_events(input, DBIN_CONVERT_HEADER, buffer, header_output_format, END_ARGS);
				if (error)
					break;

				/* Don't use FORMAT_LENGTH, it adds bytes for EOL if ASCII */
				out_count = fwrite(buffer, sizeof(char), header_output_format->max_length, pfile);
				if (out_count < header_output_format->max_length)
				{
					err_push(ROUTINE_NAME, ERR_WRITE_FILE, std_args.output_file ? std_args.output_file : "to screen");
					err_disp();
					exit(EXIT_FAILURE);
				}	

				if (IS_ASCII(header_output_format))
				{
					out_count = fwrite(eol_string, sizeof(char), EOL_SPACE, pfile);
					if (out_count < EOL_SPACE)
					{
						err_push(ROUTINE_NAME, ERR_WRITE_FILE, std_args.output_file ? std_args.output_file : "to screen");
						err_disp();
						exit(EXIT_FAILURE);
					}	
				}
	
				input->state.new_record = 0;
			}
			else if (IS_REC(input->header_format) && input->state.new_record)
			{ /* There is no output format, so show record or variables */
				if ((IS_BINARY(input->header_format)))
				{ /* show the header variables */
					error = db_show(input, DBIN_HEADER, buffer, (void *)NULL, END_ARGS);
					if (error)
						break;
					printf("%s\n", buffer);
				}
				else
				{
					out_count = fwrite(input->header, sizeof(char), input->header_format->max_length, pfile);
					if (out_count < input->header_format->max_length)
					{
						err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
						err_disp();
						exit(EXIT_FAILURE);
					}	

					if (!IS_BINARY(input->header_format))
					{
						out_count = fwrite(eol_string, sizeof(char), EOL_SPACE, pfile);
						if (out_count < EOL_SPACE)
						{
							err_push(ROUTINE_NAME, ERR_WRITE_FILE, std_args.output_file ? std_args.output_file : "to screen");
							err_disp();
							exit(EXIT_FAILURE);
						}	
					}
				}
				input->state.new_record = 0;
	
			} /* end of RECORD HEADER */
			else
			{
				/* show the header variables */
				error = db_show(input, DBIN_HEADER, buffer, header_output_format, END_ARGS);
				if (error)
					break;
				fprintf(stderr,"\n\nHeader variables:\n\n%s\n", buffer);
				done++;
			} /* end of FILE and SEPERATE headers */
		}/* End Processing */
  } /* else, if separate header file */
  
	if (std_args.output_file)
		fclose(pfile);

	if (error == EOF)
		fprintf(stderr,"\n\n");
	else if (error && error != EOF)
	{
		err_push(ROUTINE_NAME, ERR_PROCESS_DATA, "Headers");
		err_disp();
		exit(EXIT_FAILURE);
	}
	else
		fprintf(stderr,"\n\n");
} /* end main() for program gethdr */


/* FUNCTION: print_info()
 *
 * PURPOSE:	display appropraite messege 
 *
 * COMMENTS:
 */
int print_info(int messege)
{
	switch(messege){

	  case GREET:
			fprintf(stderr, "%s%s",
#ifdef ALPHA
"\nWelcome to gethdr alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n",
#elif defined(BETA)
"\nWelcome to gethdr beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n",
#else
"\nWelcome to gethdr release "FF_LIB_VER" -- an NGDC FreeForm application\n\n",
#endif
"Default extensions: .bin = binary, .dat = ASCII, .dab = dBASE\n\
\t.fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE variable description file\n\n\
gethdr data_file [-f format_file] [-if input_format_file]\n\
                 [-of output_format_file] [-ft \"format title\"]\n\
                 [-ift \"input format title\"] [-oft \"output format title\"]\n\
                 [-o output_file] default = output to screen\n\n\
See the FreeForm User's Guide for detailed information.\n"
			       );
			break;
	  case FMT:
			fprintf(stderr,"ERROR: Extension .fmt is needed for a format_list_file.\n\n"); 
			break;
	  default:
			break;
	}/* end switch */
	return 0;

}/* end print_info() */


