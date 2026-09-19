/*
 * NAME:	ff_string_to_binary
 *
 * PURPOSE:	Convert a string to a binary number.
 *
 * USAGE:	int ff_string_to_binary(
 *				char *variable_str,         variable string address 
 *				FF_TYPES_t type, 		Output variable type
 *				FF_DATA_BUFFER destination)	destination
 *																		  
 * RETURNS:	0 if all goes well, otherwise an error code defined in err.h
 *
 * DESCRIPTION:	This function takes the string pointed to by variable_str
 *				and converts it to a binary number of the type given by
 *				the output variable.
 *				
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * ERRORS:
 *		Problem in conversion,"floating point number"
 *		Problem in conversion,"floating point number"
 *		ASCII->Binary Overflow,"unsigned char"
 *		ASCII->Binary Overflow,"short"
 *		ASCII->Binary Overflow,"unsigned short"
 *		ASCII->Binary Overflow,"long"
 *		ASCII->Binary Overflow,"unsigned long"
 *		Unknown variable type,"output type"
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 *
 * COMMENTS:
 *
 * KEYWORDS:	
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <float.h>

#define WANT_NCSA_TYPES
#include <freeform.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "String_to_binary"


int ff_string_to_binary(char *variable_str,	/* variable string address*/ 
	FF_TYPES_t output_type, 			/* Output variable type */	
	FF_DATA_BUFFER destination)				/* Destination  */
															  
{
	double double_var; /* NOTE: leave as double, or change calling convention */
	char *endptr;

	/* Error checking on NULL parameters */
	assert(variable_str && destination);

	if (IS_TEXT_TYPE(output_type))
	{
		strcpy((char *)destination, (char *)variable_str);
		return(0);
	}
	else
	{
		errno = 0;
		double_var = strtod(variable_str, &endptr);
		if (ok_strlen(endptr) || errno == ERANGE)
		{
			return(err_push(ROUTINE_NAME,ERR_CONVERT,"ASCII to binary number conversion"));
		}
	}

	return(btype_to_btype(&double_var, FFV_DOUBLE, destination, FFV_DATA_TYPE_TYPE(output_type)));
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_binary_to_string"

/*****************************************************************************
 * NAME:  ff_binary_to_string()
 *
 * PURPOSE:  Write a binary representation of a number as a string
 *
 * USAGE:  error = ff_binary_to_string(binary_data, data_type, text_string);
 *
 * RETURNS:	0 if all goes well, otherwise an error code defined in err.h
 *
 * DESCRIPTION:  Depending on data_type, binary_data is cast as a pointer to
 * the given type, and sprintf()'d into string.  If binary_data contains a
 * string (data_type is FFV_CHAR) then the string is copied into text_string.
 * Doubles and floats are converted using default precision.
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

int ff_binary_to_string(void *binary_data, FF_TYPES_t data_type,
    char *text_string, int string_size)
{
	assert(binary_data != NULL && text_string != NULL);
	
	switch (FFV_DATA_TYPE_TYPE(data_type))
	{
		char error_buffer[_MAX_PATH];
		
		case FFV_TEXT:
			if (string_size)
			{
				strncpy(text_string, (char *)binary_data, string_size);
				text_string[string_size] = STR_END;
			}
			else
				strcpy(text_string, (char *)binary_data);
		break;
		
		case FFV_INT8:
			sprintf(text_string, fft_cnv_flags[FFNT_INT8], *(int8 *)binary_data);
		break;
		
		case FFV_UINT8:
			sprintf(text_string, fft_cnv_flags[FFNT_UINT8], *(uint8 *)binary_data);
		break;
		
		case FFV_INT16:
			sprintf(text_string, fft_cnv_flags[FFNT_INT16], *(int16 *)binary_data);
		break;
		
		case FFV_UINT16:
			sprintf(text_string, fft_cnv_flags[FFNT_UINT16], *(uint16 *)binary_data);
		break;
		
		case FFV_INT32:
			sprintf(text_string, fft_cnv_flags[FFNT_INT32], *(int32 *)binary_data);
		break;
		
		case FFV_UINT32:
			sprintf(text_string, fft_cnv_flags[FFNT_UINT32], *(uint32 *)binary_data);
		break;
		
		case FFV_INT64:
			sprintf(text_string, fft_cnv_flags[FFNT_INT64], *(int64 *)binary_data);
		break;
		
		case FFV_UINT64:
			sprintf(text_string, fft_cnv_flags[FFNT_UINT64], *(uint64 *)binary_data);
		break;
		
		case FFV_FLOAT32:
			sprintf(text_string, fft_cnv_flags[FFNT_FLOAT32], *(float32 *)binary_data);
		break;
		
		case FFV_FLOAT64:
			sprintf(text_string, fft_cnv_flags[FFNT_FLOAT64], *(float64 *)binary_data);
		break;
		
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)FFV_DATA_TYPE_TYPE(data_type),
			        os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(ERR_SWITCH_DEFAULT);
	}

	return(0);
}
