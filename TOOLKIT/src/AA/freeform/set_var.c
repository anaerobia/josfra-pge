/* FILENAME:  set_var.c
 *
 * CONTAINS:  set_var()
 *            get_var_space()
 *            expandvar()
 *
 */

/*
 * NAME:                set_var
 *              
 * PURPOSE:     To set a variable back to a data buffer.
 *              
 *
 * USAGE:       int set_var(VARIABLE_LIST *var, char *buffer, char *variable_str,
 *                                        FORMAT *format)
 *
 * RETURNS:     if success, return 0, otherwise return -1.
 *
 * DESCRIPTION: This function put variable value stored in variable_str as ASCII 
 * into a buffer called buffer according format specified by var and format->type
 *
 * This function overwrites the variable space within buffer with the value given
 * by variable_str, adjusting variable spaces within the buffer for size
 * differences if appropriate.
 *
 * ERRORS:
 * Data overflow, var->name
 * Data roundup error, var->name
 * Unknown variable type, "ASCII"
 * Unknown variable type, "binary"
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 * modified by  Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	10/17/95		-rf01
 *		(FF_DATA_BUFFER) cast needed for CW
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define WANT_NCSA_TYPES
#include <freeform.h>
#include <databin.h>
#include <os_utils.h>

#ifdef PROTO
static int get_var_space(FORMAT_PTR format, VARIABLE_PTR var, char *buffer, int request);
static BOOLEAN expandvar(FORMAT_PTR format, VARIABLE_PTR var, char *buffer, int nbytes);
#else
static int get_var_space();
static BOOLEAN expandvar();
#endif

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "set_var"

#ifdef PROTO
int set_var(VARIABLE *var,      /* The variable description */
	char *buffer,                           /* data buffer pointer */
	char *variable_str,                     /* Data string(null-terminated sring) */
	FORMAT *format)                 /* Format pointer */
	
#else
int set_var(var, buffer, variable_str, format)
	VARIABLE *var;
	char *buffer;
	char *variable_str;
	FORMAT *format;
#endif

