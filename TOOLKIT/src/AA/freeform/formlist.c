/* FILENAME:  formlist.c
 *
 * CONTAINS:    Functions for maintaining format list for data bin:
 * db_format_list_mark_io()
 * db_merge_format_lists()
 * db_delete_all_formats()
 * db_delete_rivals()
 * db_free_format_list()
 * db_show_format_list()
 * db_replace_format()
 * db_delete_format()
 * db_find_format()
 * db_find_format_is_isnot(FORMAT_LIST_PTR, ...)
 *
 */

#include <limits.h>
#include <freeform.h>
#include <databin.h>
/* #include <os_utils.h> */

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_show_format_list"

/*
 * NAME: db_show_format_list
 *
 * PURPOSE: This function lists the formats in a format list into a buffer.
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * USAGE: db_show_format_list(FORMAT_LIST_PTR format_list, FF_SCRATCH_BUFFER buffer)
 *
 * DESCRIPTION: 
 *
 * COMMENTS:  
 *
 * RETURNS:     
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

int db_show_format_list(FORMAT_LIST_PTR f_list, FF_TYPES_t info_type, FF_SCRATCH_BUFFER buffer)
{
	FORMAT_PTR format;
	FF_BUFSIZE bufsize;
	int error;

	char *lookup_str = NULL;

	/* Error checking on NULL parameters */

	assert(f_list && buffer);

	f_list = dll_first(f_list);

	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);
		
		if (format != format->check_address)
			return(err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address"));

		if (info_type & FFF_INFO)
		{
			bufsize.buffer = buffer;
			bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
			bufsize.bytes_used = 0;
			error = ff_format_info(format, &bufsize);

			if (error == 0)
				return(err_push(ROUTINE_NAME,ERR_SHOW_FORM+ERR_DEBUG_MSG,"text buffer"));
			CH_TO_END(buffer);
		}
	
		if (info_type & FFF_FMT_INFO)
		{
			*buffer++ = '\n';
			lookup_str = ff_lookup_string(format_types, format->type);
			if (!lookup_str)
				return(err_push(ROUTINE_NAME, ERR_SHOW_FORM, "cannot find format string"));

			sprintf(buffer,"%s",lookup_str);
			CH_TO_END(buffer);
			sprintf(buffer," \"%s\"",format->name);
			CH_TO_END(buffer);
			*buffer++ = '\n';

			bufsize.buffer = buffer;
			bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
			bufsize.bytes_used = 0;
			error = ff_list_vars(format, &bufsize);

			if (error == 0)
				return(err_push(ROUTINE_NAME,ERR_SHOW_FORM,NULL));

			CH_TO_END(buffer);
		}
	
		if (info_type & FFF_VARIABLES)
		{
			bufsize.buffer = buffer;
			bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
			bufsize.bytes_used = 0;
			error = ff_list_vars(format, &bufsize);

			if (error == 0)
				return(err_push(ROUTINE_NAME,ERR_SHOW_FORM,NULL));

			CH_TO_END(buffer);
		}
	
		CH_TO_END(buffer);
		f_list = dll_next(f_list);
	}
	return(0);
}


