/* FILENAME:  proclist.c
 *
 * CONTAINS:  ff_process_variable_list()
 *            (MODULE) cv_find_convert_var()
 *            (MODULE) ff_make_tmp_format()
 *            (MODULE) ff_bin2bin()
 */


/*
 * NAME:        ff_process_variable_list() 
 *                      
 * PURPOSE:     Convert a buffer from one format to another.
 *
 * USAGE:       ff_process_variable_list(
 *                      char   *input_buffer,
 *                      char   *output_buffer,
 *                      FORMAT *input_format,
 *                      FORMAT *output_format
 *                      long num_bytes)
 *                      
 * RETURNS: bytes still remaining after processing      
 *                      -1 on Error.
 *           
 * DESCRIPTION: This function takes input and output format specifications,
 *                              reads the input data buffer according to the input list
 *                              and writes the output buffer according to the output list.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *              
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 * 
 * ERRORS:
 *              Pointer not defined,((!input_format->variables) ? "Input_format->variable
 *              Record Length or CR Problem, "In Input Data Format"
 *              Record Length or CR Problem, variable_str
 *              Out of memory, "Output Initialization String"
 *              System File Error,"Initialization String"
 *              System File Error,"initial_file"
 *              Unknown variable type,"binary var->type"
 *              Problem in conversion,variable_str
 *              Problem in conversion,(strcat("bin2bin: ",out_var->name))
 *
 * COMMENTS:    
 *                      This function now processes a certain number of bytes until done
 *                      EOL's are skipped while processing the input and added to output
 *                      It is assumed:
 *                              input_buffer begins with the start of a format
 *                              num_bytes in the cache has been calculated correctly
 *                              the pointers to input and output buffers are set correctly
 *      
 *                      A negative num_bytes indicates that processing is to start at end
 *                      of input_buffer. This use to be handled in CONVERT_CACHE so that
 *                      the cache can process onto itself. THIS NEEDS TESTING.
 *
 * KEYWORDS:
 *      
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define WANT_NCSA_TYPES
#include <freeform.h>
#include <os_utils.h>

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif

#ifdef CCMSC
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif

#ifdef SUNCC
#include <errno.h>
#endif

/* REH: The NUMBER_OF_CONVERT_FUNCTIONS must be changed if
	convert functions are added or deleted!! */
#define NUMBER_OF_CONVERT_FUNCTIONS 105

static const CONVERT_VARIABLE convert_functions[NUMBER_OF_CONVERT_FUNCTIONS] = {
	{"*",                   cv_units},
	{"longitude",           cv_deg},
	{"longitude",           cv_deg_nsew},
	{"longitude",           cv_lon_east},
	{"longitude",           cv_noaa_eq},
	{"longitude_deg",       cv_degabs_nsew},
	{"longitude_deg",       cv_dms},
	{"longitude_min",       cv_dms},
	{"longitude_sec",       cv_dms},
	{"longitude_abs",       cv_abs},

	{"longitude_abs",       cv_deg_abs},
	{"longitude_deg_abs",   cv_abs},
	{"longitude_deg_abs",   cv_degabs},
	{"longitude_min_abs",   cv_degabs},
	{"longitude_sec_abs",   cv_degabs},
	{"longitude_ew",        cv_nsew},
	{"longitude_ew",        cv_geog_sign},
	{"longitude_sign",      cv_abs},
	{"longitude_sign",      cv_geog_sign},
	{"longitude_east",      cv_lon_east},

	{"latitude",            cv_deg},
	{"latitude",            cv_deg_nsew},
	{"latitude",            cv_noaa_eq},
	{"latitude_abs",        cv_abs},
	{"latitude_abs",        cv_deg_abs},
	{"latitude_deg_abs",    cv_abs},
	{"latitude_deg_abs",    cv_degabs},
	{"latitude_min_abs",    cv_degabs},
	{"latitude_sec_abs",    cv_degabs},
	{"latitude_ns",         cv_geog_sign},

	{"latitude_ns",         cv_nsew},
	{"latitude_deg",        cv_degabs_nsew},
	{"latitude_deg",        cv_dms},
	{"latitude_min",        cv_dms},
	{"latitude_sec",        cv_dms},
	{"latitude_sign",       cv_abs},
	{"latitude_sign",       cv_geog_sign},
	{"mb",                  cv_long2mag},
	{"ms1",                 cv_long2mag},
	{"ms2",                 cv_long2mag},

	{"mb-maxlike",          cv_long2mag},
	{"magnitude_mb",        cv_long2mag},
	{"magnitude_mb",        cv_noaa_eq},
	{"magnitude_ms",        cv_long2mag},
	{"magnitude_ms1",       cv_long2mag},
	{"magnitude_ms2",       cv_long2mag},
	{"magnitude_ms",        cv_noaa_eq},
	{"magnitude_mo",        cv_noaa_eq},
	{"magnitude_ml",        cv_noaa_eq},
	{"magnitude_ml",        cv_long2mag},

	{"magnitude_local",     cv_long2mag},
	{"serial",              cv_ymd2ser},
	{"serial_day_1980",     cv_ymd2ser},
	{"serial",              cv_ipe2ser},
	{"serial_day_1980",     cv_ipe2ser},
	{"ipe_date",            cv_ymd2ipe},
	{"ipe_date",            cv_ser2ipe},
	{"century",             cv_ser2ymd},
	{"century",             cv_ydec2ymd},
	{"century_and_year",	  cv_ser2ymd},

	{"century_and_year",	  cv_ydec2ymd},
	{"year",                cv_ser2ymd},
	{"year",                cv_ydec2ymd},
	{"year",                cv_noaa_eq},
	{"year_decimal",  		cv_ymd2ser},
	{"month",               cv_ser2ymd},
	{"month",               cv_ydec2ymd},
	{"month",               cv_noaa_eq},
	{"day",                 cv_ser2ymd},
	{"day",                 cv_ydec2ymd},

	{"day",                 cv_noaa_eq},
	{"hour",                cv_ser2ymd},
	{"hour",                cv_ydec2ymd},
	{"hour",                cv_noaa_eq},
	{"minute",              cv_ser2ymd},
	{"minute",              cv_ydec2ymd},
	{"minute",              cv_noaa_eq},
	{"second",              cv_ser2ymd},
	{"second",              cv_ydec2ymd},
	{"second",              cv_noaa_eq},

	{"date_yymmdd",         cv_date_string},
/* date_m/d/y must be phased out -- it is "not right" */	
	{"date_m/d/y",          cv_date_string},
	{"date_mm/dd/yy",          cv_date_string}, /* date_mm/dd/yy replaces date_m/d/y */
	{"time_hhmmss",         cv_time_string},
	{"time_h:m:s",          cv_time_string},
	{"longmag",             cv_mag2long},
	{"cultural",            cv_sea_flags},
	{"cultural",            cv_noaa_eq},
	{"ngdc_flags",          cv_sea_flags},
	{"depth_control",       cv_sea_flags},

	{"depth",               cv_noaa_eq},
	{"WMO_quad_code",       cv_geog_quad},  
	{"geog_quad_code",      cv_geog_quad},
	{"time_offset",         cv_geo44tim},
	{"cultural",            cv_slu_flags},
	{"non_tectonic",        cv_slu_flags},
	{"magnitude_ml",        cv_slu_flags},
	{"scale",               cv_slu_flags},
	{"ml_authority",        cv_slu_flags},
	{"intensity",           cv_slu_flags},

	{"intensity",           cv_noaa_eq},
	{"fe_region",           cv_noaa_eq},
	{"no_station",          cv_noaa_eq},
	{"source_code",         cv_noaa_eq},
	{"zh_component",        cv_noaa_eq},
};

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "ff_process_variable_list"

