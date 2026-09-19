/*
 * NAME:         make_param_list        
 *              
 * PURPOSE:     This function reads a character buffer and creates a parameter list.
 *
 * USAGE:       PARAM_LIST *make_param_list(char *buffer, FORMAT *format)
 *                      buffer: the character buffer
 *                      format: the data format to which the plist belongs.
 *
 * RETURNS:     The pointer to the PARAM_LIST
 *
 * DESCRIPTION: Each line of the parameter buffer has a variable name and a minimum
 *                      and maximum value.      
 *
 * ERRORS:
 *                      Problem making parameter list, error_buffer
 *                      Variable Not Found, name
 *                      Out of memory, "plist"
 *                      Unknown variable type, strcat("plist var name-> ",plist->var->name)
 *                      Out of memory, plist->var->name
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * GLOBALS:     
 *
 * AUTHOR:      unknown
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WANT_NCSA_TYPES
#include <freeform.h>

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "make_param_list"

struct pl
{
	VARIABLE_PTR var;
	void *minimum;
	void *maximum;
	struct pl *next;
};

typedef struct pl PARAM_LIST;

PARAM_LIST *make_param_list(char *buffer, FORMAT *format)
{
	char *ch;
	char name[MAX_NAME_LENGTH];
	char min_str[MAX_NAME_LENGTH];
	char max_str[MAX_NAME_LENGTH];
	char error_buffer[MAX_ERRSTR_BUFFER];

	double d_min, d_max;

	short num_lines = 0;
	unsigned short var_length = 0;
	short bytes_to_skip = 0;

	int error;
	
	char *endptr = NULL;

	PARAM_LIST *root = NULL, *plist = NULL;
	VARIABLE_PTR var;

	assert(buffer && format && ((void *)format == format->check_address));

	ch = buffer;
	while((ch = strtok((num_lines++) ? NULL : ch, "\n")) != NULL)
	{
		/* Skip Comment Lines in the Parameter Buffer */
		if (*ch == '/')
			continue;

		if (sscanf(ch,"%s %s %s", name, min_str, max_str) != 3)
		{
			sprintf(error_buffer,"\nline ->%s<- does not contain 3 entries\n",ch);
			err_push(ROUTINE_NAME,ERR_MAKE_PLIST, error_buffer);
			return(NULL);
		}
		var = ff_find_variable(name, format);

		/* Do some error checking on the parameter list */

		if (!var)
		{
			err_push(ROUTINE_NAME,ERR_VARIABLE_NOT_FOUND, name);
			return(NULL);
		}

		if (!root) /* First Variable */
			root = plist = (PARAM_LIST *)memCalloc(1, sizeof(PARAM_LIST), "make_param_list");
		else /* Subsequent Variables */
			plist = plist->next = (PARAM_LIST *)memCalloc(1, sizeof(PARAM_LIST), "make_param_list");

		if (!plist)
		{
			err_push(ROUTINE_NAME,ERR_MEM_LACK, "plist");
			return(NULL);
		}
		plist->var = var;

		/* allocate memory (and copy in string if FFV_CHAR) */

		if (IS_TEXT(plist->var))
		{
				/* Do minimum */
				var_length = FF_VAR_LENGTH(plist->var) + 1;
				plist->minimum = memMalloc(var_length, "make_param_list");
				if (!plist->minimum) 
					break;
				bytes_to_skip = var_length - (strlen(min_str)) - 1;
				assert(bytes_to_skip >= 0);
				memMemset(plist->minimum,' ', var_length,"plist->minimum,' ', var_length");
				sprintf(((char*)plist->minimum + bytes_to_skip),"%s",min_str);                   
	
				/* Do Maximum */
				plist->maximum = memMalloc(var_length, "make_param_list");
				if (!plist->maximum)
					break;
				memMemset(plist->maximum,' ', var_length,"plist->maximum,' ',var_length");
				bytes_to_skip = var_length - (strlen(max_str)) - 1;
				assert(bytes_to_skip >= 0);
				sprintf(((char*)plist->maximum + bytes_to_skip),"%s",max_str);
		}
		else if (IS_INTEGER(plist->var) || IS_REAL(plist->var))
		{
			size_t byte_size = 0;
			
			byte_size = ffv_type_size(FFV_DATA_TYPE(plist->var));
			if (byte_size)
			{
				plist->minimum = memMalloc(byte_size, "plist->minimum");
				plist->maximum = memMalloc(byte_size, "plist->maximum");
			}
			else
				assert(byte_size); /* should never happen */
		}
		else
		{ /* do what? */
			assert(0);
		}
			
		if ((!plist->minimum) || (!plist->maximum))
		{
			err_push(ROUTINE_NAME,ERR_MEM_LACK, plist->var->name);
			return(NULL);
		}

		/* Read the minimum and maximum for this variable */
		
		if (!IS_TEXT(plist->var))
		{
			errno = 0;
			d_min = strtod(min_str, &endptr);
			if (ok_strlen(endptr) || errno)
			{
				err_push(ROUTINE_NAME, ERR_CONVERT, "numeric string to binary representation");
				return(NULL);
			}
				
			d_max = strtod(max_str, &endptr);
			if (ok_strlen(endptr) || errno)
			{
				err_push(ROUTINE_NAME, ERR_CONVERT, "numeric string to binary representation");
				return(NULL);
			}
				
			if (IS_INTEGER(plist->var) && plist->var->precision)
			/* Adjust the minimum and maximum to match the precision of the data */
			{
				d_min *= pow(10.0, plist->var->precision);
				d_max *= pow(10.0, plist->var->precision);
			}

			error = btype_to_btype(&d_min, FFV_DOUBLE, plist->minimum, FFV_DATA_TYPE(plist->var));
			if (error)
			{
				err_push(ROUTINE_NAME, ERR_CONVERT, "parameter list minimum");
				return(NULL);
			}
			error = btype_to_btype(&d_max, FFV_DOUBLE, plist->maximum, FFV_DATA_TYPE(plist->var));
			if (error)
			{
				err_push(ROUTINE_NAME, ERR_CONVERT, "parameter list maximum");
				return(NULL);
			}
		} /* switch() to convert min/max_str's into plist->minimum/maximum's */
	
		plist->next = NULL;
	} /* while() lines to process */
	
	return(root);
}

