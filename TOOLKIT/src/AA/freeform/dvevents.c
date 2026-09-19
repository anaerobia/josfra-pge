/* FILENAME: dvevents.c
 *
 * CONTAINS: dv_events()
 *           ff_compare_variables()
 *
 */

/*
 * NAME:        dv_events
 *              
 * PURPOSE:     Handle events for data views.
 *
 * USAGE:       dv_events(DATA_VIEW_PTR, event, [args], event, args,... NULL)
 *
 * RETURNS:     0 if all goes well.
 *
 * DESCRIPTION: This function handles events which are sent to data views.
 * The function processes a list of argument groups which have
 * the form: event [arguments]. The list of groups is terminated
 * by a NULL, which ends processing.
 * The presently supported events and their arguments are:
 *
 * SYSTEM DEPENDENT FUNCTIONS:  Processing of RETURN formats assumes EOL = 2 bytes
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

#define WANT_NCSA_TYPES
#include <freeform.h>
#include <databin.h>
#include <dataview.h>

#ifdef ROUTINE_NAME
#undef ROUTINE_NAME
#endif
#define ROUTINE_NAME "dv_events"

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

/* Allow a maximum of 256 ROW_SIZE structures */
#define MAX_ROWS 256

#ifdef PROTO

/* ff_compare_variables() is also used by sortpts.c */
int ff_compare_variables(char *record_1, char* record_2);

int dv_events(DATA_VIEW_PTR dview, ...)

#else

static int ff_compare_variables();
int dv_events()

