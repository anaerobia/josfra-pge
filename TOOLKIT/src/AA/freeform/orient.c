#include <math.h>
#include <limits.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA
#include <databin.h>
#include <dataview.h>
#include <ndarray.h>

static int mkstdbin_cb(int event_failure)
{
	return(event_failure != OUTPUT_FORMAT &&
	       event_failure != INPUT_FORMAT
	      );
}

/* Function Prototypes */			    
static void command_line_usage_help(void);

int make_descriptor_string_from_header(DATA_BIN_PTR dbin, char *descriptor);
int get_descriptor_strings_from_eqv(DATA_BIN_PTR dbin, char *super_descriptor,
    char *sub_descriptor);
int check_grid_origin(DATA_BIN_PTR dbin, ARRAY_DESCRIPTOR_PTR super_array_descriptor,
    char *sub_descriptor_string);
int get_output_name_table(char *file_name, FFF_STD_ARGS *std_args,
    DATA_BIN_PTR idbin, DATA_BIN_PTR odbin);
int make_interim_descriptors(ARRAY_DESCRIPTOR_PTR super_descriptor,
    ARRAY_DESCRIPTOR_PTR sub_descriptor,
    ARRAY_DESCRIPTOR_PTR *interim1_descriptor, ARRAY_DESCRIPTOR_PTR *interim2_descriptor);

#define DYNAMICALLY_GENERATED_ARRAY_NAME "created_from_header"
#define DYNAMICALLY_GENERATED_ROWS_NAME "rows"
#define DYNAMICALLY_GENERATED_COLUMNS_NAME "columns"

/* name table keywords */
#define INPUT_ARRAY_DESCRIPTOR_KEYWORD "super_ff_array"
#define OUTPUT_ARRAY_DESCRIPTOR_KEYWORD "sub_ff_array"

#define END_COLUMN_KEYWORD "end_column"
#define END_ROW_KEYWORD "end_row"
#define GRID_ORIGIN "grid_origin"
#define START_X_KEYWORD "left_map_x"
#define END_Y_KEYWORD "lower_map_y"
#define NUM_COLUMNS_KEYWORD "number_of_columns"
#define NUM_ROWS_KEYWORD "number_of_rows"
#define END_X_KEYWORD "right_map_x"
#define START_COLUMN_KEYWORD "start_column"
#define START_ROW_KEYWORD "start_row"
#define START_Y_KEYWORD "upper_map_y"

#define END_COLUMN_KEYWORD_TYPE FFV_SHORT
#define END_ROW_KEYWORD_TYPE FFV_SHORT
#define GRID_ORIGIN_CHAR_TYPE FFV_CHAR
#define GRID_ORIGIN_SHORT_TYPE FFV_SHORT
#define START_X_KEYWORD_TYPE FFV_FLOAT
#define END_Y_KEYWORD_TYPE FFV_FLOAT
#define NUM_COLUMNS_KEYWORD_TYPE FFV_SHORT
#define NUM_ROWS_KEYWORD_TYPE FFV_SHORT
#define END_X_KEYWORD_TYPE FFV_FLOAT
#define START_COLUMN_KEYWORD_TYPE FFV_SHORT
#define START_ROW_KEYWORD_TYPE FFV_SHORT
#define START_Y_KEYWORD_TYPE FFV_FLOAT

typedef short C_END_COLUMN_KEYWORD_TYPE;
typedef short C_END_ROW_KEYWORD_TYPE;
typedef short C_GRID_ORIGIN_SHORT_TYPE;
typedef float C_START_X_KEYWORD_TYPE;
typedef float C_END_Y_KEYWORD_TYPE;
typedef short C_NUM_COLUMNS_KEYWORD_TYPE;
typedef short C_NUM_ROWS_KEYWORD_TYPE;
typedef float C_END_X_KEYWORD_TYPE;
typedef short C_START_COLUMN_KEYWORD_TYPE;
typedef short C_START_ROW_KEYWORD_TYPE;
typedef float C_START_Y_KEYWORD_TYPE;

