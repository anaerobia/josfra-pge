/* FILENAME:  newform.c
 *
 * CONTAINS:  newform()
 */

#include <limits.h>

#ifdef XVT
#include <freeform.h>
#else
#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA
#endif
#include <os_utils.h>
#include <databin.h>
#include <dataview.h>

#ifdef TIMER
#include <time.h>
#endif 

#if defined(XVT) && !defined(TEST_SUITE)
int newform(DATA_BIN_PTR GVdbin, FFF_STD_ARGS_PTR std_args);
#else
int newform(FFF_STD_ARGS_PTR std_args);
#endif

int newform_main(FFF_STD_ARGS_PTR std_args, DATA_BIN_PTR input);

#define SCRATCH_SIZE 10240

static void ff_make_contiguous_format(FORMAT_PTR format);
static int ff_trim_format(FORMAT_PTR format, char *buffer);
static int wfprintf(FILE *stream, const char *format, ...);
static void print_info(int message);

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "Newform"

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
	
#if defined(XVT) && !defined(TEST_SUITE)
int newform(DATA_BIN_PTR GVdbin, FFF_STD_ARGS_PTR std_args)
#else
int newform(FFF_STD_ARGS_PTR std_args)
#endif
{
	DATA_BIN_PTR dbin = NULL;
	int error = 0;
	
#if defined(XVT) && !defined(TEST_SUITE)

	dbin = db_make(std_args->input_file);
	if (!dbin)
		return(ERR_MEM_LACK);

#ifdef SDB_MENU
/*SDB MENU*/	dbin->strdb = GVdbin->strdb; /* "Graft" menu information */
#else
	dbin->mindex = GVdbin->mindex; /* "Graft" menu information */
	dbin->gvmopen = GVdbin->mindex;
	dbin->gvmselection = GVdbin->mindex;
#endif
	
#endif

	check_for_unused_flags(std_args);
	if (make_standard_dbin(std_args, &dbin, NULL))
	{

#if defined(XVT) && !defined(TEST_SUITE)

#ifdef SDB_MENU
/*SDB MENU*/	dbin->strdb = NULL;
#else
	dbin->mindex = NULL; /* "Graft" menu information */
	dbin->gvmselection = NULL;
#endif
		std_args->local_buffer = NULL;
		db_free(dbin);
	
#endif

		return(ERR_MAKE_DBIN);
	}
	
	if (err_state())
#if !defined(DEBUG_MSG)
		err_clear();
#else
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#endif

	error = newform_main(std_args, dbin);
	
#if defined(XVT) && !defined(TEST_SUITE)
#ifdef SDB_MENU
/*SDB MENU*/	dbin->strdb = NULL;
#else
	dbin->mindex = NULL; /* "Graft" menu information */
	dbin->gvmselection = NULL;
#endif
#endif

	std_args->local_buffer = NULL;
	db_free(dbin);
	
	return(error);
}

#if !defined(XVT) && !defined(TEST_SUITE)

/* messages for print_info */
#define INFO             1
#define GREET            2
#define FMT              3

void main(int argc, char *argv[])
{
	int error = 0;
	FFF_STD_ARGS std_args;

	print_info(GREET);
	
	/* Check number of command args */
	if (argc < 2)
	{
		print_info(INFO);
		exit(EXIT_FAILURE);
	}
    
	if ((error = parse_command_line(argc, argv, &std_args)) != 0)
	{
		err_disp();
		exit(EXIT_FAILURE);
	}

	if (err_state())
#if !defined(DEBUG_MSG)
		err_clear();
#else
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#endif

	error = newform(&std_args);
	if (error)
	{
		err_push(ROUTINE_NAME, error, NULL);
		err_disp();
		exit(EXIT_FAILURE);
	}
}

/* FUNCTION: print_info()
 *
 * PURPOSE:     display appropriate message 
 *
 * COMMENTS:
 */
static void print_info(int message)

