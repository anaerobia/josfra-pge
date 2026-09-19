/*
 * NAME:                view_size
 *              
 * PURPOSE:     To create ROW_SIZES structure and calculate the view size for
 *                      the indiced file
 *
 * USAGE:       long view_size(VIEW *view, INDEX *index, int input_file, FORMAT *input_format)
 *
 *
 * RETURNS:     if larger than zero, return the view size, otherwise, an error occurs   
 *
 * DESCRIPTION: *view: pointer to a VIEW structure
 *                              *index: pointer to a INDEX structure
 *                              input_file: handle of the input file
 *                              *input_format:  a pointer to the input format           
 *
 * ERRORS:
 *                      Out of memory,"view->row_sizes"
 *              Out of memory, "view->row_sizes"
 *
 * SYSTEM DEPENDENT FUNCTIONS:  none
 *
 * GLOBALS:     none
 *
 * AUTHOR:      Ted, Liping Di
 *
 * COMMENTS:    none
 *
 * KEYWORDS:    none
 *
 */

#include <stdlib.h>
#include <freeform.h>
#include <os_utils.h>
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "Data View Size"

/* Operating System Dependent Includes: */
#ifdef CCLSC
#include <unix.h>
#endif

#ifdef CCMSC
#include <io.h>
#include <sys\types.h>
#include <malloc.h>
#endif


long view_size(DATA_VIEW *view, INDEX *index, int input_file_never_used, FORMAT *input_format_never_used)
{
	unsigned long box_num;
	unsigned int row;
	unsigned int remaining_boxes;

	long *start = NULL, *end = NULL;
	long headbyte, filesize;
	long view_bytes = 0L;
	LEVEL *level;

	assert(view && ((void *)view == view->check_address));

	/* no index, only have one row structure for whole file */
	if (index == NULL)
	{
		headbyte=db_embedded_header_length(view->dbin);
		filesize = os_filelength(view->dbin->file_name) - headbyte;
		view->num_pointers = 1;
		if(view->row_sizes != NULL)memFree((void *)view->row_sizes, "view_size: view->row_sizes");
		if((view->row_sizes =(ROW_SIZES *)memMalloc((size_t)(view->num_pointers * sizeof(ROW_SIZES)), "view_size: view->row_sizes")) == NULL){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"view->row_sizes");
			return(-1);
		}
		view->row_sizes->start = headbyte;
		view->row_sizes->num_bytes=filesize;
		view->size=filesize;
		return(filesize);
	}

	level=index->levels;

	/* Allocate an array of row sizes */
#ifdef CCMSC
/*      view->row_sizes = (ROW_SIZES far *)_fmalloc(view->num_pointers * sizeof(ROW_SIZES)); */
	view->row_sizes = (ROW_SIZES *)memMalloc(view->num_pointers * sizeof(ROW_SIZES), "view_size: view->row_sizes");
#else
	view->row_sizes = (ROW_SIZES *)memMalloc(view->num_pointers * sizeof(ROW_SIZES), "view_size: view->row_sizes");
#endif
	if(!(view->row_sizes)) {
		err_push(ROUTINE_NAME,ERR_MEM_LACK, "view->row_sizes");
		return(-2);
	}

	/* loop over the rows of the view to determine the size */
	for (box_num = view->y_lower_right * level->num_boxes + view->x_upper_left,
	     row = 0, view_bytes = 0L;
	     row < view->num_pointers;
	     ++row, box_num += level->num_boxes)
	{
		/* Define pointer to beginning of data */
		start = index->offsets + box_num;
		remaining_boxes = (view->x_lower_right - view->x_upper_left + 1);
/*              printf("\nremaining_boxes=%d, %d\n",remaining_boxes, level->num_boxes);
		for(i=0; i< view->length; i++)
			printf("%d, %ld,", i, *(start+i));  */
		while(!*start && remaining_boxes){
			++start;
			--remaining_boxes;
		}

		if(remaining_boxes == 0){       /* No data in this row */
			(view->row_sizes + row)->start = 0L;
			(view->row_sizes + row)->num_bytes = 0L;
			continue;
		}

		(view->row_sizes + row)->start = *start - 1;

		/* Find the next box with data */
		end = start + remaining_boxes;
		while(!*end && end < index->last_offset) ++end;

		if(end < index->last_offset){
			(view->row_sizes + row)->num_bytes = (*end - *start);
		}
		else {
/*                      (view->row_sizes + row)->num_bytes = (view->dbin->bytes_available) - db_embedded_header_length(view->dbin)); */
			(view->row_sizes + row)->num_bytes = os_filelength(view->dbin->file_name) - (db_embedded_header_length(view->dbin));
			(view->row_sizes + row)->num_bytes = 1 + ((view->row_sizes + row)->num_bytes - *start);
		}
		view_bytes += (view->row_sizes + row)->num_bytes;
	}

	return(view_bytes);
}
