/*
 * NAME:        dv_set
 *              
 * PURPOSE:     Set the attributes of data views.
 *
 * USAGE:       dv_set(DATA_VIEW_PTR, attribute, [args], attribute, args,... NULL)
 *
 * RETURNS:     0 if all goes well.
 *
 * DESCRIPTION: 
		case BUFFER:
		case OUTPUT_FORMAT:
		case TITLE:
		case VIEW_DATA_BIN:
		case VIEW_DATA_SIZE:
		case VIEW_DIRECTION:
		case VIEW_GET_DATA:
			case V_TYPE_RECORD:
			case V_TYPE_VECTOR:
		case VIEW_NUM_POINTERS:
		case VIEW_UPPER_LEFT:
		case VIEW_LOWER_RIGHT:
		case VIEW_INCREMENT:
		case VIEW_STEP:
		case VIEW_SKIP:
		case VIEW_TYPE:
		case VIEW_USER_XMIN:
		case VIEW_USER_XMAX:
		case VIEW_USER_YMIN:
		case VIEW_USER_YMAX:
		case VIEW_INFO:
		case VIEW_QUERY_RESTRICTION:
 *
 * ERRORS:
 *                      Possible memory corruption, view->title
 *                                      Problem deallocating format, "view->output_format"
 *                              Problem making format, "view->output_format"
 *                                      Out of memory, "view title"
 *                              Data bin not defined, "view->dbin"
 *                                      Problem performing dbin events, "Getting data- DBIN_NEXT_RECORD"
 *                                      Problem performing dbin events, "Getting data- DBIN_GET_VECTOR"
 *                              Dbin format not defined, "No Input Format"
 *                              Problem setting view, "Problem making parameter list"
 *                      Unknown object type, "Unknown Data View Attribute"
 *						Out of memory, "Creating a copy of the query restriction"
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * COMMENTS:    
 *                                 
 * KEYWORDS: databins
 *
*/

#include <stdlib.h>
#include <stdarg.h>

#include <freeform.h>
#include <databin.h>
#include <dataview.h>

#undef ROUTINE_NAME
#define ROUTINE_NAME "Set_view"