{
	char *greeting = 
{
#ifdef ALPHA
"\nWelcome to newform alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n"
#elif defined(BETA)
"\nWelcome to newform beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n"
#else
"\nWelcome to newform release "FF_LIB_VER" -- an NGDC FreeForm application\n\n"
#endif
};

	char *command_line_usage = {
"Several newform command line elements have default extensions:\n\
data_file: .bin = binary  .dat = ASCII  .dab = dBASE\n\
input/output_format_file: .fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE variable description file\n\n\
newform data_file [-f format_file] [-if input_format_file]\n\
                  [-of output_format_file] [-ft \"format title\"]\n\
                  [-ift \"input format title\"] [-oft \"output format title\"]\n\
                  [-c count] No. records to process at head(+)/tail(-) of file\n\
                  [-q query] Output records matching criteria in query file\n\
                  [-v var] Process variables listed in var file, default = all\n\
                  [-o output_file] default = output to screen\n\n\
See the FreeForm User's Guide for detailed information.\n"
};

	switch(message)
	{
	  case FMT:
			fprintf(stderr,"ERROR: Extension .fmt is needed for a format description file.\n\n"); 
		break;
		
		case GREET:
			fprintf(stderr, "%s", greeting);
		break;
		
		case INFO:
			fprintf(stderr, "%s", command_line_usage);
		break;
	}/* end switch */
}/* end print_info() */

#endif /* !XVT && !TEST_SUITE */

/*****************************************************************************
 * NAME:  newform()
 *
 * PURPOSE:  To change the format of a data set.
 *
 * USAGE:  error = newform(FFF_STD_ARGS_PTR pstd_args);
 *
 * RETURNS:  Zero on success, otherwise an error code defined in err.h or errno.h
 *
 * DESCRIPTION:  newform takes an allocated and filled std_args structure
 * containing at a minimum the data file name to process.  Other fields, if
 * left initialized (zero for integer variables and NULL for char pointers)
 * will result in the following:  Default searches for format files,
 * local_buffer (the data bin scratch buffer) allocated with a size of
 * DEFAULT_BUFFER_SIZE, and no variable thinning on output, no queries, and
 * no initial or trailing records to skip.
 *
 * In any event, local_buffer should never be allocated by the calling routine.
 * Also, while the user has ability to change the local_buffer size, this
 * should probably never actually be done, as newform() changes local_buffer_size
 * to LOCAL_BUFFER_SIZE (32K) but only if local_buffer_size has been set to
 * DEFAULT_BUFFER_SIZE (8K).
 *
 * AUTHOR:  Ted Habermann, NGDC, (303)497-6472, haber@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:
 *
 * COMMENTS:  If GeoVu is calling NEWFORM (as determined by XVT being defined
 * but TEST_SUITE is not) then the menu information associated with the current
 * DATA SOURCE (as contained in the data bin string data base) gets grafted
 * into the "input" data bin.  I'm assuming that the only way a newform()-
 * generated data bin can get the menu information is by such a graft.  So,
 * before I call db_free(input), I always set input->strdb to NULL, so that
 * db_free() cannot possibly damage the menu information associated with the
 * GeoVu-generated data bin.
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

int newform_main(FFF_STD_ARGS_PTR std_args, DATA_BIN_PTR input)
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

	FORMAT_PTR      in_format       = NULL;         /* Local FORMAT pointers */
	FORMAT_PTR      out_format      = NULL;
	DATA_VIEW_PTR view      = NULL;
	FF_DATA_BUFFER old_cache_end = NULL;

	FILE            *testfile = NULL;
 
	char            *point          = NULL;
	char            *output         = NULL;
	char            *data_file = NULL;
	char            *input_format_name = NULL;
	char            *output_format_name = NULL;
	char            *var_file_name = NULL;
	char            *position = NULL;
	char            *query_to_use = NULL;

	int num_formats                 = 1; /* Default program reads formats from seperate files */
											/* Otherwise formats read from one file */   
	int error                               = 0;
	int cache_num = 0;
	int output_file;
	int perc_left           = 0;

	size_t out_count      = 0;

	long local_buffer_size  = 0L;
	long file_size          = 0L;
	long input_bytes        = 0L;
	long output_bytes       = 0L;
	long old_num_rec = -1;
	FILE *pfile = NULL;

#ifdef TIMER	
	long elapsed_time;
	time_t start_time, finish_time;
	(void)time(&start_time);
#endif

#if defined(DEBUG_MSG)
  wfprintf(stderr,"Local Buffer/Output Cache Size: %ld\n", std_args->local_buffer_size);
