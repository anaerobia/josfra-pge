/* FILENAME: dbevents.c
 *
 * CONTAINS: db_events()
 *           ff_get_delimiters() (a module-level function to db_events())
 *
 */

/*
 * NAME:        db_events
 *              
 * PURPOSE:     Handle events for data bins.
 *
 * USAGE:       db_events(DATA_BIN_PTR, event, [args], event, args,... NULL)
 *
 * RETURNS:     0 if all goes well.
 *
 * DESCRIPTION: This function handles events which are sent to data bins.
 * The function processes a list of argument groups which have
 * the form: event [arguments]. The list of groups is terminated
 * by a NULL, which ends processing.
 * The presently supported events and their arguments are:
 *
 * DBIN_CACHE_TO_HUGE:     char HUGE *destination
 * DBIN_CONVERT_CACHE:     DATA_BUFFER output
 *                         FORMAT_PTR output
 *                         long *output_bytes
 * DBIN_CONVERT_HEADER:    DATA_BUFFER output
 *                         FORMAT_PTR output 
 * DBIN_CONVERT_ROWSIZES:  FORMAT_PTR input_format
 *                         DATA_BUFFER output_data
 *                         FORMAT_PTR format
 *                         long *bytes_to_write
 *                         DLL_NODE_PTR rowsize_list
 * DBIN_DATA_TO_NATIVE:    char *start_pos
 *                         char *end_pos
 *                         FORMAT_PTR format
 * DBIN_DEMUX_CACHE:       DATA_BUFFER output_data
 *                         FORMAT_PTR format
 * DBIN_FILL_CACHE:        none
 * DBIN_GET_VECTOR:        none
 * DBIN_HUGE_TO_CACHE:     char HUGE *huge_ch
 *                         size_t bytes_to_read
 * DBIN_NEXT_REC_IN_CACHE: DATA_BUFFER *data_ptr
 * DBIN_NEXT_RECORD:       DATA_BUFFER *data_ptr 
 * DBIN_SCALE_CACHE:       (see DBIN_CONVERT_CACHE)
 * PROCESS_FORMAT_LIST:    int format_type 
 *
 * ERRORS:
 *
 * SYSTEM DEPENDENT FUNCTIONS:  Processing of RETURN formats assumes EOL = 2 bytes
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:    Only one header format is checked for in a format list.
 *				MEDIUM model:   need to malloc arrays
 *				check that allocations are allocating
 *				the correct type(near or huge)
 *				ff_get_delimiter() takes a scratch buffer as as argument
 *				but is being passed a huge data pointer
 *				will probably have to allocate a scratch buffer to copy data to
 *
 * KEYWORDS: databins
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <freeform.h>

#include <databin.h>
#include <dataview.h>

static int ff_get_delimiters(FORMAT_PTR format, FF_SCRATCH_BUFFER buffer, unsigned char *list);


#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "Dbin_event"
#define TMP_BUFF_SIZE   256

/* Operating System Dependent Definitions: */
#ifdef SUNCC
#include <errno.h>
#ifndef O_BINARY
#define O_BINARY 0
#endif
#endif

