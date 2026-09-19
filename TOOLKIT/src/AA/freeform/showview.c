/* 
 *      DESCRIPTION -   show_view shows attributes of a view
 *
 *      ARGUMENTS -     
 *
 *      RETURNS -
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 */
#include <stdio.h>
#include <stdarg.h>

#include <freeform.h>
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "dv_show"

int dv_show(DATA_VIEW_PTR view, ...)
{
	va_list args;
	char *ch_ptr = NULL;
	int attribute;
	unsigned u;
	ROW_SIZES *row_size = NULL;

	/* Error checking on NULL parameter view */
	assert(view && ((void *)view == view->check_address));

	va_start(args, view);

	while ( (attribute = va_arg (args, int)) != 0 ) {
		
		assert((void *)view == view->check_address);

		switch (attribute) {
		case BUFFER:    /* Define Output Buffer */
			ch_ptr = va_arg (args, SCRATCH_BUFFER);
			*(ch_ptr) = '\0';
			break;

		case TITLE:
			sprintf(ch_ptr, "View %s:\n", view->title);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_DATA_BIN:
			sprintf(ch_ptr, "Data Bin: %s\n", view->dbin->title);
			CH_TO_END(ch_ptr);
			break;                     

		case VIEW_TYPE:
			sprintf(ch_ptr, "Type: %u\n", view->type);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_DIRECTION:
			sprintf(ch_ptr, "Direction: %u\n", view->type);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_NUM_POINTERS:
			sprintf(ch_ptr, "Number of Pointers: %u\n", view->type);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_UPPER_LEFT:
			sprintf(ch_ptr, "Upper Left X: %u Y: %u\n",
				view->x_upper_left, view->y_upper_left);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_LOWER_RIGHT:
			sprintf(ch_ptr, "Lower right X: %u Y: %u\n",
				view->x_lower_right, view->y_lower_right);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_ROW_SIZES:
			sprintf(ch_ptr, "Row Sizes:\n");
			CH_TO_END(ch_ptr);
			for(u = view->num_pointers, row_size = view->row_sizes;
				u; --u, ++row_size){
				sprintf(ch_ptr, "Start: %ld, %ld bytes\n", row_size->start, row_size->num_bytes);
				CH_TO_END(ch_ptr);
			}
			break;

		case VIEW_INCREMENT:
			sprintf(ch_ptr, "INCREMENT: %d\n", view->increment);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_DATA_SIZE:
			sprintf(ch_ptr, "SIZE: %ld\n", view->size);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_STARTS:
			break;

		case VIEW_USER_XMIN:
			sprintf(ch_ptr, "User xmin: %f\n", view->user_xmin);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_USER_XMAX:
			sprintf(ch_ptr, "User xmax: %f\n", view->user_xmax);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_USER_YMIN:
			sprintf(ch_ptr, "User ymin: %f\n", view->user_ymin);
			CH_TO_END(ch_ptr);
			break;

		case VIEW_USER_YMAX:
			sprintf(ch_ptr, "User ymax: %f\n", view->user_ymax);
			CH_TO_END(ch_ptr);
			break;
		}
	}
	va_end(args);
	return(0);
}