#endif
	wfprintf(stderr, "Input file: %s\n", input->file_name);
	if (std_args->output_file)
		wfprintf(stderr, "Output file: %s\n", std_args->output_file);
	else if (!isatty(fileno(stdout)))
		wfprintf(stderr, "Output redirected from screen\n");
	else
		wfprintf(stderr, "Output to screen\n");
  if (std_args->query_file)
  	wfprintf(stderr, "Using query file: %s\n", std_args->query_file);

	wfprintf(stderr, "\nThe input file size is %lu bytes.\n", os_filelength(input->file_name));
	
	if (std_args->output_file)
	{
		pfile = fopen(std_args->output_file, "wb");
		if (pfile == NULL)
			return(err_push(ROUTINE_NAME, ERR_OPEN_FILE, std_args->output_file));
		output_file = fileno(pfile);
	}
	else
	{
#ifdef XVT
		return(ERR_FILE_DEFINED);
#else
		output_file = fileno(stdout);
#endif
	}

	if(!db_find_format(input->format_list, FFF_GROUP, FFF_HD | FFF_FILE | FFF_OUTPUT, END_ARGS))
	{ /* no output file header -- safe to predict output file size */
		output_bytes = (long)FORMAT_LENGTH(input->output_format) * 
		               (input->bytes_available /
		               (long)FORMAT_LENGTH(input->input_format));
		if (std_args->query_file)
			wfprintf(stderr, "The maximum output file size will be %lu bytes.\n", output_bytes);
		else
			wfprintf(stderr, "The output file size will be %lu bytes.\n", output_bytes);
	}
    
	if (err_state())
#if !defined(DEBUG_MSG)
		err_clear();
#else
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#endif

#ifdef CCMSC
	if ( (setmode(output_file, O_BINARY)) == -1)
		return(err_push(ROUTINE_NAME, errno, "for output file"));
#endif

	/* Check for variable file */
	if(std_args->var_file){
		ff_file_to_buffer(std_args->var_file, input->buffer);
		ff_trim_format(input->output_format, input->buffer);
		ff_make_contiguous_format(input->output_format);                
	}

	/* Display some information about the data, ignore errors */
	(void)db_show(input, SHOW_FORMAT_LIST, FFF_INFO, END_ARGS);
	wfprintf(stderr, "%s", input->buffer);

	/* Check for query file */
	if (std_args->query_file)
	{
		/* check to make sure that the query file exists */
		if((testfile = fopen(std_args->query_file, "r")) == NULL)
			return(err_push(ROUTINE_NAME, ERR_OPEN_FILE, std_args->query_file));

		fclose(testfile);

		if (!ff_file_to_buffer(std_args->query_file, input->buffer))
			return(ERR_GEN_QUERY);

		/* The query is the first line of the file.  Any subsequent lines are ignored. */
		position = input->buffer - 1;
		while(position++){
			if(position[0] < ' '){
				if(position[0] == '\0') break;

				/* End-of-line character */
				position[0] = '\0';
				break;
			}
		}

		if((query_to_use = (char *)memMalloc((size_t)(strlen(input->buffer) + 10), "query_to_use")) == NULL)
			return(ERR_MEM_LACK);

		memStrcpy(query_to_use, input->buffer, "query_to_use, input->buffer");

		if((view = dv_make("query_view", input->buffer)) == NULL)
			return(ERR_GENERAL);

		if(dv_set(view,
				VIEW_DATA_BIN, input,
				VIEW_INCREMENT, FORMAT_LENGTH(input->input_format),
				VIEW_FIRST_POINTER, ((DATA_BUFFER)input->cache),
				VIEW_TYPE, (long)V_TYPE_RECORD,
				VIEW_QUERY_RESTRICTION, query_to_use, input->input_format, (int)END_ARGS))
			return(ERR_GENERAL);
	}

	/*
	** process the data
	*/
	
	wfprintf(stderr, "\n");
  file_size = input->bytes_available;

	if (err_state())
#if !defined(DEBUG_MSG)
		err_clear();
