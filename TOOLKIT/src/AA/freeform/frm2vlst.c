/*
 * NAME:        db_format_to_vlist
 *              
 * PURPOSE:     To create a list of views for all variables in a format.
 *
 * USAGE:               DLL_NODE_PTR db_format_to_view_list(FORMAT_PTR format, char *buffer)
 *
 * RETURNS:     Pointer to list if all goes well, NULL if an error occurs.
 *
 * DESCRIPTION: This function creates a list of views for all of the variables
 *              in a format if buffer is NULL, otherwise it creates a list of
 *              views for all the variable names in buffer.
 *
 * ERRORS:
 *                      Problem making view, var->name
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:    
 *
 * KEYWORDS: databins
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>

#include <freeform.h>
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "db_format_to_view_list"

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif

#ifdef CCMSC
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#endif       

#define IS_VAR_NAME(v_start,v_end,buffer, buffer_end) ( ( v_start == buffer || isspace(*(v_start-1)) ) && ( ((unsigned long)v_end <= (unsigned long)buffer_end) && (*v_end == '\0' || isspace(*v_end)) ) )

DLL_NODE_PTR db_format_to_view_list(FORMAT_PTR format, char *buffer)
{
	DLL_NODE_PTR            view_list = NULL;
	DATA_VIEW_PTR   view            = NULL;
	VARIABLE_PTR    var     = NULL;
	VARIABLE_LIST_PTR v_list = NULL;
	
	char * ch_ptr;

	/* Loop the variables in the format */
	v_list = FFV_FIRST_VARIABLE(format);
    while(dll_data(v_list)){
		var = FFV_VARIABLE(v_list); 
		
		if (buffer) {
			/* Search the buffer for the variable string as a token */
			ch_ptr = buffer - 1;
			do {
				ch_ptr = memStrstr(ch_ptr + 1, var->name, "db_format_to_view_list: ch_ptr");
			}while(ch_ptr && !IS_VAR_NAME((char*)ch_ptr, (char*)(ch_ptr + strlen(var->name)), (char*)buffer, (char*)(buffer + strlen(buffer)) ));
		
			if(!ch_ptr) {
				v_list = dll_next(v_list);
				continue;
			}
		}

		/* MAKE A VIEW FOR THIS VARIABLE */
		view = dv_make(var->name, NULL);
		if(!view) {
			err_push(ROUTINE_NAME,ERR_MAKE_VIEW, var->name);
			err_disp();
			return(NULL);
		}
		
		/* Initialize the list and add view */             
		if(!view_list)view_list = dll_init();
		view_list = dll_add(view_list, 0);

		/* SET A POINTER TO VAR FOR FURTHER REFERENCE */
		view->var_ptr = var;
		view_list->data_ptr = (void *)view;
		v_list = dll_next(v_list);
	}
	
	return(dll_next(view_list));
}