#endif
{

	va_list args;

	FF_SCRATCH_BUFFER tmp_buffer_ptr = NULL;
	char *record_1 = NULL;
	char *record_2 = NULL;


	DATA_BIN *dbin;

	DATA_BUFFER start_ptr = NULL;
	DATA_BUFFER nt_replace_rec_ptr = NULL;
	DATA_BUFFER cache_end_ptr   = NULL;

	double *max = NULL;
	double *min = NULL;
	double *double_ptr = NULL;
	double min_value, max_value, scale=1;
	
	short count = 0;
	short intersect = 1;

	int error = 0;
	int event_name = 0;
	int i = 0;
	int cmp_var_return;
	char last_rec_ok = 0;

	long offset;
	long num_records_intersect = 0;
	
	VARIABLE_PTR var = NULL;
	FORMAT_PTR eqn_format = NULL;
	
	extern VARIABLE *key_var;

	PARAM_LIST_PTR pl = NULL;
	POINT_DATA *point;
	
	DLL_NODE_PTR	dll_node;

	ROW_SIZES_PTR row_size_list = NULL;
	ROW_SIZES_PTR row_size = NULL;
	
	assert(dview && ((void *)dview == dview->check_address));
		
	va_start(args, dview);

	while ( (event_name = va_arg (args, int)) != 0 )
	{
		/* Check for possible memory corruption */
		if (dview != dview->check_address)
			return(err_push(ROUTINE_NAME, ERR_MEM_CORRUPT, "dview"));

		switch (event_name)
		{
		/*
		 * MESSAGE:     VIEW_PLIST_TO_ROWSIZE
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:       dview->p_list must be defined
		 *                                      dview->first_pointer and
		 *                                      dview->increment must be defined
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR dview->row_sizes
		 *
		 * DESCRIPTION: 
		 *
		 * ERRORS:      Lack of memory for row_sizes
		 *
		 * AUTHOR:      TH, NGDC,(303)497-6472, haber@ngdc.noaa.gov
		 *
		 * COMMENTS:    (kbf) This message doesn't use a DLL to store the created
		 *				row_sizes structures, but instead uses a #defined constant,
		 *				MAX_ROWS, and allocates an array, assuming that this will be
		 *				sufficient.  This needs to change.
		 *
		 * KEYWORDS:    
		 *
		 */
		case VIEW_PLIST_TO_ROWSIZE:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events VIEW_PLIST_TO_ROWSIZE"

			/* Check for preconditions */
			if (dview->p_list == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->p_list"));
			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));
			if (dview->first_pointer == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->first_pointer"));

			if (row_size_list == NULL)
				row_size_list = (ROW_SIZES_PTR)memMalloc(MAX_ROWS * sizeof(ROW_SIZES), "dv_events: view_plist_to_rowsize: row_size_list");
			if (row_size_list == NULL)
				return(err_push(MESSAGE_NAME,ERR_MEM_LACK,"row_size_list"));

			row_size = row_size_list;
			record_2 = (char*)memMalloc((size_t)FORMAT_LENGTH(dview->dbin->input_format),"dv_events: view_plist_to_rowsize: record_2");
			if (!record_2)
				return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "record_2"));

			/* Loop through the cache, checking each record */
			for (i=1; (unsigned long)i <= (unsigned long)dview->length; i++)
			{
				error = dv_set(dview, VIEW_GET_DATA, 0);

				if (error)
					return(err_push(MESSAGE_NAME, ERR_SET_VIEW, "Getting lookup_table record"));

				/* Initialize pointers */
				pl = dview->p_list;

				while (pl) /* Do a Variable Type Dependent Comparison */
				{
					
					/*** This is how ff_compare_var does it:
					 * 
					 * data_1 = record_1 + key_var->start_pos - 1;
					 * data_2 = record_2 + key_var->start_pos - 1;
					 *
					 * RETURNS:     -1 if variable_1 < variable_2
					 *               0 if the variables are equal
					 *               1 if variable_1 > variable_2
					 *               2 if an error occurs
					 *
					 ***/
					 
					key_var=pl->var;
					record_1 = dview->data;
					
					/*** compare minimum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->minimum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing minimum"));       

					if (cmp_var_return == -1)
					{
						pl = pl->next;
						continue;
					}
					
					/*** compare maximum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->maximum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing maximum"));       

					if (cmp_var_return == 1)
					{
						pl = pl->next;
						continue;
					}
					
					break;

				}       /* End of while (pl) */

				if (pl) /* Data in Range, compute offset and size */
				{
					/* Copy Offset from lookup table */
					var = ff_find_variable("offset", dview->dbin->input_format);
					dview->data = dview->data + var->start_pos - 1;
					memMemcpy((char *)&(row_size->start),
						(char *)dview->data,sizeof(long),NO_TAG);
	
					row_size->num_bytes = do_get_count(FFF_INDEX_ARRAY, dview);
					dview->data = record_1;

					count++;
					row_size++;
				}
			}
			dview->row_sizes = row_size_list;
			dview->num_pointers = count;

			break;

		/*
		 * MESSAGE:     VIEW_QUERY_TO_ROWSIZE
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS:   FORMAT_PTR format the cache is in
		 *
		 * PRECONDITIONS:       dview->eqn_info must be defined
		 *                      dview->first_pointer and
		 *                      dview->increment must be defined
		 *						dview->data must point to the cache
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR dview->row_sizes
		 *
		 * DESCRIPTION:		This message creates a DLL of rowsizes and stuffs it into
		 *					the view->row_sizes structure (after being typecast to a 
		 *					ROW_SIZES_PTR).  The rowsizes are generated using the query
		 *					stored in dview->eqn_info; domain errors which happen inside
		 *					the query functions are ignored and the record is considered
		 *					unwanted.
		 *
		 * ERRORS:      Lack of memory for row_sizes being created.
		 *
		 * AUTHOR:      Kevin Frender (kbf@ngdc.noaa.gov)
		 *
		 * COMMENTS:    This message has NEVER BEEN TESTED.  As of 8/11/94 nothing
		 *				calls it.
		 *
		 *				Like CLIP_VIEW_ON_QUERY, this function calls 
		 *				dv_set(dview, VIEW_GET_NEXT_REC, 0) to find the next record,
		 *				which assumes that the cache is in dbin->input_format.  Although
		 *				it has an additional argument of the cache format (To conform
		 *				to the standards I have tried to create witht the ee_ functions)
		 *				it is important that (at this point) this argument always be 
		 *				dbin->input_format.
		 *
		 * KEYWORDS:    equation
		 *
		 */
		case VIEW_QUERY_TO_ROWSIZE:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events VIEW_QUERY_TO_ROWSIZE"

			eqn_format = va_arg(args, FORMAT_PTR);

			assert(eqn_format && dview->eqn_info && (dview->eqn_info == dview->eqn_info->check_address));

			/* Check for preconditions */
			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));

			if (dview->first_pointer == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->first_pointer"));

			if (!dview->eqn_info)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->eqn_info"));

			if (!dview->data)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->data"));

			if ((dll_node = dll_init()) == NULL)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "Creating a DLL for rowsizes"));

			/* Check to make sure all our variables still exist */
			if (ee_check_vars_exist(dview->eqn_info, eqn_format))
				return(err_push(MESSAGE_NAME, ERR_EE_VAR_NFOUND, "In passed format"));
			
			if ((tmp_buffer_ptr = (FF_SCRATCH_BUFFER)memMalloc((size_t)50, "dv_events: VIEW_QUERY_TO_ROWSIZE: tmp_buffer_ptr")) == NULL)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "For temporary buffer"));

			error = 0;
			count = 0;
			last_rec_ok = 0;

			/* Loop through the cache, checking each record */
			while (1)
			{
				if (ee_set_var_values(dview->eqn_info, dview->data, eqn_format, tmp_buffer_ptr))
				{
					memFree(tmp_buffer_ptr, "dv_events: VIEW_QUERY_TO_ROWSIZE: tmp_buffer_ptr");
					return(err_push(MESSAGE_NAME, ERR_GEN_QUERY, "Setting equation variables"));
				}

				if (ee_evaluate_equation(dview->eqn_info, &error))
				{
					if (!last_rec_ok)
					{
						if ((dll_node = dll_add(dll_node, (unsigned int)sizeof(ROW_SIZES))) == NULL)
						{
							memFree(tmp_buffer_ptr, "dv_events: VIEW_QUERY_TO_ROWSIZE: tmp_buffer_ptr");
							return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "creating row_size struct"));
						}
						row_size = (ROW_SIZES_PTR)(dll_data(dll_node));
						row_size->start = (long)dview->data;
						last_rec_ok = 1;
						nt_replace_rec_ptr = dview->data;
						count++;
					}
				}
				else if (last_rec_ok) /* Record not OK, but last one was */
				{
					last_rec_ok = 0;
					row_size->num_bytes = (long)(nt_replace_rec_ptr - dview->data);
				}
				
				if (error)
				{
					if (error == EE_ERR_MEM_CORRUPT)
					{
						memFree(tmp_buffer_ptr, "dv_events: CLIP_VIEW_ON_QUERY: tmp_buffer_ptr");
						return(err_push(MESSAGE_NAME, ERR_MEM_CORRUPT, "In view->eqn_info"));
					}
				}
				
				if ((dv_set(dview, VIEW_GET_NEXT_REC, 0)) == EOF)
					break;
			}
			if (last_rec_ok) /* Last record was OK */
			{
				last_rec_ok = 0;
				row_size->num_bytes = (long)(nt_replace_rec_ptr - dview->data);
			}
			
			dview->row_sizes = (ROW_SIZES_PTR)dll_next(dll_node);
			dview->num_pointers = count;

			break;
		/*
		 * MESSAGE:     VIEW_PLIST_TO_VIEW_AREA
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:       dview->p_list must be defined
		 *                              dbin->data_type must be point
		 *                           point structure must be defined
		 *
		 * CHANGES IN STATE:    change view->user_xmin, view->user_ymin,..
		 *
		 * DESCRIPTION: 
		 *
		 * ERRORS:      
		 *
		 * AUTHOR:      Liping Di, NGDC,(303)497-6284, lpd@mail.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */
		case VIEW_PLIST_TO_VIEW_AREA:

			/* Check for preconditions */
			if (dview->p_list == NULL)
				return(err_push(ROUTINE_NAME, ERR_STRUCT_FIELD, "dview->p_list"));

			dbin=dview->dbin;
			if (!dbin)
				return(err_push(ROUTINE_NAME, ERR_PTR_DEF, "no dbin associated with view"));

			if (dbin->type != DATA_TYPE_POINT)
				return(err_push(ROUTINE_NAME, ERR_GENERAL, "the data is not point data"));

			point=(POINT_DATA *)dbin->data_parameters;
			if (!point)
				return(err_push(ROUTINE_NAME, ERR_STRUCT_FIELD, "point structure"));

			/* Initialize pointers */
			pl = dview->p_list;

			while (pl) /* get the value */
			{
/*				if (pl->var->type > FFV_CHAR && pl->var->type <= FFV_DOUBLE)*/
				if ( IS_INTEGER(pl->var) || IS_REAL(pl->var) )
				{
					scale=1;
					i=pl->var->precision;
					while (i)
					{
						scale=scale*10;
						i--;
					}
				
					error = btype_to_btype((void *)&pl->minimum, FFV_DATA_TYPE(pl->var),
					                       (void *)&min_value, FFV_DOUBLE);
					if (error)
						return(error);

					error = btype_to_btype((void *)&pl->maximum, FFV_DATA_TYPE(pl->var),
					                       (void *)&max_value, FFV_DOUBLE);
					if (error)
						return(error);

					if ((min_value != max_value) || min_value != 0)
					{
						min_value=min_value/scale;
						max_value=max_value/scale;
						if (memStrcmp(point->x_name, pl->var->name, "dv_events: view_plist_to_view_area") == 0)
						{
							dview->user_xmin=(float)min_value;
							dview->user_xmax=(float)max_value;
						}
						
						if (memStrcmp(point->y_name, pl->var->name, "dv_events: view_plist_to_view_area") == 0)
						{
							dview->user_ymin=(float)min_value;
							dview->user_ymax=(float)max_value;
						}
					}
				}

				pl=pl->next;
			}       /* End of while (pl) */

			break;
		/*
		 * MESSAGE:     MAKE_ROWSIZE_AREA_INTERSECT
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:       dview->p_list must be defined
		 *                                      dview->first_pointer and
		 *                                      dview->increment must be defined
		 *
		 * CHANGES IN STATE:    ALLOCATES MEMORY FOR dview->row_sizes
		 *
		 * DESCRIPTION: MAKE_ROWSIZE_AREA_INTERSECT searches for areas 
		 *                              intersecting a parameterized region. If area does
		 *                              intersect, the offset and size of the given record
		 *                              is added to the rowsize list. For intersection to be
		 *                              true, the data has to lie between the max and min 
		 *                              values given in each p_list node (equivalent to a
		 *                              logical and (&&) of all nodes)
		 *
		 * ERRORS:      Lack of memory for row_sizes
		 *
		 * AUTHOR:      MVG, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS:    (kbf) This message doesn't use a DLL to store the created
		 *				row_sizes structures, but instead uses a #defined constant,
		 *				MAX_ROWS, and allocates an array, assuming that this will be
		 *				sufficient.  This needs to change.
		 *
		 * KEYWORDS:    
		 *
		 */

		case MAKE_ROWSIZE_AREA_INTERSECT:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events MAKE_ROWSIZE_AREA_INTERSECT"

			/* Check for preconditions */
			if (dview->p_list == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->p_list"));

			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));

			if (dview->first_pointer == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->first_pointer"));

			if (row_size_list == NULL)
				row_size_list = (ROW_SIZES_PTR)memMalloc(MAX_ROWS * sizeof(ROW_SIZES), "dv_events make_rowsize_area_intersect: row_size_list");
			if (row_size_list == NULL)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "allocating row_size_list"));

			row_size = row_size_list;
			record_2 = (char*)memMalloc((size_t)FORMAT_LENGTH(dview->dbin->input_format), "dv_events make_rowsize_area_intersect: record_2");
			if (!record_2)
				return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "record_2"));

			/* Loop through the cache, checking each record */
			for (i=1; (unsigned long)i <= (unsigned long)dview->length; i++)
			{
				error = dv_set(dview, VIEW_GET_DATA, 0);
				if (error)
				{
					memFree(record_2, "dv_events: MAKE_ROWSIZE_AREA_INTERSECT: record_2");
					return(err_push(MESSAGE_NAME, ERR_SET_VIEW, "Getting lookup_table record"));
				}

				/* Initialize pointers */
				pl = dview->p_list;

				while (pl) /* Do a Variable Type Dependent Comparison */
				{
					key_var=pl->var;
					record_1 = dview->data;
					
					/*** compare minimum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->minimum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
					{
						memFree(record_2, "dv_events: MAKE_ROWSIZE_AREA_INTERSECT: record_2");
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing minimum"));
					}                                                       
					if (cmp_var_return == -1) 
						break;
					
					/*** compare maximum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->maximum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
					{
						memFree(record_2, "dv_events: MAKE_ROWSIZE_AREA_INTERSECT: record_2");
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing maximum"));       
					}       
					if (cmp_var_return == 1) 
						break;  
					pl = pl->next;


				}       /* End of while (pl) */

				if (!pl) /* Data in Range, compute offset and size */
				{
					/* Copy Offset from lookup table */
					var = ff_find_variable("offset", dview->dbin->input_format);
					dview->data = dview->data + var->start_pos - 1;
					memMemcpy((char *)&(row_size->start),
						(char *)dview->data,sizeof(long),NO_TAG);
	
					row_size->num_bytes = do_get_count(FFF_INDEX_ARRAY, dview);
					dview->data = record_1;

					count++;
					row_size++;
				}
			}
			dview->row_sizes = row_size_list;
			dview->num_pointers = count;
			memFree(record_2, "dv_events: MAKE_ROWSIZE_AREA_INTERSECT: record_2");
			break;
			
		/*
		 * MESSAGE:     CLIP_VIEW_INTERSECT
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS:   None
		 *
		 * PRECONDITIONS:       dview->p_list must be defined
		 *                                      dview->first_pointer and
		 *                                      dview->increment must be defined
		 *
		 * CHANGES IN STATE: dview->cache is changed if necessary for
		 *                                       clipping data
		 *
		 * DESCRIPTION: Takes a data view and clips out irrelevant data.
		 *                              The clipping is done in place in the cache by
		 *                              copying needed records over unwanted records. The
		 *                              cache end is then reset to include only the needed
		 *                              data. Intersection is determined for each parameter
		 *                              list node. If intersection is not true for every
		 *                              node (&&), then the associated record is clipped.
		 *
		 * ERRORS:      
		 *
		 * AUTHOR:      MVG, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS:    
		 *
		 * KEYWORDS:    
		 *
		 */

		case CLIP_VIEW_INTERSECT:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events CLIP_VIEW_INTERSECT"

			count = 0;
			/* Check for preconditions */
			if (dview->p_list == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->p_list"));

			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));

			if (dview->first_pointer == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->first_pointer"));

			nt_replace_rec_ptr = dview->first_pointer;
			record_2 = (char*)memMalloc((size_t)FORMAT_LENGTH(dview->dbin->input_format), "dv_events clip_view_intersect: record_2");
			if (!record_2)
				return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "record_2"));

			/* Loop through the cache, checking each record */
			while ((dv_set(dview, VIEW_GET_DATA, 0)) != EOF)
			{
				/* Initialize pointer */
				pl = dview->p_list;

				while (pl) /* Do a Variable Type Dependent Comparison */
				{
					key_var=pl->var;
					record_1 = dview->data;
					
					/*** compare minimum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->minimum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
					{
						memFree(record_2, "dv_events: CLIP_VIEW_INTERSECT: record_2");
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing minimum"));       
					}       
					if (cmp_var_return == -1) 
						break;
					
					/*** compare maximum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->maximum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
					{
						memFree(record_2, "dv_events: CLIP_VIEW_INTERSECT: record_2");
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing maximum")); 
					}       
					if (cmp_var_return == 1) 
						break;  
					pl = pl->next;
	
				}       /* End of while (pl) */

				if (!pl)
				{
					count++;
					if (nt_replace_rec_ptr != dview->data) 
						memMemcpy((char *)nt_replace_rec_ptr, (char *)dview->data, dview->increment,NO_TAG);
					nt_replace_rec_ptr += dview->increment;
				}
			}
			
			dview->dbin->cache_end = nt_replace_rec_ptr - 1;
			/* seting dview->data to NULL allows VIEW_GET_DATA to once again
			   start at the beginning of the cache (dview->first_pointer) */
			dview->dbin->records_in_cache = count;
			dview->data = NULL;
			memFree(record_2, "dv_events: CLIP_VIEW_INTERSECT: record_2");
			break;

		/*
		 * MESSAGE:     CLIP_VIEW_ON_QUERY
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS:   (FORMAT_PTR) format cache is in currently
		 *
		 * PRECONDITIONS:       dview->eqn_info must be defined
		 *                      dview->first_pointer and
		 *                      dview->increment must be defined
		 *
		 * CHANGES IN STATE: dview->cache is changed if necessary for
		 *                              clipping data
		 *
		 * DESCRIPTION: Takes a data view and clips out irrelevant data.
		 *                  The clipping is done in place in the cache by
		 *                  copying needed records over unwanted records. The
		 *                  cache end is then reset to include only the needed
		 *                  data. The record is wanted if the query evaluates
		 *                  to true.  If domain errors occour in any of the
		 *                  functions in the query, the record is considered
		 *					unwanted, but no error is generated.
		 *
		 * ERRORS:      
		 *
		 * AUTHOR:      Kevin Frender (kbf@kryton.ngdc.noaa.gov)
		 *
		 * COMMENTS:    While this message has an argument for the format the cache
		 *				is currently in (to conform with the ee_ functions), it calls
		 *				dv_set(dview, VIEW_GET_NEXT_REC, 0), which assumes that the
		 *				cache is in dbin->input_format format.  Thus it is apparently
		 *				important that the argument for format, at this time, always
		 *				be input_format.
		 *
		 * KEYWORDS:    equation
		 *
		 */

		case CLIP_VIEW_ON_QUERY:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events CLIP_VIEW_ON_QUERY"
			eqn_format = va_arg(args, FORMAT_PTR);

			assert(dview->eqn_info && (dview->eqn_info == dview->eqn_info->check_address));

			count = 0;
			/* Check for preconditions */
			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));

			if (dview->first_pointer == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->first_pointer"));

			if (!dview->eqn_info)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->eqn_info"));

			nt_replace_rec_ptr = dview->first_pointer;
			dview->data = dview->first_pointer;
			
			if (ee_check_vars_exist(dview->eqn_info, eqn_format))
				return(err_push(MESSAGE_NAME, ERR_EE_VAR_NFOUND, "In passed format"));

			if ((tmp_buffer_ptr = (FF_SCRATCH_BUFFER)memMalloc((size_t)50, "dv_events: CLIP_VIEW_ON_QUERY: tmp_buffer_ptr")) == NULL)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "For temporary buffer"));

			error = 0;

			/* Loop through the cache, checking each record */
			while (1)
			{
				if (ee_set_var_values(dview->eqn_info, dview->data, eqn_format, tmp_buffer_ptr))
				{
					memFree(tmp_buffer_ptr, "dv_events: CLIP_VIEW_ON_QUERY: tmp_buffer_ptr");
					return(err_push(MESSAGE_NAME, ERR_GEN_QUERY, "Seting equation variables"));
				}

				if (ee_evaluate_equation(dview->eqn_info, &error))
				{
					count++;
					if (nt_replace_rec_ptr != dview->data) 
						memMemcpy((char *)nt_replace_rec_ptr, (char *)dview->data, dview->increment,NO_TAG);
					nt_replace_rec_ptr += dview->increment;
				}
				else if (error)
				{
					if (error == EE_ERR_MEM_CORRUPT)
					{
						memFree(tmp_buffer_ptr, "dv_events: CLIP_VIEW_ON_QUERY: tmp_buffer_ptr");
						return(err_push(MESSAGE_NAME, ERR_MEM_CORRUPT, "In view->eqn_info"));
					}
				}
				
				if ((dv_set(dview, VIEW_GET_NEXT_REC, 0)) == EOF)
					break;
			}
			
			dview->dbin->cache_end = nt_replace_rec_ptr - 1;
			/* seting dview->data to NULL allows VIEW_GET_DATA to once again
			   start at the beginning of the cache (dview->first_pointer) */
			dview->dbin->records_in_cache = count;
			dview->data = NULL;
			memFree(tmp_buffer_ptr, "dv_events: CLIP_VIEW_ON_QUERY: tmp_buffer_ptr");
			break;

		/*
		 * MESSAGE:     PLIST_CACHE_TO_ROWSIZE
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS: (view, [ptr to rowsize], [ptr to data], [ptr to cache end])
		 *                              
		 *                              defaults : 
		 *                                ptr to rowsize = dview->rowsizes; or a malloc is done
		 *                                ptr to data = dview->first_pointer
		 *                                ptr to cache end = dview->dbin->cache_end
		 *
		 * PRECONDITIONS: dview->p_list and dview->increment must be defined
		 *
		 * CHANGES IN STATE: Possibly allocates memory for MAX_ROWS 
		 *                                       row_sizes structs
		 *
		 * DESCRIPTION: This will take a p_list and a pointer to data. A cor-
		 *                              responding row_size struct will be filled with an offset
		 *                              and number of bytes of records whose designated variable
		 *                              value lies between the max and min in the p_list node. If
		 *                              the data_ptr does not point to an intersecting value, the
		 *                              values in the next p_list node are checked and the 
		 *                              row_size is pointer advanced to the next structure
		 *
		 * ERRORS: Not enough memory to allocate row_sizes struct -- if necessary
		 *                 cache_end_ptr and dview->data not becoming defined
		 *
		 * AUTHOR:      MVG, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS: This case HAS NOT BEEN tested extensively as it was designed for
		 *                       sortpts, but a decision to go with MAXMIN_CACHE_TO_ROWSIZE was
		 *                       made instead. As of 1/3/93, no application calls this case and it
		 *                       thus may be changed if needed. Also, note that currently the #
		 *                       of nodes in p_list must be <= # of nodes in the row_size list.
		 *
		 * 			    (kbf) This message doesn't use a DLL to store the created
		 *				row_sizes structures, but instead uses a #defined constant,
		 *				MAX_ROWS, and allocates an array, assuming that this will be
		 *				sufficient.  This needs to change.
		 *
		 * KEYWORDS:    
		 *
		 */

		case PLIST_CACHE_TO_ROWSIZE:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events PLIST_CACHE_TO_ROWSIZE"


			/* Check for preconditions */
			if (dview->p_list == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->p_list"));

			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));

			row_size = va_arg(args,ROW_SIZES_PTR);
			if (!row_size) 
				row_size = dview->row_sizes;
			if (!row_size)
				row_size = dview->row_sizes = (ROW_SIZES_PTR)memMalloc(MAX_ROWS * sizeof(ROW_SIZES), "dv_events plist_cache_to_rowsize: row_size");
			if (!row_size)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "row_size"));

			/* Initialize pointers */
			dview->data = va_arg(args, char*);
			cache_end_ptr = va_arg(args, char*);
			if (!dview->data)
				dview->data = dview->first_pointer;
			if (!cache_end_ptr)
				cache_end_ptr = dview->dbin->cache_end;
			if ((!dview->data) || (!cache_end_ptr))
                                return(err_push(MESSAGE_NAME, ERR_PTR_DEF, (char *) ((!dview->data) ? "dview->data":"cache_end_ptr")));

			pl = dview->p_list;
			start_ptr = dview->data;
			intersect = 1;
			record_2 = (char*)memMalloc((size_t)FORMAT_LENGTH(dview->dbin->input_format), "dv_events plist_cache_to_rowsize: record_2");
			if (!record_2)
				return(err_push(ROUTINE_NAME, ERR_MEM_LACK, "record_2"));

			/* Loop through the cache, checking for intersecting records */
			while (pl) /* Do a Variable Type Dependent Comparison */
			{
				if (dview->data <= cache_end_ptr)
				{
					start_ptr = dview->data;
				}

				while (intersect && (dview->data <= cache_end_ptr))
				{
					key_var=pl->var;
					record_1 = dview->data;
					
					/*** compare minimum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->minimum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing minimum"));       

					if (cmp_var_return == -1)
						intersect = 0;
					
					/*** compare maximum ***/
					memMemcpy((void*)(record_2 + key_var->start_pos - (long)1), pl->maximum, FF_VAR_LENGTH(key_var),NO_TAG);
					if ((cmp_var_return = ff_compare_variables((char *)record_1, (char*)record_2)) == 2)
						return(err_push(MESSAGE_NAME, ERR_PROCESS_DATA, "comparing maximum")); 

					if (cmp_var_return == 1)
						intersect = 0;
					
					if (intersect)
					{
						error = dv_set(dview, VIEW_GET_DATA, 0);
						if ((error) && (error != EOF))
							return(err_push(MESSAGE_NAME, ERR_SET_VIEW, "in dv_set VIEW_GET_DATA"));


					}
				
				}       /* End of while intersect */

				if (dview->data <= cache_end_ptr)
				{
					intersect = 1;
					offset = (long)(start_ptr - dview->dbin->cache);
					row_size->start = offset;       
					row_size->num_bytes = (long)(dview->data - start_ptr);
				}
				else
				{
					offset = (long)(start_ptr - dview->dbin->cache);
					row_size->start = offset;
					if (intersect)
					{
						intersect = 0;
						row_size->num_bytes = (long)(dview->data - start_ptr);
					}
					else
						row_size->num_bytes = 0L;
				}

				pl = pl->next;
				if (pl)
					row_size++;
			} /* End of while pl */
			break;

		/*
		 * MESSAGE:     MAXMIN_CACHE_TO_ROWSIZE
		 *
		 * OBJECT TYPE: DATA_VIEW
		 *
		 * ARGUMENTS: (view, [ptr to row_size], double ptr to min, double ptr to max)
		 *                                      default:
		 *                                              ptr to row_size = dview->row_sizes
		 *
		 * PRECONDITIONS: dview->first_pointer, dview->increment, dview->var,
		 * must be defined
		 *
		 * CHANGES IN STATE: memory is allocated for double_ptr and later released
		 *
		 * DESCRIPTION: This case takes double maximum and minimum values and a 
		 *                              a pointer to the data (default is dview->first_pointer). It
		 *                              then finds all susequent records floored between the given
		 *                              maximum and minimum values. When data no longer intersects, 
		 *                              the row_sizes struct is filled with the offset and number of
		 *                              intersecting bytes. If the data pointer begins beyond the end
		 *                              of the cache, then the offset is taken to be the size of the
		 *                              cache
		 *
		 * ERRORS: No memory for double_ptr
		 *                 Problems with dv_set or ff_get_double
		 *
		 * AUTHOR:      MVG, NGDC, (303)497-6221, mvg@kryton.ngdc.noaa.gov
		 *
		 * COMMENTS: Note that no memory is allocated for dview->row_sizes
		 *                       as a default   
		 * 			    (kbf) This message doesn't use a DLL to store the created
		 *				row_sizes structures, but instead trusts that enough room has
		 *				already been allocated in the row_size array of pointers.
		 *				This needs to change.
		 *
		 * KEYWORDS:    
		 *
		 */

		case MAXMIN_CACHE_TO_ROWSIZE:

