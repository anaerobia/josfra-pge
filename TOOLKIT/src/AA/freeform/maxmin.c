/*
 * NAME:	maxmin.c
 *		
 * PURPOSE:	To read a series of files with columns of ASCII data,
 *			determine the best binary format for each file,
 *			convert the files to binary, and paste them together.
 *
 * USAGE:	maxmin file1 file2 .....
 *
 * DESCRIPTION:	
 *
 * SYSTEM DEPENDENT FUNCTIONS:	
 *
 * AUTHOR:	T. Habermann, NGDC, (303) 497 - 6472, haber@mail.ngdc.noaa.gov
 *
 * COMMENTS: 
 *	
 * KEYWORDS:	
 *
 */

#include <math.h>
#include <limits.h>
#include <stdlib.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#include <maxmin.h>
#include <databin.h>
#include <dataview.h>

DLL_NODE_PTR db_format_to_view_list(FORMAT_PTR);
DATA_BIN_PTR make_standard_dbin(int, char **, FFF_STD_ARGS_PTR);

#undef ROUTINE_NAME
#define ROUTINE_NAME "maxmin"

void main(int argc, char **argv)
{
	int error;
	int precision;
	long number_of_records = 0L;
	long result_bytes = 0L;
	long binary_buffer_size = 0L;
	long file_size = 0L;
	char *point = NULL; 
	char *non_zero = NULL;
	char *binary_buffer = NULL;
	unsigned char *precision_info = NULL;
	unsigned int num_lines 	= 0;
	unsigned char difference 	= 0;
	double minimum;
	double maximum;

    FFF_STD_ARGS std_args;
	DATA_BIN_PTR input		 = NULL;
	DATA_VIEW_PTR view = NULL;
	VARIABLE_PTR var = NULL;
	DLL_NODE_PTR view_list	 = NULL;
	DLL_NODE_PTR ascii_view_list	 = NULL;
	FORMAT_PTR binary_format = NULL;	
	
	if(argc == 1){
		fprintf(stderr,"\t\tWelcome to  MAXMIN Version 3.0A\n\t\t  a FREEFORM application by NGDC.\n\n\n");
		fprintf(stderr,"The MAXMIN command line has changed to allow more flexibility!!\n");
		fprintf(stderr,"The MAXMIN command line has several elements with default extensions:\n");
		fprintf(stderr,"data_file: .bin = binary  .dat = ASCII  .dab = dbase\n");
		fprintf(stderr,"format description: .fmt\n");
		fprintf(stderr,"input/output format: .bfm (binary), .afm (ASCII), .dfm(dbase)\n");
		fprintf(stderr,"\n\n");
		fprintf(stderr,"These elements can be arranged in any order following the input file name\n");
		fprintf(stderr,"maxmin data_file [-if input format file] [-of output format file]\n");
		fprintf(stderr,"                  [-b local buffer size (default = 61,440, must be < 65,536)]\n");
		fprintf(stderr,"                  [-c count: Number of records from (+)head/(-)tail of file]\n");

		exit(1);
	}

	/* Create a standard data bin */
	input = make_standard_dbin(argc, argv, &std_args);
	if(!input){
		err_push(ROUTINE_NAME, ERR_MAKE_DBIN, "Making Standard Data Bin");
		err_disp();
		exit(1);
	}
    fprintf(stderr,"Local Buffer Size: %ld\n", std_args.local_buffer_size);
    file_size = input->data_available;

	if(IS_ASCII(input->input_format) || IS_DBASE(input->input_format)){
		binary_format = ff_afm2bfm(input->input_format);
		if(!binary_format){
			err_push(ROUTINE_NAME, ERR_MAKE_FORM, "ASCII Format");
			err_disp();
			exit(1);
		}
		binary_buffer_size = input->records_in_cache * FORMAT_LENGTH(binary_format);

		binary_buffer = (char *)malloc((size_t)binary_buffer_size);
		if (!(binary_buffer)){
			err_push(ROUTINE_NAME, ERR_MEM_LACK, "Binary Buffer");
			err_disp();
			exit(1);
		}
		
		/* If the input format is ASCII or dbase, maxmin determines the precision
		of all variables as well as their minima and maxima. This is done on the
		ASCII cache using a separate list of views from the input format */
 		ascii_view_list = db_format_to_view_list(input->input_format);

		ascii_view_list = dll_first(ascii_view_list);
		while(dll_data(ascii_view_list)){
			view = (DATA_VIEW_PTR)dll_data(ascii_view_list);
			var = (VARIABLE_PTR)(view->var_ptr);

			/* Make a unsigned int to hold the precision for this variable */
			precision_info = (unsigned char *)malloc(sizeof(unsigned char));
			if (!(precision_info)){
				err_push(ROUTINE_NAME, ERR_MEM_LACK, "Precision Information");
				err_disp();
				exit(1);
			}
			*precision_info = 0;
				
			if(dv_set(view,
					 VIEW_DATA_BIN,			input,
					 VIEW_TYPE,					(long)V_TYPE_ARRAY,
					 VIEW_INCREMENT,		FORMAT_LENGTH(input->input_format),
					 VIEW_FIRST_POINTER,    	((DATA_BUFFER)(input->cache + var->start_pos - 1)),
					 VIEW_NUM_POINTERS,		1,
					 VIEW_INFO,					(void *)precision_info,
					 END_ARGS)
			){
	        	err_push(ROUTINE_NAME, ERR_SET_VIEW, view->title);
				err_disp();
				exit(1);
			}
			ascii_view_list = dll_next(ascii_view_list);
	 	}
	 }	/* End of ASCII setup */
	else {
		binary_buffer = input->cache;
		binary_format = input->input_format;
	}
			
	/* Create a list of views from the input format */
 	view_list = db_format_to_view_list(binary_format);

	view_list = dll_first(view_list);
	while(dll_data(view_list)){
		view = (DATA_VIEW_PTR)dll_data(view_list);
		var = (VARIABLE_PTR)(view->var_ptr);

		/* Print Out the Variable Information */
		(void)fprintf(stderr,"%s Start=%u End=%u Precision=%u;Type=%u\n",
			var->name, var->start_pos, var->end_pos, var->precision, var->type);

		if(dv_set(view,
				 VIEW_DATA_BIN,			input,
				 VIEW_TYPE,					(long)V_TYPE_ARRAY,
				 VIEW_INCREMENT,		FORMAT_LENGTH(binary_format),
				 VIEW_FIRST_POINTER,    	((DATA_BUFFER)(binary_buffer + var->start_pos - 1)),
				 VIEW_NUM_POINTERS,		1,
				 END_ARGS)
		){
        	err_push(ROUTINE_NAME, ERR_SET_VIEW, view->title);
			err_disp();
			exit(1);
		}

		/* create a MAXMIN for this variable */
		view->info = (void *)mm_make(var, 0);

		view_list = dll_next(view_list);
	}	/* End setting up view list */

	/* Loop the data file, computing the max and min for each variable */
	while(1){
	
		/* use PROCESS_FORMAT_LIST to fill cache */
		if (error = db_events(input, PROCESS_FORMAT_LIST, 
						FFF_ALL_TYPES, END_ARGS)) break;

		/* Convert ASCII or DBASE data to binary */
		if(ascii_view_list){
			if(db_events(input,
				DBIN_CONVERT_CACHE, binary_buffer, (FORMAT_PTR)binary_format,
				(long*)&result_bytes,
				END_ARGS)){
 				err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Converting cache to binary");
 				err_disp();
 				exit(1);
			}

			/* Initialize ASCII data pointers */			
			ascii_view_list = dll_first(ascii_view_list);
			while(dll_data(ascii_view_list)){
				view = (DATA_VIEW_PTR)dll_data(ascii_view_list);
				view->data = view->first_pointer;
   				ascii_view_list = dll_next(ascii_view_list);
		 	}
		 }	/* End of ASCII section */

		/* Loop the list of views to initialize the binary pointers */
		view_list = dll_first(view_list);
		while(dll_data(view_list)){
			view = (DATA_VIEW_PTR)dll_data(view_list);
			view->data = view->first_pointer;
			view_list = dll_next(view_list);
		}

		/* Loop the records in the cache */				
		for(num_lines = input->records_in_cache; num_lines; --num_lines){

			++number_of_records;

			/* Loop the list of views to fill the minmax structures */
			view_list = dll_first(view_list);
			while(dll_data(view_list)){
				view = (DATA_VIEW_PTR)dll_data(view_list);
				mm_set((MAX_MIN_PTR)view->info,
					MM_MAX_MIN,(char *)view->data,
					END_ARGS);
				view->data += view->increment;
				view_list = dll_next(view_list);
			}   /* End of Binary View Loop */
			
			/* Loop ASCII cache to determine precision */
			if(ascii_view_list){
				ascii_view_list = dll_first(ascii_view_list);
				while(dll_data(ascii_view_list)){
					view = (DATA_VIEW_PTR)dll_data(ascii_view_list);
					var = (VARIABLE_PTR)(view->var_ptr);
					
					if(IS_FLOAT(var) || IS_DOUBLE(var)){
						non_zero = point = view->data + FF_VAR_LENGTH(var) - 1;
						if(*non_zero != ' '){
							while(*non_zero == '0' && *non_zero != '.')--non_zero;
							while(*point != '.')--point;
							difference = ( (unsigned char)(non_zero - point) );
							if(difference > *((unsigned char *)(view->info)))
								 *((unsigned char *)(view->info)) = difference;
						}
					}
				view->data += view->increment;
				ascii_view_list = dll_next(ascii_view_list);
			 	}
			 }	/* End of ASCII section */
			 
		}	/* End of loop through cache */
		fprintf(stderr,"%.1f%% Done   \r", 100.0 * ((float)(input->file_location)) /  file_size);

		if(input->data_available < FORMAT_LENGTH(input->input_format))break;
		
    }	/* End of file loop */

	fprintf(stderr,"%ld Records Processed\n", number_of_records);    
	if (error && error != EOF){
			err_push(ROUTINE_NAME, ERR_PROCESS_DATA, NULL);
			err_disp();
			exit(1);
	}

	if (input->bytes_available != 0)	
			fprintf(stderr,"%ld BYTES OF DATA NOT PROCESSED\n\n",input->bytes_available);
	else
			fprintf(stderr,"PROCESSING COMPLETE\n");

	/* Loop the list of views to output the minmax structures */
	view_list = dll_first(view_list);
	if(ascii_view_list) ascii_view_list = dll_first(ascii_view_list);
	
	while(dll_data(view_list)){
		view = (DATA_VIEW_PTR)dll_data(view_list);
		var = (VARIABLE_PTR)(view->var_ptr);
		minimum = mm_getmn((MAX_MIN_PTR)view->info);
		maximum = mm_getmx((MAX_MIN_PTR)view->info);
		
		switch(FFV_TYPE(var)){
		case FFV_LONG:	
		case FFV_ULONG:
		case FFV_SHORT:
		case FFV_USHORT:
		case FFV_UCHAR:
			precision = var->precision;
			while(precision--){
				minimum /= 10.0;
				maximum /= 10.0;
			}
			sprintf(binary_buffer,"%%s %%.%df %%.%df", var->precision, var->precision);
			printf(binary_buffer, view->title, minimum, maximum);
			break;
			
		case FFV_FLOAT:
		case FFV_DOUBLE:
			sprintf(binary_buffer,"%%s %%.%df %%.%df", var->precision, var->precision);
			printf(binary_buffer, view->title, minimum, maximum);
			break;
			
		case FFV_CHAR:
			printf("%s %s %s", view->title, (char *)((long)(minimum + 0.01)), (char *)((long)(maximum + 0.01)));
			break;
					
	    default:	/* Error, Unknown Output Variable type */
			err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE,"out_var->type");
			err_disp();
			break;
		}
		view_list = dll_next(view_list);
				
		if(ascii_view_list){
			view = (DATA_VIEW_PTR)dll_data(ascii_view_list);		
			printf(" %d\n", *((unsigned char *)(view->info)));
			ascii_view_list = dll_next(ascii_view_list);
		}
		else printf("\n");
			
	}   /* End of View Loop */
}
