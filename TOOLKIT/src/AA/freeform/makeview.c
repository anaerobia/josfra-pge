/*
 * NAME:        dv_make
 *
 * PURPOSE:     To create and initialize a data view in memory.
 *
 * USAGE:       dv_make(char *title, SCRATCH_BUFFER buffer)
 *
 * RETURNS:     A pointer to the new data view, else NULL
 *
 * DESCRIPTION: This function allocates memory for and initializes
 *                              a data view.
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS:
 *
 * ERRORS:   out of memory, "view"   
 *           out of memory, "view->title"
 *
 * SYSTEM DEPENDENT FUNCTIONS: 
 *
 * KEYWORDS:
 *
 */
/*
 * HISTORY:
 *	r fozzard	4/20/95		-rf01 
 *		(char *) for Think C
*/

#include <stdlib.h>
#include <fcntl.h>

#include <freeform.h>
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "dv_make"

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

DATA_VIEW_PTR dv_make(char *title, SCRATCH_BUFFER buffer)
{
	DATA_VIEW_PTR view = NULL;

	/* Allocate the memory for the view */

	view = (DATA_VIEW_PTR)memMalloc(sizeof(DATA_VIEW), "view");

	if (!view) {
	     err_push(ROUTINE_NAME, ERR_MEM_LACK, "view");
		 return (NULL);
	}

	view->check_address = (void*) view;

	if (title) {
		view->title = (char *)memStrdup(title, "view->title"); /* (char *) for Think C -rf01 */
		if (!view->title) {
			memFree(view, "Data View");
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "view->title");
			return (NULL);
		}
	}

	/* Initialize Pointers */

	view->data = NULL;
	view->buffer = NULL;
	view->row_sizes = NULL;
	view->dbin = NULL;
	view->data = NULL;
	view->output_format = NULL;
	view->info = NULL;
	view->var_ptr = NULL;
	view->p_list = NULL;
	view->eqn_info = NULL;

	view->type = 0;
	view->x_lower_right = 0;
	view->y_lower_right = 0;
	view->x_upper_left = 0;
	view->y_upper_left = 0;
	view->num_pointers = 0;
	view->direction = 0;
	view->increment = 0;
	view->palette   = 1;

	view->type = V_TYPE_INITIALIZED;

	view->size = 0L;

	return(view);
}

