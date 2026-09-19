/*
 * NAME:        ff_get_value
 *
 * PURPOSE:     To get the value of a variable in a binary representation.
 * This Function is similar to  ff_get_double.c, but the value is left as
 * it's native type. If the variable is an integer type, the precision is
 * not adjusted.
 *
 * USAGE:  ff_get_value(
 * VARIABLE      *var,          The variable description 
 * void          *data_src,     The address of the variable 
 * void          *data_dest,    The destination for the variable 
 * FF_TYPES_t format_type)  The Format Type  
 *
 * RETURNS:     0 if goes as planned, else an error code from err.h
 *
 * DESCRIPTION:  This Function is similar to
 * ff_get_double, but the value is left as it's native type.
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS: -- USE THIS FUNCTION CAREFULLY!  IT CAN CAUSE PROBLEMS WHICH ARE
 * DIFFICULT TO DETECT --
 * It is easy to make assumptions about the type of a given
 * variable when using this function. These assumptions can
 * lead to problems when applications with your assumptions
 * are used with formats which are derived using someone elses
 * assumptions. For example, the variable day is often
 * represented as a unsigned char. If you use ff_get_value to
 * get the value of day and give it a pointer to an unsigned char
 * as the destination it will work well with your formats where
 * day is, in fact, an unsigned char. Someone else may consider
 * day to be a float and store decimal days. When the application
 * is used with that format a float will be returned in the
 * location that you are treating as an unsigned char. This will
 * stomp on the next three bytes.
 * The function ff_get_double() is somewhat safer.
 *
 * ERRORS:
 * Out of memory,"tmp_str"
 * ,"variable precision is larger than the variable_length"
 * Unknown variable type,"ASCII var->type"
 * ,NULL
 *
 * KEYWORDS:
 *
 */

#include <stdlib.h>
#include <memory.h>
#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_get_value"

int ff_get_value(VARIABLE_PTR var, /* The variable description */
		FF_DATA_PTR data_src, /* The address of the variable */
		FF_DATA_PTR data_dest, /* The destination for the variable */
		FF_TYPES_t format_type) /* The Format Type */
{
	unsigned variable_length;
	int error;

	FF_SCRATCH_BUFFER decimal_ptr = NULL;
	FF_SCRATCH_BUFFER ch  = NULL;
	FF_SCRATCH_BUFFER last_char = NULL;

	char *tmp_str = NULL;

	assert(var && data_src && data_dest && ((void *)var == var->check_address));
	
	variable_length = FF_VAR_LENGTH(var);

	if((unsigned)abs(var->precision) > variable_length)
		return(err_push(ROUTINE_NAME, ERR_SYNTAX,"variable precision is larger than the variable_length"));
	
	switch (format_type & FFF_FORMAT_TYPES)
	{
		case FFF_ASCII:         /* If the format is ASCII, copy the string */
		case FFF_DBASE:         /* If the format is DBASE, copy the string */

			/* If the variable type is FFV_CHAR, copy to destination, return now */
			if (IS_TEXT(var))
			{
				memcpy(data_dest, data_src, variable_length);
				*((char *)data_dest + variable_length) = STR_END;
				return(0);
			}

			tmp_str = (char *)memMalloc(variable_length + 2, "tmp_str"); /* one more space in case of memmov */
			if (!tmp_str)
			{
				return(err_push(ROUTINE_NAME,ERR_MEM_LACK,"tmp_str"));
			}
			memMemcpy(tmp_str, data_src, variable_length, NO_TAG);
		
			/* define a pointer to the end of the string */
			last_char = tmp_str + variable_length;
			*last_char = STR_END;
	
			/* Fill in trailing blanks with zeroes */
			ch = last_char - 1;
			while (*ch == ' ' && ch >= tmp_str)
				*ch-- = '0';
	
			if (ch < tmp_str)        /* String is blank, fill with 0 */
				*tmp_str = '0';
	
			/* If a precision is specified and there is no decimal point then
			the decimal point must be added. This requires that the variable
			become a double */
			
			/* MAO:c This seems very curious to me.  What's happening is that an
			   integer with an implied decimal point (e.g., an integer scaled by
			   a factor of ten) is going to be converted as a double into data_dest
			   -- do calling functions know about this? 
			   LASTLY, this seems inconsistent because this is not done for the
			   binary case below. */
	
			decimal_ptr = memStrchr(tmp_str, '.', "decimal_ptr");
			if (var->precision && !decimal_ptr)
			{
				/* shift the string to make room for the decimal point */
				memmove(last_char + (int)(1 - var->precision),
				 last_char - var->precision, var->precision + 1);
				*(last_char - var->precision) = '.';
				*(last_char + 1) = STR_END;
				*((double *)data_dest) = atof(tmp_str);
				memFree(tmp_str, "tmp_str");
				return(0);
			}
	
			/* We are left with non-character variables
				with decimal points in the correct place or
				with precision = 0 and no decimal points.
				These variables can be directly converted. */
	
			error = ff_string_to_binary(tmp_str, FFV_TYPE(var), (char *)data_dest);
			memFree(tmp_str, "tmp_str");
			return(error);

		case FFF_BINARY:
			/* In the BINARY case just copy the variable */
			memcpy(data_dest, data_src, variable_length);
			if(IS_TEXT(var))
				*((char *)data_dest + variable_length) = STR_END;
			return(0);
		
		default:
			return(err_push(ROUTINE_NAME, ERR_UNKNOWN_FORM_TYPE, NULL));
	} /* End of format_type switch() */

}/* End ff_get_value() */