int db_events(DATA_BIN_PTR dbin, ...)
{
/* MAO this is redundant, and file_separator is switched for CCLSC and SUNCC
#ifdef CCMSC
char eol_string[] = {'\015', '\012', '\0'};
long eol_L      = 2L;
char file_separator = '\134'; // this is a back slash
#endif

#ifdef CCLSC
char eol_string[] = {'\015', '\0'};
long eol_L      = 1L;
char file_separator = '\057'; // this is a forward slash -- switched with SUNCC
#endif

#ifdef SUNCC
char eol_string[] = {'\012', '\0'};
long eol_L      = 1L;
char file_separator = '\072'; // this is a colon -- switched with CCLSC
#endif
*/
	va_list args;

	FF_SCRATCH_BUFFER	ch_ptr = NULL;

	char *point = NULL;
	char *start_pos  = NULL;
	char *end_pos  = NULL;

	char tmp_buffer[TMP_BUFF_SIZE];

	char HUGE	*huge_ch = NULL;

	signed char increment;

	DATA_BUFFER output_data = NULL;
	DATA_BUFFER tmp_output_data = NULL;
	DATA_BUFFER *data_ptr = NULL;

	unsigned bytes_to_read = 0;
	int bytes_read = 0;
	unsigned input_length = 0;
	unsigned output_length = 0;
	unsigned var_length = 0;
	unsigned num_input_vars = 0;
	unsigned shift_amount = 0;
	unsigned format_type = 0;
	unsigned overlap = 0;

	long tmp_long = 0L;
	long count = 1L;
	long cache_bytes = 0L;
	long output_bytes = 0L;
	long num_records = 0L;
	long record = 0L;

	int error = 0;
	int	event_name = 0;

	VARIABLE_PTR var	= NULL;
	VARIABLE_LIST_PTR	v_list = NULL;
	
	DLL_NODE_PTR rowsize_list = NULL;
	ROW_SIZES_PTR	rowsize = NULL;
	
	FORMAT_PTR format = NULL;
	FORMAT_PTR input_format = NULL;
	FORMAT_LIST_PTR f_list = NULL;

	long nbytes = 0L;
	long *bytes_to_write = NULL;

	assert(dbin && ((void *)dbin == dbin->check_address));
	
    /* some other functions downstream may need access to the path of the
	data file for the dbin. An environmental variable is set here to provide
	that access */
#if defined(CCMSC) || defined(SUNCC)
	if (dbin->file_name)
	{ /* DATA_PATH is later accessed in ff_process_variable_list() */
		memStrcpy(tmp_buffer,"DATA_PATH=",NO_TAG);
		memStrcat(tmp_buffer, dbin->file_name,NO_TAG);
		point = memStrrchr(tmp_buffer, NATIVE_DIR_SEPARATOR, "db_events: point");
		if (point)
		{
			*(point+1) = '\0';
			putenv(tmp_buffer); /* this should be changed to os_put_env() ? */
		}
	}
#endif

	ch_ptr = dbin->buffer;

	va_start(args, dbin);

	while ( (event_name = va_arg (args, int)) != 0 )
	{
		/* Check For Possible Memory Corruption */
		assert((void *)dbin == dbin->check_address);

		switch (event_name)
		{

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event DBIN_DATA_TO_NATIVE"

		case DBIN_DATA_TO_NATIVE:       /* arguments: start cache location, end cache location, format point */
		/* if both cache start and end location is NULL, using DBIN cache */
		/* if format_ptr is NULL, using the dbin->input_format */
			/*  transform binary data to native binary byte order */
			start_pos = va_arg (args, char *);
			end_pos = va_arg(args, char *);
			format = va_arg(args, FORMAT_PTR);
			
			if (!start_pos && !end_pos)
			{
				if (dbin->state.cache_defined == 0)
					return(err_push(MESSAGE_NAME, ERR_CACHE_DEFINED, ""));
			
				/*      cache not filled */ 
				if (!dbin->state.cache_filled)
					return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "Cache Not Filled"));

				start_pos=dbin->cache;
				end_pos=dbin->cache_end;
			}
			else if (!start_pos || !end_pos)
				err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "The cache from the arguments in DATA_TO_NATIVE");

			if (!format)
				format = dbin->input_format;

			/* Check to make sure the input format is binary */
			if (!IS_BINARY(format))
				break;

			/* check whether or not data is in the native format */
			if (endian() == (int)dbin->state.byte_order)
				break;

			v_list = FFV_FIRST_VARIABLE(format);
			while ((var = FFV_VARIABLE(v_list)) != NULL)
			{
				/* point at current variable */
				point = start_pos + var->start_pos - 1;

				/* handle only variables which can be in canonical form */
				while (point < end_pos)
				{
					byte_swap(point, FFV_TYPE(var));
					
					/* move to next row, same column in cache */
					point += format->max_length;
				}
				/* point at next variable */
				v_list = dll_next(v_list);
			}
			break;

		/*
		 * MESSAGE:     DBIN_CACHE_TO_HUGE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   char HUGE *destination
		 *
		 * PRECONDITIONS:	dbin->cache must be defined
		 *					dbin->cache must be filled
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Copies the bin cache to a huge buffer
		 *
		 * ERRORS:	Data cache not defined, "Can not copy cache"
		 *			Data cache not defined, "Cache Not Filled"
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event DBIN_CACHE_TO_HUGE"
		
		case DBIN_CACHE_TO_HUGE:

			/* assign buffer for output, output format and data ptr */
			huge_ch = va_arg (args, char HUGE *);

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, NULL));
			
			if (!dbin->state.cache_filled)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "Cache Not Filled"));
		
			tmp_long = dbin->cache_end - dbin->cache + 1;
			dbin->data_ptr = dbin->cache;
			while (tmp_long)
			{
				bytes_to_read =
					((unsigned long)UINT_MAX < (unsigned long)tmp_long) ? UINT_MAX : (unsigned int)tmp_long;
				_fmemMemcpy(huge_ch, dbin->data_ptr, bytes_to_read,NO_TAG);
				dbin->data_ptr += bytes_to_read;
				tmp_long -= bytes_to_read;
			}

			break;

		/*
		 * MESSAGE:	DBIN_CONVERT_CACHE
		 *			DBIN_SCALE_CACHE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:	DATA_BUFFER output, FORMAT_PTR output, long *result_bytes.
		 *
		 *				If output buffer is null, overwrite cache.
		 *					If (output > input) like binary to ascii ...
		 *					realloc and work from rear of cache     
		 *				If (input > output)
		 *					work from front of cache
		 *				If (input = output)
		 *					use 1st record as output and next as input
		 *					then use memove to make room for saved first record
		 *
		 *				else buffer is passed in and written to.        
		 *
		 *				If format is NULL, use dbin->output_format.
		 *
		 *				result_bytes will return the number of bytes in the 
		 *				output buffer.
		 *      
		 *
		 * PRECONDITIONS:	dbin->cache must be defined
		 *					dbin->cache must be filled
		 *					dbin->records_in_cache and/or cache_size
		 *					should reflect current cache
		 *
		 *					If SCALE_CACHE, the scale format must be defined
		 *						by DBIN_SET_SCALE.
		 *
		 * CHANGES IN STATE:Sets num_input_vars
		 *					ALLOCATES MEMORY FOR dbin->input_delimited_fields
		 *
		 *					If output buffer is null, cache is overwritten.
		 *					When DBIN_CONVERT_CACHE is completed, dbin->cache
		 *					is pointing to dbin->records_in_cache number of records
		 *					with records the size of the ouput format.
		 *
		 *					If (output > input) like binary to ascii ...
		 *						realloc and work from rear of cache     
		 *						dbin->cache_size is changed, and dbin->cache_end
		 *						is adjusted.
		 *					If (input > output)
		 *						work from front of cache
		 *						dbin->cache_end is adjusted
		 *					If (input = output)
		 *						use 1st record as output and next as input
		 *						then use memove to make room for saved first record
		 *
		 * DESCRIPTION: Reads the bin cache according to the input
		 *				format of the cache and converts it to the output
		 *				format into the buffer given by the argument to
		 *				db_events. dbin->data_ptr is a character pointer
		 *				which traverses the input cache.
		 *				The input format is checked each time to see if
		 *				it is delimited.
		 *				All records are converted and written to output_data,
		 *				which traverses the output buffer or the cache.
		 *				num_input_variables is increased if CONVERT variavbles are
		 *				added.
		 *
		 * ERRORS:	Undefined Format List,"scale format undefined"
		 *			Data cache not defined, "case DBIN_CONVERT_CACHE"
		 *			Data cache not defined, "Cache Not Filled"
		 *			Out of memory, "tmp buffer in CONVERT CACHE"
		 *			Out of memory, "tmp buffer in CONVERT CACHE"
		 *			Out of memory, "Realloc dbin->input_delimited_fields"
		 *			Out of memory, "tmp buffer in CONVERT CACHE"
		 *			Problem processing variable list, "dbin->data_ptr"
		 *			Problem getting delimiter, "case DBIN_CONVERT_CACHE"
		 *			Problem processing variable list, "Bytes remaining in cache"
		 *			Problem processing variable list, "dbin->data_ptr"
		 *			Out of memory, "dbin->input_delimited_fields"
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    Why is the delimited fields array allocated every time?
		 *				NOTE: No checking is done on the size of the output buffer.
		 *				If buffer is not large enough data is written into space!!
		 *
		 *				ff_process_variable_list has been changed to process
		 *				cache_bytes of cache.
		 *				If there are bytes remaining after processing, they
		 *				are written to the front of the cache.
		 *
		 * TESTING NEEDED:
		 *	      cache on cache
		 *	      non-record style of formats with variable EOL's
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event SCALE_CACHE"

		case DBIN_SCALE_CACHE:

			if ((input_format = db_find_format(dbin->format_list, FFF_GROUP, FFF_SCALE, END_ARGS)) == NULL)
				return(err_push(MESSAGE_NAME,ERR_FORMAT_LIST_DEF,"scale format undefined"));

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event CONVERT_CACHE"

		case DBIN_CONVERT_CACHE:

			/* assign buffer for output and an output format */
			output_data = va_arg (args, DATA_BUFFER);
			format = va_arg (args, FORMAT_PTR);
			bytes_to_write = va_arg(args, long *);

			if (format == NULL)
				format = dbin->output_format;

			/* initialize bytes_to_write to zero */
			*bytes_to_write = 0L;

			if (event_name == DBIN_CONVERT_CACHE)
				input_format=dbin->input_format;

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, NULL));
			
			if (!dbin->state.cache_filled)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "Cache Not Filled"));
		
			input_length = FORMAT_LENGTH(input_format);
			output_length = FORMAT_LENGTH(format);

			/* get number of bytes in output_bytes which is used for
			self cache, cache_bytes keeps track of bytes left in cache and
			may not be a multiple of record length */
			(void)db_show(dbin, DBIN_RECORD_COUNT, &output_bytes, END_ARGS);
			output_bytes *= FORMAT_LENGTH(format);
			cache_bytes = dbin->cache_end - dbin->cache + (long)1;

			overlap = 0;
			increment = 1;
			shift_amount = 0;
			dbin->data_ptr = dbin->cache;

			/* If output_data is NULL, determine overlap and set the
			pointers and variables needed to overwrite cache */
			if (output_data == NULL)
			{
				if (output_length == input_length)
					overlap = 1;

				else if (output_length < input_length)
				{ 
					overlap = (input_length / (input_length - output_length)) + 1;
				}
				else
				{
					if ((unsigned long)output_bytes > (unsigned long)UINT_MAX)
						return(err_push(MESSAGE_NAME,ERR_DATA_GT_BUFFER, "output size exceeds cache limits"));

					/* Output record is longer thant input record */	        
					if (output_bytes > dbin->cache_size)
					{
						tmp_output_data = dbin->cache = (char *)memRealloc(dbin->cache, (size_t)output_bytes, "db_events: convert_cache: tmp_output_data");

						if (!tmp_output_data)
							return(err_push(MESSAGE_NAME,ERR_MEM_LACK, "temporary buffer"));
					}
					overlap = (output_length/(output_length - input_length)) + 1;
					increment = -1;
				}
				dbin->cache_size = output_bytes;
				dbin->cache_end  = dbin->cache + output_bytes - 1;
			}

			if (dbin->state.input_delimited)
			{
				/* Save the original number of variables */
				num_input_vars = input_format->num_in_list;

				/* Initialize an array of unsigned chars, one for each
				variable in the format. The elements of this array are
				set to 1 if that variable is delimited and 0 if it is not.
				This setting occurs in the function ff_get_delimiters */

				dbin->input_delimited_fields =
					(unsigned char *)memRealloc((void *)(dbin->input_delimited_fields),
					input_format->num_in_list, "db_events convert_cache: dbin->input_delimited_fields");

				if (!dbin->input_delimited_fields)
				{
					memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
					return(err_push(MESSAGE_NAME,ERR_MEM_LACK, NULL));
				}
				memMemset((void *)dbin->input_delimited_fields,'\0', input_format->num_in_list,"dbin->input_delimited_fields,'\\0', input_format->num_in_list");
			}

			/* reset the temporary buffer pointer */
			tmp_output_data = NULL;

			/* If self cache, fill a temporary buffer and reset pointers */
			if (overlap)
			{
				/* shift_amount is the number of bytes that need to be saved
				into a temporary buffer to avoid overlapping data pointers */
				shift_amount = output_length * overlap;

				tmp_output_data = (DATA_BUFFER)memMalloc(shift_amount, "db_events: CONVERT CACHE: tmp_output_data");
				if (!tmp_output_data)
					return(err_push(MESSAGE_NAME,ERR_MEM_LACK, NULL));
				
				nbytes=overlap * FORMAT_LENGTH(input_format);
				cache_bytes -= nbytes;
				error = ff_process_variable_list(dbin->data_ptr, tmp_output_data, input_format, format, &nbytes);
				*bytes_to_write += nbytes;

				if (error < 0)
				{
					memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
					return(ERR_PROC_LIST);
				}

				if (increment == 1)
				{
					/* Output record length is <= input record length */
					if (output_length == input_length)
					{
						output_data = dbin->cache;
						dbin->data_ptr = dbin->cache + input_length;
					}
					else
						output_data = dbin->cache + shift_amount;
						dbin->data_ptr = dbin->cache + overlap * FORMAT_LENGTH(input_format);
				}
				else
				{

					/* Output record is longer than input record.
					Define the input data pointer in terms of the front
					of the cache because the end of the cache has already
					been moved to accommodate the longer output records. This
					is why the output pointer is defined in terms of the (new)
					end of the cache. */

					dbin->data_ptr = dbin->cache + (dbin->records_in_cache - 1) * FORMAT_LENGTH(input_format);
					output_data = dbin->cache_end - output_length + 1;
				}
			}/* Done filling tmp_output_data from start of cache */

			if (dbin->state.input_delimited)
			{ 
				if (ff_get_delimiters(input_format, (SCRATCH_BUFFER)dbin->data_ptr, dbin->input_delimited_fields))
				{
					if (tmp_output_data)
						memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
					return(err_push(MESSAGE_NAME,ERR_GET_DELIMITER, NULL));
				}
			}

			/* negative cache_bytes processes from rear of cache */
			nbytes = cache_bytes * increment;
			error = ff_process_variable_list(dbin->data_ptr, output_data, input_format, format, &nbytes);

			*bytes_to_write += nbytes;
			if (error > 0) /* there is a remainder */
			{
				if (overlap)
				{
					if (tmp_output_data)
						memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
					return(err_push(MESSAGE_NAME,ERR_PROC_LIST, "Bytes remaining in cache"));
				}
				else
				{
					dbin->file_location -= error;
					lseek(dbin->data_file, dbin->file_location, SEEK_SET);
					dbin->data_available += error;
					dbin->bytes_available += error;
				}
			}
			if (error < 0)
			{
				if (tmp_output_data)
					memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
				return(err_push(MESSAGE_NAME,ERR_PROC_LIST, "dbin->data_ptr"));
			}

			/* ff_process_variable_list can modify the input format by adding
			FFV_CONVERT variables to it. If this happens, the input_delimited_fields
			array gets out of registration and needs to be shifted */

			if (dbin->state.input_delimited && input_format->num_in_list != num_input_vars)
			{
				dbin->input_delimited_fields =
					(unsigned char *)memRealloc((void *)(dbin->input_delimited_fields),
					input_format->num_in_list,"db_events: CONVERT_CACHE: dbin->input_delimited_fields");
				if (!dbin->input_delimited_fields)
				{
					if (tmp_output_data)
						memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
					return(err_push(MESSAGE_NAME,ERR_MEM_LACK, "dbin->input_delimited_fields"));
				}

				shift_amount = input_format->num_in_list - num_input_vars;
				memmove((char *)dbin->input_delimited_fields + shift_amount,
					(char *)dbin->input_delimited_fields,
					num_input_vars);

				memMemset((void *)dbin->input_delimited_fields,'\0', shift_amount,"dbin->input_delimited_fields,'\\0',shift_amount");
				num_input_vars = input_format->num_in_list;
			}

			/* Cache has been converted */

			/* Make room for saved data at start of cache, if cache was used */
			if ((overlap) && (input_length == output_length))
			{
				output_data = dbin->cache + output_length;
/* MAO:d 4/95 possibly an error (it is for me in this case) in which too few
   bytes are moved...
				memMemmove(output_data, dbin->cache, (size_t)(cache_bytes - output_length),NO_TAG);
*/
				memMemmove(output_data, dbin->cache, (size_t)cache_bytes,NO_TAG);
			}

			/* Copy saved data to start of cache */
			if (tmp_output_data)
			{
				memMemcpy(dbin->cache, tmp_output_data, shift_amount,NO_TAG);
				memFree(tmp_output_data, "dbin_events convert_cache: tmp_output_data");
				tmp_output_data = NULL;
			}
		
			break;
			
		/*
		 * MESSAGE:	DBIN_CONVERT_ROWSIZES
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:	FORMAT_PTR input_format, DATA_BUFFER output_buffer, FORMAT_PTR output_format, long *result_bytes, DLL_NODE_PTR rowsize_list.
		 *				output buffer MUST BE DEFINED and large enough to handle the 
		 *				output.
		 *
		 * PRECONDITIO NS:	output must be defined and large enough to handle the output.
		 *
		 * CHANGES IN STATE:Sets num_input_vars
		 *					ALLOCATES MEMORY FOR dbin->input_delimited_fields
		 *
		 * DESCRIPTION: Converts records in the rowsizes structs from input_format to 
		 *				output format.
		 *				If input_format is NULL, dbin->input_format is used.
		 *				If output_format is NULL, dbin->output_format is used.
		 *				The input format is checked each time to see if
		 *				it is delimited.
		 *				All records are converted and written to output_data,
		 *				which is kept contiguous.
		 *
		 * ERRORS:	
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *				Kevin Frender (kbf@ngdc.noaa.gov)
		 *
		 * COMMENTS:    Why is the delimited fields array allocated every time?
		 *				NOTE: No checking is done on the size of the output buffer.
		 *				If buffer is not large enough data is written into space!!
		 *
		 *				ff_process_variable_list has been changed to process
		 *				cache_bytes of cache.
		 *				If there are bytes remaining after processing, they
		 *				are written to the front of the cache.
		 *
		 * TESTING NEEDED:
		 *				Everything- currently untested
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event CONVERT_ROWSIZES"

		case DBIN_CONVERT_ROWSIZES:

			input_format= va_arg(args, FORMAT_PTR);
			/* assign buffer for output and an output format */
			output_data = va_arg (args, DATA_BUFFER);
			format = va_arg (args, FORMAT_PTR);
			bytes_to_write = va_arg(args, long *);
			rowsize_list = va_arg(args, DLL_NODE_PTR);
            
			if (!input_format)
				input_format = dbin->input_format;
            
			if (!format)
				format = dbin->output_format;
			
			if (!output_data)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "Output cache not defined"));

			if (!rowsize_list)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "For rowsizes DLL"));

			/* initialize bytes_to_write to zero */
			*bytes_to_write = 0L;
			
			input_length = FORMAT_LENGTH(input_format);
			output_length = FORMAT_LENGTH(format);

			/* get number of bytes in output_bytes which is used for
			self cache, cache_bytes keeps track of bytes left in cache and
			may not be a multiple of record length */
			(void)db_show(dbin, DBIN_RECORD_COUNT, &output_bytes, END_ARGS);
			output_bytes *= FORMAT_LENGTH(format);
			cache_bytes = dbin->cache_end - dbin->cache + (long)1;

			overlap = 0;
			increment = 1;
			shift_amount = 0;
			dbin->data_ptr = dbin->cache;

			if (dbin->state.input_delimited)
			{
				/* Save the original number of variables */
				num_input_vars = input_format->num_in_list;

				/* Initialize an array of unsigned chars, one for each
				variable in the format. The elements of this array are
				set to 1 if that variable is delimited and 0 if it is not.
				This setting occurs in the function ff_get_delimiters */

				dbin->input_delimited_fields =
					(unsigned char *)memRealloc((void *)(dbin->input_delimited_fields),
					input_format->num_in_list, "db_events convert_cache: dbin->input_delimited_fields");

				if (!dbin->input_delimited_fields)
					return(err_push(MESSAGE_NAME,ERR_MEM_LACK, "Realloc dbin->input_delimited_fields"));

				memMemset((void *)dbin->input_delimited_fields,'\0', input_format->num_in_list,"dbin->input_delimited_fields,'\\0',input_format->num_in_list");
			}

			/* reset the temporary buffer pointer to the output buffer*/
			tmp_output_data = output_data;

			if (dbin->state.input_delimited)
			{ 
				if ((ff_get_delimiters(input_format, (SCRATCH_BUFFER)dbin->data_ptr, dbin->input_delimited_fields)))
					return(err_push(MESSAGE_NAME,ERR_GET_DELIMITER, "case DBIN_CONVERT_CACHE"));
			}
			
			rowsize_list = dll_first(rowsize_list);
			while ((rowsize = (ROW_SIZES *)dll_data(rowsize_list)) != NULL)
			{
				nbytes = rowsize->num_bytes;
				error = ff_process_variable_list((FF_DATA_BUFFER)rowsize->start, tmp_output_data, input_format, format, &nbytes);
				tmp_output_data += nbytes;
	
				*bytes_to_write += nbytes;
				if (error > 0) /* there is a remainder */
					return(err_push(MESSAGE_NAME, ERR_PROC_LIST, "record length/rowsize num_bytes mismatch"));

				if (error < 0)
					return(err_push(MESSAGE_NAME,ERR_PROC_LIST, "dbin->data_ptr"));
				
				rowsize_list = dll_next(rowsize_list);
			}

			/* ff_process_variable_list can modify the input format by adding
			FFV_CONVERT variables to it. If this happens, the input_delimited_fields
			array gets out of registration and needs to be shifted */

			if (dbin->state.input_delimited && input_format->num_in_list != num_input_vars)
			{
				dbin->input_delimited_fields =
					(unsigned char *)memRealloc((void *)(dbin->input_delimited_fields),
					input_format->num_in_list,"db_events: CONVERT_CACHE: dbin->input_delimited_fields");

				if (!dbin->input_delimited_fields)
					return(err_push(MESSAGE_NAME,ERR_MEM_LACK, "dbin->input_delimited_fields"));

				shift_amount = input_format->num_in_list - num_input_vars;
				memmove((char *)dbin->input_delimited_fields + shift_amount,
					(char *)dbin->input_delimited_fields,
					num_input_vars);

				memMemset((void *)dbin->input_delimited_fields,'\0', shift_amount,"dbin->input_delimited_fields,'\\0',shift_amount");
				num_input_vars = input_format->num_in_list;
			}

			/* Cache has been converted */

			break;
		

		/*
		 * MESSAGE:     DBIN_CONVERT_HEADER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS: DATA_BUFFER destination
		 *				FORMAT_PTR output_format
		 *
		 * PRECONDITIONS:       dbin->header must be defined
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Fills the buffer with the converted header      
		 *
		 * ERRORS:	Data cache not defined, "case DBIN_CONVERT_HEADER"
		 *			Problem performing dbin events, "Need buffer for CONVERT_HEADER"
		 *			Problem performing dbin events, "Need format for DBIN_CONVERT_HEADER"
		 *			Problem processing variable list, "dbin->data_ptr in header"
		 *
		 * AUTHOR:      TAM, NGDC,(303)497-6472, tam@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event CONVERT_HEADER"

		case DBIN_CONVERT_HEADER:

			output_data = va_arg(args, DATA_BUFFER);
			format = va_arg (args, FORMAT_PTR);

			/* Check on header definition */
			if (dbin->state.header_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "Header Cache"));

			if (!output_data)
				return(err_push(MESSAGE_NAME,ERR_DBIN_EVENT, "Need buffer for CONVERT_HEADER"));
			
			/* assign formats */
			input_format = dbin->header_format;
			if ((!input_format) || (!format))
				return(err_push(MESSAGE_NAME,ERR_DBIN_EVENT, "Need format for DBIN_CONVERT_HEADER"));

			/* get the length of the header */
			input_length = FORMAT_LENGTH(input_format);

			/* Convert the header into the buffer */
			dbin->data_ptr = dbin->header;
			nbytes=FORMAT_LENGTH(input_format);
			error = ff_process_variable_list(dbin->data_ptr, output_data, input_format, format, &nbytes);
			if (error == -1)
				return(err_push(MESSAGE_NAME,ERR_PROC_LIST, "dbin->data_ptr in header"));

			break;

		/*
		 * MESSAGE:     DBIN_DEMUX_CACHE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   DATA_BUFFER output_data
		 *              FORMAT_PTR format  (default is dbin->input_format)
		 *
		 * PRECONDITIONS:       dbin->cache must be defined
		 *                      dbin->cache must be filled
		 *                      Size of output_data buffer must be large enough to
		 *                      handle the demultiplexed data. 
		 *                      The format must correspond to the data in the cache
		 *
		 * CHANGES IN STATE: data_buffer will contain the demultiplexed binary data
		 *
		 * DESCRIPTION: DBIN_DEMUX_CACHE takes a binary BIP dbin->cache 
		 *              and demultiplexes it into the given data buffer
		 *
		 * ERRORS:      dbin->cache not defined or filled
		 *              No memory for allocating the cache_offsets array of ptrs
		 *
		 * AUTHOR:      Mark Van Gorp, NGDC, mvg@ngdc.noaa.gov, (303)497-6221
		 *
		 * COMMENTS:    Specified variables can be demultiplexed by sending in a format
		 *              which matches the format of the cache but whose variable list contains
		 *              only the desired variables with their correct record offsets 
		 *              in the cache. format->num_in_list would have to be the number
		 *              of variables in this new list
		 *              
		 *
		 * KEYWORDS:    
		 *
		 */
		case DBIN_DEMUX_CACHE:

			/* assign buffer for demultiplexed output */
			ch_ptr = output_data = va_arg (args, DATA_BUFFER);
			/* determine the current cache format */
			format = va_arg (args, FORMAT_PTR);

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(ROUTINE_NAME,ERR_CACHE_DEFINED, "DBIN_DEMUX_CACHE"));

			if (!dbin->state.cache_filled)
				return(err_push(ROUTINE_NAME,ERR_CACHE_DEFINED, "DBIN_DEMUX_CACHE: Cache Not Filled"));
		
			if (!format)
				format = dbin->input_format;

			/* initialize */
			input_length = FORMAT_LENGTH(format);
			num_records = dbin->records_in_cache;
			v_list = FFV_FIRST_VARIABLE(format);
	
			/* Loop over the variables in the cache and de-multiplex the data
			   point is incremented through each variable array in the cache */
			while ((var = FFV_VARIABLE(v_list)) != NULL)
			{
				point = dbin->cache + var->start_pos - 1;
				var_length = FF_VAR_LENGTH(var);
	
				for (record = 0;record < num_records;
				    ++record, point += input_length)
				{
					memMemcpy(ch_ptr, point, var_length,NO_TAG);
					ch_ptr += var_length;
				}
				v_list=dll_next(v_list);
			}

			break;

		/*
		 * MESSAGE:     DBIN_FILL_CACHE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:       dbin->cache must be defined
		 *
		 * CHANGES IN STATE:    Adjusts cache size to be even multiple of
		 *	              input record length.
		 *	              If a header exists, its length is subtracted
		 *	              from bytes_available.
		 *	              Sets dbin->bytes_to_read.
		 *	              Sets dbin->cache_end
		 *	              Sets dbin->cache_size
		 *	              Sets dbin->records_in_cache
		 *	              Sets dbin->data_ptr to beginning of cache
		 *	              Sets dbin->state.cache_filled = 1
		 *	              Decrements dbin->bytes_available
		 * Increments dbin->file_location or sets to length of header if
		 * dbin->file_location is less than length of an unseparated header.
		 * In DESCRIPTION: below the First filling: is no longer true -- with
		 * random file access the top of the file is not always in the first
		 * filling -- a better check has been implemented.
		 *
		 * DESCRIPTION: Fills the cache for the data_bin
		 *				First filling:
		 *					If a header format exists seek to file_location
		 *					then set file_location to max_length of header
		 *					format. If no header format exists, look for a
		 *					header variable in the old-fashioned way.
		 *				Seek to file_location.
		 *				Read smaller of cache_size and bytes_available
		 *
		 * ERRORS:      No cache defined
		 *				No bytes in file (bytes_available = 0) (Return EOF)
		 *				Problem skipping header
		 *				Problems seeking in file
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    What is being done by the lseek at the beginning of
		 *				files with headers? Should file headers be read here?
		 *				Somehow the header stuff needs to be worked out.
		 *
		 * KEYWORDS:
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event FILL_CACHE"

		case DBIN_FILL_CACHE:

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "case DBIN_FILL_CACHE"));
			
			/* calculate length of header */
			if (dbin->header_format && !IS_SEPARATE(dbin->header_format))
				tmp_long = dbin->header_format->max_length;
			else
			{
				tmp_long = ff_skip_header(dbin->input_format, dbin->data_file);
				if (err_state())
					return(err_push(MESSAGE_NAME,ERR_HEAD_DEFINED, "ff_skip_header dbin->input_format"));
			}

			/* check if file_position needs to be advanced beyond header */
			if (dbin->file_location < tmp_long)
			{ 
				dbin->file_location = tmp_long;
				dbin->bytes_available -= dbin->file_location;
			}
			
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// MAO:d This is the old code which did not allow for random file access
// This old code can probably be deleted by, oh, say 6/95 if all is well
			// Read bytes from the file for this bin if they are available
			if (dbin->state.cache_filled == 0) // Fill for first time
			{
				// check for header and adjust cache
				if (dbin->header_format && !IS_SEPARATE(dbin->header_format))
				{
					if ((lseek(dbin->data_file, dbin->file_location, SEEK_SET)) == -1L)
					{
						err_push(MESSAGE_NAME,errno, "lseek dbin->data_file");
						return(errno);
					}
					dbin->file_location = dbin->header_format->max_length;
				}
				else // fill without header
				{
					// Check for old-fashioned header type description
					dbin->file_location = ff_skip_header(dbin->input_format, dbin->data_file);
					if (err_state())
					{
						err_push(MESSAGE_NAME,ERR_HEAD_DEFINED, "ff_skip_header dbin->input_format");
						return(ERR_HEAD_DEFINED);
					}
				}

				dbin->bytes_available -= dbin->file_location;
			}