/*
 * NAME: db_replace_format
 *
 * PURPOSE: This function searches a format list for a format with the same type
 * as new_format and replaces that with new_format
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * USAGE: FORMAT_LIST_PTR db_replace_format(FORMAT_LIST_PTR list, FORMAT_PTR new_format)
 *
 * DESCRIPTION: This function provides a general tool for replacing a format
 * in a format list. The replacement process includes the following steps:
 * If no list exists: initialize a list and add new_format to it.
 * If a list exists: search it for a format of the same type and replace
 * that format with new format.
 *
 * A list node data_ptr is made to point to new_format.
 *
 * COMMENTS:  Ignores FFF_FORMAT_TYPES bits in doing format->type comparisons.
 * The effect of this is to ignore the file type of the format descriptor in
 * making a replacement.  For example, a binary_input_data will replace the
 * first occurrence of binary_input_data, ASCII_input_data, or dbase_input_data.
 *                              
 *
 * RETURNS: A pointer to the format list if all goes well, NULL on error
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * ERRORS: If new_format is already in list then db_replace_format() will
 * free() new_format (within list).  Calling routines have the burden of
 * ensuring that they are not replacing the same format.
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_replace_format"

FORMAT_LIST_PTR db_replace_format(FORMAT_LIST_PTR list, FORMAT_PTR new_format)
{
	FORMAT_LIST_PTR f_list;
	FORMAT_PTR format;

	FF_TYPES_t format_group;
	FF_TYPES_t new_format_group;
	
	assert(new_format && ((void *)new_format == new_format->check_address));
	
	if (new_format != new_format->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
		return(NULL);
	}

	if (!list)
	{
		list = dll_init();
		if (!list)
		{
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"Format List");
			return(NULL);
		}
		f_list = dll_add(list, 0);
		if (!f_list)
		{
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"New Format List Node");
			return(NULL);
		}
		f_list->data_ptr = (void *)new_format;
		return(list);
	}

	/* Search list for format with same type */
	f_list = dll_first(list);
	new_format_group = new_format->type & ~FFF_FORMAT_TYPES;
	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);
		if (format != format->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
			return(NULL);
		}

		format_group = format->type & ~FFF_FORMAT_TYPES;
		if (format_group == new_format_group)
			break;
		f_list = dll_next(f_list);
	}

	if (format)
	{
		ff_free_format(format);
		dll_data(f_list) = (void *)new_format;
		return(list);
	}

	/* No match found, just add format to existing list */
	f_list = dll_insert(f_list, 0);
	f_list->data_ptr = (void *)new_format;
	
	return(list);
}

/*
 * NAME: db_delete_format
 *
 * PURPOSE: This function searches a format list for a format with a given
 * type and deletes that format from the list.
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * USAGE: int db_replace_format(FORMAT_LIST_PTR list, int format_type)
 *
 * DESCRIPTION: This function provides a tool for deleting a format
 * from a format list.
 *
 * COMMENTS:    
 *                              
 *
 * RETURNS: 0 if all goes well, non-0 on error
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_delete_format"

int db_delete_format(FORMAT_LIST_PTR f_list, FF_TYPES_t format_type)
{
	FORMAT_PTR format = NULL;

	assert(format_type && f_list);
	
	/* Search list for format with same type */
	f_list = dll_first(f_list);
	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);
		
		f_list = dll_next(f_list);

		if (format != format->check_address)
			return(err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address"));

		if ((format->type & format_type) == format_type)
			dll_delete(dll_previous(f_list), (void (*)(void *))ff_free_format);
	}

	return(0);
}

/*
 * NAME: db_delete_all_formats
 *
 * PURPOSE: This function searches a format list for all formats
 * which at least partially match the format_type key,
 * and deletes those formats from the list.
 *
 * AUTHOR: Kevin Frender, kbf@ngdc.noaa.gov
 *
 * USAGE: int db_delete_all_formats(FORMAT_LIST_PTR list, FF_TYPES_t format_type)
 *
 * DESCRIPTION: This function provides a tool for deleting formats
 * from a format list.  It differs from db_delete_format in
 * that it does not stop removing formats after one is
 * found matching the key. The key must only
 * partially match.  For instance, if the key were:
 * FFF_INPUT|FFF_BINARY|FFF_HEADER, and the format type
 * it was currently examining was
 * FFF_INPUT|FFF_BINARY|FFF_DATA, the format would NOT
 * be deleted from the format list.  However, with a key
 * of FFF_INPUT|FFF_BINARY, all formats which had a type
 * of FFF_INPUT|FFF_BINARY|FFF_DATA, 
 * FFF_INPUT|FFF_BINARY|FFF_HEADER, etc. would be deleted.
 * In otherwords, the key must be a subset of the format type.
 *
 * COMMENTS:    
 *                              
 *
 * RETURNS: 0 if all goes well, non-0 on error
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_delete_all_formats"

int db_delete_all_formats(FORMAT_LIST_PTR f_list, FF_TYPES_t format_type)
{
	FORMAT_PTR format = NULL;

	assert(format_type && f_list);

	/* Search list for formats with roughly same type */
	f_list = dll_first(f_list);
	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);

		f_list = dll_next(f_list);

		if (format != format->check_address)
			return(err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address"));

		if ((format->type & format_type) == format_type)
			dll_delete(dll_previous(f_list), (void (*)(void *))ff_free_format);
	}

	return(0);
}