{
	char *var_loc, n_str[MAX_PV_LENGTH];
	int precision, allowed; /* allowed data space for a var */
	size_t str_length;
	big_var_type big_var;
	
	assert(var && format && buffer && variable_str &&
	   ((void *)format == format->check_address) &&
	   ((void *)var == var->check_address));

	var_loc = buffer + var->start_pos - 1;

	(void)os_str_trim_whitespace(variable_str, variable_str);
	str_length = strlen(variable_str);

	if (os_strcmpi(variable_str, "undefined") == 0)
	{
		if (FF_VAR_LENGTH(var) > 0 )
			get_var_space(format, var, buffer, 0);
		return(0);
	}

	switch (format->type & FFF_FORMAT_TYPES)
	{
		char error_buffer[_MAX_PATH];
		
		case FFF_ASCII:
			if (!IS_TEXT(var))
				if (ff_string_to_binary(variable_str, FFV_DATA_TYPE(var), (FF_DATA_BUFFER)&big_var)) /* (FF_DATA_BUFFER) cast needed for CW -rf01 */
					return(-1);

			switch (FFV_DATA_TYPE(var))
			{
				case FFV_TEXT:                  /* string to ASCII char */
					allowed = get_var_space(format, var, buffer, str_length);
					memcpy(var_loc, variable_str, allowed);
				break;

				case FFV_INT8:
					sprintf(n_str, fft_cnv_flags_width[FFNT_INT8],
					        (int)FF_VAR_LENGTH(var), *(int8 *)&big_var);
				break;
				
				case FFV_UINT8:
					sprintf(n_str, fft_cnv_flags_width[FFNT_UINT8],
					        (int)FF_VAR_LENGTH(var), *(uint8 *)&big_var);
				break;
				
				case FFV_INT16:
					sprintf(n_str, fft_cnv_flags_width[FFNT_INT16],
					        (int)FF_VAR_LENGTH(var), *(int16 *)&big_var);
				break;
				
				case FFV_UINT16:
					sprintf(n_str, fft_cnv_flags_width[FFNT_UINT16],
					        (int)FF_VAR_LENGTH(var), *(uint16 *)&big_var);
				break;
				
				case FFV_INT32:
					sprintf(n_str, fft_cnv_flags_width[FFNT_INT32],
					        (int)FF_VAR_LENGTH(var), *(int32 *)&big_var);
				break;
				
				case FFV_UINT32:
					sprintf(n_str, fft_cnv_flags_width[FFNT_UINT32],
					        (int)FF_VAR_LENGTH(var), *(uint32 *)&big_var);
				break;
				
				case FFV_INT64:
					sprintf(n_str, fft_cnv_flags_width[FFNT_INT64],
					        (int)FF_VAR_LENGTH(var), *(int64 *)&big_var);
				break;
				
				case FFV_UINT64:
					sprintf(n_str, fft_cnv_flags_width[FFNT_UINT64],
					        (int)FF_VAR_LENGTH(var), *(uint64 *)&big_var);
				break;
				
				case FFV_FLOAT32:         /* string to ASCII float */
					sprintf(n_str, "%*.*f", (int)FF_VAR_LENGTH(var),
					        (int)var->precision, *(float32 *)&big_var);
					str_length = strlen(n_str);
					allowed = get_var_space(format, var, buffer, str_length);
					if (allowed != (int)str_length)
					{
						if (((int)str_length - allowed <= (int)(var->precision + 1)) &&
						    var->precision > 0 )
						{
							memcpy(var_loc, n_str, (size_t)allowed);
							err_push(ROUTINE_NAME,ERR_DATA_ROUNDUP, var->name);
						}
						else
							err_push(ROUTINE_NAME,ERR_OVERFLOW_UNCHANGE, var->name);
					}
					else
						memcpy(var_loc, n_str, (size_t)allowed);
				break;
	
				case FFV_FLOAT64:                /* string to ASCII double */
					sprintf(n_str, "%*.*E", (int)FF_VAR_LENGTH(var),
					        (int)var->precision, *(float64 *)&big_var);
					str_length = strlen(n_str);
					allowed = get_var_space(format, var, buffer, str_length);
					if (allowed != (int)str_length)
						if (((int)str_length - allowed <= (int)(var->precision + 1)) &&
						    var->precision > 0 )
						{
							precision = var->precision - (str_length - allowed);
							if (precision == -1)
								precision = 0;
							sprintf(n_str, "%*.*E", (int)FF_VAR_LENGTH(var),
							        (int)precision, *(float64 *)&big_var);
							err_push(ROUTINE_NAME,ERR_DATA_ROUNDUP, var->name);
							memcpy(var_loc, n_str, (size_t)allowed);
						}
						else 
							err_push(ROUTINE_NAME,ERR_OVERFLOW_UNCHANGE, var->name);
					else
						memcpy(var_loc, n_str, (size_t)allowed);
				break;
	
				default:                        /* Unknown variable type */
					sprintf(error_buffer, "%d, %s:%d",
					        (int)FFV_DATA_TYPE(var), os_path_return_name(__FILE__), __LINE__);
					err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
					err_disp();
					return(-1);
			} /* switch on var->type, case FFF_ASCII in switch on format->type */
		
			if (IS_INTEGER(var))
			{
				str_length = strlen(n_str);
				allowed = get_var_space(format, var, buffer, str_length);
				if (allowed != (int)str_length)
					err_push(ROUTINE_NAME,ERR_OVERFLOW_UNCHANGE, var->name);
				else
					memcpy(var_loc, n_str, (size_t)allowed);
			}
			
		break;
	
		case FFF_BINARY:
			if (IS_TEXT(var))
			{
				memcpy(var_loc, variable_str, min(str_length, FF_VAR_LENGTH(var)));
			}
			else
			{
				if (ff_string_to_binary(variable_str, FFV_DATA_TYPE(var), var_loc))
					return(-1);
			}
		break;
		
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)format->type & FFF_FORMAT_TYPES, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(-1);
	} /* switch on format->type) */

	return(0);
}

/*
 * NAME:		get_var_space
 *		
 * PURPOSE:	To meet the user required var space	
 *
 * USAGE:	int get_var_space(FORMAT_PTR format, VARIABLE_LIST_PTR var, char *buffer, int type, int request) 
 *
 * RETURNS:	 Requested variable length if header format is varied and variable
 * space can be adjusted, otherwise the minimum of the requested and actual
 * variable length.
 *
 * DESCRIPTION:	 If (header_) type is varied, then expandvar is called with
 * the difference of request and FF_VAR_LENGTH(var), and if expandvar succeeds,
 * request is returned.  If expandvar fails, or if type is not varied, then the
 * minimum of request and FF_VAR_LENGTH(var) is returned.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * GLOBALS:
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * ERRORS:
 *
 * KEYWORDS:	
 *
 */

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "get_var_space"