#ifdef PROTO

static FORMAT_PTR ff_make_tmp_format(FORMAT_PTR input_format,
	FORMAT_PTR output_format,
	FF_DATA_BUFFER input_buffer);

static int cv_find_convert_var(VARIABLE_PTR var,	/* Description of desired variable */
	FORMAT_PTR input_format,			/* The Input Format Description */
	FF_DATA_BUFFER input_buffer);				/* The input buffer */

static int ff_bin2bin(VARIABLE_PTR in_var, FF_DATA_BUFFER input_buffer,
          VARIABLE_PTR out_var, FF_DATA_BUFFER output_buffer);
          
int ff_process_variable_list(FF_DATA_BUFFER input_buffer,
	FF_DATA_BUFFER output_buffer,
	FORMAT_PTR input_format,
	FORMAT_PTR output_format,
	long *bytes)

#else

static FORMAT_PTR ff_make_tmp_format();
static int cv_find_convert_var();
static int ff_bin2bin();

int ff_process_variable_list(input_buffer,
	output_buffer,
	input_format,
	output_format,
	bytes)
FF_DATA_BUFFER input_buffer;
FF_DATA_BUFFER output_buffer;
FORMAT_PTR input_format;
FORMAT_PTR output_format;
long *bytes;

#endif
          
