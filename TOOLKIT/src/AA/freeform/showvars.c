/*
 * NAME:	ff_show_variables
 *
 * PURPOSE:	This function creates a long list of the variables in a given format
 *			specification. The list is of the form
 *			variable name: variable value.
 *			The order of the list is determined from the format specification
 *			which is usually an input format.
 *
 * AUTHOR:	T. Habermann, NGDC, June, 1990
 *																	
 * USAGE:	ff_show_variables(
 *				FF_SCRATCH_BUFFER input_buffer,
 *				FF_SCRATCH_BUFFER output_buffer,
 *				FORMAT_PTR form )
 *
 * COMMENTS:
 *
 * RETURNS:	The number of variables read and listed, else 0
 *
 * ERRORS:
 *				Problem in conversion,"\nCannot convert variable to string"
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#include <freeform.h>
#include <memory.h>

#undef ROUTINE_NAME
#define  ROUTINE_NAME "ff_show_variables"

int ff_show_variables(FF_SCRATCH_BUFFER input_buffer, FF_SCRATCH_BUFFER output_buffer, FORMAT_PTR format)
{
	short		num_variables = 0;

	VARIABLE_PTR var = NULL;
	FF_DATA_PTR data_ptr = NULL;
	VARIABLE_LIST_PTR v_list = NULL;
	FF_SCRATCH_BUFFER ch_ptr = NULL;


	assert(format && input_buffer && output_buffer &&
	      ((void *)format == format->check_address));

	/* Initialize pointers */
	ch_ptr = output_buffer;
	v_list = FFV_FIRST_VARIABLE(format);

	while(var = FFV_VARIABLE(v_list))
	{
		assert(var == var->check_address);
		if (var != var->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format's variable list");
			return(0);
		}
		
		++num_variables;

		/* Set the data pointer */
		data_ptr = input_buffer + var->start_pos - 1;

		sprintf(ch_ptr, "%s: ", var->name);
		CH_TO_END(ch_ptr);

		/* check whether or not the variable is defined */
		if (var->start_pos <= var->end_pos)
		{
			/* Convert the variable to a string */
			if (ff_get_string(var,	data_ptr, ch_ptr, format->type) == -1)
			{
				err_push(ROUTINE_NAME,ERR_CONVERT,"Cannot convert variable to string");
				return(0);
			}
		}
		else
		 	memStrcpy(ch_ptr, "undefined",NO_TAG);	

		ch_ptr += strlen(ch_ptr);

		*(ch_ptr++) = '\n';

		v_list = dll_next(v_list);
	}
	*ch_ptr = '\0';
	return(num_variables);
}

