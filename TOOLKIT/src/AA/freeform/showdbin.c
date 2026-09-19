/*
 * NAME:        db_show
 *              
 * PURPOSE:     db_show shows various attributes of a data bin.
 *
 * USAGE:       db_show(DATA_BIN_PTR, attribute, [args], attribute, args,... NULL)
 *
 * RETURNS:     0 if all goes well.
 *
 * DESCRIPTION: This function provides access to the attributes and methods
 *                              of the DATA_BINs. The function processes a list of argument
 *                              groups which have the form: attribute [arguments]. The list
 *                              of groups is terminated by a NULL, which ends processing.
 *                              The presently supported attributes and their arguments are:
 *
 * SYSTEM DEPENDENT FUNCTIONS:  Processing of RETURN formats assumes EOL = 2 bytes
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * COMMENTS:
 *                                 
 * KEYWORDS: databins
 *
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <freeform.h>
#include <databin.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "Show_dbin"
#define LINE_LENGTH             256

int db_show(DATA_BIN_PTR dbin, ...)
{
	va_list args;

	char *buffer = NULL;
	char *tmp_buffer = NULL;
	char error_buff[MAX_ERRSTR_BUFFER];
	SCRATCH_BUFFER ch_ptr = NULL;
	FF_BUFSIZE bufsize;

	FORMAT_PTR format = NULL;

	int attribute;
	int error = 0;

	unsigned flag;

	long *l_ptr = NULL;

	LEVEL *level = NULL;

	assert(dbin && ((void *)dbin == dbin->check_address));

	ch_ptr = dbin->buffer;

	va_start(args, dbin);

	while ( (attribute = va_arg (args, int)) != 0 ) {

		assert((void *)dbin == dbin->check_address);

		switch (attribute) {

		/*
		 * MESSAGE:     BUFFER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET) char *buffer
		 *
		 * PRECONDITIONS:       buffer must be allocated externally.
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Sets dbin->buffer to argument and initializes
		 *                              first character to '\0'.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    This call will become vestigial when the memory
		 *                              management moves into the bin library.
		 *
		 * KEYWORDS:    
		 *
		 */
		case BUFFER:
			ch_ptr = va_arg (args, SCRATCH_BUFFER);
			if(attribute == BUFFER)*(ch_ptr) = '\0';
			break;

		/*
		 * MESSAGE:     BUFFER_IN
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SET) char *buffer
		 *
		 * PRECONDITIONS:       buffer must be allocated externally.
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Sets dbin->buffer to argument and does not
		 *                              initialize first character to '\0'.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case BUFFER_IN:
			ch_ptr = va_arg (args, SCRATCH_BUFFER);
			break;

		/*
		 * MESSAGE:     DBIN_BOX_NUMBER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) long *number
		 *
		 * PRECONDITIONS:       dbin->index->levels must be specified
		 *                                      
		 *
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Determine the box which a data point is in given:
		 *                              1) the name     of the variable (set by BUFFER) and
		 *                              2) a pointer to a long with an assumed precision
		 *                                      of three (to match the index).
		 *                              The box number is returned as an int.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    This Cannot Work In Present State.
		 *                              The name should come in as a seperate argument.
		 *              (kbf) The fact that this message returns anything when not an error
		 *              is fundamentally wrong and against the freeform conventions.
		 *
		 * KEYWORDS:    
		 *
		 */
		case DBIN_BOX_NUMBER:

			l_ptr = va_arg(args, long *);
			level = dbin->index->levels;
			while(memStrcmp(ch_ptr, level->name, "showdbin DBIN_BOX_NUMBER"))++level;
			return((int)((*l_ptr - level->min)/level->size));

		/*
		 * MESSAGE:     DBIN_BYTE_COUNTS
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   Data bin element type, long *bytes, .... END_ARGS
		 *
		 * PRECONDITIONS:       element must be defined for meaningful count,
		 *                                      if element is not defined count = 0.
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Determine the size of various parts of a data_bin.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case DBIN_BYTE_COUNTS:

#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin DBIN_BYTE_COUNTS"

			/* Loop over the arguments */
			while ( (attribute = va_arg (args, int)) != END_ARGS ) {
				switch (attribute) {
				case DBIN_FORMAT:
				case INPUT_FORMAT:
				case OUTPUT_FORMAT:
				case HEADER_FORMAT:                             
					flag = va_arg(args, unsigned int);
					l_ptr = va_arg(args, long *);
					format = db_find_format(dbin->format_list, FFF_GROUP, flag, END_ARGS); 
					if(format)      *l_ptr = FORMAT_LENGTH(format);
					else            *l_ptr = 0L;
					break;
					
				case DBIN_INPUT_CACHE:
					if(!dbin->state.cache_defined){
						err_push(MESSAGE_NAME, ERR_CACHE_DEFINED, "dbin->state.cache_defined");
						return(ERR_CACHE_DEFINED);
					}
					if(!dbin->state.cache_filled){
						err_push(MESSAGE_NAME, ERR_CACHE_DEFINED, "Cache not filled");
						return(ERR_CACHE_DEFINED);
					}
					if (!dbin->input_format){
						err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "dbin->input_format, BYTE_COUNTS");
						return(ERR_BINFORM_DEFINED);
					}
					/* Determine the number of bytes in the cache */
					l_ptr = va_arg(args, long *);
		
					*l_ptr = dbin->cache_end - dbin->cache + 1;
					break;
					
				case DBIN_OUTPUT_CACHE:
					l_ptr = va_arg(args, long *);
		
					if (!dbin->output_format){
						err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "dbin->output_format");
						return(ERR_BINFORM_DEFINED);
					}
		
					/* convert tmp_long to number of output bytes */
					*l_ptr = dbin->records_in_cache * FORMAT_LENGTH(dbin->output_format);
					break;
				}
			}
			break;

		/*
		 * MESSAGE:     DBIN_FILE_NAME
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) None
		 *
		 * PRECONDITIONS:       file_name must be defined
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Writes the current file name for the data bin into
		 *                              it's buffer.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case DBIN_FILE_NAME:

			if(dbin->file_name)
				memStrcpy(ch_ptr, dbin->file_name,NO_TAG);

			else
				memStrcpy(ch_ptr, "Undefined",NO_TAG);

			ch_ptr += strlen(ch_ptr);

			break;

		/*
		 * MESSAGE:     DBIN_HEADER
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) char *destination 
		 *
		 * PRECONDITIONS:       header must be defined
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Writes the current header into the buffer 
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case DBIN_HEADER:

#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin DBIN_HEADER"

			if(dbin->state.header_defined){

				buffer = va_arg(args, DATA_BUFFER);
				format = va_arg(args, FORMAT_PTR);

				if(format){ /* Convert header */

					if (format != format->check_address) {
						err_push(MESSAGE_NAME, ERR_MEM_CORRUPT, "case DBIN_HEADER");
						return(ERR_MEM_CORRUPT);
					}

					tmp_buffer = (char *)memMalloc((format->num_in_list * LINE_LENGTH) + 1, "Show_dbin DBIN_HEADER: tmp_buffer");
					if(!tmp_buffer){
						err_push(MESSAGE_NAME, ERR_MEM_LACK, "tmp buffer in show header");
						return(ERR_MEM_LACK);
					}
	
					error = db_events(dbin, DBIN_CONVERT_HEADER, tmp_buffer, format, END_ARGS);
					if(error){
						err_push(MESSAGE_NAME, ERR_CONVERT, "Converting header");
						return(ERR_CONVERT);
					}
					if((ff_show_variables(tmp_buffer, buffer, format)) == 0){
						err_push(MESSAGE_NAME, ERR_SHOW_VARS, "header variable");
						memFree(tmp_buffer, "Show_dbin DBIN_HEADER: tmp_buffer");
						return(ERR_SHOW_VARS);
					}
	
					memFree(tmp_buffer,"Show_dbin DBIN_HEADER: tmp_buffer");
				}
				else { /* show current header variables */
	
					if((ff_show_variables(dbin->header, buffer, dbin->header_format)) == 0){
						err_push(MESSAGE_NAME, ERR_SHOW_VARS, "header");
						return(ERR_SHOW_VARS);
					}
				}
			}
			else memStrcpy(ch_ptr, "Undefined",NO_TAG);

			ch_ptr += strlen(ch_ptr);
			break;


		/*
		 * MESSAGE:     DBIN_RECORD_COUNT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) long *input records
		 *
		 * PRECONDITIONS:       cache must be defined and filled.
		 *                                      input and output formats must be defined.
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Determine the number of records in the cache.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case DBIN_RECORD_COUNT:

#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin DBIN_RECORD_COUNT"

			if (!(dbin->state.cache_defined)) {
				err_push(MESSAGE_NAME, ERR_CACHE_DEFINED, "DBIN_RECORD_COUNT");
				return(ERR_CACHE_DEFINED);
			}
			if (!dbin->input_format) {
				err_push(MESSAGE_NAME, ERR_BINFORM_DEFINED, "dbin->input_format, RECORD_COUNT");
				return(ERR_BINFORM_DEFINED);
			}                               
																 
			/* Determine the number of input records */
			l_ptr = va_arg(args, long *);

			*l_ptr = 0L;

			if(dbin->state.cache_filled){

				/* assign tmp_long to number of input records */
				*l_ptr = dbin->records_in_cache;
			}
			
			break;
			