#else
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#endif

	do { /*  while (input->bytes_available >= (long)FORMAT_LENGTH(input->input_format)) */
  	/* use PROCESS_FORMAT_LIST to fill cache and fill headers */
		if ((error = db_events(input, PROCESS_FORMAT_LIST, FFF_ALL_TYPES, END_ARGS)) != 0)
			break;

		/* Deal with headers */
		if (!cache_num &&                                                                  /* At the beginning of the file */
			  (in_format = db_find_format(input->format_list, FFF_GROUP, 
			                FFF_HD | FFF_FILE, END_ARGS)) != NULL)
		{
			/* Check for header output format: */
			if (IS_INPUT(in_format))
			{ /* header input format found first */
				out_format = db_find_format(input->format_list, FFF_GROUP, 
				              FFF_HD | FFF_FILE | FFF_OUTPUT, END_ARGS);
			}                       
			else if(IS_OUTPUT(in_format))
			{ /* header output format found first */
				out_format = in_format;
				in_format = db_find_format(input->format_list, FFF_GROUP,
				             FFF_HD | FFF_FILE | FFF_INPUT, END_ARGS);
			}
		
			/* The NEWFORM header conventions are:
				Format Description Includes:
				header only: Copy header in all but binary to ASCII or dbase cases
				input_header only: No translation or copy
				input and output_record_header: Translation
				output_header only: fill with blanks
			*/
#ifdef DEBUG
			if (out_format)
				assert(!IS_SEPARATE(out_format));
#endif
			if ((IS_INPUT(in_format) && !out_format) || IS_SEPARATE(out_format))
				; /* do nothing */
			else
			{
				if (in_format && !out_format) /* input but no output header format */
				{ /* Copy header - unless header is binary and output is not */
					if (!IS_BINARY(in_format) || IS_BINARY(input->output_format))
					{
						write(output_file, input->header, input->header_format->max_length);
						if (IS_ASCII(input->output_format))
							write(output_file, eol_string, EOL_SPACE);
					}               
				}
				else if (in_format && out_format)
				{ /* Translate header */
					input->header_format = in_format;
					error = db_events(input,
						DBIN_DATA_TO_NATIVE, input->header,
							((char *)input->header) + input->header_format->max_length - 1, in_format,
						DBIN_CONVERT_HEADER, input->buffer, out_format,
						END_ARGS);
					if (error)
					{
						err_push(ROUTINE_NAME, ERR_PROCESS_DATA, "Translating Header");
						break;
					}

					write(output_file, input->buffer, FORMAT_LENGTH(out_format));
				}
				else if (!in_format && out_format)
				{ /* write out blanks */
					out_count = FORMAT_LENGTH(out_format);
					if (IS_BINARY(out_format))
						memMemset(input->buffer, '\0', out_count,"input->buffer,'\\0',out_count");
					else
						memMemset(input->buffer, ' ', out_count,"input->buffer,' ',out_count");
						
					write(output_file, input->buffer, out_count);
					if (!IS_BINARY(out_format))
						write(output_file, eol_string, EOL_SPACE);
				}
	    }

			if (!out_count)
				input->state.new_record = 0;

		}/* if cache_num is 0 */
		cache_num++;

		/* Output the record header - if defined */ 
		if (input->state.new_record  &&
			  (in_format = db_find_format(input->format_list, FFF_GROUP,
			                FFF_HD | FFF_REC, END_ARGS)) != NULL &&
			  !(IS_SEPARATE(in_format))) /* Headers are in data file */
		{

			/* Check for header output format: */
			if (IS_INPUT(in_format))
			{ /* record input format found first */
				out_format = db_find_format(input->format_list, FFF_GROUP,
				              FFF_HD | FFF_REC | FFF_OUTPUT, END_ARGS);
			}
			else if (IS_OUTPUT(in_format))
			{ /* record output format found first */
				out_format = in_format;
				in_format = db_find_format(input->format_list, FFF_GROUP,
				             FFF_HD | FFF_REC | FFF_INPUT, END_ARGS);
			}
		
			/* The NEWFORM header conventions are:
				Format Description Includes:
				record_header only: Copy header in all but binary to ASCII or dbase cases
				input_record_header only: No translation or copy
				input and output_record_header: Translation
				output_record_header only: write out blanks
			*/
			
			if (IS_INPUT(in_format) && !out_format)
				; /* do nothing */
			else
			{
				if (in_format && !out_format)
				{ /* Copy header - unless header is binary and output is not */
					if (!IS_BINARY(in_format) || IS_BINARY(input->output_format))
					{
						write(output_file, input->header, input->header_format->max_length);
						if (IS_ASCII(input->output_format))
							write(output_file, eol_string, EOL_SPACE);
					}               
				}
				else if (in_format && out_format)
				{ /* Translate header */
					input->header_format = in_format;
					error = db_events(input,
						DBIN_DATA_TO_NATIVE, input->header,
							((char *)input->header) + input->header_format->max_length - 1, in_format,
						DBIN_CONVERT_HEADER, input->buffer, out_format,
						END_ARGS);
					if (error)
					{
						err_push(ROUTINE_NAME, ERR_PROCESS_DATA, "Translating Header");
						break;
					}

					write(output_file, input->buffer, FORMAT_LENGTH(out_format));
				}
				else if (!in_format && out_format)
				{ /* write out blanks */
					out_count = FORMAT_LENGTH(out_format);
					if (IS_BINARY(out_format))
						memMemset(input->buffer, '\0', out_count,"input->buffer,'\\0',out_count");
					else
						memMemset(input->buffer, ' ', out_count,"input->buffer,' ',out_count");
						
					write(output_file, input->buffer, out_count);
					if (!IS_BINARY(out_format))
						write(output_file, eol_string, EOL_SPACE);
				}
			}
			input->state.new_record = 0;

		} /* End of Header Handling */
		
		/* Make sure buffer is large enough for the cache */
		(void)db_show(input, DBIN_BYTE_COUNTS,
			DBIN_INPUT_CACHE,  &input_bytes,
			DBIN_OUTPUT_CACHE, &output_bytes,
			(int)END_ARGS, (int)END_ARGS);
		
		if ((unsigned long)output_bytes > (unsigned long)UINT_MAX){
			error = 1;
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation size too big");
			break;
		}
		if (output_bytes > std_args->local_buffer_size)
		{
			if ((std_args->local_buffer = (char *)memRealloc((char *)input->buffer, (size_t)output_bytes, "input->buffer")) == NULL)
			{
				std_args->local_buffer = input->buffer;
				error = 1;
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "reallocation of buffer");
				break;
			}
			else
				input->buffer = std_args->local_buffer;
		}
		
		error = db_events(input, DBIN_DATA_TO_NATIVE, NULL, NULL, NULL, END_ARGS);
		if (error)
			break;

		/* Clip out unwanted records if a query exists */
		if (std_args->query_file)
		{ /* Save the cache end and the number of records it could hold... */
		
			old_cache_end = input->cache_end;
			old_num_rec = input->records_in_cache;
			
			error = dv_events(view, CLIP_VIEW_ON_QUERY, input->input_format, END_ARGS);
			if (error)
			{
				err_push(ROUTINE_NAME, ERR_GENERAL, "Clipping the cache by query");
				break;
			}
		}

		/* Convert the cache into the buffer */
		output_bytes = 0L;
		error = db_events(input,
			DBIN_CONVERT_CACHE, input->buffer, (void*)NULL, &output_bytes,
			END_ARGS);

		if (error)
			break;

		out_count = write(output_file, input->buffer, (size_t)output_bytes);
		if (out_count != (size_t)output_bytes)
		{
			err_push(ROUTINE_NAME, ERR_WRITE_FILE,"output file");
			break;
		}
		
		if (std_args->query_file)
		{ /* Restore the old cache end and number of records... */
			/* This is necessary because the CLIP_VIEW_ON_QUERY view event scales down
			 * the size of the cache to accomodate only those records that match the
			 * query...  So if we don't reset the cache size, it slowly dwindles to 
			 * 0, and then newform seems to stagnate.  */
			input->cache_end = old_cache_end;
			input->records_in_cache = old_num_rec;
		}

		/* Calculate percentage left to process and display */
		perc_left = (int)((1 - ((float)input->bytes_available / file_size)) * 100);
		wfprintf(stderr,"\r%3d%% processed", perc_left);

