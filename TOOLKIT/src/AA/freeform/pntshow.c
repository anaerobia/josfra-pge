
/*******************************************************************************
 * NAME:	pntshow
 *
 * PURPOSE:
 *
 * AUTHOR: 	Mark VanGorp, NGDC, (303)496-6221, mvg@mail.ngdc.noaa.gov
 *
 * USAGE:		
 *
 * COMMENTS
 *
 *****************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#define DEFINE_DATA
#include <freeform.h>
#undef DEFINE_DATA

#include <databin.h>
#include <dataview.h>
#include <vg.h>
#include <df.h>
#include <hfile.h>

/* Function Prototypes */
int add_begin_extent(DATA_BIN_PTR);
int default_format_to_buffer(char *format_description, char **output_format); 

#undef ROUTINE_NAME
#define ROUTINE_NAME "Pntshow"
#define BUFFER_SIZE 61440
#ifdef CCMSC
#define INPUT_DATA_CACHE_SIZE 10240
#else
#define INPUT_DATA_CACHE_SIZE 16384
#endif
#define CLASS_INDEX 0
#define CLASS_DATA 1
#define HDF_OBJECT_DEFAULT 2

#ifdef PROTO
main(int argc,char **argv)
#else
main(argc,argv)
int argc; 
char**argv;
#endif
{

	DATA_BIN_PTR	input_data	= NULL;

/*	char	vgname[MAX_NAME_LENGTH];*/
/*	char	vsname[MAX_NAME_LENGTH];*/
/*	char	vgclass[MAX_NAME_LENGTH];*/
	char	vsclass[MAX_NAME_LENGTH];
	char	file_name[MAX_NAME_LENGTH];

	char*	stdout_str = "stdout";
	char*	data_buffer		= NULL;
	char*	input_format_ptr	= NULL;
	char*	output_format_ptr	= NULL;
	char* 	ch_ptr			= NULL;
	char*	header_output_file	= NULL;
	char* 	data_output_file	= NULL;
	char*	data_format_file	= NULL;
	char*	header_format_file	= NULL;
	char*	format_description	= NULL;
	char*	output_format_file 	= NULL;
	char*	default_format_file 	= NULL;
	char*	name			= NULL;
	char*	fields			= NULL;
	

	FORMAT_PTR	vdata_input_format = NULL;

/*	int k;*/
	int j = 0;
	int index;
	int perc_left = 0;
	int output_file = 0;
	int error	= 0;
	int format_lines_to_skip = 0;
	int	hdf_object_array[]	= {0, 0, 0};


/*	long vg, vgt;*/
/*	long vgotag,vgoref;*/
/*	long vs;*/
/*	long vsotag,vsoref;*/
	long vgid = -1;
	long vsid = -1;
	long vsno = 0;
/*	long	vstag;*/
	long i; /*t,*/ /*nvg,*/ /*n,*/ /*ne,*/ /*nv,*/ /*interlace, vsize*/
/*	long * lonevs;*/ /* array to store refs of all lone vdatas */
/* 	long nlone; total number of lone vdatas */
/*	long descrip_length;*/
	long	num_vdata_bytes	= 0;
	long	vdata_offset	= 0;
  
	long	bytes_to_read	= 0L;
	long	bytes_to_write	= 0L;
	long	data_buffer_size	= BUFFER_SIZE;
	long 	output_bytes = 0L;

	unsigned int output_format_type = 0;
	unsigned short hdf_ref = 0;
	unsigned short hdf_tag = 0;
	unsigned short annotation_hdf_ref = 0;
	unsigned short annotation_hdf_tag = 0;

	ROW_SIZES row_size;
	FF_BUFSIZE bufsize;

	if (argc == 1)
	{
		printf("%s%s",
#ifdef ALPHA
"\nWelcome to pntshow alpha "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm/HDF application\n\n",
#elif defined(BETA)
"\nWelcome to pntshow beta "FF_LIB_VER" "__DATE__" -- an NGDC FreeForm/HDF application\n\n",
#else
"\nWelcome to pntshow release "FF_LIB_VER" -- an NGDC FreeForm/HDF application\n\n",
#endif
"USAGE 1: pntshow packed_HDF_file [-h [header_output_file]]\n\
  [-hof header_output_format] [-d [data_output_file]] [-dof data_output_format]\n\
USAGE 2: pntshow packed_HDF_file [-of default_output_format] [> output_file]\n\
pntshow takes a packed HDF file and displays Vdatas or possibly an SDS\n\
-h: header_output_file is the file designated to contain the data stored in\n\
    a Vdata with a class name of \"Index\" (default is stdout)\n\
-hof: header_output_format is the format used to dump the data into\n\
      header_output_file. (FREEFORM .afm (ASCII) or .bfm (binary) extension)\n\
-d: data_output_file is the file designated to contain the data stored in\n\
    a Vdata with a class name of \"Data\" (default is stdout)\n\
-dof: data_output_format is the format used to dump the data into\n\
      data_output_file (FREEFORM .afm (ASCII) or .bfm (binary) extension)\n\
USAGE 2 uses the hdf2buf defaults if no .eqv specification:\n\
   1) Display the first Vdata with class name \"Data\"\n\
   2) Display the largest Vdata\n\
   3) Display the first SDS\n\
-of: default_output_format is the file containing the output format for\n\
     the default HDF object to be displayed\n\
If an output_format is not specified, the default is the output format\n\
annotation stored in the HDF file. If no annotation, a default ASCII output\n\
format is used.\n"
		      );
		exit(0);
	}
	strcpy(file_name, *(argv + 1));
	if (argc == 2) 
		hdf_object_array[HDF_OBJECT_DEFAULT] = 1;
		
	/* Interpret the remaining command line */
	for (i=2; i<argc; i++) {
		/* header output file */
		if (!strcmp(*(argv+i), "-h")) { 
			i++;
			hdf_object_array[CLASS_INDEX] = 1;
			if ( (i == argc) || (**(argv+i) == '-') ) {
				/* Use stdout */
				i--;
				header_output_file = stdout_str;
			}
			else /* user supplied header output file */
				header_output_file = *(argv+i);

		}
		/* header output format file */
		else if (!strcmp(*(argv+i),"-hof")) {
			i++;
			/* Check Header Format File */
			error = open(*(argv + i), O_RDONLY);
			if(error == -1){
				err_push(ROUTINE_NAME, errno, *(argv+i));
				err_disp();
				exit(1);
			}
			header_format_file = *(argv+i);
			close(error);
		}
		/* Data output format file */
		else if (!strcmp(*(argv+i),"-dof")) {
			i++;
			/* Check Header Format File */
			error = open(*(argv + i), O_RDONLY);
			if(error == -1){
				err_push(ROUTINE_NAME, errno, *(argv+i));
				err_disp();
				exit(1);
			}
			data_format_file = *(argv+i);
			close(error);
		}
		/* data output file */
		else if (!strcmp(*(argv+i),"-d")) {
			i++;
			hdf_object_array[CLASS_DATA] = 1;
			if ( (i == argc) || (**(argv+i) == '-') ) {
				/* Use stdout */
				i--;
				data_output_file = stdout_str;
			}
			else /* user supplied header output file */
				data_output_file = *(argv+i);

		}
		else if (!strcmp(*(argv+i),"-of")) {
			i++;
			hdf_object_array[HDF_OBJECT_DEFAULT] = 1;
			/* Check default Format File */
			error = open(*(argv + i), O_RDONLY);
			if(error == -1){
				err_push(ROUTINE_NAME, errno, *(argv+i));
				err_disp();
				exit(1);
			}
			default_format_file = *(argv+i);
			close(error);
		}
		/* invalid argument */
		else {	
			err_push(ROUTINE_NAME, ERR_MAKE_HDF, strcat("invalid argument --> ", *(argv+i)) );
			err_disp();
			exit(1);
   		} /* end if */
	} /* end for */

	data_buffer = (char *)malloc((size_t)BUFFER_SIZE);
	if(!data_buffer){
        	err_push(ROUTINE_NAME, ERR_MEM_LACK, "data buffer");
        	err_disp();
        	exit(1);
	}

	format_description = (char *)malloc((size_t)INPUT_DATA_CACHE_SIZE);
	if(!format_description){
        	err_push(ROUTINE_NAME, ERR_MEM_LACK, "format description");
        	err_disp();
        	exit(1);
	}
	
	fields = (char *)malloc((size_t)512);
	if(!fields){
        	err_push(ROUTINE_NAME, ERR_MEM_LACK, "fields");
        	err_disp();
        	exit(1);
	}
	for (index=0; index<3; index++) {

		input_data = db_make("vset_file");
		if(!input_data){
			err_push(ROUTINE_NAME, ERR_MAKE_DBIN, "vset_file");
			err_disp();
			exit(1);
		}
		error = db_set(input_data,
					DBIN_BUFFER_SIZE,	DEFAULT_BUFFER_SIZE,
					DBIN_FILE_NAME,		file_name,
					DBIN_FILE_HANDLE,
					MAKE_NAME_TABLE,	NULL,
					END_ARGS);

		if(error){
			err_push(ROUTINE_NAME, ERR_SET_DBIN, file_name);
			err_disp();
			exit(1);
		}

		if ( (index == CLASS_INDEX) && hdf_object_array[index]) {
			error = db_hdf_fmt_to_buffer(input_data, format_description, "Index", &hdf_tag, &hdf_ref);
			if (error) {
				err_push(ROUTINE_NAME, ERR_READ_HDF, "Index format to buffer");
				err_disp();
				exit(1);
			}
			strcpy(vsclass, "Index");
			if (header_output_file == stdout_str) {
				output_file = fileno(stdout);
#ifndef SUNCC
				if( (setmode(output_file, O_BINARY)) == -1){
					err_push(ROUTINE_NAME, errno, "for Index stdout");
					err_disp();
					exit(1);
				}
#endif
			}
			else {
				output_file = open(header_output_file, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
				if (output_file == -1) {
					err_push(ROUTINE_NAME, errno, "Problem opening output header file");
					err_disp();
					exit(1);
				}
			}
			if (header_format_file)
				output_format_file = header_format_file;
		}
		else if ( (index == CLASS_DATA) && hdf_object_array[index]) {
			error = db_hdf_fmt_to_buffer(input_data, format_description, "Data", &hdf_tag, &hdf_ref);
  			if (error) {
				err_push(ROUTINE_NAME, ERR_READ_HDF, "Data format to buffer");
				err_disp();
				exit(1);
			}
			strcpy(vsclass, "Data");
			if (data_output_file == stdout_str) {
				output_file = fileno(stdout);
#ifndef SUNCC
				if( (setmode(output_file, O_BINARY)) == -1){
					err_push(ROUTINE_NAME, errno, "for Index stdout");
					err_disp();
					exit(1);
				}
#endif
			}
			else {
				output_file = open(data_output_file, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
				if (output_file == -1) {
					err_push(ROUTINE_NAME, errno, "Problem opening output data file");
					err_disp();
					exit(1);
				}
			}
			if (data_format_file)
				output_format_file = data_format_file;
		}
		else if ( (index == HDF_OBJECT_DEFAULT) && hdf_object_array[index] ) {
			error = db_hdf_fmt_to_buffer(input_data, format_description, NULL, &hdf_tag, &hdf_ref);
  			if (error) {
				err_push(ROUTINE_NAME, ERR_READ_HDF, "Default format to buffer");
				err_disp();
				exit(1);
			}
			strcpy(vsclass, "Hdf2buf defaults");
			output_file = fileno(stdout);
#ifndef SUNCC
			if( (setmode(output_file, O_BINARY)) == -1){
				err_push(ROUTINE_NAME, errno, "for output file");
				err_disp();
				exit(1);
			}
#endif
			if (default_format_file)
				output_format_file = default_format_file;
			}
		
		else {
			db_free(input_data); 
			continue;
		}

		nt_askvalue(input_data, "data_bytes_offset", (unsigned int)FFV_LONG, (void*)&vdata_offset, data_buffer);
		nt_askvalue(input_data, "data_bytes_available", (unsigned int)FFV_LONG, (void*)&num_vdata_bytes, data_buffer);
		annotation_hdf_ref = 0;
		error = db_hdf_tag_to_rowsize(input_data, DFTAG_DIA, &annotation_hdf_ref, 0, &row_size);
		while (error != -1) {
			lseek(input_data->data_file, row_size.start, SEEK_SET);
			READ_DATA(input_data->data_file, &annotation_hdf_tag, 2, "input_data->data_file");
#if defined(CCMSC) || defined(DEC)
			byte_swap((char *)&annotation_hdf_tag, FFV_USHORT);
#endif
			if (annotation_hdf_tag != DFTAG_VH) {
				error = db_hdf_tag_to_rowsize(input_data, DFTAG_DIA, &annotation_hdf_ref, 0, &row_size);
				continue;
			}
			READ_DATA(input_data->data_file, &annotation_hdf_ref, 2, "input_data->data_file");
#if defined(CCMSC) || defined(DEC)
			byte_swap((char *)&annotation_hdf_ref, FFV_USHORT);
#endif
			if (annotation_hdf_ref == hdf_ref) 
				break;
			error = db_hdf_tag_to_rowsize(input_data, DFTAG_DIA, &annotation_hdf_ref, 0, &row_size);
		}
		/* if error != -1, the format is defined as an annotation in the hdf file */
		if (error != -1) {
			error = db_set(input_data,
						FORMAT_BUFFER, format_description, NULL, NULL,
						DBIN_FILE_POSITION,		vdata_offset,
						DBIN_DATA_AVAILABLE,	num_vdata_bytes,
						DBIN_CACHE_SIZE,        INPUT_DATA_CACHE_SIZE,
						END_ARGS);
	
			if(error){
				err_push(ROUTINE_NAME, ERR_SET_DBIN, "Lookup table");
				err_disp();
				exit(1);
			}
			if (output_format_file)
				error = db_set(input_data, OUTPUT_FORMAT, output_format_file, (char*)NULL, END_ARGS);
			if(error){
				err_push(ROUTINE_NAME, ERR_SET_DBIN, "input_data");
				err_disp();
				exit(1);
			}
			ch_ptr = strstr(format_description, "ASCII_output");
			if (ch_ptr) 
				output_format_type = FFF_ASCII_OUTPUT_DATA;
			else
				output_format_type = FFF_BINARY_OUTPUT_DATA;

		}
		else {
			error = default_format_to_buffer(format_description, &output_format_ptr);
			output_format_type = FFF_ASCII_OUTPUT_DATA;
			input_format_ptr = strchr(format_description, '\n');
			input_format_ptr++;
			error = db_set(input_data,
						INPUT_FORMAT,	(void*)NULL, input_format_ptr,
						OUTPUT_FORMAT,		
						(output_format_file ? output_format_file : (void*)NULL),
						(output_format_file ? (void*)NULL : output_format_ptr),
						DBIN_FILE_POSITION,		vdata_offset,
						DBIN_DATA_AVAILABLE,	num_vdata_bytes,
						DBIN_CACHE_SIZE,        INPUT_DATA_CACHE_SIZE,
						END_ARGS);
	
			if(error){
				err_push(ROUTINE_NAME, ERR_SET_DBIN, "input_data");
				err_disp();
				exit(1);
			}
		}

	/* Set other necessary attributes */
	/* If dealing with header vdata and taking the format from the annotation,
	   then add begin and extent variables if necessary. */
	/* The output_format_type was defined above if using the formats from the
	   annotation. */
	if ( ((os_strcmpi(vsclass, "Index")) == 0) && !header_format_file ) {
		input_data->output_format->type = output_format_type;
		add_begin_extent(input_data);
	}
	else if ( (((os_strcmpi(vsclass, "Data")) == 0) && !data_format_file )) 
		input_data->output_format->type = output_format_type;
	else if ( (index == HDF_OBJECT_DEFAULT) && !default_format_file ) 
		input_data->output_format->type = output_format_type;
	input_data->bytes_to_read = FORMAT_LENGTH(input_data->input_format);
	input_data->records_in_cache = input_data->cache_size / input_data->bytes_to_read;
	/* As of Feb 5, 1994, this is needed for DBIN_DATA_TO_NATIVE to run on
	   Little Endian architectures. Vsets are stored in Big Endian */
	input_data->state.byte_order = (unsigned char)1;
	
	bufsize.total_bytes =  -1; /* on purpose, sets to UINT_MAX with compiler warning */
	bufsize.bytes_used  = ~-1; /* ugly, funky zero */
	bufsize.buffer = input_data->buffer;
	ff_show_format(input_data->input_format, &bufsize);
	fprintf(stderr,"\nClass: %s\nInput format:\n\n%s\n",vsclass, bufsize.buffer);
	ff_show_format(input_data->output_format, &bufsize);
	fprintf(stderr,"\nClass: %s\nOutput format:\n\n%s\n",vsclass, bufsize.buffer);

	/* write the vdata out to file */
	while(input_data->data_available){
	    error = db_events(input_data,
	       		 	PROCESS_FORMAT_LIST, FFF_ALL_TYPES,
				DBIN_DATA_TO_NATIVE, (char*)NULL, (char*)NULL, (FORMAT_PTR*)NULL,
	        		END_ARGS);
		
	    if(error) {
	        err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Process format list");
	        err_disp();
	        exit(1);
	    }
	             
	    /* Convert the cache into the output buffer */
	    error = db_show(input_data,
	        DBIN_BYTE_COUNTS, DBIN_INPUT_CACHE, &bytes_to_read, 
			DBIN_OUTPUT_CACHE, &bytes_to_write,
	        END_ARGS, END_ARGS);
	    if(error) {
	        err_push(ROUTINE_NAME, ERR_SHOW_DBIN, "Finding bytes to read/write");
	        err_disp();
	        exit(1);
	    }
	    if(bytes_to_write > (unsigned int)UINT_MAX){
	        err_push(ROUTINE_NAME, ERR_WRITE_FILE, "Output buffer to large");
	        err_disp();
	        exit(1);
	    }

 	        	    if(bytes_to_write > data_buffer_size){
	        data_buffer = realloc(data_buffer, (size_t) bytes_to_write);
	        if (!data_buffer) {
	               err_push(ROUTINE_NAME, ERR_MEM_LACK, "Realloc data buffer");
	               err_disp();
	               exit(1);
	        }
                	 	data_buffer_size =  bytes_to_write;
        	    }

	    error = db_events(input_data,
                DBIN_CONVERT_CACHE, data_buffer, (void*)NULL, &output_bytes,
                END_ARGS);
	    if(error) {
	       err_push(ROUTINE_NAME, ERR_DBIN_EVENT, "Problem converting cache");
	       err_disp();
	       exit(1);
	    }

	    error = write(output_file, data_buffer, (size_t)output_bytes);
	    if (error == -1) {
	        err_push(ROUTINE_NAME, errno, "Problem writing data_buffer to file");
       		err_disp();
        	exit(1);
	    }
	    perc_left = ((float)input_data->data_available / num_vdata_bytes) *100;
	    fprintf(stderr,"\r%4d %% left to process...",perc_left);
        
	} /* End while */
	fprintf(stderr,"\n");
/*	if (output_file != fileno(stdout))
		close(output_file);
*/	output_format_file = NULL;
	db_free(input_data);
}
} /* main */

#undef ROUTINE_NAME
#define ROUTINE_NAME "add_begin_extent"
int add_begin_extent(DATA_BIN_PTR dbin)
{
	/* add_begin_extent searches for the begin and extent variables
	   existing in the input format but not the output format. If
	   this scenario occurs, the begin and/or extent variables are
	   added to the output format too. */

        FORMAT_PTR input_format = dbin->input_format;
        FORMAT_PTR output_format = dbin->output_format;
        VARIABLE_PTR new_var;
        VARIABLE_LIST_PTR v_list_ptr;
        VARIABLE_LIST_PTR prev_v_list_ptr;
        VARIABLE_PTR var_ptr;
        VARIABLE_PTR prev_var_ptr;

	if ( (var_ptr = ff_find_variable("begin", input_format)) && !(ff_find_variable("begin", output_format)) ) {
		v_list_ptr = dll_insert(output_format->variables, 0);
		if(!v_list_ptr){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"begin: new v_list_ptr");
			return(ERR_MEM_LACK);
		}
		if((new_var = (VARIABLE_PTR)calloc(1, sizeof(VARIABLE))) == NULL) {
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"begin variable");
			return(ERR_MEM_LACK);
		}
		new_var->precision = 0;
		new_var->check_address = (void*)new_var;
		strcpy(new_var->name, var_ptr->name);
		new_var->type = var_ptr->type;
		prev_v_list_ptr = dll_previous(v_list_ptr);
		prev_var_ptr = FFV_VARIABLE(prev_v_list_ptr);
		new_var->start_pos = prev_var_ptr->end_pos + 1;
		if (IS_ASCII(output_format))
			new_var->end_pos = new_var->start_pos + 12;
		else
			new_var->end_pos = new_var->start_pos + sizeof(long) - 1;
		output_format->num_in_list++;
		output_format->max_length = new_var->end_pos;
		v_list_ptr->data_ptr = (void*)new_var;
	}
	
	if ( (var_ptr = ff_find_variable("extent", input_format)) && !(ff_find_variable("extent", output_format)) ) {
		v_list_ptr = dll_insert(output_format->variables, 0);
		if(!v_list_ptr){
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"extent: new v_list_ptr");
			return(ERR_MEM_LACK);
		}
		if((new_var = (VARIABLE_PTR)calloc(1, sizeof(VARIABLE))) == NULL) {
			err_push(ROUTINE_NAME,ERR_MEM_LACK,"extent variable");
			return(ERR_MEM_LACK);
		}
		new_var->precision = 0;
		new_var->check_address = (void*)new_var;
		strcpy(new_var->name, var_ptr->name);
		new_var->type = var_ptr->type;
		prev_v_list_ptr = dll_previous(v_list_ptr);
		prev_var_ptr = FFV_VARIABLE(prev_v_list_ptr);
		new_var->start_pos = prev_var_ptr->end_pos + 1;
		if (IS_ASCII(output_format))
			new_var->end_pos = new_var->start_pos + 12;
		else
			new_var->end_pos = new_var->start_pos + sizeof(long) - 1;
		output_format->num_in_list++;
		output_format->max_length = new_var->end_pos;
		v_list_ptr->data_ptr = (void*)new_var;
	}
	return 0;
}

