/* FILENAME:  showform.c
 *
 * CONTAINS: ff_show_format()	
 *		ff_format_info()
 *		ff_list_vars()
 */		   

#include <stdio.h>
#include <math.h>
#include <freeform.h>

static long digit_count(long l)
{
  return( l ? (long)(log10((double)labs(l)) + 1) : 1);
}


/*
 * NAME:	ff_show_format	
 *
 * PURPOSE:	List a format specification into a string.
 *
 * USAGE:	ff_show_format(FORMAT_PTR format, FF_SCRATCH_BUFFER scratch_buffer)
 *
 * RETURNS: The number of variables in the format.
 *			
 * DESCRIPTION:	This function writes a full description of a format into
 *				a character buffer. This information includes:
 *					The number of variables
 *					The maximum length of the format
 *					The format type
 *					and
 *					the name
 *					the start position
 *					the end position
 *					the type
 *					the precision
 *				for each variable.
 *
 * Any new text is appended to bufsize.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *  
 * AUTHOR:	T. Habermann, NGDC, June, 1990
 *
 * COMMENTS:	The buffer must be large enough to hold the information.
 *
 * ERRORS:
 *		Problem writing to file,"No Information written"
 *		Problem showing variables, NULL
 *
 * KEYWORDS:
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_show_format"

int ff_show_format(FORMAT_PTR format, FF_BUFSIZE_PTR bufsize)
{
	int num_chars = 0;
	int num_vars = 0;

	/* Error checking on NULL parameters and corrupt memory*/
	assert(format && bufsize &&
		(format == format->check_address) );
	assert(bufsize->bytes_used <= bufsize->total_bytes);

	num_chars = ff_format_info(format, bufsize);
	
	if (num_chars <= 0) {
		err_push(ROUTINE_NAME,ERR_WRITE_FILE,"No Information written");
		return(0);
	}

	num_vars = ff_list_vars(format, bufsize);
	
	if ((num_vars <= 0) && !(format->type & FFF_RETURN)) {
		err_push(ROUTINE_NAME,ERR_SHOW_VARS, NULL);
		return(0);
	}

	return(num_vars);
}


/*
 * NAME:	ff_format_info
 *			   
 * PURPOSE:	To get the header information from a format.
 *				     
 * USAGE:	ff_format_info(FORMAT *format, FF_SCRATCH_BUFFER scratch_buffer)
 *		  
 * RETURNS:	the number of characters written
 *
 * DESCRIPTION:	This function prints the information from a format header
 *				into a character buffer. This information includes:
 *					the number of variables
 *					the maximum length of the format
 *					the format type
 *					
 * Any new text is appended to bufsize.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * ERRORS:
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	The buffer must be large enough to hold the information.
 *
 * KEYWORDS:	
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_format_info"

int ff_format_info(FORMAT_PTR format, FF_BUFSIZE_PTR bufsize)
{
	char *cp = NULL;
	size_t old_byte_count = 0;

	/* Error checking on NULL parameters and corrupt memory*/
	assert(format && bufsize &&
		(format == format->check_address) );
	assert(bufsize->bytes_used <= bufsize->total_bytes);
																	
	cp = bufsize->buffer + bufsize->bytes_used;
	old_byte_count = (size_t)bufsize->bytes_used;
	
	sprintf(cp, "\n");

	bufsize->bytes_used += strlen(cp);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	CH_TO_END(cp);
	
	if (IS_DATA(format) && IS_INPUT(format))
		sprintf(cp, "Input data");
	else if (IS_DATA(format) && IS_OUTPUT(format))
		sprintf(cp, "Output data");
	else if (IS_HD(format) && IS_FILE(format) && IS_INPUT(format))
		sprintf(cp, "Input file header");
	else if (IS_HD(format) && IS_FILE(format) && IS_OUTPUT(format))
		sprintf(cp, "Output file header");
	else if (IS_HD(format) && IS_REC(format) && IS_INPUT(format))
		sprintf(cp, "Input record header");
	else if (IS_HD(format) && IS_REC(format) && IS_OUTPUT(format))
		sprintf(cp, "Output record header");
	else if (IS_HD(format) && IS_FILE(format))
		sprintf(cp, "File header");
	else if (IS_HD(format) && IS_REC(format))
		sprintf(cp, "Record header");
	else
		sprintf(cp, "Irregular");

	bufsize->bytes_used += strlen(cp);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	CH_TO_END(cp);

	/* the format type must be looked up */
	sprintf(cp," format\t(%s)\n", format->locus);

	bufsize->bytes_used += strlen(cp);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	CH_TO_END(cp);
	
	sprintf(cp,"%s\t%s%s%s\n",
			    ff_lookup_string(format_types, FFF_TYPE(format)),
			    strlen(format->name) ? "\"" : "",
			    strlen(format->name) ? format->name : "",
			    strlen(format->name) ? "\"" : "");

	bufsize->bytes_used += strlen(cp);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	CH_TO_END(cp);
	
	sprintf(cp, "The format contains %u variable%s; length is %u.\n",
		      format->num_in_list, format->num_in_list == 1 ? "" : "s",
		      FORMAT_LENGTH(format));

	bufsize->bytes_used += strlen(cp);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	CH_TO_END(cp);

	return((size_t)bufsize->bytes_used - old_byte_count);
}