/*
 * NAME: db_merge_format_lists
 *
 * PURPOSE: This function merges two format lists together.
 *				
 * AUTHOR: Kevin Frender, kbf@ngdc.noaa.gov
 *
 * USAGE: FORMAT_LIST_PTR db_merge_format_lists(FORMAT_LIST_PTR addee_list, FORMAT_LIST_PTR adder_list)
 *
 * DESCRIPTION: This function takes the list given as the second argument
 * and merges it with the list given in the first argument.
 * The merging is done with db_replace_format, so that if
 * a format of duplicate type exists in the first list, it
 * is replaced by the format from the second.  Otherwise, the
 * formats from the second list are added to the first.
 * The second format list is freed as it is merged.
 *
 * if the first format list is NULL, it is created.
 *
 * COMMENTS:    
 *                              
 *
 * RETURNS: FORMAT_LIST_PTR to the addee list, or NULL on error
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_merge_format_lists"

FORMAT_LIST_PTR db_merge_format_lists(FORMAT_LIST_PTR listone, FORMAT_LIST_PTR listtwo)
{
	FORMAT_LIST_PTR f_list;
	FORMAT_PTR format;

	assert(listtwo);

	f_list = dll_first(listtwo);
	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);
		if (format != format->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
			return(NULL);
		}

		if ((listone = db_replace_format(listone, (FORMAT_PTR)format)) == NULL)
		{
			err_push(ROUTINE_NAME, ERR_MAKE_FORM, "merging lists");
			return(NULL);
		}
		f_list = dll_next(f_list);
	}
 	
	/* Free the second format lists' DLL */
	/* MAO:c 4/95 This is technically correct, as (some of) the formats of
	   listtwo have been grafted into listone.  We are freeing the _DLL_,
	   and not the formats!
	*/
	
	f_list = dll_first(listtwo);
	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		/* NOTE:  There's either a bug with this code (or nearby code) or I am
		   not understanding something.  I got an assertion failure below when
		   I ran newform on an example in the User's Guide, but sadly, don't
		   remember which.  I need to look into this further...
		*/
		assert(format == format->check_address);

		f_list = dll_next(f_list);
		memFree(dll_previous(f_list), "listtwo (Format List Node)");
	}
	memFree(listtwo, "listtwo (Format List Root)");
	
	/* The above block should be replaced by:
	
	dll_free(listtwo, NULL);
	
	   This is safe to do now, because with a NULL function pointer parameter,
	   dll_free will only free the nodes.  dll_free is preferable because it
	   NULL-ifies each node as it deletes it.
	*/

	return(listone);
}