#undef MESSAGE_NAME
#define MESSAGE_NAME "dv_events MAXMIN_CACHE_TO_ROWSIZE"

			/* Check for preconditions */
			if (dview->increment == 0)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->increment"));

			if (dview->first_pointer == NULL)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->first_pointer"));

			if (!dview->var_ptr)
				return(err_push(MESSAGE_NAME, ERR_STRUCT_FIELD, "dview->var_ptr"));

			double_ptr = (double*)memMalloc(sizeof(double), "dv_events maxmin_cache_to_rowsize: double_ptr");
			if (!double_ptr)
				return(err_push(MESSAGE_NAME, ERR_MEM_LACK, "allocating double_ptr"));

			var = dview->var_ptr;
			row_size = va_arg(args,ROW_SIZES_PTR);
			if (!row_size) 
				row_size = dview->row_sizes;
			if (!row_size)
				return(err_push(MESSAGE_NAME, ERR_PTR_DEF, "rowsize"));

			min = va_arg(args, double*);
			max = va_arg(args, double*);
			if (!min || !max)
                                return(err_push(MESSAGE_NAME, ERR_PTR_DEF, (char *)((!min) ? "minimum value" : "maximum value")));

			if (!dview->data)
			{
				error = dv_set(dview, VIEW_GET_DATA, END_ARGS);
				if ((error) && (error != EOF))
					return(err_push(MESSAGE_NAME, ERR_SET_VIEW, "in dv_set VIEW_GET_DATA"));

			}
			start_ptr = dview->data;
			if (start_ptr <= dview->dbin->cache_end)
			{
				error = ff_get_double(var, dview->data, double_ptr, dview->dbin->input_format->type);
				if (error)
					return(err_push(MESSAGE_NAME, ERR_GET_VALUE, "getting a double"));
			}                       

			/* floor the values between the given double boundaries
			   fabs used for possible floating point deviations */

			while ((dview->data <= dview->dbin->cache_end) &&
			      ((fabs((double)(*double_ptr - *min)) <= DBL_EPSILON) ||
				  ((*double_ptr > *min) && (*double_ptr < *max))))
			{ 
				num_records_intersect++;
				error = dv_set(dview, VIEW_GET_DATA, END_ARGS);
				if ((error) && (error != EOF))
					return(err_push(MESSAGE_NAME, ERR_SET_VIEW, "in dv_set VIEW_GET_DATA"));

				if (dview->data <= dview->dbin->cache_end)
				{
					error = ff_get_double(var, dview->data, double_ptr, dview->dbin->input_format->type);
					if (error)
						return(err_push(MESSAGE_NAME, ERR_GET_VALUE, "getting a double"));
				}  
			}       
			/* if start_ptr was already beyond the end of cache on function
			   call, the offset is set to the size of cache. Else the offset
			   is the # of bytes to beginning of the first intersecting record */

			if (start_ptr <= dview->dbin->cache_end)
				offset = (long)(start_ptr - dview->dbin->cache) - (var->start_pos - 1);
			else
				offset = dview->dbin->cache_size;
			row_size->start = offset;
			row_size->num_bytes = num_records_intersect * dview->increment;
			memFree(double_ptr, "dv_events maxmin_cache_to_rowsize: double_ptr");
			break;

		default:
			err_push(MESSAGE_NAME, ERR_DVIEW_EVENT, "Unknown event");
			return(4);
		}       /* End of Message Switch */
	}       /* End of Attribute Processing (while) */

	va_end(args);
	return(0);
}

