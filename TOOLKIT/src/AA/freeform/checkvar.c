/*
 * NAME:	checkvar
 *		
 * PURPOSE:	To create histograms of data values in a file
 *
 * USAGE:	
 *
 * RETURNS:	
 *
 * DESCRIPTION:	This program produces output which can be used to make
 * histograms of variables in binary or ascii data files 
 * through the FREEFORM format specification system.
 * It can have up to four command line arguments:
 *   1) The file name.
 *   2) The precision.
 *   3) The maximum number of bins. This	controls the
 *      resolution of the histogram.
 *   4) The format file name.
 * The format file name is assumed to be the same as the
 * data file name  with an extension of .afm or .bfm and if
 * this is the case, this argument may be omitted.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	 T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 * Modified (or rewritten shall we say) by Merrill Bennett, July, 1991
 * Major revision to list of views: Mark Van Gorp, mvg@ngdc.noaa.gov, 497-6221
 * ASCII data handling, merging with maxmin.c: kbf@ngdc.noaa.gov, February, 1994
 * COMMENTS:	
 *
 * KEYWORDS:	
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	12/22/95		-rf01
 *		(HEADER *) cast for CW compiler
*/


#undef TREESIZ
#include <limits.h>    /*  for LONG_MIN & LONG_MAX  */
#include <float.h>     /*  for DBL_MIN & DBL_MAX  */
#include <math.h>

#ifdef TIMER
#include <time.h>
#endif 

#ifdef XVT
#include <xvt.h>
#define WANT_NCSA_TYPES
#include <freeform.h>
#else
#define DEFINE_DATA
#define WANT_NCSA_TYPES
#include <freeform.h>
#undef DEFINE_DATA
#endif
#include <os_utils.h>
#include <databin.h>
#include <dataview.h>

#include <maxmin.h>

#undef HEAPCHK
#ifdef HEAPCHK
#include <heap.h>
#endif

typedef struct
{
	char bin[MAX_NAME_LENGTH];
	long count;
} CHAR_ARRAY_TYPE;

typedef struct
{
	double      missing_data;
	double      temp_dvar;
	MAX_MIN_PTR maxmininfo;
	char        missing_data_exists;
#ifdef CCMSC
	char        md_pc_padding; /* explicitly force 2-byte alignment */
#endif
} MISSING_DATA_STRUCT, *MISSING_DATA_PTR;

#include <avltree.h>

/* Function prototypes */
#if defined(XVT) && !defined(TEST_SUITE)
int checkvar(DATA_BIN_PTR GVdbin, FFF_STD_ARGS_PTR std_args);
#else
int checkvar(FFF_STD_ARGS_PTR std_args);
#endif

int checkvar_main(FFF_STD_ARGS_PTR std_args, DATA_BIN_PTR input_data, FILE *output_file);

static void output_number_histogram(HEADER *root, int user_precision);
static void tpass1(HEADER *root, int *col1_len, int *col1_prec, int *col2_len);
static void tpass2(HEADER *root, int col1_len, int col1_prec, int col2_len);
static void tcwrite(HEADER *);
static int collapse(HEADER *, long);
void print_help(void);
static int mkstdbin_cb(int routine_name);
static int wfprintf(FILE *stream, const char *format, ...);
static void print_info(int message);

int icmp(BIN_ARRAY_TYPE *,BIN_ARRAY_TYPE *);
int iccmp(CHAR_ARRAY_TYPE *,CHAR_ARRAY_TYPE *);

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "Checkvar"

#if defined(XVT) && !defined(TEST_SUITE)
int checkvar(DATA_BIN_PTR GVdbin, FFF_STD_ARGS_PTR std_args)
#else
int checkvar(FFF_STD_ARGS_PTR std_args)
#endif
{
	int           error       =    0;
	DATA_BIN_PTR  dbin        = NULL;
	FILE         *pfile       = NULL;

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

	if (make_standard_dbin(std_args, &dbin, mkstdbin_cb))
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

	if (error || err_state())
#if !defined(DEBUG_MSG)
		err_clear();
#else
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#endif
	
	if (std_args->output_file)
	{
		pfile = fopen(std_args->output_file, "w");
		if (pfile == NULL)
			return(err_push(ROUTINE_NAME, ERR_OPEN_FILE, std_args->output_file));
	}
	else
	{
#ifdef XVT
/* implement error return and expect output file, when ready
		return(ERR_FILE_DEFINED);
*/
		xvt_dm_post_note("Remember to change user interface to allow naming an output file");
		pfile = tmpfile();
		if (!pfile)
		{
			xvt_dm_post_error("Could not create throw-away file -- aborting!");
			return(ERR_FILE_DEFINED);
		}
#else
		pfile = stdout;
#endif
	}

	error = checkvar_main(std_args, dbin, pfile);
	
	if (std_args->output_file)
		fclose(pfile);

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

void main(int argc, char *argv[])

/* messages for print_info */
#define INFO             1
#define GREET            2

{
	int error = 0;
	FFF_STD_ARGS std_args;

#define FF_CHECK_SIZES
#include <ff_types.h>

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

	error = checkvar(&std_args);
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
"\nWelcome to checkvar alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n"
#elif defined(BETA)
"\nWelcome to checkvar beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n"
#else
"\nWelcome to checkvar release "FF_LIB_VER" -- an NGDC FreeForm application\n\n"
#endif
};

char *command_line_usage =
{
"Default extensions: .bin = binary, .dat = ASCII, .dab = dBASE\n\
\t.fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE variable description file\n\n\
checkvar data_file [-f format_file] [-if input_format_file]\n\
                   [-of output_format_file] [-ft \"format title\"]\n\
                   [-ift \"input format title\"] [-oft \"output format title\"]\n\
                   [-c count] No. records to process at head(+)/tail(-) of file\n\
                   [-q query] Output records matching criteria in query file\n\
                   [-v var] Process variables listed in var file, default = all\n\
                   [-p precision] No. of decimal places in variable summaries\n\
                                  min = 0, max = 5\n\
                   [-m maxbins] approximate maximum number of bins desired\n\
                                default = 100, min = 6, max = 10,000\n\
                   [-md] Data flag value to ignore in creating histogram data\n\
                   [-mm] Output min/max values; no variable summaries created\n\
                   [-o log] File to contain log of processing information\n\n\
See the FreeForm User's Guide for detailed information.\n"
};

	switch(message)
	{
		case GREET:
			fprintf(stderr, "%s", greeting);
		break;
		
		case INFO:
			fprintf(stderr, "%s", command_line_usage);
		break;
	}/* end switch */
}/* end print_info() */