{
#ifdef CCMSC
char eol_string[] = {'\015', '\012', STR_END};
#endif

#ifdef CCLSC
char eol_string[] = {'\015', STR_END};
#endif

#ifdef SUNCC
char eol_string[] = {'\012', STR_END};
#endif


	VARIABLE_PTR in_var     = NULL;
	VARIABLE_PTR out_var    = NULL;
	VARIABLE_LIST_PTR in_v_list     = NULL;
	VARIABLE_LIST_PTR out_v_list    = NULL;

	CONVERT_VARIABLE *conv_func = NULL;

	/* Process variable list maintains several static variables in order
		to speed up subsequent processing */
	static char *initial_record = NULL;
	static FORMAT_PTR tmp_format = NULL;
	static VARIABLE_PTR     conv_var_ptr = NULL;
	static VARIABLE         conv_var;
	static FORMAT_PTR last_input  = NULL;
	static FORMAT_PTR last_output = NULL;

	short make_tmp_format_flag = 0;

	int num_variables = 0;
	int initial_file;

	unsigned input_increment        = 0;
	unsigned output_increment       = 0;
	unsigned variable_length = 0;
	unsigned bytes_to_write  = 0;

	double  double_value    = 0;
	double  cv_double       = 0; /* convert variable, double will hold 8 char's */

	signed char    direction               = 1;
	char    variable_str[256];

	char    *cp                             = NULL;

	unsigned char conversion = 0;

	FF_DATA_PTR data_ptr            = NULL; /* for the conversion/input_buffer */
	char *input_ptr                 = NULL; /* increments along input_buffer */
	char *output_ptr                = NULL; /* increments along output_buffer */
	long num_bytes;

	char error_buffer[_MAX_PATH];

/*	short decimal_shift;*/

	/* Error checking on NULL parameters and lists */
	assert(input_buffer != NULL && output_buffer != NULL);
	assert(input_format != NULL && output_format != NULL);
	assert(input_format == input_format->check_address);
	assert(output_format == output_format->check_address);

	if((!input_format->variables) || (!output_format->variables)) {
		err_push(ROUTINE_NAME,ERR_PTR_DEF,
		(char *) ((!input_format->variables) ? "Input_format->variables" : "Output_format->variables"));
		return(-1);
	}
	  
	num_bytes = *bytes;

	/* if num_bytes is negative, we are moving from rear of input_buffer towards front */
	if(num_bytes < 0){
		direction = -1;
		num_bytes *= direction;
	}
		
	/* Check for EOL. */
	 
	if(IS_ASCII(input_format)){
		if(*(input_buffer + input_format->max_length) != EOL_1STCHAR){
			err_push(ROUTINE_NAME,ERR_UNEXPECTED_CR, "In Input Data Format");
			return(-1);
		}
	}

	/* Determine whether new temporary format is needed */
	if((last_input != input_format) || (last_output != output_format)) {
		make_tmp_format_flag = 1;
		last_input = input_format;
		last_output = output_format;
		if (initial_record)
			memFree(initial_record, "ff_process_variable_list: Initial Record");
		initial_record = NULL;
	}

	/* EOL's will be added to output and skipped in input */
	output_increment = FORMAT_LENGTH(output_format);
	input_increment  = input_format->max_length;

	/* Do things that only happen once:
			check existence of EOL constants
			create the static input format
			initialize initial_record
	*/
	if(!initial_record){

		/* FREEFORM allows end of lines to be defined as constant variables
			with the name "EOL". We check these in the input record to
			making sure the format anf the data match.       */

		if(IS_ASCII(input_format)){
			in_v_list = FFV_FIRST_VARIABLE(input_format);

			while(dll_data(in_v_list) ){
				in_var = FFV_VARIABLE(in_v_list);
				if(IS_CONSTANT (in_var) && memStrcmp("EOL", in_var->name, "ff_process_variable_list: looking for EOL") == 0){
					if(*(input_buffer + in_var->start_pos - 1) != EOL_1STCHAR){
						sprintf(variable_str, "No CR at %u", in_var->start_pos);
						err_push(ROUTINE_NAME,ERR_UNEXPECTED_CR, variable_str);
						return(-1);
					}
				}
				in_v_list = dll_next(in_v_list);
			}
		}/* end EOL check */
		
		/* Create a temporary format to use in the processing. This format
			has the variables needed for the output format and the convert
			variables added. It is created to avoid searching the input
			format for each variable. */

		if(make_tmp_format_flag){
			if(tmp_format)
			{
				ff_free_format(tmp_format);
				tmp_format = NULL;
			}
			if((tmp_format = ff_make_tmp_format(input_format, output_format, input_buffer)) == NULL)
			{
				err_push(ROUTINE_NAME, ERR_GENERAL, "Creating temporary format");
				return(-1);
			}
		}
		else {
			assert(tmp_format == tmp_format->check_address);
		}
	
		/* Allocate an initialization string */
		initial_record = (char *)memMalloc(output_increment, "ff_process_variable_list: initialization record");
		if(!initial_record){
			err_push(ROUTINE_NAME,ERR_MEM_LACK, "Output Initialization String");
			return(-1);
		}
		if(IS_BINARY(output_format))
			memMemset(initial_record, STR_END, output_increment,"initial_record,'\\0',output_increment" );
		else
			memMemset(initial_record, ' ', output_increment,"initial_record,' ',output_increment");
	
	
		/* If an FFV_INITIAL type variable exists in the output format, a file
		with the initialization string is copied to the initialization buffer */

		out_v_list = FFV_FIRST_VARIABLE(output_format);
		while( dll_data(out_v_list) ){
			out_var = FFV_VARIABLE(out_v_list);
			if(IS_INITIAL(out_var)){
				cp = os_get_env("DATA_PATH");
				if(cp){
					memStrcpy(variable_str, cp,"variable_str, os_get_env(\"DATA_PATH\")");
					memStrcat(variable_str, out_var->name,"variable_str, os_get_env(\"DATA_PATH\")");
				}
				else memStrcpy(variable_str, out_var->name,"variable_str, out_var->name");
	
				if ((initial_file = open(variable_str, O_RDONLY | O_BINARY)) == -1) {
					err_push(ROUTINE_NAME,errno,"Initialization String");
					return(-1);
				}
				if ((memRead(initial_file, initial_record, output_format->max_length,"initial_file,initial_record,output_format->max_length")) == -1) {
					err_push(ROUTINE_NAME,errno,"initial_file");
					return(-1);
				}
				break;
			}
			out_v_list = dll_next(out_v_list);
	
		}/* end initial variable search */
	
		/* Search the output format for constants */
		out_v_list = FFV_FIRST_VARIABLE(output_format);
		while( dll_data(out_v_list) ){
			out_var = FFV_VARIABLE(out_v_list);
			if(IS_CONSTANT(out_var))                /* Constant found */
				if(memStrcmp(out_var->name, "EOL", "ff_process_variable_list: Looking for EOL") == 0){
					switch(EOL_SPACE){
					case 1:
						*(initial_record + out_var->end_pos - 1) = EOL_1STCHAR;
						break;
					case 2:
						*(initial_record + out_var->end_pos - 1) = '\012';
						*(initial_record + out_var->end_pos - 2) = EOL_1STCHAR;
						break;
					}
				}
				else            
				(void) memMemcpy(initial_record + out_var->start_pos - 1,
					out_var->name, out_var->end_pos - out_var->start_pos + 1,"initial_record + ...,out_var->name");

			out_v_list = dll_next(out_v_list);
		}/* end search for constants */

	}/* end one-time initialization */

	input_ptr = input_buffer;
	output_ptr = output_buffer;


	/********** MAIN LOOP ON INPUT DATA  - process num_bytes ***************/
	while (num_bytes >= (long)FORMAT_LENGTH(input_format) )
	{
		/* Initialize variables for this loop */
		num_variables = 0;
		in_v_list       = FFV_FIRST_VARIABLE(tmp_format);
		out_v_list      = FFV_FIRST_VARIABLE(output_format);
	
		/* initialize the  record */
		memMemcpy(output_ptr, initial_record, output_increment,"output_ptr, initial_record");
	
		/* Initialize the generic conversion variable */
		conv_var_ptr = &conv_var;
		conv_var_ptr->check_address = (void*)conv_var_ptr;
		conv_var_ptr->start_pos = 1;
		conv_var_ptr->end_pos   = SIZE_FLOAT64;
		conv_var_ptr->type      = FFV_FLOAT64;
		conv_var_ptr->precision = 6;
		memStrcpy(conv_var_ptr->name, "generic convert variable","conv_var_ptr->name,\"generic convert variable\"" );

		/***** MAIN LOOP ON OUTPUT VARIABLE LIST *****/
		while(dll_data(out_v_list))
		{    
			in_var          = FFV_VARIABLE(in_v_list);
			out_var         = FFV_VARIABLE(out_v_list);

			/* Check for possible corrupt memory */
			assert(out_var == out_var->check_address);
			assert(in_var == in_var->check_address);

			/* Skip NULL Variables in the input format. */
			if (in_var->type == FFV_NULL)
			{
				in_v_list   = dll_next(in_v_list);
				out_v_list  = dll_next(out_v_list);
				continue;
			}
	
			++num_variables;
	
			/* Convert variables are processed by creating a double in double_value
			and setting in_var to the generic convert variable. */
			if (IS_CONVERT(in_var))
			{
				conversion = 1;
	
				conv_func = (CONVERT_VARIABLE *)(convert_functions + in_var->start_pos);

				(*conv_func->convert_func)(out_var, &cv_double,
				                           input_format, input_ptr);
	
				data_ptr = (FF_DATA_PTR)&cv_double;
	
				in_var = conv_var_ptr;
	
				/* The conversion variable is a double, so it's precision is really
				arbitrary. We used to set it = to the output precision to force
				correct rounding. On 5/19/94 REH changed this to 8. This was
				because the conversion to integer variables was getting rounded
				up by sprintf and not truncated in the conversion from a double
				to an integer. This didn't work, because the rounding didn't happen
				and the conversion to output precision below truncated, rather than
				rounded.        */
				conv_var_ptr->precision = out_var->precision;
	
				/* We must deal with the special case of a character type conversion
				variable which is buried in a double value. This can happen when one
				is doing a conversion to a character type variable. */
				if(conversion && IS_TEXT(out_var))
				{
					conv_var_ptr->type = FFV_TEXT;
					conv_var_ptr->precision = 0;
				}
			} /* if convert variable */
			else
				data_ptr = input_ptr + in_var->start_pos - 1;

			/* A string with the output precision and decimal point is needed
				for all but FFF_BINARY to FFF_BINARY Conversions */
			if ( !( IS_BINARY(input_format) && IS_BINARY(output_format) ) )
			{
				if (IS_TEXT(in_var))
				{
					memset(variable_str, 0x20, sizeof(variable_str));
					memcpy(variable_str, data_ptr,   conversion 
					                               ? sizeof(double) 
					                               : FF_VAR_LENGTH(in_var)
					      );
					variable_str[  conversion
					             ? FF_VAR_LENGTH(out_var)
					             : FF_VAR_LENGTH(in_var)
					            ] = STR_END;
					/* Right justify convert digits, pad left with spaces */
					if (conversion)
					{
						/* Seek past leading digits (and leading spaces, if any) */
						for (  cp = variable_str + strlen(variable_str) - 1
						     ; *cp == ' ' && (char HUGE *)cp > (char HUGE *)variable_str
						     ; cp--
						    )
						{
							;
						}
						cp++;
						
						if (*cp)
						{
							/* Flush leading digits (and any leading spaces) right */
							memmove(  variable_str + strlen(cp)
							        , variable_str
							        , (char HUGE *)cp - (char HUGE *)variable_str
							       );
							/* Fill vacated positions with spaces */
							memset(  variable_str
							       , 0x20
							       , strlen(cp)
							      );
						}
					}
				}
				else
				{
                                        /* once in the default case is enough */
					static BOOLEAN fuse = FALSE; 

					/* Get a double with the value of the input variable */
					ff_get_double(in_var, data_ptr, &double_value,   conversion
					                                               ? FFF_BINARY_INPUT_DATA
					                                               : input_format->type
					             );

					/* adjust the value to the output precision */
					if (IS_INTEGER(out_var))
					{
						if (out_var->precision)
							double_value *= pow(10.0, out_var->precision);

						double_value = (double)ROUND(double_value);
					}
				
					/* Create the output string */                                  
					switch (FFV_DATA_TYPE(out_var))
					{

					
						case FFV_TEXT:                  /* Binary to ASCII char */
							memMemcpy(variable_str, (char *)&double_value, sizeof(double),"variable_str, (char *)&double_value");
						break;
		
						case FFV_INT8:
							sprintf(variable_str, fft_cnv_flags[FFNT_INT8], (int8)double_value);
						break;
						
						case FFV_UINT8:
							sprintf(variable_str, fft_cnv_flags[FFNT_UINT8], (uint8)double_value);
						break;

						case FFV_INT16:
							sprintf(variable_str, fft_cnv_flags[FFNT_INT16], (int16)double_value);
						break;
						
						case FFV_UINT16:
							sprintf(variable_str, fft_cnv_flags[FFNT_UINT16], (uint16)double_value);
						break;

						case FFV_INT32:
							sprintf(variable_str, fft_cnv_flags[FFNT_INT32], (int32)double_value);
						break;
						
						case FFV_UINT32:
							sprintf(variable_str, fft_cnv_flags[FFNT_UINT32], (uint32)double_value);
						break;

						case FFV_INT64:
							sprintf(variable_str, fft_cnv_flags[FFNT_INT64], (int64)double_value);
						break;
						
						case FFV_UINT64:
							sprintf(variable_str, fft_cnv_flags[FFNT_UINT64], (uint64)double_value);
						break;

						case FFV_FLOAT32:         /* Binary to ASCII float */
							sprintf(variable_str, fft_cnv_flags_prec[FFNT_FLOAT32],
							        (int)(out_var->precision), double_value);
						break;
				
						case FFV_FLOAT64:                /* Binary to ASCII double */
							sprintf(variable_str, fft_cnv_flags_prec[FFNT_FLOAT64],
							        (int)(out_var->precision), double_value);
							break;
				
						default:
							if (!fuse)
							{
								fuse = TRUE;
								
								sprintf(error_buffer, "%d, %s:%d",
								        (int)FFV_DATA_TYPE(out_var), os_path_return_name(__FILE__), __LINE__);
								err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
								err_disp();
								err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE,"binary var->type");
							}
						break;
					} /* End of Variable Type Switch*/
				} /* End of !IS_TEXT variables */
		    bytes_to_write = strlen(variable_str);
		    
				/* Now the output string is ready */
				if (IS_ASCII(output_format) || IS_DBASE(output_format))
				{
	
					/* Check size of output string */
					variable_length = FF_VAR_LENGTH(out_var);
		
					if (bytes_to_write > variable_length)
					{
						memset(variable_str, '*', variable_length);
						bytes_to_write = variable_length;
					}
	
					/* Copy string to output buffer */
					memcpy(output_ptr + (int)(out_var->end_pos - bytes_to_write),
						variable_str, bytes_to_write);
				}
				else if (IS_BINARY(output_format))
				{
					if(ff_string_to_binary(variable_str, out_var->type,
					    output_ptr + out_var->start_pos - 1))
					{
						memStrcat(variable_str," Going to ","variable_str,\" Going to \"");
						memStrcat(variable_str,out_var->name,"variable_str,out_var->name");
						err_push(ROUTINE_NAME,ERR_CONVERT,variable_str);
						return(-1);
					}
				}
			}
	
			else
			{  /* BINARY TO BINARY CONVERSION */

				if (ff_bin2bin(in_var, (char *)conversion ? (char *)data_ptr : (char *)input_ptr,
					out_var, (char *)output_ptr))
				{
					sprintf(error_buffer, "bin2bin: %s", out_var->name);
					err_push(ROUTINE_NAME,ERR_CONVERT, error_buffer);
					return(-1);
				}
			}               /* End of Binary to Binary Conversion */

			in_v_list   = dll_next(in_v_list);
			out_v_list  = dll_next(out_v_list);
			conversion = 0;
			conv_var_ptr->type = FFV_FLOAT64;
			conv_var_ptr->end_pos = SIZE_FLOAT64;
			conv_var_ptr->precision = 6;
	
		}       /***** End of Main loop on variable list *****/

	/* increment pointers and decrement num_bytes for more processing */

	if(IS_ASCII(output_format) )
		memMemcpy(output_ptr + output_format->max_length, eol_string, EOL_SPACE,"output_ptr+...,eol_string");

	output_ptr += output_increment * direction;
	input_ptr += input_increment * direction;
	num_bytes -= input_increment;
	
	/* skip EOL's */
	if(IS_ASCII(input_format) )
		while(IS_EOL(input_ptr)){
			input_ptr += EOL_SPACE;
			num_bytes -= EOL_SPACE * direction;
		}

	}/********** End Main loop processing num_bytes of data **********/

	/* Calculate how many bytes have been processed and assign to bytes,
	return the number of bytes not processed
	abs() can not be used here as it returns an int(INT_MAX = 32,767)
	*/

	*bytes= (long)output_ptr - (long)output_buffer;
	if (*bytes < 0) *bytes *= -1;

	return( (int)num_bytes);
}

