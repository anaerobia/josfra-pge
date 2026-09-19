/*
 * NAME:        do_get_count
 *              
 * PURPOSE:     To get a count of some sort from some kind of thing.
 *
 * USAGE:       long = do_get_count(OBJECT_TYPE, Argument, ...)
 *
 * RETURNS:     A long on success, -1 on error.
 *
 * DESCRIPTION: do_get_count is a general function for getting a count from
 *                              an object. The presently supported objects are:
 *      case FFF_FORMAT:        Arguments: FORMAT_PTR format, char *buffer, [DATA_BIN_PTR dbin] 
 *      case FFF_INDEX_ARRAY:   Argument:  VIEW_PTR 
 *
 * ERRORS:
 *                                      , " record_count from table"
 *                              Unknown variable type, "Finding \"count\""
 *                      Problem getting value, "Into &d_count"
 *              Unknown object type, NULL
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS: MEDIUM model: need to malloc arrays
 * check that allocations are allocating the correct type (near or huge)
 * compiler doesn't like buffer cast to long in return()
 *
 * MAO changes: eliminated some intermediary variables.  Removed some unused
 * code in case FFF_INDEX_ARRAY.  ff_get_value() call now uses dbin->buffer
 * instead of (removed) local scratch.  Cast warnings are retained to remind
 * that the cases for float and double are being allowed.
 *
 * KEYWORDS:    
 *
 */

#include <stdarg.h>

#include <freeform.h>
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "do_get_count"

long do_get_count(FF_TYPES_t type, ...)
{
	va_list args;

	double d_count = 0;
	long long_count = 0;

	DATA_BUFFER     buffer    = NULL;

	FORMAT_PTR      format    = NULL;
	VARIABLE_PTR    count_var = NULL;
	DATA_BIN_PTR    dbin      = NULL;
	DATA_VIEW_PTR   view      = NULL;
	
	int error;
	
	va_start(args, type);

	switch (type)
	{
		case FFF_FORMAT:        /* Arguments: FORMAT_PTR format, char *buffer, [DATA_BIN_PTR dbin] */
			format = va_arg(args, FORMAT_PTR);
			buffer = va_arg(args, char HUGE *);
			dbin = va_arg(args, DATA_BIN_PTR);
	
			/* Error checking */
			assert(format && buffer && dbin && (void *)dbin == dbin->check_address);
	
			/* Search the format for a variable called count */
			count_var = ff_find_variable("count", format);
			if (!count_var)
			{
		
				/* Count was not found in the record header, so check to see if 
				it is defined in an eqv table */
				if(nt_askvalue(dbin, "record_count", FFV_LONG, &long_count, NULL))
					return(long_count);
				else 
				{
					err_push(ROUTINE_NAME, ERR_GET_VALUE, " record_count from table");
					return(-1);
				}
			}
	
			/* Get the value of count */
	
			/* In this case we are reading a data count from a header record.
			Because this is a count, I assume that it will usually be an integer
			type. In the face of this assumption I use the function ff_get_value,
			which gets the value of count into count with the type of the variable
			count in the header format. I use ff_get_value rather than ff_get_double
			in order to avoid two unnecessary conversions.
			The returned value must then be converted to a long.
			*/
			
			if (ff_get_value(count_var, buffer + count_var->start_pos - 1, 
					(void *)&d_count, format->type))
			{
				return(-1);
			}
			
			error = btype_to_btype(&d_count, FFV_TYPE(count_var), &long_count, FFV_LONG);
			if (error)
			{
				return(-1);
			}
			
			return(long_count);
	
		case FFF_INDEX_ARRAY:   /* Argument:  VIEW_PTR */
	
			/* Read the arguments */
			view = va_arg(args, DATA_VIEW_PTR);
	
			/* MAO GETTRACK example: view->data is pointer to an offset value in an
			   index record, and view->increment is an index record length, giving the
			   offset value in the following index record.  Their difference is the
			   length of the band, given by the first offset.  Values taken as longs.
			*/
	
			return( *((long *)(view->data + view->increment)) - *((long *)view->data) );
				
		default:
		{
			/* Error: Unknown Object Type */
			char scratch_buffer[80];

			sprintf(  scratch_buffer, "%s, %s:%d"
			        , ROUTINE_NAME, os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, scratch_buffer);
			err_disp();
			return(-1);
		}
	} /* switch type */
}