char *greeting =
{
#ifdef ALPHA
"\nWelcome to orient alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n"
#elif defined(BETA)
"\nWelcome to orient beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n"
#else
"\nWelcome to orient release "FF_LIB_VER" -- an NGDC FreeForm application\n\n"
#endif
};

typedef unsigned long count_t; /* nonnegative counts */
typedef short vector_t; /* magnitude and direction */

#define ROUTINE_NAME "ORIENT"
int main(int argc, char *argv[])
{
	DATA_BIN_PTR idbin, /* Data Bin for User Data */
	             odbin; /* output data bin -- necessarily stripped */
	
	char super_descriptor_string[2 * MAX_NAME_LENGTH],
	     sub_descriptor_string[2 * MAX_NAME_LENGTH],
	     *descriptor_copy = NULL;
	FILE *output_file = stdout;
	
	FFF_STD_ARGS std_args;
	int error, num_dstrings;
	int embedded_header_length;
	int input_complete, output_complete;
	long bytes_written, bytes_read, total_bytes_read, total_bytes_written,
	     input_file_size;
	BOOLEAN do_convert_cache;
	
	ARRAY_DESCRIPTOR_PTR super_array_descriptor = NULL;
	ARRAY_DESCRIPTOR_PTR sub_array_descriptor = NULL;
	ARRAY_DESCRIPTOR_PTR interim1_array_descriptor = NULL;
	ARRAY_DESCRIPTOR_PTR interim2_array_descriptor = NULL;
	ARRAY_MAPPING_PTR file_to_cache_mapping = NULL;
	ARRAY_MAPPING_PTR cache_to_file_mapping = NULL;
	
	if (argc < 3)
	{
		command_line_usage_help();
		return(EXIT_FAILURE);
	}

	if ((error = parse_command_line(argc, argv, &std_args)) != 0)
	{
		err_disp();
		exit(EXIT_FAILURE);
	}

	if (err_state())
#ifdef DEBUG_MSG
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#else
		err_clear();
#endif

	if (make_standard_dbin(&std_args, &odbin, mkstdbin_cb))
	{
		return(ERR_MAKE_DBIN);
	}

	if (err_state())
#ifdef DEBUG_MSG
	{
		err_push(ROUTINE_NAME, ERR_DEBUG_MSG, NULL);
		err_disp();
	}
#else
		err_clear();
#endif

	if ((odbin = db_make("output")) == NULL)
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, "Unable to create output data bin");

	num_dstrings = get_descriptor_strings_from_eqv(idbin, super_descriptor_string,
	               sub_descriptor_string);

	if (num_dstrings != 2 &&
	 (error = make_descriptor_string_from_header(idbin, super_descriptor_string)))
	{
		err_push(ROUTINE_NAME, ERR_GENERAL, "\nUnable to describe input array -- cannot continue\n");
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	if ((descriptor_copy = (char *)memStrdup(super_descriptor_string,
	 "descriptor_copy")) == NULL)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "temporary string");
		err_disp();
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Input Array Description:  %s\n", super_descriptor_string);
	
	if (!(super_array_descriptor = ndarr_create_from_str(descriptor_copy)))
	{
		err_push(ROUTINE_NAME, ERR_SYNTAX, "Input array descriptor string");
		err_disp();
		exit(EXIT_FAILURE);
	}
	memFree(descriptor_copy, "descriptor_copy");

	if (num_dstrings != 2)
	{
		if (!(error = get_output_name_table(argv[argc - 1], &std_args, idbin, odbin)))
 			error = make_descriptor_string_from_header(odbin, sub_descriptor_string);
		
		/* If unable to create output name table/header then 
		   try input array grid_origin to indicate reorientation */
		if (error && (error = check_grid_origin(idbin, super_array_descriptor,
		 sub_descriptor_string)))
		{
			err_push(ROUTINE_NAME, ERR_GENERAL, "\nUnable to determine desired reorientation -- cannot continue\n");
			err_disp();
			exit(EXIT_FAILURE);
		}
	}
	
	if ((descriptor_copy = (char *)memStrdup(sub_descriptor_string,
	 "descriptor_copy")) == NULL)
	{
		err_push(ROUTINE_NAME, ERR_MEM_LACK, "temporary string");
		err_disp();
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Output Array Description:  %s\n", sub_descriptor_string);
	
	if (!(sub_array_descriptor = ndarr_create_from_str(descriptor_copy)))
	{
		err_push(ROUTINE_NAME, ERR_SYNTAX, "Input array descriptor string");
		err_disp();
		exit(EXIT_FAILURE);
	}
	memFree(descriptor_copy, "descriptor_copy");

	error = make_interim_descriptors(super_array_descriptor,
	        sub_array_descriptor, &interim1_array_descriptor,
	        &interim2_array_descriptor);
	if (error)
	{
		err_push(ROUTINE_NAME, ERR_NDARRAY, "Transitional descriptors");
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	if (!(file_to_cache_mapping = ndarr_create_mapping(interim1_array_descriptor,
	      super_array_descriptor)))
	{
		err_push(ROUTINE_NAME, ERR_GENERAL, "\nUnable to create first transitional array mapping -- cannot continue\n");
		err_disp();
		exit(EXIT_FAILURE);
	}

	if (!(cache_to_file_mapping = ndarr_create_mapping(sub_array_descriptor,
	      interim2_array_descriptor)))
	{
		err_push(ROUTINE_NAME, ERR_GENERAL, "\nUnable to create second transitional array mapping -- cannot continue\n");
		err_disp();
		exit(EXIT_FAILURE);
	}

	embedded_header_length =
	 (idbin->header_format->type & FFF_SEPARATE) == FFF_SEPARATE ? 0 :
	 FORMAT_LENGTH(idbin->header_format);
	
	do_convert_cache =
	 (super_array_descriptor->element_size == (long)FORMAT_LENGTH(idbin->input_format)
	 && sub_array_descriptor->element_size == (long)FORMAT_LENGTH(idbin->output_format));
	
	if (do_convert_cache)
	{
		idbin->state.cache_filled = 1;
		
		if (error = db_show(idbin, INPUT_FORMAT, END_ARGS))
		{
			err_push(ROUTINE_NAME, error, "Error in describing data bin -- input format");
			err_disp();
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "Reading %s with %s\n", idbin->file_name, idbin->buffer);
		
		if (error = db_show(idbin, OUTPUT_FORMAT, END_ARGS))
		{
			err_push(ROUTINE_NAME, error, "Error in describing data bin -- output_format");
			err_disp();
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "Writing output data with %s\n", idbin->buffer);

		/* downsize cache size so that it can contain a multiple of either of
		   input or output format lengths
		   -- reuse variables bytes_read */
		bytes_read = max((int)FORMAT_LENGTH(idbin->input_format),
				(int)FORMAT_LENGTH(idbin->output_format));
		idbin->records_in_cache = idbin->cache_size / bytes_read;
		odbin->records_in_cache = idbin->cache_size / bytes_read;
			
		if (db_set(idbin, DBIN_CACHE_SIZE,
		 idbin->records_in_cache * FORMAT_LENGTH(idbin->input_format),
		 END_ARGS))
			return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "buffers"));

		if (db_set(odbin, DBIN_CACHE_SIZE,
		 odbin->records_in_cache * FORMAT_LENGTH(idbin->output_format),
		 END_ARGS))
			return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "buffers"));
	}
	else
	{
		if (super_array_descriptor->element_size != sub_array_descriptor->element_size)
		{
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "\nInput and Output Array element sizes do not match!\nProcessing cannot continue unless format conversion is indicated\n");
			err_disp();
			exit(EXIT_FAILURE);
		}
		else
		{
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "\nArray element size(s) do not match format length(s)\nProcessing will continue without format conversion\n");
			err_disp();
		}
	}
	
	input_complete = output_complete = 0;
	total_bytes_read = total_bytes_written = 0;
	input_file_size = os_filelength(idbin->file_name);
	
	fprintf(stderr, "\n");
  while (!input_complete)
  {
		if (output_complete)
		{
			err_push(ROUTINE_NAME, ERR_UNKNOWN, "\ncache-to-file completed before file-to-cache");
			err_disp();
		}

		bytes_read = ndarr_reorient(file_to_cache_mapping,
		             NDARRS_FILE, argv[1], embedded_header_length,
		             NDARRS_BUFFER, idbin->cache, idbin->cache_size,
		             &input_complete);
		
		if (bytes_read <= 0)
		{
			err_push(ROUTINE_NAME, ERR_READ_FILE, "file to cache reorientation");
			err_disp();
			exit(EXIT_FAILURE);
		}
		
		total_bytes_read += bytes_read;
	
		if (do_convert_cache)
		{
			idbin->records_in_cache = bytes_read / (long)FORMAT_LENGTH(idbin->input_format);
			idbin->cache_end = idbin->cache + (bytes_read - 1);
			
			if ((error = db_events(idbin, DBIN_CONVERT_CACHE, odbin->cache,
			 (FORMAT_PTR)NULL, &bytes_written, END_ARGS)) != 0)
			{
				err_push(ROUTINE_NAME, error, "Error in filling data bin cache");
				err_disp();
				exit(EXIT_FAILURE);
			}
			bytes_written = ndarr_reorient(cache_to_file_mapping,
			                 NDARRS_BUFFER, odbin->cache,
			                 (bytes_read / (long)FORMAT_LENGTH(idbin->input_format)) *
			                  FORMAT_LENGTH(idbin->output_format),
			                 NDARRS_FILE | NDARRS_CREATE, argv[argc - 1], embedded_header_length,
			                 &output_complete);
		}
		else
			bytes_written = ndarr_reorient(cache_to_file_mapping,
			                 NDARRS_BUFFER, idbin->cache, bytes_read,
			                 NDARRS_FILE | NDARRS_CREATE, argv[argc - 1], embedded_header_length,
			                 &output_complete);
		
		if (bytes_written <= 0)
		{
			err_push(ROUTINE_NAME, ERR_READ_FILE, "cache to file flushing");
			err_disp();
			exit(EXIT_FAILURE);
		}
	
		total_bytes_written += bytes_written;
	
		fprintf(stderr, "\r%4d%% processed", (int)(((float)total_bytes_read / input_file_size) * 100));
  } /* while data to process */
  
  fprintf(stderr, "\n\nTotal elements read:\t%ld\nTotal elements written:\t%ld\n",
          total_bytes_read / (do_convert_cache ?
          (long)FORMAT_LENGTH(idbin->input_format) : super_array_descriptor->element_size),
          total_bytes_written / (do_convert_cache ?
          (long)FORMAT_LENGTH(idbin->output_format) : sub_array_descriptor->element_size));
  
  if (!output_complete)
  {
  	err_push(ROUTINE_NAME, ERR_UNKNOWN, "\nfile-to-cache completed before cache-to_file");
  	err_disp();
  }
  
	return(EXIT_SUCCESS);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "get_descriptor_strings_from_eqv"