/*
 * NAME:	cv_find_convert_var
 *
 * PURPOSE:	This function checks the array of known conversion functions for one
 *			which makes a particular variable. If a conversion is possible,
 *			the variable is added to the input format with type FFV_CONVERT
 *			and the start position set to the index of the function to call.
 *
 * AUTHOR:	T. Habermann (303) 497-6472, haber@mail.ngdc.noaa.gov
 *
 * USAGE:	Used by Proclist (ff_process_variable_list)
 *
 *			cv_find_convert_var(
 *				VARIABLE_LIST_PTR var,		Description of desired variable 
 *				FORMAT_PTR input_format,	The Input Format Description 	
 *				FF_SCRATCH_BUFFER input_buffer)	The input buffer 			  
 *  													 
 * COMMENTS:
 *
 * RETURNS:	-1 if conversion not possible;
 *			otherwise J where J is the appropriate element of
 *			convert_functions[].
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "cv_find_convert_var"

static int cv_find_convert_var(VARIABLE_PTR var,	/* Description of desired variable */
	FORMAT_PTR input_format,			/* The Input Format Description */
	FF_DATA_BUFFER input_buffer)				/* The input buffer */
{
	double double_value = 0;
	int i = 0;

	/* Loop Through the Possible Conversion Functions:
		var_look is a pointer to a CONVERT_VARIABLE, it contains the name
		of a variable and a pointer to a function which can construct that
		variable. convert_functions is an array of CONVERT_VARIABLES which
		is initialized in freeform.h. This array is searched here.
		
		If var_look->name == NULL, the end of the array has been reached. */

	/*	If var->name (the name of the variable we are trying to construct)
		matches convert_functions[i]->name, call the function to see if it can find
		the variables needed to make the conversion. If it can, it returns
		a 1, if it can't it returns 0.
			
		The last element of the convert_functions array has * as the
		variable name and cv_units as the convert function. This function
		handles the default unit conversions. */

	for (i = NUMBER_OF_CONVERT_FUNCTIONS - 1; i; i--)
	{
		if (   !strcmp(var->name, convert_functions[i - 1].name)
		    || convert_functions[i - 1].name[0] == '*'
		   )
		{
			if ((*(convert_functions[i - 1].convert_func))(var, &double_value, input_format, input_buffer))
			{
				/* Successful Conversion Found */
				return(i - 1);
			}
		}
	}

	return(-1);
}