int dv_set(DATA_VIEW_PTR view, ...)
{
	va_list args;
	SCRATCH_BUFFER  ch_ptr = NULL;
	FORMAT_PTR		eqn_format = NULL;
	char            *file_name = NULL;
	int             attribute;
	int             error = 0;
	char			*equation = NULL;
	char    *ch=NULL;
	
	FORMAT_LIST_PTR f_list = NULL;
	
	FF_BUFSIZE generic_bufsize;
	PP_OBJECT pp_object;
	
	va_start(args, view);

	if (view->buffer)
		ch_ptr = view->buffer;

	while ( (attribute = va_arg (args, int)) != 0 )
	{
		/* Error checking on NULL parameter view */
		assert(view && ((void *)view == view->check_address));

		if (view != view->check_address)
			return(err_push(ROUTINE_NAME,ERR_MEM_CORRUPT, view->title));

		switch (attribute)
		{

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: BUFFER"
		case BUFFER:            /* Define Output Buffer */
			ch_ptr = view->buffer = va_arg (args, SCRATCH_BUFFER);
			*(ch_ptr) = '\0';
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: OUTPUT_FORMAT"
		case OUTPUT_FORMAT:     /* Define Output Format */
			file_name = va_arg (args, char *);

			if(view->output_format) {
				ff_free_format(view->output_format);
				view->output_format = NULL;

				/* Error check in case view->output_format somehow was
				   assigned NULL when passed as a value parameter */

				if (err_state())
				{
					va_end(args);
					return(err_push(MESSAGE_NAME,ERR_FREE_FORM, "view->output_format"));
				}
			}
			
			generic_bufsize.buffer = ch_ptr;
			generic_bufsize.total_bytes = BUFSIZE_TOTAL_BYTES_UNKNOWN;
					
			error = ff_file_to_bufsize(file_name, &generic_bufsize);
			if (error)
				return(err_push(ROUTINE_NAME, error, file_name));

			pp_object.ppo_type = PPO_FORMAT_LIST;
			pp_object.ppo_object.hf_list = &f_list;
	
			if (ff_text_pre_parser(os_path_return_ext(file_name),
			                       &generic_bufsize, 
			                       &pp_object))
			{
			}
			else
			{
				view->output_format = db_find_format(f_list, FFF_NAME, FORMAT_NAME_INIT, END_ARGS);
			}

			if (!view->output_format)
			{
				va_end(args);
				return(err_push(MESSAGE_NAME,ERR_MAKE_FORM, "Generaring view output_format"));
			}

			ch_ptr += strlen(ch_ptr);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: TITLE"
		case TITLE:
			ch = va_arg (args, char *);
			if(view->title != NULL)memFree(view->title, "dv_set: view->title");
			if(ch != NULL){
				view->title = (char *)memMalloc(strlen(ch) + 1, "dv_set: view->title");
				if (!view->title) 
					return(err_push(MESSAGE_NAME,ERR_MEM_LACK, "view title"));
				else
					memStrcpy(view->title, ch,NO_TAG);
			}
			else
				view->title=NULL;

			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_DATA_BIN"
		case VIEW_DATA_BIN:
			view->dbin = va_arg (args, DATA_BIN *);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_DATA_SIZE"
		case VIEW_DATA_SIZE:
			view->size = va_arg (args, long);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_DIRECTION"
		case VIEW_DIRECTION:
			view->direction = (unsigned char)va_arg (args, int);
			break;

		/* VIEW_GET_NEXT_REC gets the next record in the cache, or returns EOF if no more cache */
#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_GET_NEXT_REC"
		case VIEW_GET_NEXT_REC:

			/* Check for a data bin */
			if (!view->dbin)
			{
				view->data = NULL;
				va_end(args);
				return(err_push(MESSAGE_NAME,ERR_BIN_NOT_DEFINED, "view->dbin"));
			}

			error = db_events(view->dbin,
				DBIN_NEXT_REC_IN_CACHE, &(view->data),
				NULL);
			if ((error) && (error != EOF))
			{
				va_end(args);
				return(err_push(MESSAGE_NAME,ERR_DBIN_EVENT, "Getting data- DBIN_NEXT_RECORD"));
			}
			if (error == EOF)
				return(error);
			break;
			

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_GET_DATA"
		case VIEW_GET_DATA:

			/* Check for a data bin */
			if (!view->dbin)
			{
				view->data = NULL;
				va_end(args);
				return(err_push(MESSAGE_NAME,ERR_BIN_NOT_DEFINED, "view->dbin"));
			}
			switch(view->type){

			case V_TYPE_ARRAY:

			/* An ARRAY type view has a view array defined */
				if (view->data)
					view->data += view->increment;
				else
					view->data = view->first_pointer;

				if (view->data > view->dbin->cache_end)
					return(EOF);

				break;

			case V_TYPE_RECORD:

			/* A RECORD type view is a file which has records which have 
			fields which are defined by some format. This view type is cached. */
				error = db_events(view->dbin,
					DBIN_NEXT_RECORD, &(view->data),
					NULL);
				if ((error) && (error != EOF))
				{
					va_end(args);
					return(err_push(MESSAGE_NAME,ERR_DBIN_EVENT, "Getting data- DBIN_NEXT_RECORD"));
				}
				if (error == EOF)
					return(error);
					
				break;

			case V_TYPE_VECTOR:

			/* A VECTOR type view is a file which has vector information.
			At present these vectors consist of a long which gives the number
			of elements in the vector, and then the elements. The VECTOR type
			call takes an offset and a count, reads count bytes from the bin and
			puts them at the view data pointer. In this case, the pointer
			always points to the beginning of the cache. */

				/* get the count and the offset */
				view->dbin->bytes_to_read = va_arg(args, unsigned int);
				view->dbin->file_location = va_arg(args, long);         
				error = db_events(view->dbin,
					DBIN_GET_VECTOR,
					NULL);
				if (error)
				{
					view->data = NULL;
					va_end(args);
					return(err_push(MESSAGE_NAME,ERR_DBIN_EVENT, "Getting data- DBIN_GET_VECTOR"));
				}
				view->data = view->dbin->cache;
				break;
			}
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_FIRST_POINTER"
		case VIEW_FIRST_POINTER:
			view->first_pointer = va_arg (args, DATA_BUFFER);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_LENGTH"
		case VIEW_LENGTH:
			view->length = va_arg (args, unsigned int);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_NUM_POINTERS"
		case VIEW_NUM_POINTERS:
			view->num_pointers = va_arg (args, unsigned int);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_PARAMETER_LIST"
		case VIEW_PARAMETER_LIST:

			/* Make sure dbin has input_format defined */
			if(view->dbin->input_format == NULL)
				return(err_push(MESSAGE_NAME,ERR_BINFORM_DEFINED, "No Input Format"));

			view->p_list = make_param_list(va_arg (args, char *),view->dbin->input_format);
			if(view->p_list == NULL)
				return(err_push(MESSAGE_NAME,ERR_SET_VIEW, "Problem making parameter list"));
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_UPPER_LEFT"
		case VIEW_UPPER_LEFT:
			view->y_upper_left = va_arg (args, unsigned int );
			view->x_upper_left = va_arg (args, unsigned int );
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_LOWER_RIGHT"
		case VIEW_LOWER_RIGHT:
			view->y_lower_right = va_arg (args, unsigned int );
			view->x_lower_right = va_arg (args, unsigned int );
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_INCREMENT"
		case VIEW_INCREMENT:
			view->increment = va_arg (args, int);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_TYPE"
		case VIEW_TYPE:
			view->type = (unsigned char)va_arg (args, long);
			break;


		/* Due to unknown reason, var_arg(args, float) always retrieve
		   wrong value in the microsoft C when the argument is float. 
		   Only va_arg(args, double) can retrieve correct value. LPD */
#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_USER_XMIN"
		case VIEW_USER_XMIN:
			view->user_xmin = (float) va_arg (args, double);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_USER_XMAX"
		case VIEW_USER_XMAX:
			view->user_xmax = (float) va_arg (args, double);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_USER_YMIN"
		case VIEW_USER_YMIN:
			view->user_ymin = (float) va_arg (args, double);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_USER_YMAX"
		case VIEW_USER_YMAX:
			view->user_ymax = (float) va_arg (args, double);
			break;

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_INFO"
		case VIEW_INFO:
			view->info = va_arg(args, void *);
			break;
		
#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_set: VIEW_QUERY_RESTRICTION"
		case VIEW_QUERY_RESTRICTION: /* set up a query restriction on a view */
			equation = va_arg(args, char *);
			eqn_format = va_arg(args, FORMAT_PTR);
			
			assert(equation && eqn_format);
			
			view->eqn_info = ee_make_std_equation(equation, eqn_format);
			if (!view->eqn_info)
				return(err_push(MESSAGE_NAME, ERR_GEN_QUERY, "Setting up the query"));
						
			break;
	
		default:
			err_push(ROUTINE_NAME, ERR_UNKNOWN_OBJECT, "Unknown Data View Attribute");
			return(4);

		} /* switch (attribute) */
	} /* while (attribute = ... ) */
	va_end(args);
	return(0);
}