/*
 * NAME: db_find_format
 *              
 * PURPOSE: To search a format list for a format whose attribute matches a key.
 *
 * USAGE: FORMAT_PTR format = db_find_format(FORMAT_LIST_PTR, attrib, key, ... NULL)
 *
 * RETURNS: A pointer to the format or NULL if no format is found.
 *
 * DESCRIPTION: db_find_format searches a format list looking for a format
 * with an attribute matching a key. The attributes are
 * specified in a list of arguments followed by the
 * key. The format type identifiers are given in freeform.h:
 *  
 * The attributes which db_find_format identifies are
 * FFF_IO, FFF_GROUP and FFF_NAME
 * of these, FFF_IO, and FFF_GROUP all do essentially
 * the same thing at present. This is so that a call with
 * multiple bits set like:
 *                                                                               or FFF_TYPE can be used instead of
 * form = db_find_format(list, FFF_GROUP, FFF_INPUT | FFF_HD, END_ARGS)
 *                                                                               or FFF_IO
 * form = db_find_format(list, FFF_GROUP, FFF_HD, FFF_IO, FFF_INPUT, END_ARGS).
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR: T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS:    
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_find_format"

FORMAT_PTR db_find_format(FORMAT_LIST_PTR f_list, ...)
{
	va_list args;
	FF_TYPES_t search_key = 0;
	FF_TYPES_t attribute;
	char *key_name = NULL;
	FORMAT_PTR format = NULL;

	/* Error checking on NULL parameter */

	assert(f_list);

	va_start(args, f_list);

	/* Read the arguments */
	while ( (attribute = (FF_TYPES_t)va_arg (args, FF_TYPES_t)) != END_ARGS )
	{
		switch (attribute)
		{
			case FFF_GROUP: /* Argument FF_TYPES_t */
			case FFF_IO:    /* Argument: FF_TYPES_t */
				search_key = (FF_TYPES_t)va_arg (args, FF_TYPES_t);
				
				assert(search_key);
				if (!search_key)
				{
					err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "zero value search_key");
					return(FFF_FORMAT_FMT(dll_first(f_list)));
				}
			break;
	
			case FFF_NAME:  /* Argument: char* */
				key_name = va_arg (args, char *);

				assert(key_name);
				if (!key_name)
				{
					err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "NULL key_name");
					return(FFF_FORMAT_FMT(dll_first(f_list)));
				}
			break;
	
			default:
				err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "undefined search type");
				return(NULL);
				/* Error: Unknown Format Key */
		}
	}

	/* Search the list */
	f_list = dll_first(f_list);

	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);
		
		if (format != format->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
			return(NULL);
		}

		if (search_key && key_name)
		{
			if ((search_key & format->type) == search_key &&
			    !strcmp(format->name, key_name))
				break;
		}
		else if (search_key && ( (search_key & format->type) == search_key) )
			break;
		else if (key_name && !strcmp(format->name, key_name))
			break;

		f_list = dll_next(f_list);
	}
	return(format);
}

/*
 * NAME:  db_format_list_mark_io
 *              
 * PURPOSE: To mark ambiguous formats in a format list as being either input
 * or output formats.
 *
 * USAGE: FORMAT_LIST_PTR format = db_format_list_mark_io(FORMAT_LIST_PTR form_list, char *data_filename)
 *
 * RETURNS: void
 *
 * DESCRIPTION: Looks for and marks ambiguous data format descriptions to be
 * either input or output formats.  This means that format types in .fmt files
 * that were not given the input or output format descriptor (e.g.,
 * "ASCII_input_data") are defined by this function to be either input or
 * output according to the current context.  The extension of the data file
 * being processed is the sole determinant of this context.
 *
 * The rules for marking ambiguous formats based on data file extension
 * are detailed below:
 *
 * data_filename ends in ".dat":
 * Assume ASCII to binary conversion, mark all ambiguous ASCII formats
 * as FFF_INPUT and all ambiguous binary data formats and dBASE formats
 * as FFF_OUTPUT.
 *
 * data_filename ends in ".dab":
 * Assume dBASE to ASCII/binary conversion, mark all ambiguous dBASE formats
 * as FFF_INPUT and all ambiguous ASCII data formats and binary formats as
 * FFF_OUTPUT.
 *
 * data_filename ends in anything else:
 * Assume binary to ASCII conversion, mark all ambiguous binary formats
 * as FFF_INPUT and all ambiguous ASCII data formats and dBASE formats
 * as FFF_OUTPUT.
 *
 * It is worth making explicit that this function no longer attempts to set
 * first choice versus second choice output formats (e.g., dBASE to
 * ASCII/binary where ASCII could be first choice and binary second choice).
 * Unless the user specifically selects formats by their title, the first
 * eligible formats found in the .fmt file (or wherever) are taken as the
 * input and output formats.  This means that a .fmt file containing multiple
 * candidate input and output formats will produce different results depending
 * on the order of the formats.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:  Mark Ohrenschall, NGDC, (303) 497-6124, mao@ngdc.noaa.gov
 *
 * COMMENTS: 
 *
 * KEYWORDS:  MAGIC WAND WAVING
 *
 */

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_format_list_mark_io"

