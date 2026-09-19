/*
 * NAME:	ff_find_variable
 *		
 * PURPOSE:	This function does a case sensitive search of a format for a
 *						variable with a given name.
 *  		
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 * 		
 * USAGE:	ff_find_variable(char *name, FORMAT_PTR format)
 *			
 * ERRORS:	
 *		Pointer not defined,"Variable Name"
 *		Pointer not defined,"format"
 *		Possible memory corruption, format->name
 * 	
 * COMMENTS:
 * 	
 * RETURNS:	A pointer to the variable if it exists, NULL if it doesn't
 *
 * SYSTEM DEPENDENT FUNCTIONS:
 *  	   
 */ 

#include <freeform.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_find_variable"

VARIABLE_PTR ff_find_variable(char *name, FORMAT_PTR format)
{
	VARIABLE_LIST_PTR v_list = NULL;

	/* Error checking on Pointers */
	assert(name && format && (format == format->check_address) );

	v_list = FFV_FIRST_VARIABLE(format);

	while(dll_data(v_list)){
		if(memStrcmp(name, FFV_VARIABLE(v_list)->name,NO_TAG) == 0)
			return(FFV_VARIABLE(v_list));	/* name in list */
		v_list = dll_next(v_list);
	}
	return(NULL);		/* Not in list */
}