/*
		 * MESSAGE:     FORMAT_LIST
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) FFF_VARIABLES, FFF_INFO
		 *
		 * PRECONDITIONS:       
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Provides information regarding formats in the
		 *                              current format list.
		 *                              If the argument is FFF_INFO, the header is listed.
		 *                              If the argument is FFF_VARIABLES, the variables
		 *                                      are listed.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case SHOW_FORMAT_LIST:
#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin FORMAT_LIST"

			flag = va_arg(args, unsigned int);
			
			/* Traverse format list */
			if (dbin->format_list){
				if (db_show_format_list(dbin->format_list, flag, ch_ptr))
					return(err_push(MESSAGE_NAME, ERR_SHOW_FORM, "dbin->format_list"));
			}
			else memStrcpy(ch_ptr, "Undefined",NO_TAG);

			ch_ptr += strlen(ch_ptr);

			break;

		/*
		 * MESSAGE:     INPUT_FORMAT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) None
		 *
		 * PRECONDITIONS:       
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Writes the current input_format for the data bin into
		 *                              it's buffer.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case INPUT_FORMAT:

#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin INPUT_FORMAT"

			if (dbin->input_format)
			{
				bufsize.buffer = ch_ptr;
				/* next assignment below wraps to USHRT_MAX with compiler warning,
				   figure out exact size later, somehow...
				*/
				bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
				bufsize.bytes_used = 0;

				if ((ff_show_format(dbin->input_format, &bufsize)) == 0)
				{
					sprintf(error_buff,"INPUT_FORMAT -> %s",dbin->title);
					err_push(MESSAGE_NAME, ERR_SHOW_FORM, error_buff);
					return(ERR_SHOW_FORM);
				}
				ch_ptr = bufsize.buffer;
			}
			else memStrcpy(ch_ptr, "Undefined",NO_TAG);

			ch_ptr += strlen(ch_ptr);

			break;

		/*
		 * MESSAGE:     OUTPUT_FORMAT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) None
		 *
		 * PRECONDITIONS:       
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Writes the current output_format for the data bin into
		 *                              it's buffer.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case OUTPUT_FORMAT:

#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin OUTPUT_FORMAT"

			if(dbin->output_format)
			{
				bufsize.buffer = ch_ptr;
				/* assignment below wraps to USHRT_MAX with compiler warning,
				   figure out exact size later, somehow...
				*/
				bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
				bufsize.bytes_used = 0;

				if ((ff_show_format(dbin->output_format, &bufsize)) == 0)
				{
					sprintf(error_buff,"OUTPUT_FORMAT -> %s",dbin->title);
					err_push(MESSAGE_NAME, ERR_SHOW_FORM, error_buff);
					return(ERR_SHOW_FORM);
				}
				ch_ptr = bufsize.buffer;
			}
			else memStrcpy(ch_ptr, "Undefined",NO_TAG);

			ch_ptr += strlen(ch_ptr);

			break;

		/*
		 * MESSAGE:     HEADER_FORMAT
		 *
		 * OBJECT TYPE: DATA_BIN
		 *
		 * ARGUMENTS:   (TO SHOW) None
		 *
		 * PRECONDITIONS:       
		 *                                      
		 * CHANGES IN STATE:    None
		 *
		 * DESCRIPTION: Writes the current header_format for the data bin into
		 *                              it's buffer.
		 *
		 * ERRORS:      None
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case HEADER_FORMAT:

#undef MESSAGE_NAME
#define MESSAGE_NAME "Show_dbin HEADER_FORMAT"

			if(dbin->header_format)
			{
				bufsize.buffer = ch_ptr;
				/* assignment below wraps to USHRT_MAX with compiler warning,
				   figure out exact size later, somehow...
				*/
				bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
				bufsize.bytes_used = 0;

				if ((ff_show_format(dbin->header_format, &bufsize)) == 0)
				{
					sprintf(error_buff,"HEADER_FORMAT -> %s",dbin->title);
					err_push(MESSAGE_NAME, ERR_SHOW_FORM, error_buff);
					return(ERR_SHOW_FORM);
				}
				ch_ptr = bufsize.buffer;
			}
			else memStrcpy(ch_ptr, "Undefined",NO_TAG);

			ch_ptr += strlen(ch_ptr);

			break;
			
		default:
			err_push(ROUTINE_NAME, ERR_SHOW_DBIN, "Unknown event");
			return(ERR_SHOW_DBIN);
		}               
	}
	va_end(args);
	return(0);
}