#undef ROUTINE_NAME
#define ROUTINE_NAME "default_format_to_buffer"
int default_format_to_buffer(char *format_description, char **output_format) {
	
	char*	ch	= format_description;
	char* new_format	= format_description;
	char* new_format_ptr	= NULL;
	char* newline	= NULL;
	short num_read = 0;
	unsigned short num_lines 	= 0;
	unsigned short start 		= 0; 
	unsigned short new_start 		= 1; 
	unsigned short end			= 0;
	unsigned short new_end			= 0;
	unsigned short prec			= 0;
	FF_TYPES_t var_type = 0;
	char str[65];
	char var_type_str[64];
	char errorbuff[MAX_ERRSTR_BUFFER];

	CH_TO_END(new_format);	
	new_format_ptr = ++new_format;
	while((ch = strtok((num_lines++) ? NULL : ch, "\n")) != NULL){

		if (num_lines == 1) 
			continue;
		
		num_read = sscanf(ch,"%s %hu %hu %s %hu", str, &start, &end, var_type_str, &prec);

		if(num_read >= 0 && num_read != 5){
			newline = strchr(ch, EOL_1STCHAR);
			if(newline) *newline = ' ';
			sprintf(errorbuff,"\nLine ->%s<- Does Not Have 5 Entries\n",ch);
			err_push(ROUTINE_NAME,ERR_READ_FILE,errorbuff);
			return(ERR_READ_FILE);
		}

		if(end < start){
			newline = strchr(ch, EOL_1STCHAR);
			if(newline) *newline = ' ';
			sprintf(errorbuff,"\nLine ->%s<- End Position < Start Position\n",ch);
			err_push(ROUTINE_NAME,ERR_READ_FILE,errorbuff);
			return(ERR_READ_FILE);
		}

		/* Determine The Variable Type */
		if(isdigit(*(var_type_str))){
			var_type = atoi(var_type_str);
		}
		else {		/* Convert From String to Variable Type */
			var_type = ff_lookup_number(variable_types, var_type_str);
		}

		if(var_type == USHRT_MAX){
			newline = strchr(ch, EOL_1STCHAR);
			if(newline) *newline = ' ';
			sprintf(errorbuff,"\nLine ->%s<- Unknown Variable Type\n", ch);
			err_push(ROUTINE_NAME,ERR_READ_FILE,errorbuff);
			return(USHRT_MAX);
		}

		switch (var_type) {

		case FFV_LONG:
		case FFV_ULONG:
			new_end = new_start + 12;
			break;

		case FFV_SHORT:
		case FFV_USHORT:
			new_end = new_start + 8;
			break;

		case FFV_UCHAR:
			new_end = new_start + 5;
			break;

		case FFV_CHAR:
			new_end = new_start + (end - start) + 2;
			break;

		case FFV_FLOAT:
			new_end = new_start + 15;
			break;

		case FFV_DOUBLE:
			new_end = new_start + 15;
			break;

		default:	/* Error, Unknown Input Variable type */
			err_push(ROUTINE_NAME,ERR_UNKNOWN_VAR_TYPE,str);
			return(ERR_UNKNOWN_VAR_TYPE);
		}

		/* END OF ERROR CHECKING */

		sprintf(new_format_ptr,"%s %hu %hu %s %hu", str, new_start, new_end, var_type_str, prec);
		CH_TO_END(new_format_ptr);
		*new_format_ptr = '\n';
		new_format_ptr++;
		new_start = new_end + 1;
	}

	ch = format_description;	
	while (ch < (new_format - 1)){
		if (*ch == '\0')
			*ch = '\n';
		ch++;
	}
	*new_format_ptr = '\0';
	*output_format = new_format;
}
