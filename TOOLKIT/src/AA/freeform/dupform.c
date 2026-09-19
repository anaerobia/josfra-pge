/*
 * FILENAME: dupform.c
 *
 * CONTAINS: ff_dup_format
 *
 *
 *
 */
#include <stdlib.h>
#include <limits.h>
#include <freeform.h>
/* freeform.h includes:
		ctype.h, stdio.h, string.h, stddef.h, fcntl.h
		types.h, stat.h
		dl_lists.h
		err.h
		for PC: io.h, malloc.h
		for SUN: uio.h stdlib.h
		for Mac: unix.h
*/
#include <os_utils.h>

#if defined(CCLSC) || defined(SUNCC)
#include <errno.h>
#endif


#undef ROUTINE_NAME
#define ROUTINE_NAME "ff_dupe_format"

/*
 * NAME:	ff_dup_format
 *
 * PURPOSE:	To create a format in memory.
 *
 * USAGE:	FORMAT_PTR ff_dup_format(FORMAT_PTR format)
 *
 * RETURNS:	A pointer to the new format, else NULL
 *
 * DESCRIPTION:	This function makes a duplicate of an existing format
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * COMMENTS:	
 *
 * ERRORS:	Memory Allocation Problems:
 *				FORMAT
 *				VARIABLES
 *		,"New Format"
 *		,"variable header"
 *			,old_var->name
 *			,"dll_node"
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *
 * KEYWORDS:	freeform, format
 *
 */
 
FORMAT_PTR ff_dup_format(FORMAT_PTR format)
{
	FORMAT_PTR new_format = NULL;
	VARIABLE_PTR old_var;
	VARIABLE_PTR new_var;
	VARIABLE_LIST *old_list, *new_list;


	if ((new_format = (FORMAT_PTR)memCalloc(1, sizeof(FORMAT), "ff_dup_format new_format")) == NULL) {
		err_push(ROUTINE_NAME, ERR_MEM_LACK,"New Format");
		return(NULL);
	}

	memMemcpy((void*)new_format, (void*)format, sizeof(FORMAT),NO_TAG);
	new_format->check_address=new_format;
	new_format->variables = new_list = dll_init();
	if(!new_list){
		memFree(new_format, "ff_dup_format: Format");
		err_push(ROUTINE_NAME, ERR_MEM_LACK,"variable header");
		return(NULL);
	}

	memStrcat(new_format->name, "_copy",NO_TAG);

	old_list = FFV_FIRST_VARIABLE(format);
	while((old_var = FFV_VARIABLE(old_list)) != NULL)
	{
  	new_var = (VARIABLE *)memMalloc(sizeof(VARIABLE), "ff_dup_format new_var");
		if(!new_var){
			dll_free(format->variables, (void (*)(void *))ff_free_variable);
			memFree(new_format, "ff_dup_format: Format");
			err_push(ROUTINE_NAME, ERR_MAKE_FORM,old_var->name);
			return(NULL);
		}

		memMemcpy((void*)new_var, (void*)old_var, sizeof(VARIABLE),NO_TAG);
		new_var->check_address=new_var;

		new_list=dll_insert(dll_last(new_format->variables), 0);
		if(!new_list){
			dll_free(format->variables, (void (*)(void *))ff_free_variable);
			memFree(new_format, "ff_dup_format: Format");
			err_push(ROUTINE_NAME, ERR_MAKE_FORM,"dll_node");
			return(NULL);
		}
		new_list->data_ptr=new_var;
		old_list=dll_next(old_list);
	}
	return(new_format);
}
