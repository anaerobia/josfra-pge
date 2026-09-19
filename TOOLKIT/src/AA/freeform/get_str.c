/*
 * NAME:	ff_get_string
 *
 * PURPOSE: Reads data to an ASCII string using info from var.
 *			The output string has the precision of the input variable.
 * 			
 * AUTHOR:	S. D. Davis, National Geophysical Center, September 1990
 *				     	  
 * USAGE:	ff_get_string(
 *				VARIABLE *var, 		The input variable description 
 *				void *data_ptr,				Address of the variable 
 *				char *variable_str,			Destination (null-termined sring) 
 *				FF_TYPES_t format_type)		Input format type
 *									
 * ERRORS:
 *		Pointer not defined,"var"
 *		Possible memory corruption, "var"
 *		Pointer not defined,"data_ptr"
 *		Pointer not defined,"variable_str"
 *		Out of memory,"tmp_str"
 *		Unknown variable type,"binary var->type"
 *		Unknown format type,NULL
 *
 * COMMENTS:
 *
 * RETURNS:	if processed correctly 0, else -1 if error occurs
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#include <stdlib.h>
#include <limits.h>

#define WANT_NCSA_TYPES
#include <freeform.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_get_string"
int ff_get_string(VARIABLE_PTR var,	/* The variable description */
	FF_DATA_PTR data_ptr,			/* Address of the variable */
	char *variable_str,			/* Destination (null-terminated sring) */
	FF_TYPES_t format_type)		/* Format type */
{
	unsigned short var_length;
	unsigned int decimal_shift;

	unsigned short num_chars = 0;

	char *ch = NULL;
	char *last_char = NULL;
	char *tmp_str	= NULL;

	/*error checking on undefined parameter pointers*/
	assert(var && data_ptr && variable_str &&
		(var == var->check_address) );

	/* initialize the pointer to the beginning of the variable: */
	ch = (char *)data_ptr;
	var_length = FF_VAR_LENGTH(var);

	/* Character variables are handled the same way in either case*/
	if (IS_TEXT(var))
	{
		memcpy(variable_str, ch, var_length);
		variable_str[var_length] = STR_END;
		return(0);
	}
		
	/* so that switch will include the composite Format types */
	format_type &= FFF_FORMAT_TYPES;
	switch (format_type)
	{
		char error_buffer[_MAX_PATH];
		
		case FFF_ASCII:
		case FFF_DBASE:
	
			/* ASCII Input:
				Numeric variables must be left justified in the variable
				string for the precision determinations to work: 
	
				Move the pointer to the first non-blank character in the
				input variable string and decrease the number of characters
				to copy.
	
				Example:
					Before: ___123
					            |
							  ch      var_length = 6
	
					After:  ___123
					                 |
							        ch   var_length = 3
			*/
	
			while (*ch == ' ' && var_length)
			{
				--var_length;
				++ch;
			}
	
			/* The string is blank so fill from left with precision
			number of 0's */
			if (var_length == 0)
			{
				assert(var->precision >= 0);
				
				var_length = num_chars = (unsigned short)(var->precision + 1);
				ch = (char *)data_ptr;
				for ( ; num_chars > 0; num_chars--, ch++)
					*ch = '0';
				ch = (char *)data_ptr;
			}
			else
			{
				/* ASCII Input:
					Numeric variables can not have blanks on the end for the
					precision determinations to work:
	
					Pad the right side of the string with blanks.
	
					Example:	var_length = 6
						Before: 123___
						             |
								     tmp_str
									                       
						After:  123000
						          |
								  tmp_str
				*/
	
				tmp_str = ch + var_length - 1;
				while (*tmp_str == ' ')
				{
					*tmp_str = '0';
					tmp_str--;
				}
			}
	
			/* Copy the variable into variable_str */
			memcpy(variable_str, ch, var_length);
			variable_str[var_length] = STR_END;
		break;
	
		case FFF_BINARY:
	
			/* copy data to variable and put it into a string */
			/* tmp_str is a string created to avoid allignment errors on the SUN */
			tmp_str = (char *)memMalloc(var_length + 1, "ff_get_string: tmp_str");
			if (!tmp_str)
			{
				err_push(ROUTINE_NAME,ERR_MEM_LACK,"tmp_str");
				return(-1);
			}
	
			memMemcpy(tmp_str, (char *)data_ptr, var_length,NO_TAG);
			tmp_str[var_length] = STR_END;
	
			switch (FFV_DATA_TYPE(var))
			{
				case FFV_INT8:
					sprintf(variable_str, fft_cnv_flags[FFNT_INT8], *(int8 *)tmp_str);
				break;
				
				case FFV_UINT8:
					sprintf(variable_str, fft_cnv_flags[FFNT_UINT8], *(uint8 *)tmp_str);
				break;
				
				case FFV_INT16:
					sprintf(variable_str, fft_cnv_flags[FFNT_INT16], *(int16 *)tmp_str);
				break;
				
				case FFV_UINT16:
					sprintf(variable_str, fft_cnv_flags[FFNT_UINT16], *(uint16 *)tmp_str);
				break;
				
				case FFV_INT32:
					sprintf(variable_str, fft_cnv_flags[FFNT_INT32], *(int32 *)tmp_str);
				break;
				
				case FFV_UINT32:
					sprintf(variable_str, fft_cnv_flags[FFNT_UINT32], *(uint32 *)tmp_str);
				break;
				
				case FFV_INT64:
					sprintf(variable_str, fft_cnv_flags[FFNT_INT64], *(int64 *)tmp_str);
				break;
				
				case FFV_UINT64:
					sprintf(variable_str, fft_cnv_flags[FFNT_UINT64], *(uint64 *)tmp_str);
				break;
				
				
		
				case FFV_FLOAT32:		/* Binary to ASCII float */
					sprintf(variable_str, "%.*f", var->precision, *((float32 *)tmp_str));
				break;
		
				case FFV_FLOAT64:		/* Binary to ASCII double */
					/* The case of precision = USHRT_MAX indicates a conversion to an integer
					type variable. In these cases, the variable needs to be truncated, rather than
					rounded */
					if (var->precision == USHRT_MAX)
					{
						sprintf(variable_str, fft_cnv_flags[FFNT_INT32], (int32)*(float64 *)tmp_str);
						var->precision = 0;
					}				
					else
					{
						sprintf(variable_str, "%.*f", var->precision, *((float64 *)tmp_str));
					}
				break;
		
				default:			/* Unknown variable type */
					sprintf(error_buffer, "%d, %s:%d",
					        (int)FFV_DATA_TYPE(var), os_path_return_name(__FILE__), __LINE__);
					err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
					err_disp();
					memFree(tmp_str, "ff_get_string: tmp_str");
					return(-1);
			}

			memFree(tmp_str, "ff_get_string: tmp_str");
		break;
	
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)format_type, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(-1);
	}

	/*	Variable strings which are shorter than the precision
		can cause some problems:

		Example:	precision  = 6 String: 123 Number = 0.000123
					var_length = 3
					decimal_shift = 3

                        Before: 123____
                                |  |
                                |  last_char
                                |  destination
                                |  source

                        After:  123123
                                |
                                ch

					After:  000123
	*/
	var_length = strlen(variable_str);
	if (strchr(variable_str, '-'))
		--var_length;

	assert(var->precision >= 0);
	if (var_length <= var->precision)
	{
		decimal_shift = var->precision - var_length + 1;

		/* Left pad with zeroes */
		last_char = memStrrchr(variable_str, STR_END,NO_TAG);
		memmove(last_char + (int)(decimal_shift - var_length),
		        last_char - var_length,
		        var_length);
										  
		ch = last_char - var_length;
		while (decimal_shift--)
		{
			*ch = '0';
			ch++;
		}

		ch[var_length] = STR_END;
	}
	
	return(0);
}

