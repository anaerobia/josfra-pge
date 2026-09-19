/* FILENAME: freeform.c
 *
 * CONTAINS: ff_free_format
 *           ff_create_format
 *           ff_free_variable
 *           ff_create_bufsize
 *           ff_resize_bufsize
 *           ff_destroy_bufsize
 *
*/

#define WANT_NCSA_TYPES
#include <freeform.h>

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "ff_free_format"

/*****************************************************************************
 * NAME:  ff_free_format
 *
 * PURPOSE:  To free a format and its variable list
 *
 * USAGE:  ff_free_format(format_ptr);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Frees the variable list, then de-initializes the format's
 * contents and frees it.
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

void ff_free_format(FORMAT_PTR format)
{
	/* Error checking on NULL parameter */
	assert(format);
	assert(format == format->check_address);

	if (format->variables)
		dll_free(format->variables, (void (*)(void *))ff_free_variable);
	
	format->check_address = NULL;
	strcpy(format->name, "This format has been freed");
	format->type = FFF_NULL;
	format->num_in_list = 0;
	format->max_length = 0;
	format->variables = NULL;

	memFree(format, "format");
}

/*****************************************************************************
 * NAME: ff_create_format
 *
 * PURPOSE:  Allocate memory, initialize fields of a format structure
 *
 * USAGE:  format = ff_create_format();
 *
 * RETURNS:  NULL if insufficient memory, an empty format structure otherwise.
 *
 * DESCRIPTION:
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

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_create_format"

FORMAT_PTR ff_create_format(void)
{
	FORMAT_PTR format;
	
	format = (FORMAT_PTR)memMalloc(sizeof(FORMAT), "format");
	if (format)
	{
		format->check_address = (void *)format;
		format->num_in_list = 0;
		format->max_length  = 0;
		format->type = 0;
		format->variables = NULL;

		strncpy(format->locus, FORMAT_LOCUS_INIT, sizeof(format->locus) - 1);
		format->locus[sizeof(format->locus) - 1] = STR_END;

		strncpy(format->name, FORMAT_NAME_INIT, sizeof(format->name) - 1);
		format->name[sizeof(format->name) - 1] = STR_END;
	}
	
	return(format);
}


#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_free_variable"

/*****************************************************************************
 * NAME: ff_free_variable
 *
 * PURPOSE: To free a variable
 *
 * USAGE:  ff_free_variable(variable_ptr);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  De-initializes the variable's contents, then frees it.
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

void ff_free_variable(VARIABLE_PTR variable)
{
	/* Error checking on NULL parameter */
	assert(variable);
	assert(variable == variable->check_address);

#ifdef MEMTRACK
	/* If the following assertion fails, then this is a memory leak.  I am
	   employing the assertion below to expose such leaks */
	assert(variable->info == NULL);
#endif

	variable->check_address = NULL;
	strcpy(variable->name, "This variable has been freed");
	variable->type = FFV_NULL;
	variable->start_pos = 0;
	variable->end_pos = 0;
	variable->precision = 0;
	variable->info = NULL;
	
	memFree(variable, "variable");
}

