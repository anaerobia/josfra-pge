/*
 * NAME:	fillhdr
 *		
 * PURPOSE:	To add maximum and minimum data values to the header
 *
 * USAGE:	fillhdr datafile.dat or
 *			fillhdr datafile.bin
 *
 * RETURNS:	
 *
 * DESCRIPTION: fillhdr looks for _min and _max appended to variable names
 *				in the header description. These values are determined by:
 *				scanning through the associated data file; finding the max
 *				and min of each desired variable; and writing these values
 *				back to each appropriate header variable location.
 *				
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	 Mark Van Gorp, NGDC, (303)497-6221, mvg@ngdc.noaa.gov
 *
 * COMMENTS: Space for the header and subsequently each _max and _min to be
 *			 filled must be previously allocated.
 *			 Also, .fmt files are assumed to be used
 *
 * KEYWORDS:	
 *
 */

#include <math.h>
#include <limits.h>    /*  for LONG_MIN & LONG_MAX  */
#include <float.h>     /*  for DBL_MIN & DBL_MAX  */

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#include <databin.h>
#include <dataview.h>


/* Function prototypes */
void print_help(void);
int add_header_info(char *, FF_TYPES_t, VARIABLE_PTR, double);

#undef ROUTINE_NAME
#define ROUTINE_NAME "fillhdr"