#ifdef TIMER
		(void)time(&finish_time);
		elapsed_time = (long)difftime(finish_time, start_time);
		wfprintf(stderr, "     Elapsed time - %02d:%02d:%02d",
		        (int)(elapsed_time / (3600)),
		        (int)((elapsed_time / 60) % 60),
		        (int)(elapsed_time % 60)
		);
#endif

	} while (input->bytes_available >= (long)FORMAT_LENGTH(input->input_format));
	/* End Processing */
	
	if (std_args->output_file)
		fclose(pfile);
	if (error && err_state())
		err_disp();
 
	if (input->bytes_available != 0)
	{
		wfprintf(stderr,"\n\n%ld BYTES OF DATA NOT PROCESSED.\n",input->bytes_available);
		if (std_args->query_file)
			dv_free(view);
		return(err_push(ROUTINE_NAME, ERR_WRITE_DATA, "Data processing incomplete"));
	}
	else
	{
		wfprintf(stderr,"\n");
		if (std_args->query_file)
			dv_free(view);
		return(0);
	}

} /* end main() for program newform */


/*
 * NAME:  ff_make_contiguous_format
 *
 * PURPOSE:  Realign start and end positions of variables to be contiguous.
 *
 * USAGE:  ff_make_contiguous_format(format);
 *
 * RETURNS:
 *
 * DESCRIPTION:  Steps through each variable in format, reassigning the start
 * position to be one more than the end position of the previous variable, and
 * reassigning the end position to preserve the variable's field width.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6118, mao@mail.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * COMMENTS:
 *
 * KEYWORDS:
 */