#endif /* !XVT && !TEST_SUITE */

static char error_buffer[MAX_ERRSTR_BUFFER];
static unsigned char ignore_missing_data = 0;
static int	firstnode = 1;
static long bins = 0L;
static long thisbin = 0L;
static long cnt = 0L;
static double factor;
static HEADER *newroot = NULL;
static FILE *histo_file = NULL;

#define FLAG_PRECISION SHRT_MIN
#define MAXIMUM_PRECISION 5
#define DEFAULT_MAXBINS 100
#define MAXIMUM_MAXBINS 10000
#define SCRATCH_SIZE 16384
#define NUM_RECORDS_UPDATE 1000

int checkvar_main(FFF_STD_ARGS_PTR std_args, DATA_BIN_PTR input_data, FILE *output_file)
{ 
	int precision = 0;
	int maxbins = 0;
	BIN_ARRAY_TYPE *bat_ptr = NULL;

	BOOLEAN record_header_exist = 0;
	CHAR_ARRAY_TYPE *cat_ptr = NULL;
	char str[MAX_NAME_LENGTH];
	char file_name[MAX_NAME_LENGTH + 16];
	char *tmp_buffer = NULL;
	char *position = NULL;
	char *scratch_buffer = NULL;

	DATA_VIEW_PTR view = NULL;

	DLL_NODE_PTR view_list = NULL;
	DLL_NODE_PTR collider_list = NULL;
	DLL_NODE_PTR ascii_view_list = NULL;
	MISSING_DATA_PTR miss_data = NULL;

	double double_var = 0;
	double adjustment = 0;
	double minimum = 0, maximum = 0;
	
	FORMAT_PTR input_format = NULL;

	int perc_left = 0;
	int count = 0;
	int occurrence = 0;
	int nevercollapse = 0;
	int index = -1;
	int error = 0;
	int bad_record = 0;
	int missing_data = 0;
	int maxmin_only = 0;    
	int record_length	= 0;
		
	long num_lines = 0;
	
	long num_records = 1;
	long min_rec_occurrence = 0;
	long max_rec_occurrence = 0;
	long sorter = 0;
	long sorterfactor = 1L;
	long binary_buffer_size = 0L;
	long result_bytes = 0L;
	long file_size = 0L;
	
	char *binary_buffer = NULL;
	char *binary_buffer_end = NULL;
	
	int col1_len = 0, col2_len = 0;
	
	short num_char = 0;
	static HEADER *root = NULL;

	VARIABLE_PTR var = NULL;
	VARIABLE_PTR collider = NULL;
	EQUATION_INFO_PTR einfo = NULL;
	 	
	void *p = NULL, *p2 = NULL;
	void *data_ptr = NULL;
	
	DATA_BUFFER record_in_buffer = NULL; /* MAO:d -- formerly DATA_BUFFER* */
	
	FORMAT_PTR binary_format = NULL;
	
#ifdef TIMER
	long elapsed_time = 0;
	time_t start_time = 0, finish_time = 0, net_start_time = 0;
	net_start_time = time(&start_time);
#endif

	/*	checkvar can have optional arguments:
		-mm (maxmin only)
		-md (ignore missing data)
		-p precision
		-m maxbins
	*/

	if (std_args->user.set_cv_precision)
		precision = std_args->cv_precision;
	else
		precision	= FLAG_PRECISION;

	if (std_args->user.set_cv_maxbins)
	{
		maxbins = std_args->cv_maxbins;
		if (maxbins < 1)
			nevercollapse = 1;
	}
	else
		maxbins = DEFAULT_MAXBINS;
	
	if (std_args->user.set_cv_maxmin_only)
		maxmin_only = 1;
	if (std_args->user.set_cv_missing_data)
	{
		double_var = std_args->cv_missing_data;
		ignore_missing_data = 1;
	}
		
	scratch_buffer = std_args->local_buffer; 
#ifdef DEBUG_MSG
	wfprintf(stderr,"Local Buffer Size: %ld\n", std_args->local_buffer_size);
#endif
	file_size = input_data->bytes_available;

	(void)wfprintf(stderr, "Input file : %s",
		input_data->file_name);
	if ( 1 || std_args->output_file  )
		(void)wfprintf(output_file, "Input file : %s",
		 input_data->file_name);

	if (precision != FLAG_PRECISION)
	{
		wfprintf(stderr, "\nRequested precision = %d, Approximate number of sorting bins = %d\n", precision, maxbins);
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\nRequested precision = %d, Approximate number of sorting bins = %d\n", precision, maxbins);
	}
	else
	{
		wfprintf(stderr, "\nNo requested precision, Approximate number of sorting bins = %d\n", maxbins);
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\nNo requested precision, Approximate number of sorting bins = %d\n", maxbins);
	}
	/* Check for query file */
	if (std_args->query_file)
	{
		if (!ff_file_to_buffer(std_args->query_file, scratch_buffer))
			return(ERR_READ_FILE);

		/* The query is the first line of the file.  Any subsequent lines are ignored. */
		position = scratch_buffer - 1;
		while (position++)
		{
			if (position[0] < ' ')
			{
				if (position[0] == '\0')
					break;

				/* End-of-line character */
				position[0] = '\0';
				break;
			}
		}
		
		if ((einfo = ee_make_std_equation(scratch_buffer, input_data->input_format)) == NULL)
			return(ERR_GEN_QUERY);
	} /* end if std_args->query_file */
	
	if (std_args->var_file)
	{
		wfprintf(stderr, "\nUsing variable file: %s", std_args->var_file);
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\nUsing variable file: %s", std_args->var_file);
	}
	
	if (maxmin_only)
	{
		wfprintf(stderr, "\nCalculating minimums and maximums only");
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\nCalculating minimums and maximums only");
	}
	
	if (std_args->query_file)
	{
		wfprintf(stderr, "\nUsing query file: %s", std_args->query_file);
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\nUsing query file: %s", std_args->query_file);
	}
	
	/* Display some information about the data, ignore errors */
	(void)db_show(input_data, SHOW_FORMAT_LIST, FFF_INFO, END_ARGS);
	wfprintf(stderr, "\n%s", input_data->buffer);
	if ( 1 || std_args->output_file  )
		wfprintf(output_file, "\n%s", input_data->buffer);

	if (IS_ASCII(input_data->input_format) || IS_DBASE(input_data->input_format))
	{
		binary_format = ff_afm2bfm(input_data->input_format);
		binary_buffer_size = input_data->records_in_cache * FORMAT_LENGTH(binary_format);

		binary_buffer = (char *)memMalloc((size_t)binary_buffer_size, "binary_buffer");
		if (!(binary_buffer))
			return(ERR_MEM_LACK);

		binary_buffer_end = binary_buffer + binary_buffer_size;
		
		/* If the input format is ASCII or dbase, maxmin determines the precision
		of all variables as well as their minima and maxima. This is done on the
		ASCII cache using a separate list of views from the input format */
 		ascii_view_list = db_format_to_view_list(input_data->input_format, NULL);

		ascii_view_list = dll_first(ascii_view_list);
		while (dll_data(ascii_view_list))
		{
			view = (DATA_VIEW_PTR)dll_data(ascii_view_list);
			var = (VARIABLE_PTR)(view->var_ptr);

			if (dv_set(view,
			 VIEW_DATA_BIN, input_data,
			 VIEW_TYPE, (long)V_TYPE_ARRAY,
			 VIEW_INCREMENT, FORMAT_LENGTH(input_data->input_format),
			 VIEW_FIRST_POINTER, ((DATA_BUFFER)(input_data->cache + var->start_pos - 1)),
			 VIEW_NUM_POINTERS, 1,
			 VIEW_INFO, (void *)NULL,
			 END_ARGS))
				return(ERR_SET_VIEW);

			ascii_view_list = dll_next(ascii_view_list);
	 	} /* while dll_data(ascii_view_list) */
	 }	/* End of ASCII setup -- IS_ASCII(input_data->input_format) ... */
	else
	{
		binary_buffer = input_data->cache;
		binary_format = input_data->input_format;
	}
    
	/* check to see if a variable file was included in the command line;
	 * if so, read it into a buffer and send it to ff_file_to_buffer to
	 * trim all unwanted variables out of the view_list. */
	 
	if (std_args->var_file)
	{ /* Variable file specified on command line */
		if (ff_file_to_buffer(std_args->var_file, scratch_buffer) == 0)
			return(ERR_READ_FILE);

	    /* Create list of views from the input format, removing those nodes
	     * not mentioned in the variable file (now in scratch_buffer) */
		view_list = db_format_to_view_list(binary_format, scratch_buffer);
	}
	else
	{ /* No variable file specified on command line */
	  /* Create a list of views from the input format */
 		view_list = db_format_to_view_list(binary_format, NULL);
 	}

	/*	If stdin is redirected, the precision and maxbins stated on
	the command line will be ignored unless they are left out
	of the list in the redirected file.
	The format of the entries in the redirected file are as follows:
			
	var_name precision maxbins
		
	for each variable to be histogrammed. */

#ifdef HEAPCHK
	/* Print the heap size at the beginning    */
	heapPrt("Before mallocs of data ");
#endif

	if (std_args->var_file ||
	    maxmin_only ||
	    ignore_missing_data ||
	    std_args->query_file)
	{
		wfprintf(stderr, "\n");
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\n");
	}
	
	/* LOOP THE LIST OF VIEWS */
	view_list = dll_first(view_list);
	while (dll_data(view_list))
	{
		view = (DATA_VIEW_PTR)dll_data(view_list);
		var = (VARIABLE_PTR)(view->var_ptr);

#ifdef LONGS_ARE_32
		if ( (!maxmin_only && IS_UINT32(var)) || 
#endif
#ifdef LONGS_ARE_64
		if ( (!maxmin_only && IS_UINT64(var)) || 
#endif
		    IS_CONSTANT(var) || IS_HEADER(var)|| IS_INITIAL(var))
		{
			wfprintf(stderr, "\nVariable %s:\nUnsigned Longs, headers, initial, And Constants Are Not Supported\n",var->name);
			if ( 1 || std_args->output_file  )
				wfprintf(output_file, "\nVariable %s:\nUnsigned Longs, headers, initial, And Constants Are Not Supported\n",var->name);
			
			view_list = dll_previous(view_list);
			dll_delete(dll_next(view_list), (void (*)(void *))dv_free);
			view_list = dll_next(view_list);
			
			continue;
		}
		
		/* CREATE AN OUTPUT FILE FOR EACH VARIABLE */
		(void)strcpy(file_name, var->name);
		
		if (!maxmin_only)
		{
		
#ifndef SUNCC
			/* Check for duplicate variable file names */
			/* Repeat occurrences are checked against the format list -- no matter if
		   	  a variable file exists or not. The decision to go this way was made to
		   	  guarantee no duplicate file names (although the occurrence counter will
		   	  not always match up with repeated variables in a variable file).
              
		   	  ie. 
			  If you did checkvar on the same file with two different variable files,
			  you may end up with duplicate files -- if the occurrence counter was
			  based solely on the variable file
			*/
			num_char = (short)min(strlen(var->name), 8);
			occurrence = 0;
		
			collider_list = dll_previous(view_list);
			while (dll_data(collider_list))
			{
				collider = (VARIABLE_PTR)(((DATA_VIEW_PTR)dll_data(collider_list))->var_ptr);

				if ( num_char == (short)min(strlen(collider->name),8) )
				{
					if (strncmp(var->name, collider->name, num_char) == 0)
						++occurrence;
				}
				collider_list = dll_previous(collider_list);
			}

			/* Surely there won't be more than 99 variables with the 
		      first 8 characters the same  :- */
			if (occurrence > 9)
				sprintf((file_name + num_char - 2), "%d", occurrence);
			else if (occurrence)
				sprintf((file_name + num_char - 1), "%d", occurrence);

			/* Truncate the file name at num_char characters */
			*(file_name + num_char) = '\0';
		
#endif
			if (IS_TEXT(var))
				(void)strcat(file_name, ".cst");
			else
				(void)strcat(file_name, ".lst");

		} /* end if (!maxmin_only) */

		/* Set up the view attributes */
		if (dv_set(view,
		 TITLE, file_name,
		 VIEW_DATA_BIN, input_data,
		 VIEW_TYPE, (long)V_TYPE_ARRAY,
		 VIEW_INCREMENT, FORMAT_LENGTH(binary_format),
		 VIEW_FIRST_POINTER, ((DATA_BUFFER)(binary_buffer + var->start_pos - 1)),
		 VIEW_NUM_POINTERS,	1,
		 END_ARGS))
			return(ERR_SET_VIEW);
				
		/* allocate the MISSING_DATA_STRUCT */
		if ((view->info = memMalloc(sizeof(MISSING_DATA_STRUCT), "view->info")) == NULL)
			return(ERR_MEM_LACK);

		miss_data = (MISSING_DATA_PTR)view->info;
		
		miss_data->missing_data_exists = 0;
		miss_data->missing_data = 0;
		
		/* create a MAXMIN for this variable */
		miss_data->maxmininfo = mm_make(var, 0);
		if (!miss_data->maxmininfo)
			return(ERR_MEM_LACK);
		
		/* Set the missing data flag for this variable */
		if (ignore_missing_data)
		{
			miss_data->missing_data_exists = 1;
			miss_data->missing_data = double_var;
			
			if (IS_REAL(var) || var->precision)
      {
				wfprintf(stderr, "Ignoring missing data flag value %0.*f for %s\n",
				        var->precision,
				        miss_data->missing_data,
				        var->name);
				if ( 1 || std_args->output_file  )
					wfprintf(output_file, "Ignoring missing data flag value %0.*f for %s\n",
			            var->precision,
					        miss_data->missing_data,
					        var->name);
        if (IS_REAL(var))
        { /* scale float up to an integer, according to precision, for floating
           point comparisons.  Complementary scaling-up for actual data values
           occurs below...
          */
					miss_data->missing_data *= pow(10, var->precision);
					miss_data->missing_data += 0.5;
					miss_data->missing_data -= fmod(miss_data->missing_data, 1);
				}
				else
				{
					/* scale-up "implied" float to a whole number.  missing data flag
					   must be entered as a floating point number if it applies to an
					   implied precision integer.
					*/
					miss_data->missing_data *= pow(10, var->precision);
					assert(fmod(miss_data->missing_data, 1) == 0);
				}
			} /* if real, or non-zero precision (e.g., integers with implied precision) */
			else
			{
				assert(IS_INTEGER(var) && !var->precision);
				
				wfprintf(stderr, "Ignoring missing data flag value %ld for %s\n",
				        (long)miss_data->missing_data, var->name);
				if ( 1 || std_args->output_file  )
					wfprintf(output_file, "Ignoring missing data flag value %ld for %s\n",
					        (long)miss_data->missing_data, var->name);
			}
		} /* if ignore missing data */

		view_list = dll_next(view_list);
	}	/* End setting up view list */
	
	record_length = FORMAT_LENGTH(binary_format);
	if (std_args->query_file && ee_check_vars_exist(einfo, binary_format))
		return(ERR_GEN_QUERY);
                                    
	/* TRAVERSE THE LIST OF VIEWS AND CREATE HISTOGRAM FILES FOR EACH VIEW */
	if (db_find_format(input_data->format_list, FFF_GROUP, FFF_HD | FFF_REC, END_ARGS))
		record_header_exist = 1;
	view_list = dll_first(view_list);
	while (dll_data(view_list))
	{
		wfprintf(stderr, "\n");
		if ( 1 || std_args->output_file  )
			wfprintf(output_file, "\n");

#ifdef TIMER
		(void)time(&start_time);
#endif

		view = (DATA_VIEW_PTR)dll_data(view_list);
		miss_data = (MISSING_DATA_PTR)view->info;
		var = view->var_ptr;
	
		/* Open histogram output file */
		if (!maxmin_only && (histo_file = fopen(view->title, "w")) == NULL)
			return(ERR_OPEN_FILE);

		/* Reset file to the beginning */

		error = db_set(input_data, 
		               DBIN_SET_TOTAL_RECORDS, std_args->records_to_read, END_ARGS);
		if (error)
		{
			sprintf(error_buffer,"Resetting data file position for %s",view->title);
			return(err_push(ROUTINE_NAME, ERR_SET_DBIN, error_buffer));
		}

		if (record_header_exist)
			input_data->data_available = 0;
		if (precision == FLAG_PRECISION)
			precision = var->precision;
		else
			precision = (int)min((int)precision, (int)var->precision);
		/* This last min was introduced just to maintain consistency.
		   If not here, a precision > 5 could possibly exist if
		   entered from the variable file.
		*/
		precision = (int)min(precision, MAXIMUM_PRECISION);

		/* DETERMINE THE FACTOR FROM THE PRECISION */
		factor = (double)pow(10.0, precision);
		assert(fabs(factor) > DBL_EPSILON);

		if (IS_TEXT(var))
		{
			count = (int)(var->end_pos - var->start_pos + 1);
 			count = min(count, (MAX_NAME_LENGTH - 1));
			*(str + count) = '\0';
		}

		num_records = 1;
		/* READ EVERY DATA VALUE OF A VARIABLE AND DETERMINE THE MIN & MAX */
		while (view->dbin->bytes_available)
		{
			/* SET view->data TO NULL ALLOWING FURTHER READS
			   IF CACHE IS SMALLER THAN DATA FILE */ 
			view->data = view->first_pointer;
			if (db_events(view->dbin,
			 PROCESS_FORMAT_LIST, FFF_ALL_TYPES,
			 END_ARGS))
				return(ERR_DBIN_EVENT);
			
			if (db_events(view->dbin,
			     DBIN_DATA_TO_NATIVE, (void *)NULL, (void *)NULL, (void *)NULL,
			     END_ARGS))
				return(ERR_DBIN_EVENT);
			
			if (ascii_view_list && db_events(input_data,
				 DBIN_CONVERT_CACHE, binary_buffer, (FORMAT_PTR)binary_format,
				 (long*)&result_bytes,
				 END_ARGS))
				return(ERR_DBIN_EVENT);

			/* Loop the records in the cache */				
			for (num_lines = input_data->records_in_cache; num_lines > 0; --num_lines, ++num_records)
			{
				/* Retrieve a value for the variable and compare it to MAXMIN */
				if (IS_TEXT(var))
				{
					/* Check for a query */
					if (std_args->query_file)
					{
						record_in_buffer = ((char *)view->data - var->start_pos + 1);
						if (ee_set_var_values(einfo, (void *)record_in_buffer, binary_format, scratch_buffer))
							return(ERR_GEN_QUERY);

						/* Check to see if we pass it */
						if (!ee_evaluate_equation(einfo, &error))
						{
							view->data += view->increment;
							continue;
						}
					}
					(void)memcpy(str, view->data, count);

					mm_set(miss_data->maxmininfo,
					 MM_MAX_MIN, str, num_records,
					 END_ARGS);
				}
				else
				{
					/* The maxmin calculation is done without changing the precision
					of the variable, the adjustment gets done prior to printing it out. We also
					assume at this point that the data is binary. If it is ASCII, it will be
					converted to binary using the same scheme as in maxmin.c The copy
					to double_var is done to insure alignment. */
					double_var = 0;
					
					memcpy((void*)&double_var, (void*)view->data, FF_VAR_LENGTH(var));

					/* check to see if this is missing data */
					if (miss_data->missing_data_exists)
					{
						if (!btype_to_btype((void *)view->data, var->type, (void *)(&(miss_data->temp_dvar)), FFV_DOUBLE))
						{
							if (var->precision)
							{
								if (IS_REAL(var))
								{
									miss_data->temp_dvar *= pow(10, var->precision);
									miss_data->temp_dvar += 0.5;
									miss_data->temp_dvar -= fmod(miss_data->temp_dvar, 1);
								}
								else 
								{
									assert(IS_INTEGER(var));
									/* nothing to change, miss_data->missing_data was already
									   "upscaled" to a whole number*/
								}
							}
				      if (miss_data->temp_dvar == miss_data->missing_data)
				      { /* it is! */
								view->data += view->increment;
								continue;
							}
						}
						else /* If btype_to_btype failed, just go on with other business */
							;
					}
					
					/* Check to see if a query exists */
					if (std_args->query_file)
					{
						record_in_buffer = ((char *)view->data - var->start_pos + 1);
						if (ee_set_var_values(einfo, (void *)record_in_buffer, binary_format, scratch_buffer))
							return(ERR_GEN_QUERY);

						/* Check to see if we pass it */
						if (!ee_evaluate_equation(einfo, &error))
						{
							view->data += view->increment;
							continue;
						}
					}
					
					mm_set(miss_data->maxmininfo,
					 MM_MAX_MIN, (char *)&double_var, num_records,
					 END_ARGS);
				}

				if (!maxmin_only)
				{
					/* COLLAPSE TREE IF RUNNING OUT OF MEMORY */
					if (!IS_TEXT(var))
					{
						if (bins > MAXIMUM_MAXBINS)
						/* ^^^ this is a kludgy way of deciding we're out of memory */
						{
	#ifdef TREESIZ	
							wfprintf(stderr,"%ld bins in the tree before forced collapse.\n",bins);
	#endif
		
							bins = 0;
							sorterfactor *= 2;
							firstnode = 1;
							error = collapse(root,sorterfactor);
							if (error)
								return(error);
							if ((p = (void *)talloc(sizeof(BIN_ARRAY_TYPE))) == NULL)
								return(ERR_MEM_LACK);
							else
							{	
								bat_ptr = (BIN_ARRAY_TYPE *)((char *)p + sizeof(HEADER));
								bat_ptr->count = cnt;
								bat_ptr->bin = thisbin;
								if (insert(&newroot,(HEADER *)bat_ptr,icmp))	/* cast for CW compiler -rf01 */
								{  
									sprintf(error_buffer,"\nCollision in collapsing tree: bin %ld count %ld",bat_ptr->bin,bat_ptr->count); 
									return(ERR_PROCESS_DATA);
								}
								else 
									bins++;
							}
							freeall(&root);
							root = newroot;
							newroot = NULL;
							wfprintf(stderr,"Collapsed sorting tree because of memory limitations\n");
						} /* if ran out of memory -- if (bins >= 10000) */
					} /* if (!IS_TEXT(var)) */
		
					/* COLLAPSE TREE IF BIGGER THAN REQUESTED */ 
					if ((bins > (int)(3 * maxbins / 2)) && (!nevercollapse))
					{
						if (!IS_TEXT(var))
						{
#ifdef TREESIZ	
							wfprintf(stderr,"%ld bins in the tree before collapse.\n",bins);
#endif
							bins = 0;
							sorterfactor *= 2;
							firstnode = 1;
							error = collapse(root,sorterfactor);
							if (error)
								return(error);
							if ((p = (void *)talloc(sizeof(BIN_ARRAY_TYPE))) == NULL)
								return(ERR_MEM_LACK);
							else
							{	
								bat_ptr = (BIN_ARRAY_TYPE *)((char *)p + sizeof(HEADER));
								bat_ptr->count = cnt;
								bat_ptr->bin = thisbin;
								if (insert(&newroot,(HEADER *)bat_ptr,icmp))	/* cast for CW compiler -rf01 */
								{  
									sprintf(error_buffer,"\nCollision in collapsing tree: bin %ld count %ld\n",bat_ptr->bin,bat_ptr->count);
									return(err_push(ROUTINE_NAME, ERR_PROCESS_DATA, error_buffer));
								}
								else 
									bins++;
							}
							freeall(&root);
							root = newroot;
							newroot = NULL;
						}
					} /* if tree too big */
				   
					/* ADD DATA TO HISTOGRAM:
						The binning process assumes that double_var has the actual value of the
						variable adjusted for precision. This must be set up prior to the calculation
						of the sorter. */
	
					if (IS_TEXT(var))
					{
						if ((p = (void *)talloc(sizeof(CHAR_ARRAY_TYPE))) == NULL)
						{
							(void)wfprintf(stderr, "%s: Can't histogram this variable.\n",var->name);
							(void)wfprintf(stderr, "Out of memory: too many bins. Going to next request.\n");
							goto nexvar;
						}
						else
						{
							cat_ptr = (CHAR_ARRAY_TYPE *)( (char *)p + sizeof(HEADER) );
							cat_ptr->count = 1;
							if (var->precision)
							{
								strncpy(cat_ptr->bin, str, var->precision);
							  /* This is done because strncpy does NOT append a '\0' */
							  cat_ptr->bin[var->precision] = '\0';
							}
							else
								strcpy(cat_ptr->bin, str);
		
							if ((p2 = insert(&root,(HEADER *)cat_ptr,iccmp)) != NULL)	/* (HEADER *) cast for CW compiler -rf01 */
							{
								((CHAR_ARRAY_TYPE *)p2)->count++;
								(void)memFree((CHAR_ARRAY_TYPE *)p, "p");
							}
						} /* else, p successfully talloc'd */
					} /* if (IS_TEXT(var)) */
					else
					{	/* numeric data */
						if ((p = (void *)talloc(sizeof(BIN_ARRAY_TYPE))) == NULL)
							return(ERR_MEM_LACK);
						else
						{
							memcpy(&adjustment, &double_var, sizeof(double));
							error = btype_to_btype(&adjustment, FFV_TYPE(var),
							                       &double_var, FFV_DOUBLE);
              if (error)
              	return(ERR_OUT_OF_RANGE);
										                
							adjustment = (double)precision;                
							if (IS_INTEGER(var))
							{
								adjustment -= var->precision;
							}
							double_var *= pow(10.0, adjustment);
		/**/
							double_var = floor(double_var) + (double_var < 0 ? -FLT_EPSILON : FLT_EPSILON);
							if (btype_to_btype((void *)&double_var, FFV_DOUBLE, (void *)&sorter, FFV_LONG))
								return(ERR_OUT_OF_RANGE);
		/**/
		/* MAO:d 6/95 two lines below replaced with above function call */
							/* double_var incremented for purposes of possible round-off errors */
		/**
							double_var += .0000000001;
							sorter = (long)(floor(double_var));
**/				
							bat_ptr = (BIN_ARRAY_TYPE *)( (char *)p + sizeof(HEADER) );
							bat_ptr->count = 1;
							if ((sorter < 0) && (sorter % sorterfactor != 0))
							{
								bat_ptr->bin = ((long)((sorter - sorterfactor)/sorterfactor))*sorterfactor;
								if ((sorter < 0) && (bat_ptr->bin >= 0))
									bat_ptr->bin = LONG_MIN;
							}
							else
								bat_ptr->bin = ((long)((sorter)/sorterfactor))*sorterfactor;
							if ((p2 = insert(&root,(HEADER *)bat_ptr,icmp)) != NULL)	/* (HEADER *) cast for CW compiler -rf01 */
							{
								((BIN_ARRAY_TYPE *)p2)->count++;
								(void)memFree((BIN_ARRAY_TYPE *)p, "p");
							}
							else
								bins++;
						} /* else, p successfully talloc'd */
					} /* else, if (IS_TEXT(var)) -- var is not char */
				} /* end of if (!maxmin_only) */

				view->data += view->increment;

		  } /* End of Loop over a single cache */

			/* Calculate percentage left to process and display */
			assert(file_size);
			perc_left = (int)((1 - ((float)input_data->bytes_available / file_size)) * 100);
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

		} /* End of Loop over all caches */

		/* PRINT OUT THE MINIMUM AND MAXIMUM VALUE */
		minimum = mm_getmn(miss_data->maxmininfo);
		maximum = mm_getmx(miss_data->maxmininfo);
		min_rec_occurrence = (miss_data->maxmininfo)->min_record;
		max_rec_occurrence = (miss_data->maxmininfo)->max_record;
		
		/* record occurrences may be relative to the tail */
		if (std_args->records_to_read < 0)
		{
			file_size = os_filelength(input_data->file_name);
			assert(FORMAT_LENGTH(input_data->input_format));
			file_size /= (long)FORMAT_LENGTH(input_data->input_format);
			
			/* file_size has been divided by input format length to yield
			   total records in data file (we're assuming that the -c option
			   is not used on data files with embedded headers...).  file_size
			   + std_args->records_to_read (which is negative) gives the starting
			   record number counting from the top of the file, which is what
			   record_occurrences (above) are relative to.  Adding these two together
			   yields the absolute record occurrences. */
			min_rec_occurrence += (file_size + std_args->records_to_read);
			max_rec_occurrence += (file_size + std_args->records_to_read);
		}
		
		if (IS_INTEGER(var))
		{
			adjustment = -(double)var->precision;
			minimum *= pow(10.0, adjustment);
			maximum *= pow(10.0, adjustment);
		}
		
		wfprintf(stderr, "\n\n");

		if (IS_TEXT(var))
		{
			wfprintf(stderr, "%s: %ld values read\n", var->name, num_records - 1);
			wfprintf(stderr, "minimum: %s found at record %ld\n", (char *)((long)(minimum)), min_rec_occurrence);
			wfprintf(stderr, "maximum: %s found at record %ld\n", (char *)((long)(maximum)), max_rec_occurrence);
			wfprintf(stderr, "Summary file: %s\n", maxmin_only ? "No Summary" : view->title);
			if ( 1 || std_args->output_file  )
			{
				wfprintf(output_file, "%s: %ld values read\n", var->name, num_records - 1);
				wfprintf(output_file, "minimum: %s found at record %ld\n", (char *)((long)(minimum)), min_rec_occurrence);
				wfprintf(output_file, "maximum: %s found at record %ld\n", (char *)((long)(maximum)), max_rec_occurrence);
				wfprintf(output_file, "Summary file: %s\n", maxmin_only ? "No Summary" : view->title);
			}
		}
		else
		{
			/* determine maximum field widths for min/max report */
			col1_len = col2_len = 0;
			sprintf(str, "%.0f", minimum);
			col1_len = strlen(str);
			sprintf(str, "%.0f", maximum);
			if ((size_t)col1_len < strlen(str))
				col1_len = strlen(str);
			
			if (var->precision)
				col1_len += var->precision + 1;
			
			sprintf(str, "%ld", min_rec_occurrence);
			col2_len = strlen(str);
			sprintf(str, "%ld", max_rec_occurrence);
			if ((size_t)col2_len < strlen(str))
				col2_len = strlen(str);
			
			if (!maxmin_only)
				wfprintf(stderr, "Histogram data precision: %d, Number of sorting bins: %ld\n",
				 (int)precision, (long)bins);
			wfprintf(stderr, "%s: %ld values read", var->name, num_records - 1);
			if (std_args->records_to_read > 0)
				wfprintf(stderr, " from head of file\n");
			else if (std_args->records_to_read < 0)
				wfprintf(stderr, " from tail of file\n");
			else
				wfprintf(stderr, "\n");
			wfprintf(stderr, "minimum: %*.*f found at record %*ld\n",
			 (int)col1_len, (int)(var->precision), minimum, (int)col2_len, min_rec_occurrence);
			wfprintf(stderr, "maximum: %*.*f found at record %*ld\n",
			 (int)col1_len, (int)(var->precision), maximum, (int)col2_len, max_rec_occurrence);
			wfprintf(stderr, "Summary file: %s\n", maxmin_only ? "No Summary" : view->title);
			if ( 1 || std_args->output_file  )
			{
				if (!maxmin_only)
					wfprintf(output_file, "Histogram data precision: %d, Number of sorting bins: %ld\n",
					 (int)precision, (long)bins);
				wfprintf(output_file, "%s: %ld values read", var->name, num_records - 1);
				if (std_args->records_to_read > 0)
					wfprintf(output_file, " from head of file\n");
				else if (std_args->records_to_read < 0)
					wfprintf(output_file, " from tail of file\n");
				else
					wfprintf(output_file, "\n");
				wfprintf(output_file, "minimum: %*.*f found at record %*ld\n",
				 (int)col1_len, (int)(var->precision), minimum, (int)col2_len, min_rec_occurrence);
				wfprintf(output_file, "maximum: %*.*f found at record %*ld\n",
				 (int)col1_len, (int)(var->precision), maximum, (int)col2_len, max_rec_occurrence);
				wfprintf(output_file, "Summary file: %s\n", maxmin_only ? "No Summary" : view->title);
			}
/*		
			sprintf(scratch_buffer,"\n%%s (%%s)\nMinimum: %%.%df at %%ld Maximum: %%.%df at %%ld\n", precision, precision);
			printf(scratch_buffer, var->name, view->title, minimum, min_rec_occurrence, maximum, max_rec_occurrence);
			wfprintf(stderr,scratch_buffer, var->name, view->title, minimum, min_rec_occurrence, maximum, max_rec_occurrence);
*/
		}
		
		if (!maxmin_only)
		{
		/* PRINT OUT AND FREE THE HISTOGRAM TREES */
			if (IS_TEXT(var))
				tcwrite(root);
			else
				output_number_histogram(root, (unsigned char)precision);
	
nexvar:
			if (fclose(histo_file) != 0)
				return(ERR_WRITE_FILE); /* replace this with a better error code */
	
#ifdef HEAPCHK
			/*  Print heap size before free     */
			heapPrt("before free");	 
#endif
	
			freeall(&root);
			root = NULL;
			bins = 0;
#ifdef HEAPCHK
			/*  Print heap size after free    */
			heapPrt("after free");	 
#endif
			sorterfactor = 1;	
		}	/* end of if (!maxmin_only) */

		view_list = dll_next(view_list);

	} /* while (dll_data(view_list)) */

#ifdef TIMER
/* If multiple variables, report net elapsed time */
	if (view_list->previous != view_list->next)
	{
		(void)time(&finish_time);
		elapsed_time = (long)difftime(finish_time, net_start_time);
		wfprintf(stderr, "\nNet elapsed time - %02d:%02d:%02d\n",
		        (int)(elapsed_time / (3600)),
		        (int)((elapsed_time / 60) % 60),
		        (int)(elapsed_time % 60)
		);
	}
#endif

	return(0);

}	/* ******************** end of main ****************** */

void prnt(FILE *stream, BIN_ARRAY_TYPE   *ptrto)
{
	(void)wfprintf(stream,ptrto ? "%2d" :" ",ptrto->bin);
}
int icmp(BIN_ARRAY_TYPE *n1,BIN_ARRAY_TYPE *n2)	/* Comparison function for insert to use */
{

	/*  ORIGINAL FUNCTION return(n1->bin - n2->bin);     */

	if (n1->bin<n2->bin)
		return(-1);
	if (n1->bin>n2->bin)
		return(1);

	return(0);
}

int iccmp(CHAR_ARRAY_TYPE *n1,CHAR_ARRAY_TYPE *n2)    /* Comparison function for insert to use */
{
	return(strcmp(n1->bin,n2->bin));
}

static void output_number_histogram(HEADER *root, int user_precision)
/*****************************************************************************
 * NAME:  output_number_histogram()
 *
 * PURPOSE:  Writes the .lst file with fixed field formatting
 *
 * USAGE:  output_number_histogram(root);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Makes two passes through the bins:  first to determine
 * the maximum field widths, and the second to write the histogram file with
 * fixed fields that are sized according to the first pass.
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
	int col1_len = 0,
	    col1_prec = 0,
	    col2_len = 0;
	
	tpass1(root, &col1_len, &col1_prec, &col2_len);
	col1_len += col1_prec && user_precision ? 1 + user_precision : 0;
	tpass2(root, col1_len, user_precision, col2_len);
}

static void tpass1(HEADER *root, int *col1_len, int *col1_prec, int *col2_len)
/*****************************************************************************
 * NAME:  tpass1()
 *
 * PURPOSE:  Perform the first pass -- see output_number_histogram()
 *
 * USAGE:  tpass1(root, &col1_len, &col2_len);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Makes the first pass through the bins to determine the
 * maximum field widths.  Since the bins are nodes in a binary tree, this is
 * a recursive function.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:  factor (Module)
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

#define TPASS_LBUF_SIZE 80

{
	char col_buffer[TPASS_LBUF_SIZE],
	     *decimal;
	BIN_ARRAY_TYPE *temp;
	
	if (!root)
		return;
	
	tpass1(root->left, col1_len, col1_prec, col2_len);
	
	temp = (BIN_ARRAY_TYPE *)(root + 1);
	sprintf(col_buffer,"%-*.*g", TPASS_LBUF_SIZE - 1, TPASS_LBUF_SIZE - 1, (temp->bin) / factor);
	(void)os_str_trim_whitespace(col_buffer, col_buffer);
	decimal = strchr(col_buffer, '.');
	if (decimal && !strchr(col_buffer, 'e'))
	{
		if ((size_t)*col1_prec < strlen(decimal) - 1)
			*col1_prec = strlen(decimal) - 1;
		*col1_prec = max((size_t)*col1_prec, strlen(decimal) - 1);
		*decimal = STR_END;
	}
	*col1_len = max((size_t)*col1_len, strlen(col_buffer));
	sprintf(col_buffer,"%ld", temp->count);
	*col2_len = max((size_t)*col2_len, strlen(col_buffer));
	
	tpass1(root->right, col1_len, col1_prec, col2_len);
}

static void tpass2(HEADER *root, int col1_len, int col1_prec, int col2_len)
/*****************************************************************************
 * NAME:  tpass2()
 *
 * PURPOSE:  Performs the second pass -- see output_number_histogram()
 *
 * USAGE:  tpass2(root, col1_len, col1_prec, col2_len);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  col1_len, col1_prec, and col2_len contain the maximum field
 * width for column one, the maximum number of decimal places for column one,
 * and the maximum field width for column two, respectively.  Using this
 * information, this function will print two fixed field columns to the
 * summary (aka histogram) file, taking care that decimal points in the first
 * column are aligned, and that trailing zero's in the decimal portion are
 * added.  This is a recursive function.
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * GLOBALS:  histo_file (Module), factor (Module)
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/
/*
 * HISTORY:
 *	Rich Fozzard	12/22/95		-rf01
 *		(HEADER *) cast for CW compiler
*/

{
	BIN_ARRAY_TYPE *trojan;
	char lbuf[TPASS_LBUF_SIZE];

	if (root)
	{
		tpass2(root->left, col1_len, col1_prec, col2_len);
		trojan = (BIN_ARRAY_TYPE *)(root + 1);
		sprintf(lbuf, "%*.*g", (int)col1_len, (int)(col1_len - (col1_prec ? 1 : 0)),
		 trojan->bin / factor);
		if (strchr(lbuf, 'e'))
		{
			wfprintf(histo_file, "%*.*G\t%*ld\n",
			 (int)(col1_len), (int)(col1_len - (col1_prec ? 1 : 0)),
			  trojan->bin / factor,
			 (int)col2_len, trojan->count);
		}
		else
		{
			wfprintf(histo_file, "%*.*f\t%*ld\n", col1_len, col1_prec, trojan->bin / factor,
			 col2_len, trojan->count);
		}
		tpass2(root->right, col1_len, col1_prec, col2_len);
	}
}

static void tcwrite(HEADER *root)
/* Traverses the trees inorder and prints the histogram
		out to the file that is externally declared  	 */
{
	CHAR_ARRAY_TYPE *trojan;

	if (root)
	{
		tcwrite(root->left);
		trojan = (CHAR_ARRAY_TYPE *)(root + 1);
		wfprintf(histo_file,"%s %ld\n", trojan->bin, trojan->count);
		tcwrite(root->right);
	}
}

static int collapse(HEADER *root, long sorterfactor)
{
	BIN_ARRAY_TYPE *bat_ptr,*locroot;
	HEADER *p;
	int error = 0;
	
	if (root)
	{
		error = collapse(root->left,sorterfactor);
		if (error)
			return(error);
		++root;
		locroot = (BIN_ARRAY_TYPE *)root;

		if (firstnode)
		{
			firstnode = 0;
			cnt = 0;
			if ((locroot->bin < 0) && (locroot->bin % sorterfactor != 0))
				thisbin = ((long)((locroot->bin - sorterfactor)/sorterfactor)) * sorterfactor;
			else
 				thisbin = ((long)(locroot->bin/sorterfactor)) * sorterfactor;

			if ((locroot->bin < 0) && (thisbin >= 0))
				thisbin = LONG_MIN;
		}

		if ((locroot->bin < 0) && ((locroot->bin >= thisbin) && (locroot->bin < (thisbin + sorterfactor))))
			cnt += locroot->count;	  	
		else if ((locroot->bin >= 0) && ((((long)(locroot->bin/sorterfactor)) * sorterfactor) <= thisbin))
			cnt += locroot->count;	  	
		else
		{
			if ((p = (void *)talloc(sizeof(BIN_ARRAY_TYPE))) == NULL)
				return(ERR_MEM_LACK);
			else
			{
				bat_ptr = (BIN_ARRAY_TYPE *)( (char *)p + sizeof(HEADER) );
				bat_ptr->count = cnt;
				bat_ptr->bin = thisbin;
				if (insert(&newroot,(HEADER *)bat_ptr,icmp))	/* (HEADER *) cast for CW compiler -rf01 */
				{
					sprintf(error_buffer, "\nCollision in collapsing tree: bin %ld count %ld", bat_ptr->bin,bat_ptr->count);
					return(err_push(ROUTINE_NAME, ERR_PROCESS_DATA, error_buffer));
				}
				else 
					bins++;
			}

			if ((locroot->bin < 0) && (locroot->bin % sorterfactor != 0))
				thisbin = ((long)((locroot->bin - sorterfactor)/sorterfactor)) * sorterfactor;
			else
 				thisbin = ((long)(locroot->bin)/sorterfactor) * sorterfactor;
			  	
			cnt = locroot->count;

			if ((locroot->bin < 0) && (thisbin >= 0)) 
				thisbin = LONG_MIN;
	  } /* else, if (locroot->bin >= 0) && ... */
		--root;
		error = collapse(root->right,sorterfactor);
		if (error)
			return(error);
	} /* if (root) */
	return(0);
}

static int mkstdbin_cb(int routine_name)
{
	return(routine_name != OUTPUT_FORMAT);
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
  va_list va_args;

#ifndef XVT

/* This is a temporary hack until redirection is no longer supported
   (use -o instead)
*/
	if (   (char HUGE *)stream == (char HUGE *)stdout 
	    && isatty(fileno(stdout))
	   )
		return(0);

#endif
  
  va_start(va_args, format);
	
	return(vfprintf(stream, format, va_args));
}