/*
 * NAME:        ff_compare_variables  
 *              
 * PURPOSE:     To compare the same variable in two records.
 *
 * USAGE:       int ff_compare_variables(char *record_1, char* record_2)
 *
 * RETURNS:     -1 if variable_1 < variable_2
 *               0 if the variables are equal
 *               1 if variable_1 > variable_2
 *               2 if an error occurs
 *
 * DESCRIPTION: This function is used to compare the same variable in
 *              two different data records (record1 and record 2). It is
 *              called by qsort from sortreg / sortpts and has the typical
 *              characteristics of a qsort compare function. These include
 *              the fact that the function can only accept two pointers as
 *              arguments. In order to compare the records according to any
 *              variable we need the VARIABLE structure for the key.
 *              This comes in as an external pointer to the variable being
 *              used as the key (key_var).
 *
 * ERRORS:
 *		Pointer not defined,"record_1"
 *		Pointer not defined,"record_2"
 *		Possible memory corruption, "key_var"
 *		Unknown variable type,NULL
 * BUS Error when dereferencing a mis-aligned pointer.  This is fixed by
 * copying the to-be-dereferenced bytes into a word-aligned buffer.
 *
 * SYSTEM DEPENDENT FUNCTIONS:  
 *
 * GLOBALS:  key_var
 *
 * AUTHOR:      T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 * modified by  Mark A. Ohrenschall, NGDC, (303) 497 - 6124, mao@ngdc.noaa.gov
 *
 * KEYWORDS:    
 *
 */