void db_format_list_mark_io(FORMAT_LIST_PTR f_list, char *file_name)
{
	FORMAT_PTR format = NULL;
	FF_TYPES_t ASCII_format_type,
	           dBASE_format_type,
	           binary_format_type;
	char *point = os_path_return_ext(file_name);

	assert(f_list && file_name);
	if (!f_list)
		return;
	
	if (point && !strcmp(point, "dat"))
	{
		ASCII_format_type = FFF_INPUT;
		dBASE_format_type = binary_format_type = FFF_OUTPUT;
	}
	else if (point && !strcmp(point, "dab"))
	{
		dBASE_format_type = FFF_INPUT;
		ASCII_format_type = binary_format_type = FFF_OUTPUT;
	}
	else
	{
		binary_format_type = FFF_INPUT;
		ASCII_format_type = dBASE_format_type = FFF_OUTPUT;
	}
	
	/* Mark data formats */
	while ((format = db_find_format_is_isnot(f_list, FFF_GROUP, FFF_DATA,
	                 FFF_INPUT | FFF_OUTPUT, END_ARGS)) != NULL)
	{
		assert(format == format->check_address);
		if (format != format->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
			return;
		}
		
		if (IS_ASCII(format))
			format->type |= ASCII_format_type;
		else if (IS_DBASE(format))
			format->type |= dBASE_format_type;
		else
			format->type |= binary_format_type;
	}
	
	return;
}

/*
 * NAME: db_free_format_list
 *              
 * PURPOSE: To free format list and the associated formats
 *
 * USAGE: db_free_format_list(FORMAT_LIST_PTR *)
 *
 * RETURNS: void
 *
 * DESCRIPTION: 
 *
 * SYSTEM DEPENDENT FUNCTIONS: none
 *
 * GLOBALS: none
 *
 * AUTHOR: Liping Di, NGDC, (303) 497 - 6284, lpd@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS: format list
 *
 */
#undef ROUTINE_NAME
#define ROUTINE_NAME "db_free_format_list"


void db_free_format_list(FORMAT_LIST_PTR f_list)
{
	dll_free(f_list, (void (*)(void *))ff_free_format);
} 