// Here endeth the old code which did not allow for random file access
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

			/* Check for data in file */
			if (dbin->bytes_available == 0)
				return(EOF);
			if (dbin->bytes_available < 0)
			{
				err_push(MESSAGE_NAME,errno, "DFC, Reading past EOF");
				return(errno ? errno : EOF);
			}

			/* If an input format is defined, the cache size is adjusted to
			be the largest integral multiple of the record size */

			if (dbin->input_format)
			{
				input_length = FORMAT_LENGTH(dbin->input_format);
				dbin->cache_size = dbin->records_in_cache * input_length;
			}

			if ((lseek(dbin->data_file, dbin->file_location, SEEK_SET)) == -1L)
				return(err_push(MESSAGE_NAME,errno, "lseek dbin->data_file"));

			dbin->bytes_to_read = (dbin->cache_size < dbin->bytes_available) ?
							(unsigned int)dbin->cache_size : (unsigned int) dbin->bytes_available;
			bytes_read = READ_DATA(dbin->data_file, dbin->cache, dbin->bytes_to_read,NO_TAG);

			if (bytes_read != dbin->bytes_to_read)
				return(err_push(MESSAGE_NAME,errno, "Data File"));

			dbin->bytes_available -= bytes_read;
			dbin->file_location += bytes_read;
			dbin->cache_end = dbin->cache + bytes_read - 1;
			dbin->data_ptr = dbin->cache;
			dbin->state.cache_filled = 1;
			if (dbin->input_format)
				dbin->records_in_cache = bytes_read / input_length; 
			break;

		/*
		 * MESSAGE:     DBIN_NEXT_RECORD
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   DATA_BUFFER *data_ptr
		 *
		 * PRECONDITIONS:	file must be opened
		 *					dbin->cache must be defined
		 *					if cache is not filled this message sends a
		 *					DBIN_FILL_CACHE to the dbin.
		 *					Input_format must be defined so record length
		 *					is known.
		 *
		 * CHANGES IN STATE:    dbin->data pointer is incremented.
		 *
		 * DESCRIPTION: Increments the data pointer for the dbin by one record.
		 *				If cache is empty, it is filled and pointer to cache
		 *				beginning is returned.
		 *
		 * ERRORS:	No file defined.
		 *			No cache defined
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    Why does the file need to be defined?
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event NEXT_RECORD"

		case DBIN_NEXT_RECORD:

			/* Define the pointer which will be set */
			data_ptr = va_arg(args, DATA_BUFFER *);
				
			/* Check that file has been defined */
			if (!(dbin->state.open_file || dbin->state.std_input))
				error = db_set(dbin,
					DBIN_FILE_HANDLE,
					NULL);

			if (error)
				return(error);

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "case DBIN_NEXT_RECORD"));
			
			/* Unfilled Cache */
			if (dbin->state.cache_filled == 0)
			{
				error = db_events(dbin,
					DBIN_FILL_CACHE,
					NULL);

				if (error)
					return(error);

				/* The cache has been filled and the DBIN_FILL_CACHE event has
				set the file location. The next record is the beginning of
				the cache */
				*(data_ptr) = dbin->cache;
				break;
			}

			/* Filled cache */
			if (dbin->input_format)
				dbin->bytes_to_read = FORMAT_LENGTH(dbin->input_format);
			else
				return(err_push(MESSAGE_NAME,ERR_BINFORM_DEFINED, "Input format"));

			dbin->data_ptr += dbin->bytes_to_read;

			if (dbin->data_ptr > dbin->cache_end) /* off end */
			{
				error = db_events(dbin, 
					DBIN_FILL_CACHE,
					NULL);
				if (error)
					return(error);
			}
			*(data_ptr) = dbin->data_ptr;
			break;


		/*
		 * MESSAGE:     DBIN_NEXT_REC_IN_CACHE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS: DATA_BUFFER *data_ptr (set to NULL if off end of cache)
		 *
		 * PRECONDITIONS:      dbin->cache must be defined
		 *                     input_format must be defined so record length
		 *                     is known.
		 *
		 * CHANGES IN STATE:    dbin->data pointer is incremented.
		 *
		 * DESCRIPTION: Increments the data pointer for the dbin by one record.
		 *
		 * ERRORS:            No cache defined
		 *
		 * AUTHOR:      Kevin Frender (kbf@kryton.ngdc.noaa.gov)
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event NEXT_REC_IN_CACHE"

		case DBIN_NEXT_REC_IN_CACHE:

			/* Define the pointer which will be set */
			data_ptr = va_arg(args, DATA_BUFFER *);
				
			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "case DBIN_NEXT_RECORD"));
			
			/* Unfilled Cache */
			if (dbin->state.cache_filled == 0)
				return(err_push(MESSAGE_NAME, ERR_CACHE_DEFINED, "Cache not filled"));

			/* Filled cache */
			if (dbin->input_format)
				dbin->bytes_to_read = FORMAT_LENGTH(dbin->input_format);
			else
				return(err_push(MESSAGE_NAME,ERR_BINFORM_DEFINED, "Input format"));

			dbin->data_ptr += dbin->bytes_to_read;

			if (dbin->data_ptr > dbin->cache_end) /* off end */
				return(EOF);
			else
				*(data_ptr) = dbin->data_ptr;

			break;

		/*
		 * MESSAGE:     DBIN_GET_VECTOR
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:	file must be opened
		 *					cache must be defined
		 *
		 * CHANGES IN STATE:	Seeks to file_location
		 *						Sets dbin->data_ptr to cache beginning
		 *
		 * DESCRIPTION: Reads dbin->bytes_to_read bytes from the data file
		 *				beginning at file_location.
		 *
		 * ERRORS:	No file defined.
		 *			No cache defined
		 *
		 * AUTHOR:	TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    What if dbin->bytes_to_read is incorrect?
		 *				What if dbin->bytes_to_read is larger than cache_size?
		 *				file_location is not changed.
		 *				bytes_available not changed.
		 *				data available not changed.
		 *				THIS MESSAGE NEEDS WORK.
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event GET_VECTOR"

		case DBIN_GET_VECTOR:

			/* Check that file has been defined */
			if (!(dbin->state.open_file || dbin->state.std_input))
				error = db_set(dbin,
					DBIN_FILE_HANDLE,
					NULL);

			if (error)
				return(error);

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, dbin->title));

			tmp_long = lseek(dbin->data_file, dbin->file_location, SEEK_SET);
			if (tmp_long == -1)
				return(err_push(MESSAGE_NAME,errno, "lseek in case DBIN_GET_VECTOR"));

			bytes_read = READ_DATA(dbin->data_file, dbin->cache, dbin->bytes_to_read,NO_TAG);

			if (bytes_read != dbin->bytes_to_read)
				return(err_push(MESSAGE_NAME,errno, "Reading bytes in DBIN_GET_VECTOR"));

			dbin->data_ptr = dbin->cache;

			break;

		/*
		 * MESSAGE:		DBIN_HUGE_TO_CACHE
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   char huge *source, size_t bytes_to_read
		 *
		 * PRECONDITIONS:	dbin->cache must be defined
		 *
		 * CHANGES IN STATE:	None
		 *
		 * DESCRIPTION: Copies data from a huge buffer to the bin cache
		 *
		 * ERRORS:	No cache defined
		 *
		 * AUTHOR:	TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event HUGE_TO_CACHE"

		case DBIN_HUGE_TO_CACHE:

			huge_ch = va_arg (args, char HUGE *);
			bytes_to_read = va_arg (args, size_t);

			/* No Cache Defined */
			if (dbin->state.cache_defined == 0)
				return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, "Can not copy cache"));
			
			_fmemMemcpy(huge_ch, dbin->data_ptr, bytes_to_read,NO_TAG);
			dbin->cache_end = dbin->cache + bytes_to_read - 1;
			break;

		/*
		 * MESSAGE:     PROCESS_FORMAT_LIST
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   int format_type
		 *
		 * PRECONDITIONS:	dbin->format_list defined
		 *					dbin->cache defined
		 *					If header exists, must be defined 
		 *
		 * CHANGES IN STATE:	headers are read into dbin->header
		 *						cache is filled with input data
		 *						amount read is equal to dbin->data_available
		 *						or dbin->cache_size if data_available is
		 *						too large
		 *						lseek is done past FFF_RETURN
		 *
		 * DESCRIPTION: Read whatever is appropriate from the file
		 *
		 * ERRORS:	Problem setting data bin, "case PROCESS_FORMAT_LIST"
		 *			Dbin format not defined, "dbin->format_list"
		 *			System File Error, "PROCESS LIST, Reading past EOF "
		 *			Header not defined, "case PROCESS_FORMAT_LIST"
		 *			System File Error, "PROCESS LIST, Reading file header"
		 *			Problem getting delimiter, "dbin->header_format"
		 *			Header not defined, "case PROCESS_FORMAT_LIST")
		 *			Header not defined, "header file undefined")
		 *			System File Error, "lseek(dbin->header_file, 0, SEEK_CUR)"
		 *			System File Error, "PROCESS LIST, Reading record header"
		 *			System File Error, "lseek in PROCESS_FORMAT_LIST"
		 *			System File Error, "lseek(dbin->header_file, 0, SEEK_CUR)"
		 *			System File Error, "PROCESS LIST, Reading record header"
		 *			Out of memory, "Realloc header_delimited_fields"
		 *			Problem getting delimiter, "dbin->header_format"
		 *			System File Error, "lseek dbin->data_file"
		 *			System File Error, "PROCESS LIST, Reading past EOF"
		 *			Data cache not defined, dbin->title
		 *			Problem reading file, "Incorrect Number of Bytes Read to Cache"
		 *			System File Error, "lseek dbin->data_file"
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

#undef MESSAGE_NAME
#define MESSAGE_NAME "Dbin_event PROCESS_FORMAT_LIST"

		case PROCESS_FORMAT_LIST:       /* Argument: format type */

			/* Define the format types to be processed */
			format_type = va_arg (args, int);

			/* Determine whether a file has been opened */
			if (!(dbin->state.open_file || dbin->state.std_input))
				error = db_set(dbin,
					DBIN_FILE_HANDLE,
					NULL);

			if (error)
				return(error);

			/* Determine whether a format list has been defined */
			if (!dbin->format_list)
				return(err_push(MESSAGE_NAME,ERR_BINFORM_DEFINED, "Format list"));

			if (dbin->bytes_available == 0)
				return(EOF);
			if (dbin->bytes_available < 0)
				return(err_push(MESSAGE_NAME,errno, "PROCESS LIST, Reading past EOF "));
			
			f_list = dll_first(dbin->format_list);
			while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
			{
				/* If this is an input FILE_HEADER, Space was allocated for it when
				the format list was defined. Now is the time to read it. */
				if ((format->type & format_type) &&
					!IS_OUTPUT(format) &&
					IS_HD(format) &&
					IS_FILE(format))
				{
					/* and it has not been defined: Error */
					if (!dbin->state.header_defined)
						return(err_push(MESSAGE_NAME,ERR_HEAD_DEFINED, ""));

					/* If we are at the start of the file, it is time to read a header if the header
						is an embedded header - this assumes that a separate header has already
						been read into header buffer in db_set(DEFINE_HEADER) */        
					if ((dbin->header_file == dbin->data_file) && (dbin->file_location == 0L))
					{
						bytes_to_read = dbin->header_format->max_length;
						if ((dbin->header_format->type & FFF_ASCII) && (dbin->input_format->type & FFF_ASCII))
							bytes_to_read += EOL_SPACE;

						bytes_read = READ_DATA(dbin->header_file, dbin->header, bytes_to_read,NO_TAG);
						if (bytes_read != bytes_to_read)
							return(err_push(MESSAGE_NAME,errno, "Reading file header"));

						dbin->file_location += bytes_to_read;
						dbin->bytes_available -= bytes_to_read;
						if (dbin->data_available)
							dbin->data_available = dbin->bytes_available;

						if (dbin->state.header_delimited)
						{
							if ((ff_get_delimiters(dbin->header_format, dbin->header, dbin->header_delimited_fields)))
								return(err_push(MESSAGE_NAME,ERR_GET_DELIMITER, "dbin->header_format"));
						}/* if header_delimited */

						/* indicate header cache has been filled */
						dbin->state.new_record = 1;
					}/* done reading file header */

					f_list = dll_next(f_list);
					continue;
				}

				/* If this is a RECORD_HEADER, read it */
				if ((format->type & format_type) &&
					!IS_OUTPUT(format) &&
					IS_HD(format) &&
					IS_REC(format) &&
					dbin->data_available == 0)
				{
					/* and it has not been defined: Error */
					if (!dbin->state.header_defined)
						return(err_push(MESSAGE_NAME,ERR_HEAD_DEFINED, ""));

					/* check that the header file has been defined */
					if (!dbin->header_file)
						return(err_push(MESSAGE_NAME,ERR_HEAD_DEFINED, "header file undefined"));


					/* If there was a file header, we now need to put the
					record header format in dbin->header_format
					*/
					if (!IS_REC(dbin->header_format))
						dbin->header_format = format;

					/* In some strong motion data there are a series of null
					characters which are inserted in order to make constant
					length records. These nulls change the positioning of the
					record headers. it seems reasonable to skip non-printing
					characters if the header we are trying to read is ASCII
					or DBASE. This is done by reading a small buffer and checking
					to see if it is printable then repositioning the file */
					if (IS_ASCII(format) || IS_DBASE(format))
					{
						tmp_long = lseek(dbin->header_file, 0, SEEK_CUR);
						if (tmp_long == -1L)
							return(err_push(MESSAGE_NAME,errno, "lseek(dbin->header_file), 0, SEEK_CUR"));

						bytes_to_read = TMP_BUFF_SIZE;
						bytes_read = READ_DATA(dbin->header_file, tmp_buffer, bytes_to_read,NO_TAG);
						/* the bytes_read may be less then bytes_to_read */
						if (bytes_read == -1)
							return(err_push(MESSAGE_NAME,errno, "Reading record header"));

						for (point = tmp_buffer, shift_amount = 0;
							shift_amount < bytes_read;
							++point, ++shift_amount)
						{
							if (isprint((int)*point))
								break;
						}
						if ((lseek(dbin->header_file, tmp_long + shift_amount, SEEK_SET)) == -1L)
							return(err_push(MESSAGE_NAME,errno, "lseek on header"));

						if (!IS_SEPARATE(dbin->header_format))
							dbin->bytes_available -= shift_amount;

						tmp_long = lseek(dbin->header_file, 0, SEEK_CUR);
						if (tmp_long == -1L)
							return(err_push(MESSAGE_NAME,errno, "tell header"));
					}

					bytes_to_read = FORMAT_LENGTH(dbin->header_format);

					/* If this is an ASCII header in binary data, there is no EOL */
					if (!IS_SEPARATE(dbin->header_format))
						if (IS_ASCII(dbin->header_format) && IS_BINARY(dbin->input_format))
							bytes_to_read -= EOL_SPACE;

					bytes_read = READ_DATA(dbin->header_file, dbin->header, bytes_to_read,NO_TAG);
					if (bytes_read != bytes_to_read)
						return(err_push(MESSAGE_NAME,errno, "Reading record header"));

					*(dbin->header + bytes_read) = '\0';

					if (!IS_SEPARATE(format))
					{
						dbin->file_location += bytes_to_read;
						dbin->bytes_available -= bytes_read;
					}

					if (dbin->state.header_delimited)
					{
						/* ff_process_variable_list can modify formats by adding
						FFV_CONVERT variables. If this happens, the
						delimited_fields arrays get out of registration
						and need to be shifted */
		
						if (dbin->header_format->num_in_list != dbin->num_header_vars)
						{
							dbin->header_delimited_fields =
								(unsigned char *)memRealloc((void *)(dbin->header_delimited_fields),
								dbin->header_format->num_in_list, "db_events PROCESS LIST dbin->header_delim_fields");
							if (!dbin->header_delimited_fields)
								return(err_push(MESSAGE_NAME,ERR_MEM_LACK, "Realloc header_delimited_fields"));
		
							shift_amount = dbin->header_format->num_in_list - dbin->num_header_vars;
							memmove((char *)dbin->header_delimited_fields + shift_amount,
								(char *)dbin->header_delimited_fields,
								dbin->num_header_vars);
		
							memMemset((void *)dbin->header_delimited_fields,'\0', shift_amount,"dbin->header_delimited_fields,'\\0',shift_amount");
							dbin->num_header_vars = dbin->header_format->num_in_list;
						}
						if ((ff_get_delimiters(dbin->header_format, dbin->header, dbin->header_delimited_fields)))
							return(err_push(MESSAGE_NAME,ERR_GET_DELIMITER, "dbin->header_format"));
					}

					/* Determine amount of data:
					When data_available is set to 0, it is time to read
					a record header. At that time, we must determine how
					much data is included in this record. We look for the variable "count" in
					the record header format, then for a "record_header" defined in an eqv table */

					count = do_get_count(FFF_FORMAT, dbin->header_format, dbin->header, dbin);
					if (count < 0)
						count = 1;

					dbin->bytes_to_read = FORMAT_LENGTH(dbin->input_format);

					/* Adjust cache size */
					dbin->cache_size = dbin->bytes_to_read * count;
					dbin->records_in_cache = count;
					dbin->data_available = dbin->cache_size;

					/* if format_type is set to FFF_HD, then only headers are
					being processed. In that case we need to skip the data and set
					data_available back to 0 */
					if (!(format_type & FFF_DATA))
					{
						if ((lseek(dbin->data_file, dbin->data_available, SEEK_CUR)) == -1L)
							return(err_push(MESSAGE_NAME,errno, "lseek on data file"));

						dbin->bytes_available -= dbin->data_available;
						dbin->data_available = 0;
					}

					dbin->state.new_record = 1;
				}

				/* If this is DATA, read it */
				if ((format->type & format_type) &&
					IS_INPUT(format) &&
					IS_DATA(format))
				{

					if (dbin->data_available == 0)
						return(EOF);
					if (dbin->bytes_available < 0)
						return(err_push(MESSAGE_NAME,errno, "Reading past EOF"));

					/* No Cache Defined */
					if (dbin->state.cache_defined == 0)
						return(err_push(MESSAGE_NAME,ERR_CACHE_DEFINED, dbin->title));

					/* Adjust cache size
							watch for cache size > UINT_MAX
							conversion of cache > UINT_MAX */
					
					/* Calculate the record length and the cache_size
						with the current number of records in the cache
						We need to detirmine the values for:
							dbin->cache_size
							dbin->records_in_cache
							bytes_to_read
					*/
					dbin->bytes_to_read = FORMAT_LENGTH(dbin->input_format);
					dbin->cache_size =  dbin->bytes_to_read * dbin->records_in_cache;
				   
					/* If cache_size is greater then UINT_MAX
						then readjust the dbin->cache_size and dbin->records_in_cache */
					if ((unsigned long)dbin->cache_size >= (unsigned long)UINT_MAX)
					{
						dbin->records_in_cache = DEFAULT_CACHE_SIZE / dbin->bytes_to_read;
						dbin->cache_size = dbin->bytes_to_read * dbin->records_in_cache;
					}                
					
					/* If there is more data then the cache_size,
						read in enough data to fill the cache */
					if (dbin->data_available > dbin->cache_size)
						bytes_to_read = (unsigned int)dbin->cache_size;

					/* Otherwise readjust cache_size, records_in_cache and bytes_to_read
						to the number of remaining data */
					else
					{
						/* to avoid the output being too large, assign the
						smaller of DEFAULT_CACHE_SIZE and the bytes remaining
						*/      
						if (dbin->data_available < DEFAULT_CACHE_SIZE)
						{
							dbin->cache_size = bytes_to_read = (unsigned int)dbin->data_available;
							dbin->records_in_cache = dbin->data_available / dbin->bytes_to_read;
						}
						else
						{
							dbin->records_in_cache = DEFAULT_CACHE_SIZE / dbin->bytes_to_read;
							dbin->cache_size = bytes_to_read = dbin->bytes_to_read * dbin->records_in_cache;
						}                
					}

					bytes_read = READ_DATA(dbin->data_file, dbin->cache, bytes_to_read,NO_TAG);
					if (bytes_read != bytes_to_read)
						return(err_push(MESSAGE_NAME,ERR_READ_FILE, "Incorrect Number of Bytes Read to Cache"));

					dbin->file_location += bytes_read;
					dbin->cache_end = dbin->cache + bytes_read - 1;
					dbin->state.cache_filled = 1;
					dbin->data_ptr = dbin->cache;
					dbin->bytes_available -= bytes_read;
					dbin->data_available -= bytes_read;
				}

				if (format->type & FFF_RETURN)
				{
					if ((lseek(dbin->data_file, EOL_SPACE, SEEK_CUR)) == -1L)
						return(err_push(MESSAGE_NAME,errno, "lseek on data file"));

					dbin->file_location += EOL_SPACE;
					dbin->bytes_available -= EOL_SPACE;
				}

				f_list = dll_next(f_list);
			}       /* End of format_list processing (while) */

			break;  /* End of PROCESS_FORMAT_LIST */

		default:
			assert(0);
			err_push(MESSAGE_NAME,ERR_DBIN_EVENT, "Unknown Data Bin Event");
			return(4);
		}       /* End of Attribute Switch */
	}       /* End of Attribute Processing (while) */
	va_end(args);
	return(0);
}