/*
 * NAME:        ff_make_tmp_format
 *              
 * PURPOSE:     To create a temporary format which matches an output format
 *
 * USAGE:       FORMAT_PTR ff_make_tmp_format(FORMAT_PTR input_format,
 *                      FORMAT_PTR output_format,
 *                      FF_DATA_BUFFER input_buffer)
 *
 * RETURNS:     Pointer to new format or NULL on error
 *
 * DESCRIPTION: ff_make_tmp_format loops through the variables in the output
 *                              format and creates a new format with each of the variables
 *                              in it. If the variables exist in the input format, their
 *                              descriptions are simply copied to the temporary format. If
 *                              the variables do not exist in the input format, but can be
 *                              created with a conversion function, a FFV_CONVERT variable
 *                              is set up. If they do not exist, and no conversion function
 *                              is found, or if the output variables are FFV_CONSTANTS's,
 *                              FFV_HEADER's or FFV_INITIAL's a FFV_NULL variable is set up.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * ERRORS:
 *              ,"Temporary Format"
 *              Out of memory,"Temporary Format Variables"
 *              Out of memory,"variable_str"
 *                      ,"out_var"
 *                              Out of memory,"Making Variable: tmp_var"
 *                              Out of memory,"Temporary Variable"
 *                              Out of memory,"Making Temporary Variable"
 *                                      Out of memory,"Making Temporary Variable"
 *                                      Out of memory,"Making Temporary Variable"
 *                      Out of memory,"Temporary Variable"
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "Make Temporary Format"

static FORMAT_PTR ff_make_tmp_format(FORMAT_PTR input_format,
	FORMAT_PTR output_format,
	FF_DATA_BUFFER input_buffer)
{
	char    *variable_str   = NULL;
	int i;
	int null_type_vars = 0;

	FORMAT_PTR                      tmp_format      = NULL;
	VARIABLE_PTR            in_var          = NULL;
	VARIABLE_PTR            out_var         = NULL;
	VARIABLE_PTR            tmp_var         = NULL;
	VARIABLE_LIST_PTR       out_v_list      = NULL;
	VARIABLE_LIST_PTR       tmp_v_list      = NULL;


    assert(((void *)output_format == output_format->check_address) &&
	       ((void *)input_format == input_format->check_address));
	/* First make a FORMAT and copy input format characteristics */
	tmp_format = (FORMAT_PTR)memMalloc(sizeof(FORMAT), "make_tmp_format tmp_format");

	if(!tmp_format){
		err_push(ROUTINE_NAME, ERR_MEM_LACK,"Temporary Format");
		return(NULL);
	}

	memMemcpy((void *)tmp_format, (void *)input_format, sizeof(FORMAT),NO_TAG);

	tmp_format->num_in_list = 0;
	tmp_format->check_address = (void*)tmp_format;
	tmp_format->variables = dll_init();
	if(!tmp_format->variables){
		err_push(ROUTINE_NAME,ERR_MEM_LACK,"Temporary Format Variables");
		return(NULL);
	}

	/* variable string is a character buffer used as temporary string
	manipulation area. It must be as long as the longer of the input record
	and the output record plus 1 byte for a null character added by sprintf
	when writing ASCII formats. */

	i = (input_format->max_length >= output_format->max_length) ?
		input_format->max_length : output_format->max_length;
	i++;
	
	variable_str = (char *)memMalloc(i, "make_tmp_format variable_str");
	if(!variable_str){
		err_push(ROUTINE_NAME,ERR_MEM_LACK,"variable_str");
		return(NULL);
	}

	out_v_list = FFV_FIRST_VARIABLE(output_format);
	while(dll_data(out_v_list)) {           /* MAIN LOOP ON OUTPUT VARIABLE LIST */

		out_var = FFV_VARIABLE(out_v_list);

		/* Check for corrupt memory */
		assert((void *)out_var == out_var->check_address);
		/* Skip Variables that do not get converted */
		if(IS_HEADER(out_var) || IS_INITIAL(out_var) || IS_CONSTANT(out_var)){

			/* create a FFV_NULL variable in the tmp_format */
			tmp_var = (VARIABLE *)memMalloc(sizeof(VARIABLE), "make_tmp_format tmp_var");
			if(!tmp_var){
				err_push(ROUTINE_NAME,ERR_MEM_LACK,"Making Variable: tmp_var");
				ff_free_format(tmp_format);
				tmp_format = NULL;
				memFree(variable_str, "make_tmp_format: Variable String");
				return((FORMAT_PTR)NULL);
			}

			memMemcpy((void *)tmp_var, (void *)out_var, sizeof(VARIABLE),NO_TAG);
			memStrcpy(tmp_var->name, out_var->name,NO_TAG);
			tmp_var->type = FFV_NULL;
			null_type_vars++;

			tmp_var->check_address = (void*)tmp_var;

			tmp_v_list = dll_add(dll_last(tmp_format->variables), 0);
			if(!tmp_v_list){
				err_push(ROUTINE_NAME,ERR_MEM_LACK,"Temporary Variable");
				ff_free_format(tmp_format);
				tmp_format = NULL;
				memFree(variable_str, "make_tmp_fmt: Variable String");
				return((FORMAT_PTR)NULL);
			}
			tmp_v_list->data_ptr = (void *)tmp_var;

			++tmp_format->num_in_list;
			out_v_list = dll_next(out_v_list);
			continue;
		}

		/* Determine if the output variable exists in the input list */
		in_var = ff_find_variable(out_var->name, input_format);

		if(in_var){     /* The variable exists in the input format:
					Create new variable in tmp_format and copy in_var */

			tmp_var = (VARIABLE *)memMalloc(sizeof(VARIABLE), "make_tmp_format: tmp_var");
			if(!tmp_var){
				err_push(ROUTINE_NAME,ERR_MEM_LACK,"Making Temporary Variable");
				ff_free_format(tmp_format);
				tmp_format = NULL;
				memFree(variable_str, "make_tmp_format: Variable String");
				return((FORMAT_PTR)NULL);
			}
			
			memMemcpy((void *)tmp_var, (void *)in_var, sizeof(VARIABLE),NO_TAG);
			tmp_var->check_address = (void*)tmp_var;
		}
		else {

			/* If the output variable does not exist in the input format
			the conversion list must be searched. The searching process
			involves exploratory calls to the conversion functions.
			These functions have the capability of modifying the input
			buffer. In order to recover from modifications caused by the
			exploratory calls, the input buffer is copied and nt_replaced.
			*/

			memcpy(variable_str, input_buffer, input_format->max_length);

			if ((i = cv_find_convert_var(out_var, input_format, input_buffer)) == -1)
			{
				/* The output variable could not be found in the conversion list:
					Add FFV_NULL variable to temporary format */
				tmp_var = (VARIABLE *)memMalloc(sizeof(VARIABLE), "make_tmp_format tmp_var");
				if(!tmp_var){
					err_push(ROUTINE_NAME,ERR_MEM_LACK,"Making Temporary Variable");
					ff_free_format(tmp_format);
					memFree(variable_str, "make_tmp_format: Variable String");
					return((FORMAT_PTR)NULL);
				}

				memMemcpy((void *)tmp_var, (void *)out_var, sizeof(VARIABLE),NO_TAG);
				strcpy(tmp_var->name, out_var->name);
				tmp_var->type = FFV_NULL;
				null_type_vars++;
				tmp_var->check_address = (void*)tmp_var;
			}
			else {  /* Conversion was found, Add FFV_CONVERT variable to tmp_format */

				tmp_var = (VARIABLE *)memMalloc(sizeof(VARIABLE), "make_tmp_format tmp_var");
				if(!tmp_var){
					err_push(ROUTINE_NAME,ERR_MEM_LACK,"Making Temporary Variable");
					ff_free_format(tmp_format);
					tmp_format = NULL;
					memFree(variable_str, "make_tmp_format: Variable String");
					return((FORMAT_PTR)NULL);
				}

				(void) memStrcpy(tmp_var->name, out_var->name,NO_TAG);
				tmp_var->start_pos = (unsigned short) i;
				tmp_var->end_pos = 0;
				tmp_var->type = FFV_CONVERT;
				tmp_var->precision = 0;
				tmp_var->check_address = (void*)tmp_var;
			}

			/* reinstate the initial input buffer */
			memMemcpy(input_buffer, variable_str, input_format->max_length,NO_TAG);
		}

		tmp_v_list = dll_add(dll_last(tmp_format->variables), 0);
		if(!tmp_v_list){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"Temporary Variable");
			ff_free_format(tmp_format);
			tmp_format = NULL;
			memFree(variable_str, "make_tmp_format: Variable String");
			return((FORMAT_PTR)NULL);
		}
		tmp_v_list->data_ptr = (void *)tmp_var;
		++tmp_format->num_in_list;
		out_v_list = dll_next(out_v_list);
	}
	memFree(variable_str, "make_tmp_format: Variable String");
	if(null_type_vars == (int)output_format->num_in_list){
		/* all out temp format contains is NULL type variables */
		err_push(ROUTINE_NAME, ERR_NO_VARS_SHARED, "between input and output format");
		return((FORMAT_PTR)NULL);
	}
	return(tmp_format);
}