int get_descriptor_strings_from_eqv(DATA_BIN_PTR dbin, char *super_descriptor,
    char *sub_descriptor)
{
	assert(super_descriptor);
	assert(sub_descriptor);

	if (nt_askvalue(dbin, INPUT_ARRAY_DESCRIPTOR_KEYWORD, FFV_CHAR,
	                super_descriptor, NULL) &&
	    nt_askvalue(dbin, OUTPUT_ARRAY_DESCRIPTOR_KEYWORD, FFV_CHAR,
	                sub_descriptor, NULL))
		return(2);
	else
		return(0);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "make_descriptor_string_from_header"

int make_descriptor_string_from_header(DATA_BIN_PTR dbin, char *descriptor)
/*****************************************************************************
 * NAME:  make_descriptor_string_from_header()
 *
 * PURPOSE:  Build a two-dim array descriptor string for N-dim array library
 *
 * USAGE:  error = make_descriptor_string_from_header(data_bin, array_descriptor_string);
 *
 * RETURNS:  Zero on success and descriptor points to an allocated string
 * containing the array description, else error code defined in err.h
 *
 * DESCRIPTION:  The array descriptor string can be directly defined by the
 * user in a header or a name constant section of a name table.  Otherwise,
 * the array descriptor string must be constructed based on certain GeoVu
 * keywords.  In this latter case the array descriptor string is necessarily
 * a two dimensional description; the current set of GeoVu keywords can only
 * describe grids.
 *
 * Furthermore, GeoVu keywords can describe grids in one of two ways:
 * 1) specifying the start and ending columns and rows, or 2) specifying the
 * total number of columns and rows, and left and right map X's and Y's.  In
 * this latter case the indices used in the array descriptor subscripts will
 * range from 0 to the number of columns and rows.  The left and right map X's
 * and Y's are used to determine increasing or decreasing indices.
 *
 * This function presumes that no fancy reorientation is going on.  This means
 * that a single file contains a single array whose cells constitute single
 * whole records.  Format conversion is allowed, but no queries or conversions.
 *
 * descriptor must be an allocated string.
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
 *
 * Error return codes:  ERR_MEM_LACK, ERR_MISSING_VAR
 ****************************************************************************/

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "make_descriptor_string_from_header"
{
	C_END_COLUMN_KEYWORD_TYPE end_col;
	C_END_ROW_KEYWORD_TYPE end_row;
	C_START_X_KEYWORD_TYPE start_x;
	C_END_Y_KEYWORD_TYPE end_y;
	C_NUM_COLUMNS_KEYWORD_TYPE num_cols;
	C_NUM_ROWS_KEYWORD_TYPE num_rows;
	C_END_X_KEYWORD_TYPE end_x;
	C_START_COLUMN_KEYWORD_TYPE start_col;
	C_START_ROW_KEYWORD_TYPE start_row;
	C_START_Y_KEYWORD_TYPE start_y;
	
	BOOLEAN method_1 = TRUE; /* start/end cols/rows */
	BOOLEAN method_2 = TRUE; /* count cols/rows, left/right map X/Y */

  assert(descriptor);

	method_1 = method_1 && nt_askvalue(dbin, START_COLUMN_KEYWORD,
	 START_COLUMN_KEYWORD_TYPE, (void *)&start_col, NULL);

	method_1 = method_1 && nt_askvalue(dbin, END_COLUMN_KEYWORD,
	 END_COLUMN_KEYWORD_TYPE, (void *)&end_col, NULL);

	method_1 = method_1 && nt_askvalue(dbin, START_ROW_KEYWORD,
	 START_ROW_KEYWORD_TYPE, (void *)&start_row, NULL);

	method_1 = method_1 && nt_askvalue(dbin, END_ROW_KEYWORD,
	 END_ROW_KEYWORD_TYPE, (void *)&end_row, NULL);

	method_2 = method_2 && nt_askvalue(dbin, NUM_COLUMNS_KEYWORD,
	 NUM_COLUMNS_KEYWORD_TYPE, (void *)&num_cols, NULL);
		
	method_2 = method_2 && nt_askvalue(dbin, NUM_ROWS_KEYWORD,
	 NUM_ROWS_KEYWORD_TYPE, (void *)&num_rows, NULL);

	method_2 = method_2 && nt_askvalue(dbin, START_X_KEYWORD,
	 START_X_KEYWORD_TYPE, (void *)&start_x, NULL);
		
	method_2 = method_2 && nt_askvalue(dbin, END_X_KEYWORD, END_X_KEYWORD_TYPE,
	 (void *)&end_x, NULL);
		
	method_2 = method_2 && nt_askvalue(dbin, START_Y_KEYWORD,
	 START_Y_KEYWORD_TYPE, (void *)&start_y, NULL);
		
	method_2 = method_2 && nt_askvalue(dbin, END_Y_KEYWORD, END_Y_KEYWORD_TYPE,
	 (void *)&end_y, NULL);

	if (method_2 && !method_1)
	{
		if (start_x < end_x)
		{
			start_col = 0;
			end_col = num_cols - 1;
		}
		else
		{
			start_col = num_cols - 1;
			end_col = 0;
		}
		if (start_y > start_y) /* reversed for latitude */
		{
			start_row = 0;
			end_row = num_rows - 1;
		}
		else
		{
			start_row = num_rows - 1;
			end_row = 0;
		}
	}
	else if (!method_1 && !method_2)
		return(err_push(ROUTINE_NAME, ERR_MISSING_VAR, "\ninsufficient information in header to construct array description"));

	sprintf(descriptor, "%s [\"%s\" %d to %d][\"%s\" %d to %d] %d",
	 DYNAMICALLY_GENERATED_ARRAY_NAME, DYNAMICALLY_GENERATED_ROWS_NAME,
	 (int)start_row, (int)end_row, DYNAMICALLY_GENERATED_COLUMNS_NAME,
	 (int)start_col, (int)end_col, (int)FORMAT_LENGTH(dbin->input_format));

	return(0);		
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "check_grid_origin"

int check_grid_origin(DATA_BIN_PTR dbin, ARRAY_DESCRIPTOR_PTR super_array_descriptor,
    char *sub_descriptor_string)
/*****************************************************************************
 * NAME:  check_grid_origin()
 *
 * PURPOSE:  Construct sub-array descriptor string given GeoVu keyword
 * grid_origin and super-array descriptor string
 *
 * USAGE:  error = check_grid_origin(dbin, super_array_descriptor, sub_descriptor_string);
 *
 * RETURNS:  Zero on success, sub_array_descriptor points to an allocated
 * string containing the array descriptor string,  else an error code defined
 * in err.h
 *
 * DESCRIPTION:  Asks the (input) name table for the grid_origin keyword in
 * either of its char (string) or short (numeric) values.  If found, then
 * rearrange the dimensions of the super-array descriptor string accordingly.
 * The super-array descriptor string must have been constructed by 
 * make_descriptor_string_from_header().  This implies that it is appropriate
 * for the user to use the grid_origin keyword ONLY in conjunction with a
 * predefined image header.  Furthermore, should the user define a FreeForm
 * array descriptor string for either the super-array or sub-array, then the
 * user should take the responsibility for defining BOTH.
 *
 * The grid origin of the input array, within the context of this function,
 * is ALWAYS take as upperleft.  The grid_origin keyword specifies where the
 * output array's origin is relative to the input array.  The eight possible
 * transformations are given below.
 *
 * upperleft:
 * DO NOTHING!!!
 * lowerleft:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["rows" y2 to y1]["cols" x1 to x2]
 * lowerright:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["rows" y2 to y1]["cols" x2 to x1]
 * upperright:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["rows" y1 to y2]["cols" x2 to x1]
 * upperleft_y:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["cols" x1 to x2]["rows" y1 to y2]
 * lowerleft_y:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["cols" x1 to x2]["rows" y2 to y1]
 * lowerright_y:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["cols" x2 to x1]["rows" y2 to y1]
 * upperright_y:
 * ["rows" y1 to y2]["cols" x1 to x2] becomes ["cols" x2 to x1]["rows" y1 to y2]
 *
 * NOTE that since the grid_origin keywords determines the output array
 * descriptor string based on an input array descriptor string that itself
 * was dynamically created from a header, granularity, separation, and
 * grouping are NOT supported.  The user must write explicit FreeForm array
 * descriptor strings if these features are desired.
 * 
 * sub_descriptor_string must be an allocated string.
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
 *
 * Error return codes:  ERR_MEM_LACK, ERR_PARAM_VALUE
 ****************************************************************************/

{
	char cgrid_origin[MAX_NAME_LENGTH];
	C_GRID_ORIGIN_SHORT_TYPE grid_origin;
	
	if (!nt_askvalue(dbin, GRID_ORIGIN, FFV_CHAR, cgrid_origin, NULL) &&
	 !nt_askvalue(dbin, GRID_ORIGIN, GRID_ORIGIN_SHORT_TYPE, (void *)&grid_origin,
	 NULL))
		return(1);
	
	if (cgrid_origin[0] != STR_END)
	{
		if (!os_strcmpi(cgrid_origin, "upperleft"))
			grid_origin = 1;
		else if (!os_strcmpi(cgrid_origin, "lowerleft"))
			grid_origin = 2;
		else if (!os_strcmpi(cgrid_origin, "lowerright"))
			grid_origin = 3;
		else if (!os_strcmpi(cgrid_origin, "upperright"))
			grid_origin = 4;
		else if (!os_strcmpi(cgrid_origin, "upperleft_y"))
			grid_origin = 5;
		else if (!os_strcmpi(cgrid_origin, "lowerleft_y"))
			grid_origin = 6;
		else if (!os_strcmpi(cgrid_origin, "lowerright_y"))
			grid_origin = 7;
		else if (!os_strcmpi(cgrid_origin, "upperright_y"))
			grid_origin = 8;
		else
			grid_origin = 0;
	}

	/* overwrite temp_descriptor with NULL's for brackets for parsing */

	switch (grid_origin)
	{
		case 1:
			return(err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "grid_origin keyword indicates no reorientation"));
		break;
		
		case 2:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->end_index[0],
			 super_array_descriptor->start_index[0],
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->start_index[1],
			 super_array_descriptor->end_index[1]);
		break;

		case 3:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->end_index[0],
			 super_array_descriptor->start_index[0],
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->end_index[1],
			 super_array_descriptor->start_index[1]);
		break;

		case 4:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->start_index[0],
			 super_array_descriptor->end_index[0],
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->end_index[1],
			 super_array_descriptor->start_index[1]);
		break;
		
		case 5:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->start_index[1],
			 super_array_descriptor->end_index[1],
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->start_index[0],
			 super_array_descriptor->end_index[0]);
		break;

		case 6:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->start_index[1],
			 super_array_descriptor->end_index[1],
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->end_index[0],
			 super_array_descriptor->start_index[0]);
		break;

		case 7:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->end_index[1],
			 super_array_descriptor->start_index[1],
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->end_index[0],
			 super_array_descriptor->start_index[0]);
		break;

		case 8:
			sprintf(sub_descriptor_string, "%s [\"%s\" %ld to %ld] [\"%s\" %ld to %ld]",
			 DYNAMICALLY_GENERATED_ARRAY_NAME,
			 super_array_descriptor->dim_name[1],
			 super_array_descriptor->end_index[1],
			 super_array_descriptor->start_index[1],
			 super_array_descriptor->dim_name[0],
			 super_array_descriptor->start_index[0],
			 super_array_descriptor->end_index[0]);
		break;

		default:
			return(err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "grid_origin keyword"));
	} /* end of switch grid_origin */

	sprintf(sub_descriptor_string + strlen(sub_descriptor_string), " %d",
	 (int)FORMAT_LENGTH(dbin->output_format));

	return(0);
}
 