FORMAT_PTR db_find_format_is_isnot(FORMAT_LIST_PTR f_list, ...)
/*****************************************************************************
 * NAME:  db_find_format_is_isnot()
 *
 * PURPOSE:  Locate the first format in f_list whose type matches is_ftype
 * and not isnot_ftype
 *
 * USAGE:  format = db_find_format_is_isnot(f_list, ...);
 *
 * RETURNS:  FORMAT_PTR if found, NULL if not found
 *
 * DESCRIPTION:  Similar in role to db_find_format() except that format titles
 * are not searched on.  The addition of the isnot_ftype argument allows
 * formats that match the desired bit pattern to be disqualified if they have
 * any bits matching the isnot_ftype bit pattern.  This means I can search
 * for formats that are this, but not that.
 *
 * There is a subtle difference in how is_ftype and isnot_ftype are used.
 * A format must match all bits in is_ftype, but cannot match ANY bit in
 * isnot_ftype in order to be returned by this function.
 *
 * The following code snippet finds all file header formats in f_list that
 * do not have either the FFF_OUTPUT or the FFF_INPUT bit set, and sets the
 * FFF_INPUT bit (thus marking all read/write ambiguous file header formats
 * to read).
 
 * while (format = db_find_format_is_isnot(f_list, FFF_GROUP, FFF_HD | FFF_FILE,
 *                                         FFF_OUTPUT | FFF_INPUT))
 * {
 *   format->type |= FFF_INPUT;
 * }
 *
 * Note that this would be an infinite loop were it not for marking all
 * returned formats in such a way that they fail the search criteria.
 *
 * If FFF_NAME is used as the second argument, then the first format matching
 * the given format type but not having the given title is returned.
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
#define ROUTINE_NAME "db_find_format_is_isnot"

{
	va_list args;
	FF_TYPES_t attribute, is_ftype, isnot_ftype;
	char *key_name = NULL;
	FORMAT_PTR format = NULL;
	
	assert(f_list);
	
	va_start(args, f_list);

	/* Read the arguments */
	/* this really should not be a while(), but I'm allowing a future
	   implementation of both attribute types in one call somehow */
	   
	while ( (attribute = (FF_TYPES_t)va_arg (args, FF_TYPES_t)) != END_ARGS )
	{
		if (attribute == FFF_GROUP)
		{
			is_ftype = (FF_TYPES_t)va_arg (args, FF_TYPES_t);
			isnot_ftype = (FF_TYPES_t)va_arg (args, FF_TYPES_t);
				
			assert(is_ftype);
			assert(isnot_ftype);
			if (!is_ftype || !isnot_ftype)
			{
				err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "zero value format type(s)");
				return(FFF_FORMAT_FMT(dll_first(f_list)));
			}
			break;
		}
		else if (attribute == FFF_NAME)
		{
			is_ftype = (FF_TYPES_t)va_arg (args, FF_TYPES_t);
			key_name = va_arg (args, char *);

			assert(is_ftype);
			assert(key_name);
			if (!is_ftype || !key_name)
			{
				err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "zero value format type/name");
				return(FFF_FORMAT_FMT(dll_first(f_list)));
			}
			break;
		}
		else
		{
			err_push(ROUTINE_NAME, ERR_PARAM_VALUE, "undefined search type");
			return(NULL);
		}
	}
		
	va_end(args);

	f_list = dll_first(f_list);
	while ((format = FFF_FORMAT_FMT(f_list)) != NULL)
	{
		assert(format == format->check_address);
		if (format != format->check_address)
		{
			err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
			return(NULL);
		}

		
		if ( ((format->type & is_ftype) == is_ftype) &&
		     ((attribute == FFF_GROUP && !(format->type & isnot_ftype)) ||
			    (attribute == FFF_NAME && strcmp(format->name, key_name))))
			break;
    
		f_list = dll_next(f_list);
	}

	return(format);
}

void db_delete_rivals(FORMAT_LIST_PTR f_list, FORMAT_PTR save_format)
/*****************************************************************************
 * NAME:  db_delete_rivals()
 *
 * PURPOSE:  Delete all other formats in f_list that have the same type as
 * format.
 *
 * USAGE:  db_delete_rivals(f_list, format);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  Similar in effect to db_delete_all_formats(), using 
 * format->type as the format_type, except that all formats with the same
 * title as format (e.g., itself) are not deleted from f_list.  Like
 * db_replace_format(), this function ignores the file type of the format
 * descriptor in making deletions.  For example, if the given format is
 * binary_input_data, then all input_data formats in f_list, that do not have
 * a matching title, are deleted, regardless if they are binary_input_data,
 * ASCII_input_data, or dbase_input_data.
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
	FORMAT_PTR format;
	FF_TYPES_t format_type;
	
	assert(f_list);
	assert(save_format && save_format == save_format->check_address);
	
	if (save_format != save_format->check_address)
	{
		err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "format check-address");
		return;
	}

	/* Mark binary formats */
	format_type = ((save_format->type | FFF_BINARY) & ~FFF_ASCII) & ~FFF_DBASE;
	while ((format = db_find_format_is_isnot(f_list, FFF_NAME, format_type,
	                 save_format->name)) != NULL)
		format->type = FFF_DELETE_ME;
		
	/* Mark ASCII formats */
	format_type = ((save_format->type | FFF_ASCII) & ~FFF_BINARY) & ~FFF_DBASE;
	while ((format = db_find_format_is_isnot(f_list, FFF_NAME, format_type,
	                 save_format->name)) != NULL)
		format->type = FFF_DELETE_ME;
		
	/* Mark dBASE formats */
	format_type = ((save_format->type | FFF_DBASE) & ~FFF_ASCII) & ~FFF_BINARY;
	while ((format = db_find_format_is_isnot(f_list, FFF_NAME, format_type,
	                 save_format->name)) != NULL)
		format->type = FFF_DELETE_ME;

	db_delete_all_formats(f_list, FFF_DELETE_ME);
}