/*
 * NAME:	ff_list_vars
 *
 * PURPOSE:	List the variables in a format into text_buffer.
 *										     
 * USAGE:	ff_list_vars(FORMAT_PTR format, char *text_buffer)
 *
 * RETURNS:	The number of variables in FORMAT
 *
 * DESCRIPTION:	This function writes the format specification information
 * into a text buffer. The text buffer consists of lines (separated with the
 * newline character, i.e., '\n') with the following fields for each variable:
 *
 * 1) name,
 * 2) start position,
 * 3) end position,
 * 4) type, and
 * 5) precision
 * 
 * Each field is separated by space characters (' ', i.e., '\x20').  Each
 * field is justified within a fixed field whose width is determined by the
 * maximum length for that field in the format specification (nice fixed 
 * columns).
 *
 * Any new text is appended to bufsize.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * ERRORS:
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:	The buffer must be large enough to hold the information.
 *
 * KEYWORDS:	
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_list_vars"

int ff_list_vars(FORMAT_PTR format, FF_BUFSIZE_PTR bufsize)
{
	int                num_variables = 0,
	                   var_fw        = 0, /* name field width */
	                   start_pos_fw  = 0, /* start pos field width */
	                   end_pos_fw    = 0, /* end pos field width */
	                   type_fw       = 0, /* type field width */
	                   prec_fw       = 0; /* precision field width */
	VARIABLE_LIST_PTR  v_list        = NULL;
	char              *cp            = NULL;
	BOOLEAN            any_neg_prec  = FALSE;

	/* Error checking on NULL parameters and corrupt memory */
	assert(format && bufsize &&
		(format == format->check_address) );
	assert(bufsize->bytes_used <= bufsize->total_bytes);

	any_neg_prec = FALSE;
	v_list = FFV_FIRST_VARIABLE(format);
	while (FFV_VARIABLE(v_list))
	{
		++num_variables;
		
		var_fw = max(var_fw, (int)strlen(FFV_VARIABLE(v_list)->name));
		start_pos_fw = max(start_pos_fw
		                   , (int)digit_count(FFV_VARIABLE(v_list)->start_pos));
		end_pos_fw = max(end_pos_fw
		                 , (int)digit_count(FFV_VARIABLE(v_list)->end_pos));
		type_fw = max(  type_fw
		              , (int)strlen(ff_lookup_string(  variable_types
		                                             , FFV_VARIABLE(v_list)->type)));
		prec_fw = max(prec_fw
		              , (int)digit_count(FFV_VARIABLE(v_list)->precision));
		if (FFV_VARIABLE(v_list)->precision < 0)
			any_neg_prec = TRUE;

		v_list = dll_next(v_list);
	}
	
	if (any_neg_prec)
		prec_fw++;

	v_list = FFV_FIRST_VARIABLE(format);
	cp = bufsize->buffer + bufsize->bytes_used;
	while (FFV_VARIABLE(v_list))
	{
		sprintf(  cp, "%-*s %*hu %*hu %*s %*hd\n"
		        , var_fw, FFV_VARIABLE(v_list)->name
		        , start_pos_fw, FFV_VARIABLE(v_list)->start_pos
		        , end_pos_fw, FFV_VARIABLE(v_list)->end_pos
		        , type_fw, ff_lookup_string(  variable_types
		                                    , FFV_VARIABLE(v_list)->type)
		        , prec_fw, FFV_VARIABLE(v_list)->precision
		       );

		bufsize->bytes_used += strlen(cp);
		assert(bufsize->bytes_used <= bufsize->total_bytes);
		CH_TO_END(cp);

		v_list = dll_next(v_list);
	}

	*cp = STR_END;

	return(num_variables);
}


void ff_show_format_description(FORMAT_PTR format, FF_BUFSIZE_PTR bufsize)
/*****************************************************************************
 * NAME:  ff_show_format_description()
 *
 * PURPOSE:  Show the format description, with exact syntax
 *
 * USAGE:  ff_show_format_description(format, bufsize);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Much like ff_show_format(), except this function returns
 * a text buffer which is syntactically correct, such that it can be parsed
 * by db_make_format_list().
 *
 * Any new text is appended to bufsize.
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
	char *cp = NULL;

	assert(format);
	assert(format == format->check_address);
	assert(bufsize);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	
	if (!bufsize)
		return;

	cp = bufsize->buffer + bufsize->bytes_used;
	sprintf(  cp, "%s\t\"%s\"\n"
	        , ff_lookup_string(format_types, FFF_TYPE(format))
	        , format->name);
	
	bufsize->bytes_used += strlen(cp);
	assert(bufsize->bytes_used <= bufsize->total_bytes);
	CH_TO_END(cp);

	ff_list_vars(format, bufsize);
}