/*****************************************************************************
 * NAME:  ff_create_bufsize()
 *
 * PURPOSE:  Create an FF_BUFSIZE object
 *
 * USAGE:  bufsize = ff_create_bufsize(size);
 *
 * RETURNS:  NULL if memory allocations failed, otherwise a pointer to an
 * FF_BUFSIZE structure
 *
 * DESCRIPTION:  Allocate memory for FF_BUFSIZE structure.  Allocated buffer
 * field according to size argument.  Initialize bytes_used field to zero and
 * total_bytes field to size argument.
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

FF_BUFSIZE_PTR ff_create_bufsize(size_t total_bytes)
{
	FF_BUFSIZE_PTR bufsize = NULL;
	
	bufsize = (FF_BUFSIZE_PTR)memMalloc(sizeof(FF_BUFSIZE), "bufsize");
	if (bufsize)
	{
		bufsize->bytes_used = 0;
		bufsize->buffer = (char *)memMalloc(total_bytes, "bufsize->buffer");
		if (bufsize->buffer)
			bufsize->total_bytes = total_bytes;
		else
		{
			bufsize->total_bytes = 0;
			err_push(ROUTINE_NAME, ERR_MEM_LACK, NULL);
			memFree(bufsize, "bufsize");
			bufsize = NULL;
		}
	}
	else
		err_push(ROUTINE_NAME, ERR_MEM_LACK, NULL);
	
	return(bufsize);
}

/*****************************************************************************
 * NAME:  ff_resize_bufsize()
 *
 * PURPOSE:  Resize an existing FF_BUFSIZE object.
 *
 * USAGE:  error = ff_resize_bufsize(new_size, bufsize_handle);
 *
 * RETURNS:  Zero on success, an error code on failure.
 *
 * DESCRIPTION:  Calls realloc() on the buffer field.  Adjusts bytes_used
 * and total_bytes fields.  If new_size is smaller than bytes_used, bytes_used
 * will be set to new_size -- it seems preferable that the calling routine
 * do this to avoid surprises.
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

int ff_resize_bufsize(size_t new_size, FF_BUFSIZE_HANDLE hbufsize)
{
	char *cp = NULL;

	assert(hbufsize);
	assert(new_size);
	assert(*hbufsize);
	assert(new_size != (*hbufsize)->total_bytes);
	assert((*hbufsize)->bytes_used <= (*hbufsize)->total_bytes);
	
	if (   !hbufsize
	    || !new_size
	    || !*hbufsize
	    || new_size == (*hbufsize)->total_bytes)
		return(ERR_PARAM_VALUE);
	
	cp = (char *)memRealloc((*hbufsize)->buffer, new_size, "hbufsize-->buffer");
	if (cp)
	{
		(*hbufsize)->buffer = cp;

		if ((*hbufsize)->bytes_used > new_size)
			(*hbufsize)->bytes_used = new_size;

		(*hbufsize)->total_bytes = new_size;

		return(0);
	}
	else
		return(ERR_MEM_LACK);
}

/*****************************************************************************
 * NAME:  ff_destroy_bufsize()
 *
 * PURPOSE:  Destroy an existing FF_BUFSIZE object.
 *
 * USAGE:  ff_destroy_bufsize(bufsize);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Frees the object's memory blocks, sets bytes_used and
 * total_bytes to zero, and buffer to NULL.
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

void ff_destroy_bufsize(FF_BUFSIZE_PTR bufsize)
{
	assert(bufsize);
	
	if (bufsize)
	{
		assert(bufsize->bytes_used <= bufsize->total_bytes);
		memFree(bufsize->buffer, "bufsize->buffer");
		bufsize->buffer = NULL;
		bufsize->bytes_used = bufsize->total_bytes = 0;
		
		memFree(bufsize, "bufsize");
	}
}

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "ffv_type_size"

size_t ffv_type_size(FFV_TYPE_type var_type)
{
	size_t byte_size = 0;
	
	switch (FFV_DATA_TYPE_TYPE(var_type))
	{
		char error_buffer[_MAX_PATH];
		
		case FFV_TEXT:
			byte_size = 1; /* unknown, determine elsehow */
		break;
		
		case FFV_INT8:
			byte_size = SIZE_INT8;
		break;
					
		case FFV_UINT8:
			byte_size = SIZE_UINT8;
		break;
					
		case FFV_INT16:
			byte_size = SIZE_INT16;
		break;
					
		case FFV_UINT16:
			byte_size = SIZE_UINT16;
		break;
					
		case FFV_INT32:
			byte_size = SIZE_INT32;
		break;
					
		case FFV_UINT32:
			byte_size = SIZE_UINT32;
		break;
					
		case FFV_INT64:
			byte_size = SIZE_INT64;
		break;
					
		case FFV_UINT64:
			byte_size = SIZE_UINT64;
		break;
					
		case FFV_FLOAT32:
			byte_size = SIZE_FLOAT32;
		break;
					
		case FFV_FLOAT64:
			byte_size = SIZE_FLOAT64;
		break;
					
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)var_type, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			byte_size = 0;
		break;
	}
	
	return(byte_size);
}

