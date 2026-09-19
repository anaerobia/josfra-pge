/*
 * NAME:	showbox.c
 *		
 * PURPOSE:	Read a binary point data file and determine the number of
 *						points in each n-dimensional bin.
 *
 * USAGE:	
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@ngdc.noaa.gov
 *
 * COMMENTS: 
 *	
 * KEYWORDS:	
 *
 */

#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#include <maxmin.h>
#include <databin.h>
#include <dataview.h>

struct number{
	long bin;
	long count;
	struct number *lower;
	struct number *higher;
	}*bin_array, *tree();

struct number *tree(struct number *, long);
void ptree(struct number *);

#define IS_VAR_NAME(v,cp) ( isspace(*(cp-1)) && isspace(*(cp + strlen(v->name))) )

#undef ROUTINE_NAME
#define ROUTINE_NAME "showbox"

void main(int argc, char **argv)
{
	int boxes_in_last_level;
	int num_box;
	int error;

	long box;
	long file_size = 0L;
	long total_box = 1;
	long local_buffer_size = 0L;

	double range;
	double width;
	double align;

	char *ch_ptr;
	char *binary_buffer;
	char *scratch_buffer = NULL;
	
    FFF_STD_ARGS std_args;
	DATA_BIN_PTR input		 = NULL;
	DATA_VIEW_PTR view = NULL;
	VARIABLE_PTR var = NULL;
	DLL_NODE_PTR view_list	 = NULL;
	DLL_NODE_PTR ascii_view_list	 = NULL;
	FORMAT_PTR binary_format = NULL;	
	
	if (argc == 1)
	{
		fprintf(stderr, "%s%s",
#ifdef ALPHA_TEST
"\nWelcome to showbox alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n",
#elif defined(BETA_TEST)
"\nWelcome to showbox beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm application\n\n",
#else
"\nWelcome to showbox release "FF_LIB_VER" -- an NGDC FreeForm application\n\n",
#endif
"Default extensions: .bin = binary, .dat = ASCII, .dab = dBASE\n\
\t.fmt = format description file\n\
\t.bfm/.afm/.dfm = binary/ASCII/dBASE format specification file\n\n\
showbox data_file [-f format_file] [-if input_format_file]\n\
                  [-of output_format_file] [-ft \"format title\"]\n\
                  [-ift \"input format title\"] [-oft \"output format title\"]\n\
                  [-b local buffer size (default = 61,440, must be < 65,536)]\n\
                  [-c count: Number of records from (+)head/(-)tail of file]\n\n\
See the FreeForm User's Guide for detailed information\n"
		       );
		exit(1);
	}

	/* Create a standard data bin */
	if (parse_command_line(argc, argv, &std_args))
	{
		err_disp();
		exit(EXIT_FAILURE);
	}
	
	if (!std_args.var_file)
	{
		fprintf(stderr, "Variable File Required (-v filename)\n");
		exit(1);
	}

	if (make_standard_dbin(&std_args, &input, NULL))
	{
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, "Making Standard Data Bin");
		err_disp();
		exit(1);
	}

	fprintf(stderr,"Local Buffer Size: %ld\n", std_args.local_buffer_size);
	scratch_buffer = std_args.local_buffer;
	file_size = input->data_available;
	if (IS_ASCII(input->input_format) || IS_DBASE(input->input_format))
	{
		fprintf(stderr,"Data must be binary\n");
		exit(1);
	}
	else
	{
		binary_buffer = input->cache;
		binary_format = input->input_format;
	}    

	/* Create a list of views from the input format */
	/* Read Variable file and trim the view list */
	ff_file_to_buffer(std_args.var_file, scratch_buffer);
 	view_list = db_format_to_view_list(binary_format, scratch_buffer);

	view_list = dll_first(view_list);
	while(dll_data(view_list)){
		view = (DATA_VIEW_PTR)dll_data(view_list);
		var = (VARIABLE_PTR)(view->var_ptr);
		
		/* Search the scratch buffer for the variable string as a token */
		ch_ptr = scratch_buffer - 1;
		do {
			ch_ptr = strstr(ch_ptr + 1, var->name);
		}while(ch_ptr && !IS_VAR_NAME(var, ch_ptr));
		
		if(!ch_ptr){
			view_list = dll_next(view_list);			
			dll_delete(dll_previous(view_list), (void (*) (void *))dv_free);
			continue;
		}

		/* Read bin parameters for this variable, the user_ymin is used for the width of the bins */
		if (sscanf(  ch_ptr, "%s %lf %lf %lf"
		           , var->name
		           , &(view->user_xmin)
		           , &(view->user_xmax)
		           , &width
		          ) != 4
		   )
		{
			fprintf(stderr,"Problem reading bin parameters\n");
			exit(1);
		}
		fprintf(  stderr, "Variable %s Min, Max, Width: %f %f %f, "
		        , var->name, view->user_xmin, view->user_xmax, width
		       );

		range = view->user_xmax - view->user_xmin;
		num_box = (int)(range/width);
		align = num_box * width;
		if(range - align >= .000001)++num_box;
		
		fprintf(stderr,"%d boxes\n", num_box);
		total_box *= num_box;
				
		if(!IS_FLOAT(var) && !IS_DOUBLE(var)){
			/* Adjust precision to match variable */
			error = var->precision;
			while(error--){
				view->user_xmin *= 10.0F;
				view->user_xmax *= 10.0F;
				width *= 10.0F;
			}
		}
		/* Store bin parameters in view */
		view->user_ymin = width;
		view->increment = num_box;
		
		view_list = dll_next(view_list);
	}

	/* Allocate the list of boxes */