/*
 * HISTORY:
 *	Rich Fozzard	7/17/95		-rf01
 *		avoid compile errors in THINK C with #undef ROUTINE_NAME
 *
*/

#undef ROUTINE_NAME		/* needed to avoid compile errors in THINK C -rf01 */
#define ROUTINE_NAME "ff_compare_variables"

int ff_compare_variables(char *record_1, char* record_2)
{
	extern VARIABLE *key_var;
	
	align_var_type align_1 = 0;
	align_var_type align_2 = 0;

	/* Error checking on NULL parameters */
	assert(record_1 && record_2 && (key_var == key_var->check_address));

	if (IS_TEXT(key_var))
		return(memStrncmp(record_1 + key_var->start_pos - 1,
				record_2 + key_var->start_pos - 1, key_var->end_pos -
				key_var->start_pos + 1, "record_1..., record_2..."));

	assert(key_var->end_pos - key_var->start_pos + 1 <= sizeof(align_var_type));
	
	if (key_var->end_pos - key_var->start_pos + 1 > sizeof(align_var_type))
	{
		err_push(ROUTINE_NAME,ERR_VAR_POSITIONS,"field length greater than alignment type");
		return(2);
	}
	
	memMemcpy((char *)&align_1, record_1 + key_var->start_pos - 1,
	          key_var->end_pos - key_var->start_pos + 1, "align_1, ...");
	memMemcpy((char *)&align_2, record_2 + key_var->start_pos - 1,
	          key_var->end_pos - key_var->start_pos + 1, "align_2, ...");

	/* Do a Variable Type Dependent Comparison */
	switch (FFV_DATA_TYPE(key_var))
	{
		char error_buffer[_MAX_PATH];
		
		case FFV_INT8:
			if(*(int8 *)&align_1 < *(int8 *)&align_2)
				return(-1);                                        
			if(*(int8 *)&align_1 > *(int8 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_UINT8:
			if(*(uint8 *)&align_1 < *(uint8 *)&align_2)
				return(-1);                                        
			if(*(uint8 *)&align_1 > *(uint8 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_INT16:
			if(*(int16 *)&align_1 < *(int16 *)&align_2)
				return(-1);                                        
			if(*(int16 *)&align_1 > *(int16 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_UINT16:
			if(*(uint16 *)&align_1 < *(uint16 *)&align_2)
				return(-1);                                        
			if(*(uint16 *)&align_1 > *(uint16 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_INT32:
			if(*(int32 *)&align_1 < *(int32 *)&align_2)
				return(-1);                                        
			if(*(int32 *)&align_1 > *(int32 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_UINT32:
			if(*(uint32 *)&align_1 < *(uint32 *)&align_2)
				return(-1);                                        
			if(*(uint32 *)&align_1 > *(uint32 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_INT64:
			if(*(int64 *)&align_1 < *(int64 *)&align_2)
				return(-1);                                        
			if(*(int64 *)&align_1 > *(int64 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_UINT64:
			if(*(uint64 *)&align_1 < *(uint64 *)&align_2)
				return(-1);                                        
			if(*(uint64 *)&align_1 > *(uint64 *)&align_2)
				return(1);                                        
		break;
		
		case FFV_FLOAT32:
			if(*(float32 *)&align_1 < *(float32 *)&align_2)
				return(-1);                                             
			if(*(float32 *)&align_1 > *(float32 *)&align_2)
				return(1);                                               
		break;
	
		case FFV_FLOAT64:
			if(*(float64 *)&align_1 < *(float64 *)&align_2)
				return(-1);                                             
			if(*(float64 *)&align_1 > *(float64 *)&align_2)
				return(1);                                               
		break;
	
	
		default:
			sprintf(error_buffer, "%d, %s:%d",
			        (int)FFV_DATA_TYPE(key_var), os_path_return_name(__FILE__), __LINE__);
			err_push(ROUTINE_NAME, ERR_SWITCH_DEFAULT, error_buffer);
			err_disp();
			return(2);
	}
	
	return (0);     /* Keys are equal */
}