#undef ROUTINE_NAME
#define ROUTINE_NAME "get_output_name_table"

int get_output_name_table(char *file_name, FFF_STD_ARGS *std_args, DATA_BIN_PTR idbin,
                          DATA_BIN_PTR odbin)
/*****************************************************************************
 * NAME:  get_output_name_table()
 *
 * PURPOSE:  Make the output name table
 *
 * USAGE:  error = get_output_name_table(file_name, input_dbin, output_dbin);
 *
 * RETURNS:  Zero on success and output_dbin points to a DATA_BIN containing the
 * output name_table/header, else an error code defined in err.h
 *
 * DESCRIPTION:  STUBBED to return failure
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
 *
 * Error return codes:  ERR_MAKE_DBIN, ERR_SET_DBIN
 ****************************************************************************/

#undef ROUTINE_NAME
#define ROUTINE_NAME "get_output_name_table"

{
	
	assert(odbin && idbin);

	return(1);

	if (db_set(odbin,
	 DBIN_FILE_NAME, file_name,
	 END_ARGS))
		return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "file name"));

	if (db_set(odbin,
	 MAKE_NAME_TABLE, (char *)NULL,
	 END_ARGS))
		return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "name table"));
	
	/* FORMAT_LIST event so as to set header */
	if (db_set(odbin,
	 FORMAT_LIST, (char *)NULL, (char *)NULL,
	 END_ARGS))
	{ /* output file does not have its own formats -- use input file's */
		if (db_set(odbin,
		 DBIN_FILE_NAME, idbin->file_name,
		 END_ARGS))
			return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "file name"));

		if (db_set(odbin,
		 INPUT_FORMAT, std_args->input_format_file, (char *)NULL,
		 OUTPUT_FORMAT, std_args->output_format_file, (char *)NULL,
		 END_ARGS))
			return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "formats"));
	
		if (db_set(odbin,
		 DBIN_FILE_NAME, file_name,
		 END_ARGS))
			return(err_push(ROUTINE_NAME, ERR_SET_DBIN, "file name"));

	}

	return(0);
}
void command_line_usage_help(void)
/*****************************************************************************
 * NAME:  command_line_usage_help()
 *
 * PURPOSE:  Display help for command line usage
 *
 * USAGE:  command_line_usage_help();
 *
 * RETURNS:  void
 *
 * DESCRIPTION:
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * SYSTEM DEPENDENT FUNCTIONS:  NONE
 *
 * GLOBALS:  COMMAND_LINE_USAGE[]
 *
 * COMMENTS:
 *
 * KEYWORDS:
 *
 * ERRORS:
 ****************************************************************************/