/*	box_list = (long *)calloc(total_box, sizeof(long));
	if(!box_list){
	}
*/	
	while((error = db_events(input, DBIN_NEXT_RECORD, ch_ptr, END_ARGS)) == 0){
		boxes_in_last_level = 1;
		total_box = 0;
		
		/* Loop the list of views */
		view_list = dll_first(view_list);
		while(dll_data(view_list)){
			view = (DATA_VIEW_PTR)dll_data(view_list);
			var = (VARIABLE_PTR)(view->var_ptr);
			ch_ptr = input->data_ptr + (size_t)(var->start_pos - 1);
			memcpy(&align, ch_ptr, FF_VAR_LENGTH(var));
			switch (FFV_TYPE(var)) {
			case FFV_FLOAT:
				box = (long)(0.0001 + floor(*((float *)&align) - view->user_xmin) / view->user_ymin);
				break;
			case FFV_DOUBLE:
				box = (long)(0.0001 + floor(*((double *)&align) - view->user_xmin) / view->user_ymin);
				break;
			case FFV_LONG:
				box = (long)(0.0001 + floor(*((long *)&align) - view->user_xmin) / view->user_ymin);
				break;
			case FFV_ULONG:
				box = (long)(0.0001 + floor(*((unsigned long *)&align) - view->user_xmin) / view->user_ymin);
				break;
			case FFV_SHORT:
				box = (long)(0.0001 + floor(*((short *)&align) - view->user_xmin) / view->user_ymin);
				break;
			case FFV_USHORT:
				box = (long)(0.0001 + floor(*((unsigned short *)&align) - view->user_xmin) / view->user_ymin);
				break;
			case FFV_UCHAR:
				box = (long)(0.0001 + floor(*((unsigned char *)&align) - view->user_xmin) / view->user_ymin);
				break;
			}
			if(box < 0 || box > view->increment){
				fprintf(stderr,"Variable %s out of range: Box: %ld\n", var->name, box);
			}
			else {
				total_box += box * boxes_in_last_level;
				boxes_in_last_level = view->increment;
			}
			view_list = dll_next(view_list);
		}
 		bin_array = tree(bin_array, total_box);

		if (input->data_available < (long)FORMAT_LENGTH(input->input_format))
			break;
		
		fprintf(stderr,"%.1f%% Done   \r", 100.0 * ((float)(input->file_location)) /  file_size);		
    }	/* End of file loop */
	if (error == EOF)fprintf(stderr,"  end of input file reached.\n\n");

	if (input->bytes_available != 0)	
			fprintf(stderr,"%ld BYTES OF DATA NOT PROCESSED.\n\n",input->bytes_available);
	else	fprintf(stderr,"PROCESSING COMPLETE.\n");

	if (error && error != EOF){
			err_push(ROUTINE_NAME, ERR_PROCESS_DATA, NULL);
			err_disp();
			exit(1);
	}
	ptree(bin_array);
}

struct number *tree(struct number *root, long sorter)
{
	if(root == 0){ /* new par has arrived */
		root = (struct number *) malloc(sizeof(struct number));
		root->count = 1;
		root->bin = sorter;
		root->higher = root->lower = NULL;
	}
	else if(sorter == root->bin)root->count++;
	else if(sorter > root->bin)
		root->higher = tree(root->higher, sorter);
	else
		root->lower = tree(root->lower, sorter);
	return(root);
}

void ptree(struct number *root)
{
	if(root != NULL){
		ptree(root->lower);
		printf("%f %ld\n", 1.0 * root->bin, root->count);
		ptree(root->higher);
	}
}