/*****************************************************************************
 * NAME:  check_for_unused_flags()
 *
 * PURPOSE:  Has user asked for an unimplemented option?
 *
 * USAGE:  check_for_unused_flags(std_args_ptr);
 *
 * RETURNS:  void
 *
 * DESCRIPTION:  All FreeForm utilities do not employ all of the "standard"
 * FreeForm command line options.  Check if the user has unwittingly asked
 * for any options which this utility will ignore.
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

static void check_for_unused_flags(FFF_STD_ARGS_PTR std_args)
{
	if (std_args->user.set_output_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "output file");
	}
	
	if (std_args->user.set_var_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "variable file");
	}
	
	if (std_args->user.set_query_file)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "query file");
	}
	
	if (std_args->user.set_records_to_read)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "\"head 'n tail\" record count");
	}
	
	if (std_args->user.set_cv_precision)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "precision (checkvar only)");
	}
	
	if (std_args->user.set_cv_missing_data)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "missing data flag (checkvar only)");
	}
	
	if (std_args->user.set_cv_maxbins)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "maximum number of histogram bins (checkvar only)");
	}
	
	if (std_args->user.set_cv_maxmin_only)
	{
		err_push(ROUTINE_NAME, ERR_IGNORED_OPTION, "maximum and minimum processing only (checkvar only)");
	}
	
	if (err_state())
		err_disp();
}
	
#define FILLHDR_CACHE_SIZE		  61440

typedef struct{
	double minimum;
	double maximum;
}DOUBLE_MAX_MIN;

void main(int argc, char **argv)
{ 

	char data_file[MAX_NAME_LENGTH + 1];
/*
	char header_file[MAX_NAME_LENGTH + 1];
*/
	char str[MAX_NAME_LENGTH + 1];
	char* format_file					= NULL;
	char  name_buffer[MAX_NAME_LENGTH];
	char* underscore					= NULL;

	DATA_BIN_PTR 		input_data		= NULL;

	DATA_VIEW_PTR		var_array		= NULL;
	DATA_VIEW_PTR		view_data_ptr	= NULL;
	DATA_VIEW_PTR		view			= NULL;

	DLL_NODE_PTR 		view_list	 	= NULL;
	DLL_NODE_PTR 		view_list_end	= NULL;
	DLL_NODE_PTR 		view_list_ptr	= NULL;

	double double_var;
	double bin_data_src;

	DOUBLE_MAX_MIN  *double_max_min_ptr	= NULL;

	FORMAT_PTR 			input_format	= NULL;

	int error		 					= 0;
	int format_length					= 0;
	int var_length						= 0;

	short count							= 0;
	short lookup;

	FF_TYPES_t 		format_type		= 0;

	VARIABLE_PTR 	format_var 		= NULL;
	VARIABLE_PTR 	header_var 		= NULL;
	VARIABLE_LIST_PTR 	v_list 		= NULL;
	VARIABLE_LIST_PTR 	v2_list 		= NULL;
	
	FFF_STD_ARGS std_args;

	if (argc == 1)
		print_help();

	if (parse_command_line(argc, argv, &std_args))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
	check_for_unused_flags(&std_args);

	std_args.cache_size = FILLHDR_CACHE_SIZE;
	if (make_standard_dbin(&std_args, &input_data, NULL))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
			
	db_show(input_data, SHOW_FORMAT_LIST, FFF_INFO, END_ARGS);
	fprintf(stderr, "%s", input_data->buffer);

	/* Currently a header must be defined */
	if (!input_data->state.header_defined)
	{
		err_push(ROUTINE_NAME, ERR_HEAD_DEFINED, data_file);
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	/* Search through the list of header variables and find all _min and _max.
	   If an _min or _max is found, a view is created for the variable name and
	   linked with a list of variable views. */

	format_length = FORMAT_LENGTH(input_data->input_format);
	v_list = FFV_FIRST_VARIABLE(input_data->header_format);
	while (dll_data(v_list))
	{
		header_var = FFV_VARIABLE(v_list);
		lookup = 0;
		strcpy(name_buffer,header_var->name);
		underscore = strrchr(name_buffer, '_');
		if (underscore)
		{
			if (!os_strcmpi(underscore+1,"min") || !os_strcmpi(underscore+1,"max"))
			{
				lookup = 1;
				*underscore = STR_END;
				v2_list = FFV_FIRST_VARIABLE(input_data->input_format);
				while (dll_data(v2_list))
				{
					format_var = FFV_VARIABLE(v2_list);
					if (!os_strcmpi(name_buffer, format_var->name))
						break;
					else
						v2_list = dll_next(v2_list);
				}
				
				if (!dll_data(v2_list))
				{
					err_push(ROUTINE_NAME, ERR_VARIABLE_NOT_FOUND, name_buffer);
					err_disp();
					exit(EXIT_FAILURE);
				}
			}
		}
		/* Continue if: underscore not found
					    no min/max requested 
						format variable is already in view list */

		if (!underscore || !lookup)
		{
			v_list = dll_next(v_list);
			continue;
		}
		else if (view_list)
		{
			/* Check if a view for that variable has already been created */
			view_list_ptr = dll_first(view_list);
			while (dll_data(view_list_ptr))
			{
				view_data_ptr = dll_data(view_list_ptr);
				if (!os_strcmpi(view_data_ptr->var_ptr->name, format_var->name)) 
					break;
				view_list_ptr = dll_next(view_list_ptr);
			}
			if (dll_data(view_list_ptr))
			{
				v_list = dll_next(v_list);
				continue;
			}
		}

		/* Make a list of views for found variables */
		var_array = dv_make(format_var->name, input_data->buffer);
		if (!var_array)
		{
			err_push(ROUTINE_NAME, ERR_MAKE_VIEW, format_var->name);
			err_disp();
			exit(EXIT_FAILURE);
		}
		
		/* Initialize the list and add view */
		if(!view_list)
			view_list = view_list_end = dll_init();
		view_list_end = dll_add(view_list_end, 0);
		/* Set up the view attributes */
		error = dv_set(var_array,
				 VIEW_DATA_BIN, input_data,
				 VIEW_TYPE, (long)V_TYPE_ARRAY,
				 VIEW_INCREMENT, format_length,
				 VIEW_FIRST_POINTER, ((DATA_BUFFER)(input_data->cache + format_var->start_pos - 1)),
				 VIEW_NUM_POINTERS, 1,
				 END_ARGS);
		if (error)
		{
			err_push(ROUTINE_NAME, ERR_SET_VIEW, format_var->name);
			err_disp();
			exit(EXIT_FAILURE);
		}

		/* SET A POINTER TO FORMAT_VAR FOR FURTHER REFERENCE AND ALLOCATE MAX_MIN */
		var_array->var_ptr = format_var;
		double_max_min_ptr = (DOUBLE_MAX_MIN *)memMalloc(sizeof(DOUBLE_MAX_MIN), "fillhdr: double max/min");
		if (!double_max_min_ptr)
		{
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "double_max_min_ptr");
			err_disp();
			exit(EXIT_FAILURE);
		}

		double_max_min_ptr->maximum = -DBL_MAX;
		double_max_min_ptr->minimum = DBL_MAX;

		/* INCLUDE THE MAXMIN STRUCTURE IN THE VIEW 
		   AND ADD THE VIEW TO THE LINKED LIST DATA */
		var_array->info = (void *)double_max_min_ptr;
		view_list_end->data_ptr = (void *)var_array;
		v_list = dll_next(v_list);

	} /* End setting up view list */

	view_data_ptr = (DATA_VIEW_PTR)dll_data(dll_first(view_list));

	/* TRAVERSE THE LIST OF VIEWS AND FIND MAX/MIN FOR EACH VIEW
	   All variable view max and mins are determined in the current cache
	   before subsequent caches are filled for larger data files */

	while(input_data->data_available)
	{
		error = db_events(input_data,
				PROCESS_FORMAT_LIST, FFF_ALL_TYPES,
				NULL);

		if (error) {
			err_push(ROUTINE_NAME, ERR_DBIN_EVENT, view->title);
			err_disp();
			exit(EXIT_FAILURE);
		}

		view_list_ptr = dll_first(view_list);
		while(dll_data(view_list_ptr))
		{
			view = (DATA_VIEW_PTR)dll_data(view_list_ptr);
			format_var = view->var_ptr;

			view->data = NULL;
			while(dv_set(view, VIEW_GET_DATA, 0) != EOF ) {

				/* Any chars are stored in the parameter list structure
					and no call is made to ff_get_double */
				if (IS_CHAR(format_var)){
					var_length = FF_VAR_LENGTH(format_var);
					(void)memcpy(str, view->data, var_length);
					*(str + var_length) = STR_END;
				}
							
				else if (IS_BINARY(view->dbin->input_format)) {
						/* bin_data_src was included for alignment purposes on the
						   SUN when checking variables of binary data files */
						memcpy((void *)&bin_data_src, (void *)view->data, sizeof(double));
						ff_get_double(format_var, &bin_data_src, &double_var, view->dbin->input_format->type);
						/* Added for floating point error purposes */
						if (double_var <= 0)
							double_var -= DOUBLE_UP;
						else
							double_var += DOUBLE_UP; 
				}
				else 
						ff_get_double(format_var, (FF_DATA_PTR)view->data, &double_var, view->dbin->input_format->type);
					
				/* CHECK IT AGAINST THE MIN AND MAX */
				switch( (format_var->type & FFV_TYPES) ){
				case FFV_ULONG:
				case FFV_LONG:
				case FFV_UCHAR:
				case FFV_SHORT:
				case FFV_USHORT:
				case FFV_FLOAT:
				case FFV_DOUBLE:
					if(double_var < ((DOUBLE_MAX_MIN *)view->info)->minimum) {
						((DOUBLE_MAX_MIN *)view->info)->minimum = double_var;
					}
					if(double_var > ((DOUBLE_MAX_MIN *)view->info)->maximum) {
						((DOUBLE_MAX_MIN *)view->info)->maximum = double_var;
					}
					break;
	
				case FFV_CHAR:
					if (!view->p_list) {
						view->p_list = (PARAM_LIST_PTR)memMalloc(sizeof(PARAM_LIST), "fillhdr: Parameter List");
						view->p_list->minimum = (char *)memMalloc((size_t)(var_length + 1), "fillhdr: Parameter List max/min");
		  				view->p_list->maximum = (char *)memMalloc((size_t)(var_length + 1), "fillhdr: Parameter List max/min");
						view->p_list->next = NULL;
						(void)strcpy((char *)view->p_list->minimum, str);
		  				(void)strcpy((char *)view->p_list->maximum, str);
					}
					else {
						if (strcmp(str, (char *)view->p_list->minimum) < 0)
							(void)strcpy((char *)view->p_list->minimum, str);
						if (strcmp(str, (char *)view->p_list->maximum) > 0)
							(void)strcpy((char *)view->p_list->maximum, str);
					}
					break;
										
				default:
					err_push(ROUTINE_NAME, ERR_UNKNOWN_VAR_TYPE, "Min/Max");
					err_disp();
					exit(EXIT_FAILURE);
				}
		   	}
			view_list_ptr = dll_next(view_list_ptr);
		}
   }

	format_type = input_data->header_format->type;
	format_type &= FFF_FORMAT_TYPES;

	/* All desired max and mins have now been retrieved 
	   Traverse the view list and store in the header buffer */

	view_list_ptr = dll_first(view_list);
	while ((view = dll_data(view_list_ptr)) != NULL)
	{
		strcpy(name_buffer, view->title);
		underscore = name_buffer + strlen(name_buffer);
		strcat(name_buffer,"_max");
		header_var = ff_find_variable(name_buffer,input_data->header_format);
		if (header_var) {
			if (header_var->type == FFV_CHAR) {
				memcpy((void*)(input_data->header + (int)(header_var->start_pos - 1)), (void*)view->p_list->maximum,
				       (header_var->end_pos - header_var->start_pos + 1));
			}
			else {
				double_var = (double)(((DOUBLE_MAX_MIN *)view->info)->maximum);
				add_header_info(input_data->header, format_type, header_var, double_var);
			}
		}
		strcpy(underscore + 1,"min");
		header_var = ff_find_variable(name_buffer,input_data->header_format);
		if (header_var) {
			if (header_var->type == FFV_CHAR) {
				memcpy((void*)(input_data->header + (int)(header_var->start_pos - 1)), (void *)view->p_list->minimum,
				       (header_var->end_pos - header_var->start_pos + 1));
			}
			else {
				double_var = (double)(((DOUBLE_MAX_MIN *)view->info)->minimum);
				add_header_info(input_data->header, format_type, header_var, double_var);
			}
		}

		view_list_ptr = dll_next(view_list_ptr);
	}

	(void)lseek(input_data->data_file, 0L, SEEK_SET);
	(void)write(input_data->data_file, (void *)input_data->header, input_data->header_format->max_length);

	db_free(input_data);
}