{
	int i = 0;
		
	fprintf(stderr, "%s\n", greeting);
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "make_interim1_descriptor_string"

int make_interim_descriptors(ARRAY_DESCRIPTOR_PTR super_descriptor,
    ARRAY_DESCRIPTOR_PTR sub_descriptor,
    ARRAY_DESCRIPTOR_PTR *interim1_descriptor,
    ARRAY_DESCRIPTOR_PTR *interim2_descriptor)
/*****************************************************************************
 * NAME:  make_interim_descriptors()
 *
 * PURPOSE:  Make first intermediate array descriptor 
 *
 * USAGE: error = make_interim_descriptors(super_descriptor,
 *                sub_descriptor, &interim1_descriptor, &interim2_descriptor);
 *
 * RETURNS:  Zero on success, an error code on failure
 *                                                                   
 * DESCRIPTION:  Copies sub_descriptor to interim1_descriptor,
 * stripping off separation and grouping for all subscripts and changing
 * the element size to that of super_descriptor.
 * 
 * Copies sub_descriptor to interim2_descriptor, stripping off
 * separation and grouping for all subscripts and changing the element
 * size to that of sub_descriptor.
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
	int i;
	char interim1_descriptor_string[2 * MAX_NAME_LENGTH] = {""},
	     interim2_descriptor_string[2 * MAX_NAME_LENGTH] = {""};

	
	assert(super_descriptor);
	assert(interim1_descriptor);
	assert(interim2_descriptor);
	assert(sub_descriptor);
	
	for (i = 0; i < sub_descriptor->num_dim; i++)
	{
		sprintf(interim1_descriptor_string + strlen(interim1_descriptor_string),
		        "[\"%s\" %ld to %ld by %ld] ",
		        sub_descriptor->dim_name[i], sub_descriptor->start_index[i],
		        sub_descriptor->end_index[i], sub_descriptor->granularity[i]);

		sprintf(interim2_descriptor_string + strlen(interim2_descriptor_string),
		        "[\"%s\" %ld to %ld by %ld] ",
		        sub_descriptor->dim_name[i], sub_descriptor->start_index[i],
		        sub_descriptor->end_index[i], sub_descriptor->granularity[i]);
	}

	sprintf(interim1_descriptor_string + strlen(interim1_descriptor_string),
	        " %u", super_descriptor->element_size);

	sprintf(interim2_descriptor_string + strlen(interim2_descriptor_string),
	        " %u", sub_descriptor->element_size);

	if (!(*interim1_descriptor = ndarr_create_from_str(interim1_descriptor_string)))
	{
		err_push(ROUTINE_NAME, ERR_SYNTAX, "First transitional descriptor string");
		return(ERR_SYNTAX);
	}
	if (!(*interim2_descriptor = ndarr_create_from_str(interim2_descriptor_string)))
	{
		err_push(ROUTINE_NAME, ERR_SYNTAX, "Second transitional descriptor string");
		return(ERR_SYNTAX);
	}

	return(0);
}