#ifdef SHOW_PARAM_LIST_NEVER_CALLED

/*
 * NAME:		show_param_list
 *		
 * PURPOSE:	To covert the parameter list to ASCII
 *
 * USAGE:	int show_param_list(PARAM_LIST *plist, char *buffer)
 *			plist: a pointer to the PARAM_LIST structure
 *			buffer: the memory for the output ASCII plist
 *
 * RETURNS:	number of variables in the plist
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	none
 *
 * GLOBALS:	none
 *
 * AUTHOR:	Liping Di, NGDC, (303) 497 - 6284, lpd@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * KEYWORDS:	plist
 *
 */

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "show_param_list"

int show_param_list_never_called(PARAM_LIST *plist, char *buffer)
{
	int num_variables = 0;
	char *ch_ptr;
	PARAM_LIST *pl;

	/* Initialize pointers to list and buffer */
	pl = plist;
	ch_ptr = buffer;

	while (pl)
	{
		char error_buffer[_MAX_PATH];
		
		++num_variables;
		sprintf(ch_ptr,"%s ", pl->var->name);
		CH_TO_END(ch_ptr);

		/* Print the minimum and maximum values, two to a line, formatted
		   according to the parameter's variable type.
		*/
		switch(FFV_DATA_TYPE(pl->var))
		{
			case FFV_INT8:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT8], *(int8 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT8], *(int8 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_UINT8:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT8], *(uint8 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT8], *(uint8 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_INT16:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT16], *(int16 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT16], *(int16 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_UINT16:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT16], *(uint16 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT16], *(uint16 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_INT32:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT32], *(int32 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT32], *(int32 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_UINT32:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT32], *(uint32 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT32], *(uint32 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_INT64:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT64], *(int64 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_INT64], *(int64 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_UINT64:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT64], *(uint64 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_UINT64], *(uint64 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_FLOAT32:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_FLOAT32], *(float32 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_FLOAT32], *(float32 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
			case FFV_FLOAT64:
				sprintf(ch_ptr, fft_cnv_flags[FFNT_FLOAT64], *(float64 *)pl->minimum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, " ");
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, fft_cnv_flags[FFNT_FLOAT64], *(float64 *)pl->maximum);
				CH_TO_END(ch_ptr);
				sprintf(ch_ptr, "\n");
			break;
				
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)FFV_DATA_TYPE(pl->var), os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(num_variables);
		break;
		}

		CH_TO_END(ch_ptr);
		pl = pl->next;
	}

	*ch_ptr = STR_END;

	return(num_variables);
}

#endif /* SHOW_PARAM_LIST_NEVER_CALLED */