#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_make_contiguous_format"

void ff_make_contiguous_format(FORMAT_PTR format)
{
  VARIABLE_LIST_PTR vlist = FFV_FIRST_VARIABLE(format);
  unsigned short new_start_pos = 0, new_end_pos = 0, old_end_pos = 0;
  unsigned short i = 0;
  unsigned short buffer_char = 0;
  
   /*  if(IS_ASCII(format))buffer_char = 1;*/
  
  	/* check first variable */
	if (FFV_VARIABLE(vlist)->start_pos != 1) {
		/* realign first variable */
		new_start_pos = 1;
		new_end_pos = new_start_pos + (FFV_VARIABLE(vlist)->end_pos - 
		                               FFV_VARIABLE(vlist)->start_pos);
		FFV_VARIABLE(vlist)->start_pos = new_start_pos;
		FFV_VARIABLE(vlist)->end_pos = new_end_pos;
	}
	old_end_pos = FFV_VARIABLE(vlist)->end_pos ;
	
	for (i = 1; i < format->num_in_list; i++) { 
		vlist = dll_next(vlist);
		new_start_pos = old_end_pos + 1;
		new_end_pos = new_start_pos + (FFV_VARIABLE(vlist)->end_pos - 
		                               FFV_VARIABLE(vlist)->start_pos);
		FFV_VARIABLE(vlist)->start_pos = new_start_pos;
		FFV_VARIABLE(vlist)->end_pos = new_end_pos;		                              
		old_end_pos = new_end_pos;		                               
  }
	format->max_length = FFV_VARIABLE(vlist)->end_pos;
	return;
}

/*
 * NAME:  ff_trim_format
 *
 * PURPOSE:  Remove all but given variables from a format  
 *
 * USAGE:  ff_trim_format(format, buffer)
 *
 * RETURNS:  0 if all is OK
 *
 * DESCRIPTION:  Deletes all variables from format whose names are not in
 * buffer.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC (303) 497-6124, mao@mail.ngdc.noaa.gov
 *          Ted Habermann, NGDC (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * COMMENTS:  
 *
 * KEYWORDS:
 */
#include <freeform.h>
#include <string.h>
 
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_trim_format"

int ff_trim_format(FORMAT_PTR format, char *buffer)
{
	VARIABLE_LIST_PTR v_list = NULL;
	int weed_var_len = 0;
	char *weed_var = NULL;
	
	/* Loop format checking each variable name */
	v_list = FFV_FIRST_VARIABLE(format);
	while(FFV_VARIABLE(v_list))
	{
		v_list = dll_next(v_list);
		if (!((weed_var = memStrstr(buffer, FFV_VARIABLE(dll_previous(v_list))->name,NO_TAG)) != NULL &&
		    (isspace(weed_var[(weed_var_len = strlen(weed_var))]) || 
		     weed_var[weed_var_len] == STR_END)))
		{
    	/* Delete this node */
			dll_delete(dll_previous(v_list), (void (*)(void *))ff_free_variable);
			format->num_in_list--;
		}
	}
	return(0);
}

static int wfprintf(FILE *stream, const char *format, ...)
/*****************************************************************************
 * NAME: wfprintf()
 *
 * PURPOSE:  XVT-sensitive wrapper around fprintf()
 *
 * USAGE:  same as fprintf()
 *
 * RETURNS:  same as fprintf()
 *
 * DESCRIPTION:  If XVT is NOT defined, calls vfprintf().
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
#ifdef XVT

/* Insert appropriate XVT call(s) here */

#else
  va_list va_args;
  
  va_start(va_args, format);
	
	return(vfprintf(stream, format, va_args));
#endif
}

