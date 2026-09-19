/*
 * NAME:	ff_skip_header
 *
 * PURPOSE:	This function searches a format specification for a variable
 *					of type header and repositions the file after the header.
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * USAGE:	ff_skip_header(FORMAT *form, int file_des)
 *
 * COMMENTS:	WARNING: This is an obsolete function kept for compatibility
 *							with some older format files. This is not the method recommended
 *							for dealing with headers!! The more modern approach is to use a
 *							file header format in the .fmt file and to process that header with
 *							a PROCESS_FORMAT_LIST event.
 *
 * ERRORS: Seeking past the header or to the beginning of the file.
 *			System File Error,"Seeking Past Header"
 *		System File Error,"Seeking to beginning of file"
 *
 * RETURNS:	The length of the header in bytes
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 */
 
#include <stdlib.h>
#include <freeform.h>
#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_skip_header"

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif

#ifdef CCMSC
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif

#ifdef SUNCC
#include <errno.h>
#endif

unsigned ff_skip_header(FORMAT_PTR format, int file_des)
{
	VARIABLE_LIST_PTR v_list = NULL;

	assert(format && ((void *)format == format->check_address));
	
	v_list = FFV_FIRST_VARIABLE(format);

	while(dll_data(v_list)){
		if(IS_HEADER(FFV_VARIABLE(v_list)))break;
		v_list = dll_next(v_list);
	}

	if(dll_data(v_list)){
		if ((lseek(file_des, FFV_VARIABLE(v_list)->end_pos, SEEK_SET)) == -1L) {
			err_push(ROUTINE_NAME,errno,"Seeking Past Header");
			return(0);
		}
		return(FFV_VARIABLE(v_list)->end_pos);
	}

	/* No header in file */
	if ((lseek(file_des, 0L, SEEK_SET)) == -1L) {
		err_push(ROUTINE_NAME,errno,"Seeking to beginning of file");
		return(0);
	}

        return(0);
}