int add_header_info(char *buffer, FF_TYPES_t format_type, VARIABLE_PTR header_var, double double_var)
{
	/* add_header_info takes the incoming double_var and writes it to either
	   the ascii or binary header destination
	*/

	char *data_dest              	= NULL;
	char *tmp_str                   = NULL;
	char *ch                        = NULL;
	short variable_length;
	short tmp_variable_length;
	short i;

	variable_length = header_var->end_pos - header_var->start_pos + 1;
	data_dest = buffer + header_var->start_pos - 1; 
	switch (format_type)
	{
		case FFF_ASCII:
		case FFF_DBASE:

			tmp_str = (char *)memMalloc(variable_length + 1, "fillhdr: tmp_str"); 
			if(!tmp_str){
				err_push(ROUTINE_NAME, ERR_MEM_LACK,"temporary string");
				return(1);
			}	
		
			if (IS_INTEGER(header_var))
				for(i=header_var->precision; i>0; i--)
					double_var *= 10;

			switch(FFV_TYPE(header_var))
			{
			case FFV_DOUBLE:
				sprintf(tmp_str, "%lf", double_var);
				break;

			case FFV_FLOAT:
				sprintf(tmp_str, "%f", (float)(double_var));
				break;

			case FFV_LONG:
				sprintf(tmp_str, "%ld", (long)(double_var));
				break;
		
			case FFV_ULONG:
				sprintf(tmp_str, "%lu", (unsigned long)(double_var));
				break;
		
			case FFV_SHORT:
				sprintf(tmp_str, "%d", (short)(double_var));
				break;
		
			case FFV_USHORT:
				sprintf(tmp_str, "%u", (unsigned short)(double_var));
				break;
		
			case FFV_UCHAR:
				sprintf(tmp_str, "%u", (unsigned char)(double_var));
				break;

			default:
				err_push("Add_header_info",ERR_UNKNOWN_VAR_TYPE,"Binary header_var->type");
				return (-1);
			}

			memset((void *)data_dest, ' ',variable_length);
			tmp_variable_length = (short)strlen(tmp_str);

			/* If decimal point, erase insignificant zeroes */
			ch = strchr(tmp_str, '.');
			if (ch) {
				ch = strrchr(tmp_str, STR_END);
				ch--;
				while (*ch == '0' && (tmp_variable_length > 1)) {
					ch--;
					tmp_variable_length--;
				}
				++ch;
				*ch = STR_END;
			}
			memcpy((void*)(data_dest + (int)(variable_length - tmp_variable_length)),
			       (void*)tmp_str, tmp_variable_length);
			return(0);
				

		case FFF_BINARY:

			/* Adjust the precision back to copy the integer types */
			if (IS_INTEGER(header_var))
				for(i=header_var->precision; i>0; i--)
					double_var *= 10;

			return(btype_to_btype(data_dest, FFV_TYPE(header_var), &double_var, FFV_DOUBLE));
			
		break;


		default:
			err_push("Add_header_info",ERR_UNKNOWN_VAR_TYPE,"format_var->type");
			return (-1);

	}
}

void print_help()
{
	fprintf(stderr, "%s%s",
#ifdef ALPHA
"\nWelcome to fillhdr alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n",
#elif defined(BETA)
"\nWelcome to fillhdr beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n",
#else
"\nWelcome to fillhdr release "FF_LIB_VER" -- an NGDC FreeForm application\n\n",
#endif
"Default extensions: .bin = binary, .dat = ASCII, .dab = dBASE\n\
\t.fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE variable description file\n\n\
fillhdr data_file [-f format_file] [-if input_format_file]\n\
                  [-of output_format_file] [-ft \"format title\"]\n\
                  [-ift \"input format title\"] [-oft \"output format title\"]\n\n\
See the FreeForm User's Guide for detailed information.\n\n\
Remember, fillhdr overwrites the first portion of data_file, the length of\n\
which is determined by the header format.\n"
	       );
	exit(EXIT_FAILURE);
}