#ifdef PROTO
static int get_var_space(FORMAT_PTR format, VARIABLE_PTR var, char *buffer, int request)
#else
static int get_var_space(format, var, buffer, request)
	FORMAT_PTR format;
	VARIABLE_PTR var;
	char *buffer;
	int request;
#endif
{
	assert(format);
	assert(format == format->check_address);
	
	if (!format)
		return(0);
	
	if (format != format->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
		return(0);
	}
	
	assert(var);
	assert(var == var->check_address);
	
	if (!var)
		return(0);
	
	if (var != var->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "variable check-address");
		return(0);
	}

	/* make sure the minimum length for a variable is 0, (undefined) */
	if (var->end_pos < var->start_pos)
		var->end_pos = var->start_pos - 1; 
	
	if ((int)FF_VAR_LENGTH(var) == request)
		return(request);

	if (IS_VARIED(format))
		/* variable_length format, try to expand  the var space */
		if (expandvar(format, var, buffer, request - FF_VAR_LENGTH(var)))
			return(request);

	return(min(request, (int)FF_VAR_LENGTH(var)));
}      

/*
 * NAME:		expandvar
 *		
 * PURPOSE:		To expand variable length of a var in header buffer by n bytes and
 *				to change corresponding header format structure. 
 *
 * USAGE:		BOOLEAN expandvar(FORMAT_PTR format, VARIABLE_LIST_PTR var, 
 *									char *buffer, int nbytes)
 *
 * RETURNS:		If success, returns TRUE, otherwise, FALSE.
 *
 * DESCRIPTION:	
 *
 * This function adjusts the variable space within buffer by nbytes number
 * of characters.  If nbytes is positive, then variable spaces following the
 * given variable are moved by nbytes number of bytes into higher memory
 * address (to the right).  If nbytes is negative, then variable spaces
 * following the given variable are moved by nbytes number of bytes to the
 * left.
 *
 * Moving buffer contents to the right is safe as long as long as the net
 * offset does not exceed the extra space allocated to dbin->header in the
 * DEFINE_HEADER event of db_set() -- currently 501 bytes.
 *
 * The format->max_length is changed to reflect the repositioning, as are the
 * var->start_pos and var->end_pos values for the given variable (var->end_pos
 * only) and subsequent variables.
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBALS: 	none
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	Data bin, Format, variable-length header, variable_list.
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "expandvar"

#ifdef PROTO
static BOOLEAN expandvar(FORMAT_PTR format, VARIABLE_PTR var, char *buffer, int nbytes)
#else
static BOOLEAN expandvar(format, var, buffer, nbytes)
	FORMAT_PTR format;
	VARIABLE_PTR var;
	char *buffer;
	int nbytes;
#endif
{
	VARIABLE_PTR temp_var;
	VARIABLE_LIST_PTR v_list;
	
	assert(format);
	assert(format == format->check_address);
	assert(var);
	assert(var == var->check_address);
	assert(buffer);
	
	if (format == NULL)
		return(FALSE);

	if (format != format->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
		return(FALSE);
	}
	if (var == NULL)
		return(FALSE);

	if (var != var->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "variable check-address");
		return(FALSE);
	}
	if (buffer == NULL)
		return(FALSE);

	memmove(buffer + var->end_pos + nbytes,
	        buffer + var->end_pos, format->max_length - var->end_pos);

	/* update format->max_length and variables' start_pos and end_pos */
	format->max_length += nbytes;
	v_list = FFV_FIRST_VARIABLE(format);
	while ((temp_var = FFV_VARIABLE(v_list)) != NULL)
	{
		assert(temp_var == temp_var->check_address);
		if (temp_var != temp_var->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "Variable check-address");
			return(FALSE);
		}
		assert(temp_var->start_pos <= temp_var->end_pos);
		
		if (temp_var->start_pos > var->end_pos)
		{
			temp_var->start_pos += nbytes;
			temp_var->end_pos += nbytes;
		}
		v_list = dll_next(v_list);
	}

	var->end_pos += nbytes;

 	return(TRUE);
} 