/*
 * NAME:	ff_bin2bin
 *		
 * PURPOSE:	Converts binary representation between record fields
 *
 * USAGE:	int ff_bin2bin(	VARIABLE_PTR	in_var,
 *				FF_DATA_BUFFER		input_buffer,
 *				VARIABLE_PTR	out_var,
 *				FF_DATA_BUFFER		output_buffer)
 *
 * RETURNS:	0 if successful, error ID if not
 *
 * DESCRIPTION:	This function takes pointers to two data records and two
 * variables and converts from the input to the output.
 *
 * If the data type changes from integer to floating point then the floating
 * point value is divided by 10 raised to the power of the input precision,
 * otherwise if the data type changes from floating point to integer then the
 * integer value is multiplied by 10 raised to the power of the output
 * precision.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	If the input variable is a character type, only copies to another
 *							character are supported.
 *
 * ERRORS:
 *		Problem in conversion,"out_var->type"
 *		Unknown variable type,"out_var->type"
 *		Unknown variable type,"Input Variable"
 *		Unknown variable type,"out_var->type"
 * 
 * SYSTEM DEPENDENT FUNCTIONS:	Alignment is taken care of using doubles.
 *
 * KEYWORDS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_bin2bin"

static int ff_bin2bin(VARIABLE_PTR in_var, FF_DATA_BUFFER input_buffer,
          VARIABLE_PTR out_var, FF_DATA_BUFFER output_buffer)
{
	int error;
	double dbl_var;

	/* Error checking on NULL parameters and corrupt memory */
	assert(in_var);
	assert(input_buffer);
	assert(out_var);
	assert(output_buffer);
	assert(in_var == in_var->check_address);
	assert(out_var == out_var->check_address);

	/* The only support presently available for FFV_TEXT variables is a
	copy from the input to the output */
	if (IS_TEXT(in_var))
	{ 
		if (IS_TEXT(out_var))
		{
			(void)memcpy((void *)(output_buffer + out_var->start_pos - 1),
			 (void *)(input_buffer + in_var->start_pos - 1),
			 FF_VAR_LENGTH(in_var));
		}
		else
			return(err_push(ROUTINE_NAME, ERR_CONVERT, "convert to var type"));
	}
	else
		if ((error = btype_to_btype(input_buffer + in_var->start_pos - 1,
		 FFV_DATA_TYPE(in_var), output_buffer + out_var->start_pos - 1,
		 FFV_DATA_TYPE(out_var))) != 0)
			return(err_push(ROUTINE_NAME, error, "conversion to variable type"));
	
	/* Next check for precision conversion */
	if ((in_var->precision || out_var->precision) &&
	    ( (IS_INTEGER(in_var) && IS_REAL(out_var)) ||
	      (IS_REAL(in_var) && IS_INTEGER(out_var))) )
	{
		(void)btype_to_btype(output_buffer + out_var->start_pos - 1,
		 FFV_DATA_TYPE(out_var), &dbl_var, FFV_DOUBLE);

		if (IS_INTEGER(in_var) && IS_REAL(out_var))
			dbl_var /= pow(10.0, in_var->precision);
		else
			dbl_var *= pow(10.0, out_var->precision);
		
		if ((error = btype_to_btype(&dbl_var, FFV_DOUBLE,
		 output_buffer + out_var->start_pos - 1, FFV_DATA_TYPE(out_var))) != 0)
			return(err_push(ROUTINE_NAME, error, "implicit precision conversion"));
	}
	
	return(0);
}/* End ff_bin2bin() */
