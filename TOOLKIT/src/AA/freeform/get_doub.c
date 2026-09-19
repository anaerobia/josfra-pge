/*
 * NAME:	ff_get_double
 *
 * PURPOSE:	This is the double version of ff_get_value.
 *			IT ADJUSTS THE PRECISION OF BINARY VARIABLES AS DOUBLES.
 *			This is the fastest technique.
 *		
 *			This is similar to ff_get_valued.c, but the destination is cast
 *			a double.
 *			
 * AUTHOR:	Written by Ted Habermann, National Geophysical Data Center, Oct. 1990.
 *
 * USAGE:	ff_get_double(
 *				VARIABLE_PTR var, The variable description 
 *  			FF_DATA_PTR data_src, The address of the variable 
 *				double *data_dest, The destination for the variable 
 *				FF_TYPES_t format_type) The Format Type 
 *
 * ERRORS:
 *		Unknown variable type,"DBASE or ASCII var->type"
 *		Unknown variable type,"Binary var->type"
 *		Unknown format type, NULL
 *
 * COMMENTS:
 *
 * RETURNS:	0 if successful, else -1 if error occurred
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <freeform.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_get_double"

int ff_get_double(VARIABLE_PTR var,	/* The variable description */
		FF_DATA_PTR data_src,				/* The address of the variable */
		double *data_dest,				/* The destination for the variable */
		FF_TYPES_t format_type)		/* The Format Type */
{

	int error = 0;
	char scratch_buffer[_MAX_PATH];
	char *endptr = NULL;

	/* Error checking for NULL pointer parameters */
	assert(var && data_src && data_dest && scratch_buffer &&
		(var == var->check_address) );

	/* Create a double variable in align_var:
		The original approach was to call ff_get_string, adjust the precision
		of the string, and then call atof. On 5/23/94 this approach was replaced
		by the present one: call atof ASAP and deal with the precision as it
		is handled in the binary case below.*/

	/* so that switch will include the composite Format types */
	format_type &= FFF_FORMAT_TYPES;
	switch (format_type)
	{
		case FFF_ASCII:
		case FFF_DBASE:
		
			/* Copy the string */
			(void)memcpy(scratch_buffer, data_src, FF_VAR_LENGTH(var));
			scratch_buffer[FF_VAR_LENGTH(var)] = STR_END;
						
			switch (FFV_DATA_TYPE(var))
			{
				case FFV_TEXT:
					memcpy(data_dest, scratch_buffer,  FF_VAR_LENGTH(var));
				break;
				
				case FFV_INT8:
				case FFV_UINT8:
				case FFV_INT16:
				case FFV_UINT16:
				case FFV_INT32:
				case FFV_UINT32:
				case FFV_INT64:
				case FFV_UINT64:
			  case FFV_FLOAT32:
			  case FFV_FLOAT64:

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
		
					endptr = scratch_buffer + FF_VAR_LENGTH(var) - 1;
					while (endptr >= scratch_buffer && *endptr == ' ')
						*endptr-- = '0';

					errno = 0;
					*data_dest = strtod(scratch_buffer, &endptr);
					if (errno || ok_strlen(endptr))
					{
						err_push(ROUTINE_NAME, errno == ERANGE ? errno : ERR_STR_TO_NUMBER, endptr);
						return(-1);
					}
				break;
				
				default:
					sprintf(scratch_buffer, "%d, %s:%d",
					        (int)FFV_DATA_TYPE(var), os_path_return_name(__FILE__), __LINE__);
					err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
					err_disp();
					return (-1);
			}/* switch FFV_DATA_TYPE(var); case FFF_ASCII/FFF_DBASE switch format_type */
			
			if (IS_INTEGER(var) && var->precision)
				*data_dest /= pow(10.0, var->precision);

			return(0);
	
			
		case FFF_BINARY:
	
			switch (FFV_DATA_TYPE(var))
			{
				case FFV_TEXT:
					/* it seems the intention here is to treat a character string in
					   a binary file as a character/numeral representation of a number
					   -- (may as well...) */
					(void)memcpy(scratch_buffer, data_src, FF_VAR_LENGTH(var));
					scratch_buffer[FF_VAR_LENGTH(var)] = STR_END;
					
					errno = 0;
					*data_dest = strtod(scratch_buffer, &endptr);
					if (errno || ok_strlen(endptr))
					{
						err_push(ROUTINE_NAME, errno == ERANGE ? errno : ERR_STR_TO_NUMBER, endptr);
						return(-1);
					}
					return(0);
			
				case FFV_INT8:
				case FFV_UINT8:
				case FFV_INT16:
				case FFV_UINT16:
				case FFV_INT32:
				case FFV_UINT32:
				case FFV_INT64:
				case FFV_UINT64:
			  case FFV_FLOAT32:
			  case FFV_FLOAT64:
					error = btype_to_btype(data_src, FFV_DATA_TYPE(var), data_dest, FFV_DOUBLE);
					if (error)
						return(-1);

					if (IS_INTEGER(var) && var->precision)
						*data_dest /= pow(10.0, var->precision);
					
					return(0);
		
				default:
					sprintf(scratch_buffer, "%d, %s:%d",
					        (int)FFV_DATA_TYPE(var), os_path_return_name(__FILE__), __LINE__);
					err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
					err_disp();
					return (-1);
			}/* end switch on variable types */
				
		default:
			sprintf(scratch_buffer, "%d, %s:%d",
			        (int)format_type, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
			err_disp();
			return (-1);
	}/* switch format_type */
}