/*
 * NAME:	ff_get_delimiters
 *
 * PURPOSE:	To find the delimiters in a buffer and set variable positions
 *			in a format on the basis of the delimiters.
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * USAGE:	ff_get_delimiters(FORMAT_PTR,
 *				FF_SCRATCH_BUFFER buffer,
 *				unsigned char *list)
 *
 * ERRORS: 0
 *
 * RETURNS: void
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_get_delimiters"

static int ff_get_delimiters(FORMAT_PTR format, FF_SCRATCH_BUFFER buffer, unsigned char *list)
{
	char delimiters[] 			= {',' , '\n' , '\0'};
	FF_SCRATCH_BUFFER position 	= NULL;
	unsigned char *var_delimited = list;
	size_t offset = 1;
	VARIABLE_PTR var 		= NULL;
	VARIABLE_LIST_PTR v_list= NULL;
	
	/* Error checking on NULL paramaters format and buffer */

	assert(format && buffer && ((void *)format == format->check_address));

	position = buffer;

	/* Point at the first variable */
	v_list = FFV_FIRST_VARIABLE(format);

	while ((var = FFV_VARIABLE(v_list)) != NULL)
	{
		assert((void *)var == var->check_address);
		if (var != var->check_address)
			return(err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "variable check-address"));

		/* Check to see if it is a delimited variable */
		if (*var_delimited || (var->start_pos == 0 && !IS_CONVERT(var)))
		{
			/* offset = position of the start of the present variable. */
			var->start_pos = (unsigned short)offset;

			/* Find the next delimiter */
			offset += strcspn(position, delimiters);

			var->end_pos = (unsigned short)(offset - 1);

			position = buffer + offset;

			if (*(position - 1) == '\n')
				var->end_pos--;

			offset++;

			/* Set the list element */
			*var_delimited = 1;
		}
		else
		{
			position = buffer + var->end_pos;
			offset = (var->end_pos) ? var->end_pos : 1;
			*var_delimited = 0;
		}
		v_list = dll_next(v_list);
		++var_delimited;
	}
	return(0);
}

